#include "TransientBypassRouter.h"
#include <algorithm>
#include <cmath>

namespace pps {

void TransientBypassRouter::prepare(double sampleRate, size_t numChannels, size_t maxLatency)
{
    currentSampleRate = (sampleRate > 1000.0) ? sampleRate : 48000.0;
    allocatedChannels = (numChannels > 0) ? numChannels : 2;

    // Enforce power of two capacity for fast bitwise wrapping
    size_t cap = 1024;
    while (cap < maxLatency + 1024)
        cap <<= 1;

    delayLineCapacity = cap;
    delayLineMask = delayLineCapacity - 1;

    dryDelayBuffers.assign(allocatedChannels, std::vector<float>(delayLineCapacity, 0.0f));
    activationMask.assign(2048, 0.0f);
    transientDelayBuffer.assign(delayLineCapacity, 0.0f);
    prevWetSamples.assign(allocatedChannels, 0.0f);

    detector.prepare(currentSampleRate, allocatedChannels);
    reset();
}

void TransientBypassRouter::reset() noexcept
{
    for (auto &buf : dryDelayBuffers)
        std::fill(buf.begin(), buf.end(), 0.0f);

    std::fill(activationMask.begin(), activationMask.end(), 0.0f);
    std::fill(transientDelayBuffer.begin(), transientDelayBuffer.end(), 0.0f);
    std::fill(prevWetSamples.begin(), prevWetSamples.end(), 0.0f);
    writePos = 0;
    detector.reset();
}

void TransientBypassRouter::setAlignmentDelay(int delaySamples) noexcept
{
    alignmentDelaySamples = std::clamp(delaySamples, 0, static_cast<int>(delayLineCapacity - 1));
}

void TransientBypassRouter::setEnabled(bool shouldBeEnabled) noexcept
{
    isBypassEnabled = shouldBeEnabled;
}

void TransientBypassRouter::feedDryAudio(const float *const *dryChannels, size_t numChannels,
                                         size_t numSamples) noexcept
{
    if (dryChannels == nullptr || numSamples == 0 || delayLineCapacity == 0)
        return;

    const size_t channelsToProcess = std::min(numChannels, allocatedChannels);
    if (activationMask.size() < numSamples)
        activationMask.resize(numSamples * 2, 0.0f);

    // 1. Run transient detector on the incoming dry block
    detector.processBlock(dryChannels, channelsToProcess, numSamples, activationMask.data());

    // 2. Store transient activation mask into circular delay line for latency alignment
    for (size_t i = 0; i < numSamples; ++i)
    {
        transientDelayBuffer[(writePos + i) & delayLineMask] = activationMask[i];
    }

    // 3. Write dry audio into circular delay buffer
    for (size_t ch = 0; ch < channelsToProcess; ++ch)
    {
        if (dryChannels[ch] == nullptr)
            continue;

        float *dst = dryDelayBuffers[ch].data();
        const float *src = dryChannels[ch];

        size_t w = writePos;
        size_t remaining = numSamples;
        size_t srcOffset = 0;

        while (remaining > 0)
        {
            const size_t toWrite = std::min(remaining, delayLineCapacity - w);
            std::copy(src + srcOffset, src + srcOffset + toWrite, dst + w);
            srcOffset += toWrite;
            remaining -= toWrite;
            w = (w + toWrite) & delayLineMask;
        }
    }

    writePos = (writePos + numSamples) & delayLineMask;
}

void TransientBypassRouter::blendOutput(const float *const *wsolaWetChannels, float *const *outputChannels,
                                        size_t numChannels, size_t numSamples) noexcept
{
    if (wsolaWetChannels == nullptr || outputChannels == nullptr || numSamples == 0)
        return;

    const size_t channelsToProcess = std::min(numChannels, allocatedChannels);

    if (!isBypassEnabled)
    {
        // When Attack Clarity is disabled, directly pass through 100% clean pitch-shifted WSOLA audio
        for (size_t ch = 0; ch < channelsToProcess; ++ch)
        {
            if (wsolaWetChannels[ch] != nullptr && outputChannels[ch] != nullptr)
            {
                std::copy(wsolaWetChannels[ch], wsolaWetChannels[ch] + numSamples, outputChannels[ch]);
            }
        }
        return;
    }

    // Dynamic Attack Clarity on the pitch-shifted signal (Neural DSP / Helix style)
    // Latency-aligned: transient detection envelope from dry input delayed by alignmentDelaySamples
    const float sens = detector.getSensitivity();
    // Sensitivity scaling: 1.8 is default baseline, range [0.1, 5.0] maps to [0.2, 2.5]
    const float sensNorm = std::clamp(sens / 1.8f, 0.2f, 2.5f);

    for (size_t ch = 0; ch < channelsToProcess; ++ch)
    {
        if (wsolaWetChannels[ch] == nullptr || outputChannels[ch] == nullptr)
            continue;

        const float *wet = wsolaWetChannels[ch];
        float *out = outputChannels[ch];
        float prevSample = (ch < prevWetSamples.size()) ? prevWetSamples[ch] : 0.0f;

        for (size_t i = 0; i < numSamples; ++i)
        {
            const size_t samplesBack = numSamples - i + static_cast<size_t>(alignmentDelaySamples);
            const size_t readIdx = (writePos + delayLineCapacity - (samplesBack & delayLineMask)) & delayLineMask;
            const float mask = transientDelayBuffer[readIdx];

            const float cur = wet[i];
            const float hfEdge = cur - prevSample;
            prevSample = cur;

            if (mask > 0.001f)
            {
                // Dynamic attack crest factor punch boost + high-frequency plectrum snap
                const float gainBoost = 1.0f + mask * (0.45f * sensNorm);
                const float edgeBoost = hfEdge * mask * (0.35f * sensNorm);
                float shaped = (cur * gainBoost) + edgeBoost;

                // Transparent soft saturation knee above 0.85 to preserve punch without digital clipping
                if (shaped > 0.85f)
                {
                    shaped = 0.85f + 0.15f * std::tanh((shaped - 0.85f) / 0.15f);
                }
                else if (shaped < -0.85f)
                {
                    shaped = -0.85f + 0.15f * std::tanh((shaped + 0.85f) / 0.15f);
                }

                out[i] = shaped;
            }
            else
            {
                out[i] = cur;
            }
        }

        if (ch < prevWetSamples.size())
            prevWetSamples[ch] = prevSample;
    }
}

void TransientBypassRouter::extractDelayedDry(float *const *destChannels, size_t numChannels,
                                              size_t numSamples) const noexcept
{
    if (destChannels == nullptr || numSamples == 0 || delayLineCapacity == 0)
        return;

    const size_t channelsToProcess = std::min(numChannels, allocatedChannels);

    for (size_t ch = 0; ch < channelsToProcess; ++ch)
    {
        if (destChannels[ch] == nullptr)
            continue;

        float *dst = destChannels[ch];
        const float *delayBuf = dryDelayBuffers[ch].data();

        for (size_t i = 0; i < numSamples; ++i)
        {
            const size_t samplesBack = numSamples - i + static_cast<size_t>(alignmentDelaySamples);
            const size_t readIdx = (writePos + delayLineCapacity - (samplesBack & delayLineMask)) & delayLineMask;
            dst[i] = delayBuf[readIdx];
        }
    }
}

void TransientBypassRouter::setSensitivity(float sensitivity) noexcept
{
    detector.setSensitivity(sensitivity);
}

void TransientBypassRouter::setHoldTimeMs(float holdMs) noexcept
{
    detector.setHoldTimeMs(holdMs);
}

float TransientBypassRouter::getSensitivity() const noexcept
{
    return detector.getSensitivity();
}

void TransientBypassRouter::process(const float *const *dryChannels, const float *const *wsolaWetChannels,
                                    float *const *outputChannels, size_t numChannels, size_t numSamples) noexcept
{
    feedDryAudio(dryChannels, numChannels, numSamples);
    blendOutput(wsolaWetChannels, outputChannels, numChannels, numSamples);
}

} // namespace pps

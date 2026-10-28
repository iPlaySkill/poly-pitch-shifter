#include "WsolaEngine.h"
#include <cmath>
#include <cstring>

namespace pps {

void WsolaEngine::prepare(double sampleRate, size_t maxBlockSize, size_t numChannels)
{
    currentSampleRate = (sampleRate > 1000.0) ? sampleRate : 48000.0;
    allocatedChannels = (numChannels > 0) ? numChannels : 2;
    currentMaxBlock = (maxBlockSize > 0) ? maxBlockSize : 512;

    updateWindowAndHopSizes();

    // Pre-allocate history buffer (16384 samples provides ample lookback for 44.1 - 96 kHz)
    historyBuffer.prepare(allocatedChannels, 16384);

    // Pre-allocate correlation finder
    correlationFinder.prepare(windowSize, static_cast<size_t>(maxSearchOffset));

    // Pre-calculate Hann window
    Windowing::generate(WindowType::Hann, windowSize, hannWindow);

    // Pre-allocate resampler
    resampler.prepare(allocatedChannels, currentMaxBlock * 4);

    // Pre-allocate synthesis ring buffers (capacity 16384)
    synthesisBuffer.assign(allocatedChannels, std::vector<float>(16384, 0.0f));

    // Pre-allocate anti-aliasing filter and filtered input buffers
    antiAliasingFilter.prepare(currentSampleRate, allocatedChannels);
    filteredInputBuffers.assign(allocatedChannels, std::vector<float>(currentMaxBlock * 4, 0.0f));

    // Pre-allocate scratch vectors for audio processing to guarantee zero heap allocation in process()
    const size_t searchLen = windowSize + 2 * static_cast<size_t>(maxSearchOffset) + hopSize;
    channelRefBuffers.assign(allocatedChannels, std::vector<float>(windowSize, 0.0f));
    channelSearchBuffers.assign(allocatedChannels, std::vector<float>(searchLen, 0.0f));
    grainBuffers.assign(allocatedChannels, std::vector<float>(windowSize, 0.0f));
    resampledOutputBuffers.assign(allocatedChannels, std::vector<float>(currentMaxBlock * 4, 0.0f));
    contiguousSynthBuffer.assign(currentMaxBlock * 8 + windowSize * 4, 0.0f);

    reset();
}

void WsolaEngine::updateWindowAndHopSizes()
{
    // Fix: 1024 samples at 48 kHz (21.33 ms) for better LF stability
    windowSize = 1024;
    hopSize = 512;
    maxSearchOffset = 512;
}

void WsolaEngine::reset() noexcept
{
    historyBuffer.reset();
    resampler.reset();
    antiAliasingFilter.reset();

    for (auto &buf : synthesisBuffer)
        std::fill(buf.begin(), buf.end(), 0.0f);

    synthesisReadPos = 0;
    synthesisWritePos = 0;
    lastBestOffset = 0;
    nominalAnalysisPos = 0.0;
    isFirstFrame = true;
    transientOnset = false;
    currentPitchRatio = targetPitchRatio;
}

void WsolaEngine::setPitchShift(float semitones, float cents) noexcept
{
    const double totalSemitones = static_cast<double>(semitones) + (static_cast<double>(cents) / 100.0);
    const double ratio = std::pow(2.0, totalSemitones / 12.0);
    setPitchRatio(ratio);
}

void WsolaEngine::setPitchRatio(double ratio) noexcept
{
    targetPitchRatio = std::clamp(ratio, 0.4, 2.5);
    antiAliasingFilter.updateForPitchRatio(targetPitchRatio);
}

int WsolaEngine::getLatencyInSamples() const noexcept
{
    // Algorithmic latency corresponding to lookback window
    return 1024; 
}

void WsolaEngine::processWsolaFrame() noexcept
{
    const size_t synthCap = synthesisBuffer[0].size();
    const size_t synthMask = synthCap - 1;

    // Available synthesis samples in the synthesis buffer ahead of read position
    const size_t availableSynth = (synthesisWritePos >= synthesisReadPos)
                                      ? (synthesisWritePos - synthesisReadPos)
                                      : (synthCap - synthesisReadPos + synthesisWritePos);

    // Target latency in samples (e.g. 1024 at 48k)
    const double targetLatency = static_cast<double>(getLatencyInSamples());

    // Because the resampler advances through synthesis audio at rate 'resampleRatio = currentPitchRatio',
    // 'availableSynth' synthesis samples correspond to (availableSynth / currentPitchRatio) input samples.
    // To ensure constant latency L at the output of the resampler:
    // The grain placed at synthesisWritePos corresponds to input lookback:
    // nominalLookback = L - (availableSynth / currentPitchRatio)
    const double synthInInputSamples = static_cast<double>(availableSynth) / currentPitchRatio;
    double nominalLookback = targetLatency - synthInInputSamples;
    if (nominalLookback < 0.0)
        nominalLookback = 0.0;

    const size_t searchLen = windowSize + 2 * static_cast<size_t>(maxSearchOffset) + hopSize;

    // Calculate lookbackOffset for historyBuffer.extractSegment.
    // extractSegment extracts samples from [writePos - lookbackOffset - searchLen] to [writePos - lookbackOffset].
    // With offset maxSearchOffset in search buffer having lookback nominalLookback:
    // lookbackOffset = nominalLookback - maxSearchOffset - hopSize
    const int64_t rawLookbackOffset = static_cast<int64_t>(std::round(nominalLookback)) -
                                      static_cast<int64_t>(maxSearchOffset) -
                                      static_cast<int64_t>(hopSize);
    const size_t clampedLookbackOffset = (rawLookbackOffset > 0) ? static_cast<size_t>(rawLookbackOffset) : 0;

    for (size_t ch = 0; ch < allocatedChannels; ++ch)
    {
        historyBuffer.extractSegment(ch, clampedLookbackOffset, channelSearchBuffers[ch].data(), searchLen);
    }

    int bestDelta = 0;

    if (isFirstFrame)
    {
        for (size_t ch = 0; ch < allocatedChannels; ++ch)
        {
            std::copy(channelSearchBuffers[ch].data() + maxSearchOffset,
                      channelSearchBuffers[ch].data() + maxSearchOffset + windowSize, channelRefBuffers[ch].data());
        }
        bestDelta = 0;
        isFirstFrame = false;
    }
    else
    {
        // Search for optimal phase-coherent offset across all channels
        const float *refPtrs[8]{nullptr};
        const float *searchPtrs[8]{nullptr};
        const size_t chCount = std::min(allocatedChannels, size_t(8));

        for (size_t ch = 0; ch < chCount; ++ch)
        {
            refPtrs[ch] = channelRefBuffers[ch].data();
            searchPtrs[ch] = channelSearchBuffers[ch].data();
        }

        const auto correlation =
            correlationFinder.findBestStereoOffset(refPtrs, searchPtrs, chCount, windowSize, maxSearchOffset, static_cast<int>(lastBestOffset));
        bestDelta = correlation.bestOffset;
        lastCorrelationScore = correlation.peakCorrelation;
    }

    lastBestOffset = bestDelta;
    const size_t bestGrainOffset = static_cast<size_t>(maxSearchOffset + bestDelta);

    // Extract best-matching grain for each channel and overlap-add into synthesis buffer
    for (size_t ch = 0; ch < allocatedChannels; ++ch)
    {
        const float *searchData = channelSearchBuffers[ch].data() + bestGrainOffset;

        // Apply Hann window
        Windowing::apply(searchData, grainBuffers[ch].data(), hannWindow.data(), windowSize);

        // Overlap-add into circular synthesis buffer
        float *synth = synthesisBuffer[ch].data();
        for (size_t i = 0; i < windowSize; ++i)
        {
            const size_t writeIdx = (synthesisWritePos + i) & synthMask;
            synth[writeIdx] += grainBuffers[ch][i];
        }

        // Prepare reference for the next synthesis hop: the natural continuation of this grain
        // Advanced by hopSize in the search buffer
        const size_t nextRefOffset = bestGrainOffset + hopSize;
        if (nextRefOffset + windowSize <= searchLen)
        {
            std::copy(channelSearchBuffers[ch].data() + nextRefOffset,
                      channelSearchBuffers[ch].data() + nextRefOffset + windowSize, channelRefBuffers[ch].data());
        }
    }

    synthesisWritePos = (synthesisWritePos + hopSize) & synthMask;
}

void WsolaEngine::process(const float *const *inputChannels, float *const *outputChannels, size_t numChannels,
                          size_t numSamples) noexcept
{
    if (inputChannels == nullptr || outputChannels == nullptr || numSamples == 0)
        return;

    const size_t channelsToProcess = std::min(numChannels, allocatedChannels);

    // Smooth currentPitchRatio towards targetPitchRatio
    if (std::abs(currentPitchRatio - targetPitchRatio) > 1e-4)
    {
        currentPitchRatio += (targetPitchRatio - currentPitchRatio) * smoothingCoeff;
    }
    else
    {
        currentPitchRatio = targetPitchRatio;
    }

    // Dynamic anti-aliasing pre-filtering when pitching up (alpha > 1.0)
    antiAliasingFilter.updateForPitchRatio(currentPitchRatio);

    if (antiAliasingFilter.isFilteringActive())
    {
        if (filteredInputBuffers[0].size() < numSamples)
        {
            for (auto &buf : filteredInputBuffers)
                buf.resize(numSamples * 2, 0.0f);
        }

        float *filteredPtrs[8]{nullptr};
        const size_t chLimit = std::min(channelsToProcess, size_t(8));
        for (size_t ch = 0; ch < chLimit; ++ch)
        {
            if (inputChannels[ch] != nullptr)
            {
                std::copy(inputChannels[ch], inputChannels[ch] + numSamples, filteredInputBuffers[ch].data());
                filteredPtrs[ch] = filteredInputBuffers[ch].data();
            }
        }

        antiAliasingFilter.processBlock(filteredPtrs, chLimit, numSamples);
        historyBuffer.write(filteredPtrs, chLimit, numSamples);
    }
    else
    {
        // Always push input to history buffer to maintain continuous historical context
        historyBuffer.write(inputChannels, channelsToProcess, numSamples);
    }

    // Bit-perfect bypass optimization with exact host PDC latency alignment
    if (std::abs(currentPitchRatio - 1.0) < 0.0005)
    {
        const size_t targetLatency = static_cast<size_t>(getLatencyInSamples());
        for (size_t ch = 0; ch < channelsToProcess; ++ch)
        {
            if (outputChannels[ch] != nullptr)
            {
                historyBuffer.extractSegment(ch, targetLatency, outputChannels[ch], numSamples);
            }
        }
        return;
    }

    const size_t synthCap = synthesisBuffer[0].size();
    const size_t synthMask = synthCap - 1;

    // Resampling ratio is the playback speed:
    // alpha > 1.0 (Pitch Up) => resampleRatio > 1.0 (steps through synthesis audio faster)
    // alpha < 1.0 (Pitch Down) => resampleRatio < 1.0 (steps through synthesis audio slower)
    const double resampleRatio = currentPitchRatio;
    const size_t neededSynthSamples =
        static_cast<size_t>(std::ceil(static_cast<double>(numSamples) * resampleRatio)) + 16;

    // Calculate how many samples are currently available in the synthesis buffer
    auto getAvailableSynth = [&]() noexcept -> size_t {
        return (synthesisWritePos >= synthesisReadPos) ? (synthesisWritePos - synthesisReadPos)
                                                       : (synthCap - synthesisReadPos + synthesisWritePos);
    };

    // Synthesize grains until enough audio is available for resampling
    while (getAvailableSynth() < neededSynthSamples)
    {
        processWsolaFrame();
    }

    // Use preallocated scratch buffer to extract contiguous synthesis samples for resampling
    if (contiguousSynthBuffer.size() < neededSynthSamples)
        contiguousSynthBuffer.resize(neededSynthSamples * 2, 0.0f);

    float *contiguousSynth = contiguousSynthBuffer.data();
    size_t actualConsumed = 0;

    // Smooth perceptual equal-loudness compensation across pitch shifts:
    // Compensates for anti-aliasing HF attenuation when pitching up and sub-bass buildup when pitching down
    float loudnessCompensation = 1.0f;
    if (currentPitchRatio > 1.0)
    {
        loudnessCompensation = static_cast<float>(1.0 + 0.12 * (currentPitchRatio - 1.0));
    }
    else if (currentPitchRatio < 1.0)
    {
        loudnessCompensation = static_cast<float>(1.0 / (1.0 + 0.08 * (1.0 - currentPitchRatio)));
    }

    for (size_t ch = 0; ch < channelsToProcess; ++ch)
    {
        if (outputChannels[ch] == nullptr)
            continue;

        // Unroll circular synthesis buffer into contiguous scratch buffer
        const float *synth = synthesisBuffer[ch].data();
        for (size_t i = 0; i < neededSynthSamples; ++i)
        {
            contiguousSynth[i] = synth[(synthesisReadPos + i) & synthMask];
        }

        // Resample from contiguous synthesis audio into output buffer
        const size_t consumed = resampler.processChannel(
            contiguousSynth, neededSynthSamples, outputChannels[ch], numSamples, resampleRatio, ch);

        if (std::abs(loudnessCompensation - 1.0f) > 1e-4f)
        {
            float *out = outputChannels[ch];
            for (size_t i = 0; i < numSamples; ++i)
                out[i] *= loudnessCompensation;
        }

        if (ch == 0)
            actualConsumed = consumed;

        // Clear consumed portion in circular synthesis buffer
        for (size_t i = 0; i < actualConsumed; ++i)
        {
            synthesisBuffer[ch][(synthesisReadPos + i) & synthMask] = 0.0f;
        }
    }

    synthesisReadPos = (synthesisReadPos + actualConsumed) & synthMask;
}

} // namespace pps

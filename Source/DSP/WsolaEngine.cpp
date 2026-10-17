#include "WsolaEngine.h"
#include <cmath>
#include <cstring>

namespace pps
{

void WsolaEngine::prepare (double sampleRate, size_t maxBlockSize, size_t numChannels)
{
    currentSampleRate = (sampleRate > 1000.0) ? sampleRate : 48000.0;
    allocatedChannels = (numChannels > 0) ? numChannels : 2;
    currentMaxBlock   = (maxBlockSize > 0) ? maxBlockSize : 512;

    updateWindowAndHopSizes();

    // Pre-allocate history buffer (16384 samples provides ample lookback for 44.1 - 96 kHz)
    historyBuffer.prepare (allocatedChannels, 16384);

    // Pre-allocate correlation finder
    correlationFinder.prepare (windowSize, static_cast<size_t> (maxSearchOffset));

    // Pre-calculate Hann window
    Windowing::generate (WindowType::Hann, windowSize, hannWindow);

    // Pre-allocate resampler
    resampler.prepare (allocatedChannels, currentMaxBlock * 4);

    // Pre-allocate synthesis ring buffers (capacity 16384)
    synthesisBuffer.assign (allocatedChannels, std::vector<float> (16384, 0.0f));

    // Pre-allocate scratch vectors for audio processing to guarantee zero heap allocation in process()
    channelRefBuffers.assign (allocatedChannels, std::vector<float> (windowSize, 0.0f));
    channelSearchBuffers.assign (allocatedChannels, std::vector<float> (windowSize + 2 * static_cast<size_t> (maxSearchOffset), 0.0f));
    grainBuffers.assign (allocatedChannels, std::vector<float> (windowSize, 0.0f));
    resampledOutputBuffers.assign (allocatedChannels, std::vector<float> (currentMaxBlock * 4, 0.0f));
    contiguousSynthBuffer.assign (currentMaxBlock * 8 + windowSize * 4, 0.0f);

    reset();
}

void WsolaEngine::updateWindowAndHopSizes()
{
    // Window length: ~16-20 ms depending on sample rate
    // 512 samples at 48 kHz = 10.67 ms (low latency for guitar playing)
    // 1024 samples at 96 kHz = 10.67 ms
    if (currentSampleRate >= 88200.0)
    {
        windowSize = 1024;
        hopSize = 512;
        maxSearchOffset = 256;
    }
    else
    {
        windowSize = 512;
        hopSize = 256;
        maxSearchOffset = 128;
    }
}

void WsolaEngine::reset() noexcept
{
    historyBuffer.reset();
    resampler.reset();

    for (auto& buf : synthesisBuffer)
        std::fill (buf.begin(), buf.end(), 0.0f);

    synthesisReadPos = 0;
    synthesisWritePos = 0;
    lastBestOffset = 0;
    nominalAnalysisPos = 0.0;
    isFirstFrame = true;
    currentPitchRatio = targetPitchRatio;
}

void WsolaEngine::setPitchShift (float semitones, float cents) noexcept
{
    const double totalSemitones = static_cast<double> (semitones) + (static_cast<double> (cents) / 100.0);
    const double ratio = std::pow (2.0, totalSemitones / 12.0);
    setPitchRatio (ratio);
}

void WsolaEngine::setPitchRatio (double ratio) noexcept
{
    targetPitchRatio = std::clamp (ratio, 0.4, 2.5);
}

int WsolaEngine::getLatencyInSamples() const noexcept
{
    // Latency is composed of the history lookback plus one synthesis hop
    return static_cast<int> (windowSize + static_cast<size_t> (maxSearchOffset) + hopSize);
}

void WsolaEngine::processWsolaFrame() noexcept
{
    const size_t synthCap = synthesisBuffer[0].size();
    const size_t synthMask = synthCap - 1;

    // Nominal analysis step: Ha = Hs / pitchRatio
    const auto nominalHopA = static_cast<int64_t> (std::round (static_cast<double> (hopSize) / currentPitchRatio));

    // Lookback distance in history buffer:
    // Base latency anchor + nominal position
    const size_t baseLookback = windowSize + static_cast<size_t> (maxSearchOffset) + hopSize;

    if (isFirstFrame)
    {
        // On the first frame, reference is directly from the base lookback
        for (size_t ch = 0; ch < allocatedChannels; ++ch)
        {
            historyBuffer.extractSegment (ch, baseLookback, channelRefBuffers[ch].data(), windowSize);
        }
        lastBestOffset = 0;
        isFirstFrame = false;
    }

    // Extract search window around nominal analysis point: length = windowSize + 2 * maxSearchOffset
    const auto searchLookback = static_cast<int64_t> (baseLookback) - nominalHopA + static_cast<int64_t> (lastBestOffset);
    const size_t clampedSearchLookback = (searchLookback > 0) ? static_cast<size_t> (searchLookback) : 0;
    const size_t searchLen = windowSize + 2 * static_cast<size_t> (maxSearchOffset);

    for (size_t ch = 0; ch < allocatedChannels; ++ch)
    {
        historyBuffer.extractSegment (ch, clampedSearchLookback, channelSearchBuffers[ch].data(), searchLen);
    }

    // Search for optimal phase-coherent offset across all channels
    const float* refPtrs[8] { nullptr };
    const float* searchPtrs[8] { nullptr };
    const size_t chCount = std::min (allocatedChannels, size_t (8));

    for (size_t ch = 0; ch < chCount; ++ch)
    {
        refPtrs[ch] = channelRefBuffers[ch].data();
        searchPtrs[ch] = channelSearchBuffers[ch].data();
    }

    const auto correlation = correlationFinder.findBestStereoOffset (
        refPtrs, searchPtrs, chCount, windowSize, maxSearchOffset);

    lastBestOffset = correlation.bestOffset;

    // Extract best-matching grain for each channel and overlap-add into synthesis buffer
    for (size_t ch = 0; ch < allocatedChannels; ++ch)
    {
        const size_t bestGrainOffset = static_cast<size_t> (maxSearchOffset + correlation.bestOffset);
        const float* searchData = channelSearchBuffers[ch].data() + bestGrainOffset;

        // Apply Hann window
        Windowing::apply (searchData, grainBuffers[ch].data(), hannWindow.data(), windowSize);

        // Overlap-add into circular synthesis buffer
        float* synth = synthesisBuffer[ch].data();
        for (size_t i = 0; i < windowSize; ++i)
        {
            const size_t writeIdx = (synthesisWritePos + i) & synthMask;
            synth[writeIdx] += grainBuffers[ch][i];
        }

        // Prepare reference for the next synthesis hop: the natural continuation of this grain
        // Advanced by hopSize
        const size_t nextRefOffset = bestGrainOffset + hopSize;
        if (nextRefOffset + windowSize <= searchLen)
        {
            std::copy (channelSearchBuffers[ch].data() + nextRefOffset,
                       channelSearchBuffers[ch].data() + nextRefOffset + windowSize,
                       channelRefBuffers[ch].data());
        }
    }

    synthesisWritePos = (synthesisWritePos + hopSize) & synthMask;
}

void WsolaEngine::process (const float* const* inputChannels,
                          float* const* outputChannels,
                          size_t numChannels,
                          size_t numSamples) noexcept
{
    if (inputChannels == nullptr || outputChannels == nullptr || numSamples == 0)
        return;

    const size_t channelsToProcess = std::min (numChannels, allocatedChannels);

    // Smooth currentPitchRatio towards targetPitchRatio
    if (std::abs (currentPitchRatio - targetPitchRatio) > 1e-4)
    {
        currentPitchRatio += (targetPitchRatio - currentPitchRatio) * smoothingCoeff;
    }
    else
    {
        currentPitchRatio = targetPitchRatio;
    }

    // Always push input to history buffer to maintain continuous historical context
    historyBuffer.write (inputChannels, channelsToProcess, numSamples);

    // Bit-perfect bypass optimization when pitch ratio is unity
    if (std::abs (currentPitchRatio - 1.0) < 0.0005)
    {
        for (size_t ch = 0; ch < channelsToProcess; ++ch)
        {
            if (inputChannels[ch] != nullptr && outputChannels[ch] != nullptr)
                std::copy (inputChannels[ch], inputChannels[ch] + numSamples, outputChannels[ch]);
        }
        return;
    }

    const size_t synthCap = synthesisBuffer[0].size();
    const size_t synthMask = synthCap - 1;

    // How many synthesis samples are needed to produce 'numSamples' of resampled output:
    // With Hermite resampler, ratio = 1.0 / currentPitchRatio
    const double resampleRatio = 1.0 / currentPitchRatio;
    const size_t neededSynthSamples = static_cast<size_t> (std::ceil (static_cast<double> (numSamples) * resampleRatio)) + windowSize;

    // Calculate how many samples are currently available in the synthesis buffer
    auto getAvailableSynth = [&]() noexcept -> size_t
    {
        return (synthesisWritePos >= synthesisReadPos)
                   ? (synthesisWritePos - synthesisReadPos)
                   : (synthCap - synthesisReadPos + synthesisWritePos);
    };

    // Synthesize grains until enough audio is available for resampling
    while (getAvailableSynth() < neededSynthSamples)
    {
        processWsolaFrame();
    }

    // Use preallocated scratch buffer to extract contiguous synthesis samples for resampling
    if (contiguousSynthBuffer.size() < neededSynthSamples)
        contiguousSynthBuffer.resize (neededSynthSamples * 2, 0.0f);

    float* contiguousSynth = contiguousSynthBuffer.data();

    for (size_t ch = 0; ch < channelsToProcess; ++ch)
    {
        if (outputChannels[ch] == nullptr)
            continue;

        // Unroll circular synthesis buffer into contiguous scratch buffer
        const float* synth = synthesisBuffer[ch].data();
        for (size_t i = 0; i < neededSynthSamples; ++i)
        {
            contiguousSynth[i] = synth[(synthesisReadPos + i) & synthMask];
        }

        // Resample from contiguous synthesis audio into output buffer
        resampler.processChannel (contiguousSynth,
                                  neededSynthSamples,
                                  outputChannels[ch],
                                  numSamples,
                                  resampleRatio,
                                  ch);

        // Clear consumed portion in circular synthesis buffer
        const size_t consumed = static_cast<size_t> (std::floor (static_cast<double> (numSamples) * resampleRatio));
        for (size_t i = 0; i < consumed; ++i)
        {
            synthesisBuffer[ch][(synthesisReadPos + i) & synthMask] = 0.0f;
        }
    }

    const size_t consumed = static_cast<size_t> (std::floor (static_cast<double> (numSamples) * resampleRatio));
    synthesisReadPos = (synthesisReadPos + consumed) & synthMask;
}

} // namespace pps

#include "HermiteResampler.h"
#include <cassert>

namespace pps
{

void HermiteResampler::prepare (size_t numChannels, size_t maxBlockSize)
{
    (void) maxBlockSize;
    allocatedChannels = numChannels;
    channelStates.assign (numChannels, ChannelState {});
    reset();
}

void HermiteResampler::reset() noexcept
{
    for (auto& state : channelStates)
    {
        state.history[0] = 0.0f;
        state.history[1] = 0.0f;
        state.history[2] = 0.0f;
        state.history[3] = 0.0f;
        state.fractionalPhase = 0.0;
        state.inputReadPos = 0;
    }
}

size_t HermiteResampler::processChannel (const float* input,
                                        size_t inputSamples,
                                        float* output,
                                        size_t outputSamples,
                                        double ratio,
                                        size_t channelIndex) noexcept
{
    if (channelIndex >= channelStates.size() || input == nullptr || output == nullptr)
        return 0;

    if (inputSamples == 0 || outputSamples == 0 || ratio <= 0.0)
        return 0;

    auto& state = channelStates[channelIndex];
    size_t outIndex = 0;
    double phase = state.fractionalPhase;

    auto getSample = [&] (int64_t idx) noexcept -> float
    {
        if (idx < 0)
        {
            const int64_t histIdx = 4 + idx;
            if (histIdx >= 0 && histIdx < 4)
                return state.history[histIdx];
            return input[0];
        }
        if (static_cast<size_t> (idx) < inputSamples)
            return input[idx];

        return input[inputSamples - 1];
    };

    while (outIndex < outputSamples)
    {
        const auto baseIdx = static_cast<int64_t> (std::floor (phase));
        if (static_cast<size_t> (baseIdx) >= inputSamples && baseIdx > 0)
            break;

        const float frac = static_cast<float> (phase - std::floor (phase));

        const float ym1 = getSample (baseIdx - 1);
        const float y0  = getSample (baseIdx);
        const float y1  = getSample (baseIdx + 1);
        const float y2  = getSample (baseIdx + 2);

        output[outIndex++] = interpolate4pt3o (ym1, y0, y1, y2, frac);
        phase += ratio;
    }

    // Update history for next block
    if (inputSamples >= 4)
    {
        state.history[0] = input[inputSamples - 4];
        state.history[1] = input[inputSamples - 3];
        state.history[2] = input[inputSamples - 2];
        state.history[3] = input[inputSamples - 1];
    }
    else
    {
        for (size_t i = 0; i < inputSamples; ++i)
        {
            state.history[0] = state.history[1];
            state.history[1] = state.history[2];
            state.history[2] = state.history[3];
            state.history[3] = input[i];
        }
    }

    // Keep residual phase for seamless block continuation
    state.fractionalPhase = phase - static_cast<double> (inputSamples);
    if (state.fractionalPhase < 0.0)
        state.fractionalPhase = 0.0;

    return outIndex;
}

void HermiteResampler::resampleBlock (const float* input,
                                     size_t inputSamples,
                                     float* output,
                                     size_t outputSamples) noexcept
{
    if (input == nullptr || output == nullptr || inputSamples == 0 || outputSamples == 0)
        return;

    if (inputSamples == outputSamples)
    {
        std::copy (input, input + inputSamples, output);
        return;
    }

    const double ratio = static_cast<double> (inputSamples - 1) / static_cast<double> (outputSamples - 1);

    auto getSampleClamped = [input, inputSamples] (int64_t idx) noexcept -> float
    {
        if (idx < 0) return input[0];
        if (static_cast<size_t> (idx) >= inputSamples) return input[inputSamples - 1];
        return input[idx];
    };

    for (size_t i = 0; i < outputSamples; ++i)
    {
        const double pos = static_cast<double> (i) * ratio;
        const auto baseIdx = static_cast<int64_t> (std::floor (pos));
        const float frac = static_cast<float> (pos - std::floor (pos));

        const float ym1 = getSampleClamped (baseIdx - 1);
        const float y0  = getSampleClamped (baseIdx);
        const float y1  = getSampleClamped (baseIdx + 1);
        const float y2  = getSampleClamped (baseIdx + 2);

        output[i] = interpolate4pt3o (ym1, y0, y1, y2, frac);
    }
}

} // namespace pps

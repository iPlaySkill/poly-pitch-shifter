#include "HermiteResampler.h"
#include <cassert>

namespace pps {

void HermiteResampler::prepare(size_t numChannels, size_t maxBlockSize)
{
    (void)maxBlockSize;
    allocatedChannels = numChannels;
    channelStates.assign(numChannels, ChannelState{});
    reset();
}

void HermiteResampler::reset() noexcept
{
    for (auto &state : channelStates)
    {
        state.history[0] = 0.0f;
        state.history[1] = 0.0f;
        state.history[2] = 0.0f;
        state.history[3] = 0.0f;
        state.fractionalPhase = 0.0;
        state.inputReadPos = 0;
        state.lastConsumedSamples = 0;
    }
}

size_t HermiteResampler::processChannel(const float *input, size_t inputSamples, float *output, size_t outputSamples,
                                        double ratio, size_t channelIndex) noexcept
{
    if (channelIndex >= channelStates.size() || input == nullptr || output == nullptr)
        return 0;

    if (inputSamples == 0 || outputSamples == 0 || ratio <= 0.0)
        return 0;

    auto &state = channelStates[channelIndex];
    size_t outIndex = 0;
    double phase = state.fractionalPhase;

    auto getSample = [&](int64_t idx) noexcept -> float {
        if (idx < 0)
        {
            const int64_t histIdx = 4 + idx;
            if (histIdx >= 0 && histIdx < 4)
                return state.history[histIdx];
            return input[0];
        }
        if (static_cast<size_t>(idx) < inputSamples)
            return input[idx];

        return input[inputSamples - 1];
    };

    while (outIndex < outputSamples)
    {
        const auto baseIdx = static_cast<int64_t>(std::floor(phase));
        if (static_cast<size_t>(baseIdx) >= inputSamples && baseIdx > 0)
            break;

        const float frac = static_cast<float>(phase - std::floor(phase));

        const float ym1 = getSample(baseIdx - 1);
        const float y0 = getSample(baseIdx);
        const float y1 = getSample(baseIdx + 1);
        const float y2 = getSample(baseIdx + 2);

        output[outIndex++] = interpolate4pt3o(ym1, y0, y1, y2, frac);
        phase += ratio;
    }

    // Number of integer input samples completely traversed
    const auto consumed = static_cast<size_t>(std::max(0.0, std::floor(phase)));

    // Update history for next block ending exactly at input[consumed - 1]
    if (consumed >= 4)
    {
        state.history[0] = input[consumed - 4];
        state.history[1] = input[consumed - 3];
        state.history[2] = input[consumed - 2];
        state.history[3] = input[consumed - 1];
    }
    else if (consumed > 0)
    {
        for (size_t i = 0; i < consumed; ++i)
        {
            state.history[0] = state.history[1];
            state.history[1] = state.history[2];
            state.history[2] = state.history[3];
            state.history[3] = input[i];
        }
    }

    // Keep residual sub-sample phase strictly in [0.0, 1.0) for seamless Catmull-Rom continuation
    state.fractionalPhase = phase - static_cast<double>(consumed);
    if (state.fractionalPhase < 0.0)
        state.fractionalPhase = 0.0;
    else if (state.fractionalPhase >= 1.0)
        state.fractionalPhase = 0.0;

    state.lastConsumedSamples = consumed;
    return consumed;
}

void HermiteResampler::resampleBlock(const float *input, size_t inputSamples, float *output,
                                     size_t outputSamples) noexcept
{
    if (input == nullptr || output == nullptr || inputSamples == 0 || outputSamples == 0)
        return;

    if (inputSamples == outputSamples)
    {
        std::copy(input, input + inputSamples, output);
        return;
    }

    const double ratio = static_cast<double>(inputSamples - 1) / static_cast<double>(outputSamples - 1);

    auto getSampleClamped = [input, inputSamples](int64_t idx) noexcept -> float {
        if (idx < 0)
            return input[0];
        if (static_cast<size_t>(idx) >= inputSamples)
            return input[inputSamples - 1];
        return input[idx];
    };

    for (size_t i = 0; i < outputSamples; ++i)
    {
        const double pos = static_cast<double>(i) * ratio;
        const auto baseIdx = static_cast<int64_t>(std::floor(pos));
        const float frac = static_cast<float>(pos - std::floor(pos));

        const float ym1 = getSampleClamped(baseIdx - 1);
        const float y0 = getSampleClamped(baseIdx);
        const float y1 = getSampleClamped(baseIdx + 1);
        const float y2 = getSampleClamped(baseIdx + 2);

        output[i] = interpolate4pt3o(ym1, y0, y1, y2, frac);
    }
}

} // namespace pps

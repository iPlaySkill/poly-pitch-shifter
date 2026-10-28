#include "CrossCorrelationFinder.h"
#include <cmath>

namespace pps {

void CrossCorrelationFinder::prepare(size_t maxWindowLength, size_t maxSearchOffset)
{
    (void)maxWindowLength;
    const size_t numOffsets = 2 * maxSearchOffset + 1;
    correlationValues.assign(numOffsets, 0.0f);
}

CorrelationResult CrossCorrelationFinder::findBestOffset(const float *reference, const float *searchBuffer,
                                                         size_t windowLength, int maxSearchOffset) noexcept
{
    CorrelationResult result;
    if (reference == nullptr || searchBuffer == nullptr || windowLength == 0 || maxSearchOffset <= 0)
        return result;

    const size_t numOffsets = static_cast<size_t>(2 * maxSearchOffset + 1);
    if (correlationValues.size() < numOffsets)
        correlationValues.resize(numOffsets, 0.0f);

    const float refEnergy = computeEnergy(reference, windowLength);
    if (refEnergy < 1e-12f)
        return result; // Silence, return zero offset

    float bestScore = -std::numeric_limits<float>::infinity();
    float bestNccf = -std::numeric_limits<float>::infinity();
    int bestOffset = 0;
    size_t bestIdx = static_cast<size_t>(maxSearchOffset);

    for (int delta = -maxSearchOffset; delta <= maxSearchOffset; ++delta)
    {
        const size_t idx = static_cast<size_t>(delta + maxSearchOffset);
        const float *candidate = searchBuffer + static_cast<size_t>(maxSearchOffset + delta);

        const float cross = computeInnerProduct(reference, candidate, windowLength);
        const float candEnergy = computeEnergy(candidate, windowLength);
        const float denominator = std::sqrt(refEnergy * candEnergy + 1e-12f);
        const float nccf = cross / denominator;

        correlationValues[idx] = nccf;

        // Polyphonic chord stabilization: mild lag penalty prevents phase flutter between competing chord harmonics
        const float lagPenalty = 0.005f * (static_cast<float>(std::abs(delta)) / static_cast<float>(maxSearchOffset));
        const float score = nccf - lagPenalty;

        if (score > bestScore)
        {
            bestScore = score;
            bestNccf = nccf;
            bestOffset = delta;
            bestIdx = idx;
        }
    }

    result.bestOffset = bestOffset;
    result.peakCorrelation = bestNccf;

    // Sub-sample parabolic interpolation around peak
    if (bestIdx > 0 && bestIdx + 1 < numOffsets)
    {
        const float ym1 = correlationValues[bestIdx - 1];
        const float y0 = correlationValues[bestIdx];
        const float yp1 = correlationValues[bestIdx + 1];
        const float deltaFrac = parabolicInterpolate(ym1, y0, yp1);
        result.fractionalOffset = static_cast<float>(bestOffset) + deltaFrac;
    }
    else
    {
        result.fractionalOffset = static_cast<float>(bestOffset);
    }

    return result;
}

CorrelationResult CrossCorrelationFinder::findBestStereoOffset(const float *const *channelReferences,
                                                               const float *const *channelSearchBuffers,
                                                               size_t numChannels, size_t windowLength,
                                                               int maxSearchOffset) noexcept
{
    CorrelationResult result;
    if (channelReferences == nullptr || channelSearchBuffers == nullptr || numChannels == 0 || windowLength == 0 ||
        maxSearchOffset <= 0)
        return result;

    if (numChannels == 1)
        return findBestOffset(channelReferences[0], channelSearchBuffers[0], windowLength, maxSearchOffset);

    const size_t numOffsets = static_cast<size_t>(2 * maxSearchOffset + 1);
    if (correlationValues.size() < numOffsets)
        correlationValues.resize(numOffsets, 0.0f);

    float totalRefEnergy = 0.0f;
    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        if (channelReferences[ch] != nullptr)
            totalRefEnergy += computeEnergy(channelReferences[ch], windowLength);
    }

    if (totalRefEnergy < 1e-12f)
        return result;

    float bestScore = -std::numeric_limits<float>::infinity();
    float bestNccf = -std::numeric_limits<float>::infinity();
    int bestOffset = 0;
    size_t bestIdx = static_cast<size_t>(maxSearchOffset);

    for (int delta = -maxSearchOffset; delta <= maxSearchOffset; ++delta)
    {
        const size_t idx = static_cast<size_t>(delta + maxSearchOffset);
        float totalCross = 0.0f;
        float totalCandEnergy = 0.0f;

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            if (channelReferences[ch] == nullptr || channelSearchBuffers[ch] == nullptr)
                continue;

            const float *candidate = channelSearchBuffers[ch] + static_cast<size_t>(maxSearchOffset + delta);
            totalCross += computeInnerProduct(channelReferences[ch], candidate, windowLength);
            totalCandEnergy += computeEnergy(candidate, windowLength);
        }

        const float denominator = std::sqrt(totalRefEnergy * totalCandEnergy + 1e-12f);
        const float nccf = totalCross / denominator;
        correlationValues[idx] = nccf;

        // Polyphonic stereo chord stabilization: mild lag penalty prevents phase flutter
        const float lagPenalty = 0.005f * (static_cast<float>(std::abs(delta)) / static_cast<float>(maxSearchOffset));
        const float score = nccf - lagPenalty;

        if (score > bestScore)
        {
            bestScore = score;
            bestNccf = nccf;
            bestOffset = delta;
            bestIdx = idx;
        }
    }

    result.bestOffset = bestOffset;
    result.peakCorrelation = bestNccf;

    if (bestIdx > 0 && bestIdx + 1 < numOffsets)
    {
        const float ym1 = correlationValues[bestIdx - 1];
        const float y0 = correlationValues[bestIdx];
        const float yp1 = correlationValues[bestIdx + 1];
        const float deltaFrac = parabolicInterpolate(ym1, y0, yp1);
        result.fractionalOffset = static_cast<float>(bestOffset) + deltaFrac;
    }
    else
    {
        result.fractionalOffset = static_cast<float>(bestOffset);
    }

    return result;
}

} // namespace pps

#include "TestHelpers.h"
#include <shared_modules/shared_modules.h>

TEST_CASE ("Test getMaximumDelayInSamples returns correct value", "[VariableDelayLine]")
{
    size_t expected = static_cast<size_t> (GENERATE (10, 25, 102));

    VariableDelayLine delayLine (expected);

    CHECK (delayLine.getMaximumDelayInSamples() == expected);
}

TEST_CASE ("Test process with different sample rates", "[VariableDelayLine]")
{
    VariableDelayLine delayLine (10, 3);

    float expected = 2.0;
    std::vector<float> delayInSamples = { 5.0, 3.0, 1.0 };

    const auto runTest = [&] (const float fs)
    {
        juce::dsp::ProcessSpec spec{ fs, static_cast<juce::uint32> (delayLine.getMaximumDelayInSamples()), 2 };
        delayLine.prepare (spec);

        for (size_t i = 0; i < delayInSamples.size(); i++)
            delayLine.setDelayInSamples (delayInSamples[i], i, true);

        auto input = TestHelpers::generateInputBuffer (spec.numChannels, spec.maximumBlockSize, expected);

        TestHelpers::runProcess (delayLine, input);

        for (size_t ch = 0; ch < spec.numChannels; ch++)
        {
            CHECK (delayLine.getTapOutBuffer (ch, 0)[static_cast<size_t> (delayInSamples[0])] == expected);
            CHECK (delayLine.getTapOutBuffer (ch, 1)[static_cast<size_t> (delayInSamples[1])] == expected);
            CHECK (delayLine.getTapOutBuffer (ch, 2)[static_cast<size_t> (delayInSamples[2])] == expected);
        }
    };

    std::vector<float> sampleRates{ 44100, 48000, 96000, 192000 };

    for (auto& s : sampleRates)
        runTest (s);
}

TEST_CASE ("Test process with fractional delays", "[VariableDelayLine]")
{
    float fs = 48000;
    float sampleValue = 1.0;
    std::vector<float> delayInSamples = { 5.1, 3.5, 1.2 };

    VariableDelayLine delayLine (10, 3);

    juce::dsp::ProcessSpec spec{ fs, static_cast<juce::uint32> (delayLine.getMaximumDelayInSamples()), 2 };
    delayLine.prepare (spec);

    for (size_t i = 0; i < delayInSamples.size(); i++)
        delayLine.setDelayInSamples (delayInSamples[i], i, true);

    auto input = TestHelpers::generateInputBuffer (spec.numChannels, spec.maximumBlockSize, sampleValue);
    TestHelpers::runProcess (delayLine, input);

    for (size_t ch = 0; ch < spec.numChannels; ch++)
    {
        float fractional = delayInSamples[0] - static_cast<size_t> (delayInSamples[0]);
        CHECK (delayLine.getTapOutBuffer (ch, 0)[static_cast<size_t> (delayInSamples[0])] == sampleValue - fractional);

        fractional = delayInSamples[1] - static_cast<size_t> (delayInSamples[1]);
        CHECK (delayLine.getTapOutBuffer (ch, 1)[static_cast<size_t> (delayInSamples[1])] == sampleValue - fractional);

        fractional = delayInSamples[2] - static_cast<size_t> (delayInSamples[2]);
        CHECK (delayLine.getTapOutBuffer (ch, 2)[static_cast<size_t> (delayInSamples[2])] == sampleValue - fractional);
    }
}

TEST_CASE ("Test process calling setDelayInSamples", "[VariableDelayLine]")
{
    float fs = 48000;
    float sampleValue = 0.1234;

    VariableDelayLine delayLine (10, 1);

    juce::dsp::ProcessSpec spec{ fs, static_cast<juce::uint32> (delayLine.getMaximumDelayInSamples()), 2 };
    delayLine.prepare (spec);

    auto input = TestHelpers::generateInputBuffer (spec.numChannels, spec.maximumBlockSize, sampleValue);

    const auto runTest = [&] (const std::pair<float, size_t>& bufferIndexPerDelay)
    {
        delayLine.setDelayInSamples (bufferIndexPerDelay.first, 0, true);
        TestHelpers::runProcess (delayLine, input);

        for (size_t ch = 0; ch < spec.numChannels; ch++)
            CHECK (delayLine.getTapOutBuffer (ch, 0)[bufferIndexPerDelay.second] == sampleValue);
    };

    std::vector<std::pair<float, int>> expectedBufferIndexPerDelay = { { 1.0, 1 },
                                                                       { 2.0, 3 },
                                                                       { 3.0, 6 } };

    for (auto& expected : expectedBufferIndexPerDelay)
        runTest (expected);
}

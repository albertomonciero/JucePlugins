#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <shared_modules/shared_modules.h>


namespace VariableDelayLineTest
{
    juce::AudioBuffer<float> singleTap(const juce::dsp::ProcessSpec& spec, VariableDelayLine& delayLine, float sampleValue, float delayInSamples)
    {
        delayLine.prepare(spec);
        delayLine.setDelayInSamples(delayInSamples);

        juce::AudioBuffer<float> input(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
        input.clear();

        for (size_t ch = 0; ch < spec.numChannels; ch++)
            input.setSample(ch, 0, sampleValue);

        juce::dsp::AudioBlock<float> block (input);
        juce::dsp::ProcessContextReplacing<float> context(block);

        delayLine.process(block);

        return input;
    }

    void multiTap(const juce::dsp::ProcessSpec& spec, VariableDelayLine& delayLine, const std::vector<float>& sampleValues, const std::vector<float>& delayInSamples)
    {
        delayLine.prepare(spec);

        for (size_t i = 0; i < delayInSamples.size(); i++)
            delayLine.setDelayInSamples(i, delayInSamples[i]);

        juce::AudioBuffer<float> input(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
        input.clear();

        for (size_t ch = 0; ch < spec.numChannels; ch++)
            for (size_t i = 0; i < sampleValues.size(); i++)
                input.setSample(ch, i, sampleValues[i]);

        juce::dsp::AudioBlock<float> block (input);
        juce::dsp::ProcessContextReplacing<float> context(block);

        delayLine.process(block);
    }
}

TEST_CASE ("Test that single tap delay line return correct values setting different delays in sample")
{
    float fs = GENERATE(44100, 48000, 96000);
    float delayInSamples = GENERATE(1.0, 5.0, 9.0);
    float sampleValue = GENERATE(0.3, 1.0, -0.7);

    VariableDelayLine delayLine(10, 1);

    juce::dsp::ProcessSpec spec {fs, static_cast<juce::uint32>(delayLine.getMaximumDelayInSamples()), 2};

    juce::AudioBuffer<float> expected = VariableDelayLineTest::singleTap(spec, delayLine, sampleValue, delayInSamples);

    for (size_t ch = 0; ch < spec.numChannels; ch++)
    {
        CHECK(expected.getSample(ch, delayInSamples) == sampleValue);
        CHECK(delayLine.getTapOutBuffer(ch, 0)[static_cast<size_t>(delayInSamples)] == sampleValue);
    }     
}

TEST_CASE ("Test tha single tap delay line return correct values setting different fractional delays in sample")
{
    float fs = GENERATE(44100, 48000, 96000);
    float delayInSamples = GENERATE(1.203, 3.612, 5.43);
    float sampleValue = 1.0;

    VariableDelayLine delayLine(10, 1);

    juce::dsp::ProcessSpec spec {fs, static_cast<juce::uint32>(delayLine.getMaximumDelayInSamples()), 2};
    
    juce::AudioBuffer<float> expected = VariableDelayLineTest::singleTap(spec, delayLine, sampleValue, delayInSamples);

    for (size_t ch = 0; ch < spec.numChannels; ch++)
    {
        float fractional = delayInSamples - static_cast<size_t>(delayInSamples);
        CHECK(expected.getSample(ch, delayInSamples) == sampleValue - fractional);
    }   
}

TEST_CASE ("Test that multitap delay line returns correct values")
{
    float fs = GENERATE(44100, 48000, 96000);

    std::vector<float> sampleValues = {0.2, -0.7, 1.5};
    std::vector<float> delayInSamples = {5.0, 3.0, 1.0};

    VariableDelayLine delayLine(10, delayInSamples.size());

    juce::dsp::ProcessSpec spec {fs, static_cast<juce::uint32>(delayLine.getMaximumDelayInSamples()), 2};

    VariableDelayLineTest::multiTap(spec, delayLine, sampleValues, delayInSamples);

    for (size_t ch = 0; ch < spec.numChannels; ch++)
        for (size_t i = 0; i < delayInSamples.size(); i++)
            for (size_t n = 0; n < sampleValues.size(); n++)
            {
                size_t index = static_cast<size_t>(delayInSamples[i] + n);
                CHECK(delayLine.getTapOutBuffer(ch, i)[index] == sampleValues[n]);
            }         
}

TEST_CASE ("Test that multitap fractional delay line return correct values")
{
    float fs = GENERATE(44100, 48000, 96000);

    std::vector<float> sampleValues = {1.0, 1.0, 1.0};
    std::vector<float> delayInSamples = {5.1, 3.5, 1.2};

    VariableDelayLine delayLine(10, delayInSamples.size());

    juce::dsp::ProcessSpec spec {fs, static_cast<juce::uint32>(delayLine.getMaximumDelayInSamples()), 2};
    
    VariableDelayLineTest::multiTap(spec, delayLine, sampleValues, delayInSamples);

    for (size_t ch = 0; ch < spec.numChannels; ch++)
        for (size_t i = 0; i < delayInSamples.size(); i++)
            for (size_t n = 0; n < sampleValues.size(); n++)
            {
                float fractionalDelay = delayInSamples[i] - static_cast<size_t>(delayInSamples[i]);
                size_t index = std::ceil(delayInSamples[i]) + sampleValues.size() - 1;

                CHECK(delayLine.getTapOutBuffer(ch, i)[index] == fractionalDelay);
            }         
}

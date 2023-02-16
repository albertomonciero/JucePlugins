#include "TestHelpers.h"

namespace TestHelpers
{
    TEST_CASE ("Test that impulseResponseGenerator returns correct audio buffer")
    {
        float expected = GENERATE (1.0, -0.7, 2.5);

        SimpleMultiplier multiplier(expected);

        juce::AudioBuffer<float> actual = impulseResponseGenerator(multiplier, 2, 1); 

        for (int ch = 0; ch < actual.getNumChannels(); ch++)
            for (int i = 0; i < actual.getNumSamples(); i++)
                CHECK(actual.getSample(ch, i) == expected);
    }

    TEST_CASE ("Test that getFrequencyResponse returns correct spectrum")
    {
        float expected = GENERATE (1.0, 2.0, 3.0);

        juce::AudioBuffer<float> actual (2, 4);
        actual.clear();
        actual.setSample(0, 0, expected);
        actual.setSample(1, 0, expected);

        size_t fftSize = static_cast<size_t> (GENERATE(4, 8, 16));
        
        ComplexBuffer complexBuffer = getFrequencyResponse(actual, fftSize);

        for (size_t ch = 0; ch < static_cast<size_t> (actual.getNumChannels()); ch++)
            for (size_t i = 0; i < fftSize; i++)
            {
                CHECK(complexBuffer[ch][i].real() == expected);
                CHECK(complexBuffer[ch][i].imag() == 0.0);
            }
    }

    TEST_CASE ("Test that getMagnitudeResponse returns correct values")
    {
        ComplexBuffer complexBuffer  { { { 1.0, 1.0 }, { 2.0, 2.0 } },
                                       { { 3.0, 3.0 }, { 4.0, 4.0 } } };
 
        juce::AudioBuffer<float> expected(complexBuffer.size(), complexBuffer[0].size() / 2);
        expected.setSample(0, 0, 1.4142135623730951);
        expected.setSample(1, 0, 4.242640687119285);

        juce::AudioBuffer<float> actual = getMagnitudeResponse(complexBuffer); 

        for (int ch = 0; ch < actual.getNumChannels(); ch++)
            for (int i = 0; i < actual.getNumSamples(); i++)
                CHECK(actual.getSample(ch, i) == expected.getSample(ch, i));
    }

    TEST_CASE ("Test that getPhaseResponse returns correct values")
    {
        ComplexBuffer complexBuffer  { { { 1.0, 1.0 }, { 2.0, 2.0 } },
                                       { { 2.0, 3.0 }, { 4.0, 4.0 } } };
 
        juce::AudioBuffer<float> expected(complexBuffer.size(), complexBuffer[0].size() / 2);
        expected.setSample(0, 0, 0.7853981633974483);
        expected.setSample(1, 0, 0.982793723247329);

        juce::AudioBuffer<float> actual = getPhaseResponse(complexBuffer); 

        for (int ch = 0; ch < actual.getNumChannels(); ch++)
            for (int i = 0; i < actual.getNumSamples(); i++)
                CHECK(actual.getSample(ch, i) == expected.getSample(ch, i));
    }

    TEST_CASE ("Test that unwrapPhase returns correct values")
    {
        std::array<float, 8> wrappedPhase = {0, -M_PI / 4, -M_PI / 2, -M_PI,  M_PI,      M_PI / 2,      M_PI / 4,         0 };
        std::array<float, 8> expected =     {0, -M_PI / 4, -M_PI / 2, -M_PI, -M_PI, -3 * M_PI / 2, -7 * M_PI / 4, -2 * M_PI };
        std::array<float, 8> actual;

        unwrapPhase(&wrappedPhase[0], &actual[0], wrappedPhase.size());

        for (size_t i = 0; i < actual.size(); i++)
            CHECK_THAT(actual[i], Catch::Matchers::WithinRel(expected[i], 1e-7f));
    }
}

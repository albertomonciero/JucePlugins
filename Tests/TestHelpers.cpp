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
        
        ComplexBuffer spectrum = getFrequencyResponse(actual, fftSize);

        for (size_t ch = 0; ch < static_cast<size_t> (actual.getNumChannels()); ch++)
            for (size_t i = 0; i < fftSize; i++)
            {
                CHECK(spectrum[ch][i].real() == expected);
                CHECK(spectrum[ch][i].imag() == 0.0);
            }
    }
}

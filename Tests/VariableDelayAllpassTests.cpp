#include "TestHelpers.h"
#include <shared_modules/shared_modules.h>

TEST_CASE ("Test allpass filter with different cutoff frequencies", "[VariableDelayAllpass]")
{
    float fs = 48000;

    VariableDelayAllpass allpass (1);

    juce::dsp::ProcessSpec spec{ fs, 256, 2 };
    allpass.prepare (spec);

    allpass.setDelayInSamples (1.0, 0, true);

    size_t fftSize = spec.maximumBlockSize;

    const auto runTest = [&] (const size_t frequencyIndex)
    {
        float fc = spec.sampleRate / static_cast<float> (fftSize) * (frequencyIndex);

        // Equation from Zölzer DAFX p.39
        float c = (tan (M_PI * fc / fs) - 1) / (tan (M_PI * fc / fs) + 1);
        allpass.setGain (c, true);

        juce::AudioBuffer<float> impulseResponse = TestHelpers::impulseResponseGenerator (allpass, spec.numChannels, spec.maximumBlockSize);
        TestHelpers::ComplexBuffer frequencyResponse = TestHelpers::getFrequencyResponse (impulseResponse, fftSize);
        juce::AudioBuffer<float> magnitudeResponse = TestHelpers::getMagnitudeResponse (frequencyResponse);
        juce::AudioBuffer<float> phaseResponse = TestHelpers::getPhaseResponse (frequencyResponse);

        for (size_t ch = 0; ch < spec.numChannels; ch++)
        {
            // magnitude should be at 0 dB for all frequency values
            for (auto i = 0; i < magnitudeResponse.getNumSamples(); i++)
                CHECK_THAT (juce::Decibels::gainToDecibels<float> (magnitudeResponse.getSample (ch, i)), Catch::Matchers::WithinAbs (0.0, 1e-5));

            // phase response should be -90 deg at fc
            CHECK_THAT (phaseResponse.getSample (ch, frequencyIndex), Catch::Matchers::WithinAbs (-M_PI / 2, 1e-6));
        }
    };

    std::vector<size_t> frequencyIndices{ 10, 20, 22, 35 };

    for (auto& f : frequencyIndices)
        runTest (f);
}

TEST_CASE ("Test allpass filter with different delays", "[VariableDelayAllpass]")
{
    float fs = 48000;

    VariableDelayAllpass allpass (50);

    // with longer sample delays than 1, more samples are required to increase the resolution
    juce::dsp::ProcessSpec spec{ fs, 1024, 2 };
    allpass.prepare (spec);

    size_t fftSize = spec.maximumBlockSize;
    size_t frequencyIndex = 80;

    float fc = spec.sampleRate / static_cast<float> (fftSize) * (frequencyIndex);

    // Equation from Zölzer DAFX p.39
    float c = (tan (M_PI * fc / fs) - 1) / (tan (M_PI * fc / fs) + 1);
    allpass.setGain (c, true);

    const auto runTest = [&] (std::pair<float, float> expectedPhasePerDelay)
    {
        allpass.setDelayInSamples (expectedPhasePerDelay.first, 0, true);

        juce::AudioBuffer<float> impulseResponse = TestHelpers::impulseResponseGenerator (allpass, spec.numChannels, spec.maximumBlockSize);
        TestHelpers::ComplexBuffer frequencyResponse = TestHelpers::getFrequencyResponse (impulseResponse, fftSize);
        juce::AudioBuffer<float> magnitudeResponse = TestHelpers::getMagnitudeResponse (frequencyResponse);
        juce::AudioBuffer<float> phaseResponse = TestHelpers::getPhaseResponse (frequencyResponse);

        for (size_t ch = 0; ch < spec.numChannels; ch++)
        {
            // magnitude should be at 0 dB for all frequency values
            for (auto i = 0; i < magnitudeResponse.getNumSamples(); i++)
                CHECK_THAT (juce::Decibels::gainToDecibels<float> (magnitudeResponse.getSample (ch, i)), Catch::Matchers::WithinAbs (0.0, 1e-3));

            CHECK_THAT (phaseResponse.getSample (ch, frequencyIndex), Catch::Matchers::WithinAbs (expectedPhasePerDelay.second, 1e-4));
        }
    };

    // pair delayInSamples with expected phase response values from numerical simulations at fc
    std::vector<std::pair<float, float>> expectedPhasePerDelay{ { 2.0, -2.265119315942736 },
                                                                { 15.0, -8.633073980079889 },
                                                                { 50.0, -23.37163478234349 } };

    for (auto& p : expectedPhasePerDelay)
        runTest (p);
}

TEST_CASE ("Test allpass filter returns correct multitap outputs", "[VariableDelayAllpass]")
{
    float fs = 48000;

    std::array<size_t, 3> taps = { 3, 2, 1 };

    size_t delayLineLength = 4;
    VariableDelayAllpass allpass (delayLineLength, taps.size());

    juce::dsp::ProcessSpec spec{ fs, static_cast<juce::uint32> (delayLineLength), 2 };
    allpass.prepare (spec);

    for (size_t i = 0; i < taps.size(); i++)
        allpass.setDelayInSamples (taps[i], i, true);

    juce::AudioBuffer<float> input (static_cast<int> (spec.numChannels), static_cast<int> (spec.maximumBlockSize));

    float sampleValue = 1.0;

    for (size_t ch = 0; ch < spec.numChannels; ch++)
        input.setSample (ch, 0, sampleValue);

    juce::dsp::AudioBlock<float> block (input);
    juce::dsp::ProcessContextReplacing<float> context (block);

    allpass.process (context);

    for (size_t ch = 0; ch < spec.numChannels; ch++)
        for (size_t n = 0; n < taps.size(); n++)
            CHECK (allpass.getTapOutBuffer (ch, n)[taps[n]] == sampleValue);
}

TEST_CASE ("Test process with different sample rates", "[VariableDelayAllpass]")
{
    VariableDelayAllpass allpass (1);

    size_t frequencyIndex = 10;

    const auto runTest = [&] (const float& fs)
    {
        juce::dsp::ProcessSpec spec{ fs, 256, 2 };
        allpass.prepare (spec);
        allpass.setDelayInSamples (1.0, 0, true);
        size_t fftSize = spec.maximumBlockSize;

        float fc = fs / static_cast<float> (fftSize) * frequencyIndex;

        // Equation from Zölzer DAFX p.39
        float c = (tan (M_PI * fc / fs) - 1) / (tan (M_PI * fc / fs) + 1);
        allpass.setGain (c, true);

        juce::AudioBuffer<float> impulseResponse = TestHelpers::impulseResponseGenerator (allpass, spec.numChannels, spec.maximumBlockSize);
        TestHelpers::ComplexBuffer frequencyResponse = TestHelpers::getFrequencyResponse (impulseResponse, fftSize);
        juce::AudioBuffer<float> magnitudeResponse = TestHelpers::getMagnitudeResponse (frequencyResponse);
        juce::AudioBuffer<float> phaseResponse = TestHelpers::getPhaseResponse (frequencyResponse);

        for (size_t ch = 0; ch < spec.numChannels; ch++)
        {
            // magnitude should be at 0 dB for all frequency values
            for (auto i = 0; i < magnitudeResponse.getNumSamples(); i++)
                CHECK_THAT (juce::Decibels::gainToDecibels<float> (magnitudeResponse.getSample (ch, i)), Catch::Matchers::WithinAbs (0.0, 1e-5));

            // phase response should be -90 deg at fc
            CHECK_THAT (phaseResponse.getSample (ch, frequencyIndex), Catch::Matchers::WithinAbs (-M_PI / 2, 1e-6));
        }
    };

    std::vector<float> sampleRates{ 44100, 48000, 96000, 192000 };

    for (auto& s : sampleRates)
        runTest (s);
}

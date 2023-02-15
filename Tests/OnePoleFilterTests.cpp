#include "TestHelpers.h"
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <shared_modules/shared_modules.h>

TEST_CASE ("Test that one pole lowpass filter has correct frequency response")
{
    float fs = GENERATE(44100, 48000, 96000);

    juce::dsp::ProcessSpec spec {fs, 256, 2};

    size_t fftSize = spec.maximumBlockSize;

    // generate frequency at exact fft bin positions
    size_t index = static_cast<size_t> (GENERATE(5, 13, 17));
    float fc = index * spec.sampleRate / static_cast<float> (fftSize);

    OnePoleFilter::Lowpass lowpassFilter;
    lowpassFilter.prepare(spec);
    lowpassFilter.setCutoffFrequency(fc);

    juce::AudioBuffer<float> impulseResponse = TestHelpers::impulseResponseGenerator(lowpassFilter, spec.numChannels, spec.maximumBlockSize);
    TestHelpers::ComplexBuffer frequencyResponse = TestHelpers::getFrequencyResponse(impulseResponse, fftSize);
    juce::AudioBuffer<float> magnitudeResponse = TestHelpers::getMagnitudeResponse(frequencyResponse);

    // check result per channel for DC, fc, and last possible index
    for (size_t ch = 0; ch < spec.numChannels; ch++)
    {
        REQUIRE_THAT(juce::Decibels::gainToDecibels<float>(magnitudeResponse.getSample(ch, 0)), Catch::Matchers::WithinAbs(0.0, 1e-8));
        REQUIRE_THAT(juce::Decibels::gainToDecibels<float>(magnitudeResponse.getSample(ch, index)), Catch::Matchers::WithinAbs(-3.0, 1e-1));
        REQUIRE(juce::Decibels::gainToDecibels<float>(magnitudeResponse.getSample(ch, fftSize / 2 - 1)) < -13.0f);
    }
}

TEST_CASE ("Test that one pole highpass filter has correct frequency response")
{
    float fs = GENERATE(44100, 48000, 96000);

    juce::dsp::ProcessSpec spec {fs, 256, 2};

    size_t fftSize = spec.maximumBlockSize;

    // generate frequency at exact fft bin positions
    size_t index = static_cast<size_t>(GENERATE(8, 14, 20));
    float fc = index * spec.sampleRate / static_cast<float> (fftSize);

    OnePoleFilter::Highpass highpassFilter;
    highpassFilter.prepare(spec);
    highpassFilter.setCutoffFrequency(fc);

    juce::AudioBuffer<float> impulseResponse = TestHelpers::impulseResponseGenerator(highpassFilter, spec.numChannels, spec.maximumBlockSize);
    TestHelpers::ComplexBuffer frequencyResponse = TestHelpers::getFrequencyResponse(impulseResponse, fftSize);
    juce::AudioBuffer<float> magnitudeResponse = TestHelpers::getMagnitudeResponse(frequencyResponse);

    // check result per channel for DC, fc, and last possible index
    for (size_t ch = 0; ch < spec.numChannels; ch++)
    {
        REQUIRE(juce::Decibels::gainToDecibels<float>(magnitudeResponse.getSample(ch, 0)) < -90.0f);
        REQUIRE_THAT(juce::Decibels::gainToDecibels<float>(magnitudeResponse.getSample(ch, index)), Catch::Matchers::WithinRel(-3.0, 1e-1));
        REQUIRE_THAT(juce::Decibels::gainToDecibels<float>(magnitudeResponse.getSample(ch, fftSize / 2 -1)), Catch::Matchers::WithinAbs(0.0, 1e-2));
    }
}

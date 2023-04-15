#include "TestHelpers.h"
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <shared_modules/shared_modules.h>

TEST_CASE ("One pole lowpass filter has correct frequency response for different sample rates", "[OnePoleFilter]")
{
    OnePoleFilter::Lowpass lowpassFilter;

    const auto runTest = [&] (const float fs)
    {
        juce::uint32 blockSize = 256;
        juce::uint32 numChannels = 2;

        juce::dsp::ProcessSpec spec{ fs, blockSize, numChannels };

        size_t fftSize = spec.maximumBlockSize;

        size_t frequencyIndex = 13;
        float fc = frequencyIndex * spec.sampleRate / static_cast<float> (fftSize);

        lowpassFilter.prepare (spec);
        lowpassFilter.setCutoffFrequency (fc, true);

        juce::AudioBuffer<float> impulseResponse = TestHelpers::impulseResponseGenerator (lowpassFilter, spec.numChannels, spec.maximumBlockSize);
        TestHelpers::ComplexBuffer frequencyResponse = TestHelpers::getFrequencyResponse (impulseResponse, fftSize);
        juce::AudioBuffer<float> magnitudeResponse = TestHelpers::getMagnitudeResponse (frequencyResponse);

        // check result per channel for DC, fc, and last possible index
        for (size_t ch = 0; ch < spec.numChannels; ch++)
        {
            CHECK_THAT (juce::Decibels::gainToDecibels<float> (magnitudeResponse.getSample (ch, 0)), Catch::Matchers::WithinAbs (0.0, 1e-8));
            CHECK_THAT (juce::Decibels::gainToDecibels<float> (magnitudeResponse.getSample (ch, frequencyIndex)), Catch::Matchers::WithinAbs (-3.0, 1e-1));
            CHECK (juce::Decibels::gainToDecibels<float> (magnitudeResponse.getSample (ch, fftSize / 2 - 1)) < -13.0f);
        }
    };

    std::vector<float> sampleRates{ 44100, 48000, 96000, 192000 };

    for (auto& s : sampleRates)
        runTest (s);
}

TEST_CASE ("One pole lowpass filter has correct frequency response for different cutoff frequencies", "[OnePoleFilter]")
{
    OnePoleFilter::Lowpass lowpassFilter;

    float fs = 48000.0;
    juce::uint32 blockSize = 256;
    juce::uint32 numChannels = 2;

    juce::dsp::ProcessSpec spec{ fs, blockSize, numChannels };
    lowpassFilter.prepare (spec);

    size_t fftSize = spec.maximumBlockSize;

    const auto runTest = [&] (const size_t frequencyIndex)
    {
        float fc = frequencyIndex * spec.sampleRate / static_cast<float> (fftSize);

        lowpassFilter.setCutoffFrequency (fc, true);

        juce::AudioBuffer<float> impulseResponse = TestHelpers::impulseResponseGenerator (lowpassFilter, spec.numChannels, spec.maximumBlockSize);
        TestHelpers::ComplexBuffer frequencyResponse = TestHelpers::getFrequencyResponse (impulseResponse, fftSize);
        juce::AudioBuffer<float> magnitudeResponse = TestHelpers::getMagnitudeResponse (frequencyResponse);

        // check result per channel for DC, fc, and last possible index
        for (size_t ch = 0; ch < spec.numChannels; ch++)
        {
            CHECK_THAT (juce::Decibels::gainToDecibels<float> (magnitudeResponse.getSample (ch, 0)), Catch::Matchers::WithinAbs (0.0, 1e-8));
            CHECK_THAT (juce::Decibels::gainToDecibels<float> (magnitudeResponse.getSample (ch, frequencyIndex)), Catch::Matchers::WithinAbs (-3.0, 1e-1));
            CHECK (juce::Decibels::gainToDecibels<float> (magnitudeResponse.getSample (ch, fftSize / 2 - 1)) < -13.0f);
        }
    };

    std::vector<size_t> frequencyIndices{ 5, 10, 13, 17 };

    for (auto& f : frequencyIndices)
        runTest (f);
}

TEST_CASE ("One pole highpass filter has correct frequency response for different sample rates", "[OnePoleFilter]")
{
    OnePoleFilter::Highpass highpassFilter;

    const auto runTest = [&] (const float fs)
    {
        juce::uint32 blockSize = 256;
        juce::uint32 numChannels = 2;

        juce::dsp::ProcessSpec spec{ fs, blockSize, numChannels };

        size_t fftSize = spec.maximumBlockSize;

        size_t frequencyIndex = 13;
        float fc = frequencyIndex * spec.sampleRate / static_cast<float> (fftSize);

        highpassFilter.prepare (spec);
        highpassFilter.setCutoffFrequency (fc, true);

        juce::AudioBuffer<float> impulseResponse = TestHelpers::impulseResponseGenerator (highpassFilter, spec.numChannels, spec.maximumBlockSize);
        TestHelpers::ComplexBuffer frequencyResponse = TestHelpers::getFrequencyResponse (impulseResponse, fftSize);
        juce::AudioBuffer<float> magnitudeResponse = TestHelpers::getMagnitudeResponse (frequencyResponse);

        // check result per channel for DC, fc, and last possible index
        for (size_t ch = 0; ch < spec.numChannels; ch++)
        {
            CHECK (juce::Decibels::gainToDecibels<float> (magnitudeResponse.getSample (ch, 0)) < -90.0f);
            CHECK_THAT (juce::Decibels::gainToDecibels<float> (magnitudeResponse.getSample (ch, frequencyIndex)), Catch::Matchers::WithinRel (-3.0, 1e-1));
            CHECK_THAT (juce::Decibels::gainToDecibels<float> (magnitudeResponse.getSample (ch, fftSize / 2 - 1)), Catch::Matchers::WithinAbs (0.0, 1e-2));
        }
    };

    std::vector<float> sampleRates{ 44100, 48000, 96000, 192000 };

    for (auto& s : sampleRates)
        runTest (s);
}

TEST_CASE ("One pole highpass filter has correct frequency response for different cutoff frequencies", "[OnePoleFilter]")
{
    OnePoleFilter::Highpass highpassFilter;

    float fs = 48000.0;
    juce::uint32 blockSize = 256;
    juce::uint32 numChannels = 2;

    juce::dsp::ProcessSpec spec{ fs, blockSize, numChannels };
    highpassFilter.prepare (spec);

    size_t fftSize = spec.maximumBlockSize;

    const auto runTest = [&] (const size_t frequencyIndex)
    {
        float fc = frequencyIndex * spec.sampleRate / static_cast<float> (fftSize);

        highpassFilter.setCutoffFrequency (fc, true);

        juce::AudioBuffer<float> impulseResponse = TestHelpers::impulseResponseGenerator (highpassFilter, spec.numChannels, spec.maximumBlockSize);
        TestHelpers::ComplexBuffer frequencyResponse = TestHelpers::getFrequencyResponse (impulseResponse, fftSize);
        juce::AudioBuffer<float> magnitudeResponse = TestHelpers::getMagnitudeResponse (frequencyResponse);

        // check result per channel for DC, fc, and last possible index
        for (size_t ch = 0; ch < spec.numChannels; ch++)
        {
            CHECK (juce::Decibels::gainToDecibels<float> (magnitudeResponse.getSample (ch, 0)) < -90.0f);
            CHECK_THAT (juce::Decibels::gainToDecibels<float> (magnitudeResponse.getSample (ch, frequencyIndex)), Catch::Matchers::WithinRel (-3.0, 1e-1));
            CHECK_THAT (juce::Decibels::gainToDecibels<float> (magnitudeResponse.getSample (ch, fftSize / 2 - 1)), Catch::Matchers::WithinAbs (0.0, 1e-2));
        }
    };

    std::vector<size_t> frequencyIndices{ 8, 14, 20, 22 };

    for (auto& f : frequencyIndices)
        runTest (f);
}

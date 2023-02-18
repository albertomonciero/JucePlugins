#include "TestHelpers.h"
#include <shared_modules/shared_modules.h>


TEST_CASE ("Test allpass filter with 1 sample delay generates correct frequency response")
{
    float fs = GENERATE (44100, 48000, 96000);

    VariableDelayAllpass allpass(1);

    juce::dsp::ProcessSpec spec {fs, 256, 2};
    allpass.prepare(spec);

    allpass.setDelayInSamples(1);

    size_t fftSize = spec.maximumBlockSize;

    size_t frequencyIndex = static_cast<size_t> (GENERATE(10, 20, 35));
    float fc = spec.sampleRate / static_cast<float> (fftSize) * (frequencyIndex);

    // Equation from Zölzer DAFX p.39
    float c = (tan(M_PI * fc / fs) - 1) / (tan(M_PI * fc / fs) + 1); 
    allpass.setGain(c);

    juce::AudioBuffer<float> impulseResponse = TestHelpers::impulseResponseGenerator(allpass, spec.numChannels, spec.maximumBlockSize);
    TestHelpers::ComplexBuffer frequencyResponse = TestHelpers::getFrequencyResponse(impulseResponse, fftSize);
    juce::AudioBuffer<float> magnitudeResponse = TestHelpers::getMagnitudeResponse(frequencyResponse);
    juce::AudioBuffer<float> phaseResponse = TestHelpers::getPhaseResponse(frequencyResponse);

    for (size_t ch = 0; ch < spec.numChannels; ch++)
    {
        // magnitude should be at 0 dB for all frequency values
        for (auto i = 0; i < magnitudeResponse.getNumSamples(); i++)
            CHECK_THAT(juce::Decibels::gainToDecibels<float>(magnitudeResponse.getSample(ch, i)), Catch::Matchers::WithinAbs(0.0, 1e-5));

        // phase response should be -90 deg at fc
        CHECK_THAT(phaseResponse.getSample(ch, frequencyIndex), Catch::Matchers::WithinAbs(-M_PI / 2, 1e-6));
    }
}

TEST_CASE ("Test allpass filter with delays bigger than 1 samples generates correct frequency response")
{
    float fs = GENERATE(44100, 48000, 96000);

    VariableDelayAllpass allpass(50);

    // with longer sample delays than 1, more samples are required to increase the resolution
    juce::dsp::ProcessSpec spec {fs, 1024, 2};
    allpass.prepare(spec);

    size_t fftSize = spec.maximumBlockSize;
    size_t frequencyIndex =  80;

    float fc = spec.sampleRate / static_cast<float> (fftSize) * (frequencyIndex);

    // Equation from Zölzer DAFX p.39
    float c = (tan(M_PI * fc / fs) - 1) / (tan(M_PI * fc / fs) + 1); 
    allpass.setGain(c);

    // pair delayInSamples with expected phase response values from numerical simulations at fc
    auto delayInSamples = GENERATE(table<float, float>
    ({
         {2.0, -2.265119315942736},
         {15.0, -8.633073980079889},
         {50.0, -23.37163478234349}
     }));

    allpass.setDelayInSamples(std::get<0>(delayInSamples));

    juce::AudioBuffer<float> impulseResponse = TestHelpers::impulseResponseGenerator(allpass, spec.numChannels, spec.maximumBlockSize);
    TestHelpers::ComplexBuffer frequencyResponse = TestHelpers::getFrequencyResponse(impulseResponse, fftSize);
    juce::AudioBuffer<float> magnitudeResponse = TestHelpers::getMagnitudeResponse(frequencyResponse);
    juce::AudioBuffer<float> phaseResponse = TestHelpers::getPhaseResponse(frequencyResponse);

    for (size_t ch = 0; ch < spec.numChannels; ch++)
    {
        // magnitude should be at 0 dB for all frequency values
       for (auto i = 0; i < magnitudeResponse.getNumSamples(); i++)
            CHECK_THAT(juce::Decibels::gainToDecibels<float>(magnitudeResponse.getSample(ch, i)), Catch::Matchers::WithinAbs(0.0, 1e-3));

        CHECK_THAT(phaseResponse.getSample(ch, frequencyIndex), Catch::Matchers::WithinAbs(std::get<1>(delayInSamples), 1e-4));     
    }
}

TEST_CASE ("Test allpass filter return correct multitap outputs")
{
    float fs = GENERATE(44100, 48000, 96000);

    std::array<size_t, 3> taps = {3, 2, 1};

    VariableDelayAllpass allpass(4, taps.size());

    juce::dsp::ProcessSpec spec {fs, 4, 2};
    allpass.prepare(spec);

    for (size_t i = 0; i < taps.size(); i++)
        allpass.setDelayInSamples(i, taps[i]);
 
    juce::AudioBuffer<float> input(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));

    float sampleValue = 1.0;

    for (size_t ch = 0; ch < spec.numChannels; ch++)
        input.setSample(ch, 0, sampleValue);
    
    juce::dsp::AudioBlock<float> block(input);
    juce::dsp::ProcessContextReplacing<float> context (block);

    allpass.process(context);

    for (size_t ch = 0; ch < spec.numChannels; ch++)
        for (size_t n = 0; n < taps.size(); n++)
            CHECK(allpass.getTapOutBuffer(ch, n)[taps[n]] == sampleValue);
}

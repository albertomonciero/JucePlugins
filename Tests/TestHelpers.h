#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <juce_dsp/juce_dsp.h>

namespace TestHelpers
{
    using ComplexArray = std::vector<std::complex<float>>;
    using ComplexBuffer = std::vector<ComplexArray>;

    class SimpleMultiplier : public juce::dsp::ProcessorBase
    {
    public:
        SimpleMultiplier(float initValue) : _value(initValue) {}
        virtual void prepare (const juce::dsp::ProcessSpec& spec) override { (void) spec; }
        virtual void reset() override {}
        virtual void process (const juce::dsp::ProcessContextReplacing<float>& context)
        {
            context.getOutputBlock().multiplyBy(_value);
        }

        void setValue(float newValue) { _value = newValue; }

    private:
        float _value = 0.0f;
    };

    juce::AudioBuffer<float> impulseResponseGenerator(juce::dsp::ProcessorBase& processorToTest, size_t numChannels, size_t length) 
    {
        juce::AudioBuffer<float> pulse(numChannels, length);
        pulse.clear();

        // generate single pulse
        for (size_t ch = 0; ch < numChannels; ch++)
            pulse.setSample(ch, 0, 1.0);

        // generate impulse response
        juce::dsp::AudioBlock<float> block(pulse);
        juce::dsp::ProcessContextReplacing context (block);
        processorToTest.process(context);

        return pulse;
    }

    ComplexBuffer getFrequencyResponse(const juce::AudioBuffer<float>& impulseResponse, size_t fftSize)
    {
        size_t numSamples = static_cast<size_t> (impulseResponse.getNumSamples());
        size_t numChannels = static_cast<size_t> (impulseResponse.getNumChannels());

        assert (numSamples <= fftSize);

        // perform frequency response
        juce::dsp::FFT fft(std::log2(fftSize));
        juce::AudioBuffer<float> fftBuffer (numChannels, 2 * fftSize);
        fftBuffer.clear();

        for (size_t ch = 0; ch < numChannels; ch++)
            fftBuffer.copyFrom(ch, 0, impulseResponse.getReadPointer(0), numSamples);

        for (size_t ch = 0; ch < numChannels; ch++)
            fft.performRealOnlyForwardTransform(fftBuffer.getWritePointer(ch));

        // copy interleaved response to complex vector
        ComplexArray frequencyResponsePerChannel(fftSize);
        ComplexBuffer multiChannelsFrequencyResponse(numChannels, frequencyResponsePerChannel);

        for (size_t ch = 0; ch < numChannels; ch++)
            for (size_t i = 0; i < fftSize; i++)
            {   
                multiChannelsFrequencyResponse[ch][i].real (fftBuffer.getSample(0, i * 2));
                multiChannelsFrequencyResponse[ch][i].imag (fftBuffer.getSample(0, i * 2 + 1));
            }

        return multiChannelsFrequencyResponse;
    } 

    juce::AudioBuffer<float> getMagnitudeResponse (const ComplexBuffer& multiChannelsFrequencyResponse)
    {
        juce::AudioBuffer<float> multiChannelsMagnitudeResponse(multiChannelsFrequencyResponse.size(), multiChannelsFrequencyResponse[0].size() / 2);

        for (size_t ch = 0; ch < multiChannelsFrequencyResponse.size(); ch++)
            for (size_t i = 0; i < multiChannelsFrequencyResponse[0].size() / 2; i++)
                multiChannelsMagnitudeResponse.setSample(ch, i, std::abs(multiChannelsFrequencyResponse[ch][i]));

        return multiChannelsMagnitudeResponse;
    }
}

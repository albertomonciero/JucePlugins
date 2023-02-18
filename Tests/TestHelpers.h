#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
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

    static juce::AudioBuffer<float> impulseResponseGenerator(juce::dsp::ProcessorBase& processorToTest, size_t numChannels, size_t length) 
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

    static ComplexBuffer getFrequencyResponse(const juce::AudioBuffer<float>& impulseResponse, size_t fftSize)
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

    static juce::AudioBuffer<float> getMagnitudeResponse (const ComplexBuffer& multiChannelsFrequencyResponse)
    {
        juce::AudioBuffer<float> multiChannelsMagnitudeResponse(multiChannelsFrequencyResponse.size(), multiChannelsFrequencyResponse[0].size() / 2);

        for (size_t ch = 0; ch < multiChannelsFrequencyResponse.size(); ch++)
            for (size_t i = 0; i < multiChannelsFrequencyResponse[0].size() / 2; i++)
                multiChannelsMagnitudeResponse.setSample(ch, i, std::abs(multiChannelsFrequencyResponse[ch][i]));

        return multiChannelsMagnitudeResponse;
    }

    static void unwrapPhase(const float* phase, float* unwrappedPhase, size_t length) 
    {
        unwrappedPhase[0] = phase[0];

        float phaseCorrection = 0.0f;
        float diff, diffModulo;

        for (size_t i = 1; i < length; i++)
        {
            diff = phase[i] - phase[i - 1];
            diffModulo = fmod(diff + 3 * M_PI, 2 * M_PI) - M_PI;

            if (diffModulo == -M_PI && diff > 0)
                diffModulo = M_PI;

            if (abs(diff) >= M_PI)
                phaseCorrection += (diffModulo - diff);
        
            unwrappedPhase[i] = phase[i] + phaseCorrection;
        }      
    }

    static juce::AudioBuffer<float> getPhaseResponse (const ComplexBuffer& multiChannelsFrequencyResponse, bool unwrap = true)
    {
        juce::AudioBuffer<float> multiChannelsPhaseResponse(multiChannelsFrequencyResponse.size(), multiChannelsFrequencyResponse[0].size() / 2);

        for (size_t ch = 0; ch < multiChannelsFrequencyResponse.size(); ch++)
            for (size_t i = 0; i < multiChannelsFrequencyResponse[0].size() / 2; i++)
                multiChannelsPhaseResponse.setSample(ch, i, std::arg(multiChannelsFrequencyResponse[ch][i]));

        if (! unwrap)
            return multiChannelsPhaseResponse;
        
        else
        {
            juce::AudioBuffer<float> unwrappedPhase(multiChannelsPhaseResponse.getNumChannels(), multiChannelsPhaseResponse.getNumSamples());

            for (auto ch = 0; ch < multiChannelsPhaseResponse.getNumChannels(); ch++)
                unwrapPhase(multiChannelsPhaseResponse.getReadPointer(ch), unwrappedPhase.getWritePointer(ch), static_cast<size_t>(unwrappedPhase.getNumSamples()));

            return unwrappedPhase;
        }
    }
}

#include "OnePoleFilter.h"

namespace OnePoleFilter
{
    void Lowpass::prepare (const juce::dsp::ProcessSpec& spec)
    {
        _b0.resize (spec.numChannels);
        _a1.resize (spec.numChannels);

        _zPole.clear();

        _fs = spec.sampleRate;

        for (size_t ch = 0; ch < spec.numChannels; ch++)
        {
            _b0[ch].reset (spec.sampleRate, 0.05);
            _a1[ch].reset (spec.sampleRate, 0.05);
            _zPole.emplace_back (0.0);
        }
    }

    void Lowpass::reset()
    {
        for (size_t ch = 0; ch < _b0.size(); ch++)
        {
            _b0[ch].reset (_fs, 0.05);
            _a1[ch].reset (_fs, 0.05);
            _zPole[ch] = 0.0f;
        }
    }

    void Lowpass::setCutoffFrequency (float fc, bool force)
    {
        jassert (_fs > 0);

        float alpha = std::exp (-2.0 * M_PI * fc / _fs);

        if (force)
        {
            for (size_t ch = 0; ch < _b0.size(); ch++)
            {
                _a1[ch].setCurrentAndTargetValue (alpha);
                _b0[ch].setCurrentAndTargetValue (1.0 - alpha);
            }
        }
        else
        {
            for (size_t ch = 0; ch < _b0.size(); ch++)
            {
                _a1[ch].setTargetValue (alpha);
                _b0[ch].setTargetValue (1.0 - alpha);
            }
        }
    }

    void Lowpass::process (const juce::dsp::ProcessContextReplacing<float>& context)
    {
        auto& outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();

        for (size_t ch = 0; ch < numChannels; ch++)
        {
            auto* samples = outputBlock.getChannelPointer (ch);

            for (size_t i = 0; i < numSamples; i++)
            {
                _zPole[ch] = samples[i] * _b0[ch].getNextValue() + _zPole[ch] * _a1[ch].getNextValue();
                samples[i] = _zPole[ch];
            }
        }
    }

    void Highpass::prepare (const juce::dsp::ProcessSpec& spec)
    {
        _b0.resize (spec.numChannels);
        _b1.resize (spec.numChannels);
        _a1.resize (spec.numChannels);

        _zPole.clear();
        _zZero.clear();

        _fs = spec.sampleRate;

        for (size_t ch = 0; ch < spec.numChannels; ch++)
        {
            _b0[ch].reset (spec.sampleRate, 0.05);
            _b1[ch].reset (spec.sampleRate, 0.05);
            _a1[ch].reset (spec.sampleRate, 0.05);
            _zPole.emplace_back (0.0);
            _zZero.emplace_back (0.0);
        }
    }

    void Highpass::reset()
    {
        for (size_t ch = 0; ch < _b0.size(); ch++)
        {
            _b0[ch].reset (_fs, 0.05);
            _b1[ch].reset (_fs, 0.05);
            _a1[ch].reset (_fs, 0.05);
            _zPole[ch] = 0.0f;
            _zZero[ch] = 0.0f;
        }
    }

    void Highpass::setCutoffFrequency (float fc, bool force)
    {
        jassert (_fs > 0);

        float alpha = std::exp (-2.0 * M_PI * fc / _fs);
        float b0 = (1.0 + alpha) / 2.0;

        if (force)
        {
            for (size_t ch = 0; ch < _b0.size(); ch++)
            {
                _a1[ch].setCurrentAndTargetValue (alpha);
                _b0[ch].setCurrentAndTargetValue (b0);
                _b1[ch].setCurrentAndTargetValue (-b0);
            }
        }
        else
        {
            for (size_t ch = 0; ch < _b0.size(); ch++)
            {
                _a1[ch].setTargetValue (alpha);
                _b0[ch].setTargetValue (b0);
                _b1[ch].setTargetValue (-b0);
            }
        }
    }

    void Highpass::process (const juce::dsp::ProcessContextReplacing<float>& context)
    {
        auto& outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();

        for (size_t ch = 0; ch < numChannels; ch++)
        {
            auto* samples = outputBlock.getChannelPointer (ch);

            for (size_t i = 0; i < numSamples; i++)
            {
                _zPole[ch] = samples[i] * _b0[ch].getNextValue() + _zZero[ch] * _b1[ch].getNextValue() + _zPole[ch] * _a1[ch].getNextValue();
                _zZero[ch] = samples[i];
                samples[i] = _zPole[ch];
            }
        }
    }
} // namespace OnePoleFilter

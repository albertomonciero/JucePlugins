#include "OnePoleFilter.h"

namespace OnePoleFilter
{
    void Lowpass::prepare (const juce::dsp::ProcessSpec& spec)
    {
        _b0.clear();
        _a1.clear();
        _zPole.clear();

        _fs = spec.sampleRate;

        for (size_t ch = 0; ch < spec.numChannels; ch++)
        {
            _b0.emplace_back(1.0);
            _a1.emplace_back(0.0);
            _zPole.emplace_back(0.0);
        }
    }

    void Lowpass::reset()
    {
        for (size_t ch = 0; ch < _b0.size(); ch++)
        {
            _b0[ch] = 1.0f;
            _a1[ch] = 0.0f;
            _zPole[ch] = 0.0f;
        }
    }

    void Lowpass::setCutoffFrequency(float fc)
    {
        jassert(_fs > 0);

        float alpha = std::exp(-2.0 * M_PI * fc / _fs);

        for (size_t ch = 0; ch < _b0.size(); ch++)
        {
            _a1[ch] = alpha;
            _b0[ch] = 1.0 - _a1[ch];
        }
    }
    void Lowpass::process (const juce::dsp::ProcessContextReplacing<float>& context)
    {
        auto& outputBlock      = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples  = outputBlock.getNumSamples();

        for (size_t ch = 0; ch < numChannels; ch++)
        {
            auto* samples = outputBlock.getChannelPointer (ch);

            for (size_t i = 0; i < numSamples; i++)
            {
                _zPole[ch] = samples[i] * _b0[ch] + _zPole[ch] * _a1[ch];
                samples[i] = _zPole[ch];
            }
        }
    }

    void Highpass::prepare (const juce::dsp::ProcessSpec& spec)
    {
        _b0.clear();
        _b1.clear();
        _a1.clear();
        _zPole.clear();
        _zZero.clear();

        _fs = spec.sampleRate;

        for (size_t ch = 0; ch < spec.numChannels; ch++)
        {
            _b0.emplace_back(1.0);
            _b1.emplace_back(0.0);
            _a1.emplace_back(0.0);
            _zPole.emplace_back(0.0);
            _zZero.emplace_back(0.0);
        }
    }

    void Highpass::reset()
    {
        for (size_t ch = 0; ch < _b0.size(); ch++)
        {
            _b0[ch] = 1.0f;
            _b1[ch] = 0.0f;
            _a1[ch] = 0.0f;
            _zPole[ch] = 0.0f;
            _zZero[ch] = 0.0f;
        }
    }

    void Highpass::setCutoffFrequency(float fc)
    {
        jassert(_fs > 0);

        float alpha = std::exp(-2.0 * M_PI * fc / _fs);

        for (size_t ch = 0; ch < _b0.size(); ch++)
        {
            _a1[ch] = alpha;
            _b0[ch] = (1.0 + alpha) / 2.0;
            _b1[ch] = -_b0[ch];
        }
    }

    void Highpass::process (const juce::dsp::ProcessContextReplacing<float>& context)
    {
        auto& outputBlock      = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples  = outputBlock.getNumSamples();

        for (size_t ch = 0; ch < numChannels; ch++)
        {
            auto* samples = outputBlock.getChannelPointer (ch);

            for (size_t i = 0; i < numSamples; i++)
            {
                _zPole[ch] = samples[i] * _b0[ch] + _b1[ch] * _zZero[ch] + _zPole[ch] * _a1[ch];
                _zZero[ch] = samples[i];
                samples[i] = _zPole[ch];
            }
        }
    }
}
    
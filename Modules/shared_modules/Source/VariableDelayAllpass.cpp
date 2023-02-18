#include "VariableDelayAllpass.h"

VariableDelayAllpass::VariableDelayAllpass(size_t maxDelayInSamples, size_t numTaps)
    :   _delayLine(maxDelayInSamples),
        _numTaps(numTaps)
{
}

void VariableDelayAllpass::prepare (const juce::dsp::ProcessSpec& spec)
{
    _delayLine.prepare(spec);

    _delayInSamples.clear();
    _tapOutBuffer.clear();

    for (size_t i = 0; i < _numTaps; i++)
        _delayInSamples.emplace_back(0.0f);

    for (size_t ch = 0; ch < spec.numChannels; ch++)
    {
        _tapOutBuffer.emplace_back(_numTaps, spec.maximumBlockSize);  
        _tapOutBuffer[ch].clear();
    }

    reset();
}

void VariableDelayAllpass::reset()
{
    _delayLine.reset();

    for (size_t ch = 0; ch < _tapOutBuffer.size(); ch++)
        _tapOutBuffer[ch].clear();

    for (size_t i = 0; i < _numTaps; i++)
        _delayInSamples[i] = 0.0f;
}

void VariableDelayAllpass::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& outputBlock      = context.getOutputBlock();
    const auto numChannels = outputBlock.getNumChannels();
    const auto numSamples  = outputBlock.getNumSamples();

    for (size_t ch = 0; ch < numChannels; ch++)
    {
        auto* samples = outputBlock.getChannelPointer (ch);

        for (size_t i = 0; i < numSamples; i++)
        {
            for (size_t n = 1; n < _numTaps; n++)
                _tapOutBuffer[ch].setSample(n, i, _delayLine.popSample(ch, _delayInSamples[n], false));

            float mainTapOut = _delayLine.popSample(ch, _delayInSamples[0]);

            _tapOutBuffer[ch].setSample(0, i, mainTapOut);

            float in = samples[i] - mainTapOut * _gain;
            float out = mainTapOut + in * _gain;
            _delayLine.pushSample(ch, in);

            samples[i] = out;
        }
    }
}

void VariableDelayAllpass::setDelayInSamples (float newDelayInSamples)
{
    jassert(newDelayInSamples <= _delayLine.getMaximumDelayInSamples());
    
    _delayInSamples[0] = newDelayInSamples;
}

void VariableDelayAllpass::setDelayInSamples (size_t tapIndex, float newDelayInSamples)
{
    jassert(newDelayInSamples <= _delayLine.getMaximumDelayInSamples());
    jassert(tapIndex < _numTaps);

    _delayInSamples[tapIndex] = newDelayInSamples;
}

void VariableDelayAllpass::setGain (float newGain)
{
    _gain = newGain;
}

const float* VariableDelayAllpass::getTapOutBuffer(size_t channelIndex, size_t tapIndex) const
{
    jassert(channelIndex < _tapOutBuffer.size());
    jassert(tapIndex < _numTaps);

    return _tapOutBuffer[channelIndex].getReadPointer(tapIndex);
}

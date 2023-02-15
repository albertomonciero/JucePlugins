#include "VariableDelayLine.h"

VariableDelayLine::VariableDelayLine(size_t maxDelayInSample, size_t numTaps)
    : _delayLine(maxDelayInSample),
      _numTaps(numTaps)
{
}

void VariableDelayLine::prepare (const juce::dsp::ProcessSpec& spec)
{
    _delayInSamples.clear();
    _tapOutBuffer.clear();

    for (size_t i = 0; i < _numTaps; i++)
        _delayInSamples.emplace_back();
    
    for (size_t ch = 0; ch < spec.numChannels; ch++)
        _tapOutBuffer.emplace_back(_numTaps, spec.maximumBlockSize);

    _delayLine.prepare(spec);

    reset();
}

void VariableDelayLine::reset()
{
    for (size_t i = 0; i < _delayInSamples.size(); i++)
        _delayInSamples[i] = 0.0f;
    
    for (size_t ch = 0; ch < _tapOutBuffer.size(); ch++)
        _tapOutBuffer[ch].clear();

    _delayLine.reset();
}

void VariableDelayLine::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& outputBlock      = context.getOutputBlock();
    const auto numChannels = outputBlock.getNumChannels();
    const auto numSamples  = outputBlock.getNumSamples();

    for (size_t ch = 0; ch < numChannels; ch++)
    {
        auto* samples = outputBlock.getChannelPointer (ch);

        for (size_t i = 0; i < numSamples; i++)
        {
            _delayLine.pushSample(ch, samples[i]);

            for (size_t n = 1; n < _numTaps; n++)
                _tapOutBuffer[ch].setSample(n, i, _delayLine.popSample(ch, _delayInSamples[n], false));

            float mainTapOut = _delayLine.popSample(ch, _delayInSamples[0]);
            _tapOutBuffer[ch].setSample(0, i, mainTapOut);
        
            samples[i] = mainTapOut;
        }
    }
}

void VariableDelayLine::setDelayInSamples (float newDelayInSamples)
{
    jassert(newDelayInSamples < getMaximumDelayInSamples());

    _delayInSamples[0] = newDelayInSamples;
}

void VariableDelayLine::setDelayInSamples (size_t tapIndex, float newDelayInSamples)
{
    jassert(tapIndex < _numTaps);
    jassert(newDelayInSamples < getMaximumDelayInSamples());

    _delayInSamples[tapIndex] = newDelayInSamples;
}

const float* VariableDelayLine::getTapOutBuffer (size_t channelIndex, size_t tapIndex) const
{
    jassert(channelIndex < _tapOutBuffer.size());
    jassert(tapIndex < _numTaps);

    return _tapOutBuffer[channelIndex].getReadPointer(tapIndex);
}

size_t VariableDelayLine::getMaximumDelayInSamples() const
{
    return static_cast<size_t>(_delayLine.getMaximumDelayInSamples());
}
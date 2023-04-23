#include "VariableDelayAllpass.h"

VariableDelayAllpass::VariableDelayAllpass (size_t maxDelayInSamples, size_t numTaps)
    : _delayLine (maxDelayInSamples),
      _numTaps (numTaps)
{
}

void VariableDelayAllpass::prepare (const juce::dsp::ProcessSpec& spec)
{
    _delayLine.prepare (spec);

    _delayInSamples.resize (spec.numChannels);
    _gain.resize (spec.numChannels);

    _tapOutBuffer.clear();

    for (size_t ch = 0; ch < spec.numChannels; ch++)
    {
        _tapOutBuffer.emplace_back (_numTaps, spec.maximumBlockSize);
        _tapOutBuffer[ch].clear();
        _gain[ch].reset (spec.sampleRate, 0.05);
        _delayInSamples[ch].resize (_numTaps);
    }

    for (size_t ch = 0; ch < spec.numChannels; ch++)
        for (size_t i = 0; i < _numTaps; i++)
            _delayInSamples[ch][i].reset (spec.sampleRate, 0.05);

    reset();
}

void VariableDelayAllpass::reset()
{
    _delayLine.reset();

    for (size_t ch = 0; ch < _tapOutBuffer.size(); ch++)
        _tapOutBuffer[ch].clear();
}

void VariableDelayAllpass::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& outputBlock = context.getOutputBlock();
    const auto numChannels = outputBlock.getNumChannels();
    const auto numSamples = outputBlock.getNumSamples();

    for (size_t ch = 0; ch < numChannels; ch++)
    {
        auto* samples = outputBlock.getChannelPointer (ch);

        for (size_t i = 0; i < numSamples; i++)
        {
            for (size_t n = 1; n < _numTaps; n++)
                _tapOutBuffer[ch].setSample (n, i, _delayLine.popSample (ch, _delayInSamples[ch][n].getNextValue(), false));

            float mainTapOut = _delayLine.popSample (ch, _delayInSamples[ch][0].getNextValue());

            _tapOutBuffer[ch].setSample (0, i, mainTapOut);

            float in = samples[i] - mainTapOut * _gain[ch].getNextValue();
            float out = mainTapOut + in * _gain[ch].getNextValue();
            _delayLine.pushSample (ch, in);

            samples[i] = out;
        }
    }
}

void VariableDelayAllpass::setDelayInSamples (float newDelayInSamples, size_t tapIndex, bool force)
{
    jassert (tapIndex < _numTaps);
    jassert (newDelayInSamples < _delayLine.getMaximumDelayInSamples());

    if (force)
        for (int ch = 0; ch < _delayInSamples.size(); ch++)
            _delayInSamples[ch][tapIndex].setCurrentAndTargetValue (newDelayInSamples);
    else
        for (int ch = 0; ch < _delayInSamples.size(); ch++)
            _delayInSamples[ch][tapIndex].setTargetValue (newDelayInSamples);
}

void VariableDelayAllpass::setGain (float newGain, bool force)
{
    if (force)
        for (int ch = 0; ch < _gain.size(); ch++)
            _gain[ch].setCurrentAndTargetValue (newGain);
    else
        for (int ch = 0; ch < _gain.size(); ch++)
            _gain[ch].setTargetValue (newGain);
}

const float* VariableDelayAllpass::getTapOutBuffer (size_t channelIndex, size_t tapIndex) const
{
    jassert (channelIndex < _tapOutBuffer.size());
    jassert (tapIndex < _numTaps);

    return _tapOutBuffer[channelIndex].getReadPointer (tapIndex);
}

#pragma once

class VariableDelayAllpass : public juce::dsp::ProcessorBase
{
public:
    VariableDelayAllpass (size_t maxDelayInSamples, size_t numTaps = 1);

    virtual void prepare (const juce::dsp::ProcessSpec& spec) override;
    virtual void reset() override;
    virtual void process (const juce::dsp::ProcessContextReplacing<float>& context) override;

    void setDelayInSamples (float newDelayInSamples, size_t tapIndex = 0, bool force = false);
    void setGain (float newGain, bool force = false);

    const float* getTapOutBuffer (size_t channelIndex, size_t tapIndex) const;

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> _delayLine;
    std::vector<std::vector<juce::LinearSmoothedValue<float>>> _delayInSamples;
    size_t _numTaps;
    std::vector<juce::AudioBuffer<float>> _tapOutBuffer;
    std::vector<juce::LinearSmoothedValue<float>> _gain;
};

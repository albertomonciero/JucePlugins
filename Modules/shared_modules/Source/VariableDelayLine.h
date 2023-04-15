#pragma once

class VariableDelayLine : public juce::dsp::ProcessorBase
{
public:
    VariableDelayLine (size_t maxDelayInSample, size_t numTaps = 1);

    virtual void prepare (const juce::dsp::ProcessSpec& spec) override;
    virtual void reset() override;
    virtual void process (const juce::dsp::ProcessContextReplacing<float>& context) override;

    void setDelayInSamples (float newDelayInSamples, bool force = false);
    void setDelayInSamples (size_t tapIndex, float newDelayInSamples, bool force = false);

    const float* getTapOutBuffer (size_t channelIndex, size_t tapIndex) const;
    size_t getMaximumDelayInSamples() const;

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> _delayLine;
    std::vector<juce::LinearSmoothedValue<float>> _delayInSamples;
    size_t _numTaps;
    std::vector<juce::AudioBuffer<float>> _tapOutBuffer;
};

#pragma once

class VariableDelayAllpass : public juce::dsp::ProcessorBase
{
public:
    VariableDelayAllpass(size_t maxDelayInSamples, size_t numTaps = 1);

    virtual void prepare (const juce::dsp::ProcessSpec& spec) override;
    virtual void reset() override;
    virtual void process (const juce::dsp::ProcessContextReplacing<float>& context) override;

    void setDelayInSamples (float newDelayInSamples);
    void setDelayInSamples (size_t tapIndex, float newDelayInSamples);
    void setGain (float newGain);

    const float* getTapOutBuffer(size_t channelIndex, size_t tapIndex) const;
    
private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> _delayLine;
    std::vector<float> _delayInSamples;
    size_t _numTaps;
    std::vector<juce::AudioBuffer<float>> _tapOutBuffer;
    float _gain = 0.0f;
};

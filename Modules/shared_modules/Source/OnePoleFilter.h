#pragma once

namespace OnePoleFilter
{
    class Lowpass : public juce::dsp::ProcessorBase
    {
    public:
        virtual void prepare (const juce::dsp::ProcessSpec& spec) override;
        virtual void reset() override;
        virtual void process (const juce::dsp::ProcessContextReplacing<float>& context) override;

        void setCutoffFrequency (float fc, bool force = false);

    private:
        std::vector<juce::LinearSmoothedValue<float>> _b0;
        std::vector<juce::LinearSmoothedValue<float>> _a1;
        std::vector<float> _zPole;
        float _fs = 0.0f;
    };

    class Highpass : public juce::dsp::ProcessorBase
    {
    public:
        virtual void prepare (const juce::dsp::ProcessSpec& spec) override;
        virtual void reset() override;
        virtual void process (const juce::dsp::ProcessContextReplacing<float>& context) override;

        void setCutoffFrequency (float fc, bool force = false);

    private:
        std::vector<juce::LinearSmoothedValue<float>> _b0;
        std::vector<juce::LinearSmoothedValue<float>> _b1;
        std::vector<juce::LinearSmoothedValue<float>> _a1;
        std::vector<float> _zPole;
        std::vector<float> _zZero;
        float _fs = 0.0f;
    };
} // namespace OnePoleFilter

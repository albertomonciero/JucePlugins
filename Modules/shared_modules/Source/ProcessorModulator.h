#pragma once

class OscillatorWrapper : public juce::dsp::ProcessorBase
{
public:
    enum Waveform
    {
        Sine,
        Saw,
        Square,
        Random
    };

    virtual void prepare (const juce::dsp::ProcessSpec& spec) override
    {
        _oscillator.prepare(spec);
    }

    virtual void reset() override
    {
        _oscillator.reset();
    }

    virtual void process (const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        _oscillator.process(context);
    }

    float processSample(float input)
    {
        return _oscillator.processSample(input);
    }

    void setWaveform(Waveform newWaveform, size_t numSamples = 256)
    {
        switch (newWaveform)
        {
            case Sine:
                _oscillator.initialise([] (float x) { return std::sin(x); }, numSamples);
                break;
            
            case Square:
                _oscillator.initialise([] (float x) {return x < 0.0f ? -1.0f : 1.0f; }, numSamples);
                break;
            
            case Saw:
                _oscillator.initialise([] (float x) {return x / juce::MathConstants<float>::pi; }, numSamples);
                break;
            
            case Random:
                _oscillator.initialise([] (float x) { (void) x; return 2.0 * std::rand() / RAND_MAX - 1.0; }, numSamples);
                break;

            default:
                _oscillator.initialise([] (float x) { return std::sin(x); }, numSamples);
                break;
        }
    }

    void setFrequency(float newFrequency) { _oscillator.setFrequency(newFrequency); }
    float getFrequency() const { return _oscillator.getFrequency(); }

private:
    juce::dsp::Oscillator<float> _oscillator;
};

class ProcessorModulator : public juce::dsp::ProcessorBase
{
public:
    ProcessorModulator(OscillatorWrapper& modulator, size_t updateRate);

    virtual void prepare (const juce::dsp::ProcessSpec& spec) override;
    virtual void reset() override;
    virtual void process (const juce::dsp::ProcessContextReplacing<float>& context) override;

    void setProcessorToModulate (juce::dsp::ProcessorBase& newProcessor);
    void setModulationWaveform(OscillatorWrapper::Waveform newWaveform, size_t numSamples = 256);
    void setModulationTarget(const std::function<void(float)>& newModulationTarget);
    void setModulationFrequency (float newFrequency);
    void setModulationRange (const juce::Range<float>& newRange);

private:
    OscillatorWrapper& _modulator;
    juce::dsp::ProcessorBase* _processorToModulate = nullptr;
    std::function<void(float)> _modulationTarget = nullptr;
    juce::Range<float> _modulationRange;
    size_t _updateCounter;
    size_t _updateRate;
};

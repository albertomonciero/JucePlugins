#include "ProcessorModulator.h"

ProcessorModulator::ProcessorModulator(OscillatorWrapper& modulator, size_t updateRate)
        :   _modulator(modulator), 
            _updateRate(updateRate)
{
}

void ProcessorModulator::prepare (const juce::dsp::ProcessSpec& spec)
{
   _modulator.prepare ({ spec.sampleRate / _updateRate, spec.maximumBlockSize, spec.numChannels });
    reset();
}

void ProcessorModulator::reset()
{
    _modulator.reset();
    _updateCounter = _updateRate;
}

void ProcessorModulator::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& outputBlock      = context.getOutputBlock();
    const auto numSamples  = outputBlock.getNumSamples();

    for (size_t pos = 0; pos < (size_t) numSamples; )
    {
        auto numSamplesToProcess = juce::jmin ((size_t) numSamples - pos, _updateCounter);
        auto subBlock = outputBlock.getSubBlock (pos, numSamplesToProcess);

        juce::dsp::ProcessContextReplacing<float> subContext (subBlock);

        if (_processorToModulate != nullptr)
            _processorToModulate->process(subContext);

        pos += numSamplesToProcess;
        _updateCounter -= numSamplesToProcess;

        if (_updateCounter == 0)
        {
            _updateCounter = _updateRate;

            float targetValue;

            if (_modulator.getFrequency() != 0)
            {
                float modulatorOut = _modulator.processSample (0.0f);
                targetValue = juce::jmap (modulatorOut, -1.0f, 1.0f, _modulationRange.getStart(), _modulationRange.getEnd());
            }
            else
            {
                targetValue = 0.0f;                    
            }
            
            if (_modulationTarget)
                _modulationTarget(targetValue);
        }
    }
}

void ProcessorModulator::setProcessorToModulate (juce::dsp::ProcessorBase& newProcessor)
{
    _processorToModulate = &newProcessor;
    _modulationTarget = nullptr;
}

void ProcessorModulator::setModulationWaveform(OscillatorWrapper::Waveform newWaveform, size_t numSamples)
{
    _modulator.setWaveform(newWaveform, numSamples);
}

void ProcessorModulator::setModulationTarget(const std::function<void(float)>& newModulationTarget)
{
    _modulationTarget = newModulationTarget;
}

void ProcessorModulator::setModulationFrequency (float newFrequency)
{
    _modulator.setFrequency(newFrequency);
}

void ProcessorModulator::setModulationRange (const juce::Range<float>& newRange)
{
    _modulationRange = newRange;
}

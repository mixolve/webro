#include "PluginProcessor.h"
#include "PluginEditor.h"

WbeAudioProcessor::WbeAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

WbeAudioProcessor::~WbeAudioProcessor()
{
}

const juce::String WbeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WbeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool WbeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool WbeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double WbeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WbeAudioProcessor::getNumPrograms()
{
    return 1;
}

int WbeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WbeAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String WbeAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void WbeAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void WbeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void WbeAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WbeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
   #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
   #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    #endif

    return true;
   #endif
}
#endif

void WbeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;

    for (int channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear (channel, 0, buffer.getNumSamples());
}

bool WbeAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* WbeAudioProcessor::createEditor()
{
    return new WbeAudioProcessorEditor (*this);
}

void WbeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree state ("WbeState");
    state.setProperty ("url", currentURL, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void WbeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary (data, sizeInBytes))
    {
        auto state = juce::ValueTree::fromXml (*xmlState);

        if (state.isValid())
        {
            auto newURL = state.getProperty ("url", "").toString();

            if (currentURL != newURL)
            {
                currentURL = newURL;
                sendChangeMessage();
            }
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WbeAudioProcessor();
}

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WebroAudioProcessor::WebroAudioProcessor()
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

WebroAudioProcessor::~WebroAudioProcessor()
{
}

//==============================================================================
const juce::String WebroAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WebroAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool WebroAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool WebroAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double WebroAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
int WebroAudioProcessor::getNumPrograms()
{
    return 1;
}

int WebroAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WebroAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String WebroAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void WebroAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void WebroAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void WebroAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WebroAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void WebroAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;

    for (int channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear (channel, 0, buffer.getNumSamples());
}

//==============================================================================
bool WebroAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* WebroAudioProcessor::createEditor()
{
    return new WebroAudioProcessorEditor (*this);
}

//==============================================================================
void WebroAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree state ("WebroState");
    state.setProperty ("url", currentURL, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void WebroAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WebroAudioProcessor();
}

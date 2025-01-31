/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NegDelayAudioProcessor::NegDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      ), apvts(*this, nullptr, "Parameters", Parameters::createParameterLayout())
#endif
{
}

NegDelayAudioProcessor::~NegDelayAudioProcessor() {
}

//==============================================================================
const juce::String NegDelayAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool NegDelayAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool NegDelayAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool NegDelayAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double NegDelayAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int NegDelayAudioProcessor::getNumPrograms() {
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int NegDelayAudioProcessor::getCurrentProgram() {
    return 0;
}

void NegDelayAudioProcessor::setCurrentProgram(int index) {
}

const juce::String NegDelayAudioProcessor::getProgramName(int index) {
    return {};
}

void NegDelayAudioProcessor::changeProgramName(int index, const juce::String &newName) {
}

//==============================================================================
void NegDelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    int silence = 5;
    sine.setup(sampleRate, silence);
    /*std::atomic<float>* delayType = nullptr;
    delayType = apvts.getRawParameterValue(ids.delayType);
    currentType = static_cast<DelayType>(static_cast<int>(delayType->load()));*/
    // Initialize delay with maximum possible delay time
    std::pair<float, float> bpmAndDivision{120.0f, 1.0f};
    switch (currentType) {
        case DelayType::FeedForward:
            // Chooses which delay Time type is chooses
                //uses function overloads for refactoring
            if (currentDelayTimeChoice == DelayTimeChoice::TempoSync)
                feedForwardDelay.setup(sampleRate, bpmAndDivision);
            else
                feedForwardDelay.setup(sampleRate, 1.f);

            break;
        case DelayType::FeedBack:
            if (currentDelayTimeChoice == DelayTimeChoice::TempoSync)
                feedbackDelay.setup(sampleRate, bpmAndDivision);
            else
                feedbackDelay.setup(sampleRate, 1.f);

            break;
        default:
            feedForwardDelay.setup(sampleRate, 1.f);
            break;
    }
}


void NegDelayAudioProcessor::releaseResources() {
    // delayProcessor.reset();
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    sine.reset();
    switch (currentType) {
        case DelayType::FeedForward:
            feedForwardDelay.reset();
            break;
        case DelayType::FeedBack:
            feedbackDelay.reset();
            break;
        default:
            feedForwardDelay.reset();
            break;
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NegDelayAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void NegDelayAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Get the current delay time value
    auto *delayTime = apvts.getRawParameterValue(ids.delayTime);
    float currentDelayTime = delayTime->load();
    auto *feedbackAmount = apvts.getRawParameterValue(ids.feedbackAmount);
    auto feedback = feedbackAmount->load();

    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        auto *channelData = buffer.getWritePointer(channel);
        std::vector<float> channelVector(channelData, channelData + buffer.getNumSamples());

        // Generate test signal
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            channelVector[sample] = sine.processSineWave(sample, buffer.getNumSamples());
        }

        // Process the channel through delay
        auto processedVector = channelVector;
        /* std::atomic<float>* delayType = nullptr;
         delayType = apvts.getRawParameterValue(ids.delayType);
         currentType = static_cast<DelayType>(static_cast<int>(delayType->load()));*/
        DBG("currentType: " + std::to_string(static_cast<int>(currentType)));
        switch (currentType) {
            case DelayType::FeedForward:
                processedVector = feedForwardDelay.process(channelVector);
                if (currentDelayTimeChoice == DelayTimeChoice::TempoSync)
                    feedbackDelay.setDelayTime(tempoParams);
                else
                    feedbackDelay.setDelayTime(currentDelayTime);

                break;
            case DelayType::FeedBack:
                processedVector = feedbackDelay.process(channelVector);
                if (currentDelayTimeChoice == DelayTimeChoice::TempoSync)
                    feedbackDelay.setDelayTime(tempoParams);
                else
                    feedbackDelay.setDelayTime(currentDelayTime);
                feedbackDelay.setFeedback(feedback);

                break;
            default:
                processedVector = feedForwardDelay.process(channelVector);
                break;
        }

        // Copy processed data back to the AudioBuffer
        std::copy(processedVector.begin(),
                  processedVector.begin() + buffer.getNumSamples(),
                  channelData);
    }

    // Clear unused channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
}

//==============================================================================
bool NegDelayAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *NegDelayAudioProcessor::createEditor() {
    //return new NegDelayAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void NegDelayAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NegDelayAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the /getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor * JUCE_CALLTYPE createPluginFilter() {
    return new NegDelayAudioProcessor();
}

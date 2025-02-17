/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ParamId.h"

//==============================================================================
/**
*/
class NegDelayAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Value::Listener {
public:
    NegDelayAudioProcessorEditor(NegDelayAudioProcessor &);

    ~NegDelayAudioProcessorEditor() override;

    void paint(juce::Graphics &) override;

    void resized() override;

    void valueChanged(juce::Value &value) override {
    }

    void updateDelayParams(float bpm, float noteDivision) const {
        const DSP::DelayParams<float> newParams = {audioProcessor.delayParams.delayTime, {bpm, noteDivision}};
        audioProcessor.setDelayParams(newParams);
    }

    void updateDelayTimeType(const bool isTempoSync) const {
        const auto type = isTempoSync ? DSP::DelayTimeType::TempoSync : DSP::DelayTimeType::Seconds;
        audioProcessor.updateDelayTimeType(type);
    }

    void updateDelayType(const bool isFeedback) const {
        const auto type = isFeedback ? DSP::DelayType::FeedBack : DSP::DelayType::FeedForward;
        audioProcessor.updateDelayType(type);
    }

private:
    NegDelayAudioProcessor &audioProcessor;

    // Sliders
    juce::Slider feedbackSlider;
    juce::Slider mixSlider;
    juce::Slider delayTimeSlider;
    juce::Slider delayTempoSyncSlider;

    // Labels
    juce::Label feedbackLabel;
    juce::Label mixLabel;
    juce::Label delayTimeLabel;
    juce::Label delayTempoSyncLabel;

    std::unique_ptr<SliderAttachment> feedbackAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> delayTimeAttachment;
    std::unique_ptr<SliderAttachment> delayTempoSyncAttachment;

    // Buttons for TempoSync/Seconds
    juce::ToggleButton tempoSyncButton;
    juce::ToggleButton secondsButton;

    // Buttons for Feedback/Feedforward
    juce::ToggleButton feedbackButton;
    juce::ToggleButton feedforwardButton;

    juce::Value delayTempoSyncValue;
    juce::Value delayTypeValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NegDelayAudioProcessorEditor)
};

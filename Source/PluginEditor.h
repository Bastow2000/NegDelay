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

private:
    NegDelayAudioProcessor &audioProcessor;

    // Sliders
    juce::Slider feedbackSlider;
    juce::Slider mixSlider;
    juce::Slider delayTimeSlider;
    juce::Slider delayFeedbackSlider;

    // Labels
    juce::Label feedbackLabel;
    juce::Label mixLabel;
    juce::Label delayTimeLabel;
    juce::Label delayFeedbackLabel;

    std::unique_ptr<SliderAttachment> feedbackAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> delayTimeAttachment;
    std::unique_ptr<SliderAttachment> delayFeedbackAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NegDelayAudioProcessorEditor)
};

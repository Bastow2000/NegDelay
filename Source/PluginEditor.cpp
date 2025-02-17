/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <objc/objc.h>

//==============================================================================
NegDelayAudioProcessorEditor::NegDelayAudioProcessorEditor(NegDelayAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      tempoSyncButton("Tempo Sync"), secondsButton("Seconds"),
      feedbackButton("Feedback"), feedforwardButton("Feedforward") {
    // Set initial states
    const Parameters::Ids ids;
    auto &apvts = audioProcessor.getAPVTS();
    delayTempoSyncValue = apvts.getParameterAsValue(ids.delayTempoSync.toString());
    delayTypeValue = apvts.getParameterAsValue(ids.delayType.toString());

    auto buttonSetupTimeType = [&, this](juce::ToggleButton &button, juce::Value &value, bool isTempoSync) {
        button.setToggleState(isTempoSync
                                  ? static_cast<bool>(value.getValue())
                                  : !static_cast<bool>(value.getValue()),
                              juce::dontSendNotification);
        addAndMakeVisible(button);
        value.addListener(this);

        button.setRadioGroupId(1);
        button.onClick = [this, &value, isTempoSync] {
            value.setValue(isTempoSync);
            updateDelayTimeType(isTempoSync);
        };
    };

    auto buttonSetupDelayType = [&,this](juce::ToggleButton &button, juce::Value &value, bool isFeedforward,
                                         DSP::DelayType type) {
        button.setToggleState(static_cast<int>(value.getValue()) == static_cast<int>(type),
                              juce::dontSendNotification);

        addAndMakeVisible(button);
        value.addListener(this);

        button.setRadioGroupId(2);
        button.onClick = [this, &value, type, isFeedforward] {
            value.setValue(static_cast<int>(type));
            updateDelayType(isFeedforward);
        };
    };

    auto sliderAttachments = [&](std::unique_ptr<SliderAttachment> &sliderAttachment,
                                 NegDelayAudioProcessor &audioProcessor, const juce::Identifier &id,
                                 juce::Slider &slider) {
        sliderAttachment = std::make_unique<SliderAttachment>(
            audioProcessor.getAPVTS(), id.toString(), slider);
        addAndMakeVisible(slider);
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
    };

    auto sliderLabels = [](const juce::String &text, juce::Label &label, juce::Slider &slider) {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::topLeft);
        label.attachToComponent(&slider, false);
        label.setBounds(slider.getX(), slider.getY() - 20, slider.getWidth(), 20);
    };

    sliderLabels("Feedback", feedbackLabel, feedbackSlider);
    sliderLabels("Mix", mixLabel, mixSlider);
    sliderLabels("DelayTime", delayTimeLabel, delayTimeSlider);
    sliderLabels("DelayTempoSync", delayTempoSyncLabel, delayTempoSyncSlider);

    sliderAttachments(feedbackAttachment, audioProcessor, ids.feedbackAmount, feedbackSlider);
    sliderAttachments(mixAttachment, audioProcessor, ids.mix, mixSlider);
    sliderAttachments(delayTimeAttachment, audioProcessor, ids.delayTime, delayTimeSlider);
    sliderAttachments(delayTempoSyncAttachment, audioProcessor, ids.delayTempoSync, delayTempoSyncSlider);

    buttonSetupTimeType(secondsButton, delayTempoSyncValue, false);
    buttonSetupTimeType(tempoSyncButton, delayTempoSyncValue, true);

    buttonSetupDelayType(feedforwardButton, delayTypeValue, false, DSP::DelayType::FeedForward);
    buttonSetupDelayType(feedbackButton, delayTypeValue, true, DSP::DelayType::FeedBack);


    setSize(400, 500);
}

NegDelayAudioProcessorEditor::~NegDelayAudioProcessorEditor() {
}

//==============================================================================
void NegDelayAudioProcessorEditor::paint(juce::Graphics &g) {
    g.fillAll(NegCol::darkGrey);
}

void NegDelayAudioProcessorEditor::resized() {
    auto area = getLocalBounds().reduced(10);
    constexpr int buttonHeight = 24;
    constexpr int labelHeight = 20;

    // Delay Time Unit
    auto tempoSyncArea = area.removeFromTop(buttonHeight + labelHeight);

    auto buttonRow = tempoSyncArea.removeFromTop(buttonHeight);
    secondsButton.setBounds(buttonRow.removeFromLeft(100));
    tempoSyncButton.setBounds(buttonRow.removeFromLeft(100));

    // Delay Type
    auto delayTypeArea = area.removeFromTop(buttonHeight + labelHeight);
    buttonRow = delayTypeArea.removeFromTop(buttonHeight);
    feedforwardButton.setBounds(buttonRow.removeFromLeft(100));
    feedbackButton.setBounds(buttonRow.removeFromLeft(100));

    feedbackSlider.setBounds(area.removeFromTop(80));
    mixSlider.setBounds(area.removeFromTop(80));
    delayTimeSlider.setBounds(area.removeFromTop(80));
    delayTempoSyncSlider.setBounds(area.removeFromTop(80));

    delayTimeLabel.setBounds(delayTimeSlider.getX(), delayTimeSlider.getY(), delayTimeSlider.getWidth(), 20);
    feedbackLabel.setBounds(feedbackSlider.getX(), feedbackSlider.getY(), feedbackSlider.getWidth(), 20);
    mixLabel.setBounds(mixSlider.getX(), mixSlider.getY(), mixSlider.getWidth(), 20);
    delayTempoSyncLabel.setBounds(delayTempoSyncSlider.getX(), delayTempoSyncSlider.getY(),
                                  delayTempoSyncSlider.getWidth(), 20);
}

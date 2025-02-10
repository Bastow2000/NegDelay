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
    : AudioProcessorEditor(&p), audioProcessor(p) {
    addAndMakeVisible(feedbackSlider);
    addAndMakeVisible(mixSlider);
    addAndMakeVisible(delayTimeSlider);

    feedbackSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    delayTimeSlider.setSliderStyle(juce::Slider::LinearHorizontal);

    feedbackSlider.setRange(0.0f, 1.0f, 0.01f);
    mixSlider.setRange(0.0f, 1.0f, 0.01f);
    delayTimeSlider.setRange(0.0f, 1.0f, 0.01f);

    auto sliderAttachments = [](std::unique_ptr<SliderAttachment> &sliderAttachment,
                                NegDelayAudioProcessor &audioProcessor, const juce::Identifier &id,
                                juce::Slider &slider) {
        sliderAttachment = std::make_unique<SliderAttachment>(
            audioProcessor.getAPVTS(), id.toString(), slider);
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

    Parameters::Ids ids;
    sliderAttachments(feedbackAttachment, audioProcessor, ids.feedbackAmount, feedbackSlider);
    sliderAttachments(mixAttachment, audioProcessor, ids.mix, mixSlider);
    sliderAttachments(delayTimeAttachment, audioProcessor, ids.delayTime, delayTimeSlider);


    setSize(400, 500);
}

NegDelayAudioProcessorEditor::~NegDelayAudioProcessorEditor() {
}

//==============================================================================
void NegDelayAudioProcessorEditor::paint(juce::Graphics &g) {
    g.fillAll(NegCol::darkGrey);
}

void NegDelayAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds();
    auto sliderHeight = bounds.getHeight() / 4;


    delayTimeSlider.setBounds(bounds.removeFromTop(sliderHeight).reduced(10));
    feedbackSlider.setBounds(bounds.removeFromTop(sliderHeight).reduced(10));
    mixSlider.setBounds(bounds.removeFromTop(sliderHeight).reduced(10));

    delayTimeLabel.setBounds(delayTimeSlider.getX(), delayTimeSlider.getY() + 20, delayTimeSlider.getWidth(), 20);
    feedbackLabel.setBounds(feedbackSlider.getX(), feedbackSlider.getY() + 20, feedbackSlider.getWidth(), 20);
    mixLabel.setBounds(mixSlider.getX(), mixSlider.getY() + 20, mixSlider.getWidth(), 20);
}

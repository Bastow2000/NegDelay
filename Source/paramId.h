#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

using APVTS = juce::AudioProcessorValueTreeState;
namespace Parameters
{

	APVTS::ParameterLayout createParameterLayout();

struct Ids
{
    Ids();
    
#define MAKE_ID(name) const juce::Identifier name = #name;
	MAKE_ID (delayTime)
	MAKE_ID (feedbackAmount)
#undef MAKE_ID

Ids& operator= (const Ids&) = delete;
};
}
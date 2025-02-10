#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

using APVTS = juce::AudioProcessorValueTreeState;

namespace Parameters {
	APVTS::ParameterLayout createParameterLayout();

	struct Ids {
		Ids();

#define MAKE_ID(name) const juce::Identifier name = #name;
		MAKE_ID(delayTime)
		MAKE_ID(mix)
		MAKE_ID(feedbackAmount)
		MAKE_ID(delayType)
#undef MAKE_ID

		Ids &operator=(const Ids &) = delete;
	};
}


enum class DelayType {
	FeedForward,
	FeedBack,
	MultiTap /*,
	TempoSync,
	MultiTapDelay,
	StereoDelay,
	ImpulseDelay,
	ConvolutionDelay*/
};

enum class DelayTimeChoice {
	TempoSync,
	Normal /*,
	TempoSync,
	MultiTapDelay,
	StereoDelay,
	ImpulseDelay,
	ConvolutionDelay*/
};

enum class DelayMode {
	Normal,
	MultiTap
};

enum class DelayTimeType {
	Normal,
	TempoSync
};


inline auto currentType{DelayType::FeedBack};
static constexpr size_t maxDelay{240000};

namespace NegCol {
	const juce::Colour darkGrey = juce::Colour::fromFloatRGBA(0.27f, 0.27f, 0.27f, 1.0f);
}

using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

//inline DelayType currentType = DelayType::FeedBack;

//inline DelayTimeChoice currentDelayTimeChoice = DelayTimeChoice::TempoSync;

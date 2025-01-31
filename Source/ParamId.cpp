#include "ParamId.h"

namespace Parameters {
    Ids::Ids() = default;

    APVTS::ParameterLayout createParameterLayout() {
        const Ids ids;
        std::vector<std::unique_ptr<juce::RangedAudioParameter> > params;
        params.reserve(2);
        static constexpr int versionHint = 1;

        auto addFloatParam = [&](const juce::Identifier &id, const juce::String &description,
                                 const juce::NormalisableRange<float> &rng, float defaultValue) {
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID{id.toString(), versionHint}, description, rng, defaultValue));
        };

        auto addBoolParam = [&](const juce::Identifier &id, const juce::String &description, bool defaultValue) {
            params.push_back(
                std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id.toString(), versionHint}, description,
                                                           defaultValue));
        };

        auto addIntParam = [&](const juce::Identifier &id, const juce::String &description,
                               juce::NormalisableRange<int> &rng, int defaultValue) {
            params.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{id.toString(), versionHint},
                                                                       description, static_cast<int>(rng.start),
                                                                       static_cast<int>(rng.end), defaultValue,
                                                                       juce::String()));
        };

        juce::NormalisableRange<int> delayTypeRange{
            static_cast<int>(DelayType::FeedForward),
            static_cast<int>(DelayType::FeedBack)
        };

        addFloatParam(ids.delayTime, "DelayTime", juce::NormalisableRange<float>{0.0f, 1.0f}, 0.5f);
        addFloatParam(ids.feedbackAmount, "FeedbackAmount", juce::NormalisableRange<float>{0.0f, 0.5f}, 0.05f);
        addIntParam(ids.delayType, "DelayType", delayTypeRange, static_cast<int>(DelayType::FeedForward));


        return {params.begin(), params.end()};
    }
}

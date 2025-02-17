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

        auto addIntFuncParam = [&](const juce::Identifier &id, const juce::String &description,
                                   juce::NormalisableRange<int> &rng, int defaultValue,
                                   std::function<juce::String(int, int)> stringFromValue = nullptr) {
            params.push_back(std::make_unique<juce::AudioParameterInt>(
                juce::ParameterID{id.toString(), versionHint},
                description,
                static_cast<int>(rng.start),
                static_cast<int>(rng.end),
                defaultValue,
                juce::String(),
                stringFromValue
            ));
        };

        juce::NormalisableRange<int> delayTypeRange{
            static_cast<int>(DSP::DelayType::FeedForward),
            static_cast<int>(DSP::DelayType::FeedBack)
        };

        juce::NormalisableRange<int> delayTimeTypeRange{
            static_cast<int>(DSP::DelayTimeType::Seconds),
            static_cast<int>(DSP::DelayTimeType::TempoSync)
        };

        addFloatParam(ids.delayTime, "DelayTime", juce::NormalisableRange<float>{0.0f, 1.0f}, 0.5f);
        addFloatParam(ids.feedbackAmount, "FeedbackAmount", juce::NormalisableRange<float>{0.0f, 0.5f}, 0.05f);
        addFloatParam(ids.mix, "mix", juce::NormalisableRange<float>{0.0f, 1.0f}, 0.05f);

        addIntParam(ids.delayType, "DelayType", delayTypeRange, static_cast<int>(DSP::DelayType::FeedForward));
        addIntParam(ids.delayTypeTime, "DelayTypeTime", delayTimeTypeRange,
                    static_cast<int>(DSP::DelayTimeType::Seconds));


        // Define the range for tempo sync divisions (0-5)
        juce::NormalisableRange<int> tempoSyncRange(0, 5);

        // Add the tempo sync parameter with custom string conversion
        addIntFuncParam(ids.delayTempoSync, "Tempo Sync", tempoSyncRange, 2, // Default to 1/4 note
                        [](const int value, int) -> juce::String {
                            const char *divisions[] = {"1", "1/2", "1/4", "1/8", "1/16", "1/32"};
                            return divisions[value];
                        }
        );


        return {params.begin(), params.end()};
    }
}

#pragma once

#include <array>
#include <cmath>
#include <vector>
#include <JuceHeader.h>
#include "ParamId.h"


namespace DSP {
    //--------- Delay Modifications -------

    enum class DelayType {
        FeedForward,
        FeedBack
    };

    enum class DelayTimeType {
        Seconds,
        TempoSync
    };

    //--------- Interpolation -------
    template<typename T>
    struct linearInterpolation {
        static T interpolation(T valueOne, T valueTwo, double fraction) {
            // Linear interpolation formula
            // y[n] = x[0] + (frac * (x[1] - x[0])
            return valueOne + static_cast<T>(fraction * (valueTwo - valueOne));
        }

        T process(const size_t current /* index */, const size_t bufferSize, const std::vector<T> &buffer,
                  const double fraction) {
            size_t next = (current + 1) & (bufferSize - 1);
            return interpolation(buffer[current], buffer[next], fraction);
        }
    };

    template<typename T>
    struct cubicInterpolation {
        // Cubic interpolation formula
        // y[n] = x[1] + 0.5 * frac * (x[2] - x[0] + frac * (2.0 * x[0] - 5.0 * x[1] + 4.0 * x[2] - x[3] + frac * (3.0 * (x[1] - x[2]) + x[3] - x[0])))
        static T formula(T valueOne, T valueTwo, T valueThree, T valueFour, double fraction) {
            // first Term: (3.0 * (x1 - x2) + x3 - x0)
            T firstTerm = (3.0 * (valueTwo - valueThree) + valueFour - valueOne);

            // Second term: (2.0 * x0 - 5.0 * x1 + 4.0 * x2 - x3 + frac * firstTerm)
            T secondTerm = 2.0 * valueOne - 5.0 * valueTwo + 4.0 * valueThree - valueFour + fraction * firstTerm;

            // third term: (x2 - x0 + frac * secondTerm)
            T thirdTerm = valueThree - valueOne + fraction * secondTerm;

            // Final calculation: x1 + 0.5 * frac * outerTerm
            return valueTwo + 0.5 * fraction * thirdTerm;
        }

        T process(size_t current /*current Index */, const size_t bufferSize, const std::vector<T> &buffer,
                  const double fraction) {
            size_t prev = (current == 0) ? bufferSize - 1 : current - 1;
            size_t next = (current + 1) & (bufferSize - 1);
            size_t future = (next + 1) & (bufferSize - 1);
            return formula(buffer[prev], buffer[current], buffer[next], buffer[future], fraction);
        }
    };

    //--------- DC Filtering -------
    template<typename T>
    struct DCFiltering {
        T output{};
        T input{};
        T delayedInput{};
        T coeff{0.999};

        T process(T newCurrentValue) {
            input = newCurrentValue;

            // DC Filtering formula
            // y[n] = x[n] - x[n-1] + R * y[n-1]
            output = input - delayedInput + coeff * output;
            delayedInput = input;

            return output;
        }

        void setCoefficient(const T newCoeff) {
            coeff = newCoeff;
        }

        T cascadedProcess(T value, T filterOrder) {
            // Cascade DC filtering
            for (int i = 0; i < filterOrder; ++i) {
                value = process(value);
            }
            return value;
        }

        void reset() {
            input = T{};
            output = T{};
            delayedInput = T{};
        }
    };

    //--------- Smoothing -------

    template<typename T>
    struct EMASmoothing {
        T output{};
        T input{};
        T smoothingFactor{0.001};

        T process(T newInput) {
            input = newInput;

            // Exponential Moving Average (EMA) formula
            // y[n] = y[n-1] + alpha * (x[n] - y[n-1])
            output += smoothingFactor * (input - output);
            return output;
        }

        void reset() {
            output = input;
        }
    };

    //--------- Delay Implementation -------

    template<typename T>
    struct DelayParams {
        T delayTime{};
        std::pair<T, T> bpmAndNoteDivision;
    };

    template<typename T>
    struct Delay {
        size_t delaySamples{};
        size_t writePos{};
        size_t readPos{};
        size_t bufferMask{};
        size_t bufferSize{};

        double sampleRate{};
        T feedback{0.01};
        T mix{0.5};

        std::vector<T> buffer{};
        DelayType delayType{DelayType::FeedForward};
        DelayTimeType timeType{DelayTimeType::TempoSync};

        EMASmoothing<T> EMASmoothing{};
        linearInterpolation<T> linearInterpolation{};
        DCFiltering<T> dcFiltering{};

        DelayParams<T> params;

        void setup(const double newSampleRate, const DelayParams<T> &delayParams) {
            sampleRate = newSampleRate;


            params = delayParams;

            delaySamples = setDelaySamples(params);

            // Resize buffer with samples
            bufferSize = 4 << static_cast<size_t>(std::ceil(std::log2(delaySamples)));
            bufferMask = bufferSize - 1;
            buffer.resize(bufferSize);

            // fill buffer with zeros
            std::fill(buffer.begin(), buffer.end(), T(0));

            setDelayTime(delayParams);
        }

        void setTimeType(const DelayTimeType type) {
            timeType = type;
        }

        void setDelayType(const DelayType type) {
            delayType = type;
        }

        size_t setDelaySamples(const DelayParams<T> &delayParams) {
            // sec * Fs = Samples
            // Calculates maximum delay length in samples

            auto [bpm, division] = delayParams.bpmAndNoteDivision;
            float noteDuration = 4.0f / std::pow(2.0f, division);

            if (timeType == DelayTimeType::Seconds) {
                return static_cast<size_t>(delayParams.delayTime * sampleRate);
            } else {
                return static_cast<size_t>((60.0 / bpm) * noteDuration * sampleRate);
            }
        }

        void setDelayTime(const DelayParams<T> &delayParams) {
            delaySamples = setDelaySamples(delayParams);
            delaySamples = std::min(delaySamples, bufferSize);
            // smooth delay
            T smoothedDelay = EMASmoothing.process(static_cast<T>(delaySamples));
        }

        void reset() {
            writePos = 0;
            readPos = 0;
            std::fill(buffer.begin(), buffer.end(), T(0));
            EMASmoothing.reset();
            dcFiltering.reset();
        }

        void setFeedback(T feedbackAmount) {
            feedback = std::clamp(feedbackAmount, T(0), T(1));
        }

        void setMix(T mixAmount) {
            mix = std::clamp(mixAmount, T(0), T(1));
        }

        std::vector<T> process(const std::vector<T> &input) {
            std::vector<T> output(input.size());

            for (size_t i = 0; i < input.size(); ++i) {
                // Prevent overflow
                writePos = std::min(writePos, buffer.size() - 1);

                // Write input to buffer
                buffer[writePos] = input[i];

                // Dry signal at 0
                if (delaySamples == 0) {
                    std::copy(input.begin(), input.end(), output.begin());
                    return output;
                }


                // Increment write position and wrap if needed using multiplication
                // With this code feedforward works
                // Y[n] = x[n] + x[n-d]
                if (delayType == DelayType::FeedForward)
                    writePos = (writePos + 1) & bufferMask;

                const double smoothedDelayTime = EMASmoothing.process(delaySamples);

                // Calculate read position based on current delay time
                readPos = (writePos - static_cast<size_t>(smoothedDelayTime) + buffer.size());
                readPos %= buffer.size();

                const auto readIndex = static_cast<size_t>(readPos);
                const double frac = readPos - readIndex;

                T delayedSample = linearInterpolation.process(readIndex, bufferSize, buffer, frac);

                T filteredDelay = dcFiltering.cascadedProcess(delayedSample, 3);
                std::cout << "DelayType = " << static_cast<int>(delayType) << std::endl;


                if (delayType == DelayType::FeedForward) {
                    buffer[writePos];
                } else {
                    buffer[writePos] += feedback * filteredDelay;
                }

                output[i] = (1 - mix) * input[i] + mix * delayedSample;

                // With this code feedback works
                // Y[n] = x[n] + f * y[n-d]
                if (delayType == DelayType::FeedBack)
                    writePos = (writePos + 1) & bufferMask;
            }

            return output;
        }
    };

    template<typename T>
    struct SineWaveGeneration {
        // Continuous
        // y = A * sin(2πft + φ)
        // Digital

        float frequency{459.375};
        float amplitude{0.01f};
        double sampleRate_{};
        float phase{0.0f};
        float phaseIncrement{0.0f};
        size_t silencePerSec{};
        size_t noisePerSec{};
        size_t totalCycle{};
        size_t sampleCounter{0};

        // 96 Samples * 455 = 44160 Which is one full cycle
        // Has clicks in could use interpolation not important only a test signal
        void setup(const double sampleRate, size_t silence) {
            sampleRate_ = sampleRate;

            // Number of samples in 5 seconds
            silencePerSec = static_cast<size_t>(sampleRate) * silence;

            // Number of samples in 1 second
            noisePerSec = static_cast<size_t>(sampleRate);

            // Total samples in one complete cycle
            totalCycle = (silence + 1) * static_cast<size_t>(sampleRate);
        }

        T processSineWave(T n, T bufferSize) {
            // Sample Counter Reaches total Cycle reset
            const size_t currentPosition = sampleCounter % totalCycle;
            sampleCounter++;

            if (currentPosition < silencePerSec) {
                return static_cast<T>(0);
            } else {
                // Increments phase
                phaseIncrement = 2 * M_PI * frequency / sampleRate_;
                phase += phaseIncrement;

                // Phase reaches 2 Pi reset cycle
                if (phase > M_PI * 2)
                    phase -= M_PI * 2;

                return static_cast<T>(amplitude * std::sin(phase));
            }
        }

        void reset() {
            phase = 0.0f;
            sampleCounter = 0;
        }
    };
}

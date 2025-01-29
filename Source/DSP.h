#pragma once

#include <array>
#include <cmath>
#include <functional>
#include <unistd.h>
#include <variant>
#include <vector>
#include <JuceHeader.h>


    template <typename T>
   struct linearInterpolation
    {
        static T interpolation(T valueOne, T valueTwo, double fraction)
        {
            // Linear interpolation formula
            // y[n] = x[0] + (frac * (x[1] - x[0])
            return valueOne + static_cast<T>(fraction * (valueTwo - valueOne));
        }

        T process(const size_t index, const std::vector<T>& buffer, const double fraction)
        {
            size_t nextIndex = (index + 1) % buffer.size();
            return interpolation(buffer[index], buffer[nextIndex], fraction);
        }

    };

    template <typename  T>
    struct cubicInterpolation
    {
        // Cubic interpolation formula
        // y[n] = x[1] + 0.5 * frac * (x[2] - x[0] + frac * (2.0 * x[0] - 5.0 * x[1] + 4.0 * x[2] - x[3] + frac * (3.0 * (x[1] - x[2]) + x[3] - x[0])))
        static T formula(T valueOne, T valueTwo, T valueThree, T valueFour, double fraction)
        {
            // first Term: (3.0 * (x1 - x2) + x3 - x0)
            T firstTerm =  (3.0 * (valueTwo - valueThree) + valueFour - valueOne);

            // Second term: (2.0 * x0 - 5.0 * x1 + 4.0 * x2 - x3 + frac * firstTerm)
            T secondTerm = 2.0 * valueOne - 5.0 * valueTwo + 4.0 * valueThree - valueFour + fraction * firstTerm;

            // third term: (x2 - x0 + frac * secondTerm)
            T thirdTerm = valueThree - valueOne + fraction * secondTerm;

            // Final calculation: x1 + 0.5 * frac * outerTerm
            return valueTwo + 0.5 * fraction * thirdTerm;
        }

        T process(size_t current /*current Index */, const std::vector<T>& buffer, const double fraction)
        {
            size_t prev = (current == 0) ? buffer.size() - 1 : current - 1;
            size_t next = (current + 1) % buffer.size();
            size_t future = (next + 1) % buffer.size();
            return formula(buffer[prev], buffer[current], buffer[next], buffer[future], fraction);
        }
    };

    template <typename T>
    struct DCFiltering
    {
        T value {};
        T currentValue {};
        T prevValue {};
        T memory {0.999};

        T process(T newCurrentValue)
        {
            currentValue = newCurrentValue;

            // DC Filtering formula
            // y[n] = x[n] - x[n-1] + R * y[n-1]
            value = currentValue - prevValue + memory * value;
            prevValue = currentValue;

            return value;
        }

        void reset()
        {
            currentValue = T{};
            value = T{};
            prevValue = T{};
        }
    };

    template <typename T>
    struct EMASmoothing
    {
        T value {};
        T currentValue {};
        T smoothingFactor {0.001};

        T process(T newCurrentValue)
        {
            currentValue = newCurrentValue;

            // Exponential Moving Average (EMA) formula
            // y[n] = y[n-1] + alpha * (x[n] - y[n-1])
            value += smoothingFactor * (currentValue - value);
            return value;
        }

        void reset()
        {
            value = currentValue;
        }
    };

    template <typename T>
    struct TrendFilter
    {
        T level {};
        T trend {};
        T levelRate {0.1};
        T trendRate {0.09};

        T process(T newValue)
        {
            if (level == 0)
            {
                level = newValue;
                trend = 0;
                return level;
            }

            T prevLevel = level;
            // Trend Filter
            // where y[n] is the smoothed value, x[n] is the new value, and b[n] is the trend
            // y[n] = alpha * x[n] + (1 - alpha) * (y[n-1] + b[n-1])
            level = levelRate * newValue + (1 - levelRate) * (level + trend);

            // b[n] = beta * (y[n] - y[n-1]) + (1 - beta) * b[n-1]
            trend = trendRate * (level - prevLevel) + (1 - trendRate) * trend;

            return level + trend;
        }

        void reset()
        {
            level = 0;
            trend = 0;
        }
    };


    template <typename T>
    struct FeedforwardDelay
    {
        size_t maxDelaySamples{};
        size_t currentDelaySamples{};
        size_t writePos {};
        size_t readPos {};

        double sampleRate{};
        T mix {0.15};

        std::vector<T> buffer{};

        TrendFilter<T> trendSmoothingFilter{};
        EMASmoothing<T> EMASmoothing{};
        cubicInterpolation<T> cubicInterpolation{};
        DCFiltering<T> dcFiltering{};

        T tanhSaturation(T input, T drive = T(10.0))
        {
            return std::tanh(input * drive);
        }

        void setup(const double newSampleRate, T delayLength)
        {
            sampleRate = newSampleRate;
            maxDelaySamples = static_cast<size_t>(delayLength * sampleRate);


            buffer.resize(maxDelaySamples + 1);
            std::fill(buffer.begin(), buffer.end(), T(0));
        }

        void setDelayTime(T delayTime)
        {

            currentDelaySamples = static_cast<size_t>(delayTime * maxDelaySamples);

            const auto minDelaySamples = static_cast<size_t>(0.007 * maxDelaySamples);

            T smoothedDelay = trendSmoothingFilter.process(static_cast<T>(currentDelaySamples));

            currentDelaySamples = std::max(minDelaySamples, std::min(static_cast<size_t>(smoothedDelay), maxDelaySamples));


        }

        void reset()
        {
            writePos = 0;
            readPos = 0;
            std::fill(buffer.begin(), buffer.end(), T(0));
            trendSmoothingFilter.reset();
        }

        std::vector<T> process(const std::vector<T>& input)
        {
            std::vector<T> output;
            output.reserve(input.size());

            for (size_t i = 0; i < input.size(); ++i)
            {
                // Prevent overflow
                writePos = std::min (writePos, buffer.size() - 1);

                // Write input to buffer
                buffer[writePos] = input[i];

                // Alternative bufferSize power of 2 only
                //writePos = (writePos + 1) & bufferMask;

                // Increment write position and wrap if needed using multiplication
               // writePos = (writePos + 1) % buffer.size();

                const double smoothedDelayTime = trendSmoothingFilter.process(currentDelaySamples);

                // Calculate read position based on current delay time
                readPos = writePos + (static_cast<size_t>(smoothedDelayTime) > writePos) * buffer.size() - smoothedDelayTime;

                const auto readIndex = static_cast<size_t>(readPos);
                const double frac = readPos - readIndex;

                T delayedSample = cubicInterpolation.process(readIndex, buffer, frac);
                // Read delayed sample

                // Apply DC filtering to the delayed signal
                T filteredDelay = dcFiltering.process(delayedSample);

                T saturation = tanhSaturation(filteredDelay);

                buffer[writePos] = input[i] + 0.09 *  saturation;

                // Mix dry and wet signals and write to output
                output[i] = (1 - mix) * input[i] + mix * saturation;

                writePos = (writePos + 1) % buffer.size();
            }

            return output;
        }
    };

    template <typename T>
    struct SineWaveGeneration
    {
        // Continuous
        // y = A * sin(2πft + φ)
        // Digital

        float frequency {459.375};
        float amplitude {0.01f};
        double sampleRate_ {};
        float phase {0.0f};
        float phaseIncrement {0.0f};
        size_t silencePerSec{};
        size_t noisePerSec{};
        size_t totalCycle{};
        size_t sampleCounter {0};

        // 96 Samples * 455 = 44160 Which is one full cycle
        // Has clicks in could use interpolation not important only a test signal
        void setup (const double sampleRate,  size_t silence )
        {
            sampleRate_ = sampleRate;

            // Number of samples in 5 seconds
            silencePerSec = static_cast<size_t>(sampleRate) * silence;

            // Number of samples in 1 second
            noisePerSec = static_cast<size_t>(sampleRate);

            // Total samples in one complete cycle
            totalCycle = (silence + 1) * static_cast<size_t>(sampleRate);
        }

        T processSineWave(T n, T bufferSize)
        {
            // When Sample Counter Reaches total Cycle reset
            size_t currentPosition = sampleCounter % totalCycle;
            sampleCounter++;

            //DBG("currentPosition: " + std::to_string(currentPosition));

            if (currentPosition < silencePerSec)
            {
                return static_cast<T>(0);
            }
            else
            {
                // Increments phase
                phaseIncrement = 2 * M_PI * frequency / sampleRate_;
                phase += phaseIncrement;
                // Wraps phase
                if (phase > M_PI * 2)
                    phase -= M_PI * 2;

                return static_cast<T>(amplitude * std::sin(phase));
            }

        }

        void reset()
        {
            phase = 0.0f;
            sampleCounter = 0;
        }
    };

}

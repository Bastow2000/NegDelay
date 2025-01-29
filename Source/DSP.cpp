#include "DSP.h"
 template <typename T>
struct LinearInterpolation {
    T process(T x0, T x1, double frac) {
        return x0 + static_cast<T>(frac * (x1 - x0));
    }
};

template <typename T>
struct Smoothing {
    T current{};
    T target{};
    T smoothingFactor{0.001};

    T process(T newTarget) {
        target = newTarget;
        current += smoothingFactor * (target - current);
        return current;
    }

    void reset() { current = target; }
};

template <typename T>
struct Delay {
    std::vector<T> buffer{};
    size_t writePosition{};
    T feedback{0.5};
    Smoothing<double> smoothing{};
    LinearInterpolation<T> interpolation{};
    double sampleRate{44100.0};

    void setup(double newSampleRate)
    {
        sampleRate = newSampleRate;
        // Allocate buffer for maximum delay time (e.g., 2 seconds)
        buffer.resize(static_cast<size_t>(sampleRate * 10.0));
        std::fill(buffer.begin(), buffer.end(), T{});
    }

    std::vector<T> process(const std::vector<T>& signal, double delayTimeSeconds) {
        double delayTimeSamples = delayTimeSeconds * sampleRate;
        std::vector<T> result(signal.size());

        for (size_t i = 0; i < signal.size(); ++i) {
            // Calculate read position first
            double smoothedDelay = smoothing.process(delayTimeSamples);
            double readPos = writePosition - smoothedDelay;
            if (readPos < 0) readPos += buffer.size();

            size_t readIndex = static_cast<size_t>(readPos);
            double frac = readPos - readIndex;

            // Get delayed signal
            size_t nextIndex = (readIndex + 1) % buffer.size();
            T delayedSignal = interpolation.process(buffer[readIndex], buffer[nextIndex], frac);

            // Write new value with feedback
            buffer[writePosition] = signal[i] + feedback * delayedSignal;

            // Output is input plus delayed signal
            result[i] = signal[i] + delayedSignal;

            writePosition = (writePosition + 1) % buffer.size();
        }
        return result;
    }



    void reset() {
        std::fill(buffer.begin(), buffer.end(), T{});
        smoothing.reset();
    }
};

    template <typename T>
    struct TestSignal {
        double phase {0.0};
        double sampleRate {44100.0};
        double frequency {440.0};
        double periodDuration {1.25}; // Duration in seconds
        size_t silenceSamples {static_cast<size_t>(sampleRate * periodDuration)}; // Samples of silence
        size_t currentSample {0};

        auto generateSineWave(size_t numSamples)
        {
            std::vector<T> signal(numSamples);
            const double phaseIncrement = 2.0 * M_PI * frequency / sampleRate;

            for (size_t i = 0; i < numSamples; ++i) {
                // Check if we're in the active or silent period
                if (currentSample < silenceSamples) {
                    signal[i] = static_cast<T>(std::sin(phase));
                    phase += phaseIncrement;
                    if (phase >= 2.0 * M_PI)
                        phase -= 2.0 * M_PI;
                } else {
                    signal[i] = 0;
                }

                // Update and wrap the current sample counter
                currentSample++;
                if (currentSample >= (silenceSamples * 2)) { // Double the period for silence
                    currentSample = 0;
                }
            }

            return signal;
        }

        auto reset()
        {
            phase = 0.0;
            currentSample = 0;
        }
    };

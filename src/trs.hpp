#include "plugin.hpp"
#include "ui.hpp"

using simd::float_4;

struct StereoInHandler {
	Input * input;
	void configure(Input * thisInput) {
		input = thisInput;
		input->setChannels(16);
	}

	float_4 getLeft(int polySection) {
		polySection &= 1;
		return input->getVoltageSimd<float_4>(polySection * 4);
	}

	float_4 getRight(int polySection) {
		polySection &= 1;
		return input->getVoltageSimd<float_4>(8 + polySection * 4);
	}

	float getLeft(void) {
		return input->getVoltage(0);
	}

	float getRight(void) {
		return input->getVoltage(8);
	}

};

struct StereoOutHandler {
	
	Output * output;
	
	void configure(Output * thisOutput) {
		output = thisOutput;
		output->setChannels(16);
	}

	void setLeft(float_4 value, int polySection) {
		polySection &= 1;
		return output->setVoltageSimd<float_4>(value, polySection * 4);
	}

	void setRight(float_4 value, int polySection) {
		polySection &= 1;
		return output->setVoltageSimd<float_4>(value, 8 + polySection * 4);
	}

	void setLeft(float value) {
		return output->setVoltage(value, 0);
	}

	void setRight(float value) {
		return output->setVoltage(value, 8);
	}

};

////// DSP Resources

// One pole allpass

template <typename T = float>
struct AP1 {

    AP1() {}

    T process(T input) {
        T out1 = (input - d2) * a0 + d1;
        d1 = input;
        d2 = out1;
        return out1;
    }

    T d1 = T(0);
    T d2 = T(0);

    T a0 = T(0);

};

// Chamberlin SVF

// needs the diodes
template <typename T = float>
struct JOSSVF {

    T delay1 = T(0);
    T delay2 = T(0);

    T hpOut = T(0);
    T bpOut = T(0);
    T lpOut = T(0);

    JOSSVF() {};

    inline void process(float cutoff, float res, T input, T linFM, T expoFM, T resCV) {

        // Using notation from paper, sampling interval in seconds
        T sampleTime = APP->engine->getSampleTime();

        // absolute target cutoff frequency
        T resFreq = 30.0 * pow(2.0, cutoff * 12.0);
        resFreq *= dsp::approxExp2_taylor5(expoFM + T(5.f)) / T(1024.f);

        linFM += 5.0;

        resFreq *= clamp(linFM, 0.1f, 10.f)/T(5.f);

        // T flip = sgn(linFM);

        // normalized target cutoff frequency
        // also using notation from paper
        T w = sin(resFreq * sampleTime * 2 * M_PI);
        // float w = 0.1;

        w = clamp(w, 0.f, 1.f);

        // resonance, given in paper as sqrt(2)
        T q = res * 2.0;
        q = 2.0 - q;

        // T q = T(1.f);

        T highpass = input/T(5.f) - delay2 - delay1 * q;
        T bandpass = highpass * w + delay1;
        T lowpass = bandpass * w + delay2;

        delay1 = bandpass;
        delay2 = lowpass;

        lpOut = lowpass * T(5.f);
        bpOut = bandpass * T(5.f);
        hpOut = highpass * T(5.f);

    }

};


/// Delay Line

template <typename T = float_4>
struct Delay {

	T * buffer;
	uint32_t length;
	uint32_t writeIndex = 0;
	// in ms
	float sampleTime = (1.f/48000.f) * 1000.f;

	void write(T input) {
		buffer[writeIndex] = input;
		writeIndex ++;
		writeIndex -= (writeIndex == length) * length;
	}

	T readLinear(float ms) {
		float normalized = ms/sampleTime;
		int32_t sampleOffset = (int32_t) (normalized);
		T frac = T(normalized - float(sampleOffset));
		int32_t nextSample = writeIndex - sampleOffset;
		int32_t previousSample = nextSample - 1;
		nextSample += (nextSample < 0) * length;
		previousSample += (previousSample < 0) * length;

		return buffer[nextSample] + (buffer[previousSample] - buffer[nextSample]) * frac;

	}

	void changeSR(float sr) {
		sampleTime = (1.f/sr) * 1000.f;
	}

	void init(int32_t maxLength) {
		buffer = (T*) malloc(maxLength * sizeof(T));
		length = maxLength;
	}

};

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
	T differentiator = T(0.f);
	T leakyIntegrator = T(0.f);

	void writeDCBlock(T input) {
		leakyIntegrator = input - differentiator + 0.99f * leakyIntegrator;
		buffer[writeIndex] = leakyIntegrator;
		differentiator = input;
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

// from DAFX '18 Holters and Parker "Combined Model for a Bucket Brigade Device and its Input and Output Filters"

template <typename T = float_4, int32_t SIZE = 512>
struct BBD {

	/////////////// Filter stuff 

	//
	// coefficient precalculation helpers
	//

	float complexMagnitude(float r, float i) {
		return sqrt(r * r + i * i);
	}
	float complexAngle(float r, float i) {
		return atan2(r, i);
	} 
	void transform(float r, float i, float * rOut, float * iOut, float ts) {
		float realPart = exp(r * ts);
		float complexPart = realPart * sin(i * ts);
		*rOut = realPart * cos(i * ts);
		*iOut = complexPart;
	}

	void divideZ(float rNum, float iNum, float rDem, float iDem, float * rOut, float * iOut) {
		float denominator = rDem * rDem + iDem * iDem; 
		*rOut = (rNum * rDem + iNum * iDem)/denominator;
		*iOut = (iNum * rDem - rNum * iDem)/denominator;
	}

	//
	// input filter
	//

	int numRealIn;
	T * realStatesIn;
	T * realPolesIn;
	T * realResiduesIn;

	int numConjIn;
	T * zStates1In;
	T * zStates2In;
	T * zA0In;
	T * zA1In;
	T * zpArgIn;
	T * zpAbsIn;
	T * zrArgIn;
	T * zBetasIn;

	//
	// input filter
	//

	void initInputFilter(int realSections, float * realResidues, float * realPoles,
							int conjSections, float * conjRResidues, float * conjIResidues, 
								float * conjRPoles, float * conjIPoles) {

		numRealIn = realSections;
		numConjIn = conjSections;

		realStatesIn = (T*) malloc(realSections * sizeof(T));
		realPolesIn = (T*) malloc(realSections * sizeof(T));
		realResiduesIn = (T*) malloc(realSections * sizeof(T));

		for (int i = 0; i < realSections; i++) {
			realStatesIn[i] = T(0);
			realPolesIn[i] = T(exp(realPoles[i] * hostSampleTime));
			realResiduesIn[i] = T(realResidues[i]);
		}

		zStates1In = (T*) malloc(conjSections * sizeof(T));
		zStates2In = (T*) malloc(conjSections * sizeof(T));
		zA0In = (T*) malloc(conjSections * sizeof(T));
		zA1In = (T*) malloc(conjSections * sizeof(T));
		zpArgIn = (T*) malloc(conjSections * sizeof(T));
		zpAbsIn = (T*) malloc(conjSections * sizeof(T));
		zrArgIn = (T*) malloc(conjSections * sizeof(T));
		zBetasIn = (T*) malloc(conjSections * sizeof(T));


		for (int i = 0; i < conjSections; i++) {

			float laplacePoleR;
			float laplacePoleI;

			transform(conjRPoles[i], conjIPoles[i], &laplacePoleR, &laplacePoleI, hostSampleTime);
			float pAbs = complexMagnitude(laplacePoleR, laplacePoleI);
			float pArg = complexAngle(laplacePoleR, laplacePoleI);

			zStates1In[i] = T(0);
			zStates2In[i] = T(0);
			zA0In[i] = T(2 * cos(pArg));
			zA1In[i] = T(-pAbs * pAbs);
			zpArgIn[i] = T(pArg);
			zpAbsIn[i] = T(pAbs);
			zrArgIn[i] = T(complexAngle(conjRResidues[i], conjIResidues[i]));
			zBetasIn[i] = T(2 * hostSampleTime * complexMagnitude(conjRResidues[i], conjIResidues[i]));

		}

	}

	void updateInputStateR(T input, int sectionIndex) {

		realStatesIn[sectionIndex] = realPolesIn[sectionIndex] * realStatesIn[sectionIndex] + input;

	}

	void updateInputStateZ(T input, int sectionIndex) {

		T state1 = zStates1In[sectionIndex];
		T state2 = zStates1In[sectionIndex];
		T x1 = input;
		x1 += state1 * zA0In[sectionIndex];
		x1 += state2 * zA1In[sectionIndex];
		zStates2In[sectionIndex] = state1;
		zStates1In[sectionIndex] = state2;

	}

	T calculateInputWeightR(float delay, int sectionIndex) {

		return T(hostSampleTime * realResiduesIn[sectionIndex] * pow(realPolesIn[sectionIndex], delay));

	}

	T calculateInputWeightB0(float delay, int sectionIndex) {

		return zBetasIn[sectionIndex] * pow(zpAbsIn[sectionIndex], delay) * cos(zrArgIn[sectionIndex] + delay * zpArgIn[sectionIndex]);

	}

	// consolidate with above to save a pow
	T calculateInputWeightB1(float delay, int sectionIndex) {

		return -zBetasIn[sectionIndex] * pow(zpAbsIn[sectionIndex], delay + 1) * cos(zrArgIn[sectionIndex] + (delay - 1) * zpArgIn[sectionIndex]);

	}


	//
	// output filter
	//

	int numRealOut;
	T * realStatesOut;
	T * realMultirateSumOut;
	T * realPolesOut;
	T * realResiduesOut;

	int numConjOut;
	T * zStates1Out;
	T * zStates2Out;
	T * zMultirateSum1Out;
	T * zMultirateSum2Out;
	T * zA0Out;
	T * zA1Out;
	T * zpArgOut;
	T * zpAbsOut;
	T * zrArgOut;
	T * zBetasOut;

	void initOutputFilter(int realSections, float * realResidues, float * realPoles,
							int conjSections, float * conjRResidues, float * conjIResidues, 
								float * conjRPoles, float * conjIPoles) {

		numRealOut = realSections;
		numConjOut = conjSections;

		realStatesOut = (T*) malloc(realSections * sizeof(T));
		realMultirateSumOut = (T*) malloc(realSections * sizeof(T));
		realPolesOut = (T*) malloc(realSections * sizeof(T));
		realResiduesOut = (T*) malloc(realSections * sizeof(T));

		for (int i = 0; i < realSections; i++) {
			realStatesOut[i] = T(0);
			realMultirateSumOut[i] = T(0);
			realPolesOut[i] = T(exp(realPoles[i] * hostSampleTime));
			realResiduesOut[i] = T(realResidues[i]/realPoles[i]);
		}

		zStates1Out = (T*) malloc(conjSections * sizeof(T));
		zStates2Out = (T*) malloc(conjSections * sizeof(T));
		zMultirateSum1Out = (T*) malloc(conjSections * sizeof(T));
		zMultirateSum2Out = (T*) malloc(conjSections * sizeof(T));
		zA0Out = (T*) malloc(conjSections * sizeof(T));
		zA1Out = (T*) malloc(conjSections * sizeof(T));
		zpArgOut = (T*) malloc(conjSections * sizeof(T));
		zpAbsOut = (T*) malloc(conjSections * sizeof(T));
		zrArgOut = (T*) malloc(conjSections * sizeof(T));
		zBetasOut = (T*) malloc(conjSections * sizeof(T));


		for (int i = 0; i < conjSections; i++) {

			float laplacePoleR;
			float laplacePoleI;

			transform(conjRPoles[i], conjIPoles[i], &laplacePoleR, &laplacePoleI, hostSampleTime);
			float pAbs = complexMagnitude(laplacePoleR, laplacePoleI);
			float pArg = complexAngle(laplacePoleR, laplacePoleI);

			zStates1Out[i] = T(0);
			zStates2Out[i] = T(0);
			zMultirateSum1Out[i] = T(0);
			zMultirateSum2Out[i] = T(0);
			zA0Out[i] = T(2 * cos(pArg));
			zA1Out[i] = T(-(pAbs * pAbs));
			zpArgOut[i] = T(pArg);
			zpAbsOut[i] = T(pAbs);
			zrArgOut[i] = T(complexAngle(conjRResidues[i], conjIResidues[i]));
			float pRQuotientR;
			float pRQuotientI;
			divideZ(conjRResidues[i], conjIResidues[i], conjRPoles[i], conjIPoles[i], &pRQuotientR, &pRQuotientI); 
			zBetasOut[i] = T(2 * complexMagnitude(pRQuotientR, pRQuotientI));

		}

	}

	T updateOutputStateR(int sectionIndex) {

		realStatesOut[sectionIndex] = realPolesOut[sectionIndex] * realStatesOut[sectionIndex];
		return realStatesOut[sectionIndex];

	}

	T updateOutputStateZ(int sectionIndex) {

		T out = zStates1Out[sectionIndex];
		zStates1Out[sectionIndex] = zStates2Out[sectionIndex] + out * zA0Out[sectionIndex];
		zStates2Out[sectionIndex] = out * zA1Out[sectionIndex];
		return out;

	}

	T calculateOutputWeightR(float delay, int sectionIndex) {

		return T(realResiduesOut[sectionIndex] * pow(realPolesOut[sectionIndex], 1 - delay));

	}

	T calculateOutputWeightB0(float delay, int sectionIndex) {

		return zBetasOut[sectionIndex] * pow(zpAbsOut[sectionIndex], 1 - delay) * cos(zrArgOut[sectionIndex] + (1 - delay) * zpArgOut[sectionIndex]);

	}

	// consolidate with above to save a pow
	T calculateOutputWeightB1(float delay, int sectionIndex) {

		return -zBetasOut[sectionIndex] * pow(zpAbsOut[sectionIndex], 2 - delay) * cos(zrArgOut[sectionIndex] - delay * zpArgOut[sectionIndex]);

	}

	// input process steps

	T processInputBBD(float delay) {

		T bbdIn = T(0);

		for (int i = 0; i < numRealIn; i++) {
			bbdIn += realStatesIn[i] * calculateInputWeightR(delay, i);
		}

		for (int i = 0; i < numConjIn; i++) {
			bbdIn += zStates1In[i] * calculateInputWeightB0(delay, i);
			bbdIn += zStates2In[i] * calculateInputWeightB1(delay, i);
		}

		return bbdIn;

	}

	void processInputNative(T input) {

		for (int i = 0; i < numRealIn; i++) {
			updateInputStateR(input, i);
		}

		for (int i = 0; i < numConjIn; i++) {
			updateInputStateZ(input, i);
		}
		
	}

	// output process steps

	void processOutputBBD(T input, float delay) {

		for (int i = 0; i < numRealOut; i++) {
			realStatesOut[i] += input * calculateOutputWeightR(delay, i);
		}

		for (int i = 0; i < numConjOut; i++) {
			zStates1Out[i] += input * calculateOutputWeightB0(delay, i);
			zStates2Out[i] += input * calculateOutputWeightB1(delay, i);
		}

	}

	T processOutputNative(void) {

		T h0 = T(.5f);
		T output = lastOut * h0;

		for (int i = 0; i < numRealOut; i++) {
			output += realStatesOut[i];
			updateOutputStateR(i);
		}

		for (int i = 0; i < numConjOut; i++) {
			output += zStates1Out[i];
			updateOutputStateZ(i);
		}

		return output;
		
	}

	//
	// delay line
	//

	T bbd[SIZE];
	T lastOut = T(0);
	int bbdWrite = 0;

	void writeBBD(T input) {
		bbd[bbdWrite] = input;
		bbdWrite++;
		bbdWrite = (bbdWrite >= SIZE) ? 0 : bbdWrite;
	}

	T readBBD(void) {
		return bbd[bbdWrite];
	}

	T readBBDDelta(void) {
		T output = bbd[bbdWrite] - lastOut;
		lastOut = bbd[bbdWrite];
		return output;
	}

	float clockFreq = .5;
	float hostSampleTime = 1.f / 44100.f;
	float nativeTimeIndex = 0;
	float bbdTimeIndex = 0;

	// maintain the bucket brigade at the variable sample rate and the filter states at the main sample rate

	T process(T input) {

		// same rate test
		// for (int i = 0; i < 2; i++) {
		// 	if (i & 1) {
		// 		writeBBD(processInputBBD(0.f));
		// 	} else {
		// 		processOutputBBD(readBBDDelta(), 0.5f);
		// 	}
		// }

		// processInputNative(input);
		// return processOutputNative();

		writeBBD(processInputBBD(0));
		processOutputBBD(readBBDDelta(), 0);
		processInputNative(input / 8.47f);
		return processOutputNative();

	}

	//
	// Juno filter pair analysis from the paper
	//

	#define _DEFAULT_REAL_SECTIONS 1
	#define _DEFAULT_CONJ_SECTIONS 2

	float rInRDefault[_DEFAULT_REAL_SECTIONS] = {251589.f};
	float rInPDefault[_DEFAULT_REAL_SECTIONS] = {-46580.f};

	float zRInRDefault[_DEFAULT_CONJ_SECTIONS] = {-130428.f, 4634.f};
	float zRInPDefault[_DEFAULT_CONJ_SECTIONS] = {-55482.f, -26292.f};

	float zIInRDefault[_DEFAULT_CONJ_SECTIONS] = {-4165.f, -22873.f};
	float zIInPDefault[_DEFAULT_CONJ_SECTIONS] = {25082.f, -59437.f};

	float rOutRDefault[_DEFAULT_REAL_SECTIONS] = {5092.f};
	float rOutPDefault[_DEFAULT_REAL_SECTIONS] = {-176261.f};

	float zROutRDefault[_DEFAULT_CONJ_SECTIONS] = {11256.f, -13802.f};
	float zROutPDefault[_DEFAULT_CONJ_SECTIONS] = {-51468.f, -26276.f};

	float zIOutRDefault[_DEFAULT_CONJ_SECTIONS] = {-99566.f, -24606.f};
	float zIOutPDefault[_DEFAULT_CONJ_SECTIONS] = {21437.f, -59699.f};

	BBD() {

		initInputFilter(_DEFAULT_REAL_SECTIONS, rInRDefault, rInPDefault,
							_DEFAULT_CONJ_SECTIONS, zRInRDefault, zIInRDefault, 
								zRInPDefault, zIInPDefault);

		initOutputFilter(_DEFAULT_REAL_SECTIONS, rOutRDefault, rOutPDefault,
							_DEFAULT_CONJ_SECTIONS, zROutRDefault, zIOutRDefault, 
								zROutPDefault, zIOutPDefault);

		for (int i = 0; i < SIZE; i++) {
			bbd[i] = T(0);
		}

	}

};


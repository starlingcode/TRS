#include "plugin.hpp"
#include "ui.hpp"
#include "matrix/math.hpp"


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

// Naive impementation
// shoving the delay in the feedback path makes this unstable
template <typename T = float>
struct fourPolePhaser {

    AP1<T> stage1;
    AP1<T> stage2;
    AP1<T> stage3;
    AP1<T> stage4;

    T feedback = T(0);

    fourPolePhaser() {}

    void setParams(T freq, T fb) {
        stage1.a0 = freq;
        stage2.a0 = freq;
        stage3.a0 = freq;
        stage4.a0 = freq;
        feedback = fb;
    }

    T process(T input) {
        T signal = stage1.process(input + feedback * -stage4.d2);
        signal = stage2.process(signal);
        signal = stage3.process(signal);
        signal = stage4.process(signal);
        return signal;
    }

};

// state space 4 pole filter cooked up from https://github.com/google/music-synthesizer-for-android/blob/master/lab/Zero%20delay%20the%20easy%20way.ipynb
// aka "Zero delay the easy way"
// float only no vectors

struct ZDFPhaser4 {

	// analog state space prototype
	// extended from first order section
	// ap = lp - hp = lp - (in - lp) = 2*lp - in
	// A = [-1], B = [1], C = [2], D = [-1]

	float A[16] = {-1, 0, 0, 0,
              	    2, -1, 0, 0,
              	   -2, 2, -1, 0,
              	    2, -2, 2, -1};
	float B[4] = {1, -1, 1, -1};
	float C[4] = {-2, 2, -2, 2};
	float D = 1;

	// storage bins for discretized matrixes
	float Az[16] = {-1, 0, 0, 0,
              	     2, -1, 0, 0,
              	    -2, 2, -1, 0,
              	     2, -2, 2, -1};
	float Bz[4] = {1, -1, 1, -1};
	float Cz[4] = {-2, 2, -2, 2};
	float Dz = 1;

	float X[4] = {0, 0, 0, 0};

	// discretize with bilinear transform

	void setParams(float freq, float res) {

		// freq normalized to sr
		// prewarp for bilinear transform
		float g = tan(M_PI * freq);
		// printf("g: %4.4f \n", g);

		// sneak in the feedback gain as res 0 - 1
		A[3] = -res;

		// Inv = (I - gA)^(-1)
		// Inverting the transition matrix solves the system of differential equations
		matrix::SquareMatrix<float, 4> Ag(A);
		Ag *= g;
		matrix::SquareMatrix<float, 4> Inv;
    	Inv.setIdentity();
    	Inv -= Ag;
    	Inv = matrix::inv(Inv);

    	// discretize the transition matrix
    	// Az = Inv * (I + gA)
    	matrix::SquareMatrix<float, 4> A_;
    	A_.setIdentity();
    	A_ += Ag;
    	A_ = Inv * A_;

    	// discretize the input vector (column)
    	// Bz = 2g * Inv * B
    	matrix::Vector<float, 4> Bs(B);
    	matrix::Vector<float, 4> B_;
    	B_ = 2 * g * Inv * Bs;

    	// discretize the output vector (row)
    	// Cz = C * Inv
    	matrix::Matrix<float, 1, 4> C_;
    	matrix::Vector<float, 4> Cs(C);
    	C_ = Cs.T() * Inv;

    	// Calculate the direct input feedthrough
    	// Prototype D is added when storing
    	// Dz = g * C * Inv * B + D
    	matrix::Matrix<float, 1, 1> D_;
    	D_ = Cs.T() * Inv * (g * Bs);
    	
    	// store above calculations 
    	A_.copyTo(Az);
    	B_.copyTo(Bz);
    	C_.copyTo(Cz);
    	Dz = D_(0, 0) + D;

	}

	float process(float in) {

		// calculate output from current state and current input 
		matrix::Vector<float, 4> X_(X);
		matrix::Vector<float, 4> C_(Cz);
		float out = in * Dz + X_.dot(C_);

		// update the state for the next input from current input and transition matrix
		matrix::SquareMatrix<float, 4> A_(Az);
		matrix::Vector<float, 4> B_(Bz);
		X_ = A_ * X_ + in * B_;
		// store it
		X_.copyTo(X);

		// return the output from the first section		
		return out;

	}

};

// same thing, 8 pole prototype

struct ZDFPhaser8 {

	float A[64] = {-1, 0, 0, 0, 0, 0, 0, 0,
              		2, -1, 0, 0, 0, 0, 0, 0,
              		-2, 2, -1, 0, 0, 0, 0, 0,
              		2, -2, 2, -1, 0, 0, 0, 0,
              		-2, 2, -2, 2, -1, 0, 0, 0,
              		2, -2, 2, -2, 2, -1, 0, 0,
              		-2, 2, -2, 2, -2, 2, -1, 0,
              		2, -2, 2, -2, 2, -2, 2, -1};
	float B[8] = {1, -1, 1, -1, 1, -1, 1, -1};
	float C[8] = {-2, 2, -2, 2, -2, 2, -2, 2};
	float D = 1;

	float Az[64] = {-1, 0, 0, 0, 0, 0, 0, 0,
              		2, -1, 0, 0, 0, 0, 0, 0,
              		-2, 2, -1, 0, 0, 0, 0, 0,
              		2, -2, 2, -1, 0, 0, 0, 0,
              		-2, 2, -2, 2, -1, 0, 0, 0,
              		2, -2, 2, -2, 2, -1, 0, 0,
              		-2, 2, -2, 2, -2, 2, -1, 0,
              		2, -2, 2, -2, 2, -2, 2, -1};
	float Bz[8] = {1, -1, 1, -1};
	float Cz[8] = {-2, 2, -2, 2};
	float Dz = 1;

	float X[8] = {0, 0, 0, 0, 0, 0, 0, 0};


	void setParams(float freq, float res) {

		float g = tan(M_PI * freq);

		A[7] = -res;

		matrix::SquareMatrix<float, 8> Ag(A);
		Ag *= g;
		matrix::SquareMatrix<float, 8> Inv;
    	Inv.setIdentity();
    	Inv -= Ag;
    	Inv = matrix::inv(Inv);

    	matrix::SquareMatrix<float, 8> A_;
    	A_.setIdentity();
    	A_ += Ag;
    	A_ = Inv * A_;

    	matrix::Vector<float, 8> Bs(B);
    	matrix::Vector<float, 8> B_;
    	B_ = 2 * g * Inv * Bs;

    	matrix::Matrix<float, 1, 8> C_;
    	matrix::Vector<float, 8> Cs(C);
    	C_ = Cs.T() * Inv;

    	matrix::Matrix<float, 1, 1> D_;
    	D_ = Cs.T() * Inv * (g * Bs);

    	A_.copyTo(Az);
    	B_.copyTo(Bz);
    	C_.copyTo(Cz);
    	Dz = D_(0, 0) + D;

	}

	float process(float in) {

		matrix::Vector<float, 8> X_(X);
		matrix::Vector<float, 8> C_(Cz);
		float out = in * Dz + X_.dot(C_);
		matrix::SquareMatrix<float, 8> A_(Az);
		matrix::Vector<float, 8> B_(Bz);
		X_ = A_ * X_ + in * B_;
		X_.copyTo(X);		
		return out;

	}

};

// Classic maligned JOS Chamberlin SVF discretized with forward/backward euler, unstable above ~ sr/6

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

// SVF with same technique as the above state space phasers from "Zero delay the easy way"
// This one can process a float_4 due to simple explicit formula for inverting 2 x 2 matrix
// Includes some trivial modifications to https://github.com/google/music-synthesizer-for-android/blob/master/lab/Second%20order%20sections%20in%20matrix%20form.ipynb
// for simultaineuous outputs from same state/transition matrix

template <typename T = float>
struct ZDFSVF {

	// storage for the discretized coefficients

	T Az[4] = {T(0.f), T(0.f),
			   T(0.f), T(0.f)};
	T Bz[2] = {T(0.f), T(0.f)};
	T CLPz[2] = {T(0.f), T(0.f)};
	T CBPz[2] = {T(0.f), T(0.f)};
	T CHPz[2] = {T(0.f), T(0.f)};
	T DLPz = T(0.f);
	T DBPz = T(0.f);
	T DHPz = T(0.f);

	T X[2] = {T(0.f), T(0.f)};

	// analog prototype is woven into the discretization formula
	// putting a 2 where resonance gain will eventually be inserted

	// T A[4] = {2, -1
	// 		  1, 0};
	// T B[2] = {1, 0};
	// T CLP[2] = {0, 1};
	// T CBP[2] = {1, 0};
	// T CHP[2] = {2, -1};
	// T D = 0;
	// T DHP = 1;

	T lpOut = T(0);
	T bpOut = T(0);
	T hpOut = T(0);

	void setParams(T freq, T res) {

		T g = tan(T(M_PI) * freq);
    	T k = T(2.f) - T(2) * res;
   		T a1 = T(1.f)/(T(1.f) + g * (g + k));
    	T a2 = g * a1;
   		T a3 = g * a2;

   		Az[0] = T(2.f) * a1 - T(1.f);
   		Az[1] = T(-2.f) * a2;
   		Az[2] = T(2.f) * a2;
   		Az[3] = T(1.f) - T(2.f) * a3;

   		// printf("A : %4.4f, %4.4f \n", Az[0][0], Az[1][0]);
   		// printf("A : %4.4f, %4.4f \n\n", Az[2][0], Az[3][0]);

   		Bz[0] = T(2.f) * a2;
   		Bz[1] = T(2.f) * a3;

   		// printf("B : %4.4f, %4.4f \n\n", Bz[0][0], Bz[1][0]);

   		CLPz[0] = a2;
   		CLPz[1] = T(1.f) - a3;

   		// printf("CLP : %4.4f, %4.4f \n\n", CLPz[0][0], CLPz[1][0]);

   		CBPz[0] = a1;
   		CBPz[1] = -a2;

   		CHPz[0] = -k * a1 - a2;
   		CHPz[1] = k * a2 - (T(1.f) - a3);

   		DLPz = a3;
   		DBPz = a2;
   		DHPz = T(1.f) - k * a2 - a3;

	}

	void process(T in) {

		lpOut = in * DLPz + X[0] * CLPz[0] + X[1] * CLPz[1];
		bpOut = in * DBPz + X[0] * CBPz[0] + X[1] * CBPz[1];
		hpOut = in * DHPz + X[0] * CHPz[0] + X[1] * CHPz[1];

		T X0 = in * Bz[0] + X[0] * Az[0] + X[1] * Az[1];
		T X1 = in * Bz[1] + X[0] * Az[2] + X[1] * Az[3];

		X[0] = X0;
		X[1] = X1;

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

template <typename T = float_4, int32_t SIZE = 256>
struct BBD {

	/////////////// Filter stuff 

	
	//
	// coefficient precalculation helpers
	//

	float complexMagnitude(float r, float i) {
		return sqrt(r * r + i * i);
	}
	float complexAngle(float r, float i) {
		return atan2(i, r);
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
	// input filter init
	//

	int numRealIn;
	T * realStatesIn;
	T * realPolesIn;
	T * realResiduesIn;

	int numConjIn;
	T * zStates1In;
	T * zStates2In;
	T * zStates3In;
	T * zA0In;
	T * zA1In;
	T * zpArgIn;
	T * zpAbsIn;
	T * zrArgIn;
	T * zBetasIn;

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
		zStates3In = (T*) malloc(conjSections * sizeof(T));
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
			if (laplacePoleR > 0) {
				laplacePoleR *= -1;
			}
			float pAbs = complexMagnitude(laplacePoleR, laplacePoleI);
			float pArg = complexAngle(laplacePoleR, laplacePoleI);

			zStates1In[i] = T(0);
			zStates2In[i] = T(0);
			zStates3In[i] = T(0);
			zA0In[i] = T(2 * cos(pArg));
			zA1In[i] = T(-pAbs * pAbs);
			zpArgIn[i] = T(pArg);
			zpAbsIn[i] = T(pAbs);
			zrArgIn[i] = T(complexAngle(conjRResidues[i], conjIResidues[i]));
			zBetasIn[i] = T(2 * hostSampleTime * complexMagnitude(conjRResidues[i], conjIResidues[i]));

		}

	}

	
	//
	// output filter init
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
	T H0 = 0;

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
			H0 += T(realResidues[i]/realPoles[i]);
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

			printf("laplacePoleR section %d: %4.4f \n", i, laplacePoleR);
			printf("laplacePoleI section %d: %4.4f \n", i, laplacePoleI);
			printf("pAbs section %d: %4.4f \n", i, pAbs);
			printf("pArg section %d: %4.4f \n", i, pArg);
			printf("A0 section %d: %4.4f \n", i, 2 * cos(pArg));

			zStates1Out[i] = T(0);
			zStates2Out[i] = T(0);
			zMultirateSum1Out[i] = T(0);
			zMultirateSum2Out[i] = T(0);
			zA0Out[i] = T(2 * cos(pArg));
			zA1Out[i] = T(-pAbs * pAbs);
			zpArgOut[i] = T(pArg);
			zpAbsOut[i] = T(pAbs);
			zrArgOut[i] = T(complexAngle(conjRResidues[i], conjIResidues[i]));
			float pRQuotientR;
			float pRQuotientI;
			divideZ(conjRResidues[i], conjIResidues[i], conjRPoles[i], conjIPoles[i], &pRQuotientR, &pRQuotientI); 
			printf("pRQuotientR section %d: %4.4f \n", i, pRQuotientR);
			printf("pRQuotientI section %d: %4.4f \n", i, pRQuotientI);
			zBetasOut[i] = T(2 * complexMagnitude(pRQuotientR, pRQuotientI));
			printf("zBetasOut section %d: %4.4f \n", i, zBetasOut[i][0]);
			H0 += pRQuotientR;


		}

	}

	
	//
	// input filter helpers
	//

	void updateInputStateR(T input, int sectionIndex) {

		realStatesIn[sectionIndex] = realPolesIn[sectionIndex] * realStatesIn[sectionIndex] + input;

	}

	void updateInputStateZ(T input, int sectionIndex) {

		T x1 = input;
		x1 += zStates1In[sectionIndex] * zA0In[sectionIndex];
		x1 += zStates2In[sectionIndex] * zA1In[sectionIndex];
		zStates2In[sectionIndex] = zStates1In[sectionIndex];
		zStates1In[sectionIndex] = x1;

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
	// output filter helpers
	//

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

	
	//
	// input process steps
	//

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

	
	//
	// output process steps
	//

	void processOutputBBD(T input, float delay) {

		for (int i = 0; i < numRealOut; i++) {
			realStatesOut[i] += input * calculateOutputWeightR(delay, i);
		}

		for (int i = 0; i < numConjOut; i++) {
			T b0 = calculateOutputWeightB0(delay, i);
			T b1 = calculateOutputWeightB1(delay, i);
			zStates1Out[i] += input * b0;
			zStates2Out[i] += input * b1;
		}

	}

	T processOutputNative(void) {

		T output = lastOut * H0;

		for (int i = 0; i < numRealOut; i++) {
			output += realStatesOut[i];
			updateOutputStateR(i);
		}

		// for (int i = 0; i < numConjOut; i++) {
		// 	output += zStates1Out[i];
		// 	updateOutputStateZ(i);
		// }

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

	void changeSR(float sr) {

	}

	float hostSampleTime = 1.f / 44100.f;
	float nativeTimeIndex = 0;
	float bbdTimeIndex = 0;
	int32_t bbdStepTracker = 0; 

	// maintain the bucket brigade at the variable sample rate and the filter states at the main sample rate

	T process(T input, float clockFreq) {

		float bbdStep = 1/(clockFreq * hostSampleTime);

		nativeTimeIndex ++;

		while (bbdTimeIndex < nativeTimeIndex) {

			float delay = bbdTimeIndex - (nativeTimeIndex - 1);
			// delay = 1 - delay;

			if (bbdStepTracker & 1) {
				// even steps
				writeBBD(processInputBBD(delay));
			} else {
				// odd steps
				processOutputBBD(readBBDDelta(), delay);
			}
			
			bbdStepTracker++;
			bbdTimeIndex += bbdStep;

		}

		if (nativeTimeIndex >= 1000) {
			bbdTimeIndex -= 1000;
			nativeTimeIndex -= 1000;
		}

		processInputNative(input);
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


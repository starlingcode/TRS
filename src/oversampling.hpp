

using simd::float_4;

// From Fredrick Harris Multirate Signal Processing for Communication Systems
// Original paper with AG Constantinides
// https://www.researchgate.net/publication/259753999_Digital_Signal_Processing_with_Efficient_Polyphase_Recursive_All-pass_Filters

template <typename T = float>
struct APPath1 {

	APPath1() {}

	T process(T input) {
		T out1 = (input - d2) * a0 + d1;
        d1 = input;
        d2 = out1;
        return out1;
	}

	void setCoefficients(float coeff0) {
		a0 = T(coeff0);
	}

	T d1 = T(0);
	T d2 = T(0);

	T a0 = T(0);

};

template <typename T = float>
struct APPath2 {

	APPath2() {}

	T process(T input) {
		T out1 = (input - d2) * a0 + d1;
        T out2 = (out1 - d3) * a1 + d2;
        d1 = input;
        d2 = out1;
        d3 = out2;
        return out2;
	}

	void setCoefficients(float coeff0, float coeff1) {
		a0 = T(coeff0);
		a1 = T(coeff1);
	}

	T d1 = T(0);
	T d2 = T(0);
	T d3 = T(0);

	T a0 = T(0);
	T a1 = T(0);

};

/** Decimate by a factor of 32 with cascaded half band filters. */
template <int OVERSAMPLE, typename T = float>
struct DecimatePow2 {
	
	T in32Buffer[32];

	T in16Buffer[16];
	
	T in8Buffer[8];
	
	T in4Buffer[4];

	T in2Buffer[2];

	APPath2<T> from2to1Path1;
	APPath2<T> from2to1Path2;
	APPath2<T> from4to2Path1;
	APPath2<T> from4to2Path2;
	APPath1<T> from8to4Path1;
	APPath1<T> from8to4Path2;
	APPath1<T> from16to8Path1;
	APPath1<T> from16to8Path2;
	APPath1<T> from32to16Path1;
	APPath1<T> from32to16Path2;

	DecimatePow2() {

		// filter design routine is laid out in the original paper
		// these are designed from a lucky find of the matlab code for the harris book

		from2to1Path1.setCoefficients(0.0798664262025582, 0.5453236511825826);
		from2to1Path2.setCoefficients(0.283829344898100, 0.834411891201724);

		from4to2Path1.setCoefficients(0.0798664262025582, 0.5453236511825826);
		from4to2Path2.setCoefficients(0.283829344898100, 0.834411891201724);

		from8to4Path1.setCoefficients(0.11192);
		from8to4Path2.setCoefficients(0.53976);

		from16to8Path1.setCoefficients(0.11192);
		from16to8Path2.setCoefficients(0.53976);

		from32to16Path1.setCoefficients(0.11192);
		from32to16Path2.setCoefficients(0.53976);

		reset();

	}

	void reset() {
		std::memset(in32Buffer, 0, sizeof(in32Buffer));
		std::memset(in16Buffer, 0, sizeof(in16Buffer));
		std::memset(in8Buffer, 0, sizeof(in8Buffer));
		std::memset(in4Buffer, 0, sizeof(in4Buffer));
		std::memset(in2Buffer, 0, sizeof(in2Buffer));
	}

	T process(T * in) {
		if (OVERSAMPLE == 2) {
			return process2x(in);
		} else if (OVERSAMPLE == 4) {
			return process4x(in);
		} else if (OVERSAMPLE == 8) {
			return process8x(in);
		} else if (OVERSAMPLE == 16) {
			return process16x(in);
		}else if (OVERSAMPLE == 32) {
			return process32x(in);
		}  else {
			return in[0];
		}
	}

	/** `in` must be length 32 */
	T process32x(T * in) {
		
		// copy in the data
		std::memcpy(&in32Buffer[0], in, 32*sizeof(T));		

		process32to16();
		process16to8();
		process8to4();
		process4to2();
		return process2to1();

	}

	/** `in` must be length 16 */
	T process16x(T * in) {
		
		// copy in the data
		std::memcpy(&in16Buffer[0], in, 16*sizeof(T));		

		process16to8();
		process8to4();
		process4to2();
		return process2to1();

	}

	/** `in` must be length 8 */
	T process8x(T * in) {
		
		// copy in the data
		std::memcpy(&in8Buffer[0], in, 8*sizeof(T));

		process8to4();
		process4to2();
		return process2to1();

	}

	/** `in` must be length 4 */
	T process4x(T * in) {
		
		// copy in the data
		std::memcpy(&in4Buffer[0], in, 4*sizeof(T));

		process4to2();
		return process2to1();

	}

	/** `in` must be length 2 */
	T process2x(T * in) {
		
		// copy in the data
		std::memcpy(&in2Buffer[0], in, 2*sizeof(T));

		return process2to1();

	}

	void inline process32to16(void) {

		int writeIndex = 0;

		// filter every other sample and write to the x8 buffer
		for (int i = 0; i < 32; i += 2) {
			
			in16Buffer[writeIndex] = (from32to16Path1.process(in32Buffer[i + 1]) + from32to16Path2.process(in32Buffer[i])) * 0.5;
			writeIndex++;

		}

	}

	void inline process16to8(void) {

		int writeIndex = 0;

		// filter every other sample and write to the x8 buffer
		for (int i = 0; i < 16; i += 2) {
			
			in8Buffer[writeIndex] = (from16to8Path1.process(in16Buffer[i + 1]) + from16to8Path2.process(in16Buffer[i])) * 0.5;;
			writeIndex++;

		}

	}

	void inline process8to4(void) {

		in4Buffer[0] = (from8to4Path1.process(in8Buffer[1]) + from8to4Path2.process(in8Buffer[0])) * 0.5;

		in4Buffer[1] = (from8to4Path1.process(in8Buffer[3]) + from8to4Path2.process(in8Buffer[2])) * 0.5;

		in4Buffer[2] = (from8to4Path1.process(in8Buffer[5]) + from8to4Path2.process(in8Buffer[4])) * 0.5;

		in4Buffer[3] = (from8to4Path1.process(in8Buffer[7]) + from8to4Path2.process(in8Buffer[6])) * 0.5;

	}

	void inline  process4to2(void) {

		in2Buffer[0] = (from4to2Path1.process(in4Buffer[1]) + from4to2Path2.process(in4Buffer[0])) * 0.5;

		in2Buffer[1] = (from4to2Path1.process(in4Buffer[3]) + from4to2Path2.process(in4Buffer[2])) * 0.5;


	}


	T inline process2to1(void) {

		return (from2to1Path1.process(in2Buffer[1]) + from2to1Path2.process(in2Buffer[0])) * 0.5;
	}
	
};




/** Upsample by a factor of 32 with cascaded half band filters. */
// This time weave alternating samples from the two allpass paths into the upsampled data stream
template <int OVERSAMPLE, typename T = float>
struct UpsamplePow2 {
	
	T out32Buffer[32];

	T out16Buffer[16];
	
	T out8Buffer[8];
	
	T out4Buffer[4];

	T out2Buffer[2];

	APPath2<T> from1to2Path1;
	APPath2<T> from1to2Path2;
	APPath2<T> from2to4Path1;
	APPath2<T> from2to4Path2;
	APPath1<T> from4to8Path1;
	APPath1<T> from4to8Path2;
	APPath1<T> from8to16Path1;
	APPath1<T> from8to16Path2;
	APPath1<T> from16to32Path1;
	APPath1<T> from16to32Path2;

	T * output;

	UpsamplePow2() {

		from1to2Path1.setCoefficients(0.0798664262025582, 0.5453236511825826);
		from1to2Path2.setCoefficients(0.283829344898100, 0.834411891201724);

		from2to4Path1.setCoefficients(0.0798664262025582, 0.5453236511825826);
		from2to4Path2.setCoefficients(0.283829344898100, 0.834411891201724);

		from4to8Path1.setCoefficients(0.11192);
		from4to8Path2.setCoefficients(0.53976);

		from8to16Path1.setCoefficients(0.11192);
		from8to16Path2.setCoefficients(0.53976);

		from16to32Path1.setCoefficients(0.11192);
		from16to32Path2.setCoefficients(0.53976);

		reset();

		if (OVERSAMPLE == 2) {
			output = out2Buffer;
		} else if (OVERSAMPLE == 4) {
			output = out4Buffer;
		} else if (OVERSAMPLE == 8) {
			output = out8Buffer;
		} else if (OVERSAMPLE == 16) {
			output = out16Buffer;
		} else if (OVERSAMPLE == 32) {
			output = out32Buffer;
		}  else {
			output = out2Buffer;
		}

	}

	void reset() {
		std::memset(out32Buffer, 0, sizeof(out32Buffer));
		std::memset(out16Buffer, 0, sizeof(out16Buffer));
		std::memset(out8Buffer, 0, sizeof(out8Buffer));
		std::memset(out4Buffer, 0, sizeof(out4Buffer));
		std::memset(out2Buffer, 0, sizeof(out2Buffer));
	}

	void process(T in) {
		if (OVERSAMPLE == 2) {
			process2x(in);
		} else if (OVERSAMPLE == 4) {
			process4x(in);
		} else if (OVERSAMPLE == 8) {
			process8x(in);
		} else if (OVERSAMPLE == 16) {
			process16x(in);
		} else if (OVERSAMPLE == 32) {
			process32x(in);
		}
	}

	void process32x(T in) {

		process1to2(in);
		process2to4();
		process4to8();
		process8to16();
		process16to32();

	}

	void process16x(T in) {

		process1to2(in);
		process2to4();
		process4to8();
		process8to16();

	}

	void process8x(T in) {

		process1to2(in);
		process2to4();
		process4to8();

	}

	void process4x(T in) {
		
		process1to2(in);
		process2to4();

	}

	void process2x(T in) {

		process1to2(in);

	}

	void inline process16to32(void) {

		for (int i = 0; i < 32; i += 2) {
			
			out32Buffer[i] = from8to16Path2.process(out16Buffer[i >> 1]);
			out32Buffer[i+1] = from8to16Path1.process(out16Buffer[i >> 1]);

		}

	}

	void inline process8to16(void) {

		for (int i = 0; i < 16; i += 2) {
			
			out16Buffer[i] = from8to16Path2.process(out8Buffer[i >> 1]);
			out16Buffer[i+1] = from8to16Path1.process(out8Buffer[i >> 1]);

		}

	}

	void inline process4to8(void) {

		out8Buffer[0] = from4to8Path2.process(out4Buffer[0]);
		out8Buffer[1] = from4to8Path1.process(out4Buffer[0]);
		out8Buffer[2] = from4to8Path2.process(out4Buffer[1]);
		out8Buffer[3] = from4to8Path1.process(out4Buffer[1]);

		out8Buffer[4] = from4to8Path2.process(out4Buffer[2]);
		out8Buffer[5] = from4to8Path1.process(out4Buffer[2]);
		out8Buffer[6] = from4to8Path2.process(out4Buffer[3]);
		out8Buffer[7] = from4to8Path1.process(out4Buffer[3]);


	}

	void inline  process2to4(void) {

		out4Buffer[0] = from2to4Path2.process(out2Buffer[0]);
		out4Buffer[1] = from2to4Path1.process(out2Buffer[0]);
		out4Buffer[2] = from2to4Path2.process(out2Buffer[1]);
		out4Buffer[3] = from2to4Path1.process(out2Buffer[1]);


	}


	void inline process1to2(T in) {

		out2Buffer[0] = from1to2Path2.process(in);
		out2Buffer[1] = from1to2Path1.process(in);

	}
	
};

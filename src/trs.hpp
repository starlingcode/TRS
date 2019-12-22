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


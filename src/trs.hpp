#include "plugin.hpp"
#include "starling-rack-ui.hpp"
#include "starling-dsp.hpp"

using simd::float_4;
using simd::int32_4;

struct StereoInHandler : Input {

	Input * input;

	void configure(Input * inputJack) {
		input = inputJack;
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

	float_4 getLeftNormal(float_4 normal, int polySection) {
		polySection &= 1;
		return input->getNormalVoltageSimd<float_4>(normal, polySection * 4);
	}

	float_4 getRightNormal(float_4 normal, int polySection) {
		polySection &= 1;
		return input->getNormalVoltageSimd<float_4>(normal, 8 + polySection * 4);
	}

	float getLeftNormal(float normal) {
		return input->getNormalVoltage(normal, 0);
	}

	float getRightNormal(float normal) {
		return input->getNormalVoltage(normal, 8);
	}

};

struct StereoOutHandler : Output {

	Output * output;

	void configure(Output * outputJack) {
		output = outputJack;
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


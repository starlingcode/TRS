#include "trs.hpp"

// needs the diodes
struct JOSSVF {

    float_4 delay1 = float_4(0);
    float_4 delay2 = float_4(0);

    float_4 hpOut = float_4(0);
    float_4 bpOut = float_4(0);
    float_4 lpOut = float_4(0);

    JOSSVF() {};

    inline void process(float cutoff, float res, float_4 input, float_4 linFM, float_4 expoFM, float_4 resCV) {

        float_4 sr = APP->engine->getSampleRate();

        // Using notation from paper, sampling interval in seconds
        float_4 T = 1.0/sr;

        // absolute target cutoff frequency
        float_4 resFreq = 30.0 * pow(2.0, cutoff * 12.0);
        resFreq *= dsp::approxExp2_taylor5(expoFM + float_4(5.f)) / float_4(1024.f);

        linFM += 5.0;

        resFreq *= clamp(linFM, 0.1f, 10.f)/float_4(5.f);

        // float_4 flip = sgn(linFM);

        resFreq = clamp(resFreq, 0.0, 20000.0);

        // normalized target cutoff frequency
        // also using notation from paper
        float_4 w = resFreq * T * 2 * M_PI;
        // float w = 0.1;


        // resonance, given in paper as sqrt(2)
        float_4 q = res * 2.0;
        q = 2.0 - q;

        // float_4 q = float_4(1.f);

        float_4 highpass = input/float_4(5.f) - delay2 - delay1 * q;
        float_4 bandpass = highpass * w + delay1;
        float_4 lowpass = bandpass * w + delay2;

        delay1 = bandpass;
        delay2 = lowpass;

        lpOut = lowpass * float_4(5.f);
        bpOut = bandpass * float_4(5.f);
        hpOut = highpass * float_4(5.f);

    }

};


struct TRSVCF : Module {
    enum ParamIds {
        FREQ_PARAM,
        RES_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        IN_INPUT,
        LIN_INPUT,
        EXP_INPUT,
        RES_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        HP_OUTPUT,
        BP_OUTPUT,
        LP_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    StereoInHandler signalIn;
    StereoInHandler linCV;
    StereoInHandler expoCV;
    StereoInHandler resCV;

    StereoOutHandler lpOut;
    StereoOutHandler bpOut;
    StereoOutHandler hpOut;

    TRSVCF() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(FREQ_PARAM, 0.f, 1.f, 0.f, "");
        configParam(RES_PARAM, 0.f, 1.f, 0.f, "");

        signalIn.configure(&inputs[IN_INPUT]);
        linCV.configure(&inputs[LIN_INPUT]);
        expoCV.configure(&inputs[EXP_INPUT]);
        resCV.configure(&inputs[RES_INPUT]);

        hpOut.configure(&outputs[HP_OUTPUT]);
        bpOut.configure(&outputs[BP_OUTPUT]);
        lpOut.configure(&outputs[LP_OUTPUT]);

    }

    float_4 delay1 = float_4(0);
    float_4 delay2 = float_4(0);

    JOSSVF filters[2][2];

    void process(const ProcessArgs &args) override {

        outputs[HP_OUTPUT].setChannels(16);
        outputs[BP_OUTPUT].setChannels(16);
        outputs[LP_OUTPUT].setChannels(16);

        for (int polyChunk = 0; polyChunk < 2; polyChunk++) {

            filters[0][polyChunk].process(params[FREQ_PARAM].getValue(), params[RES_PARAM].getValue(), signalIn.getLeft(polyChunk), linCV.getLeft(polyChunk), expoCV.getLeft(polyChunk), resCV.getLeft(polyChunk));

            hpOut.setLeft(filters[0][polyChunk].hpOut, polyChunk);
            bpOut.setLeft(filters[0][polyChunk].bpOut, polyChunk);
            lpOut.setLeft(filters[0][polyChunk].lpOut, polyChunk);

            filters[1][polyChunk].process(params[FREQ_PARAM].getValue(), params[RES_PARAM].getValue(), signalIn.getRight(polyChunk), linCV.getRight(polyChunk), expoCV.getRight(polyChunk), resCV.getRight(polyChunk));

            hpOut.setRight(filters[1][polyChunk].hpOut, polyChunk);
            bpOut.setRight(filters[1][polyChunk].bpOut, polyChunk);
            lpOut.setRight(filters[1][polyChunk].lpOut, polyChunk);

        }

    }
};


struct TRSVCFWidget : ModuleWidget {
    TRSVCFWidget(TRSVCF *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSVCF.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamBlack>(mm2px(Vec(15.225, 17.609)), module, TRSVCF::FREQ_PARAM));
        addParam(createParamCentered<SifamGrey>(mm2px(Vec(15.225, 41.109)), module, TRSVCF::RES_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(15.364, 71.496)), module, TRSVCF::IN_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(8.95, 85.501)), module, TRSVCF::LIN_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(8.952, 99.498)), module, TRSVCF::EXP_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(8.952, 113.499)), module, TRSVCF::RES_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.777, 85.499)), module, TRSVCF::HP_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.777, 99.501)), module, TRSVCF::BP_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.777, 113.501)), module, TRSVCF::LP_OUTPUT));
    }
};


Model *modelTRSVCF = createModel<TRSVCF, TRSVCFWidget>("TRSVCF");
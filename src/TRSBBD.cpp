#include "trs.hpp"


struct TRSBBD : Module {
    enum ParamIds {
        TIME_PARAM,
        FEEDBACK_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        FEEDBACK_INPUT,
        TIME_INPUT,
        SIGNAL_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        SIGNAL_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    Delay<float_4> delays[2][2];

    BBD<float_4> bbds[2][2];

    StereoInHandler fbIn;
    StereoInHandler timeIn;
    StereoInHandler signalIn;

    StereoOutHandler signalOut;

    TRSBBD() {

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(TIME_PARAM, 0.f, 1.f, 0.f, "");
        configParam(FEEDBACK_PARAM, 0.f, 1.f, 0.f, "");

        fbIn.configure(&inputs[FEEDBACK_INPUT]);
        timeIn.configure(&inputs[TIME_INPUT]);
        signalIn.configure(&inputs[SIGNAL_INPUT]);

        signalOut.configure(&outputs[SIGNAL_OUTPUT]);

        delays[0][0].init(48000);
        delays[0][1].init(48000);
        delays[1][0].init(48000);
        delays[1][1].init(48000);

        onSampleRateChange();

    }

    void process(const ProcessArgs &args) override {

        float_4 input = inputs[SIGNAL_INPUT].getVoltageSimd<float_4>(0);

        outputs[SIGNAL_OUTPUT].setVoltageSimd<float_4>(bbds[0][0].process(input), 0);

        // outputs[SIGNAL_OUTPUT].setChannels(16);

        // for (int polyChunk = 0; polyChunk < 2; polyChunk ++) {

        //     float ms = 0.03f + params[TIME_PARAM].getValue() * 50.f;
        //     // no poly rate CV yet (hard to vectorize), temporary hardcoded mono channel for l (0) and r (8)
        //     float_4 leftTap = delays[0][polyChunk].readLinear(ms + clamp(inputs[TIME_INPUT].getVoltage(0), 0.f, 5.f) * 10.f);
        //     float_4 rightTap = delays[1][polyChunk].readLinear(ms + clamp(inputs[TIME_INPUT].getVoltage(8), 0.f, 5.f) * 10.f);

        //     float_4 fbL = clamp(float_4(params[FEEDBACK_PARAM].getValue()) + fbIn.getLeft(polyChunk) / float_4(5.f), 0.f, 0.99f);
        //     float_4 fbR = clamp(float_4(params[FEEDBACK_PARAM].getValue()) + fbIn.getLeft(polyChunk) / float_4(5.f), 0.f, 0.99f);

        //     // add dc blockaz
        //     delays[0][polyChunk].writeDCBlock(signalIn.getLeft(polyChunk) + leftTap * fbL);
        //     delays[1][polyChunk].writeDCBlock(signalIn.getRight(polyChunk) + rightTap * fbR);

        //     signalOut.setLeft(leftTap, polyChunk);
        //     signalOut.setRight(rightTap, polyChunk);

        // }

    }

    void onSampleRateChange() override {
        
        float sampleRate = APP->engine->getSampleRate();

        delays[0][0].changeSR(sampleRate);
        delays[0][1].changeSR(sampleRate);
        delays[1][0].changeSR(sampleRate);
        delays[1][1].changeSR(sampleRate);

    }

};


struct TRSBBDWidget : ModuleWidget {
    TRSBBDWidget(TRSBBD *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSBBD.svg")));

        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamGrey>(mm2px(Vec(10.76, 23.241)), module, TRSBBD::TIME_PARAM));
        addParam(createParamCentered<SifamGrey>(mm2px(Vec(10.76, 46.741)), module, TRSBBD::FEEDBACK_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(10.127, 71.508)), module, TRSBBD::FEEDBACK_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.126, 85.506)), module, TRSBBD::TIME_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.16, 99.499)), module, TRSBBD::SIGNAL_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.16, 113.501)), module, TRSBBD::SIGNAL_OUTPUT));
    }
};


Model *modelTRSBBD = createModel<TRSBBD, TRSBBDWidget>("TRSBBD");
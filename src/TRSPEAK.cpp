#include "trs.hpp"


struct TRSPEAK : Module {
    enum ParamIds {
        THRESH_PARAM,
        GAIN_PARAM,
        ATTACK_PARAM,
        RELEASE_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        IN_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        GATE_OUTPUT,
        NONINV_OUTPUT,
        INV_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    StereoInHandler in;

    StereoOutHandler gate;
    StereoOutHandler out;
    StereoOutHandler outInv;

    PeakFollower<float_4> followers[2][2];

    TRSPEAK() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(THRESH_PARAM, 0.f, 5.f, 0.f, "");
        configParam(GAIN_PARAM, 0.f, 1.f, 0.f, "");
        configParam(ATTACK_PARAM, 0.001f, 1.f, 0.f, "");
        configParam(RELEASE_PARAM, 0.1f, 1.f, 0.f, "");

        in.configure(&inputs[IN_INPUT]);

        gate.configure(&outputs[GATE_OUTPUT]);
        out.configure(&outputs[NONINV_OUTPUT]);
        outInv.configure(&outputs[INV_OUTPUT]);

    }   

    void process(const ProcessArgs &args) override {

        outputs[GATE_OUTPUT].setChannels(16);
        outputs[NONINV_OUTPUT].setChannels(16);
        outputs[INV_OUTPUT].setChannels(16);

        for (int polyChunk = 0; polyChunk < 2; polyChunk ++) {

            followers[polyChunk][0].setTimes(params[ATTACK_PARAM].getValue(), params[RELEASE_PARAM].getValue());
            followers[polyChunk][1].setTimes(params[ATTACK_PARAM].getValue(), params[RELEASE_PARAM].getValue());

            float_4 follow = followers[polyChunk][0].process(in.getLeft(polyChunk));
            gate.setLeft(ifelse(follow > 0.f, float_4(5.f), float_4(0.f)), polyChunk);
            follow = clamp(follow - params[THRESH_PARAM].getValue(), 0.f, 10.f);
            follow *= params[GAIN_PARAM].getValue();
            out.setLeft(follow, polyChunk);
            outInv.setLeft(-follow, polyChunk);

            follow = followers[polyChunk][1].process(in.getRight(polyChunk));
            gate.setRight(ifelse(follow > 0.f, float_4(5.f), float_4(0.f)), polyChunk);
            follow = clamp(follow - params[THRESH_PARAM].getValue(), 0.f, 10.f);
            follow *= params[GAIN_PARAM].getValue();
            out.setRight(follow, polyChunk);
            outInv.setRight(-follow, polyChunk);

        }

    }

};


struct TRSPEAKWidget : ModuleWidget {
    TRSPEAKWidget(TRSPEAK *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSPEAK.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamBlack>(mm2px(Vec(8.826, 17.609)), module, TRSPEAK::THRESH_PARAM));
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(19.746, 38.717)), module, TRSPEAK::GAIN_PARAM));
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(8.091, 57.997)), module, TRSPEAK::ATTACK_PARAM));
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(20.541, 77.701)), module, TRSPEAK::RELEASE_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(8.795, 99.501)), module, TRSPEAK::IN_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.652, 99.499)), module, TRSPEAK::GATE_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(8.828, 113.501)), module, TRSPEAK::NONINV_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.685, 113.501)), module, TRSPEAK::INV_OUTPUT));
    }
};


Model *modelTRSPEAK = createModel<TRSPEAK, TRSPEAKWidget>("TRSPEAK");
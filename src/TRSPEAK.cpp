#include "trs.hpp"


struct TRSPEAK : Module {
    enum ParamIds {
        THRESH_PARAM,
        GAIN_PARAM,
        ATTACK_PARAM,
        MAKEUP_PARAM,
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

    TRSPEAK() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(THRESH_PARAM, 0.f, 1.f, 0.f, "");
        configParam(GAIN_PARAM, 0.f, 1.f, 0.f, "");
        configParam(ATTACK_PARAM, 0.f, 1.f, 0.f, "");
        configParam(MAKEUP_PARAM, 0.f, 1.f, 0.f, "");
    }

    void process(const ProcessArgs &args) override {
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
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(20.541, 77.701)), module, TRSPEAK::MAKEUP_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(8.795, 99.501)), module, TRSPEAK::IN_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.652, 99.499)), module, TRSPEAK::GATE_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(8.828, 113.501)), module, TRSPEAK::NONINV_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.685, 113.501)), module, TRSPEAK::INV_OUTPUT));
    }
};


Model *modelTRSPEAK = createModel<TRSPEAK, TRSPEAKWidget>("TRSPEAK");
#include "trs.hpp"


struct TRSSINCOS : Module {
    enum ParamIds {
        DEPTH_PARAM,
        BIAS_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        MONO_INPUT,
        STEREO_INPUT,
        DEPTH_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        OUT_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    TRSSINCOS() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(DEPTH_PARAM, 0.f, 1.f, 0.f, "");
        configParam(BIAS_PARAM, 0.f, 1.f, 0.f, "");
    }

    void process(const ProcessArgs &args) override {
    }
};


struct TRSSINCOSWidget : ModuleWidget {
    TRSSINCOSWidget(TRSSINCOS *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSSINCOS.svg")));

        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamGrey>(mm2px(Vec(10.731, 17.609)), module, TRSSINCOS::DEPTH_PARAM));
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(10.789, 38.717)), module, TRSSINCOS::BIAS_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(10.127, 71.508)), module, TRSSINCOS::MONO_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.16, 85.504)), module, TRSSINCOS::STEREO_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.16, 99.499)), module, TRSSINCOS::DEPTH_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.16, 113.501)), module, TRSSINCOS::OUT_OUTPUT));
    }
};


Model *modelTRSSINCOS = createModel<TRSSINCOS, TRSSINCOSWidget>("TRSSINCOS");
#include "plugin.hpp"


struct TRSATTENUATORS : Module {
    enum ParamIds {
        ATTL2_PARAM,
        ATTR2_PARAM,
        ATTL3_PARAM,
        ATTR3_PARAM,
        ATT4_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        CV1_INPUT,
        IN1_INPUT,
        TCV2_INPUT,
        IN2_INPUT,
        RCV2_INPUT,
        TCV3_INPUT,
        IN3_INPUT,
        RCV3_INPUT,
        IN4_INPUT,
        CV4_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        POS1_OUTPUT,
        NEG1_OUTPUT,
        POS2_OUTPUT,
        NEG2_OUTPUT,
        POS3_OUTPUT,
        NEG3_OUTPUT,
        POS4_OUTPUT,
        NEG4_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    TRSATTENUATORS() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(ATTL2_PARAM, 0.f, 1.f, 0.f, "");
        configParam(ATTR2_PARAM, 0.f, 1.f, 0.f, "");
        configParam(ATTL3_PARAM, 0.f, 1.f, 0.f, "");
        configParam(ATTR3_PARAM, 0.f, 1.f, 0.f, "");
        configParam(ATT4_PARAM, 0.f, 1.f, 0.f, "");
    }

    void process(const ProcessArgs &args) override {
    }
};


struct TRSATTENUATORSWidget : ModuleWidget {
    TRSATTENUATORSWidget(TRSATTENUATORS *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSATTENUATORS.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6.983, 45.603)), module, TRSATTENUATORS::ATTL2_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(16.386, 55.619)), module, TRSATTENUATORS::ATTR2_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(6.983, 72.77)), module, TRSATTENUATORS::ATTL3_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(16.385, 82.786)), module, TRSATTENUATORS::ATTR3_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(11.685, 106.25)), module, TRSATTENUATORS::ATT4_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.36, 17.443)), module, TRSATTENUATORS::CV1_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(52.335, 22.25)), module, TRSATTENUATORS::IN1_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(29.659, 45.603)), module, TRSATTENUATORS::TCV2_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(52.335, 50.25)), module, TRSATTENUATORS::IN2_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.062, 55.619)), module, TRSATTENUATORS::RCV2_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(29.659, 72.77)), module, TRSATTENUATORS::TCV3_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(52.335, 78.25)), module, TRSATTENUATORS::IN3_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.062, 82.786)), module, TRSATTENUATORS::RCV3_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(52.335, 106.25)), module, TRSATTENUATORS::IN4_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.36, 111.258)), module, TRSATTENUATORS::CV4_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.596, 15.25)), module, TRSATTENUATORS::POS1_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.596, 29.25)), module, TRSATTENUATORS::NEG1_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.596, 43.25)), module, TRSATTENUATORS::POS2_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.596, 57.25)), module, TRSATTENUATORS::NEG2_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.596, 71.25)), module, TRSATTENUATORS::POS3_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.596, 85.25)), module, TRSATTENUATORS::NEG3_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.596, 99.25)), module, TRSATTENUATORS::POS4_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.596, 113.25)), module, TRSATTENUATORS::NEG4_OUTPUT));
    }
};


Model *modelTRSATTENUATORS = createModel<TRSATTENUATORS, TRSATTENUATORSWidget>("TRSATTENUATORS");
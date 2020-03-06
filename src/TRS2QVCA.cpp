#include "trs.hpp"


struct TRS2QVCA : Module {
    enum ParamIds {
        MODE1_PARAM,
        LEVEL1_PARAM,
        MODE2_PARAM,
        LEVEL2_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        IN1_INPUT,
        ANTIIN1_INPUT,
        LEVEL1_INPUT,
        IN2_INPUT,
        ANTIIN2_INPUT,
        LEVEL2_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        OUTPUT1_OUTPUT,
        ANTIOUT1_OUTPUT,
        OUTPUT2_OUTPUT,
        ANTIOUT2_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    StereoInHandler in1;
    StereoInHandler antiIn1;
    StereoInHandler level1;
    StereoInHandler in2;
    StereoInHandler antiIn2;
    StereoInHandler level2;

    StereoOutHandler out1;
    StereoOutHandler antiOut1;
    StereoOutHandler out2;
    StereoOutHandler antiOut2;

    dsp::ClockDivider parseSwitches;

    TRS2QVCA() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(MODE1_PARAM, 0.f, 2.f, 2.f, "");
        configParam(LEVEL1_PARAM, 0.f, 5.f, 0.f, "");
        configParam(MODE2_PARAM, 0.f, 2.f, 2.f, "");
        configParam(LEVEL2_PARAM, 0.f, 5.f, 0.f, "");

        in1.configure(&inputs[IN1_INPUT]);
        antiIn1.configure(&inputs[ANTIIN1_INPUT]);
        level1.configure(&inputs[LEVEL1_INPUT]);
        in2.configure(&inputs[IN2_INPUT]);
        antiIn2.configure(&inputs[ANTIIN2_INPUT]);
        level2.configure(&inputs[LEVEL2_INPUT]);

        out1.configure(&outputs[OUTPUT1_OUTPUT]);
        antiOut1.configure(&outputs[ANTIOUT1_OUTPUT]);
        out2.configure(&outputs[OUTPUT2_OUTPUT]);
        antiOut2.configure(&outputs[ANTIOUT2_OUTPUT]);

        parseSwitches.setDivision(32);

        getCV1 = &TRS2QVCA::rectify;
        getCV2 = &TRS2QVCA::rectify;

    }

    float_4 (TRS2QVCA::*getCV1)(float_4 knob, float_4 cv);
    float_4 (TRS2QVCA::*getCV2)(float_4 knob, float_4 cv);

    float_4 rectify(float_4 knob, float_4 cv) {
        return clamp(abs(knob + cv), float_4(0.f), float_4(5.f));
    }
    float_4 clip(float_4 knob, float_4 cv) {
        return clamp(knob + cv, float_4(0.f), float_4(5.f));
    }
    float_4 scale(float_4 knob, float_4 cv) {
        return clamp((knob + cv + float_4(5.f)) / float_4(2.f) , float_4(0.f), float_4(5.f));
    }

    inline void assignSwitches(void) {

        int mode = (int) params[MODE1_PARAM].getValue();

        switch (mode) {
            case 2: getCV1 = &TRS2QVCA::rectify;
                break;
            case 1: getCV1 = &TRS2QVCA::clip;
                break;
            case 0: getCV1 = &TRS2QVCA::scale;
                break;
        }

        mode = (int) params[MODE2_PARAM].getValue();

        switch (mode) {
            case 2: getCV2 = &TRS2QVCA::rectify;
                break;
            case 1: getCV2 = &TRS2QVCA::clip;
                break;
            case 0: getCV2 = &TRS2QVCA::scale;
                break;
        }
    }


    void process(const ProcessArgs &args) override {

        if (parseSwitches.process()) {

            assignSwitches();

        }

        outputs[OUTPUT1_OUTPUT].setChannels(16);
        outputs[OUTPUT2_OUTPUT].setChannels(16);
        outputs[ANTIOUT1_OUTPUT].setChannels(16);
        outputs[ANTIOUT2_OUTPUT].setChannels(16);

        float_4 level1Knob = params[LEVEL1_PARAM].getValue();
        float_4 level2Knob = params[LEVEL2_PARAM].getValue();

        for (int polyChunk = 0; polyChunk < 2; polyChunk++) {

            float_4 control = (this->*getCV1)(level1Knob, level1.getLeft(polyChunk)) * float_4(0.2f);
            out1.setLeft(control * in1.getLeft(polyChunk) + (1 - control) * antiIn1.getLeft(polyChunk), polyChunk);
            antiOut1.setLeft(control * antiIn1.getLeft(polyChunk) + (1 - control) * in1.getLeft(polyChunk), polyChunk);

            control = (this->*getCV1)(level1Knob, level1.getRight(polyChunk)) * float_4(0.2f);
            out1.setRight(control * in1.getRight(polyChunk) + (1 - control) * antiIn1.getRight(polyChunk), polyChunk);
            antiOut1.setRight(control * antiIn1.getRight(polyChunk) + (1 - control) * in1.getRight(polyChunk), polyChunk);

            control = (this->*getCV2)(level2Knob, level2.getLeft(polyChunk)) * float_4(0.2f);
            out2.setLeft(control * in2.getLeft(polyChunk) + (1 - control) * antiIn2.getLeft(polyChunk), polyChunk);
            antiOut2.setLeft(control * antiIn2.getLeft(polyChunk) + (1 - control) * in2.getLeft(polyChunk), polyChunk);
                
            control = (this->*getCV2)(level2Knob, level2.getRight(polyChunk)) * float_4(0.2f);
            out2.setRight(control * in2.getRight(polyChunk) + (1 - control) * antiIn2.getRight(polyChunk), polyChunk);
            antiOut2.setRight(control * antiIn2.getRight(polyChunk) + (1 - control) * in2.getRight(polyChunk), polyChunk);

        }
    }
};


struct TRS2QVCAWidget : ModuleWidget {
    TRS2QVCAWidget(TRS2QVCA *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRS2QVCA.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<CKSSThree>(mm2px(Vec(7.492, 36.348)), module, TRS2QVCA::MODE1_PARAM));
        addParam(createParamCentered<SifamGrey>(mm2px(Vec(29.388, 36.443)), module, TRS2QVCA::LEVEL1_PARAM));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(7.492, 92.341)), module, TRS2QVCA::MODE2_PARAM));
        addParam(createParamCentered<SifamGrey>(mm2px(Vec(29.388, 92.436)), module, TRS2QVCA::LEVEL2_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(7.489, 15.529)), module, TRS2QVCA::IN1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(20.307, 15.529)), module, TRS2QVCA::ANTIIN1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(33.134, 15.529)), module, TRS2QVCA::LEVEL1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(7.489, 71.522)), module, TRS2QVCA::IN2_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(20.307, 71.522)), module, TRS2QVCA::ANTIIN2_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(33.134, 71.522)), module, TRS2QVCA::LEVEL2_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(7.512, 57.498)), module, TRS2QVCA::OUTPUT1_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(20.33, 57.498)), module, TRS2QVCA::ANTIOUT1_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(7.512, 113.491)), module, TRS2QVCA::OUTPUT2_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(20.33, 113.491)), module, TRS2QVCA::ANTIOUT2_OUTPUT));
    }
};


Model *modelTRS2QVCA = createModel<TRS2QVCA, TRS2QVCAWidget>("TRS2QVCA");
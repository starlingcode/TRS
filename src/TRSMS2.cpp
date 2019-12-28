#include "trs.hpp"


struct TRSMS2 : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        S1_INPUT,
        M1_INPUT,
        S2_INPUT,
        M2_INPUT,
        LR1_INPUT,
        LR2_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        LR1_OUTPUT,
        LR2_OUTPUT,
        S1_OUTPUT,
        M1_OUTPUT,
        S2_OUTPUT,
        M2_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    StereoInHandler s1In;
    StereoInHandler m1In;
    StereoInHandler s2In;
    StereoInHandler m2In;
    StereoInHandler lr1In;
    StereoInHandler lr2In;


    StereoOutHandler s1Out;
    StereoOutHandler m1Out;
    StereoOutHandler s2Out;
    StereoOutHandler m2Out;
    StereoOutHandler lr1Out;
    StereoOutHandler lr2Out;

    TRSMS2() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        s1In.configure(&inputs[S1_INPUT]);
        m1In.configure(&inputs[M1_INPUT]);
        lr1In.configure(&inputs[LR1_INPUT]);
        s2In.configure(&inputs[S2_INPUT]);
        m2In.configure(&inputs[M2_INPUT]);
        lr2In.configure(&inputs[LR2_INPUT]);

        s1Out.configure(&outputs[S1_OUTPUT]);
        m1Out.configure(&outputs[M1_OUTPUT]);
        lr1Out.configure(&outputs[LR1_OUTPUT]);
        s2Out.configure(&outputs[S2_OUTPUT]);
        m2Out.configure(&outputs[M2_OUTPUT]);
        lr2Out.configure(&outputs[LR2_OUTPUT]);

    }

    void process(const ProcessArgs &args) override {

        outputs[S1_OUTPUT].setChannels(8);
        outputs[M1_OUTPUT].setChannels(8);
        outputs[LR1_OUTPUT].setChannels(16);

        outputs[S2_OUTPUT].setChannels(8);
        outputs[M2_OUTPUT].setChannels(8);
        outputs[LR2_OUTPUT].setChannels(16);

        #define _MS_SCALE float_4(0.70710678118f)

        for (int polyChunk = 0; polyChunk < 2; polyChunk++) {

            s1Out.setLeft((lr1In.getLeft(polyChunk) - lr1In.getRight(polyChunk)) * _MS_SCALE, polyChunk);
            m1Out.setLeft((lr1In.getLeft(polyChunk) + lr1In.getRight(polyChunk)) * _MS_SCALE, polyChunk);

            s2Out.setLeft((lr2In.getLeft(polyChunk) - lr2In.getRight(polyChunk)) * _MS_SCALE, polyChunk);
            m2Out.setLeft((lr2In.getLeft(polyChunk) + lr2In.getRight(polyChunk)) * _MS_SCALE, polyChunk);

            lr1Out.setLeft((m1In.getLeft(polyChunk) + s1In.getLeft(polyChunk)) * _MS_SCALE, polyChunk);
            lr1Out.setRight((m1In.getLeft(polyChunk) - s1In.getLeft(polyChunk)) * _MS_SCALE, polyChunk);

            lr2Out.setLeft((m2In.getLeft(polyChunk) + s2In.getLeft(polyChunk)) * _MS_SCALE, polyChunk);
            lr2Out.setRight((m2In.getLeft(polyChunk) - s2In.getLeft(polyChunk)) * _MS_SCALE, polyChunk);

        }

    }
};


struct TRSMS2Widget : ModuleWidget {
    TRSMS2Widget(TRSMS2 *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSMS2.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH/2, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH/2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<HexJack>(mm2px(Vec(15.087, 25.386)), module, TRSMS2::S1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(5.081, 25.389)), module, TRSMS2::M1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(15.087, 53.609)), module, TRSMS2::S2_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(5.081, 53.612)), module, TRSMS2::M2_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.075, 85.481)), module, TRSMS2::LR1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.075, 113.66)), module, TRSMS2::LR2_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.076, 15.487)), module, TRSMS2::LR1_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.076, 43.709)), module, TRSMS2::LR2_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(15.07, 75.597)), module, TRSMS2::S1_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(5.08, 75.601)), module, TRSMS2::M1_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(15.07, 103.776)), module, TRSMS2::S2_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(5.08, 103.78)), module, TRSMS2::M2_OUTPUT));
    }
};


Model *modelTRSMS2 = createModel<TRSMS2, TRSMS2Widget>("TRSMS2");
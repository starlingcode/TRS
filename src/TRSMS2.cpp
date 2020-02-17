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

            float_4 s1O = (lr1In.getLeft(polyChunk) - lr1In.getRight(polyChunk)) * _MS_SCALE;
            float_4 m1O = (lr1In.getLeft(polyChunk) + lr1In.getRight(polyChunk)) * _MS_SCALE;

            s1Out.setLeft(s1O, polyChunk);
            m1Out.setLeft(m1O, polyChunk);

            float_4 s2O = (lr2In.getLeft(polyChunk) - lr2In.getRight(polyChunk)) * _MS_SCALE;
            float_4 m2O = (lr2In.getLeft(polyChunk) + lr2In.getRight(polyChunk)) * _MS_SCALE;

            s2Out.setLeft(s2O, polyChunk);
            m2Out.setLeft(m2O, polyChunk);

            lr1Out.setLeft((m1In.getLeftNormal(m1O, polyChunk) + s1In.getLeftNormal(s1O, polyChunk)) * _MS_SCALE, polyChunk);
            lr1Out.setRight((m1In.getLeftNormal(m1O, polyChunk) - s1In.getLeftNormal(s1O, polyChunk)) * _MS_SCALE, polyChunk);

            lr2Out.setLeft((m2In.getLeftNormal(m2O, polyChunk) + s2In.getLeftNormal(s2O, polyChunk)) * _MS_SCALE, polyChunk);
            lr2Out.setRight((m2In.getLeftNormal(m2O, polyChunk) - s2In.getLeftNormal(s2O, polyChunk)) * _MS_SCALE, polyChunk);

        }

    }
};


struct TRSMS2Widget : ModuleWidget {
    TRSMS2Widget(TRSMS2 *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSMS2.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH/2, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH/2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<HexJack>(mm2px(Vec(10.076, 15.487)), module, TRSMS2::LR1_INPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(15.087, 27.75)), module, TRSMS2::S1_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(5.081, 27.753)), module, TRSMS2::M1_OUTPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(15.096, 43.748)), module, TRSMS2::S1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(5.09, 43.752)), module, TRSMS2::M1_INPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.085, 56.252)), module, TRSMS2::LR1_OUTPUT));

        addInput(createInputCentered<HexJack>(mm2px(Vec(10.085, 72.25)), module, TRSMS2::LR2_INPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(15.096, 84.512)), module, TRSMS2::S2_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(5.107, 84.515)), module, TRSMS2::M2_OUTPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(15.08, 100.512)), module, TRSMS2::S2_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(5.09, 100.515)), module, TRSMS2::M2_INPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.085, 113.034)), module, TRSMS2::LR2_OUTPUT));
    }
};


Model *modelTRSMS2 = createModel<TRSMS2, TRSMS2Widget>("TRSMS2");
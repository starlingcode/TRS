#include "trs.hpp"


struct TRSMS : Module {
    enum ParamIds {
        WIDTH_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        LR_INPUT,
        MS_INPUT,
        IN_INPUT,
        DELAYCV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        MS_OUTPUT,
        LR_OUTPUT,
        OUT_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    StereoInHandler lrIn;
    StereoInHandler msIn;
    StereoInHandler signalIn;
    StereoInHandler delayCV;

    StereoOutHandler lrOut;
    StereoOutHandler msOut;
    StereoOutHandler signalOut;

    TRSMS() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(WIDTH_PARAM, 0.f, 1.f, 0.f, "");

        lrIn.configure(&inputs[LR_INPUT]);
        msIn.configure(&inputs[MS_INPUT]);
        signalIn.configure(&inputs[IN_INPUT]);
        delayCV.configure(&inputs[DELAYCV_INPUT]);

        lrOut.configure(&outputs[LR_OUTPUT]);
        msOut.configure(&outputs[MS_OUTPUT]);
        signalOut.configure(&outputs[OUT_OUTPUT]);
    } 

    void process(const ProcessArgs &args) override {

        outputs[LR_OUTPUT].setChannels(16);
        outputs[MS_OUTPUT].setChannels(16);
        outputs[OUT_OUTPUT].setChannels(16);

        for (int polyChunk = 0; polyChunk < 2; polyChunk++) {
            lrOut.setLeft(msIn.getLeft(polyChunk) + msIn.getRight(polyChunk), polyChunk);
            lrOut.setRight(msIn.getLeft(polyChunk) - msIn.getRight(polyChunk), polyChunk);

            msOut.setLeft(lrIn.getLeft(polyChunk) + lrIn.getRight(polyChunk), polyChunk);
            msOut.setRight(lrIn.getLeft(polyChunk) - lrIn.getRight(polyChunk), polyChunk);
        }

    }
};


struct TRSMSWidget : ModuleWidget {
    TRSMSWidget(TRSMS *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSMS.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH/2, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH/2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamBlack>(mm2px(Vec(9.845, 74.021)), module, TRSMS::WIDTH_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(10.16, 15.485)), module, TRSMS::LR_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.16, 43.665)), module, TRSMS::MS_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(5.08, 95.374)), module, TRSMS::IN_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.075, 113.675)), module, TRSMS::DELAYCV_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.076, 25.384)), module, TRSMS::MS_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.076, 53.57)), module, TRSMS::LR_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(15.07, 95.373)), module, TRSMS::OUT_OUTPUT));
    }
};


Model *modelTRSMS = createModel<TRSMS, TRSMSWidget>("TRSMS");
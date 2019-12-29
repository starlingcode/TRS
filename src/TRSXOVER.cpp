#include "trs.hpp"


struct TRSXOVER : Module {
    enum ParamIds {
        FREQ_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        SIGNAL_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        HIGH_OUTPUT,
        LOW_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    JOSSVF<float_4> filters[2][2];

    StereoInHandler in;
    StereoOutHandler high;
    StereoOutHandler low;


    TRSXOVER() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(FREQ_PARAM, 0.f, 1.f, 0.f, "");

        in.configure(&inputs[SIGNAL_INPUT]);
        high.configure(&outputs[HIGH_OUTPUT]);
        low.configure(&outputs[LOW_OUTPUT]);

    }

    void process(const ProcessArgs &args) override {

        outputs[HIGH_OUTPUT].setChannels(16);
        outputs[LOW_OUTPUT].setChannels(16);

        for (int polyChunk = 0; polyChunk < 2; polyChunk ++) {

            filters[0][polyChunk].process(params[FREQ_PARAM].getValue(), 0.f, in.getLeft(polyChunk), 0.f, 0.f, 0.f);
            filters[1][polyChunk].process(params[FREQ_PARAM].getValue(), 0.f, in.getRight(polyChunk), 0.f, 0.f, 0.f);

            high.setLeft(filters[0][polyChunk].hpOut, polyChunk);
            high.setRight(filters[1][polyChunk].hpOut, polyChunk);

            low.setLeft(filters[0][polyChunk].lpOut, polyChunk);
            low.setRight(filters[1][polyChunk].lpOut, polyChunk);


        }

    }
};


struct TRSXOVERWidget : ModuleWidget {
    TRSXOVERWidget(TRSXOVER *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSXOVER.svg")));

        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamBlack>(mm2px(Vec(10.76, 38.717)), module, TRSXOVER::FREQ_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(10.16, 85.742)), module, TRSXOVER::SIGNAL_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.16, 99.501)), module, TRSXOVER::HIGH_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.16, 113.498)), module, TRSXOVER::LOW_OUTPUT));
    }
};


Model *modelTRSXOVER = createModel<TRSXOVER, TRSXOVERWidget>("TRSXOVER");
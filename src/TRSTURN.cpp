#include "trs.hpp"


struct TRSTURN : Module {
    enum ParamIds {
        LEFT_PARAM,
        RIGHT_PARAM,
        BAL_PARAM,
        MERGE_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        PAN_INPUT,
        BAL_INPUT,
        MERGE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        PAN_OUTPUT,
        BAL_OUTPUT,
        MERGE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    StereoInHandler mergeIn;
    StereoOutHandler mergeOut;

    StereoInHandler balIn;
    StereoOutHandler balOut;

    StereoInHandler panIn;
    StereoOutHandler panOut;


    TRSTURN() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(MERGE_PARAM, 0.f, 1.f, 0.f, "");
        configParam(BAL_PARAM, 0.f, 1.f, 0.f, "");
        configParam(LEFT_PARAM, 0.f, 1.f, 0.f, "");
        configParam(RIGHT_PARAM, 0.f, 1.f, 0.f, "");

        mergeIn.configure(&inputs[MERGE_INPUT]);
        mergeOut.configure(&outputs[MERGE_OUTPUT]);

        balIn.configure(&inputs[BAL_INPUT]);
        balOut.configure(&outputs[BAL_OUTPUT]);

        panIn.configure(&inputs[PAN_INPUT]);
        panOut.configure(&outputs[PAN_OUTPUT]);

    }

    void process(const ProcessArgs &args) override {

        float_4 balance = float_4(params[BAL_PARAM].getValue());
        float_4 merge = float_4(params[MERGE_PARAM].getValue());
        float_4 leftPan = float_4(params[LEFT_PARAM].getValue());
        float_4 rightPan = float_4(params[RIGHT_PARAM].getValue());

        for (int polyChunk = 0; polyChunk < 2; polyChunk++) {

            float_4 left = mergeIn.getLeft(polyChunk);
            float_4 right = mergeIn.getRight(polyChunk);
            float_4 write = left * (float_4(1.f) - merge) + right * merge;
            mergeOut.setLeft(write, polyChunk);
            write = right * (float_4(1.f) - merge) + left * merge;
            mergeOut.setRight(write, polyChunk);

            left = balIn.getLeft(polyChunk);
            right = balIn.getRight(polyChunk);
            balOut.setLeft(left * (float_4(1.f) - balance), polyChunk);
            balOut.setRight(right * balance, polyChunk);

            left = panIn.getLeft(polyChunk);
            right = panIn.getRight(polyChunk);
            write = left * (float_4(1.f) - leftPan) + right * (float_4(1.f) - rightPan);
            panOut.setLeft(write, polyChunk);
            write = left * leftPan + right * rightPan;
            panOut.setRight(write, polyChunk);

        }

        outputs[MERGE_OUTPUT].setChannels(16);
        outputs[BAL_OUTPUT].setChannels(16);
        outputs[PAN_OUTPUT].setChannels(16);

    }
};


struct TRSTURNWidget : ModuleWidget {
    TRSTURNWidget(TRSTURN *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSTURN.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<Trimpot>(mm2px(Vec(8.939, 111.941)), module, TRSTURN::LEFT_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(21.764, 111.941)), module, TRSTURN::RIGHT_PARAM));
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(15.436, 73.755)), module, TRSTURN::BAL_PARAM));
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(15.35, 34.274)), module, TRSTURN::MERGE_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(8.952, 97.773)), module, TRSTURN::PAN_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(9.039, 57.063)), module, TRSTURN::BAL_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(8.954, 17.584)), module, TRSTURN::MERGE_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.777, 97.771)), module, TRSTURN::PAN_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.864, 57.065)), module, TRSTURN::BAL_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.778, 17.543)), module, TRSTURN::MERGE_OUTPUT));
    }
};


Model *modelTRSTURN = createModel<TRSTURN, TRSTURNWidget>("TRSTURN");
#include "trs.hpp"


struct TRSTS : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        L1_INPUT,
        R1_INPUT,
        R2_INPUT,
        L2_INPUT,
        LR1_INPUT,
        LR2_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        LR1_OUTPUT,
        LR2_OUTPUT,
        L1_OUTPUT,
        R1_OUTPUT,
        L2_OUTPUT,
        R2_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    StereoInHandler stereo1In;
    StereoInHandler stereo2In;
    StereoOutHandler stereo1Out;
    StereoOutHandler stereo2Out;

    int out1Channels = 1;
    int out2Channels = 1;

    TRSTS() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        stereo1In.configure(&this->inputs[LR1_INPUT]);
        stereo1Out.configure(&this->outputs[LR1_OUTPUT]);
        stereo2In.configure(&this->inputs[LR2_INPUT]);
        stereo2Out.configure(&this->outputs[LR2_OUTPUT]);
    }

    void process(const ProcessArgs &args) override {

        outputs[LR1_OUTPUT].setChannels(16);
        outputs[LR2_OUTPUT].setChannels(16);
        outputs[L1_OUTPUT].setChannels(out1Channels);
        outputs[L2_OUTPUT].setChannels(out2Channels);
        outputs[R1_OUTPUT].setChannels(out1Channels);
        outputs[R2_OUTPUT].setChannels(out2Channels);

        for (int polyChunk = 0; polyChunk < 2; polyChunk ++) {

            stereo1Out.setLeft(inputs[L1_INPUT].getNormalVoltageSimd<float_4>(inputs[R1_INPUT].getVoltageSimd<float_4>(4 * polyChunk), 4 * polyChunk), polyChunk);
            stereo1Out.setRight(inputs[R1_INPUT].getNormalVoltageSimd<float_4>(inputs[L1_INPUT].getVoltageSimd<float_4>(4 * polyChunk), 4 * polyChunk), polyChunk);

            stereo2Out.setLeft(inputs[L2_INPUT].getNormalVoltageSimd<float_4>(inputs[R2_INPUT].getVoltageSimd<float_4>(4 * polyChunk), 4 * polyChunk), polyChunk);
            stereo2Out.setRight(inputs[R2_INPUT].getNormalVoltageSimd<float_4>(inputs[L2_INPUT].getVoltageSimd<float_4>(4 * polyChunk), 4 * polyChunk), polyChunk);

            outputs[L1_OUTPUT].setVoltageSimd<float_4>(stereo1In.getLeft(polyChunk), 4 * polyChunk);
            outputs[R1_OUTPUT].setVoltageSimd<float_4>(stereo1In.getRight(polyChunk), 4 * polyChunk);

            outputs[L2_OUTPUT].setVoltageSimd<float_4>(stereo2In.getLeft(polyChunk), 4 * polyChunk);
            outputs[R2_OUTPUT].setVoltageSimd<float_4>(stereo2In.getRight(polyChunk), 4 * polyChunk);

        }

    }

    json_t* dataToJson() override {
        json_t* rootJ = json_object();
        json_object_set_new(rootJ, "channels1", json_integer(out1Channels));
        json_object_set_new(rootJ, "channels2", json_integer(out2Channels));
        return rootJ;
    }

    void dataFromJson(json_t* rootJ) override {
        json_t* channels1J = json_object_get(rootJ, "channels1");
        out1Channels = json_integer_value(channels1J);

        json_t* channels2J = json_object_get(rootJ, "channels2");
        out2Channels = json_integer_value(channels2J);
    }

};


struct TRSTSWidget : ModuleWidget {
    TRSTSWidget(TRSTS *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSTS.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH/2, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH/2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<HexJack>(mm2px(Vec(5.081, 15.245)), module, TRSTS::L1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(15.135, 15.245)), module, TRSTS::R1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(15.071, 43.748)), module, TRSTS::R2_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(5.081, 43.75)), module, TRSTS::L2_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.075, 72.248)), module, TRSTS::LR1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.075, 100.752)), module, TRSTS::LR2_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.076, 27.752)), module, TRSTS::LR1_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.076, 56.25)), module, TRSTS::LR2_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(5.08, 84.747)), module, TRSTS::L1_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(15.07, 84.748)), module, TRSTS::R1_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(5.016, 113.25)), module, TRSTS::L2_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(15.07, 113.25)), module, TRSTS::R2_OUTPUT));
    }
};


Model *modelTRSTS = createModel<TRSTS, TRSTSWidget>("TRSTS");
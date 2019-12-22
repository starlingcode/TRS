#include "trs.hpp"


struct AddSub : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        IN1_INPUT,
        IN2_INPUT,
        IN3_INPUT,
        IN4_INPUT,
        IN5_INPUT,
        IN6_INPUT,
        IN7_INPUT,
        IN8_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        OUT12_OUTPUT,
        OUT34_OUTPUT,
        OUT56_OUTPUT,
        OUT78_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    StereoInHandler stereo1In;
    StereoInHandler stereo2In;
    StereoInHandler stereo3In;
    StereoInHandler stereo4In;
    StereoInHandler stereo5In;
    StereoInHandler stereo6In;
    StereoInHandler stereo7In;
    StereoInHandler stereo8In;

    StereoOutHandler stereo12Out;
    StereoOutHandler stereo34Out;
    StereoOutHandler stereo56Out;
    StereoOutHandler stereo78Out;

    AddSub() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        stereo1In.configure(&inputs[IN1_INPUT]);
        stereo2In.configure(&inputs[IN2_INPUT]);
        stereo3In.configure(&inputs[IN3_INPUT]);
        stereo4In.configure(&inputs[IN4_INPUT]);
        stereo5In.configure(&inputs[IN5_INPUT]);
        stereo6In.configure(&inputs[IN6_INPUT]);
        stereo7In.configure(&inputs[IN7_INPUT]);
        stereo8In.configure(&inputs[IN8_INPUT]);

        stereo12Out.configure(&outputs[OUT12_OUTPUT]);
        stereo34Out.configure(&outputs[OUT34_OUTPUT]);
        stereo56Out.configure(&outputs[OUT56_OUTPUT]);
        stereo78Out.configure(&outputs[OUT78_OUTPUT]);

    }

    void process(const ProcessArgs &args) override {

        for (int polyChunk = 0; polyChunk < 2; polyChunk++) {

            stereo12Out.setLeft(stereo1In.getLeft() + stereo2In.getLeft(), polyChunk);
            stereo12Out.setRight(stereo1In.getRight() + stereo2In.getRight(), polyChunk);
            stereo34Out.setLeft(stereo3In.getLeft() + stereo4In.getLeft(), polyChunk);
            stereo34Out.setRight(stereo3In.getRight() + stereo4In.getRight(), polyChunk);
            stereo56Out.setLeft(stereo5In.getLeft() - stereo6In.getLeft(), polyChunk);
            stereo56Out.setRight(stereo5In.getRight() - stereo6In.getRight(), polyChunk);
            stereo78Out.setLeft(stereo7In.getLeft() - stereo8In.getLeft(), polyChunk);
            stereo78Out.setRight(stereo7In.getRight() - stereo8In.getRight(), polyChunk);

        }

        outputs[OUT12_OUTPUT].setChannels(16);
        outputs[OUT34_OUTPUT].setChannels(16);
        outputs[OUT56_OUTPUT].setChannels(16);
        outputs[OUT78_OUTPUT].setChannels(16);

    }
};


struct AddSubWidget : ModuleWidget {
    AddSubWidget(AddSub *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/AddSub.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH/2, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH/2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<HexJack>(mm2px(Vec(5.407, 15.247)), module, AddSub::IN1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(15.461, 15.247)), module, AddSub::IN2_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(5.407, 43.822)), module, AddSub::IN3_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(15.461, 43.822)), module, AddSub::IN4_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(5.406, 84.747)), module, AddSub::IN5_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(15.46, 84.747)), module, AddSub::IN6_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(5.406, 113.247)), module, AddSub::IN7_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(15.46, 113.247)), module, AddSub::IN8_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.401, 27.8)), module, AddSub::OUT12_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.401, 56.375)), module, AddSub::OUT34_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.401, 72.25)), module, AddSub::OUT56_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.401, 100.75)), module, AddSub::OUT78_OUTPUT));
    }
};


Model *modelAddSub = createModel<AddSub, AddSubWidget>("AddSub");
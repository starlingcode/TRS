#include "trs.hpp"


struct TRSOPS : Module {
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

    TRSOPS() {
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

            float_4 cv = stereo6In.getLeft()/5.f; 
            stereo56Out.setLeft((stereo5In.getLeft() * cv), polyChunk);

            cv = stereo6In.getRight()/5.f; 
            stereo56Out.setRight((stereo5In.getRight() * cv), polyChunk);

            cv = stereo8In.getLeft()/5.f; 
            stereo78Out.setLeft((stereo7In.getLeft() * cv), polyChunk);
            
            cv = stereo8In.getRight()/5.f; 
            stereo78Out.setRight((stereo7In.getRight() * cv), polyChunk);

        }

        outputs[OUT12_OUTPUT].setChannels(16);
        outputs[OUT34_OUTPUT].setChannels(16);
        outputs[OUT56_OUTPUT].setChannels(16);
        outputs[OUT78_OUTPUT].setChannels(16);

    }
};


struct TRSOPSWidget : ModuleWidget {
    TRSOPSWidget(TRSOPS *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSOPS.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH/2, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH/2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<HexJack>(mm2px(Vec(5.407, 15.247)), module, TRSOPS::IN1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(15.461, 15.247)), module, TRSOPS::IN2_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(5.407, 43.822)), module, TRSOPS::IN3_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(15.461, 43.822)), module, TRSOPS::IN4_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(5.406, 84.747)), module, TRSOPS::IN5_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(15.46, 84.747)), module, TRSOPS::IN6_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(5.406, 113.247)), module, TRSOPS::IN7_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(15.46, 113.247)), module, TRSOPS::IN8_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.401, 27.8)), module, TRSOPS::OUT12_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.401, 56.375)), module, TRSOPS::OUT34_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.401, 72.25)), module, TRSOPS::OUT56_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.401, 100.75)), module, TRSOPS::OUT78_OUTPUT));
    }
};


Model *modelTRSOPS = createModel<TRSOPS, TRSOPSWidget>("TRSOPS");
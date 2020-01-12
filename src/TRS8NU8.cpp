#include "trs.hpp"


struct TRS8NU8 : Module {
    enum ParamIds {
        ATT1_PARAM,
        ATTL2_PARAM,
        ATTR2_PARAM,
        ATTL3_PARAM,
        ATTR3_PARAM,
        ATT4_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        IN1_INPUT,
        IN2_INPUT,
        IN3_INPUT,
        IN4_INPUT,
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

    StereoInHandler in1;
    StereoInHandler in2;
    StereoInHandler in3;
    StereoInHandler in4;

    StereoOutHandler out1;
    StereoOutHandler out2;
    StereoOutHandler out3;
    StereoOutHandler out4;
    StereoOutHandler out1Inv;
    StereoOutHandler out2Inv;
    StereoOutHandler out3Inv;
    StereoOutHandler out4Inv;

    TRS8NU8() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(ATT1_PARAM, 0.f, 1.f, 0.f, "");
        configParam(ATTL2_PARAM, 0.f, 1.f, 0.f, "");
        configParam(ATTR2_PARAM, 0.f, 1.f, 0.f, "");
        configParam(ATTL3_PARAM, 0.f, 1.f, 0.f, "");
        configParam(ATTR3_PARAM, 0.f, 1.f, 0.f, "");
        configParam(ATT4_PARAM, 0.f, 1.f, 0.f, "");

        in1.configure(&inputs[IN1_INPUT]);
        in2.configure(&inputs[IN2_INPUT]);
        in3.configure(&inputs[IN3_INPUT]);
        in4.configure(&inputs[IN4_INPUT]);

        out1.configure(&outputs[POS1_OUTPUT]);
        out2.configure(&outputs[POS2_OUTPUT]);
        out3.configure(&outputs[POS3_OUTPUT]);
        out4.configure(&outputs[POS4_OUTPUT]);

        out1Inv.configure(&outputs[NEG1_OUTPUT]);
        out2Inv.configure(&outputs[NEG2_OUTPUT]);
        out3Inv.configure(&outputs[NEG3_OUTPUT]);
        out4Inv.configure(&outputs[NEG4_OUTPUT]);

    }

    ZenerClipperBL<float_4> shaper1[2];
    ZenerClipperBL<float_4> shaper2[2];

    void process(const ProcessArgs &args) override {

        outputs[POS1_OUTPUT].setChannels(16);
        outputs[POS2_OUTPUT].setChannels(16);
        outputs[POS3_OUTPUT].setChannels(16);
        outputs[POS4_OUTPUT].setChannels(16);
        outputs[NEG1_OUTPUT].setChannels(16);
        outputs[NEG2_OUTPUT].setChannels(16);
        outputs[NEG3_OUTPUT].setChannels(16);
        outputs[NEG4_OUTPUT].setChannels(16);

        float_4 att1 = float_4(params[ATT1_PARAM].getValue());
        float_4 att2L = float_4(params[ATTL2_PARAM].getValue());
        float_4 att2R = float_4(params[ATTR2_PARAM].getValue());
        float_4 att3L = float_4(params[ATTL3_PARAM].getValue());
        float_4 att3R = float_4(params[ATTR3_PARAM].getValue());
        float_4 att4 = float_4(params[ATT4_PARAM].getValue());

        for (int polyChunk = 0; polyChunk < 2; polyChunk++) {
            out1.setLeft(in1.getLeft(polyChunk) * att1, polyChunk);
            out1.setRight(in1.getRight(polyChunk) * att1, polyChunk);

            out2.setLeft(in2.getLeft(polyChunk) * att2L, polyChunk);
            out2.setRight(in2.getRight(polyChunk) * att2R, polyChunk);

            out3.setLeft(in3.getLeft(polyChunk) * att3L, polyChunk);
            out3.setRight(in3.getRight(polyChunk) * att3R, polyChunk);

            out4.setLeft(in4.getLeft(polyChunk) * att4, polyChunk);
            out4.setRight(in4.getRight(polyChunk) * att4, polyChunk);

            out1Inv.setLeft(in1.getLeft(polyChunk) * -att1, polyChunk);
            out1Inv.setRight(in1.getRight(polyChunk) * -att1, polyChunk);

            out2Inv.setLeft(in2.getLeft(polyChunk) * -att2L, polyChunk);
            out2Inv.setRight(in2.getRight(polyChunk) * -att2R, polyChunk);

            out3Inv.setLeft(in3.getLeft(polyChunk) * -att3L, polyChunk);
            out3Inv.setRight(in3.getRight(polyChunk) * -att3R, polyChunk);

            out4Inv.setLeft(in4.getLeft(polyChunk) * -att4, polyChunk);
            out4Inv.setRight(in4.getRight(polyChunk) * -att4, polyChunk);
        }

    }
};


struct TRS8NU8Widget : ModuleWidget {
    TRS8NU8Widget(TRS8NU8 *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRS8NU8.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamBlack>(mm2px(Vec(38.922, 22.451)), module, TRS8NU8::ATT1_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(6.983, 45.603)), module, TRS8NU8::ATTL2_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(16.433, 55.619)), module, TRS8NU8::ATTR2_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(6.983, 72.77)), module, TRS8NU8::ATTL3_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(16.385, 82.786)), module, TRS8NU8::ATTR3_PARAM));
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(38.978, 106.573)), module, TRS8NU8::ATT4_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(9.307, 22.485)), module, TRS8NU8::IN1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(29.903, 50.498)), module, TRS8NU8::IN2_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(29.903, 78.495)), module, TRS8NU8::IN3_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(9.307, 106.502)), module, TRS8NU8::IN4_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.568, 15.485)), module, TRS8NU8::POS1_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.567, 29.477)), module, TRS8NU8::NEG1_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(42.164, 43.498)), module, TRS8NU8::POS2_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(42.164, 57.491)), module, TRS8NU8::NEG2_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(42.164, 71.495)), module, TRS8NU8::POS3_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(42.164, 85.487)), module, TRS8NU8::NEG3_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.568, 99.502)), module, TRS8NU8::POS4_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.567, 113.495)), module, TRS8NU8::NEG4_OUTPUT));
    }
};


Model *modelTRS8NU8 = createModel<TRS8NU8, TRS8NU8Widget>("TRS8NU8");
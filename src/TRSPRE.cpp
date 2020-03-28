#include "trs.hpp"


struct TRSPRE : Module {
    enum ParamIds {
        GAIN1_PARAM,
        GAIN2_PARAM,
        GAIN3_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        IN1_INPUT,
        IN2_INPUT,
        IN3_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        OUT1_OUTPUT,
        OUT2_OUTPUT,
        OUT3_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        LOK1_LIGHT,
        ROK1_LIGHT,
        LCLIP1_LIGHT,
        RCLIP1_LIGHT,
        LOK2_LIGHT,
        ROK2_LIGHT,
        LCLIP2_LIGHT,
        RCLIP2_LIGHT,
        LOK3_LIGHT,
        ROK3_LIGHT,
        LCLIP3_LIGHT,
        RCLIP3_LIGHT,
        NUM_LIGHTS
    };

    StereoInHandler in1;
    StereoInHandler in2; 
    StereoInHandler in3;

    StereoOutHandler out1;
    StereoOutHandler out2;  
    StereoOutHandler out3;

    ZenerClipperBL<float_4> clippers[6]; 

    dsp::ClockDivider lightDivider;  

    TRSPRE() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(GAIN1_PARAM, 0.f, 4.f, 0.f, "");
        configParam(GAIN2_PARAM, 0.f, 4.f, 0.f, "");
        configParam(GAIN3_PARAM, 0.f, 4.f, 0.f, "");
        in1.configure(&inputs[IN1_INPUT]);
        in2.configure(&inputs[IN2_INPUT]);
        in3.configure(&inputs[IN3_INPUT]);
        out1.configure(&outputs[OUT1_OUTPUT]);
        out2.configure(&outputs[OUT2_OUTPUT]);
        out3.configure(&outputs[OUT3_OUTPUT]);
        lightDivider.setDivision(16);
    } 

    void process(const ProcessArgs &args) override {

        outputs[OUT1_OUTPUT].setChannels(16);
        outputs[OUT2_OUTPUT].setChannels(16);
        outputs[OUT3_OUTPUT].setChannels(16);

        for (int polyChunk = 0; polyChunk < 2; polyChunk++) {

            float_4 out = in1.getLeft(polyChunk) * params[GAIN1_PARAM].getValue();
            out /= 6.5f;
            out = clippers[0].process(out) * 6.5;
            out1.setLeft(out, polyChunk);

            out = in2.getLeft(polyChunk) * params[GAIN2_PARAM].getValue();
            out /= 6.5f;
            out = clippers[1].process(out) * 6.5;
            out2.setLeft(out, polyChunk);

            out = in3.getLeft(polyChunk) * params[GAIN3_PARAM].getValue();
            out /= 6.5f;
            out = clippers[2].process(out) * 6.5;
            out3.setLeft(out, polyChunk);

            out = in1.getRight(polyChunk) * params[GAIN1_PARAM].getValue();
            out /= 6.5f;
            out = clippers[3].process(out) * 6.5;
            out1.setRight(out, polyChunk);

            out = in2.getRight(polyChunk) * params[GAIN2_PARAM].getValue();
            out /= 6.5f;
            out = clippers[4].process(out) * 6.5;
            out2.setRight(out, polyChunk);

            out = in3.getRight(polyChunk) * params[GAIN3_PARAM].getValue();
            out /= 6.5f;
            out = clippers[5].process(out) * 6.5;
            out3.setRight(out, polyChunk);
        }

        if (lightDivider.process()) {

            float outl = abs(in1.getLeft() * params[GAIN1_PARAM].getValue()) / 5.f;
            float outr = abs(in1.getRight() * params[GAIN1_PARAM].getValue()) / 5.f;
            bool clippingl = outl > 1.f;
            bool clippingr = outr > 1.f;
            bool okl = outl > .2f;
            bool okr = outr > .2f;
            lights[LOK1_LIGHT].setSmoothBrightness(okl, 20.f/44100.f);
            lights[ROK1_LIGHT].setSmoothBrightness(okr, 20.f/44100.f);
            lights[LCLIP1_LIGHT].setSmoothBrightness(clippingl, 20.f/44100.f);
            lights[RCLIP1_LIGHT].setSmoothBrightness(clippingr, 20.f/44100.f);

            outl = abs(in2.getLeft() * params[GAIN2_PARAM].getValue()) / 5.f;
            outr = abs(in2.getRight() * params[GAIN2_PARAM].getValue()) / 5.f;
            clippingl = outl > 1.f;
            clippingr = outr > 1.f;
            okl = outl > .2f;
            okr = outr > .2f;
            lights[LOK2_LIGHT].setSmoothBrightness(okl, 20.f/44100.f);
            lights[ROK2_LIGHT].setSmoothBrightness(okr, 20.f/44100.f);
            lights[LCLIP2_LIGHT].setSmoothBrightness(clippingl, 20.f/44100.f);
            lights[RCLIP2_LIGHT].setSmoothBrightness(clippingr, 20.f/44100.f);

            outl = abs(in3.getLeft() * params[GAIN3_PARAM].getValue()) / 5.f;
            outr = abs(in3.getRight() * params[GAIN3_PARAM].getValue()) / 5.f;
            clippingl = outl > 1.f;
            clippingr = outr > 1.f;
            okl = outl > .2f;
            okr = outr > .2f;
            lights[LOK3_LIGHT].setSmoothBrightness(okl, 20.f/44100.f);
            lights[ROK3_LIGHT].setSmoothBrightness(okr, 20.f/44100.f);
            lights[LCLIP3_LIGHT].setSmoothBrightness(clippingl, 20.f/44100.f);
            lights[RCLIP3_LIGHT].setSmoothBrightness(clippingr, 20.f/44100.f);

        }

    }
};


struct TRSPREWidget : ModuleWidget {
    TRSPREWidget(TRSPRE *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSPRE.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamBlack>(mm2px(Vec(15.607, 14.25)), module, TRSPRE::GAIN1_PARAM));
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(15.607, 53.6)), module, TRSPRE::GAIN2_PARAM));
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(15.606, 92.235)), module, TRSPRE::GAIN3_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(9.225, 35.516)), module, TRSPRE::IN1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(9.224, 74.868)), module, TRSPRE::IN2_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(8.982, 113.502)), module, TRSPRE::IN3_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(22.049, 35.516)), module, TRSPRE::OUT1_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(22.049, 74.868)), module, TRSPRE::OUT2_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.806, 113.502)), module, TRSPRE::OUT3_OUTPUT));

        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(4.787, 11.237)), module, TRSPRE::LCLIP1_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(26.637, 11.237)), module, TRSPRE::RCLIP1_LIGHT));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(4.787, 17.253)), module, TRSPRE::LOK1_LIGHT));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(26.637, 17.253)), module, TRSPRE::ROK1_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(4.787, 50.587)), module, TRSPRE::LCLIP2_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(26.637, 50.587)), module, TRSPRE::RCLIP2_LIGHT));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(4.787, 56.603)), module, TRSPRE::LOK2_LIGHT));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(26.637, 56.603)), module, TRSPRE::ROK2_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(4.649, 86.588)), module, TRSPRE::LCLIP3_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(26.498, 86.588)), module, TRSPRE::RCLIP3_LIGHT));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(4.649, 92.246)), module, TRSPRE::LOK3_LIGHT));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(26.498, 92.246)), module, TRSPRE::ROK3_LIGHT));
    }
};


Model *modelTRSPRE = createModel<TRSPRE, TRSPREWidget>("TRSPRE");
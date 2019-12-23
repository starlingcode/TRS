#include "trs.hpp"

template <typename T = float>
struct AP1 {

    AP1() {}

    T process(T input) {
        T out1 = (input - d2) * a0 + d1;
        d1 = input;
        d2 = out1;
        return out1;
    }

    T d1 = T(0);
    T d2 = T(0);

    T a0 = T(0);

};

template <typename T = float>
struct fourPolePhaser {

    AP1<T> stage1;
    AP1<T> stage2;
    AP1<T> stage3;
    AP1<T> stage4;

    T feedback = T(0);

    fourPolePhaser() {}

    void setParams(T freq, T fb) {
        stage1.a0 = freq;
        stage2.a0 = freq;
        stage3.a0 = freq;
        stage4.a0 = freq;
        feedback = fb;
    }

    T process(T input) {
        T signal = stage1.process(input + feedback * -stage4.d2);
        signal = stage2.process(signal);
        signal = stage3.process(signal);
        signal = stage4.process(signal);
        return signal;
    }

};

struct TRSVCPH : Module {
    enum ParamIds {
        FB_PARAM,
        MIX_PARAM,
        CVAMT_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        IN_INPUT,
        CV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        WET_OUTPUT,
        MIX_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    StereoInHandler in;
    StereoInHandler cv;

    StereoOutHandler wet;
    StereoOutHandler mix;

    fourPolePhaser<float_4> phasers[2][2];

    TRSVCPH() {

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(FB_PARAM, 0.f, 1.f, 0.f, "");
        configParam(MIX_PARAM, 0.f, 1.f, 0.f, "");
        configParam(CVAMT_PARAM, 0.f, 1.f, 0.f, "");

        in.configure(&inputs[IN_INPUT]);
        cv.configure(&inputs[CV_INPUT]);

        wet.configure(&outputs[WET_OUTPUT]);
        mix.configure(&outputs[MIX_OUTPUT]);

    }

    void process(const ProcessArgs &args) override {

        outputs[WET_OUTPUT].setChannels(16);
        outputs[MIX_OUTPUT].setChannels(16);

        for (int polyChunk = 0; polyChunk < 2; polyChunk++) {

            float_4 inL = in.getLeft(polyChunk);
            float_4 inR = in.getRight(polyChunk);

            float_4 fb = float_4(params[FB_PARAM].getValue());
            float_4 cvDepth = float_4(params[CVAMT_PARAM].getValue());

            float_4 freq = clamp(cv.getLeft(polyChunk)/float_4(10.f) * cvDepth + 0.5f, float_4(0.f), float_4(1.f));
            phasers[0][polyChunk].setParams(freq, fb);
            float_4 phasedL = phasers[0][polyChunk].process(inL);

            freq = clamp(cv.getRight(polyChunk)/float_4(10.f) * cvDepth + 0.5f, float_4(0.f), float_4(1.f));
            phasers[1][polyChunk].setParams(freq, fb);
            float_4 phasedR = phasers[1][polyChunk].process(inR);

            wet.setLeft(phasedL, polyChunk);
            wet.setRight(phasedR, polyChunk);

            float_4 mixAmount = float_4(params[MIX_PARAM].getValue());

            mix.setLeft(phasedL + inL * mixAmount, polyChunk);
            mix.setRight(phasedR + inR * mixAmount, polyChunk);

        }

    }
};


struct TRSVCPHWidget : ModuleWidget {
    TRSVCPHWidget(TRSVCPH *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSVCPH.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamGrey>(mm2px(Vec(15.225, 17.609)), module, TRSVCPH::FB_PARAM));
        addParam(createParamCentered<SifamGrey>(mm2px(Vec(15.225, 41.109)), module, TRSVCPH::MIX_PARAM));
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(15.225, 64.609)), module, TRSVCPH::CVAMT_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(8.953, 99.471)), module, TRSVCPH::IN_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(21.777, 99.471)), module, TRSVCPH::CV_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(8.952, 113.523)), module, TRSVCPH::WET_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.777, 113.523)), module, TRSVCPH::MIX_OUTPUT));
    }
};


Model *modelTRSVCPH = createModel<TRSVCPH, TRSVCPHWidget>("TRSVCPH");
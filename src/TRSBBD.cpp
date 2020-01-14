#include "trs.hpp"


struct TRSBBD : Module {
    enum ParamIds {
        TIME_PARAM,
        FEEDBACK_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        FEEDBACK_INPUT,
        TIME_INPUT,
        SIGNAL_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        SIGNAL_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    BBD<float> bbds[2];

    float sr = 44100.f;

    #define BBD_OVERSAMPLE 4

    UpsamplePow2<BBD_OVERSAMPLE, float> upsamplers[2];
    DecimatePow2<BBD_OVERSAMPLE, float> decimators[2];

    float work[BBD_OVERSAMPLE];

    StereoInHandler fbIn;
    StereoInHandler timeIn;
    StereoInHandler signalIn;

    StereoOutHandler signalOut;

    TRSBBD() {

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(TIME_PARAM, 0.f, 1.f, 0.f, "");
        configParam(FEEDBACK_PARAM, 0.f, .75f, 0.f, "");

        fbIn.configure(&inputs[FEEDBACK_INPUT]);
        timeIn.configure(&inputs[TIME_INPUT]);
        signalIn.configure(&inputs[SIGNAL_INPUT]);

        signalOut.configure(&outputs[SIGNAL_OUTPUT]);

        onSampleRateChange();

    }

    float lastL = 0.f;
    float lastR = 0.f;

    void process(const ProcessArgs &args) override {

        outputs[SIGNAL_OUTPUT].setChannels(16);

        float timeCV = timeIn.getLeft();
        timeCV += 5.f;
        timeCV /= 10.f;
        timeCV = clamp(timeCV, 0.f, 1.f);
        timeCV += params[TIME_PARAM].getValue();
        timeCV = 14000.f * dsp::approxExp2_taylor5(timeCV * 3.f);

        float fb = clamp(params[FEEDBACK_PARAM].getValue() + fbIn.getLeft()/15.f, 0.f, .75f);
        float in = signalIn.getLeft() + lastL * fb;
        upsamplers[0].process(in);
        work[0] = bbds[0].process(upsamplers[0].output[0], timeCV);
        work[1] = bbds[0].process(upsamplers[0].output[1], timeCV);
        work[2] = bbds[0].process(upsamplers[0].output[2], timeCV);
        work[3] = bbds[0].process(upsamplers[0].output[3], timeCV);
        float out = decimators[0].process(work);
        // float out = bbds[0].process(in, timeCV);
        lastL = out;
        signalOut.setLeft(lastL);

        timeCV = timeIn.getRight();
        timeCV += 5.f;
        timeCV /= 10.f;
        timeCV = clamp(timeCV, 0.f, 1.f);
        timeCV += params[TIME_PARAM].getValue();
        timeCV = 14000.f * dsp::approxExp2_taylor5(timeCV * 3.f);

        fb = clamp(params[FEEDBACK_PARAM].getValue() + fbIn.getRight()/15.f, 0.f, .75f);
        in = signalIn.getRight() + lastR * fb;
        upsamplers[1].process(in);
        work[0] = bbds[1].process(upsamplers[1].output[0], timeCV);
        work[1] = bbds[1].process(upsamplers[1].output[1], timeCV);
        work[2] = bbds[1].process(upsamplers[1].output[2], timeCV);
        work[3] = bbds[1].process(upsamplers[1].output[3], timeCV);
        out = decimators[1].process(work);
        // out = bbds[1].process(in, timeCV);
        lastR = out;
        signalOut.setRight(lastR);

    }

    void onSampleRateChange() override {
        
        sr = APP->engine->getSampleTime();

        bbds[0].reformFilters(sr / 4.f);
        bbds[1].reformFilters(sr / 4.f);

    }

};


struct TRSBBDWidget : ModuleWidget {
    TRSBBDWidget(TRSBBD *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSBBD.svg")));

        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamGrey>(mm2px(Vec(10.76, 23.241)), module, TRSBBD::TIME_PARAM));
        addParam(createParamCentered<SifamGrey>(mm2px(Vec(10.76, 46.741)), module, TRSBBD::FEEDBACK_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(10.127, 71.508)), module, TRSBBD::FEEDBACK_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.126, 85.506)), module, TRSBBD::TIME_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.16, 99.499)), module, TRSBBD::SIGNAL_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.16, 113.501)), module, TRSBBD::SIGNAL_OUTPUT));
    }
};


Model *modelTRSBBD = createModel<TRSBBD, TRSBBDWidget>("TRSBBD");

// echo

struct TRSBBDLONG : Module {
    enum ParamIds {
        TIME_PARAM,
        FEEDBACK_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        FEEDBACK_INPUT,
        TIME_INPUT,
        SIGNAL_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        SIGNAL_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    BBD<float, 20000> bbds[2];

    float sr = 44100.f;

    #define BBD_OVERSAMPLE 4

    UpsamplePow2<BBD_OVERSAMPLE, float> upsamplers[2];
    DecimatePow2<BBD_OVERSAMPLE, float> decimators[2];

    float work[BBD_OVERSAMPLE];

    StereoInHandler fbIn;
    StereoInHandler timeIn;
    StereoInHandler signalIn;

    StereoOutHandler signalOut;

    TRSBBDLONG() {

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(TIME_PARAM, 0.f, 1.f, 0.f, "");
        configParam(FEEDBACK_PARAM, 0.f, .75f, 0.f, "");

        fbIn.configure(&inputs[FEEDBACK_INPUT]);
        timeIn.configure(&inputs[TIME_INPUT]);
        signalIn.configure(&inputs[SIGNAL_INPUT]);

        signalOut.configure(&outputs[SIGNAL_OUTPUT]);

        onSampleRateChange();

    }

    float lastL = 0.f;
    float lastR = 0.f;

    void process(const ProcessArgs &args) override {

        outputs[SIGNAL_OUTPUT].setChannels(16);

        float timeCV = timeIn.getLeft();
        timeCV += 5.f;
        timeCV /= 10.f;
        timeCV = clamp(timeCV, 0.f, 1.f);
        timeCV += params[TIME_PARAM].getValue();
        timeCV = 14000.f * dsp::approxExp2_taylor5(timeCV * 3.f);

        float fb = clamp(params[FEEDBACK_PARAM].getValue() + fbIn.getLeft()/15.f, 0.f, .75f);
        float in = signalIn.getLeft() + lastL * fb;
        upsamplers[0].process(in);
        work[0] = bbds[0].process(upsamplers[0].output[0], timeCV);
        work[1] = bbds[0].process(upsamplers[0].output[1], timeCV);
        work[2] = bbds[0].process(upsamplers[0].output[2], timeCV);
        work[3] = bbds[0].process(upsamplers[0].output[3], timeCV);
        float out = decimators[0].process(work);
        // float out = bbds[0].process(in, timeCV);
        lastL = out;
        signalOut.setLeft(lastL);

        timeCV = timeIn.getRight();
        timeCV += 5.f;
        timeCV /= 10.f;
        timeCV = clamp(timeCV, 0.f, 1.f);
        timeCV += params[TIME_PARAM].getValue();
        timeCV = 14000.f * dsp::approxExp2_taylor5(timeCV * 3.f);

        fb = clamp(params[FEEDBACK_PARAM].getValue() + fbIn.getRight()/15.f, 0.f, .75f);
        in = signalIn.getRight() + lastR * fb;
        upsamplers[1].process(in);
        work[0] = bbds[1].process(upsamplers[0].output[0], timeCV);
        work[1] = bbds[1].process(upsamplers[0].output[1], timeCV);
        work[2] = bbds[1].process(upsamplers[0].output[2], timeCV);
        work[3] = bbds[1].process(upsamplers[0].output[3], timeCV);
        out = decimators[1].process(work);
        // out = bbds[1].process(in, timeCV);
        lastR = out;
        signalOut.setRight(lastR);

    }

    void onSampleRateChange() override {
        
        sr = APP->engine->getSampleTime();

        bbds[0].reformFilters(sr / 4.f);
        bbds[1].reformFilters(sr / 4.f);

    }

};


struct TRSBBDLONGWidget : ModuleWidget {
    TRSBBDLONGWidget(TRSBBDLONG *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSBBDLONG.svg")));

        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamGrey>(mm2px(Vec(10.76, 23.241)), module, TRSBBD::TIME_PARAM));
        addParam(createParamCentered<SifamGrey>(mm2px(Vec(10.76, 46.741)), module, TRSBBD::FEEDBACK_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(10.127, 71.508)), module, TRSBBD::FEEDBACK_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.126, 85.506)), module, TRSBBD::TIME_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.16, 99.499)), module, TRSBBD::SIGNAL_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.16, 113.501)), module, TRSBBD::SIGNAL_OUTPUT));
    }
};


Model *modelTRSBBDLONG = createModel<TRSBBDLONG, TRSBBDLONGWidget>("TRSBBDLONG");
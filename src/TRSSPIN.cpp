#include "trs.hpp"


struct TRSSPIN : Module {
    enum ParamIds {
        RATE1_PARAM,
        RATE2_PARAM,
        RATE1_ATTEN_PARAM,
        RATE2_ATTEN_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        RATE1_INPUT,
        RATE2_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        OUT1POS_OUTPUT,
        OUT1NEG_OUTPUT,
        OUT2NEG_OUTPUT,
        OUT2POS_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    StereoInHandler topLFORate;
    StereoInHandler bottomLFORate;

    StereoOutHandler topLFO12Out;
    StereoOutHandler topLFO34Out;
    StereoOutHandler bottomLFO12Out;
    StereoOutHandler bottomLFO34Out;

    float_4 topLFOLeadPhase[2] = {float_4(0), float_4(1)};
    float_4 bottomLFOLeadPhase[2] = {float_4(0), float_4(1)};

    TRSSPIN() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(RATE1_PARAM, .0f, 12.f, 0.f, "");
        configParam(RATE2_PARAM, .0f, 12.f, 0.f, "");
        configParam(RATE1_ATTEN_PARAM, 0.f, 1.f, 0.f, "");
        configParam(RATE2_ATTEN_PARAM, 0.f, 1.f, 0.f, "");

        topLFORate.configure(&inputs[RATE1_INPUT]);
        bottomLFORate.configure(&inputs[RATE2_INPUT]);

        topLFO12Out.configure(&outputs[OUT1POS_OUTPUT]);
        topLFO34Out.configure(&outputs[OUT1NEG_OUTPUT]);
        bottomLFO12Out.configure(&outputs[OUT2POS_OUTPUT]);
        bottomLFO34Out.configure(&outputs[OUT2NEG_OUTPUT]);

    }

    void process(const ProcessArgs &args) override {

        float_4 sr = float_4(APP->engine->getSampleRate());

        float_4 baseRateTop = dsp::approxExp2_taylor5(float_4(params[RATE1_PARAM].getValue())) * float_4(.01f)/sr;
        float_4 baseRateBottom = dsp::approxExp2_taylor5(float_4(params[RATE2_PARAM].getValue())) * float_4(.01f)/sr;

        for (int polyChunk = 0; polyChunk < 2; polyChunk++) {

            float_4 topPhase = topLFOLeadPhase[polyChunk];
            float_4 bottomPhase = bottomLFOLeadPhase[polyChunk];

            float_4 rate = dsp::approxExp2_taylor5(topLFORate.getLeft(polyChunk) * params[RATE1_ATTEN_PARAM].getValue() + float_4(5.f)) * baseRateTop / float_4(32.f);
            topPhase += rate;
            topPhase -= (topPhase >= 1.f) & 1.f;

            rate = dsp::approxExp2_taylor5(bottomLFORate.getLeft(polyChunk) * params[RATE2_ATTEN_PARAM].getValue() + float_4(5.f)) * baseRateBottom / float_4(32.f);
            bottomPhase += rate;
            bottomPhase -= (bottomPhase >= 1.f) & 1.f;

            topLFOLeadPhase[polyChunk] = topPhase;
            bottomLFOLeadPhase[polyChunk] = bottomPhase;

            float_4 topQuadrature = topPhase + float_4(.25f);
            topQuadrature -= (topQuadrature >= 1.f) & 1.f;

            float_4 bottomQuadrature = bottomPhase + float_4(.25f);
            bottomQuadrature -= (bottomQuadrature >= 1.f) & 1.f;

            // replace with approx
            topPhase = bhaskaraSine<float_4, int32_4>(topPhase * float_4(2.f) - float_4(1.f)) * 5.f;
            topQuadrature = bhaskaraSine<float_4, int32_4>(topQuadrature * float_4(2.f) - float_4(1.f)) * 5.f;
            bottomPhase = bhaskaraSine<float_4, int32_4>(bottomPhase * float_4(2.f) - float_4(1.f)) * 5.f;
            bottomQuadrature = bhaskaraSine<float_4, int32_4>(bottomQuadrature * float_4(2.f) - float_4(1.f)) * 5.f;

            topLFO12Out.setLeft(topPhase, polyChunk);
            topLFO12Out.setRight(topQuadrature, polyChunk);
            topLFO34Out.setLeft(-topPhase, polyChunk);
            topLFO34Out.setRight(-topQuadrature, polyChunk);

            bottomLFO12Out.setLeft(bottomPhase, polyChunk);
            bottomLFO12Out.setRight(bottomQuadrature, polyChunk);
            bottomLFO34Out.setLeft(-bottomPhase, polyChunk);
            bottomLFO34Out.setRight(-bottomQuadrature, polyChunk);

        }

        outputs[OUT1POS_OUTPUT].setChannels(16);
        outputs[OUT2POS_OUTPUT].setChannels(16);
        outputs[OUT1NEG_OUTPUT].setChannels(16);
        outputs[OUT2NEG_OUTPUT].setChannels(16);

    }
};


struct TRSSPINWidget : ModuleWidget {
    TRSSPINWidget(TRSSPIN *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSSPIN.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamGrey>(mm2px(Vec(14.363, 35.878)), module, TRSSPIN::RATE1_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(8.311, 50.527)), module, TRSSPIN::RATE1_ATTEN_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(21.168, 76.262)), module, TRSSPIN::RATE2_ATTEN_PARAM));
        addParam(createParamCentered<SifamGrey>(mm2px(Vec(15.295, 91.258)), module, TRSSPIN::RATE2_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(21.168, 56.468)), module, TRSSPIN::RATE1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(8.311, 71.34)), module, TRSSPIN::RATE2_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(8.311, 15.489)), module, TRSSPIN::OUT1POS_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.168, 15.489)), module, TRSSPIN::OUT1NEG_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(8.311, 113.496)), module, TRSSPIN::OUT2POS_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.168, 113.496)), module, TRSSPIN::OUT2NEG_OUTPUT));
    }
};

Model *modelTRSSPIN = createModel<TRSSPIN, TRSSPINWidget>("TRSSPIN");

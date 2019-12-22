#include "trs.hpp"


struct QuadLFO : Module {
    enum ParamIds {
        RATE1_PARAM,
        RATE2_PARAM,
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
        OUT12NEG_OUTPUT,
        OUT12POS_OUTPUT,
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
    StereoOutHandler mixLFO12Out;
    StereoOutHandler mixLFO34Out;

    float_4 topLFOLeadPhase[2] = {float_4(0), float_4(1)};
    float_4 bottomLFOLeadPhase[2] = {float_4(0), float_4(1)};

    QuadLFO() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(RATE1_PARAM, .0f, 12.f, 0.f, "");
        configParam(RATE2_PARAM, .0f, 12.f, 0.f, "");

        topLFORate.configure(&inputs[RATE1_INPUT]);
        bottomLFORate.configure(&inputs[RATE2_INPUT]);

        topLFO12Out.configure(&outputs[OUT1POS_OUTPUT]);
        topLFO34Out.configure(&outputs[OUT1NEG_OUTPUT]);
        bottomLFO12Out.configure(&outputs[OUT2POS_OUTPUT]);
        bottomLFO34Out.configure(&outputs[OUT2POS_OUTPUT]);
        mixLFO12Out.configure(&outputs[OUT12POS_OUTPUT]);
        mixLFO34Out.configure(&outputs[OUT12NEG_OUTPUT]);
    }

    void process(const ProcessArgs &args) override {

        float_4 sr = float_4(APP->engine->getSampleRate());

        float_4 baseRateTop = dsp::approxExp2_taylor5(float_4(params[RATE1_PARAM].getValue())) * float_4(.01f)/sr;
        float_4 baseRateBottom = dsp::approxExp2_taylor5(float_4(params[RATE2_PARAM].getValue())) * float_4(.01f)/sr;

        for (int polyChunk = 0; polyChunk < 2; polyChunk++) {

            float_4 topPhase = topLFOLeadPhase[polyChunk];
            float_4 bottomPhase = bottomLFOLeadPhase[polyChunk];

            float_4 rate = dsp::approxExp2_taylor5(topLFORate.getLeft(polyChunk) + float_4(5.f)) * baseRateTop / float_4(32.f);
            topPhase += rate;
            topPhase -= (topPhase >= 1.f) & 1.f;

            rate = dsp::approxExp2_taylor5(bottomLFORate.getLeft(polyChunk) + float_4(5.f)) * baseRateBottom / float_4(32.f);
            bottomPhase += rate;
            bottomPhase -= (bottomPhase >= 1.f) & 1.f;

            topLFOLeadPhase[polyChunk] = topPhase;
            bottomLFOLeadPhase[polyChunk] = bottomPhase;

            float_4 topQuadrature = topPhase + float_4(.25f);
            topQuadrature -= (topQuadrature >= 1.f) & 1.f;

            float_4 bottomQuadrature = bottomPhase + float_4(.25f);
            bottomQuadrature -= (bottomQuadrature >= 1.f) & 1.f;

            topPhase = simd::sin(2 * M_PI * topPhase) * 5.f;
            topQuadrature = simd::sin(2 * M_PI * topQuadrature) * 5.f;
            bottomPhase = simd::sin(2 * M_PI * bottomPhase) * 5.f;
            bottomQuadrature = simd::sin(2 * M_PI * bottomQuadrature) * 5.f;

            topLFO12Out.setLeft(topPhase, polyChunk);
            topLFO12Out.setRight(topQuadrature, polyChunk);
            topLFO34Out.setLeft(-topPhase, polyChunk);
            topLFO34Out.setRight(-topQuadrature, polyChunk);

            bottomLFO12Out.setLeft(bottomPhase, polyChunk);
            bottomLFO12Out.setRight(bottomQuadrature, polyChunk);
            bottomLFO34Out.setLeft(-bottomPhase, polyChunk);
            bottomLFO34Out.setRight(-bottomQuadrature, polyChunk);

            mixLFO12Out.setLeft(topPhase, polyChunk);
            mixLFO12Out.setRight(bottomPhase, polyChunk);
            mixLFO34Out.setLeft(topQuadrature, polyChunk);
            mixLFO34Out.setRight(bottomQuadrature, polyChunk);

        }

        outputs[OUT1POS_OUTPUT].setChannels(16);
        outputs[OUT2POS_OUTPUT].setChannels(16);
        outputs[OUT1NEG_OUTPUT].setChannels(16);
        outputs[OUT2NEG_OUTPUT].setChannels(16);
        outputs[OUT12POS_OUTPUT].setChannels(16);
        outputs[OUT12NEG_OUTPUT].setChannels(16);

    }
};


struct QuadLFOWidget : ModuleWidget {
    QuadLFOWidget(QuadLFO *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/QuadLFO.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamBlack>(mm2px(Vec(14.363, 17.609)), module, QuadLFO::RATE1_PARAM));
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(15.24, 105.246)), module, QuadLFO::RATE2_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(14.74, 48.541)), module, QuadLFO::RATE1_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(15.24, 74.313)), module, QuadLFO::RATE2_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(8.561, 34.542)), module, QuadLFO::OUT1POS_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.168, 34.544)), module, QuadLFO::OUT1NEG_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.168, 61.254)), module, QuadLFO::OUT12NEG_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(8.311, 61.325)), module, QuadLFO::OUT12POS_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.168, 88.242)), module, QuadLFO::OUT2NEG_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(8.311, 88.313)), module, QuadLFO::OUT2POS_OUTPUT));
    }
};


Model *modelQuadLFO = createModel<QuadLFO, QuadLFOWidget>("QuadLFO");
#include "trs.hpp"


struct TRSSIN : Module {
    enum ParamIds {
        LEVEL_PARAM,
        PHASE_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        LEVEL_INPUT,
        STEREO_INPUT,
        PHASE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        OUT_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    StereoInHandler level;
    StereoInHandler phase;
    StereoInHandler stereo;
    
    StereoOutHandler output;

    #define SINCOS_OVERSAMPLE 4

    UpsamplePow2<SINCOS_OVERSAMPLE, float_4> upsamplers[2][2];
    DecimatePow2<SINCOS_OVERSAMPLE, float_4> decimators[2][2];

    float_4 work[SINCOS_OVERSAMPLE];

    TRSSIN() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(LEVEL_PARAM, 0.f, 5.f, 0.f, "");
        configParam(PHASE_PARAM, 0.f, 5.f, 0.f, "");

        stereo.configure(&inputs[STEREO_INPUT]);
        phase.configure(&inputs[LEVEL_INPUT]);
        level.configure(&inputs[PHASE_INPUT]);

        output.configure(&outputs[OUT_OUTPUT]);

    }

    void process(const ProcessArgs &args) override {

        outputs[OUT_OUTPUT].setChannels(16);

        for (int polyChunk = 0; polyChunk < 2; polyChunk ++) {

            float_4 level_value = clamp((level.getLeft(polyChunk)) + float_4(params[LEVEL_PARAM].getValue()), 0.f, 10.f);
            float_4 in = stereo.getLeft(polyChunk) + params[PHASE_PARAM].getValue() + phase.getLeft(polyChunk);
            // scale -5 to -5 to -2 to -2
            in *= float_4(2.f / 5.f);
            upsamplers[0][polyChunk].process(in);
            for (int i = 0; i < SINCOS_OVERSAMPLE; i++) {
                in = upsamplers[0][polyChunk].output[i];
                work[i] = bhaskaraSine<float_4, int32_4>(in);
            }
            float_4 out = decimators[0][polyChunk].process(work);            
            output.setLeft(out * level_value, polyChunk);

            in = stereo.getRight(polyChunk) - params[PHASE_PARAM].getValue() - phase.getLeft(polyChunk);
            // scale -5 to -5 to -2 to -2
            in *= float_4(2.f / 5.f);
            // cos
            upsamplers[1][polyChunk].process(in);
            for (int i = 0; i < SINCOS_OVERSAMPLE; i++) {
                in = upsamplers[1][polyChunk].output[i];
                work[i] = bhaskaraSine<float_4, int32_4>(in);
            }
            out = decimators[1][polyChunk].process(work);            
            output.setRight(out * level_value, polyChunk);

        }

    }
};


struct TRSSINWidget : ModuleWidget {
    TRSSINWidget(TRSSIN *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSSIN.svg")));

        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamBlack>(mm2px(Vec(10.16, 17.609)), module, TRSSIN::LEVEL_PARAM));
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(10.16, 38.717)), module, TRSSIN::PHASE_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(10.16, 71.508)), module, TRSSIN::LEVEL_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.16, 85.504)), module, TRSSIN::PHASE_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.16, 99.499)), module, TRSSIN::STEREO_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.16, 113.501)), module, TRSSIN::OUT_OUTPUT));
    }
};


Model *modelTRSSIN = createModel<TRSSIN, TRSSINWidget>("TRSSIN");

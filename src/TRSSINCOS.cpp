#include "trs.hpp"


struct TRSSINCOS : Module {
    enum ParamIds {
        DEPTH_PARAM,
        BIAS_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        MONO_INPUT,
        STEREO_INPUT,
        DEPTH_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        OUT_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    StereoInHandler mono;
    StereoInHandler stereo;
    StereoInHandler depthCV;
    
    StereoOutHandler output;

    // phase 0 - pi

    float_4 bhaskaraSine(float_4 phase) {

        float_4 pi = float_4(M_PI);

        return (float_4(16.f) * phase * (pi - phase)) / (float_4(5.f) * pi * pi - float_4(4.f) * phase * (pi - phase));
    }

    #define SINCOS_OVERSAMPLE 4

    UpsamplePow2<SINCOS_OVERSAMPLE, float_4> upsamplers[2][2];
    DecimatePow2<SINCOS_OVERSAMPLE, float_4> decimators[2][2];

    float_4 work[SINCOS_OVERSAMPLE];

    TRSSINCOS() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(DEPTH_PARAM, 0.f, 1.f, 0.f, "");
        configParam(BIAS_PARAM, 0.f, 5.f, 0.f, "");

        mono.configure(&inputs[MONO_INPUT]);
        stereo.configure(&inputs[STEREO_INPUT]);
        depthCV.configure(&inputs[DEPTH_INPUT]);

        output.configure(&outputs[OUT_OUTPUT]);

    }

    void process(const ProcessArgs &args) override {

        outputs[OUT_OUTPUT].setChannels(16);

        for (int polyChunk = 0; polyChunk < 2; polyChunk ++) {

            float_4 depth = clamp((depthCV.getLeft(polyChunk) / float_4(10.f)) + params[DEPTH_PARAM].getValue(), 0.f, 1.f);
            float_4 in = mono.getLeft(polyChunk) + stereo.getLeft(polyChunk) + params[BIAS_PARAM].getValue();
            in *= depth;

            // scale -5 to -5 to -2 to -2
            in *= float_4(2.f / 5.f);
            upsamplers[0][polyChunk].process(in);
            for (int i = 0; i < SINCOS_OVERSAMPLE; i++) {
                in = upsamplers[0][polyChunk].output[i];
                int32_4 phaseHalf = abs((int32_4) floor(in));
                phaseHalf &= int32_4(1);
                float_4 sign = float_4(1.f) - (float_4(2.f) * float_4(phaseHalf));
                float_4 out = in - floor(in);
                work[i] = bhaskaraSine(float_4(M_PI) * out) * sign;
            }
            float_4 out = decimators[0][polyChunk].process(work);            
            output.setLeft(out * float_4(5.f), polyChunk);

            depth =  clamp((depthCV.getRight(polyChunk) / float_4(5.f)) + params[DEPTH_PARAM].getValue(), 0.f, 1.f);
            in = mono.getLeft(polyChunk) + stereo.getRight(polyChunk) + params[BIAS_PARAM].getValue();
            in *= depth;

            // scale -5 to -5 to -2 to -2
            in *= float_4(2.f / 5.f);
            // cos
            in += float_4(.5f);
            upsamplers[1][polyChunk].process(in);
            for (int i = 0; i < SINCOS_OVERSAMPLE; i++) {
                in = upsamplers[1][polyChunk].output[i];
                int32_4 phaseHalf = abs((int32_4) floor(in));
                phaseHalf &= int32_4(1);
                float_4 sign = float_4(1.f) - (float_4(2.f) * float_4(phaseHalf));
                float_4 out = in - floor(in);
                work[i] = bhaskaraSine(float_4(M_PI) * out) * sign;
            }
            out = decimators[1][polyChunk].process(work);            
            output.setRight(out * float_4(5.f), polyChunk);

        }

    }
};


struct TRSSINCOSWidget : ModuleWidget {
    TRSSINCOSWidget(TRSSINCOS *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSSINCOS.svg")));

        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamGrey>(mm2px(Vec(10.731, 17.609)), module, TRSSINCOS::DEPTH_PARAM));
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(10.789, 38.717)), module, TRSSINCOS::BIAS_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(10.127, 71.508)), module, TRSSINCOS::MONO_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.16, 85.504)), module, TRSSINCOS::STEREO_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.16, 99.499)), module, TRSSINCOS::DEPTH_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.16, 113.501)), module, TRSSINCOS::OUT_OUTPUT));
    }
};


Model *modelTRSSINCOS = createModel<TRSSINCOS, TRSSINCOSWidget>("TRSSINCOS");
#include "trs.hpp"


struct TRSMULTMETER : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        TOP_INPUT,
        BOTTOM_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        TOP1_OUTPUT,
        TOP2_OUTPUT,
        TOP3_OUTPUT,
        BOTTOM1_OUTPUT,
        BOTTOM2_OUTPUT,
        BOTTOM3_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        ENUMS(VU_LIGHTS, 4 * 16),
        NUM_LIGHTS
    };

    StereoInHandler inTop;
    StereoInHandler inBottom;

    StereoOutHandler outTop1;
    StereoOutHandler outTop2;
    StereoOutHandler outTop3;

    StereoOutHandler outBottom1;
    StereoOutHandler outBottom2;
    StereoOutHandler outBottom3;

    int ledRoundRobin = 0;

    TRSMULTMETER() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        inTop.configure(&inputs[TOP_INPUT]);
        inBottom.configure(&inputs[BOTTOM_INPUT]);
        outTop1.configure(&outputs[TOP1_OUTPUT]);
        outTop2.configure(&outputs[TOP2_OUTPUT]);
        outTop3.configure(&outputs[TOP3_OUTPUT]);
        outBottom1.configure(&outputs[BOTTOM1_OUTPUT]);
        outBottom2.configure(&outputs[BOTTOM2_OUTPUT]);
        outBottom3.configure(&outputs[BOTTOM3_OUTPUT]);
    }

    void process(const ProcessArgs &args) override {

        outputs[TOP1_OUTPUT].setChannels(16);
        outputs[TOP2_OUTPUT].setChannels(16);
        outputs[TOP3_OUTPUT].setChannels(16);
        outputs[BOTTOM1_OUTPUT].setChannels(16);
        outputs[BOTTOM2_OUTPUT].setChannels(16);
        outputs[BOTTOM3_OUTPUT].setChannels(16);

        for (int polyChunk = 0; polyChunk < 2; polyChunk++) {
            float_4 write = inTop.getLeft(polyChunk);
            outTop1.setLeft(write, polyChunk);
            outTop2.setLeft(write, polyChunk);
            outTop3.setLeft(write, polyChunk);

            write = inBottom.getLeft(polyChunk);
            outBottom1.setLeft(write, polyChunk);
            outBottom2.setLeft(write, polyChunk);
            outBottom3.setLeft(write, polyChunk);
        }

        float viz = inTop.getLeft();
        float threshhold = 7.5 - float(ledRoundRobin);
        #define _LED_FRAMES 20.f
        if ((sgn(threshhold) * viz) > abs(threshhold)) {
            lights[ledRoundRobin].setSmoothBrightness(1.f, _LED_FRAMES/44100.f);
        } else {
            lights[ledRoundRobin].setSmoothBrightness(0.f, _LED_FRAMES/44100.f);
        }

        viz = inTop.getRight();
        if ((sgn(threshhold) * viz) > abs(threshhold)) {
            lights[16 + ledRoundRobin].setSmoothBrightness(1.f, _LED_FRAMES/44100.f);
        } else {
            lights[16 + ledRoundRobin].setSmoothBrightness(0.f, _LED_FRAMES/44100.f);
        }

        viz = inBottom.getLeft();
        if ((sgn(threshhold) * viz) > abs(threshhold)) {
            lights[32 + ledRoundRobin].setSmoothBrightness(1.f, _LED_FRAMES/44100.f);
        } else {
            lights[32 + ledRoundRobin].setSmoothBrightness(0.f, _LED_FRAMES/44100.f);
        }

        viz = inBottom.getRight();
        if ((sgn(threshhold) * viz) > abs(threshhold)) {
            lights[48 + ledRoundRobin].setSmoothBrightness(1.f, _LED_FRAMES/44100.f);
        } else {
            lights[48 + ledRoundRobin].setSmoothBrightness(0.f, _LED_FRAMES/44100.f);
        }

        ledRoundRobin ++;
        ledRoundRobin = ledRoundRobin >= 16 ? 0 : ledRoundRobin;

    }
};


struct TRSMULTMETERWidget : ModuleWidget {
    TRSMULTMETERWidget(TRSMULTMETER *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSMULTMETER.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<HexJack>(mm2px(Vec(10.16, 15.488)), module, TRSMULTMETER::TOP_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(10.143, 71.48)), module, TRSMULTMETER::BOTTOM_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.16, 29.49)), module, TRSMULTMETER::TOP1_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.16, 43.49)), module, TRSMULTMETER::TOP2_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.16, 57.498)), module, TRSMULTMETER::TOP3_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.16, 85.495)), module, TRSMULTMETER::BOTTOM1_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.16, 99.497)), module, TRSMULTMETER::BOTTOM2_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(10.16, 113.498)), module, TRSMULTMETER::BOTTOM3_OUTPUT));

        for (int i = 0; i < 16; i++) {
            addChild(createLight<MeterLight<RectangleLight<BlueLight>>>(mm2px(Vec(2, 11.25 + 3.2f * i)), module, TRSMULTMETER::VU_LIGHTS + i));
        }

        for (int i = 0; i < 16; i++) {
            addChild(createLight<MeterLight<RectangleLight<BlueLight>>>(mm2px(Vec(15.87, 11.25 + 3.2f * i)), module, TRSMULTMETER::VU_LIGHTS + 16 + i));
        }

        for (int i = 0; i < 16; i++) {
            addChild(createLight<MeterLight<RectangleLight<BlueLight>>>(mm2px(Vec(2, 67.243 + 3.2f * i)), module, TRSMULTMETER::VU_LIGHTS + 32 + i));
        }

        for (int i = 0; i < 16; i++) {
            addChild(createLight<MeterLight<RectangleLight<BlueLight>>>(mm2px(Vec(15.87, 67.243 + 3.2f * i)), module, TRSMULTMETER::VU_LIGHTS + 48 + i));
        }

    }
};


Model *modelTRSMULTMETER = createModel<TRSMULTMETER, TRSMULTMETERWidget>("TRSMULTMETER");
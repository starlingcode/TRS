#include "trs.hpp"


struct TRSPHASER : Module {
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

    // fourPolePhaser<float_4> phasers[2][2];
    ZDFPhaser8 phasers8[2];
    ZDFPhaser4 phasers4[2];

    int use8Pole = 0;

    TRSPHASER() {

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(FB_PARAM, 0.f, .5f, 0.f, "");
        configParam(MIX_PARAM, 0.f, .5f, 0.f, "");
        configParam(CVAMT_PARAM, 0.f, 1.f, 1.f, "");

        in.configure(&inputs[IN_INPUT]);
        cv.configure(&inputs[CV_INPUT]);

        wet.configure(&outputs[WET_OUTPUT]);
        mix.configure(&outputs[MIX_OUTPUT]);

    }

    void process(const ProcessArgs &args) override {

        float Ts = APP->engine->getSampleTime();

        outputs[WET_OUTPUT].setChannels(16);
        outputs[MIX_OUTPUT].setChannels(16);

        float inL = in.getLeft();
        float inR = in.getRight();

        float fb = params[FB_PARAM].getValue();
        float cvDepth = params[CVAMT_PARAM].getValue();

        float freq = clamp((cv.getLeft() * cvDepth), -5.f, 5.f);
        freq = 480.f * (dsp::approxExp2_taylor5(freq + 5.f) / 32.f) * Ts;
        float phasedL = 0;
        if (use8Pole) {
            phasers8[0].setParams(freq, fb);
            phasedL = phasers8[0].process(inL);
        } else {
            phasers4[0].setParams(freq, fb);
            phasedL = phasers4[0].process(inL);
        }

        freq = clamp((cv.getRight() * cvDepth), -5.f, 5.f);
        freq = 480.f * (dsp::approxExp2_taylor5(freq + 5.f) / 32.f) * Ts;
        float phasedR = 0;
        if (use8Pole) {
            phasers8[1].setParams(freq, fb);
            phasedR = phasers8[1].process(inR);
        } else {
            phasers4[1].setParams(freq, fb);
            phasedR = phasers4[1].process(inR);
        }

        wet.setLeft(phasedL);
        wet.setRight(phasedR);

        float mixAmount = params[MIX_PARAM].getValue();

        mix.setLeft(phasedL * .5f + inL * mixAmount);
        mix.setRight(phasedR * .5f + inR * mixAmount);

    }
};


struct TRSPHASERWidget : ModuleWidget {
    TRSPHASERWidget(TRSPHASER *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TRSPHASER.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<SifamBlack>(mm2px(Vec(15.225, 17.609)), module, TRSPHASER::FB_PARAM));
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(15.225, 41.109)), module, TRSPHASER::MIX_PARAM));
        addParam(createParamCentered<SifamBlack>(mm2px(Vec(15.225, 64.609)), module, TRSPHASER::CVAMT_PARAM));

        addInput(createInputCentered<HexJack>(mm2px(Vec(8.953, 99.471)), module, TRSPHASER::IN_INPUT));
        addInput(createInputCentered<HexJack>(mm2px(Vec(21.777, 99.471)), module, TRSPHASER::CV_INPUT));

        addOutput(createOutputCentered<HexJack>(mm2px(Vec(8.952, 113.523)), module, TRSPHASER::WET_OUTPUT));
        addOutput(createOutputCentered<HexJack>(mm2px(Vec(21.777, 113.523)), module, TRSPHASER::MIX_OUTPUT));
    }

    void appendContextMenu(Menu *menu) override {
        TRSPHASER *module = dynamic_cast<TRSPHASER*>(this->module);


        struct PolesHandler : MenuItem {
            TRSPHASER *module;
            int32_t phaserType;
            void onAction(const event::Action &e) override {
                module->use8Pole = phaserType;
            }
        };

        struct PolesItem : MenuItem {
            TRSPHASER *module;
            Menu *createChildMenu() override {
                Menu *menu = new Menu();
                const std::string poles[] = {
                    "4", "8 (CPU Beware)" 
                };
                for (int i = 0; i < (int) LENGTHOF(poles); i++) {
                    PolesHandler *menuItem = createMenuItem<PolesHandler>(poles[i], CHECKMARK((module->use8Pole) == i));
                    menuItem->module = module;
                    menuItem->phaserType = i;
                    menu->addChild(menuItem);
                }
                return menu;
            }
        };

        menu->addChild(new MenuEntry);
        PolesItem *poles = createMenuItem<PolesItem>("Phaser Poles");
        poles->module = module;
        poles->rightText = string::f("%d", ((module->use8Pole) + 1) * 4) + " " + RIGHT_ARROW;
        menu->addChild(poles);

    }

};


Model *modelTRSPHASER = createModel<TRSPHASER, TRSPHASERWidget>("TRSPHASER");
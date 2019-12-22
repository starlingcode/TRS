#include <app/ParamWidget.hpp>

struct SifamBlack : RoundKnob {
    SifamBlack() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob_sifam_blkcap.svg")));
    }
};

struct SifamGrey : RoundKnob {
    SifamGrey() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob_sifam_grycap.svg")));
    }
};

struct HexJack : SvgPort {
    HexJack() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/jack-nogradients.svg")));
    }
};


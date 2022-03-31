#include "plugin.hpp"


Plugin *pluginInstance;


void init(Plugin *p) {
    pluginInstance = p;

    // Add modules here
    p->addModel(modelTRSTURN);
    p->addModel(modelTRSSPIN);
    p->addModel(modelTRSTS);
    p->addModel(modelTRSOPS);
    p->addModel(modelTRSVCF);
    p->addModel(modelTRSPHASER);
    p->addModel(modelTRSMS2);
    p->addModel(modelTRSATTENUATORS);
    p->addModel(modelTRS2QVCA);
    p->addModel(modelTRSSIN);
    p->addModel(modelTRSBBD);
    p->addModel(modelTRSPEAK);
    p->addModel(modelTRSXOVER);
    p->addModel(modelTRSPRE);
    p->addModel(modelTRSMULTMETER);

    // Any other plugin initialization may go here.
    // As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}

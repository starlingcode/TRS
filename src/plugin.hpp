#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin *pluginInstance;

// Declare each Model, defined in each module source file
extern Model *modelBalanceMergePan;
extern Model *modelQuadLFO;
extern Model *modelTSTRSTS;
extern Model *modelFieldOps;
extern Model *modelTRSVCF;
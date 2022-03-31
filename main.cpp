#include <iostream>
#include <core/Engine.h>
#include "ProjectManagerLayer.h"

using namespace engine;

int main(int, char* []) {
    Engine _e;
    _e.onInit();
    auto* _testLayer = new ProjectManagerLayer;
    _e.setScene(_testLayer);
    _e.onRun();
    _e.destroy();
    return 0;
}

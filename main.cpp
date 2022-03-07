#include <iostream>
#include <core/Engine.h>
#include "ProjectManagerLayer.h"

using namespace engine;

int main(int, char* []) {
    Engine _e;
    auto* _testLayer = new ProjectManagerLayer;
    _e.pushLayer(_testLayer);
    _e.onRun();
    _e.destroy();
    return 0;
}

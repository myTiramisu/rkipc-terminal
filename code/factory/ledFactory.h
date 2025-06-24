#pragma once

#include "abstractFactory.h"
#include "abstractModule.h"
#include "led.h"

class LedFactory : public BaseFactory {
public:
    BaseModule* createModule(const ModuleParams& params = {}) const override {
        return new Led(params);
    }
};
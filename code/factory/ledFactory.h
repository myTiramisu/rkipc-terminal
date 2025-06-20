#pragma once

#include "abstractFactory.h"
#include "abstractModule.h"
#include "led.h"

class LedFactory : public AbstractFactory {
public:
    AbstractModule* createModule(const ModuleParams& params = {}) const override {
        return new Led(params.publisher, params.name, params.pin);
    }
};
#pragma once

#include "abstractFactory.h"
#include "abstractModule.h"
#include "pantilt.h"

class PtzFactory : public AbstractFactory {
public:
    AbstractModule* createModule(const ModuleParams& params = {}) const override {
        return new Pantilt();
    }
};
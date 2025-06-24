#pragma once

#include "abstractFactory.h"
#include "abstractModule.h"
#include "pantilt.h"

class PtzFactory : public BaseFactory {
public:
    BaseModule* createModule(const ModuleParams& params = {}) const override {
        return new Pantilt(params);
    }
};
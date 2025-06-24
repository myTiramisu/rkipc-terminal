#pragma once

#include "abstractFactory.h"
#include "abstractModule.h"

class BaseVideoFactory : public BaseFactory {
public:
    BaseModule* createModule(const ModuleParams& params = {}) const override {
        // return new LedModule();
    }
};
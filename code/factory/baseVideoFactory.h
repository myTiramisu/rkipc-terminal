#pragma once

#include "abstractFactory.h"
#include "abstractModule.h"

class BaseVideoFactory : public AbstractFactory {
public:
    AbstractModule* createModule() const override {
        // return new LedModule();
    }
};
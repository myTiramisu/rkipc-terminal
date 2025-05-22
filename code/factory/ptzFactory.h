#pragma once

#include "abstractFactory.h"
#include "abstractModule.h"
#include "pantilt.h"

class PtzFactory : public AbstractFactory {
public:
    AbstractModule* createModule() const override {
        return new Pantilt();
    }
};
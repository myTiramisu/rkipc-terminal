#pragma once

#include "abstractFactory.h"
#include "abstractModule.h"
#include "display.h"

class DisplayFactory : public AbstractFactory {
public:
    AbstractModule* createModule() const override {
        return new Display();
    }
};
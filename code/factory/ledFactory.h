#pragma once

#include "abstractFactory.h"
#include "abstractModule.h"
#include "led.h"

class LedFactory : public AbstractFactory {
public:
    AbstractModule* createModule() const override {
        return new Led(LED0);
    }
};
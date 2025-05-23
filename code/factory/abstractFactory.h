#pragma once

#include <iostream>
#include "abstractModule.h"

using namespace std;

class AbstractFactory
{
public:
    virtual ~AbstractFactory() = default;
    virtual AbstractModule* createModule(const ModuleParams& params = {}) const = 0;
private:
    
};
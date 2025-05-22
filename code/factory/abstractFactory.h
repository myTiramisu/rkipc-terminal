#pragma once

#include <iostream>
#include "abstractModule.h"

using namespace std;

class AbstractFactory
{
private:
public:
    virtual ~AbstractFactory() = default;
    virtual AbstractModule* createModule() const = 0;
};
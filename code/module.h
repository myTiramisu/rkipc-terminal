// dodule.h
#pragma once
#include "ControlCenter.h"

// 抽象基类
class Module: public Observer
{
public:
    virtual ~Module() = default;
    
    void update(const std::string& message, const std::string& mode) override{}
};


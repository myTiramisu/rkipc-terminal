// dodule.h
#pragma once
#include "ControlCenter.h"

// 既支持多产品族的统一接口，又允许单个产品族内部的灵活扩展
// 抽象基类
class AbstractModule
{
public:
    virtual ~AbstractModule() = default;
};


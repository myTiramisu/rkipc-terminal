#pragma once

#include "gpio.h"
#include "publisher.h"
#include "observer.h"
#include "string"

using namespace std;

struct VideoParams {
    int pipeId;
    int viChannelId;
    int vencChannelId;
    int video_width;
    int video_height;
};

struct ModuleParams {
    Publisher* publisher;
    std::string name;
    Gpio_num pin;                       
    struct VideoParams videoparams;        
};


// 既支持多产品族的统一接口，又允许单个产品族内部的灵活扩展
// 抽象基类
class AbstractModule
{
public:
    virtual ~AbstractModule() = default;
};


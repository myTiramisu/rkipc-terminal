// video_factory.h
#ifndef VIDEO_FACTORY_H
#define VIDEO_FACTORY_H

#include "videoBase.h"
#include "abstractFactory.h"
#include "videoRTSP.h"
#include "videoLCD.h"
#include "videoYOLO.h"

class VideoRTSPFactory: public AbstractFactory{
public:
    AbstractModule* createModule(const ModuleParams& params = {}) const override {
        return new VideoRTSP();
    }
    virtual ~VideoRTSPFactory() {}
};


class VideoDisplayFactory:  public AbstractFactory{
public:
    AbstractModule* createModule(const ModuleParams& params = {}) const override {
        return new VideoLCD();
    }
    virtual ~VideoDisplayFactory() {}
};

class VideoYOLOFactory:  public AbstractFactory{
public:
    AbstractModule* createModule(const ModuleParams& params = {}) const override {
        return new VideoYOLO();
    }
    virtual ~VideoYOLOFactory() {}
};

#endif 




// video_factory.h
#ifndef VIDEO_FACTORY_H
#define VIDEO_FACTORY_H

#include "videobase.h"

class VideoFactory {
public:
    virtual ~VideoFactory() {}
    virtual VideoBase* createVideo(int pipeId, int viChannelId, int vencChannelId, int width, int height) = 0;
};

#endif // VIDEO_FACTORY_H
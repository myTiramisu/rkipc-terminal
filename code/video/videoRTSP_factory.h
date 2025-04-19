#ifndef VIDEO_RTSP_FACTORY_H
#define VIDEO_RTSP_FACTORY_H

#include "video_factory.h"
#include "videoRTSP.h"

class VideoRTSPFactory : public VideoFactory {
public:
    VideoBase* createVideo(int pipeId, int viChannelId, int vencChannelId, int width, int height) override {
        return new VideoRTSP(pipeId, viChannelId, vencChannelId, width, height);
    }
};

#endif // VIDEO_RTSP_FACTORY_H
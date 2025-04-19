// video_yolo_factory.h
#ifndef VIDEO_YOLO_FACTORY_H
#define VIDEO_YOLO_FACTORY_H

#include "video_factory.h"
#include "videoYOLO.h"

class VideoYOLOFactory : public VideoFactory {
public:
    VideoBase* createVideo(int pipeId, int viChannelId, int vencChannelId, int width, int height) override {
        return new VideoYOLO(pipeId, viChannelId, vencChannelId, width, height);
    }
};

#endif // VIDEO_YOLO_FACTORY_H
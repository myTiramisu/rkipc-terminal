// video_lcd_factory.h
#ifndef VIDEO_LCD_FACTORY_H
#define VIDEO_LCD_FACTORY_H

#include "video_factory.h"
#include "videoLCD.h"

class VideoLCDFactory : public VideoFactory {
public:
    VideoBase* createVideo(int pipeId, int viChannelId, int vencChannelId, int width, int height) override {
        return new VideoLCD(pipeId, viChannelId, vencChannelId, width, height);
    }
};

#endif // VIDEO_LCD_FACTORY_H
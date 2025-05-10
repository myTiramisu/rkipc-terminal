#ifndef VIDEO_LCD_H
#define VIDEO_LCD_H

#include "videobase.h"

class VideoLCD : public VideoBase
{
protected:
    std::thread *video_thread;
    void video_thread_func();
    
    void videoCapture() override;
    void videoEncode() override;
    void videoRtspTransmit() override;
public:
    VideoLCD(int pipeId, int viChannelId, int vencChannelId, int width, int height);
    ~VideoLCD();

private:
    bool quit_flag;
    int pipeId ;
    int viChannelId;
    int vencChannelId;
    int video_width;
    int video_height;
    VIDEO_FRAME_INFO_S stViFrame;       // 视频帧信息结构体
    VENC_STREAM_S stFrame;              // 视频帧流结构体
    VIDEO_FRAME_INFO_S venc_frame;      // 编码帧结构体
};



#endif

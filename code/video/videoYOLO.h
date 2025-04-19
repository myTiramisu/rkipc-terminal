#ifndef VIDEOYOLO_H
#define VIDEOYOLO_H

#include "videobase.h"
class VideoYOLO : public VideoBase
{
public:
    VideoYOLO(int pipeId, int viChannelId, int vencChannelId, int width, int height);
    ~VideoYOLO();

protected:
    void videoCapture() override;
    void videoEncode() override;
    void videoRtspTransmit() override;

private:
    int pipeId;
    int viChannelId;
    int vencChannelId;
    int video_width;
    int video_height;
    int rgn_video_width;
    int rgn_video_height;


    std::thread *video_thread;
    void video_thread_func();
    bool  quit_flag;

    VIDEO_FRAME_INFO_S stViFrame;


};





#endif

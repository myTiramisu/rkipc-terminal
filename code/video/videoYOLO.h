#ifndef VIDEOYOLO_H
#define VIDEOYOLO_H

#include "videoBase.h"
class VideoYOLO : public VideoBase
{
public:
    VideoYOLO();
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

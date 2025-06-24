#ifndef VIDEOYOLO_H
#define VIDEOYOLO_H

#include "videoBase.h"
class VideoYOLO : public VideoBase
{
public:
    VideoYOLO(ModuleParams params);
    ~VideoYOLO();

protected:
    // 观察者模式数据通信接口
    void notify(string msg) override;
    void update(Publisher* publisher, string msg) override;

    // 摄像头采集
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

#ifndef VIDEORTSP_H
#define VIDEORTSP_H
#include "videobase.h"

class VideoRTSP : public VideoBase
{
protected:
    void videoCapture() override;
    void videoEncode() override;
    void videoRtspTransmit() override;
 

public:
    VideoRTSP(int pipeId, int viChannelId, int vencChannelId, int width, int height);
    ~VideoRTSP();

private:
    int video_width  = 2304;
    int video_height = 1296;
    int pipeId = 0;            // pipeId
    int viChannelId = 0;       // VI channel id
    int vencChannelId = 0;     // VENC channel id

    VIDEO_FRAME_INFO_S stViFrame;   
    VENC_STREAM_S stFrame;     // VENC frame
    MPP_CHN_S vi_chn;          //  VI channel
    MPP_CHN_S venc_chn;        //  VENC channel
    
    bool quit_flag;
    std::thread *video_thread;
    void video_thread_func();
};



#endif

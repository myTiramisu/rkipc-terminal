#ifndef VIDEORTSP_H
#define VIDEORTSP_H
#include "videobase.h"

class VideoRTSP : public VideoBase
{
protected:
    void video_thread_func() override;

public:
    VideoRTSP();
    ~VideoRTSP();
};



#endif

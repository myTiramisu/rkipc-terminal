#ifndef VIDEORTSP_H
#define VIDEORTSP_H
#include "video.h"

class VideoRTSP : public Video
{
protected:
    void video_thread_func() override;

public:
    VideoRTSP();
    ~VideoRTSP();
};



#endif

#ifndef VIDEOYOLO_H
#define VIDEOYOLO_H

#include "videobase.h"
class VideoYOLO : public VideoBase
{
protected:
    void video_thread_func() override;

public:
    VideoYOLO();
    ~VideoYOLO();
};





#endif

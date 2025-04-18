#ifndef VIDEOYOLO_H
#define VIDEOYOLO_H

#include "video.h"
class VideoYOLO : public Video
{
protected:
    void video_thread_func() override;

public:
    VideoYOLO();
    ~VideoYOLO();
};





#endif

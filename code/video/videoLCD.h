#ifndef VIDEO_LCD_H
#define VIDEO_LCD_H

#include "video.h"

class VideoLCD : public Video
{
protected:
    void video_thread_func() override;

public:
    VideoLCD();
    ~VideoLCD();
};



#endif

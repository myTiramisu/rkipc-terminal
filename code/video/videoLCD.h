#ifndef VIDEO_LCD_H
#define VIDEO_LCD_H

#include "videobase.h"

class VideoLCD : public VideoBase
{
protected:
    void video_thread_func() override;

public:
    VideoLCD();
    ~VideoLCD();
};



#endif

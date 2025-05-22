#ifndef _VIDEOBASE_H_
#define _VIDEOBASE_H_
extern "C" {
    #include "luckfox_video.h"
    #include "rtsp.h"
}

#include <thread>
#include <mutex>
#include <iostream>
#include "signal_slot.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "log.h"
#include <yolov5.h>

// osd
#include "luckfox_osd.h"
#include "luckfox_osd_draw.h"
#include "module.h"

class VideoBase : public Module
{
protected:
    virtual void videoCapture() = 0;
    virtual void videoEncode() = 0;
    virtual void videoRtspTransmit() = 0;

public:
    VideoBase();
    virtual  ~VideoBase();

    // 发送摄像头数据帧信号
    Signal<cv::Mat> video_frame_signal;
};
#endif
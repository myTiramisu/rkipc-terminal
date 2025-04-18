#ifndef _VIDEO_H_
#define _VIDEO_H_
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

class Video
{
protected:
    // 线程
    std::thread *video_thread;
    std::mutex  mutex_video;
    bool quit_flag;

    virtual void video_thread_func() = 0;  // 纯虚函数，每个派生类必须实现
 
public:
    Video();
    virtual  ~Video();
    
    void start_thread();
    void stop_thread();

    // 发送摄像头数据帧信号
    Signal<cv::Mat> video_frame_signal;
};
#endif
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
private:
    // 暂时设置三路摄像头线程
    std::thread *video_thread0;
    std::thread *video_thread1;
    std::thread *video_thread2;
    std::mutex  mutex_video;
    bool quit_flag;

    // 线程函数
    void video_thread_0();      // rtsp video
    void video_thread_1();      // send frame too lcd show
    void video_thread_2();      // yolo model

public:
    Video();
    ~Video();
    
    // 发送摄像头数据帧信号
    Signal<cv::Mat> video_frame_signal;

};

#endif
#ifndef _VIDEO_H_
#define _VIDEO_H_
extern "C" {
    #include "luckfox_video.h"
    #include "rtsp.h"
}
#include <thread>
#include <iostream>
#include <mutex>

class Video
{
private:
    // video size
    int width    = 720;
    int height   = 480;

    // 暂时设置三路摄像头线程
    std::thread *video_thread0;
    std::thread *video_thread1;
    std::thread *video_thread2;
    std::mutex  mutex_video;
    
    // 线程函数
    void video_thread_0();
    void video_thread_1();

public:
    Video();
    ~Video();

};

#endif
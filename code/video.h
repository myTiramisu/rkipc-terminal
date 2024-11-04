#ifndef _VIDEO_H_
#define _VIDEO_H_
extern "C" {
    #include "luckfox_video.h"
    #include "rtsp.h"
}
#include <thread>
#include <mutex>
#include <iostream>

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
public:
    Video();
    ~Video();
};

#endif
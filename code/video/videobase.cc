#include "videobase.h"
VideoBase ::VideoBase ()
{
    quit_flag = false;
    video_thread = nullptr;
}
VideoBase ::~VideoBase ()
{
    stop_thread();
    LOG_DEBUG("******************************Release video success\n");
}

void VideoBase ::start_thread()
{
    video_thread = new std::thread(&VideoBase::video_thread_func, this);
}

void VideoBase::stop_thread()
{
    {
        std::lock_guard<std::mutex> lock(mutex_video);
        quit_flag = true;
    }
    if (video_thread && video_thread->joinable()) {
        video_thread->join();
        delete video_thread;
        video_thread = nullptr;
    }
}

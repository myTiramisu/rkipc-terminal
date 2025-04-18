#include "video.h"
Video::Video()
{
    quit_flag = false;
    video_thread = nullptr;
}
Video::~Video()
{
    stop_thread();
    LOG_DEBUG("******************************Release video success\n");
}

void Video::start_thread()
{
    video_thread = new std::thread(&Video::video_thread_func, this);
}

void Video::stop_thread()
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

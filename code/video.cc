#include "video.h"



Video::Video()
{
    rkaiq_init();
    rkmpi_init();
    rtsp_init();

    // vi init	 	初始化视频输入设备和通道
	vi_dev_init();
	vi_chn_init(0, width, height);
    vpss_init(0, width, height);

    video_thread0 = new std::thread(&Video::video_thread_0, this);
    video_thread1 = new std::thread(&Video::video_thread_1, this);
    // video_thread2 = new std::thread(&Video::video_thread2, this);

}


Video::~Video()
{
    if (video_thread0->joinable()) {
        video_thread0->join();
    }
    if (video_thread1->joinable()) {
        video_thread1->join();
    }
    if (video_thread2->joinable()) {
        video_thread2->join();
    }

    // 构造函数的反初始化
    rkaiq_deinit();
    rkmpi_deinit();
    rtsp_release();
    vi_dev_deinit();
    vi_chn_deinit(0);
    vpss_deinit(0);
}

void Video::video_thread_0()
{
    
} 



void Video::video_thread_1()
{

}
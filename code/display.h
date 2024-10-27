#ifndef _DISPLAY_H_
#define _DISPLAY_H_
#include <iostream>
#include <mutex>
#include <condition_variable>

class Display
{
private:
    // 显示屏属性
	void* framebuffer = NULL; 
	int pixel_size = 0;
	int disp_width = 0;
	int disp_height = 0;

    bool display_flag = false; 
    bool pause_flag = false;

    // 互斥锁和条件变量
    std::mutex frame_mutex;
    std::condition_variable frame_cond;
    bool frame_ready = false;

public:
    Display();
    ~Display();

    void init_framebuffer(void);
};

Display::Display()
{
    
}

Display::~Display()
{

}


#endif
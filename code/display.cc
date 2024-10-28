#include "display.h"
extern "C" {
    #include "framebuffer.h"
}

Display::Display()
{
    if(init_framebuffer() < 0) {
        std::cout << "error initializing framebuffer" << std::endl;
    }
    // 获取显示参数
    disp_width = get_framebuffer_width();
    disp_height = get_framebuffer_height();
    bit_depth = get_framebuffer_pixel_size();
    printf("Screen width = %d, Screen height = %d, Pixel_size = %d\n",disp_width, disp_height, bit_depth);
    
	// 获取帧缓冲区位深度大小 设置颜色
	if( bit_depth == 4 )			//ARGB8888
		disp = cv::Mat(disp_height, disp_width, CV_8UC4);	// BGRX
    else if ( bit_depth == 2 ) 	//RGB565
		disp = cv::Mat(disp_height, disp_width, CV_16UC1); 	// GRAY

    // 启动显示线程
    display_thread = std::thread(&Display::display_on_lcd, this);
}

Display::~Display() 
{

}

 // 接收帧并将其放入队列
void Display::push_frame(const cv::Mat& frame)
{
    // Screen width = 128, Screen height = 160, Pixel_size = 2
    if (frame.rows != this->disp_height || 
        frame.cols != this->disp_width ||
        frame.type() != CV_8UC3) 
    {   
        std::cout << "frame size error" << std::endl;
        std::cout << "frame .rows: " << frame.rows << std::endl;
        std::cout << "frame.cols: " << frame.cols << std::endl;
        std::cout << "frame.type() : " <<frame.type()  << std::endl;
        return;
    }
    {
        std::lock_guard<std::mutex> lock(mutex); 
        queue.push(frame);
    }
    cond.notify_one();  // 通知显示
}

void Display::display_on_lcd(void)
{
    while (true)
    {
        // 等待条件变量 等主线程完成后 LCD显示图像
		std::unique_lock<std::mutex> lock(mutex);
        //等待队列有数据
        cond.wait(lock, [this](){
            return !queue.empty(); 
        });

        // 取出一帧
        cv::Mat frame = queue.front();
        queue.pop();
        lock.unlock(); 			// 释放锁 允许其他线程推送

        cv::Mat resized_frame, rotated_frame;
		cv::resize(frame, resized_frame, cv::Size(disp_width, disp_height));
		cv::rotate(resized_frame, rotated_frame, cv::ROTATE_90_CLOCKWISE);
        cv::cvtColor(resized_frame, disp, cv::COLOR_RGB2BGR565);
		framebuffer_set_frame((uint16_t*)disp.data, disp_width, disp_height);
    }
}
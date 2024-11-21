#include <rk_comm_rgn.h>
#include <rk_mpi_rgn.h>
#include <cstring>
#include "log.h"
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>

struct DrawTask
{
    RGN_HANDLE coverHandle; // 区域句柄
    int x, y, w, h;         // 绘制区域的坐标
    int batch;              // 批次
    int num;                // 绘制的数量
};

class OsdDraw
{
private:
    // 绘制区域参数
    RGN_HANDLE coverHandle;         // 覆盖句柄
	MPP_CHN_S stCoverChn;           // 存储多媒体处理通道的属性
    int line_pixel;                 // 线条粗细
	
    // 功能变量
    std::queue<DrawTask> drawTaskQueue;  // 绘制任务队列

    // 绘制线程函数
    void rgn_draw_thread();    
    std::thread *rgn_draw_thread_id;
    bool osd_rgn_draw_flag;
    // 同步与互斥
    std::mutex  mutex_osdDraw; 
    std::condition_variable cond_osdDraw;


    //功能函数
    int osd_drwn_rgn_init();
    int osd_drwn_rgn_deinit();
    // 2bpp图像缓冲区中绘制一个矩形
    RK_S32 osd_draw_rect_2BPP(RK_U8 *buffer,  RK_U32 width, RK_U32 height, int rgn_x, int rgn_y, int rgn_w, int rgn_h);

public:
    OsdDraw(/* args */);
    ~OsdDraw();
};
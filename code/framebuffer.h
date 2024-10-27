#ifndef _FRAMEBUFF_H_
#define _FRAMEBUFF_H_

// 帧缓冲设备
#define FB_DEVICE "/dev/fb0"

// RGB颜色定义
#define RGB(r, g, b)  (((r) << 16) | ((g) << 8) | (b))
#define RGB_WHITE  0xFFFFFF
#define RGB_BLACK  0x000000
#define RGB_RED    0xFF0000
#define RGB_GREEN  0x00FF00
#define RGB_BLUE   0x0000FF


// 初始化帧缓冲
int init_framebuffer();
// 释放帧缓冲
void framebuffer_deinit(void);

// 获取像素参数
int get_framebuffer_pixel_size();
int get_framebuffer_width();
int get_framebuffer_height();

// 获取帧缓冲区
unsigned int *get_framebuffer();

#endif
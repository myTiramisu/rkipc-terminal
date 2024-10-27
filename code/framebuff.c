#include "framebuff.h"
#include <fcntl.h>
#include <stdio.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
    
// frameBuff 全局变量
static int fd = -1;                         // Framebuffer 文件描述符
static struct fb_fix_screeninfo fb_fix;     // 固定屏幕信息
static struct fb_var_screeninfo fb_var;     // 可变屏幕信息
uint8_t* framebuffer = NULL;                // 显示缓冲区
static size_t screensize = 0;               // 显示缓冲区大小
int pixel_size = 0;     // 像素深度
int disp_width = 0;     // 显示宽度
int disp_height = 0;    // 显示高度

// 初始化帧缓冲区 内存映射
// 0: 成功  -1 失败
int init_framebuffer() 
{
    // 打开帧缓冲区设备
    fd = open(FB_DEVICE, O_RDWR);
    // 检查是否打开成功
    if (fd < 0) {
        perror("!!!!!!Failed to open framebuffer device");
        return -1;
    }
    
    // 获取帧缓冲区信息
    if(ioctl(fd, FBIOGET_VSCREENINFO, &fb_var) < 0) {
        perror("!!!!!!Failed to get framebuffer variable information");
        return -1;
    }
    if (ioctl(fd, FBIOGET_VSCREENINFO, &fb_fix) < 0) {
        perror("!!!!!!Failed to get framebuffer fixed information");
        return -1;
    }
    
    // 获取显示参数 宽度 高度 深度
    disp_width = fb_var.xres;
    disp_height = fb_var.yres;  
    pixel_size = fb_var.bits_per_pixel / 8;
    printf("Screen width = %d, Screen height = %d, Pixel_size = %d\n",disp_width, disp_height, pixel_size);
    screensize = disp_width * disp_height * pixel_size;
    framebuffer = (uint8_t*)mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (framebuffer == MAP_FAILED) {
        perror("!!!!!!Failed to mmap framebuffer");
        return -1;
    }
    printf("******int framebuffer init success!******\n");
    return 0;
}

// 释放 Framebuffer
void framebuffer_deinit(void)
{
    if (framebuffer != NULL) {
        munmap(framebuffer, screensize);
        framebuffer = NULL;
    }
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
    return;
}

int get_framebuffer_pixel_size() {
    return pixel_size;
}

int get_framebuffer_width() {
    return disp_width;
}

int get_framebuffer_height() {
    return disp_height;
}

unsigned int *get_framebuffer()
{
    return (unsigned int *)framebuffer;
}
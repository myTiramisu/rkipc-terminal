#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <time.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include "rtsp_demo.h"
#include "luckfox_mpi.h"
#include "yolov5.h"
#include <thread>

// lcd
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <time.h>

#include "image_utils.h"
#include "file_utils.h"
#include "image_drawing.h"

//opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <mutex>
#include <condition_variable>

// disp size
int width    = 720;
int height   = 480;

// model size
int model_width = 640;
int model_height = 640;	
float scale ;
int leftPadding ;
int topPadding  ;

bool quit = false;
// static void sigterm_handler(int sig) {
// 	fprintf(stderr, "signal %d\n", sig);
// 	quit = true;
// }

cv::Mat letterbox(cv::Mat input)
{
	float scaleX = (float)model_width  / (float)width; //0.888
	float scaleY = (float)model_height / (float)height; //1.125	
	scale = scaleX < scaleY ? scaleX : scaleY;
	
	int inputWidth   = (int)((float)width * scale);
	int inputHeight  = (int)((float)height * scale);

	leftPadding = (model_width  - inputWidth) / 2;
	topPadding  = (model_height - inputHeight) / 2;	
	

	cv::Mat inputScale;
    cv::resize(input, inputScale, cv::Size(inputWidth,inputHeight), 0, 0, cv::INTER_LINEAR);	
	cv::Mat letterboxImage(640, 640, CV_8UC3,cv::Scalar(0, 0, 0));
    cv::Rect roi(leftPadding, topPadding, inputWidth, inputHeight);
    inputScale.copyTo(letterboxImage(roi));

	return letterboxImage; 	
}

void mapCoordinates(int *x, int *y) {	
	int mx = *x - leftPadding;
	int my = *y - topPadding;

    *x = (int)((float)mx / scale);
    *y = (int)((float)my / scale);
}


// 互斥锁和条件变量
std::mutex frame_mutex;
std::condition_variable frame_cond;
bool frame_ready = false;
// 临界资源 主线程负责生产 子线程负责消费
cv::Mat frame;		
void display_image_on_lcd() 
{
	int disp_flag = 0;
	void* framebuffer = NULL; 
	
	int pixel_size = 0;
	int disp_width = 0;
	int disp_height = 0;
	cv::Mat disp;

	//Init fb  初始化帧缓冲区
    size_t screensize = 0;

    struct fb_fix_screeninfo fb_fix;
    struct fb_var_screeninfo fb_var;
    

    int fb = open("/dev/fb0", O_RDWR); 
    if(fb == -1)
        printf("Screen OFF!\n");
    else 
        disp_flag = 1;
    
    if(disp_flag)
	{
        ioctl(fb, FBIOGET_VSCREENINFO, &fb_var);
        ioctl(fb, FBIOGET_FSCREENINFO, &fb_fix);

        disp_width = fb_var.xres;
        disp_height = fb_var.yres;  
        pixel_size = fb_var.bits_per_pixel / 8;
        printf("Screen width = %d, Screen height = %d, Pixel_size = %d\n",disp_width, disp_height, pixel_size);
        
        screensize = disp_width * disp_height * pixel_size;
        framebuffer = (uint8_t*)mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
        
        if( pixel_size == 4 )//ARGB8888
            disp = cv::Mat(disp_height, disp_width, CV_8UC3);
        else if ( pixel_size == 2 ) //RGB565
            disp = cv::Mat(disp_height, disp_width, CV_16UC1);
		std::cout << "*****************disp_flag set success" << std::endl;
    }
    else {
        disp_height = 240;
        disp_width = 240;
    }

	while (1)
	{
		std::cout << "************frame is waiting" << std::endl;
		// 等待条件变量 等主线程完成后 LCD显示图像
		//frame_mutex.lock();
		while (!frame_ready) {
			std::unique_lock<std::mutex> lock(frame_mutex);
    		frame_cond.wait(lock); // 等待条件变量
		}
		frame_ready = false;
		//frame_mutex.unlock();

		std::cout << "************frame is ready success" << std::endl;
		std::cout << "显示图像到LCD" << std::endl;

		if(disp_flag)
		{
			// 修改frame帧到LCD的大小
			cv::Mat resized_frame;
			cv::resize(frame, resized_frame, cv::Size(disp_width, disp_height));
			cv::cvtColor(resized_frame, disp, cv::COLOR_BGR2BGR565);
			memcpy(framebuffer, disp.data, disp_width * disp_height * pixel_size);
		}
	}	
}

int main(int argc, char *argv[]) 
{
  	system("RkLunch-stop.sh");
	RK_S32 s32Ret = 0; 
	int sX,sY,eX,eY; 
	
	// Ctrl-c quit
	// signal(SIGINT, sigterm_handler);

	// Rknn model
	char text[16];
	rknn_app_context_t rknn_app_ctx;	
	object_detect_result_list od_results;
    int ret;
	const char *model_path = "./model/yolov5.rknn";
    memset(&rknn_app_ctx, 0, sizeof(rknn_app_context_t));	
	init_yolov5_model(model_path, &rknn_app_ctx);
	printf("\ninit rknn model success!\n");
	init_post_process();
	//h264_frame	
	VENC_STREAM_S stFrame;	
	stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S));
 	VIDEO_FRAME_INFO_S h264_frame;
 	VIDEO_FRAME_INFO_S stVpssFrame;
	printf("\n************************Init h264_frame  success!\n");

	// rkaiq init
	RK_BOOL multi_sensor = RK_FALSE;	
	const char *iq_dir = "/etc/iqfiles";
	rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
	//hdr_mode = RK_AIQ_WORKING_MODE_ISP_HDR2;
	SAMPLE_COMM_ISP_Init(0, hdr_mode, multi_sensor, iq_dir);
	SAMPLE_COMM_ISP_Run(0);
	printf("\n************************Init rkaiq  success!\n");

	// rkmpi init
	if (RK_MPI_SYS_Init() != RK_SUCCESS) {
		RK_LOGE("rk mpi sys init fail!");
		return -1;
	}
	printf("\n************************Init rkmpi  success!\n");

	// rtsp init	创建RTSP直播演示，并初始化RTSP会话
	rtsp_demo_handle g_rtsplive = NULL;
	rtsp_session_handle g_rtsp_session;
	g_rtsplive = create_rtsp_demo(554);
	g_rtsp_session = rtsp_new_session(g_rtsplive, "/live/0");
	rtsp_set_video(g_rtsp_session, RTSP_CODEC_ID_VIDEO_H264, NULL, 0);
	rtsp_sync_video_ts(g_rtsp_session, rtsp_get_reltime(), rtsp_get_ntptime());
	printf("\n************************Init rtsp  success!\n");

	// vi init	 	初始化视频输入设备和通道
	vi_dev_init();
	vi_chn_init(0, width, height);
	
	// vpss init 	初始化视频处理子系统
	vpss_init(0, width, height);

	// bind vi to vpss  	绑定视频输入通道到视频处理子系统通道
	MPP_CHN_S stSrcChn, stvpssChn;
	stSrcChn.enModId = RK_ID_VI;
	stSrcChn.s32DevId = 0;
	stSrcChn.s32ChnId = 0;

	stvpssChn.enModId = RK_ID_VPSS;
	stvpssChn.s32DevId = 0;
	stvpssChn.s32ChnId = 0;
	printf("\n====RK_MPI_SYS_Bind vi0 to vpss0====\n");
	s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stvpssChn);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("bind 0 ch venc failed");
		return -1;
	}
	printf("\n************************RK_MPI_SYS_Bind  success!\n");

	// venc init 初始化视频编码器
	RK_CODEC_ID_E enCodecType = RK_VIDEO_ID_AVC;
	venc_init(0, width, height, enCodecType);
	printf("\n************************venc init success\n");	

 	

	// 创建一个新的线程 用于LCD显示图像
	std::thread lcd_thread(display_image_on_lcd);

  	while(1)
	{	
		// 获取VPSS帧
		s32Ret = RK_MPI_VPSS_GetChnFrame(0,0, &stVpssFrame,-1);
		if(s32Ret == RK_SUCCESS)
		{
			void *data = RK_MPI_MB_Handle2VirAddr(stVpssFrame.stVFrame.pMbBlk);	
			//opencv	
			//将帧转换为OpenCV格式
			frame = cv::Mat(height,width,CV_8UC3,data);			
			//cv::Mat frame640;
        	//cv::resize(frame, frame640, cv::Size(640,640), 0, 0, cv::INTER_LINEAR);	
			//letterbox
			cv::Mat letterboxImage = letterbox(frame);	
			memcpy(rknn_app_ctx.input_mems[0]->virt_addr, letterboxImage.data, model_width*model_height*3);		
			inference_yolov5_model(&rknn_app_ctx, &od_results);
			
			for(int i = 0; i < od_results.count; i++)
			{					
				//获取框的四个坐标 
				if(od_results.count >= 1)
				{
					object_detect_result *det_result = &(od_results.results[i]);
					printf("%s @ (%d %d %d %d) %.3f\n", coco_cls_to_name(det_result->cls_id),
							 det_result->box.left, det_result->box.top,
							 det_result->box.right, det_result->box.bottom,
							 det_result->prop);
	
					sX = (int)(det_result->box.left   );	
					sY = (int)(det_result->box.top 	  );	
					eX = (int)(det_result->box.right  );	
					eY = (int)(det_result->box.bottom );
					mapCoordinates(&sX,&sY);
					mapCoordinates(&eX,&eY);

					cv::rectangle(frame,cv::Point(sX ,sY),
								        cv::Point(eX ,eY),
										cv::Scalar(0,255,0),3);
					sprintf(text, "%s %.1f%%", coco_cls_to_name(det_result->cls_id), det_result->prop * 100);
					cv::putText(frame,text,cv::Point(sX, sY - 8),
												 cv::FONT_HERSHEY_SIMPLEX,1,
												 cv::Scalar(0,255,0),2);
				}
			}
			memcpy(data, frame.data, width * height * 3);
			
			std::lock_guard<std::mutex> lock(frame_mutex);
			frame_ready = true;
			// C++通知线程处理完一帧
			frame_cond.notify_one();
			std::cout <<  "******************frame_cond notify success" << std::endl;
		}

		// send stream
		// encode H264
		RK_MPI_VENC_SendFrame(0, &stVpssFrame,-1);
		// rtsp
		s32Ret = RK_MPI_VENC_GetStream(0, &stFrame, -1);
		if(s32Ret == RK_SUCCESS)
		{
			if(g_rtsplive && g_rtsp_session)
			{
				//printf("len = %d PTS = %d \n",stFrame.pstPack->u32Len, stFrame.pstPack->u64PTS);
				
				void *pData = RK_MPI_MB_Handle2VirAddr(stFrame.pstPack->pMbBlk);
				rtsp_tx_video(g_rtsp_session, (uint8_t *)pData, stFrame.pstPack->u32Len,
							  stFrame.pstPack->u64PTS);
				rtsp_do_event(g_rtsplive);
			}
		}

		// release frame 
		s32Ret = RK_MPI_VPSS_ReleaseChnFrame(0, 0, &stVpssFrame);
		if (s32Ret != RK_SUCCESS) {
			RK_LOGE("RK_MPI_VI_ReleaseChnFrame fail %x", s32Ret);
		}
		s32Ret = RK_MPI_VENC_ReleaseStream(0, &stFrame);
		if (s32Ret != RK_SUCCESS) {
			RK_LOGE("RK_MPI_VENC_ReleaseStream fail %x", s32Ret);
		}
		memset(text,0,8);
	}


	printf("Release\n");
	RK_MPI_SYS_UnBind(&stSrcChn, &stvpssChn);
	
	RK_MPI_VI_DisableChn(0, 0);
	RK_MPI_VI_DisableDev(0);
	
	RK_MPI_VPSS_StopGrp(0);
	RK_MPI_VPSS_DestroyGrp(0);
	
	RK_MPI_VENC_StopRecvFrame(0);
	RK_MPI_VENC_DestroyChn(0);

	free(stFrame.pstPack);

	if (g_rtsplive)
		rtsp_del_demo(g_rtsplive);
	SAMPLE_COMM_ISP_Stop(0);

	RK_MPI_SYS_Exit();

	// Release rknn model
    release_yolov5_model(&rknn_app_ctx);		
	deinit_post_process();
	
	lcd_thread.join();
	return 0;
}

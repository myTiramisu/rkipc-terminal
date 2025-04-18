#include "videobase.h"	// video
#include "videoRTSP.h"	// videoRTSP
#include "videoYOLO.h"	// videoYOLO
#include "videoLCD.h"	// videoLCD
#include "display.h"	// lcd
#include "led.h"		// led
#include "pantilt.h"	// pantilt
#include "log.h"		// log
#include "param.h"		// param
#include <unistd.h>
#include "onvif_server.h"

#define ONVIF_SERVER_ENABLE 	1
#define CONTROL_ENABLE 			1
#define LED_ENABLE 				1
#define PTZ_ENABLE 				1
#define DISPLAY_ENABLE 			1
#define VIDEORTSP_ENABLE 		1
#define VIDEOLCD_ENABLE 		1
#define VIDEOYOLO_ENABLE 		1

char ini_path[] = "rkipc.ini";
int rkipc_log_level = LOG_LEVEL_DEBUG;

bool quit = false;
static void sigterm_handler(int sig) {
	fprintf(stderr, "signal %d\n", sig);
	std::cout << "**************quit success sig:" << sig << std::endl;
	quit = true;
}

int main(int argc, char *argv[]) 
{
  	system("RkLunch-stop.sh");
	
	// 初始化参数
    if (rk_param_init(ini_path) != 0) {
        LOG_ERROR("Failed to initialize parameters\n");
        return EXIT_FAILURE;
    }

	// Ctrl-c quit
	signal(SIGINT, sigterm_handler);

#ifdef ONVIF_SERVER_ENABLE
	LOG_INFO("onvif Module init\n");
	onvif_server_init();
#endif


#ifdef CONTROL_ENABLE
	
#endif


#ifdef LED_ENABLE
	// LED模块
	LOG_INFO("led Module init\n");
	Led *led0 = new Led(LED0);
	Led *led1 = new Led(LED1);
	Led *led2 = new Led(LED2);	
	led0->off();
	led1->off();
	led2->off();
#endif


#ifdef PTZ_ENABLE
	// 俯仰旋转控制
	LOG_INFO("PTZ Module init\n");
	Pantilt *pantilt = new Pantilt();
#endif


#ifdef DISPLAY_ENABLE
	LOG_INFO("display Module init\n");
	Display *display = new Display();
#endif

#ifdef VIDEORTSP_ENABLE
	LOG_INFO("VideoRTSP Module init\n");
	VideoBase* rtsp_video = new VideoRTSP();
#endif

#ifdef VIDEOLCD_ENABLE
	LOG_INFO("VideoLCD Module init\n");	
	VideoBase* lcd_video = new VideoLCD();
#endif

#ifdef VIDEOYOLO_ENABLE
	LOG_INFO("VideoYOLO Module init\n");	
	VideoBase* yolo_video = new VideoYOLO();
#endif

#if VIDEOLCD_ENABLE && DISPLAY_ENABLE
	LOG_INFO("videoDisplay connect to display\n");
	lcd_video->video_frame_signal.connect(display, &Display::push_frame);
#endif
	

  	while(!quit)
	{	
		sleep(1);
	}


#ifdef CONTROL_ENABLE

#endif


#ifdef LED_ENABLE
	delete led0;
	delete led1;
	delete led2;	
#endif


#ifdef PTZ_ENABLE
	delete pantilt;
#endif


#ifdef DISPLAY_ENABLE
	delete display;
#endif

#ifdef VIDEORTSP_ENABLE
    delete rtsp_video;
#endif

#ifdef VIDEOLCD_ENABLE
	delete lcd_video;
#endif

#ifdef VIDEOYOLO_ENABLE
	delete yolo_video;
#endif


	rk_param_deinit();
    LOG_INFO("Program exited\n");
	return 0;
}

#include "videobase.h"	// video
#include "videoRTSP.h"	// videoRTSP
#include "videoYOLO.h"	// videoYOLO
#include "videoLCD.h"	// videoLCD
#include "display.h"	// lcd
#include "videoLCD_factory.h"
#include "videoRTSP_factory.h"
#include "videoYOLO_factory.h"
#include "led.h"		// led
#include "pantilt.h"	// pantilt
#include "log.h"		// log
#include "param.h"		// param
#include "control.h"
#include <unistd.h>
#include "onvif_server.h"
#include "factory.h"
#include "ControlCenter.h"


#define ONVIF_SERVER_ENABLE 	1

#define LED_ENABLE 				1
#define PTZ_ENABLE 				1
#define DISPLAY_ENABLE 			1

#define VIDEORTSP_ENABLE 		1
#define VIDEODISPLAY_ENABLE 	1
#define VIDEOYOLO_ENABLE 		1

#define CONTROLLER_ENABLE 		1

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

#if ONVIF_SERVER_ENABLE
	LOG_INFO("onvif Module init\n");
	onvif_server_init();
#endif


#if CONTROLLER_ENABLE
	LOG_INFO("controller Module init\n");
	ControlCenter *controllor = new ControlCenter();
#endif

#if LED_ENABLE
	// LED模块
	LOG_INFO("led Module init\n");
	std::unique_ptr<Module> led0 = DeviceFactory::createLedDevice(LED0);
	std::unique_ptr<Module> led1 = DeviceFactory::createLedDevice(LED1);
	std::unique_ptr<Module> led2 = DeviceFactory::createLedDevice(LED2);
	controllor->addObserver(led0.get());
	controllor->addObserver(led1.get());
	controllor->addObserver(led2.get());

#endif

#if PTZ_ENABLE
	// 俯仰旋转控制
	LOG_INFO("PTZ Module init\n");
	std::unique_ptr<Module> ptz = DeviceFactory::createDevice("PTZ");
	controllor->addObserver(ptz.get());
#endif

#if DISPLAY_ENABLE
	LOG_INFO("display Module init\n");
	std::unique_ptr<Module> display = DeviceFactory::createDevice("DISPLAY");
	controllor->addObserver(display.get());
#endif

#if VIDEORTSP_ENABLE
	LOG_INFO("VideoRTSP Module init\n");
	VideoFactory* videoRtspFactory = new VideoRTSPFactory();
	VideoBase* videoRtsp = videoRtspFactory->createVideo(0, 0, 0, 2304, 1296);
	controllor->addObserver(videoRtsp);
#endif

#if VIDEODISPLAY_ENABLE
	LOG_INFO("VideoDipplay Module init\n");	
	VideoFactory* videoDiaplayFactory = new VideoLCDFactory();
	VideoBase* videoDiaplsy = videoDiaplayFactory->createVideo(0, 1, 1, 720, 480);
	controllor->addObserver(videoDiaplsy);
#endif

#if VIDEOYOLO_ENABLE
	LOG_INFO("VideoYOLO Module init\n");
	VideoFactory* videoYoloFactory = new VideoYOLOFactory();
	VideoBase* videoYolo = videoYoloFactory->createVideo(0, 2, 2, 640, 640);
	controllor->addObserver(videoYolo);
#endif

#if VIDEODISPLAY_ENABLE && DISPLAY_ENABLE
	LOG_INFO("videoDisplay connect to display\n");
	// 使用lambda表达式连接信号与槽
	videoDiaplsy->video_frame_signal.connect(dynamic_cast<Display*>(display.get()), &Display::push_frame);
#endif
	

  	while(!quit)
	{	
		sleep(1);
	}



try {
	#if VIDEORTSP_ENABLE
		delete videoRtsp;
		delete videoRtspFactory;
		
	#endif

	#if VIDEODISPLAY_ENABLE
		delete videoDiaplsy;
		delete videoDiaplayFactory;
	#endif

	#if VIDEOYOLO_ENABLE
		delete videoYolo;
		delete videoDiaplayFactory;
	#endif

	#if CONTROLLER_ENABLE
		delete controllor;
	#endif


	} catch (std::exception &e) {
        LOG_ERROR("Exception: %s\n", e.what());
    }







	rk_param_deinit();
    LOG_INFO("Program exited\n");
	return 0;
}

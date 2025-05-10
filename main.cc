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
#include "server.h"
#include "uart.h"
#include <atomic>


#define ONVIF_SERVER_ENABLE 	0

#define LED_ENABLE 				0
#define PTZ_ENABLE 				0
#define DISPLAY_ENABLE 			0

#define VIDEORTSP_ENABLE 		1
#define VIDEODISPLAY_ENABLE 	0
#define VIDEOYOLO_ENABLE 		0

#define CONTROLLER_ENABLE 		0

#define UART_ENABLE 			1
#define TCP_SERVER_ENABLE 		1


char ini_path[] = "rkipc.ini";
int rkipc_log_level = LOG_LEVEL_DEBUG;

std::atomic<bool> g_quit(false);	//全局标志位

static void sigterm_handler(int sig) {
	if (sig == SIGINT) {
        g_quit = true; // 设置标志位
		std::cout << "**************quit success sig:" << sig << std::endl;
    }
}

int main(int argc, char *argv[]) 
{
  	system("RkLunch-stop.sh");
	
	// // 初始化参数
    // if (rk_param_init(ini_path) != 0) {
    //     LOG_ERROR("Failed to initialize parameters\n");
    //     return EXIT_FAILURE;
    // }

	// Ctrl-c quit
	signal(SIGINT, sigterm_handler);

#if ONVIF_SERVER_ENABLE
	LOG_INFO("onvif Module init\n");
	onvif_server_init();
#endif

#if CONTROLLER_ENABLE
	LOG_INFO("controller Module init\n");
	ControlCenter *controllor = new ControlCenter();
	// controllor->addObserver(videoRtsp);
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
	// controllor->addObserver(videoYolo);
#endif

#if VIDEODISPLAY_ENABLE && DISPLAY_ENABLE
	LOG_INFO("videoDisplay connect to display\n");
	// 使用lambda表达式连接信号与槽
	videoDiaplsy->video_frame_signal.connect(dynamic_cast<Display*>(display.get()), &Display::push_frame);
#endif
	
#if UART_ENABLE
	Uart* uart = new Uart(4);
	LOG_INFO("uart Module init success\n");
#endif

#if TCP_SERVER_ENABLE
	TcpServer* tcpServer = new TcpServer(8888);
	LOG_INFO("tcpServer Module init\n");
#endif


#if UART_ENABLE && TCP_SERVER_ENABLE
	// 绑定tcp数据到串口发送
	uart->uart_signal.connect(tcpServer, &TcpServer::send_msg);	
	// 绑定串口数据发送到TCP客户端
	tcpServer->server_signal.connect(uart, &Uart::sendData);
#endif

  	while(!g_quit)
	{	
		std::this_thread::sleep_for(std::chrono::milliseconds(500));	// 休眠500ms
	}

 	LOG_INFO("Program starting exited\n");

try {
	#if VIDEORTSP_ENABLE
		delete videoRtsp;
		videoRtsp = nullptr;
		delete videoRtspFactory;
		videoRtspFactory = nullptr;
	#endif

	#if VIDEODISPLAY_ENABLE
		delete videoDiaplsy;
		videoDiaplsy = nullptr;
		delete videoDiaplayFactory;
		videoDiaplayFactory = nullptr;
	#endif

	#if VIDEOYOLO_ENABLE
		delete videoYolo;
		videoYolo = nullptr;
		delete videoYoloFactory;
		videoYoloFactory = nullptr;
	#endif

	#if CONTROLLER_ENABLE
		delete controllor;
		controllor = nullptr;
	#endif

	#if UART_ENABLE
		delete uart;
		uart = nullptr;
	#endif

	#if TCP_SERVER_ENABLE
		delete tcpServer;
		tcpServer = nullptr;
	#endif

	} catch (std::exception &e) {
        LOG_ERROR("Exception: %s\n", e.what());
    }

	// delete tcpServer;
	// rk_param_deinit();
    LOG_INFO("Program exited\n");
	return 0;
}

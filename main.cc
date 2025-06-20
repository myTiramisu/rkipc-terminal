#include "videoRTSP.h"	// videoRTSP
#include "videoYOLO.h"	// videoYOLO
#include "videoLCD.h"	// videoLCD
#include "display.h"	// lcd
#include "video_factory.h"
#include "displayFactory.h"
#include "ledFactory.h"
#include "ptzFactory.h"
#include "led.h"		// led
#include "pantilt.h"	// pantilt
#include "log.h"		// log
#include "param.h"		// param
#include "control.h"
#include <unistd.h>
#include "onvif_server.h"
#include "server.h"
#include "uart.h"
#include <atomic>


#define ONVIF_SERVER_ENABLE 	1

#define LED_ENABLE 				1
#define PTZ_ENABLE 				1
#define DISPLAY_ENABLE 			1

#define VIDEORTSP_ENABLE 		1
#define VIDEODISPLAY_ENABLE 	1
#define VIDEOYOLO_ENABLE 		1

#define UART_ENABLE 			0
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
	
	// 初始化基类工厂指针
	AbstractFactory* factory = nullptr;

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

#if PTZ_ENABLE
	// 俯仰旋转控制
	LOG_INFO("PTZ Module init\n");
	factory = new PtzFactory();
	AbstractModule* ptz = factory->createModule();
#endif

#if DISPLAY_ENABLE
	LOG_INFO("display Module init\n");
	factory = new DisplayFactory();
	AbstractModule* display = factory->createModule();
#endif

#if VIDEORTSP_ENABLE
	LOG_INFO("VideoRTSP Module init\n");
	factory = new VideoRTSPFactory();
	AbstractModule* videoRTSP = factory->createModule();
#endif

#if VIDEODISPLAY_ENABLE
	LOG_INFO("VideoDisplay Module init\n");
	factory = new VideoDisplayFactory();
	AbstractModule* videoDisplay = factory->createModule();
#endif

#if VIDEOYOLO_ENABLE
	LOG_INFO("VideoYOLO Module init\n");
	factory = new VideoYOLOFactory();
	AbstractModule* videoYOLO = factory->createModule();
#endif

#if VIDEODISPLAY_ENABLE && DISPLAY_ENABLE
	LOG_INFO("videoDisplay connect to display\n");
	// 使用lambda表达式连接信号与槽
	dynamic_cast<VideoLCD*>(videoDisplay)->video_frame_signal.connect(
    [display](const cv::Mat& frame) {
        dynamic_cast<Display*>(display)->push_frame(frame);
    }
	);
	// 使用lambda表达式连接信号与槽
	// dynamic_cast<VideoLCD*>(videoDisplay)->video_frame_signal.connect(dynamic_cast<Display*>(display.get()), &Display::push_frame);
	// videoDisplay->video_frame_signal.connect(dynamic_cast<Display*>(display.get()), &Display::push_frame);
#endif
	
#if UART_ENABLE
	Uart* uart = new Uart(4);
	LOG_INFO("uart Module init success\n");
#endif

#if TCP_SERVER_ENABLE
	TcpServer* tcpServer = new TcpServer();
	LOG_INFO("tcpServer Module init\n");
#endif


#if LED_ENABLE
	// LED模块
	LOG_INFO("led Module init\n");
	factory = new LedFactory();
	ModuleParams LEDParam;
	LEDParam.publisher = tcpServer; // 发布者为tcpServer
	// led0
	LEDParam.pin = LED0;
	LEDParam.name = "LED0";
	AbstractModule* led0 = factory->createModule(LEDParam);
	LEDParam.pin = LED1;
	LEDParam.name = "LED1";
	AbstractModule* led1 = factory->createModule(LEDParam);
	LEDParam.pin = LED2;
	LEDParam.name = "LED2";
	AbstractModule* led2 = factory->createModule(LEDParam);
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
	#if LED_ENABLE
		delete led0;
		led0 = nullptr;
		delete led1;
		led1 = nullptr;
		delete led2;
		led2 = nullptr;
	#endif

	#if PTZ_ENABLE
		delete ptz;
		ptz = nullptr;
	#endif

	#if DISPLAY_ENABLE
		delete display;
		display = nullptr;
	#endif			

	#if VIDEORTSP_ENABLE
		delete videoRTSP;
		videoRTSP = nullptr;
	#endif

	#if VIDEODISPLAY_ENABLE
		delete videoDisplay;
		videoDisplay = nullptr;
	#endif

	#if VIDEOYOLO_ENABLE
		delete videoYOLO;
		videoYOLO = nullptr;
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

	if(factory) {
		delete factory;
		factory = nullptr;
	}

	} catch (std::exception &e) {
        LOG_ERROR("Exception: %s\n", e.what());
    }

	// delete tcpServer;
	// rk_param_deinit();
    LOG_INFO("Program exited\n");
	return 0;
}

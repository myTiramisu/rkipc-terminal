#include "video.h"		// video
#include "display.h"	// lcd
#include "led.h"		// led
#include "pantilt.h"	// pantilt
#include "log.h"		// log
#include "param.h"		// param

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
	
	// 实例化对象	
	Video video;
	Display lcd;

	// 俯仰旋转控制
	Pantilt pantilt;

	// LED模块
	Led *led0 = new Led(LED0);
	Led *led1 = new Led(LED1);
	Led *led2 = new Led(LED2);	
	led0->off();
	led1->off();
	led2->off();

	// 连接 video 与 lcd
	// sender.signal.connect(&receiver, &Receiver::receiveData);
	video.video_frame_signal.connect(&lcd, &Display::push_frame);

  	while(!quit)
	{	
		sleep(1);
	}

	rk_param_deinit();
    LOG_INFO("Program exited\n");
	return 0;
}

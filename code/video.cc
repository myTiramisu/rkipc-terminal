#include "video.h"

//opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

Video::Video()
{
    quit_flag = false;

    // vi init	 	初始化视频输入设备和通道
    rkaiq_init();
    rkmpi_sys_init();    // rkmpi_sys_init
	vi_dev_init();
    rtsp_init();

    video_thread0 = new std::thread(&Video::video_thread_0, this);
    video_thread1 = new std::thread(&Video::video_thread_1, this);
    // video_thread2 = new std::thread(&Video::video_thread2, this);
}

Video::~Video()
{
    {
        std::lock_guard<std::mutex> lock(mutex_video);
        quit_flag = true;
    }
    printf("******************************start release video\n");
    if (video_thread0->joinable()) {
        video_thread0->join();
    }
    if (video_thread1->joinable()) {
        video_thread1->join();
    }
    if (video_thread2->joinable()) {
        video_thread2->join();
    }

    // 构造函数的反初始化
    rtsp_release();
    vi_dev_deinit();
    rkmpi_sys_deinit();
    rkaiq_deinit();
    printf("******************************Release video success\n");
}

// rtsp video
void Video::video_thread_0()
{
    std::cout << "************************video_thread_0 started success" << std::endl;
    
    int video_width  = 2304;
    int video_height = 1296;
    int pipeId = 0;            // pipeId
    int viChannelId = 0;       // VI channel id
    int vencChannelId = 0;     // VENC channel id

    // 获取编码后的帧   
    VENC_STREAM_S stFrame;      
    stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S));

    // 初始化视频输入通道 
    vi_chn_init(viChannelId, video_width, video_height);
    // 初始化视频编码器 设置编码类型为 H264		
	venc_init(pipeId, video_width, video_height, RK_VIDEO_ID_AVC);


    // 绑定输入通道到编码器
    MPP_CHN_S vi_chn;       // 视频输入通道
    MPP_CHN_S venc_chn;     // 编码器通道
    bind_vi_to_venc(pipeId, &vi_chn, &venc_chn);

    while (!quit_flag)
    {
        // 获取编码后的帧，发送到 RTSP 服务器
        rtsp_send_frame_h264(vencChannelId, &stFrame);

        // 释放编码后的帧
        rtsp_release_frame_h264(vencChannelId, &stFrame);
    }

    sleep(1);
    unbind_vi_to_venc(pipeId, &vi_chn, &venc_chn);
    venc_deinit(vencChannelId);
    vi_chn_deinit(pipeId, viChannelId);
    free(stFrame.pstPack);
} 


// lcd display
void Video::video_thread_1()
{
    std::cout << "******************************video_thread_1 started" << std::endl;

    int video_width  = 640;
    int video_height = 480;
    int pipeId = 0;            // pipeId
    int viChannelId =   1;     // VI channel id
    int vencChannelId = 1;     // VENC channel id

    // h264_frame
    VENC_STREAM_S stFrame;          // 获取编码后的帧 用于RTSP发送
    VIDEO_FRAME_INFO_S stViFrame;   // 视频输入设备
    stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S));

    // 初始化视频输入通道
    vi_chn_init(viChannelId, video_width, video_height);
    // 初始化视频编码器
    venc_init(vencChannelId, video_width, video_height, RK_VIDEO_ID_AVC);

    // 绑定输入通道到编码器
    MPP_CHN_S vi_chn;       // 视频输入通道
    MPP_CHN_S venc_chn;     // 编码器通道
    bind_vi_to_venc(pipeId, &vi_chn, &venc_chn);

    while (!quit_flag)
    {
        // 从输入摄像头获取视频帧
        RK_S32 s32Ret = RK_MPI_VI_GetChnFrame(pipeId, viChannelId, &stViFrame, -1);
        if (s32Ret == RK_SUCCESS)
        {
            // 获取 VENC 帧内存块虚拟地址
            void *data = RK_MPI_MB_Handle2VirAddr(stViFrame.stVFrame.pMbBlk);
            //opencv	
			//将帧转换为OpenCV格式
			cv::Mat frame(video_height, video_width, CV_8UC3,data);

        }
        
        RK_MPI_VENC_ReleaseStream(vencChannelId, &stFrame);

    }

}
#include "videoLCD.h"
VideoLCD::VideoLCD(ModuleParams params):VideoBase(params)
{
    pipeId = 0;
    viChannelId = 1;
    vencChannelId = 1;
    video_width = 720;
    video_height = 480;

    quit_flag = false;
    video_thread = new std::thread(&VideoLCD::video_thread_func, this);
}

VideoLCD::~VideoLCD()
{
    quit_flag = true;
    if(video_thread->joinable())
        video_thread->join();
    delete video_thread;
    LOG_DEBUG("******************************VideoLCD exit\n");
}



void VideoLCD::notify(std::string msg) 
{

}

void VideoLCD::update(Publisher* publisher, string msg)
{

}


void VideoLCD::videoCapture()
{

}
void VideoLCD::videoEncode()
{

}
    
void VideoLCD::videoRtspTransmit()
{

}



void VideoLCD::video_thread_func()
{
    LOG_DEBUG("******************************VideoLCD  video_thread_func start\n");

#if FPS_SHOW
    char fps_text[16];
    float fps = 0;
    memset(fps_text, 0, 16);
    int x_scaled = (float)50.0 / 704 * video_width;
    int y_scaled = (float)50.0 / 576 * video_height;
    int font_scaled = (float)1.0 / 576 * video_height;
    int thickness_scaled = (float)1.0 / 576 * video_height;
#endif


    stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S));

    MB_BLK src_blk;
    MB_POOL src_Pool;
    create_MB_pool(&src_blk, &src_Pool, video_width, video_height);

    
    venc_frame.stVFrame.u32Width = video_width;
    venc_frame.stVFrame.u32Height = video_height;
    venc_frame.stVFrame.u32VirWidth = video_width;
    venc_frame.stVFrame.u32VirHeight = video_height;
    venc_frame.stVFrame.enPixelFormat = RK_FMT_RGB888;
    venc_frame.stVFrame.u32FrameFlag = 160;
    venc_frame.stVFrame.pMbBlk = src_blk;

    unsigned char *venc_data = (unsigned char *)RK_MPI_MB_Handle2VirAddr(src_blk);
    cv::Mat frame(cv::Size(video_width, video_height), CV_8UC3, venc_data);
    cv::Mat yuv420sp(video_height + video_height / 2, video_width, CV_8UC1);
    cv::Mat bgr(video_height, video_width, CV_8UC3);

    vi_chn_init(viChannelId, video_width, video_height);
    venc_init(vencChannelId, video_width, video_height, RK_VIDEO_ID_AVC, RK_FMT_RGB888);

    while (!quit_flag)
    {
        yuv420sp.data = (unsigned char *)vi_get_frame(pipeId, viChannelId, video_width, video_height, &stViFrame);

        bgr.data = venc_data;
        cv::cvtColor(yuv420sp, bgr, cv::COLOR_YUV420sp2BGR);
        cv::resize(bgr, frame, cv::Size(video_width, video_height), 0, 0, cv::INTER_LINEAR);
        memcpy(venc_data, frame.data, video_width * video_height * 3);
        
        cv::Mat dst;
        cv::resize(frame, dst, cv::Size(160, 128));
        video_frame_signal.emit(dst);

#if FPS_SHOW
        sprintf(fps_text, "fps = %.2f", fps);
        cv::putText(dst, fps_text,
                    cv::Point(x_scaled, y_scaled),
                    cv::FONT_HERSHEY_SIMPLEX, 1,
                    cv::Scalar(0, 255, 0), 1);
#endif

        venc_encode_frame(vencChannelId, &venc_frame);      // 编码
        rtsp_send_frame_h264(vencChannelId, &stFrame);      // 发送

#if FPS_SHOW
        RK_U64 nowUs = TEST_COMM_GetNowUs();
        fps = (float)1000000 / (float)(nowUs - venc_frame.stVFrame.u64PTS);
#endif
        venc_release_frame(vencChannelId, &stFrame);        // 释放
        vi_release_frame(pipeId, viChannelId, &stViFrame);  // 释放   
    
    }

    venc_deinit(vencChannelId);
    vi_chn_deinit(pipeId, viChannelId);
    free(stFrame.pstPack);
    destroy_MB_pool(&src_blk, &src_Pool);
    LOG_DEBUG("******************************VideoLCD  video_thread_func exit\n");
}


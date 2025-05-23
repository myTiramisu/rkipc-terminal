#include "videoLCD.h"
VideoLCD::VideoLCD()
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


void VideoLCD::videoCapture()
{
   if(vi_get_frame(pipeId, viChannelId, video_width, video_height, &stViFrame) != 0)
    {
        LOG_DEBUG("******************************vi_get_frame error\n");
        return;
    }
}
void VideoLCD::videoEncode()
{
    if(venc_encode_frame(vencChannelId, &venc_frame) != 0) {
        LOG_DEBUG("******************************venc_encode_frame error\n");
        return;
    }
}
    
void VideoLCD::videoRtspTransmit()
{
    if(rtsp_send_frame_h264(vencChannelId, &stFrame) != 0) {
        LOG_DEBUG("******************************rtsp_send_frame_h264 error\n");
        return;
    }
}



void VideoLCD::video_thread_func()
{
    LOG_DEBUG("******************************VideoLCD  video_thread_func start\n");

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

        videoEncode();
        videoRtspTransmit();
        
        venc_release_frame(vencChannelId, &stFrame);    
        vi_release_frame(pipeId, viChannelId, &stViFrame);       
    }

    venc_deinit(vencChannelId);
    vi_chn_deinit(pipeId, viChannelId);
    free(stFrame.pstPack);
    destroy_MB_pool(&src_blk, &src_Pool);
    LOG_DEBUG("******************************VideoLCD  video_thread_func exit\n");
}


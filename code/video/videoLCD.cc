#include "videoLCD.h"


VideoLCD::VideoLCD()
{
    // 初始化特定于 LCD 的资源
    start_thread();
}

VideoLCD::~VideoLCD()
{
    stop_thread();
}


void VideoLCD::video_thread_func()
{
    LOG_DEBUG("******************************video_thread_1 started\n");
    int pipeId = 0;
    int viChannelId = 1;
    int vencChannelId = 1;
    int video_width = 720;
    int video_height = 480;

    VENC_STREAM_S stFrame;
    VIDEO_FRAME_INFO_S stViFrame;
    stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S));

    MB_BLK src_blk;
    MB_POOL src_Pool;
    create_MB_pool(&src_blk, &src_Pool, video_width, video_height);

    VIDEO_FRAME_INFO_S venc_frame;
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

        venc_encode_frame(vencChannelId, &venc_frame);
        rtsp_send_frame_h264(vencChannelId, &stFrame);
        venc_release_frame(vencChannelId, &stFrame);    

        vi_release_frame(pipeId, viChannelId, &stViFrame);       
    }

    venc_deinit(vencChannelId);
    vi_chn_deinit(pipeId, viChannelId);
    free(stFrame.pstPack);
    destroy_MB_pool(&src_blk, &src_Pool);
    LOG_DEBUG("******************************video_thread_1 exit\n");
}


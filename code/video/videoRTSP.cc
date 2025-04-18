#include "videoRTSP.h"

VideoRTSP::VideoRTSP()
{
    // 初始化特定于 RTSP 的资源
    rkaiq_init();
    rkmpi_sys_init();
    vi_dev_init();
    rtsp_init();
    start_thread();
}

VideoRTSP::~VideoRTSP()
{
    // 释放特定于 RTSP 的资源
    rtsp_release();
    vi_dev_deinit();
    rkmpi_sys_deinit();
    rkaiq_deinit();

    stop_thread();
}
void VideoRTSP::video_thread_func()
{
    LOG_DEBUG("************************video_thread_0 started success\n");
    int video_width  = 2304;
    int video_height = 1296;
    int pipeId = 0;            // pipeId
    int viChannelId = 0;       // VI channel id
    int vencChannelId = 0;     // VENC channel id

    VENC_STREAM_S stFrame;
    stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S));

    vi_chn_init(viChannelId, video_width, video_height);
    venc_init(pipeId, video_width, video_height, RK_VIDEO_ID_AVC, RK_FMT_YUV420SP);

    MPP_CHN_S vi_chn;
    MPP_CHN_S venc_chn;
    bind_vi_to_venc(pipeId, &vi_chn, &venc_chn);

    rkipc_osd_init();

    while (!quit_flag)
    {
        rtsp_send_frame_h264(vencChannelId, &stFrame);
        venc_release_frame(vencChannelId, &stFrame);
    }

    rkipc_osd_deinit();
    unbind_vi_to_venc(pipeId, &vi_chn, &venc_chn);
    venc_deinit(vencChannelId);
    vi_chn_deinit(pipeId, viChannelId);
    free(stFrame.pstPack);
    LOG_DEBUG("******************************video_thread_0 release success");
}

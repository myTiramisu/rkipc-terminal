#include "videoRTSP.h"

VideoRTSP::VideoRTSP(int pipeId, int viChannelId, int vencChannelId, int width, int height)
: pipeId(pipeId), viChannelId(viChannelId), vencChannelId(vencChannelId), video_width(width), video_height(height)
{
    // video_width  = 2304;
    // video_height = 1296;
    // pipeId = 0;            // pipeId
    // viChannelId = 0;       // VI channel id
    // vencChannelId = 0;     // VENC channel id

    quit_flag = false;
    // 初始化特定于 RTSP 的资源
    rkaiq_init();
    rkmpi_sys_init();
    vi_dev_init();
    rtsp_init();
    
    video_thread = new std::thread(&VideoRTSP::video_thread_func, this);
}

VideoRTSP::~VideoRTSP()
{
    // 释放特定于 RTSP 的资源
    rtsp_release();
    vi_dev_deinit();
    rkmpi_sys_deinit();
    rkaiq_deinit();

    if(video_thread->joinable()) {
        video_thread->join();
    }
    delete video_thread;

    if(stFrame.pstPack != nullptr){
        free(stFrame.pstPack);
    }
}


void VideoRTSP::videoCapture() 
{
    if (stFrame.pstPack == nullptr) {
        stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S));
        if (stFrame.pstPack == nullptr) {
            LOG_ERROR("Failed to allocate memory for VENC_PACK_S\n");
            return;
        }
    }

    if (vi_chn_init(viChannelId, video_width, video_height) != 0) {
        LOG_ERROR("Failed to initialize VI channel\n");
        return;
    }
    
    if (vi_get_frame(pipeId, viChannelId, video_width, video_height, &stViFrame) != 0) {
        LOG_ERROR("Failed to get frame from VI channel\n");
        return;
    }
}

void VideoRTSP::videoEncode() 
{
    if (venc_init(pipeId, video_width, video_height, RK_VIDEO_ID_AVC, RK_FMT_YUV420SP) != 0) {
        LOG_ERROR("Failed to initialize VENC\n");
        return;
    }

    if (bind_vi_to_venc(pipeId, &vi_chn, &venc_chn) != 0) {
        LOG_ERROR("Failed to bind VI to VENC\n");
        return;
    }
}

void VideoRTSP::videoRtspTransmit() 
{
    if (rtsp_send_frame_h264(vencChannelId, &stFrame) != 0) {
        LOG_ERROR("Failed to send H264 frame via RTSP\n");
        return;
    }

    if (venc_release_frame(vencChannelId, &stFrame) != 0) {
        LOG_ERROR("Failed to release VENC frame\n");
        return;
    }
 }

void VideoRTSP::video_thread_func()
{
    LOG_DEBUG("************************VideoRTSP thread started success\n");

    stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S));

    vi_chn_init(viChannelId, video_width, video_height);
    venc_init(pipeId, video_width, video_height, RK_VIDEO_ID_AVC, RK_FMT_YUV420SP);
    bind_vi_to_venc(pipeId, &vi_chn, &venc_chn);

    rkipc_osd_init();

    videoCapture();
    videoEncode();
    while (!quit_flag)
    {
        // rtsp_send_frame_h264(vencChannelId, &stFrame);
        // venc_release_frame(vencChannelId, &stFrame);
        videoRtspTransmit();
    }

    rkipc_osd_deinit();
    unbind_vi_to_venc(pipeId, &vi_chn, &venc_chn);
    venc_deinit(vencChannelId);
    vi_chn_deinit(pipeId, viChannelId);
    free(stFrame.pstPack);
    LOG_DEBUG("******************************VideoRTSP thread release success");
}

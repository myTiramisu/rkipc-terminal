#ifndef _LUCKFOX_VIDEO_H_
#define _LUCKFOX_VIDEO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "sample_comm.h"
#include "rk_common.h"

int vi_dev_init(void);
int vi_chn_init(int channelId, int width, int height);
int vpss_init(int VpssChn, int width, int height);
int venc_init(int chnId, int width, int height, RK_CODEC_ID_E enType);
int rkaiq_init(void);
int rkmpi_init(void);

int bind_vi_to_vpss(MPP_CHN_S *stSrcChn, MPP_CHN_S *stvpssChn, int chnID);
int venc_frame(int channelId, VIDEO_FRAME_INFO_S* stVpssFrame, VENC_STREAM_S* frame);

int vi_dev_deinit(void);
int vi_chn_deinit(int channelId);
int vpss_deinit(int VpssChn);
int venc_deinit(int chnId);
int rkaiq_deinit(void);
int rkmpi_deinit(void);
int venc_release_frame(VIDEO_FRAME_INFO_S *pstVideoFrame);

#ifdef __cplusplus
}
#endif
#endif
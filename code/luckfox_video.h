#ifndef _LUCKFOX_VIDEO_H_
#define _LUCKFOX_VIDEO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "sample_comm.h"
#include "rk_common.h"
#define TEST_ARGB32_PIX_SIZE 4
#define TEST_ARGB32_RED 0xFF0000FF
#define TEST_ARGB32_GREEN 0x00FF00FF
#define TEST_ARGB32_BLUE 0x0000FFFF
#define TEST_ARGB32_TRANS 0x00000000
#define TEST_ARGB32_BLACK 0x000000FF


RK_U64 TEST_COMM_GetNowUs();
RK_S32 test_rgn_overlay_line_process(int sX ,int sY,int type, int group);
RK_S32 rgn_overlay_release(int group);



int vi_dev_init(void);
int vi_chn_init(int channelId, int width, int height);
int vpss_init(int VpssChn, int width, int height);
int venc_init(int chnId, int width, int height, RK_CODEC_ID_E enType);
int rkaiq_init(void);
int rkmpi_sys_init(void);
int bind_vi_to_venc(int pipeId, MPP_CHN_S *vi_chn, MPP_CHN_S *venc_chn);
int unbind_vi_to_venc(int pipeId, MPP_CHN_S *vi_chn, MPP_CHN_S *venc_chn);


int bind_vi_to_vpss(int chnID, MPP_CHN_S *stSrcChn, MPP_CHN_S *stvpssChn);
int unbind_vi_to_vpss(MPP_CHN_S *stSrcChn, MPP_CHN_S *stvpssChn);

int get_venc_frame(int channelId, VIDEO_FRAME_INFO_S* stVpssFrame, VENC_STREAM_S* frame);

int vi_dev_deinit(void);
int vi_chn_deinit(int pipeId, int channelId);
int vpss_deinit(int VpssChn);
int venc_deinit(int chnId);
int rkaiq_deinit(void);
int rkmpi_sys_deinit(void);
int venc_release_frame(VIDEO_FRAME_INFO_S *pstVideoFrame);

#ifdef __cplusplus
}
#endif
#endif
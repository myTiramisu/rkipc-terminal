#ifndef _RTSP_H_
#define _RTSP_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "rk_comm_venc.h"


void rtsp_init(void);
void rtsp_release(void);
int rtsp_send_frame_h264(int channelId, VENC_STREAM_S* stFrame);
int rtsp_release_frame_h264(int channelId, VENC_STREAM_S* stFrame);


#ifdef __cplusplus
}
#endif
#endif
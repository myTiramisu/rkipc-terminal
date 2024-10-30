#ifndef _RTSP_H_
#define _RTSP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "rtsp_demo.h"
#include "rk_comm_venc.h"
#include <thread>
#include "rk_mpi_venc.h"
#include "rk_mpi_mb.h"

void rtsp_init(void);
void rtsp_release(void);
int rtsp_send_frame_h264(int channelId, VENC_STREAM_S* stFrame);
int rtsp_release_frame_h264(int channelId, VENC_STREAM_S* stFrame);

// int rtsp_send_frame(int channelId, unsigned char* data, int size);
// int rtsp_send_frame_yuv(int channelId, unsigned char* data, int size);


#ifdef __cplusplus
}
#endif
#endif
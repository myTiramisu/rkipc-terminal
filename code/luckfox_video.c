#include "luckfox_video.h"
int vi_dev_init(void) {
	printf("%s\n", __func__);
	int ret = 0;
	int devId = 0;
	int pipeId = devId;

	VI_DEV_ATTR_S stDevAttr;
	VI_DEV_BIND_PIPE_S stBindPipe;
	memset(&stDevAttr, 0, sizeof(stDevAttr));
	memset(&stBindPipe, 0, sizeof(stBindPipe));
	// 0. get dev config status
	ret = RK_MPI_VI_GetDevAttr(devId, &stDevAttr);
	if (ret == RK_ERR_VI_NOT_CONFIG) {
		// 0-1.config dev
		ret = RK_MPI_VI_SetDevAttr(devId, &stDevAttr);
		if (ret != RK_SUCCESS) {
			printf("RK_MPI_VI_SetDevAttr %x\n", ret);
			return -1;
		}
	} else {
		printf("RK_MPI_VI_SetDevAttr already\n");
	}
	// 1.get dev enable status
	ret = RK_MPI_VI_GetDevIsEnable(devId);
	if (ret != RK_SUCCESS) {
		// 1-2.enable dev
		ret = RK_MPI_VI_EnableDev(devId);
		if (ret != RK_SUCCESS) {
			printf("RK_MPI_VI_EnableDev %x\n", ret);
			return -1;
		}
		// 1-3.bind dev/pipe
		stBindPipe.u32Num = pipeId;
		stBindPipe.PipeId[0] = pipeId;
		ret = RK_MPI_VI_SetDevBindPipe(devId, &stBindPipe);
		if (ret != RK_SUCCESS) {
			printf("RK_MPI_VI_SetDevBindPipe %x\n", ret);
			return -1;
		}
	} else {
		printf("RK_MPI_VI_EnableDev already\n");
	}
	printf("\n***********************vi_dev_init  success!\n");
	return 0;
}
int vi_dev_deinit(void)
{
	printf("%s\n", __func__);
	int ret = 0;
	int devId = 0;

	ret = RK_MPI_VI_DisableDev(devId);
	if (ret != RK_SUCCESS) {
		printf("RK_MPI_VI_DisableDev %x\n", ret);
		return -1;
	}
	return 0;
}


int vi_chn_init(int channelId, int width, int height) {
	int ret;
	int buf_cnt = 2;
	// VI init
	VI_CHN_ATTR_S vi_chn_attr;
	memset(&vi_chn_attr, 0, sizeof(vi_chn_attr));
	vi_chn_attr.stIspOpt.u32BufCount = buf_cnt;
	vi_chn_attr.stIspOpt.enMemoryType =
	    VI_V4L2_MEMORY_TYPE_DMABUF; // VI_V4L2_MEMORY_TYPE_MMAP;
	vi_chn_attr.stSize.u32Width = width;
	vi_chn_attr.stSize.u32Height = height;
	vi_chn_attr.enPixelFormat = RK_FMT_YUV420SP;
	vi_chn_attr.enCompressMode = COMPRESS_MODE_NONE; // COMPRESS_AFBC_16x16;
	vi_chn_attr.u32Depth = 2;
	ret = RK_MPI_VI_SetChnAttr(0, channelId, &vi_chn_attr);
	ret |= RK_MPI_VI_EnableChn(0, channelId);
	if (ret) {
		printf("ERROR: create VI error! ret=%d\n", ret);
		return ret;
	}

	printf("\n************************vi_chn_init success\n");
	return ret;
}
int vi_chn_deinit(int channelId) {
	int ret;
	ret = RK_MPI_VI_DisableChn(0, channelId);
	if(ret)
	{
		printf("vi_chn_deinit error!");
		return -1;
	}
	return 0;
}


int vpss_init(int VpssChn, int width, int height) {
	printf("%s\n",__func__);
	int s32Ret;
	VPSS_CHN_ATTR_S stVpssChnAttr;
	VPSS_GRP_ATTR_S stGrpVpssAttr;

	int s32Grp = 0;

	stGrpVpssAttr.u32MaxW = 4096;
	stGrpVpssAttr.u32MaxH = 4096;
	stGrpVpssAttr.enPixelFormat = RK_FMT_YUV420SP;
	stGrpVpssAttr.stFrameRate.s32SrcFrameRate = -1;
	stGrpVpssAttr.stFrameRate.s32DstFrameRate = -1;
	stGrpVpssAttr.enCompressMode = COMPRESS_MODE_NONE;

	stVpssChnAttr.enChnMode = VPSS_CHN_MODE_USER;
	stVpssChnAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
	stVpssChnAttr.enPixelFormat = RK_FMT_RGB888;
	stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
	stVpssChnAttr.u32Width = width;
	stVpssChnAttr.u32Height = height;
	stVpssChnAttr.enCompressMode = COMPRESS_MODE_NONE;

	s32Ret = RK_MPI_VPSS_CreateGrp(s32Grp, &stGrpVpssAttr);
	if (s32Ret != RK_SUCCESS) {
		return s32Ret;
	}

	s32Ret = RK_MPI_VPSS_SetChnAttr(s32Grp, VpssChn, &stVpssChnAttr);
	if (s32Ret != RK_SUCCESS) {
		return s32Ret;
	}
	s32Ret = RK_MPI_VPSS_EnableChn(s32Grp, VpssChn);
	if (s32Ret != RK_SUCCESS) {
		return s32Ret;
	}

	s32Ret = RK_MPI_VPSS_StartGrp(s32Grp);
	if (s32Ret != RK_SUCCESS) {
		return s32Ret;
	}


	printf("\n************************venc init success\n");	
	return s32Ret;
}

int vpss_deinit(int VpssChn) {
	printf("%s\n",__func__);
	int s32Ret;
	s32Ret = RK_MPI_VPSS_DisableChn(0, VpssChn);
	if (s32Ret != RK_SUCCESS) {
		return s32Ret;
	}
	s32Ret = RK_MPI_VPSS_StopGrp(0);
	return 0;
}


int venc_init(int chnId, int width, int height, RK_CODEC_ID_E enType) {
	printf("%s\n",__func__);
	VENC_RECV_PIC_PARAM_S stRecvParam;
	VENC_CHN_ATTR_S stAttr;
	memset(&stAttr, 0, sizeof(VENC_CHN_ATTR_S));

	// RTSP H264	
	stAttr.stVencAttr.enType = enType;
	//stAttr.stVencAttr.enPixelFormat = RK_FMT_YUV420SP;
	stAttr.stVencAttr.enPixelFormat = RK_FMT_RGB888;	
	stAttr.stVencAttr.u32Profile = H264E_PROFILE_MAIN;
	stAttr.stVencAttr.u32PicWidth = width;
	stAttr.stVencAttr.u32PicHeight = height;
	stAttr.stVencAttr.u32VirWidth = width;
	stAttr.stVencAttr.u32VirHeight = height;
	stAttr.stVencAttr.u32StreamBufCnt = 2;
	stAttr.stVencAttr.u32BufSize = width * height * 3 / 2;
	stAttr.stVencAttr.enMirror = MIRROR_NONE;
		
	stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
	stAttr.stRcAttr.stH264Cbr.u32BitRate = 3 * 1024;
	stAttr.stRcAttr.stH264Cbr.u32Gop = 1;
	RK_MPI_VENC_CreateChn(chnId, &stAttr);

	memset(&stRecvParam, 0, sizeof(VENC_RECV_PIC_PARAM_S));
	stRecvParam.s32RecvPicNum = -1;
	RK_MPI_VENC_StartRecvFrame(chnId, &stRecvParam);

	printf("\n************************venc init success\n");	
	return 0;
}

int venc_deinit(int chnId) {
	printf("%s\n",__func__);
	int ret = RK_MPI_VENC_StopRecvFrame(chnId);
	ret != RK_MPI_VENC_DestroyChn(chnId);
	if(ret) {
		printf("venc_deinit error!");
		return -1;
	}
	return 0;
} 

// 绑定视频输入通道到视频处理子系统通道
/* 多媒体处理通道
typedef struct rkMPP_CHN_S {
  MOD_ID_E enModId;		// 表示模块ID
  RK_S32 s32DevId;	
  RK_S32 s32ChnId;
} MPP_CHN_S;
*/
// stSrcChn:  表示从摄像头获取的原始视频流的通道
// stvpssChn: 视频处理子系统（Video Processing Subsystem, VPSS）通道
int bind_vi_to_vpss(MPP_CHN_S *stSrcChn, MPP_CHN_S *stvpssChn, int chnID) 
{
	int s32Ret;
	
	stSrcChn->enModId = RK_ID_VI;
	stSrcChn->s32DevId = 0;
	stSrcChn->s32ChnId = chnID;
	stvpssChn->enModId = RK_ID_VPSS;
	stvpssChn->s32DevId = 0;
	stvpssChn->s32ChnId = chnID;
	printf("\n==== RK_MPI_SYS_Bind vi0 to vpss chnID ====\n");
	s32Ret = RK_MPI_SYS_Bind(stSrcChn, stvpssChn);
	if (s32Ret != RK_SUCCESS) {
		RK_LOGE("bind %d ch venc failed", chnID);
		return -1;
	}
	printf("\n************************RK_MPI_SYS_Bind  success!\n");
	return 0;
}



int venc_frame(int channelId, VIDEO_FRAME_INFO_S* stVpssFrame, VENC_STREAM_S* frame)
{
	// 发送视频帧到编码器
    RK_S32 s32Ret = RK_MPI_VENC_SendFrame(channelId, stVpssFrame, -1);
    if (s32Ret != RK_SUCCESS) {
        printf("Error: Failed to send frame to encoder, error code: %d\n", s32Ret);
        return -1;
    }

	// 从编码器获取编码后的流
	s32Ret = RK_MPI_VENC_GetStream(channelId, frame, -1);
    if (s32Ret != RK_SUCCESS) {
        printf("Error: Failed to get stream from encoder, error code: %d\n", s32Ret);
        return -1;
    }
    return 0;
}

int venc_release_frame(VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	RK_S32 s32Ret = RK_MPI_VPSS_ReleaseChnFrame(0, 0, pstVideoFrame);
	if (s32Ret != RK_SUCCESS) 
	{
		printf("RK_MPI_VI_ReleaseChnFrame error\n");
		return -1;
	}
	return 0;
}



int rkaiq_init(void)
{
	RK_BOOL multi_sensor = RK_FALSE;	
	const char *iq_dir = "/etc/iqfiles";
	rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
	SAMPLE_COMM_ISP_Init(0, hdr_mode, multi_sensor, iq_dir);
	SAMPLE_COMM_ISP_Run(0);
	printf("\n************************Init rkaiq  success!\n");
	return 0;
}

int rkaiq_deinit(void)
{
	int ret = SAMPLE_COMM_ISP_Stop(0);
	if (ret) {
		printf("rkaiq_deinit error!\n");
		return -1;
	}	
	return 0;
}


int rkmpi_init(void)
{
	int ret = RK_MPI_SYS_Init();
	if (ret) {
		printf("rkmpi_init error!\n");
		return -1;
	}
	return 0;
}


int rkmpi_deinit(void)
{
	int ret = RK_MPI_SYS_Exit();
	if (ret) {
		printf("rkmpi_deinit error!\n");
		return -1;
	}
	return 0;
}
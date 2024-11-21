#include "luckfox_osd_draw.h"


OsdDraw::OsdDraw()
{
    osd_rgn_draw_flag = false;
    coverHandle = 0;
    osd_drwn_rgn_init();
    rgn_draw_thread_id = new std::thread(&OsdDraw::rgn_draw_thread, this);
    osd_rgn_draw_flag = true;
}

OsdDraw::~OsdDraw()
{

}


// 创建/初始化一个覆盖区域 
// 参数: 区域句柄
/*
步骤1： 调用RK_MPI_RGN_Create 填充区域属性并创建区域。
步骤2：调用RK_MPI_RGN_AttachToChn 将画布绑定到通道特定区域上。
步骤3：调用RK_MPI_RGN_GetCanvasInfo 获取画布信息。
步骤4：将位图数据写入画布信息中。
步骤5：调用RK_MPI_RGN_UpdateCanvas 更新画布。
更新画布信息时重复步骤3 ~ 步骤5。
步骤6：不用时调用 RK_MPI_RGN_DetachFromChn 将画布从绑定通道中解绑。
步骤7：调用 RK_MPI_RGN_Destroy 销毁区域。
*/
int OsdDraw::osd_drwn_rgn_init()   
{
    int ret = 0;
    RGN_ATTR_S stCoverAttr;         // 存储区域的属性
    RGN_CHN_ATTR_S stCoverChnAttr;  // 存储区域在特定通道上的属性
    // 创建覆盖区域 设置stCoverAttr属性
	memset(&stCoverAttr, 0, sizeof(stCoverAttr));
    stCoverAttr.enType = OVERLAY_RGN;   // 设置区域类型为覆盖区域
    // 覆盖区域的像素格式 每个像素占用 2 位数据，可以表示 4 种颜色
    stCoverAttr.unAttr.stOverlay.enPixelFmt = RK_FMT_2BPP;   
    stCoverAttr.unAttr.stOverlay.u32CanvasNum = 1;
    // 设置覆盖大小为线程1的大小
    stCoverAttr.unAttr.stOverlay.stSize.u32Width = 2304;
    stCoverAttr.unAttr.stOverlay.stSize.u32Height = 1296;
    ret = RK_MPI_RGN_Create(coverHandle, &stCoverAttr);
    if (ret != RK_SUCCESS) {
       	LOG_ERROR("RK_MPI_RGN_Create (%d) failed with %#x\n", coverHandle, ret);
		RK_MPI_RGN_Destroy(coverHandle);
		return RK_FAILURE;
    }
    // 设置/保存 属性
    ret = RK_MPI_RGN_SetAttr(coverHandle, &stCoverAttr);
	if (RK_SUCCESS != ret) {
		printf("RK_MPI_RGN_SetAttr (%d) failed with %#x!", coverHandle, ret);
		return RK_FAILURE;
	}

	// display cover regions to venc groups
    // 设置多媒体处理通道的属性
	stCoverChn.enModId = RK_ID_VENC;
	stCoverChn.s32DevId = 0;
    stCoverChn.s32ChnId = 0;
    // 存储区域在特定通道上的属性
	memset(&stCoverChnAttr, 0, sizeof(stCoverChnAttr));
	stCoverChnAttr.bShow = RK_TRUE;
	stCoverChnAttr.enType = COVER_RGN;
    // 设置大小
	stCoverChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
	stCoverChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
	stCoverChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = 128;
	stCoverChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = 128;
    //设置颜色
	stCoverChnAttr.unChnAttr.stOverlayChn.u32ColorLUT[0] = 0x00FF0000;
    stCoverChnAttr.unChnAttr.stOverlayChn.u32ColorLUT[1] = 0x00FFFF00;
    // 设置句柄
	stCoverChnAttr.unChnAttr.stOverlayChn.u32Layer = coverHandle;
    ret = RK_MPI_RGN_AttachToChn(coverHandle, &stCoverChn, &stCoverChnAttr);
    if (RK_SUCCESS != ret) {
        LOG_ERROR("RK_MPI_RGN_AttachToChn (%d) failed with %#x\n", coverHandle, ret);
        return RK_FAILURE;
    }
    LOG_INFO("RK_MPI_RGN_AttachToChn to venc0 success\n");

    return ret;
}
int OsdDraw::osd_drwn_rgn_deinit() 
{
    int ret = 0;
    RK_MPI_RGN_DetachFromChn(coverHandle, &stCoverChn);
    if (ret != RK_SUCCESS) {
        LOG_ERROR("RK_MPI_RGN_DetachFromChn (%d) failed with %#x\n", coverHandle, ret);
        return RK_FAILURE;
    }

    // 销毁覆盖区域
    ret = RK_MPI_RGN_Destroy(coverHandle);
    if (ret != RK_SUCCESS) {
        LOG_ERROR("RK_MPI_RGN_Destroy (%d) failed with %#x\n", coverHandle, ret);
        return RK_FAILURE;
    }

    // 停止绘制线程
    osd_rgn_draw_flag = false;
    // 等待线程结束
    if (rgn_draw_thread_id->joinable()) {
        rgn_draw_thread_id->join();
    }
    LOG_DEBUG("rgn_draw_thread success and xit");
    return ret;
}

void OsdDraw::rgn_draw_thread()
{
    int ret = 0;
    // 获取区域的显示画布信息
    RGN_CANVAS_INFO_S stCanvasInfo;
	memset(&stCanvasInfo, 0, sizeof(RGN_CANVAS_INFO_S));

    while (osd_rgn_draw_flag)
    {
        // 获取区域的显示画布信息
        ret = RK_MPI_RGN_GetCanvasInfo(coverHandle, &stCanvasInfo);
		if (ret != RK_SUCCESS) {
			LOG_ERROR("RK_MPI_RGN_GetCanvasInfo failed with %#x!", ret);
			continue;
		}
    }
    
}


RK_S32 osd_draw_rect(RK_U8 *buffer,  RK_U32 width, RK_U32 height, int x, int y, int w, int h, int line_pixel)
{
    RK_U8 *ptr = buffer;
}
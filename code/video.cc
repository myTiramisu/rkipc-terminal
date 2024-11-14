#include "video.h"
Video::Video()
{
    quit_flag = false;

    // vi init	 	    初始化视频输入设备和通道
    rkaiq_init();
    rkmpi_sys_init();    // rkmpi_sys_init
	vi_dev_init();
    rtsp_init();

    video_thread0 = new std::thread(&Video::video_thread_0, this);      // rtsp
    // video_thread1 = new std::thread(&Video::video_thread_1, this);   // lcd
    video_thread2 = new std::thread(&Video::video_thread_2, this);
}

Video::~Video()
{
    {
        std::lock_guard<std::mutex> lock(mutex_video);
        quit_flag = true;
    }
    printf("******************************start release video\n");
    if (video_thread0->joinable()) {
        video_thread0->join();
    }
    if (video_thread1->joinable()) {
        video_thread1->join();
    }
    if (video_thread2->joinable()) {
        video_thread2->join();
    }

    // 构造函数的反初始化
    rtsp_release();
    vi_dev_deinit();
    rkmpi_sys_deinit();
    rkaiq_deinit();
    printf("******************************Release video success\n");
}

// rtsp video
void Video::video_thread_0()
{
    std::cout << "************************video_thread_0 started success" << std::endl;
    
    int video_width  = 2304;
    int video_height = 1296;
    int pipeId = 0;            // pipeId
    int viChannelId = 0;       // VI channel id
    int vencChannelId = 0;     // VENC channel id

    // 获取编码后的帧   
    VENC_STREAM_S stFrame;      
    stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S));

    // 初始化视频输入通道 
    vi_chn_init(viChannelId, video_width, video_height);
    // 初始化视频编码器 设置编码类型为 H264		
	venc_init(pipeId, video_width, video_height, RK_VIDEO_ID_AVC, RK_FMT_YUV420SP);


    // 绑定输入通道到编码器
    MPP_CHN_S vi_chn;       // 视频输入通道
    MPP_CHN_S venc_chn;     // 编码器通道
    bind_vi_to_venc(pipeId, &vi_chn, &venc_chn);

    while (!quit_flag)
    {
        // 获取编码后的帧，发送到 RTSP 服务器
        rtsp_send_frame_h264(vencChannelId, &stFrame);

        // 释放编码后的帧
        venc_release_frame(vencChannelId, &stFrame);
    }
    std::cout << "******************************video_thread_0 exit" << std::endl;
    sleep(1);
    unbind_vi_to_venc(pipeId, &vi_chn, &venc_chn);
    venc_deinit(vencChannelId);
    vi_chn_deinit(pipeId, viChannelId);
    free(stFrame.pstPack);
    std::cout << "******************************video_thread_0 release success" << std::endl;
} 


// lcd display
void Video::video_thread_1()
{
    std::cout << "******************************video_thread_1 started" << std::endl;
    int pipeId = 0;                 // 管道ID
    int viChannelId = 1;            // 视频输入通道ID
    int vencChannelId = 1;          // 视频编码器通道ID
    int video_width = 720;          // 视频的宽度
    int video_height = 480;         // 视频的高度

    VENC_STREAM_S stFrame;          // 编码后的帧  
    VIDEO_FRAME_INFO_S stViFrame;   // 视频帧信息
    stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S));

    // 定义内存块和内存池
    MB_BLK src_blk;                
    MB_POOL src_Pool;
    // 创建内存块池。
    create_MB_pool(&src_blk, &src_Pool, video_width, video_height);

    // Build venc_frame
    VIDEO_FRAME_INFO_S venc_frame;
    venc_frame.stVFrame.u32Width = video_width;
    venc_frame.stVFrame.u32Height = video_height;
    venc_frame.stVFrame.u32VirWidth = video_width;
    venc_frame.stVFrame.u32VirHeight = video_height;
    venc_frame.stVFrame.enPixelFormat = RK_FMT_RGB888;
    venc_frame.stVFrame.u32FrameFlag = 160;
    venc_frame.stVFrame.pMbBlk = src_blk;

    // 获取内存块的虚拟地址
    unsigned char *venc_data = (unsigned char *)RK_MPI_MB_Handle2VirAddr(src_blk);
    cv::Mat frame(cv::Size(video_width, video_height), CV_8UC3, venc_data);
    cv::Mat yuv420sp(video_height + video_height / 2, video_width, CV_8UC1);
    cv::Mat bgr(video_height, video_width, CV_8UC3);

    // 初始化视频输入通道
    vi_chn_init(viChannelId, video_width, video_height);
    // 初始化视频编码器
    venc_init(vencChannelId, video_width, video_height, RK_VIDEO_ID_AVC, RK_FMT_RGB888);

    while (!quit_flag)
    {
        // 从视频输入通道获取一帧数据
        yuv420sp.data = (unsigned char *)vi_get_frame(pipeId, viChannelId, video_width, video_height, &stViFrame);

        bgr.data = venc_data;
        cv::cvtColor(yuv420sp, bgr, cv::COLOR_YUV420sp2BGR);
        cv::resize(bgr, frame, cv::Size(video_width, video_height), 0, 0, cv::INTER_LINEAR);
        memcpy(venc_data, frame.data, video_width * video_height * 3);
        
        // 调整图像大小	frame.cols:128 		frame.rows:160
    	cv::Mat dst;
    	cv::resize(frame, dst, cv::Size(160, 128));
        // 向dipPlay类发送数据
        // video_frame_signal.emit(dst);

        // 编码
        venc_encode_frame(vencChannelId, &venc_frame);
        // 获取编码后的帧，发送到 RTSP 服务器
        rtsp_send_frame_h264(vencChannelId, &stFrame);
        vi_release_frame(pipeId, viChannelId, &stViFrame);

        // 释放视频帧
        venc_release_frame(vencChannelId, &stFrame);
    }
    std::cout << "******************************video_thread_1 exit" << std::endl;
     sleep(1);
    venc_deinit(vencChannelId);
    vi_chn_deinit(pipeId, viChannelId);
    free(stFrame.pstPack);
    destroy_MB_pool(&src_blk, &src_Pool);
}

void Video::video_thread_2()
{
    std::cout << "************************video_thread_2 started success" << std::endl;
    // video parameters
    int pipeId = 0;                  // 管道ID
    int viChannelId = 2;            // 视频输入通道ID
    int vencChannelId = 2;          // 视频编码器通道ID
    // model size
    int video_width = 640;
    int video_height = 640;

    VIDEO_FRAME_INFO_S stViFrame;   // 视频帧信息
    VENC_STREAM_S stFrame;          // 编码后的帧  
    stFrame.pstPack = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S));


    // 定义内存块和内存池
    MB_BLK src_blk;                
    MB_POOL src_Pool;
    // 创建内存块池。
    create_MB_pool(&src_blk, &src_Pool, video_width, video_height);

    // Build venc_frame
    VIDEO_FRAME_INFO_S venc_frame;
    venc_frame.stVFrame.u32Width = video_width;
    venc_frame.stVFrame.u32Height = video_height;
    venc_frame.stVFrame.u32VirWidth = video_width;
    venc_frame.stVFrame.u32VirHeight = video_height;
    venc_frame.stVFrame.enPixelFormat = RK_FMT_RGB888;
    venc_frame.stVFrame.u32FrameFlag = 160;
    venc_frame.stVFrame.pMbBlk = src_blk;

    // YUV420SP：主要用于视频压缩和硬件加速，数据量较小，适合传输和存储。
    // BGR：主要用于图像处理和显示，数据量较大，适合算法实现和可视化。
    // 存储 YUV420SP 格式的图像数据
    // 获取内存块的虚拟地址
    unsigned char *venc_data = (unsigned char *)RK_MPI_MB_Handle2VirAddr(src_blk);
    cv::Mat frame(cv::Size(video_width, video_height), CV_8UC3, venc_data);
    cv::Mat yuv420sp(video_height + video_height / 2, video_width, CV_8UC1);
    cv::Mat bgr(video_height, video_width, CV_8UC3);

    // 初始化视频输入通道
    vi_chn_init(viChannelId, video_width, video_height);

    // Rknn model
    int sX, sY, eX, eY;
	char text[16];
	rknn_app_context_t rknn_app_ctx;	
	object_detect_result_list od_results;
	const char *model_path = "./model/yolov5.rknn";
    memset(&rknn_app_ctx, 0, sizeof(rknn_app_context_t));	
	init_yolov5_model(model_path, &rknn_app_ctx);
	init_post_process();

    // rtsp
    // 初始化视频编码器
    venc_init(vencChannelId, video_width, video_height, RK_VIDEO_ID_AVC, RK_FMT_RGB888);

    while (!quit_flag)
    {
        yuv420sp.data = (unsigned char *)vi_get_frame(pipeId, viChannelId, video_width, video_height, &stViFrame);
        bgr.data = venc_data;
        cv::cvtColor(yuv420sp, bgr, cv::COLOR_YUV420sp2BGR);        // yiuv420转bgr
        // 修改成与模型一样的大小
        cv::resize(bgr, bgr, cv::Size(video_width, video_height), 0, 0, cv::INTER_LINEAR);
        cv::resize(bgr, frame, cv::Size(video_width, video_height), 0, 0, cv::INTER_LINEAR);
        memcpy(venc_data, frame.data, video_width * video_height * 3);
        
        //letterbox
		cv::Mat letterboxImage = letterbox(bgr, video_width, video_height);	
        memcpy(rknn_app_ctx.input_mems[0]->virt_addr, letterboxImage.data, MODEL_HEIGHT * MODEL_HEIGHT*3);		
		inference_yolov5_model(&rknn_app_ctx, &od_results);

        for (int i = 0; i < od_results.count; i++)
        {
            //获取框的四个坐标 
            if(od_results.count >= 1)
            {
                object_detect_result *det_result = &(od_results.results[i]);

                printf("%s @ (%d %d %d %d) %.3f\n", coco_cls_to_name(det_result->cls_id),
                        det_result->box.left, det_result->box.top,
                        det_result->box.right, det_result->box.bottom,
                        det_result->prop);

                sX = (int)(det_result->box.left   );	
                sY = (int)(det_result->box.top 	  );	
                eX = (int)(det_result->box.right  );	
                eY = (int)(det_result->box.bottom );
                mapCoordinates(&sX,&sY);
                mapCoordinates(&eX,&eY);

                cv::rectangle(bgr,cv::Point(sX ,sY),
                                    cv::Point(eX ,eY),
                                    cv::Scalar(0,255,0),3);
                sprintf(text, "%s %.1f%%", coco_cls_to_name(det_result->cls_id), det_result->prop * 100);
                cv::putText(bgr,text,cv::Point(sX, sY - 8),
                                            cv::FONT_HERSHEY_SIMPLEX,1,
                                            cv::Scalar(0,255,0),2);
            }
        }
        // 调整图像大小	frame.cols:128 		frame.rows:160
    	cv::Mat dst;
    	cv::resize(frame, dst, cv::Size(160, 128));
        // 向dipPlay类发送数据
        video_frame_signal.emit(dst);

        // 编码
        venc_encode_frame(vencChannelId, &venc_frame);
        // 获取编码后的帧，发送到 RTSP 服务器
        rtsp_send_frame_h264(vencChannelId, &stFrame);
        // 释放编码后的帧
        venc_release_frame(vencChannelId, &stFrame);

         // 释放视频帧
        vi_release_frame(pipeId, viChannelId, &stViFrame);
    }
    std::cout << "******************************video_thread_2 exit" << std::endl;
    sleep(1);
    venc_deinit(vencChannelId);
    vi_chn_deinit(pipeId, viChannelId);
    release_yolov5_model(&rknn_app_ctx);
    deinit_post_process();
    destroy_MB_pool(&src_blk, &src_Pool);
    free(stFrame.pstPack);
    std::cout << "******************************video_thread_2 release success" << std::endl;
}
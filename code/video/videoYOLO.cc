#include "videoYOLO.h"

VideoYOLO::VideoYOLO()
{
    // 初始化特定于 YOLO 的资源
    start_thread();
}

VideoYOLO::~VideoYOLO()
{
    stop_thread();
}



void VideoYOLO::video_thread_func()
{
    LOG_DEBUG("************************video_thread_2 started success\n");
    int pipeId = 0;
    int viChannelId = 2;
    int vencChannelId = 2;
    int video_width = 640;
    int video_height = 640;
    int rgn_video_width = 2304;
    int rgn_video_height = 1296;

    VIDEO_FRAME_INFO_S stViFrame;
    cv::Mat yuv420sp(video_height + video_height / 2, video_width, CV_8UC1);
    cv::Mat bgr(video_height, video_width, CV_8UC3);

    vi_chn_init(viChannelId, video_width, video_height);

    int sX, sY, eX, eY;
    char text[16];
    rknn_app_context_t rknn_app_ctx;
    object_detect_result_list od_results;
    const char *model_path = "./model/yolov5.rknn";
    memset(&rknn_app_ctx, 0, sizeof(rknn_app_context_t));
    init_yolov5_model(model_path, &rknn_app_ctx);
    init_post_process();

    RGN_HANDLE coverHandle = 0;
    std::vector<DrawTaskParams> params;
    OsdDraw osdDraw;

    while (!quit_flag)
    {
        yuv420sp.data = (unsigned char *)vi_get_frame(pipeId, viChannelId, video_width, video_height, &stViFrame);
        cv::cvtColor(yuv420sp, bgr, cv::COLOR_YUV420sp2BGR);
        cv::resize(bgr, bgr, cv::Size(video_width, video_height), 0, 0, cv::INTER_LINEAR);

        cv::Mat letterboxImage = letterbox(bgr, video_width, video_height);
        memcpy(rknn_app_ctx.input_mems[0]->virt_addr, letterboxImage.data, MODEL_HEIGHT * MODEL_HEIGHT * 3);
        inference_yolov5_model(&rknn_app_ctx, &od_results);

        for (int i = 0; i < od_results.count; i++)
        {
            if(od_results.count >= 1)
            {
                object_detect_result *det_result = &(od_results.results[i]);

                sX = (int)(det_result->box.left);
                sY = (int)(det_result->box.top);
                eX = (int)(det_result->box.right);
                eY = (int)(det_result->box.bottom);
                mapCoordinates(&sX, &sY);
                mapCoordinates(&eX, &eY);
                sX = (int)((float)sX / (float)video_width * rgn_video_width);
                sY = (int)((float)sY / (float)video_height * rgn_video_height);
                eX = (int)((float)eX / (float)video_width * rgn_video_width);
                eY = (int)((float)eY / (float)video_height * rgn_video_height);

                DrawTaskParams task;
                task.coverHandle = coverHandle;
                task.x = sX;
                task.y = sY;
                task.w = eX - sX;
                task.h = eY - sY;
                params.push_back(task);
            }
        }
        
        if (params.size() > 0)
            osdDraw.osd_rgn_add_tasks(params);
        params.clear();

        vi_release_frame(pipeId, viChannelId, &stViFrame);
    }

    vi_chn_deinit(pipeId, viChannelId);
    release_yolov5_model(&rknn_app_ctx);
    deinit_post_process();
    LOG_DEBUG("******************************video_thread_2 release success\n");
}
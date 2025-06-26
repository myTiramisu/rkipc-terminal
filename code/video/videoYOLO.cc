#include "videoYOLO.h"
#include <yolov5.h>
// osd
#include "luckfox_osd.h"
#include "luckfox_osd_draw.h"
#include <unordered_set>

VideoYOLO::VideoYOLO(ModuleParams params):VideoBase(params)
{
    pipeId = 0;
    viChannelId = 2;
    video_width = 640;
    video_height = 640;

    rgn_video_width = 2304;
    rgn_video_height = 1296;

    quit_flag = false;
    video_thread = new std::thread(&VideoYOLO::video_thread_func, this);
}

VideoYOLO::~VideoYOLO()
{
    quit_flag = true;
    if(video_thread->joinable())
        video_thread->join();
    delete video_thread;
    LOG_DEBUG("******************************VideoLCD exit\n");
}


void VideoYOLO::notify(std::string msg)
{

}
void VideoYOLO::update(Publisher* publisher, string msg)
{

}

void VideoYOLO::videoCapture() 
{
    vi_get_frame(pipeId, viChannelId, video_width, video_height, &stViFrame);
}

void VideoYOLO::videoEncode() 
{
    // 无需编码器
    LOG_DEBUG("******************************VideoYOLO encode free\n");
}

void VideoYOLO::videoRtspTransmit() 
 {
    // 无需传输
    LOG_DEBUG("******************************VideoYOLO rtsp transmit free\n");
 }

void VideoYOLO::video_thread_func()
{
    LOG_DEBUG("************************video_thread_2 started success\n");


    int line_pixel = 2;

    
    int people_detect = 1;
    int vehicle_detect = 1;
    int pet_detect = 1;
    
    // detect classes set
    std::unordered_set<int> detect_classes;
    if (people_detect) {
        detect_classes.insert(0);
    }
    if (vehicle_detect) {
        detect_classes.insert(1);
        detect_classes.insert(2);
        detect_classes.insert(3);
        detect_classes.insert(4);
        detect_classes.insert(5);
        detect_classes.insert(7);
        detect_classes.insert(8);
    } 
    if (pet_detect) {
        detect_classes.insert(15);
        detect_classes.insert(16);
    }

    int ai_follow_enable = 1;
    int ai_follow_people = 1;
    int ai_follow_vehicle = 0;
    int ai_follow_pet = 0;
    int ai_follow_tolerance_width = 100;
    int ai_follow_tolerance_height = 100;
    int ai_follow_roi_x = 50;
    int ai_follow_roi_y = 50;
    int ai_follow_roi_width = 2204;
    int ai_follow_roi_height = 1196;
    int rgn_square_size = rgn_video_width * rgn_video_height;

    // follow classes set
    std::unordered_set<int> follow_classes;
    if (ai_follow_enable) {
        // 只能选择跟随一个目标，从前往后优先级递减
        if (ai_follow_people) {
            follow_classes.insert(0);
        } else if (ai_follow_vehicle) {
            follow_classes.insert(1);
            follow_classes.insert(2);
            follow_classes.insert(3);
            follow_classes.insert(4);
            follow_classes.insert(5);
            follow_classes.insert(7);
            follow_classes.insert(8);
        } else if (ai_follow_pet) {
            follow_classes.insert(15);
            follow_classes.insert(16);
        }
    }

    // follow target info
    bool is_follow_target_detected;
    int follow_sX, follow_sY, follow_eX, follow_eY;
    int follow_sX_last, follow_sY_last, follow_eX_last, follow_eY_last;
    float follow_target_prop;



    // osd
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



        // init follow target info
        if (ai_follow_enable && follow_classes.size() > 0)
        {
            is_follow_target_detected = false;
            follow_sX = 0;
            follow_sY = 0;
            follow_eX = 0;
            follow_eY = 0;
            follow_sX_last = 0;
            follow_sY_last = 0;
            follow_eX_last = 0;
            follow_eY_last = 0;
            follow_target_prop = 0;
        }

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

                // 更新跟随目标坐标，若有多个目标则选择置信度最高的目标，
                // 若置信度相差不超过 0.1 则选择最靠近中心的目标
                if (ai_follow_enable && follow_classes.count(det_result->cls_id) > 0)
                {
                    is_follow_target_detected = true;   // 用于判断是否检测到跟随目标

                    bool update_target = false;         // 用于判断是否需要更新跟随目标

                    if (follow_sX == 0 && follow_sY == 0 && follow_eX == 0 && follow_eY == 0)
                    {
                        // 初始化跟随目标
                        update_target = true;
                    }
                    else
                    {
                        // 计算当前目标与视频中心的偏移
                        int delta_x = (sX + eX) / 2 - rgn_video_width / 2;
                        int delta_y = (sY + eY) / 2 - rgn_video_height / 2;
                        // 计算上一个目标与视频中心的偏移
                        int delta_last_x = (follow_sX_last + follow_eX_last) / 2 - rgn_video_width / 2;
                        int delta_last_y = (follow_sY_last + follow_eY_last) / 2 - rgn_video_height / 2;

                        // 计算当前目标的面积
                        int target_area = (eX - sX) * (eY - sY);
                        // 计算上一个目标的面积
                        int last_target_area = (follow_eX_last - follow_sX_last) * (follow_eY_last - follow_sY_last);

                        // 权衡置信度和面积，计算得分
                        float square_coefficient = 0.6;
                        float score = det_result->prop + square_coefficient * (float)target_area / (float)rgn_square_size;
                        float last_score = follow_target_prop + square_coefficient * (float)last_target_area/ (float)rgn_square_size;

                        // LOG_DEBUG("target_prop: %.3f, target_area: %d, score: %.3f\n", det_result->prop, target_area, score);
                        // LOG_DEBUG("last_target_prop: %.3f, last_target_area: %d, last_score: %.3f\n", follow_target_prop, last_target_area, last_score);

                        // 如果当前目标的得分更高，选择当前目标
                        if (score > last_score)
                        {
                            update_target = true;
                        }
                    }

                    // 如果决定更新目标，则更新目标的坐标和置信度
                    if (update_target)
                    {
                        follow_sX = sX;
                        follow_sY = sY;
                        follow_eX = eX;
                        follow_eY = eY;

                        // 保存当前目标的坐标和置信度
                        follow_sX_last = sX;
                        follow_eX_last = eX;
                        follow_sY_last = sY;
                        follow_eY_last = eY;
                        follow_target_prop = det_result->prop;
                    }
                }
            }
        }
        

        if (params.size() > 0)
            osdDraw.osd_rgn_add_tasks(params);
        params.clear();
        vi_release_frame(pipeId, viChannelId, &stViFrame);


        if (ai_follow_enable && is_follow_target_detected)
        {
            // 计算目标中心位置与图像中心的偏移量
            int cx = (follow_sX + follow_eX) / 2;
            int cy = (follow_sY + follow_eY) / 2;

            // 若不在感兴趣区域内则不跟随
            if (cx < ai_follow_roi_x || cx > ai_follow_roi_x + ai_follow_roi_width ||
                cy < ai_follow_roi_y || cy > ai_follow_roi_y + ai_follow_roi_height)
            {
                continue;
            }

            int delta_x = cx - rgn_video_width / 2;
            int delta_y = cy - rgn_video_height / 2;

            // 设置容忍范围，若偏移量在容忍范围内则不调整云台
            if (abs(delta_x) < ai_follow_tolerance_width && abs(delta_y) < ai_follow_tolerance_height)
            {
                continue;
            }

            // LOG_DEBUG("follow target: (%d, %d, %d, %d)\n", follow_sX, follow_sY, follow_eX, follow_eY);
            // LOG_DEBUG("cx: %d, cy: %d\n", cx, cy);
            // LOG_DEBUG("delta_x: %d, delta_y: %d\n", delta_x, delta_y);

            // 根据偏移量计算云台需要调整的角度，基于画幅 左右 90°，俯仰 45°，加上系数微调
            float pan_coefficient = 0.3;
            float tilt_coefficient = 0.3;
            int delta_pan = (int)(pan_coefficient * (float)delta_x / (float)rgn_video_width * 90);
            int delta_tilt = (int)(tilt_coefficient * (float)delta_y / (float)rgn_video_height * 45);

            // LOG_DEBUG("delta_pan: %d, delta_tilt: %d\n", delta_pan, delta_tilt);

            // 发射信号来调整云台位置
            // signal_adjust_pantilt.emit(delta_pan, delta_tilt);  // 传递偏移量给舵机控制类
        }
    }
    vi_chn_deinit(pipeId, viChannelId);
    release_yolov5_model(&rknn_app_ctx);
    deinit_post_process();
    LOG_DEBUG("******************************video_thread_2 release success\n");
}
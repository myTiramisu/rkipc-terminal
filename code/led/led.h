// Led.h
#pragma once

#include <mutex>
#include "module.h"
#include "signal_slot.h"
#include "gpio.h"
#include "observer.h"

// LED Resources
enum Led_num {
    LED0 = GPIO2_A0_d,
    LED1 = GPIO2_A1_d,
    LED2 = GPIO2_A2_d,
};

class Led : public Module
{
public:
    Led(enum Led_num led_num);

    ~Led();

    void on();

    void off();

    void toggle();

    void blink(int duration_s);

    void set_mode(const char* mode);

    void set_on_time(int on_time);

    void set_off_time(int off_time);

    void set_blink_frequency(int blink_frequency);

    void update(const std::string& message, const std::string& mode) override;
private:
    enum Led_num led_num_;  // LED 对应的 GPIO 号
    
    // 用户配置参数
    const char* mode_;      
    int on_time_;          
    int off_time_;         
    int blink_frequency_;

    // 互斥锁保护参数
    std::mutex led_mutex_;
};

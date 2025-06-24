// Led.h
#pragma once

#include <mutex>
#include "signal_slot.h"
#include "gpio.h"
#include "abstractModule.h"

// // LED Resources
// enum Led_num {
//     LED0 = GPIO2_A0_d,
//     LED1 = GPIO2_A1_d,
//     LED2 = GPIO2_A2_d,
// };

#define LED0 GPIO2_A0_d
#define LED1 GPIO2_A1_d
#define LED2 GPIO2_A2_d

class Led : public BaseModule
{
public:
    Led(ModuleParams params);

    ~Led();

    void on();

    void off();

    void toggle();

    void blink(int duration_s);

    void set_mode(const char* mode);

    void set_on_time(int on_time);

    void set_off_time(int off_time);

    void set_blink_frequency(int blink_frequency);

private:
    enum Gpio_num m_led_num;
    // 用户配置参数
    const char* m_mode;      
    int m_on_time;          
    int m_off_time;         
    int m_blink_frequency;

    void update(Publisher *publisher, string msg) override;
    void notify(string msg) override;

    // 互斥锁保护参数
    std::mutex m_led_mutex;
};

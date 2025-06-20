#include "led.h"
#include "log.h"
#include "param.h"
#include "thread"

/**
 * @brief LED 类构造函数。
 * 
 * @param led_num LED 编号。
 */
Led::Led(Publisher* publisher, const std::string& name, enum Gpio_num led_num)
    : Observer(publisher, name), 
    m_led_num(led_num), m_on_time(0), m_off_time(0), m_blink_frequency(1), m_mode("manual")
{
    // 初始化 GPIO
    if (gpio_init((Gpio_num)m_led_num, GPIO_OUTPUT) == -1) {
        LOG_ERROR("Failed to initialize LED: ", m_led_num);
    } else {
        gpio_write((Gpio_num)m_led_num, 1);  // 初始状态关闭LED
        LOG_DEBUG("LED %d initialized\n", m_led_num);
    }

    // 读取用户配置参数
    m_mode = rk_param_get_string("led:mode", "manual");
    if (m_mode == "schedule") {
        m_on_time = rk_param_get_int("led:on_time", 0);
        m_off_time = rk_param_get_int("led:off_time", 0);
    }
    m_blink_frequency = rk_param_get_int("led:blink_frequency", 1);

    // 校验 blink_frequency_，确保在合理范围内
    if (m_blink_frequency < 1 || m_blink_frequency > 3) {
        m_blink_frequency = 1;  // 设置为默认值
    }

    // 确保 LED 初始状态为关闭
    off();  
}

void Led::update(std::string msg) 
{
    std::lock_guard<std::mutex> lock(m_led_mutex);
    // 确保消息不为空
    if(msg[0] == '\0') {
        LOG_WARN("Received empty message for LED %d\n", m_led_num);
        return;
    }
    // 确保消息格式正确，前三位为LED名称，后面跟着冒号和命令
    if (msg.length() < 4 || msg[3] != ':' || msg.substr(0, 3) != m_name) {
        return;
    }

    // 打印接收到的消息
    LOG_INFO("LED %d received message: %s\n", m_led_num, msg.c_str());
    // 判断消息是否为本灯的控制命令
    if (msg == m_name + ":on") {
        LOG_INFO("%s received ON command\n", m_name.c_str());
        on();
    } else if (msg == m_name + ":off") {
        LOG_INFO("%s received OFF command\n", m_name.c_str());
        off();
    }
    else if (msg == m_name + ":toggle") {
        LOG_INFO("%s received TOGGLE command\n", m_name.c_str());
        toggle();
    } else if (msg == m_name + ":blink") {
        LOG_INFO("%s received BLINK command\n", m_name.c_str());
        blink(5);  // 默认闪烁1秒 ,闪烁5下
    } else if (msg == m_name + ":set_mode:schedule") {
        set_mode("schedule");
        LOG_INFO("%s set to SCHEDULE mode\n", m_name.c_str());
    } else if (msg == m_name + ":set_mode:manual") {
        set_mode("manual");
        LOG_INFO("%s set to MANUAL mode\n", m_name.c_str());
    }
    else if (msg == m_name + ":set_on_time") {
        set_on_time(1000);  // 设置点亮时间为1000毫秒
        LOG_INFO("%s set ON time to %d ms\n", m_name.c_str(), m_on_time);
    } else if (msg == m_name + ":set_off_time") {
        set_off_time(500);  // 设置熄灭时间为500毫秒
        LOG_INFO("%s set OFF time to %d ms\n", m_name.c_str(), m_off_time);
    } else if (msg == m_name + ":set_blink_frequency") {
        set_blink_frequency(2);  // 设置闪烁频率为2Hz
        LOG_INFO("%s set blink frequency to %d Hz\n", m_name.c_str(), m_blink_frequency);
    }
    else {
        LOG_WARN("LED %d received unknown command: %s\n", m_led_num, msg.c_str());
    }
}

/**
 * @brief LED 类析构函数。
 */
Led::~Led() {
    off();  // 关闭 LED
    gpio_deinit((Gpio_num)m_led_num);

    LOG_DEBUG("LED %d deinitialized\n", m_led_num);
}

/**
 * @brief 打开 LED。
 */
void Led::on() {
    gpio_write((Gpio_num)m_led_num, 1);  // 设置 GPIO 为高电平（开启 LED）
}

/**
 * @brief 关闭 LED。
 */
void Led::off() {
    gpio_write((Gpio_num)m_led_num, 0);  // 设置 GPIO 为低电平（关闭 LED）
}

/**
 * @brief 切换 LED 状态。
 */
void Led::toggle() {
    gpio_toggle((Gpio_num)m_led_num);  // 反转当前 LED 状态
}

/**
 * @brief 闪烁 LED。
 * 
 * @param duration_s 闪烁持续时间（秒）。
 */
void Led::blink(int duration_s) {

    // 启动一个新线程执行闪烁
    std::thread([=]() {
        int freq = m_blink_frequency;
        if (freq < 1 || freq > 3) freq = 1;
        int dur = duration_s;
        if (dur <= 0 || dur > 5) dur = 1;

        int blink_num = dur * freq;
        while (blink_num--) {
            on();
            usleep(500000 / freq);
            off();
            usleep(500000 / freq);
        } 
    }).detach();    // 使用互斥锁保护 LED 状态
}

/**
 * @brief 设置 LED 工作模式。
 * 
 * @param mode 工作模式。
 */
void Led::set_mode(const char* mode) {
    std::lock_guard<std::mutex> lock(m_led_mutex);
    m_mode = mode;
    rk_param_set_string("led:mode", m_mode);
}

/**
 * @brief 设置点亮时间。
 * 
 * @param on_time 点亮时间（毫秒）。
 */
void Led::set_on_time(int on_time) {
    std::lock_guard<std::mutex> lock(m_led_mutex);
    m_on_time = on_time;
    rk_param_set_int("led:on_time", m_on_time);
}

/**
 * @brief 设置熄灭时间。
 * 
 * @param off_time 熄灭时间（毫秒）。
 */
void Led::set_off_time(int off_time) {
    std::lock_guard<std::mutex> lock(m_led_mutex);
    m_off_time = off_time;
    rk_param_set_int("led:off_time", m_off_time);
}

/**
 * @brief 设置闪烁频率。
 * 
 * @param blink_frequency 闪烁频率。
 */
void Led::set_blink_frequency(int blink_frequency) {
    std::lock_guard<std::mutex> lock(m_led_mutex);
    if (blink_frequency >= 1 && blink_frequency <= 3) {
        m_blink_frequency = blink_frequency;
        rk_param_set_int("led:blink_frequency", m_blink_frequency);
    } else {
        LOG_WARN("Invalid blink frequency: %d\n", blink_frequency);
        m_blink_frequency = 1;  // 设置为默认值
    }
}

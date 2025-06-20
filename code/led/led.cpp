#include "led.h"
#include "log.h"
#include "param.h"

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
}

void Led::update(std::string msg) 
{
    std::lock_guard<std::mutex> lock(m_led_mutex);
    LOG_INFO("LED %d received message: %s\n", m_led_num, msg.c_str());
    if (msg == "on") {
        on();  // 打开 LED
    } else if (msg == "off") {
        off();  // 关闭 LED
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
    std::lock_guard<std::mutex> lock(m_led_mutex);

    // 确保 blink_frequency_ 在合理范围内
    if (m_blink_frequency < 1 || m_blink_frequency > 3) {
        m_blink_frequency = 1;
    }

    // 限制 duration_s 在 1 到 5 秒之间
    if (duration_s <= 0 || duration_s > 5) {
        duration_s = 1;
    }

    int blink_num = duration_s * m_blink_frequency;
    while (blink_num--) {
        on();
        usleep(500000 / m_blink_frequency);
        off();
        usleep(500000 / m_blink_frequency);
    }
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

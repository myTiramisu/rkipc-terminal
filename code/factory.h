#include <memory>
#include <string>    
#include <stdexcept> 
#include "pantilt.h"
#include "led.h"
#include "display.h"
#include "module.h"


// 工厂类
class DeviceFactory {
public:
    static std::unique_ptr<Module> createDevice(const std::string& type) {
        if (type == "PTZ") {
            return std::make_unique<Pantilt>();
        } else if (type == "LED") {
           throw std::invalid_argument("LED type requires an additional parameter");
        } else if (type == "DISPLAY") {
            return std::make_unique<Display>();
        } else {
            throw std::invalid_argument("Unknown device type");
        }
    }

    static std::unique_ptr<Module> createLedDevice(enum Led_num led_num) {
        return std::make_unique<Led>(led_num);
    }
};
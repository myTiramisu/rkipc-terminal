#pragma once
#include "observer.h"
#include <vector>
#include <string>
#include <algorithm>

class ControlCenter
{
public:

    // 构造函数
    ControlCenter() = default;

    void addObserver(Observer* observer) {
        observers.push_back(observer);
    }

    void removeObserver(Observer* observer) {
        observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
    }

    void sendCommand(const std::string& target, const std::string& command) {
        for (auto* obs : observers) {
            obs->update(target, command);
        }
    }

private:
    std::vector<Observer*> observers;
};
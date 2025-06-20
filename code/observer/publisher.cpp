#include "publisher.h"
#include "log.h"
#include <iostream>
#include "observer.h"

Publisher::Publisher()
{

}

// 1. 添加订阅者
void Publisher::attach(Observer* observer) {
    m_observers.push_back(observer);
}

// 2. 删除订阅者
void Publisher::detach(Observer* observer) {
    // 删除
    m_observers.remove(observer);
}

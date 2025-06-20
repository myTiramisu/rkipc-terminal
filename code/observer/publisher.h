#ifndef _PUBLISHER_H
#define _PUBLISHER_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include "observer.h"

using namespace std;

class Observer;

// 发布者基类
class Publisher
{ 
public:
    Publisher();
    virtual ~Publisher() {}         // 基类，虚析构，多态时能通过基类指针析构各个子类的析构函数

    // 1. 添加订阅者
    void attach(Observer* observer);
    // 2. 删除订阅者
    void detach(Observer* observer);
    // 3. 通知订阅者 
    virtual void notify(string msg) = 0;    // 不同子类发送消息不一样，定义为存虚，要求必须重写notify函数

protected:
    // 订阅者列表 添加删除效率高
    list<Observer*> m_observers;
};


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




// 发布者子类1
class Publisher1 : public Publisher
{
public:
    Publisher1();
    ~Publisher1() {}

    void notify(string msg) override;
};

// 发布者子类2
class Publisher2 : public Publisher
{
public:
    Publisher2();
    ~Publisher2() {}

    void notify(string msg) override;
};




Publisher1::Publisher1()
{

}

Publisher2::Publisher2()
{
    
}

void Publisher1::notify(string msg)
{
    cout << "Publisher1 observer num:" << m_observers.size() <<endl;

    for(const auto &observer : m_observers)
    {
        // 观察者更新数据
        observer->update(msg);
    }
}


void Publisher2::notify(string msg)
{
    cout << "Publisher2 observer num:" << m_observers.size() <<endl;

    for(const auto &observer : m_observers)
    {
        // 观察者更新数据
        observer->update(msg);
    }
}

#endif
#ifndef __OBSERVER_H__
#define __OBERVER_H__

#include "publisher.h"
#include <string>
#include "log.h"



// 抽象观察者类
class Observer {
public:
    // 和发布者进行关联 - 通过构造函数实现 - 初始化列表
    Observer(Publisher* publisher, std::string name) : m_publisher(publisher), m_name(name) {
        m_publisher->attach(this);
    }


    // 和发布者进行解除关联 - 取消订阅
    void unsubscribe() {
        m_publisher->detach(this);
    }
    // 更新消息
    virtual void update(std::string msg) = 0;

    virtual ~Observer() {
        std::cout << "Observer " << m_name << " is destroyed" << std::endl;
    }

protected:
    Publisher* m_publisher;     // 订阅的的发布者
    string m_name;              // 当前观察者的名字
};


// class Observer1 : public Observer {
// public:
//     Observer1(Publisher* publisher, string name) : Observer(publisher, name) {
//         cout << "Observer1 " << name << " is created" << endl;
//     }

//     void update(string msg) override {
//         cout << "Observer1 " << m_name << " received msg: " << msg << endl;
//     }

//     ~Observer1() {
//         cout << "Observer1 " << m_name << " is destroyed";
//     }
// };

// class Observer2 : public Observer {
// public:
//     Observer2(Publisher* publisher, string name) : Observer(publisher, name) {
//         cout << "Observer2 " << name << " is created" << endl;
//     }

//     void update(string msg) override {
//         cout << "Observer2 " << m_name << " received msg: " << msg << endl;
//     }

//     ~Observer2() {
//         cout << "Observer2 " << m_name << " is destroyed";
//     }
// };

// class Observer3 : public Observer {
// public:
//     Observer3(Publisher* publisher, string name) : Observer(publisher, name) {
//         cout << "Observer3 " << name << " is created" << endl;
//     }

//     void update(string msg) override {
//         cout << "Observer3 " << m_name << " received msg: " << msg << endl;
//     }

//     ~Observer3() {
//         cout << "Observer3 " << m_name << " is destroyed";
//     }
// };

#endif
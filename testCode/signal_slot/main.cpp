#include <iostream>
#include "signal_slot.h"

class Sender {
public:
    Signal<int> signal;

    void sendData(int data) {
        std::cout << "Sender: Sending data " << data << std::endl;
        signal.emit(data);
    }
};

class Receiver {
public:
    void receiveData(int data) {
        std::cout << "Receiver: Received data " << data << std::endl;
    }
};

int main() {
    Sender sender;
    Receiver receiver;

    // 连接信号与槽
    sender.signal.connect(&receiver, &Receiver::receiveData);
    // sender.signal.connect(std::bind(&Receiver::receiveData, &receiver, std::placeholders::_1));

    // 发送数据
    sender.sendData(42);

    return 0;
}
#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include <string.h>
#include "signal_slot.h"

class TcpServer {
public:
    TcpServer(int port);
    ~TcpServer();

    void stop();

    void send_msg(const std::string& msg);

    // 发送服务器端接收到的消息
    Signal<std::string> server_signal;

private:
    int serverSocket;
    int clientSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    bool quit_flag;
    std::mutex mutex;


    std::thread* server_thread;
    std::vector<std::thread> client_threads;

    std::thread* timer_thread;      // 定时器线程
    std::atomic<bool> timer_running; // 定时器运行标志
    void timer_func(); // 定时器线程函数

    void server_thread_func();
    void client_handler(int clientSocket);
};

#endif
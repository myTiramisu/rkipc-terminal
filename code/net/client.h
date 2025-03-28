#ifndef _CLIENT_H
#define _CLIENT_H

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstring>
#include <cstdlib>
#include <queue>
#include <unistd.h>
#include <arpa/inet.h>

class Client {
public:
    // Constructor
    Client(const char* server_ip, int server_port);
    ~Client();

    bool connect_to_server();         // Connect to server
    // Send data to server
    void send_data(const char* message);    // 发送数据函数

private:
    void receive_data_thread();        // Receive data from server
    void send_data_thread();            // Send data to server

    const char* server_ip;             // Server IP address
    int server_port;                   // Server port
    int sockfd;                          // Socket descriptor
    struct sockaddr_in server_addr;    // Server address
    bool running_flag;

    std::thread receive_thread_;
    std::thread send_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::string> send_queue_;
};

#endif 
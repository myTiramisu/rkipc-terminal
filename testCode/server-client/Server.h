#ifndef _SERVER_H
#define _SERVER_H

#include <iostream>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <mutex>
#include <condition_variable>

class TcpServer {
public:
    TcpServer(int port);
    ~TcpServer();

    void start();

private:
    void acceptConnections();
    void handleClient(int clientSocket);

    int serverSocket;
    int port;
    bool running;
    std::thread acceptThread;
    std::vector<std::thread> clientThreads;

    std::mutex server_mutex;
    std::condition_variable server_cond_var;
};

#endif // TCP_SERVER_H
#include "server.h"
#include "log.h"

TcpServer::TcpServer(int port) {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        LOG_ERROR("Error opening socket\n");
        exit(1);
    }

    int optval = 1;
    socklen_t optlen = sizeof(optval);
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, optlen);

    memset((char *)&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        LOG_ERROR("Error on binding\n");
        exit(1);
    }

    quit_flag = false;
    server_thread = new std::thread(&TcpServer::server_thread_func, this);
    // 启动定时器
    timer_running = true;
    timer_thread = new std::thread(&TcpServer::timer_func, this);
}
TcpServer::~TcpServer() {
    {
        std::lock_guard<std::mutex> lock(mutex);
        quit_flag = true;
        timer_running = false; // 停止定时器
    }

    close(serverSocket);

    for (auto& thread : client_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    LOG_DEBUG("Server stopped\n");
}
void TcpServer::timer_func() 
{
    while (timer_running) 
    {
        // 每隔 2 秒
        std::this_thread::sleep_for(std::chrono::seconds(2)); 

        // 向所有客户端发送数据
        std::lock_guard<std::mutex> lock(mutex);
        for (auto& thread : client_threads) {
            if (thread.joinable()) {
                const char* message = "Server heartbeat\n";
                write(clientSocket, message, strlen(message));
            }
        }
    }
}
// 服务器等待客户端连接线程
void TcpServer::server_thread_func()
{
    listen(serverSocket, 10);
    LOG_DEBUG("Server started\n");

    socklen_t clientLen = sizeof(clientAddr);

    while (!quit_flag) {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            LOG_ERROR("Error on accept\n");
            continue;
        }

        LOG_DEBUG("Client connected\n");
        client_threads.emplace_back(&TcpServer::client_handler, this, clientSocket);
    }
}

// 客户端处理线程
void TcpServer::client_handler(int clientSocket) {
    char buffer[256];
    while (!quit_flag) 
    {
        memset(buffer, 0, 256);
        int n = read(clientSocket, buffer, 255);
        if (n < 0) {
            LOG_INFO("Client disconnected\n");
            break;
        } else if (n == 0) {
            LOG_DEBUG("Client disconnected\n");
            break;
        }

        LOG_INFO("Received message from client: %s\n", buffer);
        // 发送服务器接收到客户端的信号
        server_signal.emit(buffer);
    }
    close(clientSocket);
}

void TcpServer::send_msg(const std::string& msg)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto& thread : client_threads) {
            if (thread.joinable()) {
                write(clientSocket, msg.c_str(), msg.length());
            }
        }
    }
}
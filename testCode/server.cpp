#include "server.h"
#include <signal.h>
#include <atomic>
#include <mutex>

TcpServer::TcpServer(int port) 
{
    this->port = port;    
    this->running = false;
}
TcpServer::~TcpServer() {
    stop();
}

void TcpServer::start() {
    if (running) 
        return;

    // 创建套接字
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }

    // 设置套接字属性 允许地址重用
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, optlen);


//   // 设置 SO_REUSEADDR 选项
//     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
//         perror("setsockopt");
//         close(server_fd);
//         exit(EXIT_FAILURE);
//     }


    // 绑定套接字
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(serverSocket);
        return;
    }

    // 监听连接
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(serverSocket);
        return;
    }

    running = true;
    acceptThread = std::thread(&TcpServer::acceptConnections, this);
}

void TcpServer::stop() {
    // 设置服务器退出标志，并唤醒所有等待的线程
    {
        std::lock_guard<std::mutex> lock(server_mutex);
        running = false;
    }
    server_cond_var.notify_all();     

    close(serverSocket);

    if (acceptThread.joinable()) {
        acceptThread.join();
    }

    for (auto& thread : clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    clientThreads.clear();
    std::cout << "********Server stop" << std::endl;
}

// 一个单独的线程中运行，负责接受新的客户端连接。
void TcpServer::acceptConnections() {
    while (running) 
    {
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        // 接收客户端连接
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);

        if (clientSocket < 0) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }
        std::cout << "New client connected: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
        // 创建新线程来处理客户端
        clientThreads.emplace_back(&TcpServer::handleClient, this, clientSocket);
    }
}
// 在一个单独的线程中运行，处理每个客户端的读写操作。
void TcpServer::handleClient(int clientSocket) {
    char buffer[1024];
    ssize_t bytesRead;

    while (running) {
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            break;
        }

        std::cout << "Received from client: " << std::string(buffer, bytesRead) << std::endl;

        // 发送响应
        send(clientSocket, buffer, bytesRead, 0);
    }
    close(clientSocket);
}

bool quit = false;
std::mutex main_mutex;
static void sigterm_handler(int sig) {
    // 加锁
    std::lock_guard<std::mutex> lock(main_mutex);
    {
        fprintf(stderr, "signal %d\n", sig);
        std::cout << "**************quit success sig:" << sig << std::endl;
        quit = true;
    }
}
int main() 
{
    TcpServer server(8088);
    server.start();
	
    // Ctrl-c quit
	signal(SIGINT, sigterm_handler);

    while (!quit)
    {
        sleep(1);
    }
    server.stop();
    return 0;
}
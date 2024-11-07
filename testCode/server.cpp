#include "server.h"
#include <signal.h>
#include <atomic>
#include <mutex>
#include <poll.h>

TcpServer::TcpServer(int port) 
{
    this->port = port;    
    this->running = false;
}
TcpServer::~TcpServer() {
    // std::cout << "********Server destructor" << std::endl;
    // 设置服务器退出标志，并唤醒所有等待的线程
    {
        std::lock_guard<std::mutex> lock(server_mutex);
        running = false;
    }
    // 唤醒所有等待的线程
    server_cond_var.notify_all();   

    if (acceptThread.joinable()) {
        acceptThread.join();
    }
    for (auto& thread : clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    clientThreads.clear();
    close(serverSocket);
    std::cout << "********Server close" << std::endl;
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

    // 绑定套接字
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    //设置套接字属性 允许地址重用
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, optlen);

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
    std::cout << "Server started success port: " << port << std::endl;
    acceptThread = std::thread(&TcpServer::acceptConnections, this);
}


// 一个单独的线程中运行，负责接受新的客户端连接。

void TcpServer::acceptConnections() 
{
    // 创建IO集合 fds[0]表示服务器套接字
    struct pollfd fds[1];  
	// 服务器套接字
	fds[0].fd = serverSocket;
	fds[0].events = POLLIN;		//读事件

    while(running) 
    {
        //3. 调用poll函数
        int poll_ret = poll(fds, 1, 1000);   //1000ms溢出
        if(poll_ret < 0) {
            perror("poll error\n");
            exit(-1);
        }
        else if(poll_ret > 0 ) 
        {
            // 检查是否有新的连接请求
            if (fds[0].revents & POLLIN)
            {
                std::cout << "New client connected" << std::endl;
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
        // 检查 running 标志位
        if (!running) {
            break;
        }
    }
    std::cout << "********Server acceptConnections exit" << std::endl;
}



// 在一个单独的线程中运行，处理每个客户端的读写操作。
void TcpServer::handleClient(int clientSocket) {
    char buffer[1024];
    ssize_t bytesRead;
    // 创建IO集合，fds[0]表示客户端套接字
    struct pollfd fds[1];  
    fds[0].fd = clientSocket;
    fds[0].events = POLLIN;  // 读事件

    while (running) {
        // 调用poll函数，设置超时时间为1000ms
        int poll_ret = poll(fds, 1, 1000);  // 1000ms超时
        if (poll_ret < 0) {
            perror("poll error");
            break;
        } 
        else if (poll_ret > 0) {
             // 检查是否有数据可读
            if (fds[0].revents & POLLIN) {
                bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesRead <= 0) {
                    break;
                }
                std::cout << "Received from client: " << std::string(buffer, bytesRead) << std::endl;
                // 发送响应
                send(clientSocket, buffer, bytesRead, 0);
            }
        }
        // 检查 running 标志位
        if (!running) {
            break;
        }
    }
    std::cout << "Client disconnected" << std::endl;
    close(clientSocket);
}
/****************************main***********************************/
bool quit = false;
std::mutex main_mutex;
static void sigterm_handler(int sig) {
    // 加锁
    {
        std::lock_guard<std::mutex> lock(main_mutex);
        quit = true;
    }
    std::cout << "**************quit success sig:" << sig << std::endl;
}
int main() 
{
    TcpServer server(8888);
    server.start();
	
    // Ctrl-c quit
	signal(SIGINT, sigterm_handler);

    while (!quit)
    {
        sleep(1);
    }
    std::cout << "********Server main exit" << std::endl;
    return 0;
}
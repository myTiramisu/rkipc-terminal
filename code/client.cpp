#include "client.h"
#include <signal.h>
#include <poll.h>
#include <fcntl.h>

Client::Client(const char* server_ip, int server_port)
{
    this->server_ip = server_ip;
    this->server_port = server_port;
    sockfd = -1;
    running_flag = true;
}

Client::~Client() 
{
    // 设置服务器退出标志，并唤醒所有等待的线程
    {
        std::lock_guard<std::mutex> lock(mutex_);
        running_flag = false;
    }
    cv_.notify_all();

    if (sockfd >= 0) 
    {
        if (receive_thread_.joinable()) {
            receive_thread_.join();
        }
        if (send_thread_.joinable()) {
            send_thread_.join();
        }
        close(sockfd);
        sockfd = -1;
        std::cout << "*****************Disconnected from server" << std::endl;
    }
}

bool Client::connect_to_server() {
    // 创建套接字
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return false;
    }

    // 设置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        close(sockfd);
        return false;
    }

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return false;
    }

    running_flag = true;
    receive_thread_ = std::thread(&Client::receive_data_thread, this);
    send_thread_ = std::thread(&Client::send_data_thread, this);
    std::cout << "Connected to server" << std::endl;
    return true;
}
void Client::send_data(const char* message) {
    std::lock_guard<std::mutex> lock(mutex_);
    send_queue_.push(message);
    cv_.notify_one();
}

void Client::send_data_thread() {
    while (running_flag) 
    {
        std::unique_lock<std::mutex> lock(mutex_);

        // 当检测到发送队列不为空 或者 running_flag == fasle时唤醒
        cv_.wait(lock, [this] { return !send_queue_.empty() || !running_flag; });

        if (!running_flag) {
            break;
        }
        std::string message = send_queue_.front();
        send_queue_.pop();

        lock.unlock();

        if (sockfd >= 0) {
            send(sockfd, message.c_str(), message.size(), 0);
            std::cout << "Message sent to server: " << message << std::endl;
        } else {
            std::cerr << "Not connected to server" << std::endl;
        }
    }
    std::cout << "*****************send_data_thread exit" << std::endl;
}

void Client::receive_data_thread() 
{
    char buffer[1024];
    // 创建IO集合 fds[0]表示客户端套接字
    struct pollfd fds[1];  
	// 服务器套接字
	fds[0].fd = sockfd;
	fds[0].events = POLLIN;		//读事件
    while (running_flag) 
    {
        //3. 调用poll函数
        int poll_ret = poll(fds, 1, 1000);   //1000ms溢出
        if(poll_ret < 0) {
            perror("poll error\n");
            exit(-1);
        }
        else if(poll_ret > 0) {
            int valread = read(sockfd, buffer, 1024);
            if (valread > 0) {
                buffer[valread] = '\0';
                std::cout << "Message received from server: " << buffer << std::endl;
            } else if (valread == 0) {
                std::cout << "Server closed the connection" << std::endl;
                running_flag = false;
                break;
            } else {
                perror("Read error");
                running_flag = false;
                break;
            }
        }
    }
    std::cout << "*****************receive_data_thread exit" << std::endl;
}

/*****************************main**************************************/
/*
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
    // Ctrl-c quit
	signal(SIGINT, sigterm_handler);

    Client client("127.0.0.1", 8888);
    if (!client.connect_to_server()) 
    {
        std::cerr << "Failed to connect to server" << std::endl;
        return -1;
    }

    //1. 创建IO集合
    struct pollfd fds[1];
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    while (!quit)
    {   
        int poll_ret = poll(fds, 1, 500);
        if (poll_ret < 0)
        {
            std::cerr << "poll failed" << std::endl;
            break;
        }
        else if (poll_ret > 0)      //键盘事件
        {
            char tempBuf[1024];  // 临时缓冲区
            if(fds[0].revents == POLLIN)
            {
                int nread = read(fds[0].fd, tempBuf, sizeof(tempBuf));
                if (nread <= 0)
                    std::cout << "recv failed" << std::endl;
                else {
                    tempBuf[nread - 1] = '\0';
                    client.send_data(tempBuf);
                }
            }
        }
    }
    std::cout << "*****************quit client main" << std::endl;
    return 0;
}
*/
#include "client.h"
#include <signal.h>
#include <atomic>
Client::Client(const char* server_ip, int server_port)
{
    this->server_ip = server_ip;
    this->server_port = server_port;
    sockfd = -1;
    running_flag = false;
}

Client::~Client() 
{
    disconnect();
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

void Client::disconnect() {
    if (sockfd >= 0) 
    {
        running_flag = false;
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

void Client::send_data(const char* message) {
    std::lock_guard<std::mutex> lock(mutex_);
    send_queue_.push(message);
    cv_.notify_one();
}

void Client::send_data_thread() {
    while (running_flag) 
    {
        std::unique_lock<std::mutex> lock(mutex_);
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
}

void Client::receive_data_thread() 
{
    char buffer[1024];
    while (running_flag) 
    {
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


/*
#include "Client.h"
#include <thread>
#include <chrono>

int main() {
    Client client("127.0.0.1", 8080);

    if (client.connect()) {
        client.send_data("Hello, Server!");
        client.send_data("Another message");

        // 模拟一些时间，让接收线程有机会接收数据
        std::this_thread::sleep_for(std::chrono::seconds(5));

        client.disconnect();
    }

    return 0;
}
*/


std::atomic<bool> quit(false);
static void sigterm_handler(int sig) {
	fprintf(stderr, "signal %d\n", sig);
	std::cout << "**************quit success sig:" << sig << std::endl;
	quit.store(true);
}
int main() 
{
    // Ctrl-c quit
	signal(SIGINT, sigterm_handler);

    Client client("127.0.0.1", 8080);
    if (!client.connect_to_server()) 
    {
        std::cerr << "Failed to connect to server" << std::endl;
        return -1;
    }

    std::string str;
    while (!quit.load())
    {
        std::cin >> str;
        if (std::cin.eof())     break;
        client.send_data(str.c_str());
    }

    client.disconnect();
    return 0;
}
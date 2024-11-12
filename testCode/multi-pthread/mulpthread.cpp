#include <condition_variable>
#include <mutex>
#include <thread>
#include <iostream>
#include <unistd.h>

std::condition_variable frame_cond;
std::mutex frame_mutex;
bool frame_ready = false;
int shared_var = 0;

void consumer() 
{
    while (1)
    {
        std::unique_lock<std::mutex> lock(frame_mutex);
        while (!frame_ready) 
        {
            frame_cond.wait(lock);  // 等待条件变量
        }
        frame_ready = false;
        int local_var = shared_var; // 避免在锁保护下进行耗时操作
        lock.unlock(); // 释放锁

        std::cout << "shared_var = " << local_var << std::endl;
    }
}

int main() {
    std::thread lcd_pthread(consumer);
    while (1)
    {
        usleep(100000); // 睡眠0.1s
        {
            std::lock_guard<std::mutex> lock(frame_mutex);  // 加锁
            shared_var++;
            if (shared_var > 10000)
                shared_var = 0;
            frame_ready = true;
        }
        frame_cond.notify_one(); // 通知消费者
    }
    return 0;
}
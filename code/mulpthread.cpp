#include <condition_variable>
#include <mutex>
#include <thread>
#include <iostream>
#include <unistd.h>

std::condition_variable frame_cond;
std::mutex frame_mutex;
bool frame_ready = false;

void consumer() 
{
    //frame_mutex.lock();
    while (!frame_ready) {
        std::unique_lock<std::mutex> lock(frame_mutex);
        frame_cond.wait(lock); // 等待条件变量
    }
    frame_ready = false;
    //frame_mutex.unlock();
    
    // std::unique_lock<std::mutex> lock(frame_mutex);
    // frame_cond.wait(lock); // 等待条件变量
    std::cout << "Frame is ready!" << std::endl;
}

int main() {
    std::thread t2(consumer);

    std::lock_guard<std::mutex> lock(frame_mutex);
    frame_ready = true;
    frame_cond.notify_one();

    t2.join();

    return 0;
}
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>

std::condition_variable cv;
std::mutex mtx;
int value = 0;

void wait_for_value(int target) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [target] { return value == target; });
    std::cout << "Thread " << std::this_thread::get_id() << " got value " << value << std::endl;
}

void set_value(int new_value) {
    std::lock_guard<std::mutex> lock(mtx);
    value = new_value;
    cv.notify_one();  // 唤醒一个等待的线程
}

int main() {
    std::vector<std::thread> threads;
    // 创建5个线程，每个线程都等待value等于对应的索引值
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(wait_for_value, i);    // 创建线程并立即运行
    }

    for (int i = 0; i < 5; ++i) {
        set_value(i);
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
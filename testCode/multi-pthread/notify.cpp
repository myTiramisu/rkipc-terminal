#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <unordered_set>
#include <signal.h>
#include <unistd.h>
#include <thread>

class Test
{
private:
    bool quit_flag;
    // 线程句柄
    std::thread *thread1_hander;
    std::thread *thread2_hander;
    // 线程函数
    void thread1_func(void);
    void thread2_func(void);
    std::mutex  mutex_thread;

public:
    Test();
    ~Test();

};

Test::Test()
{
    quit_flag = false;
    std::cout << "in test init " << std::endl;
    
    thread1_hander = new std::thread(&Test::thread1_func, this);
    thread2_hander = new std::thread(&Test::thread2_func, this);
}

Test::~Test()
{ 
    {
        std::lock_guard<std::mutex> lock(mutex_thread);
        quit_flag = true;
    }
    if (thread1_hander->joinable())
        thread1_hander->join();
    if (thread2_hander->joinable())
        thread2_hander->join();
    std::cout << "Test deinit success" << std::endl;
}


void Test::thread1_func(void)
{
    std::cout << "thread1_func start" << std::endl;
    while (!quit_flag)
    {
        std::cout << "thread1_func running" << std::endl;
        sleep(1);
    }
}

void Test::thread2_func(void)
{
    std::cout << "thread2_func start" << std::endl;
    while (!quit_flag)
    {
        std::cout << "thread2_func running" << std::endl;
        sleep(1);
    }
}


/**********************************************/
std::condition_variable cv;
std::mutex mtx;
int value = 0;
std::unordered_set<int> processed_values;

void wait_for_value(int target) 
{
    {
        std::unique_lock<std::mutex> lock(mtx);
        // cv.wait(lock);
        cv.wait(lock, [target] { return value == target && processed_values.find(target) == processed_values.end(); });
        processed_values.insert(target);
    }
    std::cout << "Thread " << std::this_thread::get_id() << " got value: " << value << std::endl;
}

void set_value(int new_value) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        value = new_value;
    }
    cv.notify_all();  // 唤醒一个等待的线程
}


bool quit = false;
static void sigterm_handler(int sig) {
	fprintf(stderr, "signal %d\n", sig);
	std::cout << "**************quit success sig:" << sig << std::endl;
	quit = true;
}

int main() 
{
    // Ctrl-c quit
	signal(SIGINT, sigterm_handler);
    Test test;
    while (!quit)
    {
        sleep(1);
    }
    return 0;
}
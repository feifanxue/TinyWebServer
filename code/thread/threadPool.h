#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

using namespace std;

#define THREADPOOL_MAX_NUM 16

class ThreadPool{
private:
    thread* m_manager;
    vector<thread> m_workers;
    atomic<int> m_maxThread;
    atomic<int> m_minThread;
    atomic<int> m_curThread;
    atomic<int> m_idlThread;
    atomic<int> m_exitThread;
    atomic<bool> m_stop;
    queue<function<void()>> m_tasks;
    mutex m_mtx;
    condition_variable m_cond;
    
    void manager();
    void worker();
    void deleteThread();

public:
    ThreadPool(int min = 2, int max = 8);
    ~ThreadPool();
    void addTask(function<void()> task);
};
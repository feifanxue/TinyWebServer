#include "threadPool.h"

ThreadPool::ThreadPool(int min, int max) : m_minThread(min), 
m_maxThread(max), m_curThread(min), m_idlThread(min), m_stop(false) {
    m_manager = new thread(&ThreadPool::manager, this);
    for(int i = 0; i < min; i++) {
        m_workers.emplace_back(thread(&ThreadPool::worker, this));
    }
}

ThreadPool::~ThreadPool() {
    m_stop = true;
    m_cond.notify_all();
    for(thread& t : m_workers) {
        if(t.joinable()) {
            cout << "**********线程 " << t.get_id() << " 正在退出..." << endl;
            t.join();
        }
        if(m_manager->joinable()) {
            cout << "管理者线程 " << m_manager->get_id() << " 正在退出..." << endl;
            m_manager->join();
        }
        delete m_manager;
    }
}

void ThreadPool::addTask(function<void()> task) {
    unique_lock<mutex> locker(m_mtx);
    m_tasks.emplace(task);
    m_cond.notify_one();
}

void ThreadPool::manager() {
    while(!m_stop.load()) {
        this_thread::sleep_for(chrono::seconds(3));
        int idle = m_idlThread.load();
        int cur = m_curThread.load();
        if(idle > cur / 2 && cur > m_minThread) {
            m_exitThread.store(2);
            m_cond.notify_all();
        } else if(idle == 0 && cur < m_maxThread) {
            m_workers.emplace_back(thread(&ThreadPool::worker, this));
            m_curThread++;
            m_idlThread++;
        }
    }
}

void ThreadPool::deleteThread() {

}

void ThreadPool::worker() {
    while(!m_stop.load()) {
        function<void()> task = nullptr;
        {
            unique_lock<mutex> locker(m_mtx);
            while(m_tasks.empty() && !m_stop) {
                m_cond.wait(locker);
                if(m_exitThread.load() > 0) {
                    m_curThread--;
                    m_exitThread--;
                    cout << "-------- 线程退出了， ID: " << this_thread::get_id() << endl;
                    return;
                }
            }
            if(!m_tasks.empty()) {
                cout << "取出了一个任务..." << endl;
                task = move(m_tasks.front());
                m_tasks.pop();
            }
        }
        if(task) {
            m_idlThread--;
            task();
            m_idlThread++;
        }
    }
}

void calc(int x, int y) {
    int z = x + y;
    cout << "z = " << z << endl;
    this_thread::sleep_for(chrono::seconds(2));
}

int main() {
    ThreadPool pool;
    for(int i = 0; i < 10; i++) {
        auto obj = bind(calc, i, i * 2);
        pool.addTask(obj);
    }
    getchar();
    return 0;
}
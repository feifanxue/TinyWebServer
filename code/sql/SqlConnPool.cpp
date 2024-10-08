#include "SqlConnPool.h"
#include <json/json.h>
#include <fstream>

using namespace Json;

SqlConnPool* SqlConnPool::getInstance() {
    static SqlConnPool pool;
    return &pool;
}

SqlConnPool::SqlConnPool() {
    if(!parseJsonFile()) {
        return;
    }
    m_stop = false;
    for(size_t i = 0; i < m_minSize; i++) {
        addConnection();
    }
    thread producer(&SqlConnPool::produceConnection, this);
    thread recycler(&SqlConnPool::recycleConnection, this);
    producer.detach();
    recycler.detach();
}

SqlConnPool::~SqlConnPool() {
    m_stop = true;
    m_cond.notify_all();
    while(!m_connectionQ.empty()) {
        MysqlConn* conn = m_connectionQ.front();
        m_connectionQ.pop();
        delete conn;
    }
}

void SqlConnPool::produceConnection() {
    while(!m_stop) {
        unique_lock<mutex> locker(m_mutexQ);
        while(m_connectionQ.size() >= m_minSize) {
            m_cond.wait(locker);
        }
        addConnection();
        m_cond.notify_all();
    }
}

void SqlConnPool::recycleConnection() {
    while(!m_stop) {
        this_thread::sleep_for(chrono::milliseconds(500));
        unique_lock<mutex> locker(m_mutexQ);
        while(m_connectionQ.size() > m_minSize) {
            MysqlConn* conn = m_connectionQ.front();
            if(conn->getAliveTime() >= m_maxIdleTime) {
                m_connectionQ.pop();
                delete conn;
            }
            else {
                break;
            }
        }
    }
}

void SqlConnPool::addConnection() {
    MysqlConn* conn= new MysqlConn;
    conn->connect(m_user, m_passwd, m_dbName, m_ip, m_port);
    conn->refreshAliveTime();
    m_connectionQ.emplace(conn);
}

shared_ptr<MysqlConn> SqlConnPool::getConnection() {
    unique_lock<mutex> locker(m_mutexQ);
    while(m_connectionQ.empty()) {
        if(cv_status::timeout == m_cond.wait_for(locker, chrono::milliseconds(m_timeout))) {
            if(m_connectionQ.empty()) {
                //return nullptr;
                continue;
            }
        }
    }
    shared_ptr<MysqlConn> connptr(m_connectionQ.front(), [this](MysqlConn* conn) {
        unique_lock<mutex> locker(m_mutexQ);
        conn->refreshAliveTime();
        m_connectionQ.emplace(conn);
    });
    m_connectionQ.pop();
    m_cond.notify_all();
    return connptr;
}

bool SqlConnPool::parseJsonFile() {
    ifstream ifs("./config/dbconf.json");
    Reader rd;
    Value root;
    rd.parse(ifs, root);
    if(root.isObject()) {
        m_ip = root["ip"].asString();
        m_port = root["port"].asInt();
        m_user = root["userName"].asString();
        m_passwd = root["password"].asString();
        m_dbName = root["dbName"].asString();
        m_minSize = root["minSize"].asInt();
        m_maxSize = root["maxSize"].asInt();
        m_maxIdleTime = root["maxIdleTime"].asInt();
        m_timeout = root["timeout"].asInt();
        return true;
    }
    return false;
}
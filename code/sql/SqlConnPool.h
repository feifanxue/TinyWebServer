#ifndef _SQLCONNPOOL_H_
#define _SQLCONNPOOL_H_

#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "MysqlConn.h"

using namespace std;

class SqlConnPool {
public:
    static SqlConnPool* getInstance();
    SqlConnPool(const SqlConnPool& obj) = delete;
    SqlConnPool& operator=(const SqlConnPool& obj) = delete;
    shared_ptr<MysqlConn> getConnection();
    ~SqlConnPool();
private:
    SqlConnPool();

    bool parseJsonFile();
    void produceConnection();
    void recycleConnection();
    void addConnection();

    string m_ip;
    string m_user;
    string m_passwd;
    string m_dbName;
    unsigned short m_port;
    size_t m_minSize;
    size_t m_maxSize;
    int m_timeout;
    int m_maxIdleTime;
    bool m_stop;
    queue<MysqlConn*> m_connectionQ;
    mutex m_mutexQ;
    condition_variable m_cond;
};

#endif
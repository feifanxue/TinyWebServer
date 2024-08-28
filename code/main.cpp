#include <iostream>
#include <memory>
#include "sql/MysqlConn.h"
#include "sql/SqlConnPool.h"

using namespace std;

void op1(int begin, int end) {
    for(int i = begin; i < end; i++) {
        MysqlConn conn;
        conn.connect("root", "root", "myTinyWebServerDB", "localhost");
        char sql[1024] = {0};
        sprintf(sql, "insert into user values(%s, 'xff')", to_string(i).c_str());
        conn.update(sql);
    }
}

void op2(SqlConnPool* pool, int begin, int end) {
    for(int i = begin; i < end; i++) {
        shared_ptr<MysqlConn> conn = pool->getConnection();
        char sql[1024] = {0};
        sprintf(sql, "insert into user values(%s, 'xff')", to_string(i).c_str());
        conn->update(sql);
    }
}

void test1() {
#if 0
    // 非连接池，单线程，用时：3275284509 纳秒，3275 毫秒
    steady_clock::time_point begin = steady_clock::now();
    op1(0, 5000);
    steady_clock::time_point end = steady_clock::now();
    auto length = end - begin;
    cout << "非连接池，单线程，用时：" << length.count() << " 纳秒，"
        <<length.count() / 1000000 << " 毫秒" << endl;
#else
    // 连接池，单线程，用时：2027934689 纳秒，2027 毫秒
    SqlConnPool* pool = SqlConnPool::getInstance();
    steady_clock::time_point begin = steady_clock::now();
    op2(pool, 0, 5000);
    steady_clock::time_point end = steady_clock::now();
    auto length = end - begin;
    cout << "连接池，单线程，用时：" << length.count() << " 纳秒，"
        <<length.count() / 1000000 << " 毫秒" << endl;
#endif
}

void test2() {
    #if 0
    // 非连接池，多线程，用时：1759788713 纳秒，1759 毫秒
    MysqlConn conn;
    conn.connect("root", "root", "myTinyWebServerDB", "localhost");
    steady_clock::time_point begin = steady_clock::now();
    thread t1(op1, 0, 1000);
    thread t2(op1, 1000, 2000);
    thread t3(op1, 2000, 3000);
    thread t4(op1, 3000, 4000);
    thread t5(op1, 4000, 5000);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    steady_clock::time_point end = steady_clock::now();
    auto length = end - begin;
    cout << "非连接池，多线程，用时：" << length.count() << " 纳秒，"
        <<length.count() / 1000000 << " 毫秒" << endl;
#else
    // 连接池，多线程，用时：1171021608 纳秒，1171 毫秒
    SqlConnPool* pool = SqlConnPool::getInstance();
    steady_clock::time_point begin = steady_clock::now();
    thread t1(op2, pool, 0, 1000);
    thread t2(op2, pool, 1000, 2000);
    thread t3(op2, pool, 2000, 3000);
    thread t4(op2, pool, 3000, 4000);
    thread t5(op2, pool, 4000, 5000);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    steady_clock::time_point end = steady_clock::now();
    auto length = end - begin;
    cout << "连接池，多线程，用时：" << length.count() << " 纳秒，"
        <<length.count() / 1000000 << " 毫秒" << endl;
#endif
}

int query() {
    MysqlConn conn;
    conn.connect("root", "root", "myTinyWebServerDB", "localhost");
    string sql = "insert into user values('xff', 'xff')";
    bool flag = conn.update(sql);
    cout << "flag value: " << flag << endl;
    sql = "select * from user";
    conn.query(sql);
    while(conn.next()) {
        cout << conn.value(0) << ", "
            << conn.value(1) << endl;
    }
    return 0;
}

int main() {
    test2();
    return 0;
}
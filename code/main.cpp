#include <iostream>
#include <memory>
#include "sql/MysqlConn.h"

using namespace std;

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
    query();
    return 0;
}
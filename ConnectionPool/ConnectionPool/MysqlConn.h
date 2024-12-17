#pragma once
#include<iostream>
#include<string>
#include<mysql.h>
#include<chrono>
using namespace std;
using namespace chrono;
class MysqlConn
{
public:
    // 构造函数：初始化数据库连接
    MysqlConn();

    // 析构函数：释放数据库连接
    ~MysqlConn();

    // 连接数据库，传入用户名、密码、数据库名、IP地址和端口号
    bool connect(string user, string passwd, string dbName, string ip, unsigned short port = 3306);

    // 执行更新操作（例如 INSERT、UPDATE、DELETE），返回操作是否成功
    bool update(string sql);

    // 执行查询操作，返回是否成功
    bool query(string sql);

    // 遍历查询得到的结果集，返回是否有下一行数据
    bool next();

    // 获取查询结果集中某列的值，返回对应的字段值
    string value(int index);

    // 开始事务操作（关闭自动提交）
    bool transaction();

    // 提交事务操作
    bool commit();

    // 事务回滚操作
    bool rollback();

    //刷新起始的空闲时间点
    void refreshAliveTime();
    //计算连接存活的总时长
    long long getAliveTime();
private:
    // 释放查询结果集的资源
    void freeResult();

    MYSQL* m_conn = nullptr;         // MySQL连接对象
    MYSQL_RES* m_result = nullptr;   // 查询结果集
    MYSQL_ROW m_row = nullptr;       // 当前行数据（指针）
    steady_clock::time_point m_alivetime;
}; 
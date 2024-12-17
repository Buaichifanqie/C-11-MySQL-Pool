#pragma once
// 防止头文件被多次包含

// 单例模式——懒汉
#include<queue>              // 引入队列，存储数据库连接对象
#include<mutex>              // 引入互斥锁，用于保护共享资源的访问
#include"MysqlConn.h"       // 引入 MysqlConn 头文件，表示数据库连接对象
#include<condition_variable> // 引入条件变量，用于线程同步

using namespace std;

class ConnectionPool
{
public:
    // 获取连接池实例，采用懒汉式单例模式
    static ConnectionPool* getConnectionPool();

    // 删除拷贝构造函数和赋值运算符，避免复制连接池对象
    ConnectionPool(const ConnectionPool& obj) = delete;
    ConnectionPool& operator=(const ConnectionPool& obj) = delete;
    shared_ptr<MysqlConn>  getConnection();
    ~ConnectionPool();
private:
    // 私有构造函数，外部无法直接创建实例
    ConnectionPool();

    // 私有函数，用于解析配置文件，初始化连接池参数
    bool parseJsonFile();

    // 生产连接的线程函数
    void produceConnection();

    // 回收连接的线程函数
    void recycleConnection();

    void addConnection();

    // 配置文件中的数据库连接参数
    string m_ip;          // 数据库IP地址
    string m_user;        // 数据库用户名
    string m_passwd;      // 数据库密码
    string m_dbName;      // 数据库名称
    unsigned short m_port; // 数据库端口
    int m_minSize;        // 最小连接池大小
    int m_maxSize;        // 最大连接池大小
    int m_timeout;        // 超时时间
    int m_maxIdleTime;    // 最大空闲时间

    // 存储数据库连接的队列
    queue<MysqlConn*> m_connectionQ;

    // 保护连接池队列访问的互斥锁
    mutex m_mutexQ;

    // 用于线程间同步的条件变量
    condition_variable m_cond;
};
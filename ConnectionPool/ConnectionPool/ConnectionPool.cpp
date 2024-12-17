#include "ConnectionPool.h"
#include<json/json.h>
#include<thread>
#include<fstream>

using namespace Json;

// 获取单例模式的数据库连接池实例
ConnectionPool* ConnectionPool::getConnectionPool()
{
    static ConnectionPool pool; // 在第一次调用时创建 ConnectionPool 实例
    return &pool; // 返回该实例的指针
}

// 解析配置文件 dbconf.json
bool ConnectionPool::parseJsonFile()
{
    ifstream ifs("dbconf.json");  // 打开配置文件
    Reader rd;                    // 创建 JSON 解析器
    Value root;                   // 存储 JSON 解析结果
    rd.parse(ifs, root);          // 解析 JSON 文件

    // 如果配置文件是一个 JSON 对象，则按配置文件中的内容初始化成员变量
    if (root.isObject())
    {
        m_ip = root["ip"].asString();              // 读取数据库 IP 地址
        m_port = root["port"].asInt();              // 读取数据库端口
        m_user = root["userName"].asString();       // 读取用户名
        m_passwd = root["passwd"].asString();       // 读取密码
        m_dbName = root["dbName"].asString();       // 读取数据库名
        m_minSize = root["minSize"].asInt();        // 读取最小连接数
        m_maxSize = root["maxSize"].asInt();        // 读取最大连接数
        m_maxIdleTime = root["maxIdleTime"].asInt(); // 读取最大空闲时间
        m_timeout = root["timeout"].asInt();         // 读取超时时间
        return true; // 解析成功
    }
    return false; // 如果 JSON 格式错误，则返回失败
}

// 生产数据库连接的线程方法
void ConnectionPool::produceConnection()
{
    while (true)  // 持续运行，保持连接池在后台工作
    {
        unique_lock<mutex> locker(m_mutexQ);  // 使用 unique_lock 保护连接池队列，确保线程安全
        // 如果当前连接池中的连接数已经达到最小连接数，则进入等待状态
        // 这里使用 while 是为了避免虚假唤醒的情况，确保条件成立才继续执行
        while (m_connectionQ.size() >= m_minSize)
        {
            m_cond.wait(locker);  // 当前连接池中连接数大于等于最小连接数时，阻塞等待
        }

        // 连接池中空闲连接不足，创建新的数据库连接
        addConnection();  // 该方法应该创建一个新的数据库连接并加入连接池队列
        m_cond.notify_all();
    }
}

// 回收数据库连接的线程方法
void ConnectionPool::recycleConnection()
{
    while (true)  // 持续运行，保持回收连接的任务
    {
        // 每隔 500 毫秒检查一次连接池中的连接，避免占用过多的 CPU 资源
        this_thread::sleep_for(chrono::milliseconds(500));  // 线程休眠 500 毫秒
        lock_guard<mutex>locker(m_mutexQ);
        // 当连接池中的连接数大于最小连接数时，开始回收空闲连接
        while (m_connectionQ.size() > m_minSize)
        {
            // 获取连接池中的第一个连接（队头连接）
            MysqlConn* conn = m_connectionQ.front();

            // 判断该连接的空闲时间是否超过最大空闲时间
            if (conn->getAliveTime() >= m_maxIdleTime)
            {
                // 如果超过最大空闲时间，则回收该连接
                m_connectionQ.pop();  // 从队列中移除该连接
                delete conn;  // 释放该连接的内存
            }
            else
            {
                // 如果该连接的空闲时间未超过最大空闲时间，则不回收，跳出循环
                break;
            }
        }
    }
}

// 向连接池中添加新的数据库连接
void ConnectionPool::addConnection()
{
    // 创建一个新的 MysqlConn 对象，用于建立新的数据库连接
    MysqlConn* conn = new MysqlConn;  // 在堆上分配内存，创建一个新的 MysqlConn 对象

    // 使用配置文件中的参数（用户名、密码、数据库名、IP 和端口）来连接数据库
    conn->connect(m_user, m_passwd, m_dbName, m_ip, m_port);  // 调用 MysqlConn 的 connect 方法建立数据库连接
    // 刷新该连接的生存时间，表示该连接刚被创建并是“活跃的”
    conn->refreshAliveTime();  // 更新连接的“生存时间”或“最后使用时间”

    // 将创建的数据库连接对象加入到连接池队列中
    m_connectionQ.push(conn);  // 将新连接加入到连接池队列中，等待被其他线程使用
}

shared_ptr<MysqlConn> ConnectionPool::getConnection()
{
    unique_lock<mutex>locker(m_mutexQ);
    while (m_connectionQ.empty())
    {
        if (cv_status::timeout == m_cond.wait_for(locker, chrono::milliseconds(m_timeout)))
        {
            if (m_connectionQ.empty())
            {
                //return nullptr;
                continue;
            }
        }
    }
    shared_ptr<MysqlConn> connptr(m_connectionQ.front(), [this](MysqlConn* conn)//指定删除器
        {
            lock_guard<mutex>locker(m_mutexQ);
            conn->refreshAliveTime();
            m_connectionQ.push(conn);
        });
    m_connectionQ.pop();
    m_cond.notify_all();
    return connptr;
}

ConnectionPool::~ConnectionPool()
{
    while (!m_connectionQ.empty())
    {
        MysqlConn* conn = m_connectionQ.front();
        m_connectionQ.pop();
        delete conn;
    }
}

// 构造函数：初始化连接池
ConnectionPool::ConnectionPool()
{
    // 加载配置文件 dbconf.json
    if (!parseJsonFile())
    {
        return; // 如果配置文件加载失败，直接返回
    }

    // 根据配置文件中的最小连接数，创建初始数据库连接并推入连接池队列中
    for (int i = 0; i < m_minSize; i++)
    {
        addConnection();
    }

    // 创建两个线程：一个负责生产连接，一个负责回收连接
    thread producer(&ConnectionPool::produceConnection, this); // 生产连接线程
    thread recycler(&ConnectionPool::produceConnection, this); // 回收连接线程
    producer.detach(); // 分离生产连接线程，使其在后台运行
    recycler.detach(); // 分离回收连接线程，使其在后台运行
}
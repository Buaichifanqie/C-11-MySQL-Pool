#include "MysqlConn.h"

MysqlConn::MysqlConn()
{
    m_conn = mysql_init(nullptr);  // 初始化MySQL连接对象
    // 防止出现中文乱码，使用UTF-8字符集
    mysql_set_character_set(m_conn, "utf8");
}

MysqlConn::~MysqlConn()
{
    if (m_conn != nullptr)
    {
        mysql_close(m_conn);  // 关闭数据库连接
    }
    freeResult();  // 释放查询结果集
}

bool MysqlConn::connect(string user, string passwd, string dbName, string ip, unsigned short port)
{
    // 使用指定的参数连接数据库
    MYSQL* ptr = mysql_real_connect(m_conn, ip.c_str(), user.c_str(), passwd.c_str(), dbName.c_str(), port, nullptr, 0);
    return ptr != nullptr;  // 如果连接成功返回true，否则返回false
}

bool MysqlConn::update(string sql)
{
    // 执行更新操作（如INSERT、UPDATE、DELETE等）
    if (mysql_query(m_conn, sql.c_str()))
    {
        return false;  // 执行失败，返回false
    }
    return true;  // 执行成功，返回true
}

bool MysqlConn::query(string sql)
{
    freeResult();  // 先释放之前的查询结果集
    // 执行查询操作
    if (mysql_query(m_conn, sql.c_str()))
    {
        return false;  // 查询失败，返回false
    }
    m_result = mysql_store_result(m_conn);  // 获取查询结果集
    return true;  // 查询成功，返回true
}

bool MysqlConn::next()
{
    if (m_result != nullptr)
    {
        m_row = mysql_fetch_row(m_result);  // 获取当前行的数据
    }
    return m_row != nullptr;  // 如果当前行存在数据，返回true，否则返回false
}

string MysqlConn::value(int index)
{
    int rowCount = mysql_num_fields(m_result);  // 获取结果集中的字段数量
    if (index >= rowCount || index < 0)
    {
        return string();  // 如果索引不合法，返回空字符串
    }
    char* val = m_row[index];  // 获取指定列的值
    unsigned long length = mysql_fetch_lengths(m_result)[index];  // 获取该列数据的长度
    return string(val, length);  // 返回该列的值，确保不会出现'\0'的截断
}

bool MysqlConn::transaction()
{
    // 开始事务，设置为非自动提交
    return mysql_autocommit(m_conn, false);
}

bool MysqlConn::commit()
{
    // 提交事务
    return mysql_commit(m_conn);
}

bool MysqlConn::rollback()
{
    // 回滚事务
    return mysql_rollback(m_conn);
}

void MysqlConn::refreshAliveTime()
{
    m_alivetime = steady_clock::now();
}

long long MysqlConn::getAliveTime()
{
    nanoseconds res = steady_clock::now() - m_alivetime;
    milliseconds millsec = duration_cast<milliseconds>(res);//高精度转低精度
    return millsec.count();
}

void MysqlConn::freeResult()
{
    if (m_result)
    {
        mysql_free_result(m_result);  // 释放查询结果集
        m_result = nullptr;  // 清空结果集指针
    }
}
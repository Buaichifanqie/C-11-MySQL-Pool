#include "MysqlConn.h"

MysqlConn::MysqlConn()
{
    m_conn = mysql_init(nullptr);  // ��ʼ��MySQL���Ӷ���
    // ��ֹ�����������룬ʹ��UTF-8�ַ���
    mysql_set_character_set(m_conn, "utf8");
}

MysqlConn::~MysqlConn()
{
    if (m_conn != nullptr)
    {
        mysql_close(m_conn);  // �ر����ݿ�����
    }
    freeResult();  // �ͷŲ�ѯ�����
}

bool MysqlConn::connect(string user, string passwd, string dbName, string ip, unsigned short port)
{
    // ʹ��ָ���Ĳ����������ݿ�
    MYSQL* ptr = mysql_real_connect(m_conn, ip.c_str(), user.c_str(), passwd.c_str(), dbName.c_str(), port, nullptr, 0);
    return ptr != nullptr;  // ������ӳɹ�����true�����򷵻�false
}

bool MysqlConn::update(string sql)
{
    // ִ�и��²�������INSERT��UPDATE��DELETE�ȣ�
    if (mysql_query(m_conn, sql.c_str()))
    {
        return false;  // ִ��ʧ�ܣ�����false
    }
    return true;  // ִ�гɹ�������true
}

bool MysqlConn::query(string sql)
{
    freeResult();  // ���ͷ�֮ǰ�Ĳ�ѯ�����
    // ִ�в�ѯ����
    if (mysql_query(m_conn, sql.c_str()))
    {
        return false;  // ��ѯʧ�ܣ�����false
    }
    m_result = mysql_store_result(m_conn);  // ��ȡ��ѯ�����
    return true;  // ��ѯ�ɹ�������true
}

bool MysqlConn::next()
{
    if (m_result != nullptr)
    {
        m_row = mysql_fetch_row(m_result);  // ��ȡ��ǰ�е�����
    }
    return m_row != nullptr;  // �����ǰ�д������ݣ�����true�����򷵻�false
}

string MysqlConn::value(int index)
{
    int rowCount = mysql_num_fields(m_result);  // ��ȡ������е��ֶ�����
    if (index >= rowCount || index < 0)
    {
        return string();  // ����������Ϸ������ؿ��ַ���
    }
    char* val = m_row[index];  // ��ȡָ���е�ֵ
    unsigned long length = mysql_fetch_lengths(m_result)[index];  // ��ȡ�������ݵĳ���
    return string(val, length);  // ���ظ��е�ֵ��ȷ���������'\0'�Ľض�
}

bool MysqlConn::transaction()
{
    // ��ʼ��������Ϊ���Զ��ύ
    return mysql_autocommit(m_conn, false);
}

bool MysqlConn::commit()
{
    // �ύ����
    return mysql_commit(m_conn);
}

bool MysqlConn::rollback()
{
    // �ع�����
    return mysql_rollback(m_conn);
}

void MysqlConn::refreshAliveTime()
{
    m_alivetime = steady_clock::now();
}

long long MysqlConn::getAliveTime()
{
    nanoseconds res = steady_clock::now() - m_alivetime;
    milliseconds millsec = duration_cast<milliseconds>(res);//�߾���ת�;���
    return millsec.count();
}

void MysqlConn::freeResult()
{
    if (m_result)
    {
        mysql_free_result(m_result);  // �ͷŲ�ѯ�����
        m_result = nullptr;  // ��ս����ָ��
    }
}
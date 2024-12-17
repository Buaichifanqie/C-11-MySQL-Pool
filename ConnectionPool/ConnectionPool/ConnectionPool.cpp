#include "ConnectionPool.h"
#include<json/json.h>
#include<thread>
#include<fstream>

using namespace Json;

// ��ȡ����ģʽ�����ݿ����ӳ�ʵ��
ConnectionPool* ConnectionPool::getConnectionPool()
{
    static ConnectionPool pool; // �ڵ�һ�ε���ʱ���� ConnectionPool ʵ��
    return &pool; // ���ظ�ʵ����ָ��
}

// ���������ļ� dbconf.json
bool ConnectionPool::parseJsonFile()
{
    ifstream ifs("dbconf.json");  // �������ļ�
    Reader rd;                    // ���� JSON ������
    Value root;                   // �洢 JSON �������
    rd.parse(ifs, root);          // ���� JSON �ļ�

    // ��������ļ���һ�� JSON �����������ļ��е����ݳ�ʼ����Ա����
    if (root.isObject())
    {
        m_ip = root["ip"].asString();              // ��ȡ���ݿ� IP ��ַ
        m_port = root["port"].asInt();              // ��ȡ���ݿ�˿�
        m_user = root["userName"].asString();       // ��ȡ�û���
        m_passwd = root["passwd"].asString();       // ��ȡ����
        m_dbName = root["dbName"].asString();       // ��ȡ���ݿ���
        m_minSize = root["minSize"].asInt();        // ��ȡ��С������
        m_maxSize = root["maxSize"].asInt();        // ��ȡ���������
        m_maxIdleTime = root["maxIdleTime"].asInt(); // ��ȡ������ʱ��
        m_timeout = root["timeout"].asInt();         // ��ȡ��ʱʱ��
        return true; // �����ɹ�
    }
    return false; // ��� JSON ��ʽ�����򷵻�ʧ��
}

// �������ݿ����ӵ��̷߳���
void ConnectionPool::produceConnection()
{
    while (true)  // �������У��������ӳ��ں�̨����
    {
        unique_lock<mutex> locker(m_mutexQ);  // ʹ�� unique_lock �������ӳض��У�ȷ���̰߳�ȫ
        // �����ǰ���ӳ��е��������Ѿ��ﵽ��С�������������ȴ�״̬
        // ����ʹ�� while ��Ϊ�˱�����ٻ��ѵ������ȷ�����������ż���ִ��
        while (m_connectionQ.size() >= m_minSize)
        {
            m_cond.wait(locker);  // ��ǰ���ӳ������������ڵ�����С������ʱ�������ȴ�
        }

        // ���ӳ��п������Ӳ��㣬�����µ����ݿ�����
        addConnection();  // �÷���Ӧ�ô���һ���µ����ݿ����Ӳ��������ӳض���
        m_cond.notify_all();
    }
}

// �������ݿ����ӵ��̷߳���
void ConnectionPool::recycleConnection()
{
    while (true)  // �������У����ֻ������ӵ�����
    {
        // ÿ�� 500 ������һ�����ӳ��е����ӣ�����ռ�ù���� CPU ��Դ
        this_thread::sleep_for(chrono::milliseconds(500));  // �߳����� 500 ����
        lock_guard<mutex>locker(m_mutexQ);
        // �����ӳ��е�������������С������ʱ����ʼ���տ�������
        while (m_connectionQ.size() > m_minSize)
        {
            // ��ȡ���ӳ��еĵ�һ�����ӣ���ͷ���ӣ�
            MysqlConn* conn = m_connectionQ.front();

            // �жϸ����ӵĿ���ʱ���Ƿ񳬹�������ʱ��
            if (conn->getAliveTime() >= m_maxIdleTime)
            {
                // �������������ʱ�䣬����ո�����
                m_connectionQ.pop();  // �Ӷ������Ƴ�������
                delete conn;  // �ͷŸ����ӵ��ڴ�
            }
            else
            {
                // ��������ӵĿ���ʱ��δ����������ʱ�䣬�򲻻��գ�����ѭ��
                break;
            }
        }
    }
}

// �����ӳ�������µ����ݿ�����
void ConnectionPool::addConnection()
{
    // ����һ���µ� MysqlConn �������ڽ����µ����ݿ�����
    MysqlConn* conn = new MysqlConn;  // �ڶ��Ϸ����ڴ棬����һ���µ� MysqlConn ����

    // ʹ�������ļ��еĲ������û��������롢���ݿ�����IP �Ͷ˿ڣ����������ݿ�
    conn->connect(m_user, m_passwd, m_dbName, m_ip, m_port);  // ���� MysqlConn �� connect �����������ݿ�����
    // ˢ�¸����ӵ�����ʱ�䣬��ʾ�����Ӹձ��������ǡ���Ծ�ġ�
    conn->refreshAliveTime();  // �������ӵġ�����ʱ�䡱�����ʹ��ʱ�䡱

    // �����������ݿ����Ӷ�����뵽���ӳض�����
    m_connectionQ.push(conn);  // �������Ӽ��뵽���ӳض����У��ȴ��������߳�ʹ��
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
    shared_ptr<MysqlConn> connptr(m_connectionQ.front(), [this](MysqlConn* conn)//ָ��ɾ����
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

// ���캯������ʼ�����ӳ�
ConnectionPool::ConnectionPool()
{
    // ���������ļ� dbconf.json
    if (!parseJsonFile())
    {
        return; // ��������ļ�����ʧ�ܣ�ֱ�ӷ���
    }

    // ���������ļ��е���С��������������ʼ���ݿ����Ӳ��������ӳض�����
    for (int i = 0; i < m_minSize; i++)
    {
        addConnection();
    }

    // ���������̣߳�һ�������������ӣ�һ�������������
    thread producer(&ConnectionPool::produceConnection, this); // ���������߳�
    thread recycler(&ConnectionPool::produceConnection, this); // ���������߳�
    producer.detach(); // �������������̣߳�ʹ���ں�̨����
    recycler.detach(); // ������������̣߳�ʹ���ں�̨����
}
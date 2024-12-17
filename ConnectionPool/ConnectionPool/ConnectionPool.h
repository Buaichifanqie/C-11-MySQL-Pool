#pragma once
// ��ֹͷ�ļ�����ΰ���

// ����ģʽ��������
#include<queue>              // ������У��洢���ݿ����Ӷ���
#include<mutex>              // ���뻥���������ڱ���������Դ�ķ���
#include"MysqlConn.h"       // ���� MysqlConn ͷ�ļ�����ʾ���ݿ����Ӷ���
#include<condition_variable> // �������������������߳�ͬ��

using namespace std;

class ConnectionPool
{
public:
    // ��ȡ���ӳ�ʵ������������ʽ����ģʽ
    static ConnectionPool* getConnectionPool();

    // ɾ���������캯���͸�ֵ����������⸴�����ӳض���
    ConnectionPool(const ConnectionPool& obj) = delete;
    ConnectionPool& operator=(const ConnectionPool& obj) = delete;
    shared_ptr<MysqlConn>  getConnection();
    ~ConnectionPool();
private:
    // ˽�й��캯�����ⲿ�޷�ֱ�Ӵ���ʵ��
    ConnectionPool();

    // ˽�к��������ڽ��������ļ�����ʼ�����ӳز���
    bool parseJsonFile();

    // �������ӵ��̺߳���
    void produceConnection();

    // �������ӵ��̺߳���
    void recycleConnection();

    void addConnection();

    // �����ļ��е����ݿ����Ӳ���
    string m_ip;          // ���ݿ�IP��ַ
    string m_user;        // ���ݿ��û���
    string m_passwd;      // ���ݿ�����
    string m_dbName;      // ���ݿ�����
    unsigned short m_port; // ���ݿ�˿�
    int m_minSize;        // ��С���ӳش�С
    int m_maxSize;        // ������ӳش�С
    int m_timeout;        // ��ʱʱ��
    int m_maxIdleTime;    // ������ʱ��

    // �洢���ݿ����ӵĶ���
    queue<MysqlConn*> m_connectionQ;

    // �������ӳض��з��ʵĻ�����
    mutex m_mutexQ;

    // �����̼߳�ͬ������������
    condition_variable m_cond;
};
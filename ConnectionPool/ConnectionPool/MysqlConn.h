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
    // ���캯������ʼ�����ݿ�����
    MysqlConn();

    // �����������ͷ����ݿ�����
    ~MysqlConn();

    // �������ݿ⣬�����û��������롢���ݿ�����IP��ַ�Ͷ˿ں�
    bool connect(string user, string passwd, string dbName, string ip, unsigned short port = 3306);

    // ִ�и��²��������� INSERT��UPDATE��DELETE�������ز����Ƿ�ɹ�
    bool update(string sql);

    // ִ�в�ѯ�����������Ƿ�ɹ�
    bool query(string sql);

    // ������ѯ�õ��Ľ�����������Ƿ�����һ������
    bool next();

    // ��ȡ��ѯ�������ĳ�е�ֵ�����ض�Ӧ���ֶ�ֵ
    string value(int index);

    // ��ʼ����������ر��Զ��ύ��
    bool transaction();

    // �ύ�������
    bool commit();

    // ����ع�����
    bool rollback();

    //ˢ����ʼ�Ŀ���ʱ���
    void refreshAliveTime();
    //�������Ӵ�����ʱ��
    long long getAliveTime();
private:
    // �ͷŲ�ѯ���������Դ
    void freeResult();

    MYSQL* m_conn = nullptr;         // MySQL���Ӷ���
    MYSQL_RES* m_result = nullptr;   // ��ѯ�����
    MYSQL_ROW m_row = nullptr;       // ��ǰ�����ݣ�ָ�룩
    steady_clock::time_point m_alivetime;
}; 
#define _CRT_SECURE_NO_WARNINGS
#include<memory>
#include"MysqlConn.h"
#include"ConnectionPool.h"
//���߳� ʹ��/��ʹ��
//���߳� ʹ��/��ʹ��

void op1(int begin, int end)
{
	for (int i = begin; i < end; i++)
	{
		MysqlConn conn;
		conn.connect("root", "123456", "My_sql", "192.168.28.128");
		char sql[1024] = { 0 };
		sprintf(sql, "insert into score values (%d, 'aoqi', 65, 100, 98)", i+5);
		conn.update(sql);
	}
}

void op2(ConnectionPool* pool, int begin, int end)
{
	for (int i = begin; i < end; i++)
	{
		shared_ptr<MysqlConn>conn = pool->getConnection();
		char sql[1024] = { 0 };
		sprintf(sql, "insert into score values (%d, 'aoqi', 65, 100, 98)", i + 5);
		conn->update(sql);
	}
}

void test1()
{
#if 0
	//�����ӳأ����߳���ʱ��35485877700����35485����
	steady_clock::time_point begin= steady_clock::now();
	op1(0, 5000);
	steady_clock::time_point end= steady_clock::now();
	auto length = end - begin;
	cout << "�����ӳأ����߳���ʱ��" << length.count() << "����"
		<< length.count() / 1000000 << "����" << endl;
#else
	//���ӳأ����߳���ʱ��3664802300����3664����
	ConnectionPool* pool = ConnectionPool::getConnectionPool();
	steady_clock::time_point begin = steady_clock::now();
	op2(pool,0, 5000);
	steady_clock::time_point end = steady_clock::now();
	auto length = end - begin;
	cout << "���ӳأ����߳���ʱ��" << length.count() << "����"
		<< length.count() / 1000000 << "����" << endl;


#endif
}

void test2()
{
#if 0
	MysqlConn conn;
	conn.connect("root", "123456", "My_sql", "192.168.28.128");
	//�����ӳأ����߳���ʱ��7115655600����7115����
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
	cout << "�����ӳأ����߳���ʱ��" << length.count() << "����"
		<< length.count() / 1000000 << "����" << endl;
#else
	//���ӳأ����߳���ʱ��1282472300����1282����
	ConnectionPool* pool = ConnectionPool::getConnectionPool();
	steady_clock::time_point begin = steady_clock::now();
	thread t1(op2,pool, 0, 1000);
	thread t2(op2,pool, 1000, 2000);
	thread t3(op2,pool, 2000, 3000);
	thread t4(op2,pool, 3000, 4000);
	thread t5(op2,pool, 4000, 5000);
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	steady_clock::time_point end = steady_clock::now();
	auto length = end - begin;
	cout << "���ӳأ����߳���ʱ��" << length.count() << "����"
		<< length.count() / 1000000 << "����" << endl;
#endif
}


int query()
{

	MysqlConn conn;
	conn.connect("root", "123456", "My_sql", "192.168.28.128");
	string sql = "insert into score values (4, 'Tom', 100, 100, 98)";
	bool flag = conn.update(sql);
	cout << "falg value:" << flag<< endl;
	sql = "select * from score";
	conn.query(sql);
	while (conn.next())
	{
		cout << conn.value(0) << ","
			<< conn.value(1) << ","
			<< conn.value(2) << ","
			<< conn.value(3) << ","
			<< conn.value(4) << "," << endl;
	}
	return 0;
}

int main()
{

	test2();
	return 0;
}
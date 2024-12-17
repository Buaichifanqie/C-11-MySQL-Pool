#define _CRT_SECURE_NO_WARNINGS
#include<memory>
#include"MysqlConn.h"
#include"ConnectionPool.h"
//单线程 使用/不使用
//多线程 使用/不使用

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
	//非连接池，单线程用时：35485877700纳秒35485毫秒
	steady_clock::time_point begin= steady_clock::now();
	op1(0, 5000);
	steady_clock::time_point end= steady_clock::now();
	auto length = end - begin;
	cout << "非连接池，单线程用时：" << length.count() << "纳秒"
		<< length.count() / 1000000 << "毫秒" << endl;
#else
	//连接池，单线程用时：3664802300纳秒3664毫秒
	ConnectionPool* pool = ConnectionPool::getConnectionPool();
	steady_clock::time_point begin = steady_clock::now();
	op2(pool,0, 5000);
	steady_clock::time_point end = steady_clock::now();
	auto length = end - begin;
	cout << "连接池，单线程用时：" << length.count() << "纳秒"
		<< length.count() / 1000000 << "毫秒" << endl;


#endif
}

void test2()
{
#if 0
	MysqlConn conn;
	conn.connect("root", "123456", "My_sql", "192.168.28.128");
	//非连接池，多线程用时：7115655600纳秒7115毫秒
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
	cout << "非连接池，多线程用时：" << length.count() << "纳秒"
		<< length.count() / 1000000 << "毫秒" << endl;
#else
	//连接池，多线程用时：1282472300纳秒1282毫秒
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
	cout << "连接池，多线程用时：" << length.count() << "纳秒"
		<< length.count() / 1000000 << "毫秒" << endl;
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
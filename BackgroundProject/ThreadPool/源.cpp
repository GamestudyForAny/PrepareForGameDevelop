
#pragma once
#include <Windows.h>
#include <list>
#include <vector>
using std::list;
using std::vector;
/* ǰ������ */
class Task;
class ThreadPara;//���ݸ��̵߳Ĳ���
class ThreadPool
{
public:
	ThreadPool();
	ThreadPool(int maxNum);
	~ThreadPool();

	//�̳߳س�ʼ��
	void init();

	//�̳߳�����
	void release();
	//��������
	void addTask(Task* task);

	//�߳�������
	static void ThreadFunc(void* arg);
private:
	typedef list<Task*> TaskList;
	TaskList m_taskList;//δ����������
	HANDLE m_mutexTaskList;//�������
	HANDLE m_semNewTask;//�����������ź�
	HANDLE m_semDoneTask;//��������ź�
	typedef vector<ThreadPara> ThreadList;
	ThreadList m_threadList;//�̼߳���
	HANDLE m_mutexThreadPools;//�̳߳���
	int m_threadNum;//�߳���Ŀ
	HANDLE m_semDestoryThreadPool;//�̳߳��ͷ��ź�
	bool isInit;//�Ƿ��ʼ��
};


#include "ThreadPara.h"
#include <errno.h>
#include <process.h>
ThreadPool::ThreadPool() :m_threadNum(10), isInit(false)
{
}
ThreadPool::ThreadPool(int maxNum) {
	m_threadNum = maxNum;
}
ThreadPool::~ThreadPool()
{
}
void ThreadPool::init() {
	if (isInit) {
		return;
	}
	//�����ź�����������
	m_mutexTaskList = CreateMutex(NULL, FALSE, L"m_mutexTaskList");//�����true����ǰ�߳��Ѿ���ȡ��������һ��Ҫע��
	m_mutexThreadPools = CreateMutex(NULL, FALSE, L"m_mutexThreadPools");
	m_semNewTask = CreateSemaphore(NULL, 0, 100, L"m_semNewTask");
	m_semDoneTask = CreateSemaphore(NULL, 0, 100, L"m_semDoneTask");
	m_semDestoryThreadPool = CreateSemaphore(NULL, 0, 100, L"m_semDestoryThreadPool");

	if (m_mutexTaskList == 0||m_mutexThreadPools == 0
		 ||m_semNewTask == 0
		 ||m_semDoneTask == 0
		 ||m_semDestoryThreadPool == 0) {
		printf("Fatal Error:�����ź�������ʧ��!\n");
		return;
	}
	//�����߳�
	m_threadList.reserve(m_threadNum);
	for (int i = 0; i < m_threadNum; i++)
	{
		ThreadPara thradPara;
		thradPara.id = i + 1;
		thradPara.pool = this;
		thradPara.run = true;
		m_threadList.push_back(thradPara);
		ThreadPara* para = &m_threadList[i];
		m_threadList[i].hid = _beginthread(ThreadFunc, 0, (void*)para);
	}
}
void ThreadPool::addTask(Task* task)
{
	WaitForSingleObject(m_mutexTaskList, INFINITE);
	m_taskList.push_back(task);
	printf("add task%d\n", task->id);
	ReleaseMutex(m_mutexTaskList);
	//�ͷ��ź���
	ReleaseSemaphore(m_semNewTask, 1, NULL);
}

void ThreadPool::ThreadFunc(void* arg) {
	//������Ϣ
	ThreadPara* threadPara = (ThreadPara*)arg;
	ThreadPool* pool = threadPara->pool;
	printf("�߳�%d[%d]����,pool=%x...\n", threadPara->id, threadPara->hid, pool);
	//��Ҫ�ȴ����ź�
	HANDLE hWaitHandle[2];
	hWaitHandle[0] = pool->m_semDestoryThreadPool;
	hWaitHandle[1] = pool->m_semNewTask;
	while (threadPara->run)
	{
		//�ȴ��ź�
		DWORD val = WaitForMultipleObjects(2, hWaitHandle, false, INFINITE);

		if (val == WAIT_OBJECT_0) {//�̳߳�����
			printf("�߳�%d�˳�...\n", threadPara->id);
			break;
		}
		//����������
		WaitForSingleObject(pool->m_mutexTaskList, INFINITE);
		Task* task = *pool->m_taskList.begin();
		pool->m_taskList.pop_front();
		ReleaseMutex(pool->m_mutexTaskList);
		task->run();
		delete task;
		//�ͷ��ź���
		//ReleaseSemaphore(pool->m_semDoneTask, 1, NULL);
	}
}
void ThreadPool::release() {
	ReleaseSemaphore(m_semDestoryThreadPool, m_threadNum, NULL);
	m_threadList.clear();
	for (TaskList::iterator iter = m_taskList.begin(); iter != m_taskList.end(); iter++) {
		delete* iter;
	}
	m_taskList.clear();
	CloseHandle(m_mutexTaskList);
	CloseHandle(m_mutexThreadPools);
	CloseHandle(m_semDestoryThreadPool);
	CloseHandle(m_semDoneTask);
	CloseHandle(m_semNewTask);
}



#pragma once
class Task
{
public:
	Task(int id);
	~Task();
	virtual void run() = 0;
public:
	int id;
};

Task::Task(int id)
{
	this->id = id;
}

Task::~Task()
{
}







class MyTask : public Task
{
public:
	MyTask(int id) :Task(id) {}
	~MyTask() {}
	virtual void run() { printf("Task %d run...\n", id); Sleep(100); }
};

void main() {
	ThreadPool threadPool(10);
	threadPool.init();
	for (int i = 0; i < 20; i++) {
		Task* task = new MyTask(i + 1);
		threadPool.addTask(task);
	}
	Sleep(1000);
	threadPool.release();
	printf("release threadpool\n");
	getchar();
}


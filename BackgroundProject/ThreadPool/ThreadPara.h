#pragma once
class ThreadPool;
class ThreadPara {
public:
	int id;//�߳����
	int hid;//�߳̾����
	bool run;//���б�־
	ThreadPool* pool;//�̳߳�
public:
	ThreadPara() :id(0), hid(0), run(false), pool(nullptr) {}
	~ThreadPara() {}
};
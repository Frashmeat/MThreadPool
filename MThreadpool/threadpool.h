#pragma once
#include<iostream>
#include<queue>
#include<vector>
#include<mutex>
#include<thread>
#include<future>
#include<functional>

using namespace std;

class threadpool
{
public:
	//��ʼ���̳߳�
	threadpool(unsigned short size = 5);
	//�ͷ��̳߳�
	~threadpool();

	//�ύ������
	template<class F,class... Args>
	auto commit(F&& f, Args&&... args) -> future<decltype(f(args...))>;
	template<class F>
	auto commit(F&& f) -> future<decltype(f())>;
	template<class F>
	void commitNoReturn(F&& f);
private:
	//0.׼������
	unsigned short initSize;
	using Task = function<void()>;
	unsigned short maxSize = 50;
	//1.�����б�
	queue<Task> tasks;
	//2.�̳߳�
	vector<thread> pool;	
	//3.������
	mutex lock;
	//4.��������
	condition_variable thread_cv;
	//5.ʣ���ִ���߳���
	atomic<int> restThreadNum = 0;
	//6.�Ƿ������̳߳�
	atomic<bool> isRun = true;
	//����̳߳�
	void addThread(unsigned short size) {
		//��ʼ���̳߳��е��̳߳�
		for (; pool.size() < maxSize && size > 0; --size)
			//���ܶ������ֵ,�������size �� thread
		{
			pool.emplace_back([this] {
				while (true) {
					Task task;
					{	//�����
						unique_lock<mutex> lock{ this->lock };
						//��������
						thread_cv.wait(lock, [this] {
							//ֹͣ���� ���� �����б�Ϊ��
							return !this->isRun || !tasks.empty();
							});
						//ֹͣ���в���û��������Ҫִ��
						if (!isRun && tasks.empty())
							return;
						//�������б���ȡ��һ������ ��ʼִ��
						--restThreadNum;
						task = move(tasks.front());
						tasks.pop();
						//cout << this_thread::get_id << endl;
					}
					task();
					//ִ����ɻظ�ʣ���ִ���̸߳���
					{
						unique_lock<mutex> lock{ this->lock };
						restThreadNum++;
					}
				}

				});
			//��ʼ�� ��ִ���̸߳���
			{
				unique_lock<mutex> lock{ this->lock };
				restThreadNum++;
			}
		}
	};
	
};


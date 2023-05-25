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
	//初始化线程池
	threadpool(unsigned short size = 5);
	//释放线程池
	~threadpool();

	//提交任务函数
	template<class F,class... Args>
	auto commit(F&& f, Args&&... args) -> future<decltype(f(args...))>;
	template<class F>
	auto commit(F&& f) -> future<decltype(f())>;
	template<class F>
	void commitNoReturn(F&& f);
private:
	//0.准备类型
	unsigned short initSize;
	using Task = function<void()>;
	unsigned short maxSize = 50;
	//1.任务列表
	queue<Task> tasks;
	//2.线程池
	vector<thread> pool;	
	//3.互斥锁
	mutex lock;
	//4.条件变量
	condition_variable thread_cv;
	//5.剩余可执行线程数
	atomic<int> restThreadNum = 0;
	//6.是否运行线程池
	atomic<bool> isRun = true;
	//添加线程池
	void addThread(unsigned short size) {
		//初始化线程池中的线程池
		for (; pool.size() < maxSize && size > 0; --size)
			//不能多于最大值,逐个增加size 个 thread
		{
			pool.emplace_back([this] {
				while (true) {
					Task task;
					{	//添加锁
						unique_lock<mutex> lock{ this->lock };
						//阻塞在这
						thread_cv.wait(lock, [this] {
							//停止运行 或者 任务列表不为空
							return !this->isRun || !tasks.empty();
							});
						//停止运行并且没有任务需要执行
						if (!isRun && tasks.empty())
							return;
						//在任务列表中取出一个任务 开始执行
						--restThreadNum;
						task = move(tasks.front());
						tasks.pop();
						//cout << this_thread::get_id << endl;
					}
					task();
					//执行完成回复剩余可执行线程个数
					{
						unique_lock<mutex> lock{ this->lock };
						restThreadNum++;
					}
				}

				});
			//初始化 可执行线程个数
			{
				unique_lock<mutex> lock{ this->lock };
				restThreadNum++;
			}
		}
	};
	
};


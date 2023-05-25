#include "threadpool.h"
inline threadpool::threadpool(unsigned short size)
{
	this->initSize = size;
	addThread(size);
}

inline threadpool::~threadpool()
{
	//停止运行
	isRun = false;
	//唤醒所有线程
	thread_cv.notify_all();

	for (thread& t : pool) {
		/*if (t.joinable()) {
			t.detach();
		}*/ // 不等线程执行完,直接结束
		if (t.joinable()) {
			t.join();	//等待所有任务被执行
		}
	}
}





//F 为函数  Args 为参数列表
template<class F, class ...Args>
 auto threadpool::commit(F&& f, Args&& ...args) -> future<decltype(f(args ...))> // 返回值为一个future
{
	 if (!isRun) throw runtime_error("commit on ThreadPool is stopped.");
	 //准备返回值类型
	 using RetType = decltype(f(args...));
	 //开始包装 任务 创建在堆区
	 auto task = make_shared<packaged_task<RetType()>>(
		 bind(forward<F>(f), forward<Args>(args)...)
	 );
	 //获取该task 的future 作为commit 的返回值
	 future<RetType> future = task->get_future();
	 //将任务提交到任务列表
	 {
		 lock_guard<mutex> lock{ this->lock };
		 tasks.emplace([task] {
			 (*task)();
			 });
	 }
	 //利用一个环境变量来通知线程池开始工作
	 thread_cv.notify_one();
	 return future;
}

 template<class F>
 auto threadpool::commit(F&& f) -> future<decltype(f())>
 {
	 if (!isRun) throw runtime_error("not able to run");
	 using RetTypeNoArgs = decltype(f());
	 auto task = make_shared<packaged_task<RetTypeNoArgs()>>(forward<F>(f));
	 future<RetTypeNoArgs> future = task->get_future();
	 {
		 lock_guard<mutex> lock{ this->lock };
		 tasks.emplace([task]{
			 (*task)();
			 });
		
	 }
	 thread_cv.notify_one();
	 return future;
 }

 template<class F>
 void threadpool::commitNoReturn(F&& f)
 {
	 if (!isRun) throw runtime_error("not able to run");
	 {
		 lock_guard<mutex> lock{ this->lock };
		 tasks.emplace(forward<F>(f));
	 }
	 thread_cv.notify_one();
 }

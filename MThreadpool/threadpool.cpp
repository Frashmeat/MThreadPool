#include "threadpool.h"
inline threadpool::threadpool(unsigned short size)
{
	this->initSize = size;
	addThread(size);
}

inline threadpool::~threadpool()
{
	//ֹͣ����
	isRun = false;
	//���������߳�
	thread_cv.notify_all();

	for (thread& t : pool) {
		/*if (t.joinable()) {
			t.detach();
		}*/ // �����߳�ִ����,ֱ�ӽ���
		if (t.joinable()) {
			t.join();	//�ȴ���������ִ��
		}
	}
}





//F Ϊ����  Args Ϊ�����б�
template<class F, class ...Args>
 auto threadpool::commit(F&& f, Args&& ...args) -> future<decltype(f(args ...))> // ����ֵΪһ��future
{
	 if (!isRun) throw runtime_error("commit on ThreadPool is stopped.");
	 //׼������ֵ����
	 using RetType = decltype(f(args...));
	 //��ʼ��װ ���� �����ڶ���
	 auto task = make_shared<packaged_task<RetType()>>(
		 bind(forward<F>(f), forward<Args>(args)...)
	 );
	 //��ȡ��task ��future ��Ϊcommit �ķ���ֵ
	 future<RetType> future = task->get_future();
	 //�������ύ�������б�
	 {
		 lock_guard<mutex> lock{ this->lock };
		 tasks.emplace([task] {
			 (*task)();
			 });
	 }
	 //����һ������������֪ͨ�̳߳ؿ�ʼ����
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

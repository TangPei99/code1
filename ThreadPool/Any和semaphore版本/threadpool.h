#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <unordered_map>


enum class PoolMode
{
	MODE_FIXED, //fix
	MODE_CACHED, //cache
};


// Any类型：这里是自己实现的Any，也可以使用c++17中的Any
class Any
{
public:
	Any() = default;
	~Any() = default;
	Any(const Any&) = delete;
	Any& operator=(const Any&) = delete;
	Any(Any&&) = default;
	Any& operator=(Any&&) = default;

	template<typename T>  // T:int    Derive<int>
	Any(T data) : base_(std::make_unique<Derive<T>>(data))
	{}

	template<typename T>
	T cast_()
	{
		Derive<T>* pd = dynamic_cast<Derive<T>*>(base_.get());
		if (pd == nullptr)
		{
			throw "type is unmatch!";
		}
		return pd->data_;
	}
private:
	class Base
	{
	public:
		virtual ~Base() = default;
	};

	template<typename T>
	class Derive : public Base
	{
	public:
		Derive(T data) : data_(data) 
		{}
		T data_; 
	};

private:
	std::unique_ptr<Base> base_;
};

// 实现一个信号量类，也可以使用C++20中的Semaphore
class Semaphore
{
public:
	Semaphore(int limit = 0) 
		:resLimit_(limit)
	{}
	~Semaphore() = default;

	void wait()
	{
		std::unique_lock<std::mutex> lock(mtx_);
		cond_.wait(lock, [&]()->bool {return resLimit_ > 0; });
		resLimit_--;
	}

	void post()
	{
		std::unique_lock<std::mutex> lock(mtx_);
		resLimit_++;
		cond_.notify_all();  
	}
private:
	int resLimit_;
	std::mutex mtx_;
	std::condition_variable cond_;
};

class Task;

// 实现接收提交到线程池的task任务执行完成后的返回值类型Result，可以通过Result获取任务返回值
class Result
{
public:
	Result(std::shared_ptr<Task> task, bool isValid = true);
	~Result() = default;

	void setVal(Any any);

	Any get();
private:
	Any any_; 
	Semaphore sem_; 
	std::shared_ptr<Task> task_;  
	std::atomic_bool isValid_; 
};

class Task
{
public:
	Task();
	~Task() = default;
	void exec();
	void setResult(Result* res);

	virtual Any run() = 0;

private:
	Result* result_;
};


class Thread
{
public:
	using ThreadFunc = std::function<void(int)>;

	Thread(ThreadFunc func);
	~Thread();
	void start();

	int getId()const;
private:
	ThreadFunc func_;
	static int generateId_;
	int threadId_; 
};

class ThreadPool
{
public:
	ThreadPool();

	~ThreadPool();

	void setMode(PoolMode mode);

	void setTaskQueMaxThreshHold(int threshhold);

	void setThreadSizeThreshHold(int threshhold);

	Result submitTask(std::shared_ptr<Task> sp);

	void start(int initThreadSize = std::thread::hardware_concurrency());

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

private:

	bool checkRunningState() const;

	void threadFunc(int threadid);


private:
	std::unordered_map<int, std::unique_ptr<Thread>> threads_; // 线程列表

	int initThreadSize_;  // 初始线程数量
	int threadSizeThreshHold_; // 线程数量上限阈值
	std::atomic_int curThreadSize_;	// 记录当前线程池里面线程的总数量
	std::atomic_int idleThreadSize_; // 记录空闲线程的数量

	std::queue<std::shared_ptr<Task>> taskQue_; // 任务队列
	std::atomic_int taskSize_; // 任务的数量
	int taskQueMaxThreshHold_;  // 任务队列数量上限阈值

	std::mutex taskQueMtx_; // 保证任务队列的线程安全
	std::condition_variable notFull_; // 表示任务队列不满
	std::condition_variable notEmpty_; // 表示任务队列不空
	std::condition_variable exitCond_; // 等到线程资源全部回收

	PoolMode poolMode_; // 当前线程池的工作模式
	std::atomic_bool isPoolRunning_; // 表示当前线程池的启动状态
};

#endif

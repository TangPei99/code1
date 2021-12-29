#include "DataThread.h"
void DataThread::Push(Data d)
{
	_mutex.lock();
	if (datas.size() > _maxList)
	{
		datas.front().Drop();
		datas.pop_front();
	}
	datas.push_back(d);
	_mutex.unlock();
}

Data DataThread::Pop()
{
	_mutex.lock();
	if (datas.empty())
	{
		_mutex.unlock();
		return Data();
	}
	Data d = datas.front();
	datas.pop_front();
	_mutex.unlock();
	return d;
}

bool DataThread::Start()
{
	_isExit = false;
	QThread::start();
	return true;
}

void DataThread::Stop()
{
	_isExit = true;
	wait();
}

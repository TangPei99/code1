#pragma once
#include <QThread>
#include <QMutex>

#include "Data.h"

class DataThread : public QThread
{
public:
	int _maxList = 100;

	virtual void Push(Data d);

	virtual Data Pop();

	virtual bool Start();

	virtual void Stop();

	DataThread() = default;
	virtual ~DataThread() = default;
protected:
	std::list<Data> datas;

	int _dataCount = 0;
	QMutex _mutex;

	bool _isExit = false;
};


#include "Filter.h"
#include "BilateraFilter.h"
#include <QDebug>

Filter* Filter::Get(FilterType type)
{
	static BilateraFilter bf;
	switch (type)
	{
	case BILATERAL:
		return &bf;
		break;
	default:
		break;
	}
	return nullptr;
}


bool Filter::Set(std::string key, double value)
{
	if (paras.find(key) == paras.end())
	{
		qDebug() << "para " << key.c_str() << "is not support";
		return false;
	}
	paras[key] = value;
}

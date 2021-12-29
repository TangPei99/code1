#include "Data.h"
#include <stdlib.h>
#include <string.h>

void Data::Drop()
{
	if (_data)
	{
		delete[] _data;
		_data = nullptr;
	}
	_size = 0;
}

Data::Data(char *data, int size,int64_t pts)
{
	_data = new char[size];
	memcpy(_data,data,size);
	_size = size;
	_pts = pts;
}

Data::~Data()
{
	
}


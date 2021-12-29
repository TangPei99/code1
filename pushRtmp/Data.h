#pragma once
#include <stdint.h>

class Data
{
public:
	char *_data = nullptr;
	int _size = 0;
	int64_t _pts = 0;
	void Drop();
	Data() = default;
	Data(char *data, int size, int64_t pts);
	~Data();
};


#pragma once
#include "DataThread.h"
#include <opencv2/highgui.hpp>
#include <vector>

#pragma comment(lib,"opencv_world320.lib")

class Filter;

class VideoCapture : public DataThread
{
public:
	int _width = 0;
	int _height = 0;
	int _fps = 0;
	cv::VideoCapture _cam;

	bool Init(int camIndex = 0);
	bool Init(const char *url);
	void Stop();
	void run() override;
	void addFilter(Filter* filter);
	~VideoCapture() = default;
	VideoCapture() = default;
protected:
	QMutex _fMutex;
	std::vector<Filter*> _filters;
};


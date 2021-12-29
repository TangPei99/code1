#include "VideoCapture.h"
#include "Filter.h"
#include <QDebug>
extern "C"
{
#include <libavutil/time.h>
}
#pragma comment(lib, "avutil.lib")

bool VideoCapture::Init(int camIndex)
{
	_cam.open(camIndex);
	if (!_cam.isOpened())
	{
		qDebug() << "cam open failed!";
		return false;
	}
	_width = _cam.get(cv::CAP_PROP_FRAME_WIDTH);
	_height = _cam.get(cv::CAP_PROP_FRAME_HEIGHT);
	_fps = _cam.get(cv::CAP_PROP_FPS);
	if (_fps == 0) _fps = 10;
	return true;
}

bool VideoCapture::Init(const char *url)
{
	_cam.open(url);
	if (!_cam.isOpened())
	{
		qDebug() << "cam open failed!";
		return false;
	}
	_width = _cam.get(cv::CAP_PROP_FRAME_WIDTH);
	_height = _cam.get(cv::CAP_PROP_FRAME_HEIGHT);
	_fps = _cam.get(cv::CAP_PROP_FPS);
	if (_fps == 0) _fps = 25;
	return true;
}

void VideoCapture::Stop()
{
	DataThread::Stop();
	if (_cam.isOpened())
	{
		_cam.release();
	}
}

void VideoCapture::run()
{
	cv::Mat frame;
	while (!_isExit)
	{
		if (!_cam.read(frame))
		{
			msleep(1);
			continue;
		}
		if (frame.empty())
		{
			msleep(1);
			continue;
		}
		int64_t time = av_gettime();

		_fMutex.lock();
		for (Filter* f:_filters)
		{
			cv::Mat des;
			f->filter(&frame, &des);
			frame = des;
		}
		_fMutex.unlock();

		Data d((char*)frame.data, frame.cols*frame.rows*frame.elemSize(),time);
		Push(d);
	}
}

void VideoCapture::addFilter(Filter* filter)
{
	_fMutex.lock();
	_filters.push_back(filter);
	_fMutex.unlock();
}



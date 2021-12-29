#pragma once
#include "DataThread.h"

class AudioRecord;
class VideoCapture;
class MediaEncode;
class Rtmp;

class Controller :public DataThread
{
public:
	std::string _outUrl;
	int _camIndex = -1;
	std::string _inUrl = "";
	std::string _err = "";

	static Controller *Get()
	{
		static Controller xc;
		return &xc;
	}
	
	virtual bool Set(std::string key, double val);
	virtual bool Start();
	virtual void Stop();
	void run();
	virtual ~Controller();
protected:
	int _vindex = -1; 
	int _aindex = -1;
	AudioRecord* _audioRecord = nullptr;
	VideoCapture* _videoCapture = nullptr;
	MediaEncode* _mediaEncode = nullptr;
	Rtmp* _rtmp = nullptr;
	int64_t _startTime = 0;
	Controller();
};


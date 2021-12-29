#include "Controller.h"

#include "VideoCapture.h"
#include "AudioRecord.h"
#include "MediaEncode.h"
#include "Rtmp.h"
#include "Filter.h"
#include <iostream>
#include <QDebug>
extern "C"
{
#include <libavutil/time.h>
#include <libavformat/avformat.h>
}


using namespace std;

void Controller::run()
{
	while (!_isExit)
	{
		Data ad = _audioRecord->Pop();
		Data vd = _videoCapture->Pop();
		if (ad._size <= 0 && vd._size <= 0)
		{
			QThread::msleep(1);
			continue;
		}

		if (ad._size > 0)
		{
			Data frame = _mediaEncode->Resample(ad);
			ad.Drop();
			frame._pts = frame._pts - _startTime;
			Data pkt = _mediaEncode->EncodeAudio(frame);
			_rtmp->SendFrame(pkt, _aindex);
		}

		if (vd._size > 0)
		{
			Data frame = _mediaEncode->RGBToYUV(vd);
			vd.Drop();
			frame._pts = frame._pts - _startTime;
			Data pkt = _mediaEncode->EncodeVideo(frame);

			_rtmp->SendFrame(pkt, _vindex);
		}
	}
}


bool Controller::Set(std::string key, double val)
{
	Filter::Get()->Set(key, val);
	return true;
}

bool Controller::Start()
{
	int sampleRate = 44100;
	int channels = 2;
	int sampleByte = 2;
	int nbSample = 1024;
	AVSampleFormat inSampleFmt = AV_SAMPLE_FMT_S16;
	AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_FLTP;

	_startTime = MediaEncode::getCurrentTime();

	//打开相机
	_videoCapture = new VideoCapture();
	Filter* filter = Filter::Get();
	filter->Set("d", 15);
	_videoCapture->addFilter(filter);

	if (!_videoCapture->Init(0))
	{
		_err = "open camera failed!";
		return false;
	}
	_videoCapture->Start();

	_audioRecord = new AudioRecord();
	_audioRecord->_sampleRate = sampleRate;
	_audioRecord->_channels = channels;
	_audioRecord->_sampleByte = sampleByte;
	_audioRecord->_nbSamples = nbSample;
	if (!_audioRecord->Init())
	{
		_err = "audioRecord->Init";
		return false;
	}
	_audioRecord->Start();

	_mediaEncode = new MediaEncode();

	_mediaEncode->_inWidth = _videoCapture->_width;
	_mediaEncode->_inHeight = _videoCapture->_height;
	_mediaEncode->_outWidth = _videoCapture->_width;
	_mediaEncode->_outHeight = _videoCapture->_height;
	if (!_mediaEncode->InitScale())
	{
		_err = "mediaEncode->InitScale";
		return false;
	}
	_mediaEncode->_channels = channels;
	_mediaEncode->_nbSample = nbSample;
	_mediaEncode->_sampleRate = sampleRate;
	_mediaEncode->_inSampleFmt = AV_SAMPLE_FMT_S16;
	_mediaEncode->_outSampleFmt = AV_SAMPLE_FMT_FLTP;
	if (!_mediaEncode->InitResample())
	{
		_err = "mediaEncode->InitResample";
		return false;
	}
	if (!_mediaEncode->InitAudioCodec())
	{
		_err = "mediaEncode->InitAudioCodec";
		return false;
	}

	if (!_mediaEncode->InitVideoCodec())
	{
		_err = "mediaEncode->InitVideoCodec";
		return false;
	}


	_rtmp = new Rtmp();
	if (!_rtmp->Init(_outUrl.c_str()))
	{
		_err = "rtmp->Init";
		return false;
	}

	_vindex = _rtmp->AddStream(_mediaEncode->_vCodecCtx);
	if (_vindex < 0)
	{
		_err = "rtmp->AddStream video";
		return false;
	}

	_aindex = _rtmp->AddStream(_mediaEncode->_aCodecCtx);
	if (_aindex < 0)
	{
		_err = "rtmp->AddStream audio";
		return false;
	}

	if (!_rtmp->SendHead())
	{
		_err = "rtmp->SendHead";
		return false;
	}
	DataThread::Start();
	return true;
}

void Controller::Stop()
{
	DataThread::Stop();
	_audioRecord->Stop();
	_videoCapture->Stop();
	_mediaEncode->close();
	_rtmp->close();
	_camIndex = -1;
	_inUrl = "";
	return;
}

Controller::Controller()
{
}

Controller::~Controller()
{
	if(_audioRecord) delete _audioRecord;
	if (_videoCapture) delete _videoCapture;
	if (_mediaEncode) delete _mediaEncode;
	if (_rtmp) delete _rtmp;
}

#pragma once
#include "Data.h"

struct AVCodecContext;
struct SwsContext;
struct SwrContext;
struct AVFrame;
struct AVPacket;
enum AVSampleFormat;

class MediaEncode
{
public:
	~MediaEncode();
	MediaEncode();

	bool InitScale();
	bool InitResample();

	Data RGBToYUV(Data data);
	Data Resample(Data data);

	bool InitVideoCodec();
	bool InitAudioCodec();

	Data EncodeVideo(Data data);
	Data EncodeAudio(Data data);

	void close();

	static int64_t getCurrentTime();

	//video parameter
	int _inWidth = 1280;
	int _inHeight = 720;
	int _inPixSize = 3;

	int _outWidth = 1280;
	int _outHeight = 720;
	int _bitrate = 400000;//50kB
	int _fps = 10;
	//audio parameter
	int _channels = 2;
	int _sampleRate = 44100;
	//AV_SAMPLE_FMT_S16
	AVSampleFormat _inSampleFmt = (AVSampleFormat)1;
	int _nbSample = 1024;
	//AV_SAMPLE_FMT_FLTP
	AVSampleFormat _outSampleFmt = (AVSampleFormat)8;

	AVCodecContext* _vCodecCtx = nullptr;
	AVCodecContext* _aCodecCtx = nullptr;

private:

	
	SwsContext* _swsCtx = nullptr;
	SwrContext* _swrCtx = nullptr;
	AVFrame* _vFrame = nullptr;
	AVPacket* _vPkt = nullptr;
	AVFrame* _aFrame = nullptr;
	AVPacket* _aPkt = nullptr;
	int _vPts = 0;
	int _aPts = 0;
	int _vLastPts = -1;
	int _aLastPts = -1;
};


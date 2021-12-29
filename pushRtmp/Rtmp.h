#pragma once
#include <string>
#include "Data.h"

struct AVCodecContext;
struct AVFormatContext;
struct AVStream;

class Rtmp
{
public:
	bool Init(const char *url);

	int AddStream(const AVCodecContext *c);

	bool SendHead();

	bool SendFrame(Data pkt,int streamIndex);

	void close();

	~Rtmp();

	Rtmp();

private:
	AVFormatContext *_fmtCtx = nullptr;

	const AVCodecContext *_vCodecCtx = nullptr;
	const AVCodecContext *_aCodecCtx = nullptr;

	AVStream *_vStream = nullptr;
	AVStream *_aStream = nullptr;

	std::string _outUrl = "";
};


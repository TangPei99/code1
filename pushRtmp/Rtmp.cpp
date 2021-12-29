#include "Rtmp.h"
#include <iostream>
#include <string>
#include <QDebug>
using namespace std;
extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}
#pragma comment(lib, "avformat.lib")


bool Rtmp::Init(const char *url)
{
	int ret = avformat_alloc_output_context2(&_fmtCtx, 0, "flv", url);
	this->_outUrl = url;
	if (ret != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(ret, buf, sizeof(buf) - 1);
		cout << buf;
		return false;
	}
	return true;
}

int Rtmp::AddStream(const AVCodecContext *c)
{
	if (!c)return false;

	AVStream *stream = avformat_new_stream(_fmtCtx, NULL);
	if (!stream)
	{
		qDebug() << "avformat_new_stream failed" << endl;
		return -1;
	}
	stream->codecpar->codec_tag = 0;
	
	avcodec_parameters_from_context(stream->codecpar, c);
	av_dump_format(_fmtCtx, 0, _outUrl.c_str(), 1);

	if (c->codec_type == AVMEDIA_TYPE_VIDEO)
	{
		_vCodecCtx = c;
		_vStream = stream;
	}
	if (c->codec_type == AVMEDIA_TYPE_AUDIO)
	{
		_aCodecCtx = c;
		_aStream = stream;
	}

	return stream->index;
}

bool Rtmp::SendHead()
{
	int ret = avio_open(&_fmtCtx->pb, _outUrl.c_str(), AVIO_FLAG_WRITE);
	if (ret != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(ret, buf, sizeof(buf) - 1);
		cout << buf << endl;
		return false;
	}

	ret = avformat_write_header(_fmtCtx, NULL);
	if (ret != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(ret, buf, sizeof(buf) - 1);
		cout << buf << endl;
		return false;
	}
	return true;
}

bool Rtmp::SendFrame(Data data,int streamIndex)
{
	if (data._size <= 0 || !data._data) return false;
	AVPacket* pkt = (AVPacket*)data._data;

	pkt->stream_index = streamIndex;
	pkt->pts = data._pts;
	AVRational codecTimeBase;
	AVRational streamTimeBase;

	if (_aCodecCtx && _aStream && pkt->stream_index == _aStream->index)
	{
		codecTimeBase = _aCodecCtx->time_base;
		streamTimeBase = _aStream->time_base;
	}
	else if(_vCodecCtx && _vStream && pkt->stream_index == _vStream->index)
	{
		codecTimeBase = _vCodecCtx->time_base;
		streamTimeBase = _vStream->time_base;
	}
	else
	{
		return false; 
	}
	pkt->pts = av_rescale_q(pkt->pts, codecTimeBase, streamTimeBase);
	pkt->dts = av_rescale_q(pkt->dts, codecTimeBase, streamTimeBase);
	pkt->duration = av_rescale_q(pkt->duration, codecTimeBase, streamTimeBase);
	int ret = av_interleaved_write_frame(_fmtCtx, pkt);
	if (ret == 0)
	{
		return true;
	}
	return false;
}

Rtmp::~Rtmp()
{
	this->close();
}

Rtmp::Rtmp()
{

}

void Rtmp::close()
{
	if (_fmtCtx)
	{
		avformat_free_context(_fmtCtx);
		_fmtCtx = nullptr;
	}
	_vStream = nullptr;
	_vCodecCtx = nullptr;

	_aStream = nullptr;
	_aCodecCtx = nullptr;
	_outUrl = "";
}

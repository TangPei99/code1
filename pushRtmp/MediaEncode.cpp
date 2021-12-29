#include "MediaEncode.h"

extern "C"
{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
}

#include <QDeBug>

#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib,"swresample.lib")

#if defined WIN32 || defined _WIN32
#include <windows.h>
#endif

static int GetCpuNum()
{
#if defined WIN32 || defined _WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	return (int)sysinfo.dwNumberOfProcessors;
#elif defined __linux__
	return (int)sysconf(_SC_NPROCESSORS_ONLN);
#elif defined __APPLE__
	int numCPU = 0;
	int mib[4];
	size_t len = sizeof(numCPU);

	// set the mib for hw.ncpu
	mib[0] = CTL_HW;
	mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;

						   // get the number of CPUs from the system
	sysctl(mib, 2, &numCPU, &len, NULL, 0);

	if (numCPU < 1)
	{
		mib[1] = HW_NCPU;
		sysctl(mib, 2, &numCPU, &len, NULL, 0);

		if (numCPU < 1)
			numCPU = 1;
	}
	return (int)numCPU;
#else
	return 1;
#endif
}


MediaEncode::~MediaEncode()
{
	this->close();
}

MediaEncode::MediaEncode()
{

}


bool MediaEncode::InitScale()
{
	_swsCtx = sws_getCachedContext(_swsCtx,
		_inWidth, _inHeight, AV_PIX_FMT_BGR24,
		_outWidth, _outHeight, AV_PIX_FMT_YUV420P,
		SWS_BICUBIC, 
		0, 0, 0
	);
	if (!_swsCtx)
	{
		return false;
	}
	
	_vFrame = av_frame_alloc();
	_vFrame->format = AV_PIX_FMT_YUV420P;
	_vFrame->width = _inWidth;
	_vFrame->height = _inHeight;
	_vFrame->pts = 0;
	
	int ret = av_frame_get_buffer(_vFrame, 32);
	if (ret != 0)
	{
		return false;
	}
	return true;
}

Data MediaEncode::RGBToYUV(Data data)
{
	Data d;
	if (data._data == nullptr || data._size <= 0) return d;
	uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };
	indata[0] = (uint8_t*)data._data;
	int insize[AV_NUM_DATA_POINTERS] = { 0 };
	insize[0] = _inWidth * _inPixSize;

	int h = sws_scale(_swsCtx, indata, insize, 0, _inHeight,
		_vFrame->data, _vFrame->linesize);
	if (h <= 0)
	{
		return d;
	}
	d._data = (char*)_vFrame;
	int i = 0;
	while (_vFrame->data[i])
	{
		d._size += _vFrame->linesize[i];
		i++;
	}
	d._pts = data._pts;
	return d;
}

bool MediaEncode::InitVideoCodec()
{
	AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	_vPkt = av_packet_alloc();

	if (!codec)
	{
		return false;
	}
	_vCodecCtx = avcodec_alloc_context3(codec);
	if (!_vCodecCtx)
	{
		return false;
	}
	_vCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	_vCodecCtx->codec_id = codec->id;
	_vCodecCtx->thread_count = GetCpuNum();

	_vCodecCtx->bit_rate = 50 * 1024 * 8;
	_vCodecCtx->width = _outWidth;
	_vCodecCtx->height = _outHeight;
	_vCodecCtx->time_base = { 1,1000000 };
	_vCodecCtx->framerate = { _fps,1 };

	_vCodecCtx->gop_size = 50;
	_vCodecCtx->max_b_frames = 0;
	_vCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

	int ret = avcodec_open2(_vCodecCtx, 0, 0);
	if (ret != 0)
	{
		return false;
	}
	return true;
}

Data MediaEncode::EncodeVideo(Data data)
{
	Data d;
	if (data._data == nullptr || data._size <= 0) return d;

	av_packet_unref(_vPkt);
	AVFrame* frame = (AVFrame*)data._data;

	frame->pts = data._pts;
	int ret = avcodec_send_frame(_vCodecCtx, frame);
	if (ret != 0)
		return d;

	ret = avcodec_receive_packet(_vCodecCtx, _vPkt);
	if (ret != 0 || _vPkt->size <= 0)
		return d;


	//if (_vPkt->pts == _vLastPts)
	//{
	//	_vPkt->pts += 1000;
	//}
	//_vLastPts = _vPkt->pts;

	d._data = (char*)_vPkt;
	d._size = _vPkt->size;
	d._pts = _vPkt->pts;
	return d;
}

void MediaEncode::close()
{
	if (_vCodecCtx) avcodec_free_context(&_vCodecCtx);
	if (_aCodecCtx) avcodec_free_context(&_aCodecCtx);
	if (_swsCtx)
	{
		sws_freeContext(_swsCtx);
		_swsCtx = nullptr;
	}
	if (_swrCtx)
	{
		swr_free(&_swrCtx);
	}
	if (_vFrame)
	{
		av_frame_free(&_vFrame);
	}
	if (_vPkt)
	{
		av_packet_unref(_vPkt);
		av_packet_free(&_vPkt);
	}
	if (_aFrame)
	{
		av_frame_free(&_aFrame);
	}
	if (_aPkt)
	{
		av_packet_unref(_aPkt);
		av_packet_free(&_aPkt);
	}
	_vPts = 0;
	_aPts = 0;

	_vLastPts = -1;
	_aLastPts = -1;
}

Data MediaEncode::Resample(Data data)
{
	Data d;
	if (data._data == nullptr || data._size <= 0) return d;

	const uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };
	indata[0] = (uint8_t *)data._data;
	int len = swr_convert(_swrCtx, _aFrame->data, _aFrame->nb_samples,
		indata, _aFrame->nb_samples);
	if (len <= 0)
	{
		return d;
	}

	d._data = (char*)_aFrame;
	int i = 0;
	while (_vFrame->data[i])
	{
		d._size += _vFrame->linesize[i];
		i++;
	}
	d._pts = data._pts;
	return d;
}

bool MediaEncode::InitResample()
{
	_swrCtx = swr_alloc_set_opts(_swrCtx,
		av_get_default_channel_layout(_channels), _outSampleFmt, _sampleRate,
		av_get_default_channel_layout(_channels), _inSampleFmt, _sampleRate,
		0, 0);
	if (!_swrCtx) return false;


	int ret = swr_init(_swrCtx);
	if (ret != 0)
	{
		char buf[1024]{ 0 };
		av_strerror(ret, buf, sizeof(buf) - 1);
		qDebug() << "swr_init failed" << buf;
		return false;
	}


	_aFrame = av_frame_alloc();
	if (!_aFrame) return false;
	_aFrame->format = _outSampleFmt;
	_aFrame->sample_rate = _sampleRate;
	_aFrame->channels = _channels;
	_aFrame->channel_layout = av_get_default_channel_layout(_channels);
	_aFrame->nb_samples = _nbSample;

	ret = av_frame_get_buffer(_aFrame, 0);
	if (ret != 0)
	{
		char buf[1024]{ 0 };
		av_strerror(ret, buf, sizeof(buf) - 1);
		qDebug() << "av_frame_get_buffer failed" << buf;
		return false;
	}
	return true;
}

bool MediaEncode::InitAudioCodec()
{
	_aPkt = av_packet_alloc();
	if (!_aPkt) return false;


	AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (codec == nullptr)
	{
		qDebug() << "avcodec_find_encoder failed";
		return false;
	}

	_aCodecCtx = avcodec_alloc_context3(codec);
	if (_aCodecCtx == nullptr)
	{
		qDebug() << "avcodec_alloc_context3 failed";
		return false;
	}
	_aCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	_aCodecCtx->thread_count = 8;
	_aCodecCtx->bit_rate = 40000;
	_aCodecCtx->sample_rate = _sampleRate;
	_aCodecCtx->sample_fmt = _outSampleFmt;
	_aCodecCtx->channels = _channels;
	_aCodecCtx->channel_layout = av_get_default_channel_layout(_channels);
	_aCodecCtx->time_base = { 1,1000000 };

	int ret = avcodec_open2(_aCodecCtx, nullptr, nullptr);
	if (ret != 0)
	{
		char buf[1024]{ 0 };
		av_strerror(ret, buf, sizeof(buf) - 1);
		qDebug() << "avcodec_open2 failed" << buf;
		return false;
	}
	return true;
}

Data MediaEncode::EncodeAudio(Data data)
{
	Data d;
	if (data._data == nullptr || data._size <= 0) return d;
	AVFrame* frame = (AVFrame*)data._data;
	frame->pts = data._pts;
	//qDebug() << _aLastPts << frame->pts;
	//if (frame->pts <= _aLastPts + 100/*23210*/) qDebug() << "###";
	_aLastPts = frame->pts;
	int ret = avcodec_send_frame(_aCodecCtx, frame);
	if (ret != 0)
	{
		return d;
	}
	av_packet_unref(_aPkt);
	ret = avcodec_receive_packet(_aCodecCtx, _aPkt);
	if (ret != 0)
	{
		return d;
	}

	d._data = (char*)_aPkt;
	d._size = _aPkt->size;
	d._pts = _aPkt->pts;
	return d;
}

int64_t MediaEncode::getCurrentTime()
{
	return av_gettime();
}

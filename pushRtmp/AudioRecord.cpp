#include "AudioRecord.h"

#include <QAudioInput>
#include <QIODevice>
#include <QDebug>
extern "C"
{
#include <libavutil/time.h>
}
#pragma comment(lib, "avutil.lib")

bool AudioRecord::Init()
{
	Stop();
	QAudioFormat fmt;
	fmt.setSampleRate(_sampleRate);
	fmt.setChannelCount(_channels);
	fmt.setSampleSize(_sampleByte * 8);
	fmt.setCodec("audio/pcm");
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::UnSignedInt);
	QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
	if (!info.isFormatSupported(fmt))
	{
		qDebug() << "Audio format not support!";
		fmt = info.nearestFormat(fmt);
	}
	_input = new QAudioInput(fmt);

	_io = _input->start();
	if (!_io)
		return false;
	return true;
}

void AudioRecord::Stop()
{
	DataThread::Stop();
	if (_input)
		_input->stop();
	if (_io)
		_io->close();
	_input = nullptr;
	_io = nullptr;
}

void AudioRecord::run()
{
	int readSize = _nbSamples * _channels*_sampleByte;
	char *buf = new char[readSize];
	while (!_isExit)
	{
		if (_input->bytesReady() < readSize)
		{
			QThread::msleep(1);
			continue;
		}

		int size = 0;
		while (size != readSize)
		{
			int len = _io->read(buf + size, readSize - size);
			if (len < 0)break;
			size += len;
		}
		if (size != readSize)
		{
			continue;
		}
		/*uint64_t time = av_gettime();
		qDebug() << time;*/
		Push(Data(buf, readSize, av_gettime()));
	}
	delete[] buf;
}
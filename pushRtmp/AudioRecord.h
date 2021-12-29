#pragma once

#include "DataThread.h"

class QIODevice;
class QAudioInput;

class AudioRecord : public DataThread
{
public:
	int _channels = 2;		
	int _sampleRate = 44100;
	int _sampleByte = 2;
	int _nbSamples = 1024;

	bool Init();
	void Stop();
	void run() override;
	AudioRecord() = default;
	~AudioRecord() = default;

private:
	QIODevice* _io = nullptr;
	QAudioInput* _input = nullptr;
};


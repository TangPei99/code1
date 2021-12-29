#include <QtCore/QCoreApplication>
#include <iostream>
#include <QThread>
#include "MediaEncode.h"
#include "Rtmp.h"
#include "AudioRecord.h"
#include "VideoCapture.h"
#include "Filter.h"
#include "Controller.h"
using namespace std;
int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	Controller* control = Controller::Get();
	//control->_inUrl = "rtsp://xxx"
	control->_camIndex = 0; //指定使用默认摄像头
	control->_outUrl = "rtmp://192.168.2.211/live"; //指定rtmp服务器地址
	if (control->Start() == false) qDebug() << control->_err.c_str(); //返回true代表开始推流

	return a.exec();
}

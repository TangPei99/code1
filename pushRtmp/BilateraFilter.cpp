#include "BilateraFilter.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

bool BilateraFilter::filter(cv::Mat* src, cv::Mat* des)
{
	double d = paras["d"];
	cv::bilateralFilter(*src,*des,d, d* 2, d/2);
	return true;
}


BilateraFilter::BilateraFilter()
{
	paras.insert(std::make_pair("d",12));
}

BilateraFilter::~BilateraFilter()
{

}

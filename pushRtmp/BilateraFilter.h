#pragma once
#include "Filter.h"

class BilateraFilter :
	public Filter
{
public:
	bool filter(cv::Mat* src, cv::Mat* des ) override;
	BilateraFilter();
	virtual ~BilateraFilter();
};


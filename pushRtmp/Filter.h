#pragma once
#include <string>
#include <map>



namespace cv
{
	class Mat;
}

class Filter
{
public:
	enum FilterType
	{
		BILATERAL //Ë«±ßÂË²¨
	};
	static Filter* Get(FilterType type = BILATERAL);
	virtual bool filter(cv::Mat* src, cv::Mat* des) = 0;
	virtual bool Set(std::string key, double value);
	virtual ~Filter() = default;
protected:
	std::map<std::string, double> paras;
	Filter() = default;
};


#pragma once
#include <vector>
#include <opencv2/core.hpp>

class OniTracker
{
public:
	bool isPersonInBorder(cv::Mat image, cv::Rect person);

	std::vector<cv::Rect> getPeople(cv::Mat image);

public:
	OniTracker()
	{
		
	}
};

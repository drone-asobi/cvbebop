#pragma once
#include <vector>
#include <opencv2/core.hpp>

class OniTracker
{
public:
	const double resize_rate = 0.7;

private:
	std::vector<cv::Mat> captured;

public:
	bool isPersonInBorder(cv::Mat image, cv::Rect person);

	std::vector<cv::Rect> getPeople(cv::Mat image);

	void addCaptured(cv::Mat image, cv::Rect person);

	std::vector<cv::Mat>& getCaptured();

	void clearCaptured();

public:
	OniTracker()
	{
		
	}
};

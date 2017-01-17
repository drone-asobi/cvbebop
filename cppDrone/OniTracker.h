#pragma once

#undef min
#undef max

#include <vector>
#include <mutex>
#include <algorithm>

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/tracking.hpp>

class OniTracker
{
public:
	const double resize_rate = 0.7;
	const bool useCascade2000 = false;
	const bool useCascade10000 = true;
	const bool useHog = false;

private:
	std::mutex captured_mutex;
	cv::CascadeClassifier cascade2000;
	cv::CascadeClassifier cascade10000;
	cv::HOGDescriptor hog;
	std::vector<cv::Mat> captured;

public:
	bool isPersonInBorder(const cv::Mat& image, const cv::Rect person);

	std::vector<cv::Rect> getPeople(const cv::Mat& image);

	void addCaptured(const cv::Mat& image, const cv::Rect person);

	std::vector<cv::Mat>& getCaptured();

	void clearCaptured();

public:
	OniTracker()
	{
		cascade2000.load("lbpcascade2000.xml");
		cascade10000.load("lbpcascade10000.xml");
		hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
	}
};

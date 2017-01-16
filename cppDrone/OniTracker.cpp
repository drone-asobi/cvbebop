#define TAG "Oni"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/objdetect.hpp>

#include "OniTracker.h"

using namespace cv;

/**
 * 人が一定距離以内にいて、鬼が捕まえたと判断する場合trueを返す。
 * 
 * 引数 image はカメラ画像を表す。
 * 引数 person は追跡中の人を表す。
 */
bool OniTracker::isPersonInBorder(cv::Mat image, cv::Rect person)
{

	/////距離計測/////
	double reference_d = 2.5;	//基準の距離(m)
	double reference_size = 7938 * 8 / 3; //基準の人領域の大きさ
	double distance = 0;	//距離(m)

							/////距離の度合い/////
	double near_range = 2.3;

	distance = reference_d*sqrt(reference_size) / sqrt(person.area());

	return distance <= near_range;
}

/**
 * カメラ画像の中に人がいるかどうか判断する。
 * 人がいる場合、検出されたすべての人を返す。
 * 
 * 引数 image はカメラ画像を表す。
 */
std::vector<cv::Rect> OniTracker::getPeople(cv::Mat image)
{
	HOGDescriptor hog;
	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

	std::vector<cv::Rect> found;
	// 画像，検出結果，閾値（SVMのhyper-planeとの距離），
	// 探索窓の移動距離（Block移動距離の倍数），
	// 画像外にはみ出た対象を探すためのpadding，
	// 探索窓のスケール変化係数，グルーピング係数
	hog.detectMultiScale(image, found, 0.2, cv::Size(8, 8), cv::Size(16, 16), 1.05, 2);

	std::cout << "found:" << found.size() << std::endl;
	return found;
}

void OniTracker::addCaptured(Mat image, Rect person)
{
	Mat region(person.height, person.width, image.type());
	Mat(image, person).copyTo(region);
	captured.push_back(region);
}

std::vector<Mat>& OniTracker::getCaptured()
{
	return captured;
}

void OniTracker::clearCaptured()
{
	for (auto person : captured)
	{
		person.release();
	}
	captured.clear();
}

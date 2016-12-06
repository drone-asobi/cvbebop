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

#include "OniTracker.h"

/**
 * 人が一定距離以内にいて、鬼が捕まえたと判断する場合trueを返す。
 * 
 * 引数 image はカメラ画像を表す。
 * 引数 person は追跡中の人を表す。
 */
bool OniTracker::isPersonInBorder(cv::Mat image, cv::Rect person)
{


	return false;
}

/**
 * カメラ画像の中に人がいるかどうか判断する。
 * 人がいる場合、検出されたすべての人を返す。
 * 
 * 引数 image はカメラ画像を表す。
 */
std::vector<cv::Rect> OniTracker::getPeople(cv::Mat image)
{


	return std::vector<cv::Rect>();
}

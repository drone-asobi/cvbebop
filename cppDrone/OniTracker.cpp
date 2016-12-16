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
	double reference_size = 7938; //基準の人領域の大きさ
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
	// ref: http://opencv.jp/cookbook/opencv_img.html#id43
	HOGDescriptor hog;
	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

	std::vector<cv::Rect> found;
	// 画像，検出結果，閾値（SVMのhyper-planeとの距離），
	// 探索窓の移動距離（Block移動距離の倍数），
	// 画像外にはみ出た対象を探すためのpadding，
	// 探索窓のスケール変化係数，グルーピング係数
	hog.detectMultiScale(image, found, 0.2, cv::Size(8, 8), cv::Size(16, 16), 1.05, 2);

	std::cout << "found:" << found.size() << std::endl;
	for (auto it = found.begin(); it != found.end(); ++it)
	{
		auto r = *it;
		// 描画に際して，検出矩形を若干小さくする
		r.x += cvRound(r.width * 0.1);
		r.width = cvRound(r.width * 0.8);
		r.y += cvRound(r.height * 0.07);
		r.height = cvRound(r.height * 0.8);
		cv::rectangle(image, r.tl(), r.br(), cv::Scalar(0, 255, 0), 3);
		r.area();
	}
	return found;
}

#define TAG "Oni"

#include "OniTracker.h"

using namespace cv;

/**
 * 人が一定距離以内にいて、鬼が捕まえたと判断する場合trueを返す。
 * 
 * 引数 image はカメラ画像を表す。
 * 引数 person は追跡中の人を表す。
 */
bool OniTracker::isPersonInBorder(const Mat& image, const Rect person)
{
	/////距離計測/////
	double reference_d = 2.5;	//基準の距離(m)
	double reference_size = 7938 * 8 / 3; //基準の人領域の大きさ
	double distance;	//距離(m)

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
std::vector<Rect> OniTracker::getPeople(const Mat& image)
{
	if (this->useCascade) {
		Mat gray_img; //グレイ画像変換
		cvtColor(image, gray_img, CV_BGR2GRAY);
		equalizeHist(gray_img, gray_img);

		std::vector<cv::Rect> people;
		this->cascade.detectMultiScale(gray_img, people); //条件設定
		gray_img.release();

		std::cout << "found:" << people.size() << std::endl;

		return people;
	}

	if (this->useHog)
	{
		std::vector<Rect> found;
		// 画像，検出結果，閾値（SVMのhyper-planeとの距離），
		// 探索窓の移動距離（Block移動距離の倍数），
		// 画像外にはみ出た対象を探すためのpadding，
		// 探索窓のスケール変化係数，グルーピング係数
		this->hog.detectMultiScale(image, found, 0.2, Size(8, 8), Size(16, 16), 1.05, 2);

		std::cout << "found:" << found.size() << std::endl;

		return found;
	}

	return std::vector<Rect>();
}

void OniTracker::addCaptured(const Mat& image, const Rect person)
{
	Mat region(person.height, person.width, image.type());
	Mat(image, person).copyTo(region);
	this->captured_mutex.lock();
	this->captured.push_back(region);
	this->captured_mutex.unlock();
}

std::vector<Mat>& OniTracker::getCaptured()
{
	this->captured_mutex.lock();
	auto clone = new std::vector<Mat>(this->captured);
	this->captured_mutex.unlock();
	return *clone;
}

void OniTracker::clearCaptured()
{
	this->captured_mutex.lock();
	for (auto person : this->captured)
	{
		person.release();
	}
	this->captured.clear();
	this->captured_mutex.unlock();
}

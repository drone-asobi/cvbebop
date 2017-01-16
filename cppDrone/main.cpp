#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <iostream>
#include <ostream>
#include <cmath>
#include <random>
#include <algorithm>
#include <set>
#include <time.h>
#include <vector>
#include <numeric>
#include <chrono>

extern "C" {
#include <libARSAL/ARSAL.h>
#include <libARController/ARController.h>
}

#include <opencv2\core.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\objdetect.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\features2d.hpp>
#include <opencv2\video.hpp>
#include <opencv2\videoio.hpp>
#include <opencv2\calib3d.hpp>
#include <opencv2\video\tracking.hpp>
#include <opencv2/tracking.hpp>


#include "bebop2_device.h"
#include "bebop2_controller.h"

#include "Oni.h"

using namespace std;
using namespace cv;
using namespace bebop_driver;

#define TAG "Main"

eARCONTROLLER_ERROR receive_frame_callback(ARCONTROLLER_Frame_t *frame, void *customData);
void process_opencv_from_image(Mat& frame1);

uint64_t prev_timestamp = 0;

const cv::Size MAX_DETECT_SIZE = cv::Size(100, 200);

const int MAX_MISS_FRAME = 10;

const double MIN_NEW_DETECT_INTERSECTION_RATE = 0.5;



// cv::Trackerのラッパー。

class MyTracker {

private:

	static int next_id;

	int id;

	int n_miss_frame = 0;

	cv::Rect2d rect;

	cv::Ptr<cv::Tracker> cv_tracker;

public:

	// フレーム画像と追跡対象(Rect)で初期化

	MyTracker(const cv::Mat& _frame, const cv::Rect2d& _rect)

		: id(next_id++), rect(_rect) {

		cv_tracker = cv::Tracker::create("BOOSTING"); //  or "MIL"

		cv_tracker->init(_frame, _rect);

	}

	// 次フレームを入力にして、追跡対象の追跡(true)

	// MAX_MISS_FRAME以上検出が登録されていない場合は追跡失敗(false)

	bool update(const cv::Mat& _frame) {

		n_miss_frame++;

		return cv_tracker->update(_frame, rect) && n_miss_frame < MAX_MISS_FRAME;

	}

	// 新しい検出(Rect)を登録。現在位置と近ければ受理してn_miss_frameをリセット(true)

	// そうでなければ(false)

	bool registerNewDetect(const cv::Rect2d& _new_detect) {

		double intersection_rate = 1.0 * (_new_detect & rect).area() / (_new_detect | rect).area();

		bool is_registered = intersection_rate > MIN_NEW_DETECT_INTERSECTION_RATE;

		if (is_registered) n_miss_frame = 0;

		return is_registered;

	}

	// trackerの現在地を_imageに書き込む

	void draw(cv::Mat& _image) const {

		cv::rectangle(_image, rect, cv::Scalar(255, 0, 0), 2, 1);

		cv::putText(_image, cv::format("%03d", id), cv::Point(rect.x + 5, rect.y + 17),

			cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1, CV_AA);

	}

};

int MyTracker::next_id = 0;



eARCONTROLLER_ERROR receive_frame_callback(ARCONTROLLER_Frame_t *frame, void *customData)
{
	auto *decoder = static_cast<VideoDecoder*>(customData);

	if (decoder == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "VideoDecoder is NULL.");
	}
	else
	{
		if (frame != nullptr)
		{
			auto res = decoder->Decode(frame);
			if (!res)
			{
				ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "frame decode has failed.");
			}
			else
			{
				uint64_t cur_timestamp = GetTickCount64();
				if (cur_timestamp - prev_timestamp < 1000) {
					CvSize size;
					size.width = decoder->GetFrameWidth();
					size.height = decoder->GetFrameHeight();

					Mat image(size, CV_8UC3, (void*)decoder->GetFrameRGBRawCstPtr());
					imshow("video", image);

					// process_opencv_from_image(image);
				}
				prev_timestamp = cur_timestamp;
			}
		}
		else
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "frame is NULL.");
		}
	}

	return ARCONTROLLER_OK;
}

void process_bebop2()
{
	cout << "called process_bebop2()" << endl;

	ARCONTROLLER_Device_t* deviceController;
	auto* videoDecoder = new VideoDecoder();

	auto error = start_bebop2(&deviceController, command_received_callback, videoDecoder, receive_frame_callback, nullptr);

	if (error == ARCONTROLLER_OK) {

		deviceController->aRDrone3->sendPilotingSettingsMaxAltitude(deviceController->aRDrone3, 2.0);
		deviceController->aRDrone3->sendMediaStreamingVideoStreamMode(deviceController->aRDrone3, ARCOMMANDS_ARDRONE3_MEDIASTREAMING_VIDEOSTREAMMODE_MODE_HIGH_RELIABILITY);

		keyboard_controller_loop(deviceController, "video");
	}

	finish_bebop2(deviceController);
}

//////////////////////
//opencv definitions//
//////////////////////

/////フラグ/////
bool flag_detect_people = false;
bool flag_detect_distance = false;
bool flag_measure_fps = false;
bool flag_track_something = false;
bool flag_detect_face = false;
bool flag_tracking_something = false;

vector<double> list(200, 1);

cv::Rect result;	//人認識の領域
cv::Rect_<float> r_keep;
cv::Rect_<float> r_predict;
cv::Rect_<float> rect_keep;

/////distance_part1/////
double f_s = 1062.9;	//カメラの焦点距離(pixel)
double distance = 0;	//カメラとの距離(m)
double H = 240;	//撮影される画像の縦の長さ(pixel)
double h = 0.53;	//カメラの高さ(m)
Point tyumoku;	//注目点の座標(pixel)

/////distance_part2/////
double reference_d = 2.33;	//基準の距離(m)
double reference_size = 15225; //基準の人領域の大きさ

/////検出結果チェック/////
int count = 0; //配列の番号を指す
int i = 0;
int n = 0;
int flag = 0;
int f_memory = 0;
int k = 0;

int distance_measurement(int S) {

	/////距離計測/////
	double reference_d = 2.33;	//基準の距離(m)
	double reference_size = 15225; //基準の人領域の大きさ
	double distance = 0;	//距離(m)

	/////距離の度合い/////
	double near_range = 2.3;
	double middle_range = 2.7;

	distance = reference_d*sqrt(reference_size) / sqrt(S);
	cout << "distance" << distance << endl;

	if (distance <= near_range) {
		return(0);
	}
	else if (distance <= middle_range && distance > near_range) {
		return(1);
	}

	return(2);
}

void opencv_detect_person(Mat &img, cv::Rect &r,int &n)
{
	// ref: http://opencv.jp/cookbook/opencv_img.html#id43
	HOGDescriptor hog;
	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

	std::vector<cv::Rect> found;
	// 画像，検出結果，閾値（SVMのhyper-planeとの距離），
	// 探索窓の移動距離（Block移動距離の倍数），
	// 画像外にはみ出た対象を探すためのpadding，
	// 探索窓のスケール変化係数，グルーピング係数
	hog.detectMultiScale(img, found, 0.2, cv::Size(8, 8), cv::Size(16, 16), 1.05, 2);

	int d;

	std::cout << "found:" << found.size() << std::endl;
	n = (int) found.size(); //size_t型をint型にキャスト
	for (auto it = found.begin(); it != found.end(); ++it)
	{
		r = *it;
		// 描画に際して，検出矩形を若干小さくする
		r.x += cvRound(r.width * 0.1);
		r.width = cvRound(r.width * 0.8);
		r.y += cvRound(r.height * 0.07);
		r.height = cvRound(r.height * 0.8);
		r.area();
		d = distance_measurement(r.area());

		if (d == 0) {	//近いと赤色の四角
			cv::rectangle(img, r.tl(), r.br(), cv::Scalar(0, 0, 255), 3);
			cv::putText(img, "near_range", cv::Point(r.tl().x, r.tl().y), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2, CV_AA);
		}
		else if (d == 1) {	//中間の距離は緑の四角
			cv::rectangle(img, r.tl(), r.br(), cv::Scalar(0, 255, 0), 3);
			cv::putText(img, "middle_range", cv::Point(r.tl().x, r.tl().y), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2, CV_AA);
		}
		else {	//遠いと青色の四角
			cv::rectangle(img, r.tl(), r.br(), cv::Scalar(255, 0, 0), 3);
			cv::putText(img, "far_range", cv::Point(r.tl().x, r.tl().y), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2, CV_AA);
		}
	}
	// 結果の描画
	cv::namedWindow("result", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::imshow("result", img);
}

void opencv_detect_person_hogsvm(Mat img, cv::Rect &r)
{
	// ref: http://opencv.jp/cookbook/opencv_img.html#id43
	HOGDescriptor hog;
	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

	std::vector<cv::Rect> found;
	// 画像，検出結果，閾値（SVMのhyper-planeとの距離），
	// 探索窓の移動距離（Block移動距離の倍数），
	// 画像外にはみ出た対象を探すためのpadding，
	// 探索窓のスケール変化係数，グルーピング係数
	hog.detectMultiScale(img, found, 0.2, cv::Size(8, 8), cv::Size(16, 16), 1.05, 2);

	std::cout << "found:" << found.size() << std::endl;
	for (auto it = found.begin(); it != found.end(); ++it)
	{
		r = *it;
		// 描画に際して，検出矩形を若干小さくする
		r.x += cvRound(r.width * 0.1);
		r.width = cvRound(r.width * 0.8);
		r.y += cvRound(r.height * 0.07);
		r.height = cvRound(r.height * 0.8);
		cv::rectangle(img, r.tl(), r.br(), cv::Scalar(0, 255, 0), 3);
		r.area();
	}

	// 結果の描画
	cv::namedWindow("result", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::imshow("result", img);

}

void static opencv_detect_person_haarcascade(Mat img, cv::Rect &r)
{

	//全身を大きめに検知して、その中に上半身があれば出力とか、誤認識は減るかもしれんが認識率そのものも減る可能性

	Mat gray_img; //グレイ画像変換
	cvtColor(img, gray_img, CV_BGR2GRAY);
	equalizeHist(gray_img, gray_img);

	std::vector<cv::Rect> people;
	CascadeClassifier cascade("haarcascade_fullbody.xml"); //haar
	cascade.detectMultiScale(gray_img, people); //条件設定

	std::cout << "found:" << people.size() << std::endl; //発見数の表示

	for (auto it = people.begin(); it != people.end(); ++it) {
		r = *it;
		r.x += cvRound(r.width * 0.1);
		r.width = cvRound(r.width * 0.8);
		r.y += cvRound(r.height * 0.07);
		r.height = cvRound(r.height * 0.8);
		cv::rectangle(img, it->tl(), it->br(), Scalar(0, 0, 255), 2, 8, 0);
		r.area();
	}

	cv::namedWindow("result", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::imshow("result", img);
}

void opencv_detect_person_hogcascade(Mat img, cv::Rect &r)
{
	Mat gray_img; //グレイ画像変換
	cvtColor(img, gray_img, CV_BGR2GRAY);
	equalizeHist(gray_img, gray_img);

	std::vector<cv::Rect> people;
	CascadeClassifier cascade("hogcascade_pedestrians.xml"); //hog
	cascade.detectMultiScale(gray_img, people, 1.1, 2); //条件設定

	std::cout << "found:" << people.size() << std::endl; //発見数の表示

	for (auto it = people.begin(); it != people.end(); ++it) {
		r = *it;
		r.x += cvRound(r.width * 0.1);
		r.width = cvRound(r.width * 0.8);
		r.y += cvRound(r.height * 0.07);
		r.height = cvRound(r.height * 0.8);
		cv::rectangle(img, it->tl(), it->br(), Scalar(0, 0, 255), 2, 8, 0);
		r.area();
	}

	cv::namedWindow("result", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::imshow("result", img);
}

void tracking(Mat img, cv::Rect &r) {
	// detector, trackerの宣言
	cv::HOGDescriptor detector;
	detector.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
	std::vector<MyTracker> trackers;
		// 人物検出
		std::vector<cv::Rect> detections;
		detector.detectMultiScale(img, detections);
		// trackerの更新(追跡に失敗したら削除)
		for (auto t_it = trackers.begin(); t_it != trackers.end();) {
			t_it = (t_it->update(img)) ? std::next(t_it) : trackers.erase(t_it);
		}
		// 新しい検出があればそれを起点にtrackerを作成。(既存Trackerに重なる検出は無視)
		for (auto& d_rect : detections) {
			if (d_rect.size().area() > MAX_DETECT_SIZE.area()) continue;
			bool exists = std::any_of(trackers.begin(), trackers.end(),
				[&d_rect](MyTracker& t) {return t.registerNewDetect(d_rect); });
			if (!exists) trackers.push_back(MyTracker(img, d_rect));
		}
		// 人物追跡と人物検出の結果を表示
		cv::Mat image = img.clone();
		for (auto& t : trackers) t.draw(image);
		for (auto& d_rect : detections) cv::rectangle(image, d_rect, cv::Scalar(0, 255, 0), 2, 1);
		cv::namedWindow("result", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
		cv::imshow("tracking", image);
	}


void opencv_track_person(Mat img, cv::Rect_<float> &r_keep, cv::Rect &r, cv::Rect_<float> &r_predict, cv::Rect_<float> &rect_keep, int64 start) //物体追跡
{
	cv::Rect_<float> rect;
	float x_aft = r.x;
	float y_aft = r.y;
	float wid_aft = r.width;
	float hei_aft = r.height;
	float x_bef = r_keep.x;
	float y_bef = r_keep.y;
	float wid_bef = r_keep.width;
	float hei_bef = r_keep.height;
	int64 end = cv::getTickCount();
	double sec = (end - start) / cv::getTickFrequency();
	double fps = cv::getTickFrequency() / (end - start);

	if (x_aft != 0 && x_aft != x_bef)
	{
		//1sあたりの変化量
		r_predict.x = (x_aft - x_bef) * fps;
		r_predict.y = (y_aft - y_bef) * fps;
		r_predict.width = (wid_aft - wid_bef) * fps;
		r_predict.height = 2 * r_predict.width;

		rect_keep.x = x_aft;
		rect_keep.y = y_aft;
		rect_keep.width = wid_aft;
		rect_keep.height = 2 * wid_aft;
	}

	rect.x += cvRound(rect_keep.x + r_predict.x * sec);
	rect.y += cvRound(rect_keep.y + r_predict.y * sec);
	rect.width = cvRound(rect_keep.width + r_predict.width * sec);
	rect.height = 2 * rect.width;
	cv::rectangle(img, rect.tl(), rect.br(), cv::Scalar(200, 200, 200), 3);
	rect.area();

	r_keep.x = r.x;
	r_keep.y = r.y;
	r_keep.width = r.width;
	r_keep.height = 2 * r.width;

	rect_keep.x = rect.x;
	rect_keep.y = rect.y;
	rect_keep.width = rect.width;
	rect_keep.height = 2 * rect.width;

	// 結果の描画
	cv::namedWindow("result", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::imshow("result", img);
}

void opencv_loadimage()
{
	auto img = imread("img.png", IMREAD_COLOR);

	if (!img.data)
	{
		cout << "Load image error" << endl;
		return;
	}

	imshow("window", img);

	waitKey(0);
}

inline double measure_fps(int64 start, int64 end, double fps)
{
	return fps = cv::getTickFrequency() / (end - start);
}

void track_something(Mat &img, cv::Rect_<float> &r_keep, cv::Rect &r, cv::Rect_<float> &r_predict, cv::Rect_<float> &rect_keep, int64 start) //物体追跡
{
	cv::Rect_<float> rect;
	float x_aft = r.x;
	float y_aft = r.y;
	float wid_aft = r.width;
	float hei_aft = r.height;
	float x_bef = r_keep.x;
	float y_bef = r_keep.y;
	float wid_bef = r_keep.width;
	float hei_bef = r_keep.height;
	int64 end = cv::getTickCount();
	double sec = (end - start) / cv::getTickFrequency();
	double fps = cv::getTickFrequency() / (end - start);

	if(x_aft != 0 && x_aft != x_bef){
	
		//1sあたりの変化量
		r_predict.x = (x_aft - x_bef) * fps;
		r_predict.y = (y_aft - y_bef) * fps;
		r_predict.width = (wid_aft - wid_bef) * fps;
		r_predict.height = 2 * r_predict.width;

		rect_keep.x = x_aft;
		rect_keep.y = y_aft;
		rect_keep.width = wid_aft;
		rect_keep.height = hei_aft;

	}

	rect.x += cvRound(rect_keep.x + r_predict.x * sec);
	rect.y += cvRound(rect_keep.y + r_predict.y * sec);
	rect.width = cvRound(rect_keep.width + r_predict.width * sec);
	rect.height = 2 * rect.width;
	cv::rectangle(img, rect.tl(), rect.br(), cv::Scalar(200, 200, 200), 3);
	rect.area();

	r_keep.x = x_aft;
	r_keep.y = y_aft;
	r_keep.width = wid_aft;
	r_keep.height = hei_aft;

	rect_keep.x = rect.x;
	rect_keep.y = rect.y;
	rect_keep.width = rect.width;
	rect_keep.height = rect.height;

	// 結果の描画
	cv::namedWindow("result", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::imshow("result", img);
}

void process_opencv_from_image(Mat &frame1)
{
	int64 start = 0;
	int64 end = 0;
	double fps = 0;

	cv::Mat frame2;

	start = cv::getTickCount(); //fps計測基準時取得

	cv::resize(frame1, frame2, cv::Size(), 0.25, 0.25);
	cv::imshow("window", frame2);//画像を表示．

	int key = cv::waitKey(1);
	
	if (key == 's')//sが押されたとき
	{
		//フレーム画像を保存する．
		cv::imwrite("img.png", frame2);
	}
	else if (key == 't') //tが押されたとき
	{
		// 保存されている(img.png)を表示させる
		opencv_loadimage();
	}
	else if (key == 'p') //人認識
	{
		flag_detect_people = !flag_detect_people;
		cout << "Detect People ON" << endl;
	}
	else if (key == 'd') //距離計測
	{
		flag_detect_distance = !flag_detect_distance;
		cout << "Distance Measurement ON" << endl;
	}
	else if (key == 'x') //fps計測
	{
		flag_measure_fps = !flag_measure_fps;
		cout << "fps Measurement ON" << endl;
	}
	else if (key == 'y') //追跡開始
	{
		flag_tracking_something = !flag_tracking_something;
		cout << "Tracking start" << endl;
	}

	if (flag_detect_people)
	{
		//opencv_detect_person_haarcascade(frame2, result); //haar+cascade(赤)
		//opencv_detect_person_hogsvm(frame2, result); //hog+svm(緑)
		//opencv_detect_person_hogcascade(frame2, result); //hog+cascade(青)
		opencv_detect_person(frame2, result, n);

		if (flag_detect_distance)
		{
			tyumoku.y = result.br().y;	//注目点は人領域の下辺の真ん中
			tyumoku.x = (result.tl().x + result.br().x) / 2;

			//				distance = (2*f_s*h) / (2*tyumoku.y - H);				
			//				cout << "distance_part1" << endl;
			//				cout << (2 * f_s*h) / (2 * tyumoku.y - H) << endl;

			//				distance = reference_d*sqrt(reference_size) / sqrt(result.area());
			cout << "distance_part2" << std::endl;
			cout << reference_d*sqrt(reference_size) / sqrt(result.area()) << endl;
		}
	}

	//if (flag_detect_people)
	//{
	//	opencv_detect_person(frame2, result, n);
	//	end = cv::getTickCount();
	//	fps = measure_fps(start, end, fps);
	//	std::cout << "::" << fps << "fps" << std::endl;

	//	if (flag == 0) {
	//		f_memory = fps;
	//		flag = 1;
	//	}

	//	list[k] = n;

	//	k++;

	//	if (k == 3 * f_memory) { //監視するフレーム数をfpsから考慮する必要がある
	//		double avg = accumulate(&list[0], &list[k - 1], 0.0) / (k - 1);
	//		std::cout << "avg:" << avg << std::endl;
	//		if (avg < 0.1)//ここの値を変えることで警告の出やすさを調節する
	//					  //	std::cout << "::" << avg << "avg" << std::endl;
	//			cout << "Stop Drone!!!" << std::endl;
	//		k = 0;
	//		flag = 0;
	//	}
	//}

	if (flag_measure_fps) //fps計測と表示
	{
		end = cv::getTickCount();
		fps = measure_fps(start, end, fps);
		std::cout << "::" << fps << "fps" << std::endl;
	}
	if (flag_track_something) //対象の追跡を開始
	{
		track_something(frame2, r_keep, result, r_predict, rect_keep, start);
		std::cout << "x = " << result.x << std::endl;
	}
}

/**** ここを実装してイメージ処理をする ****/
/**** 他のファイルは今の段階でいじる必要なし ****/
void process_opencv()
{
	// ref: http://qiita.com/vs4sh/items/4a9ce178f1b2fd26ea30

	VideoCapture cap(0);//デバイスのオープン //cap.open(0); //こっちでも良い．


	if (!cap.isOpened())//カメラデバイスが正常にオープンしたか確認．
	{
		//読み込みに失敗したときの処理
		return;
	}

	/////フラグ/////
	bool flag_detect_people = false;
	bool flag_detect_distance = false;
	bool flag_measure_fps = false;
	bool flag_track_something = false;
	bool flag_detect_face = false;
	bool flag_tracking_something = false;

	vector<double> list(200,1); 

	cv::Rect result;	//人認識の領域
	cv::Rect_<float> r_keep;
	cv::Rect_<float> r_predict;
	cv::Rect_<float> rect_keep;
	
	/////distance_part1/////
	double f_s = 1062.9;	//カメラの焦点距離(pixel)
	double distance = 0;	//カメラとの距離(m)
	double H = 240;	//撮影される画像の縦の長さ(pixel)
	double h = 0.53;	//カメラの高さ(m)
	Point tyumoku;	//注目点の座標(pixel)

	/////distance_part2/////
	double reference_d = 2.33;	//基準の距離(m)
	double reference_size = 15225; //基準の人領域の大きさ

	/////検出結果チェック/////
	int count = 0; //配列の番号を指す
	int i = 0;
	int n = 0;
	int flag = 0;
	int f_memory = 0;
	int k = 0;
	////measure_fps////
	int64 start = 0;
	int64 end = 0;
	double fps = 0;

	while (true)//無限ループ
	{
		cv::Mat frame1;
		cv::Mat frame2;

		//テスト
		cv::Mat channels[3];
		cv::Mat hsv_image;
		cv::Mat hsv_image1;
		//テスト

		start = cv::getTickCount(); //fps計測基準時取得
		
		cap >> frame1; // get a new frame from camera

		//
		//取得したフレーム画像に対して，クレースケール変換や2値化などの処理を書き込む．
		//
		cv::resize(frame1, frame2, cv::Size(), 0.6, 0.6);
		cv::imshow("window", frame2);//画像を表示．

		int key = cv::waitKey(1);
		if (key == 'q')//qボタンが押されたとき
		{
			break;//whileループから抜ける．
		}
		else if (key == 's')//sが押されたとき
		{
			//フレーム画像を保存する．
			cv::imwrite("img.png", frame2);
		}
		else if (key == 't') //tが押されたとき
		{
			// 保存されている(img.png)を表示させる
			opencv_loadimage();
		}
		else if (key == 'r') //rが押されたとき
		{
			//単に待つ
			waitKey(0);
		}
		else if (key == 'p') //人認識
		{
			flag_detect_people = !flag_detect_people;
			cout << "Detect People ON" << endl;
		}
		else if (key == 'd') //距離計測
		{
			flag_detect_distance = !flag_detect_distance;
			cout << "Distance Measurement ON" << endl;
		}
		else if (key == 'x') //fps計測
		{
			flag_measure_fps = !flag_measure_fps;
			cout << "fps Measurement ON" << endl;
		}
		else if (key == 'y') //追跡開始
		{
			flag_track_something = !flag_track_something;
			cout << "Tracking start" << endl;
		}

		if (flag_detect_people)
		{
			//opencv_detect_person_haarcascade(frame2, result); //haar+cascade(赤)
			//opencv_detect_person_hogsvm(frame2, result); //hog+svm(緑)
			//opencv_detect_person_hogcascade(frame2, result); //hog+cascade(青)

			opencv_detect_person(frame2, result, n);

			if (n != 0) {
				//ここからテスト
				cv::Rect rect(result.tl().x, result.tl().y, result.br().x - result.tl().x, result.br().y - result.tl().y);
				cv::Mat imgSub(frame2, rect);	//人領域
				cvtColor(imgSub, hsv_image, CV_RGB2HSV);
				cv::split(hsv_image, channels);
				int width = result.br().x - result.tl().x;
				int hight = result.br().y - result.tl().y;

				//HとSの値を変更
		
				channels[0] = Mat(Size(width,hight),CV_8UC1,100);
				channels[1] = Mat(Size(width, hight), CV_8UC1, 90);
				
				cv::merge(channels, 3, hsv_image1);
				cvtColor(hsv_image1, imgSub, CV_HSV2RGB);
				cv::imshow("sepia", imgSub);

				cv::Mat imgSub2;
				cv::resize(imgSub, imgSub2, cv::Size(325, 270), 0, 0);
				cv::Mat base = cv::imread("tehai.png", 1);
				cv::Mat comb(cv::Size(base.cols, base.rows), CV_8UC3);
				cv::Mat im1(comb, cv::Rect(0, 0, base.cols, base.rows));
				cv::Mat im2(comb, cv::Rect(40, 140, imgSub2.cols, imgSub2.rows));
				base.copyTo(im1);
				imgSub2.copyTo(im2);
				cv::resize(comb, comb, cv::Size(180, 320));
				cv::imshow("tehai", comb);
					//ここまでテスト
					
			}

			if (flag_detect_distance)
			{
				tyumoku.y = result.br().y;	//注目点は人領域の下辺の真ん中
				tyumoku.x = (result.tl().x + result.br().x) / 2;

				//				distance = (2*f_s*h) / (2*tyumoku.y - H);				
				//				cout << "distance_part1" << endl;
				//				cout << (2 * f_s*h) / (2 * tyumoku.y - H) << endl;

				//				distance = reference_d*sqrt(reference_size) / sqrt(result.area());
				cout << "distance_part2" << std::endl;
				cout << reference_d*sqrt(reference_size) / sqrt(result.area()) << endl;
			}
		}


		//if (flag_detect_people)
		//{
		//	opencv_detect_person(frame2, result, n);
		//	end = cv::getTickCount();
		//	fps = measure_fps(start, end, fps);
		//	std::cout << "::" << fps << "fps" << std::endl;


		//	if (flag == 0) {
		//		f_memory = fps;
		//		flag = 1;
		//	}

		//	list[k] = n;

		//	k++;
	
		//	if (k == 3*f_memory){ //監視するフレーム数をfpsから考慮する必要がある
		//		double avg = accumulate(&list[0], &list[k -1], 0.0) / (k - 1);
		//		std::cout << "avg:" << avg << std::endl;
		//		if (avg < 0.1)//ここの値を変えることで警告の出やすさを調節する
		//		//	std::cout << "::" << avg << "avg" << std::endl;
		//			cout << "Stop Drone!!!" << std::endl;
		//		k = 0;
		//		flag = 0;
		//	}
		//}

		if (flag_measure_fps) //fps計測と表示
		{
			end = cv::getTickCount();
			fps = measure_fps(start, end, fps);
			std::cout << "::" << fps << "fps" << std::endl;
		}
		if (flag_track_something) //対象の追跡を開始
		{
			tracking(frame2, result);
		}

	}
	cv::destroyAllWindows();
}

int main(void)
{
	//process_bebop2();

	auto oni = Oni::createOni();

	if(oni != nullptr)
	{
		printf("Start oni!");
		oni->startOni();
	}
	
	//process_opencv();
	return 0;
}
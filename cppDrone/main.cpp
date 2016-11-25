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

#include "bebop2_device.h"
#include "bebop2_controller.h"

using namespace std;
using namespace cv;
using namespace bebop_driver;

#define TAG "Main"

eARCONTROLLER_ERROR receive_frame_callback(ARCONTROLLER_Frame_t *frame, void *customData);
void process_opencv_from_image(Mat& frame1);

uint64_t prev_timestamp = 0;

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

					process_opencv_from_image(image);
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

	auto error = start_bebop2(&deviceController, command_received_callback, videoDecoder, receive_frame_callback);

	if (error == ARCONTROLLER_OK) {

		deviceController->aRDrone3->sendPilotingSettingsMaxAltitude(deviceController->aRDrone3, 2.0);

		keyboard_controller_loop(deviceController, "video");
	}

	finish_bebop2(deviceController);
}

//////////////////////
//opencv definitions//
//////////////////////

/////フラグ/////
bool flag_detect_people = false;
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

double distance_measurement(int S) {
	/////距離計測/////
	double reference_d = 2.33;	//基準の距離(m)
	double reference_size = 15225; //基準の人領域の大きさ
	double distance = 0;	//距離(m)

	distance = reference_d*sqrt(reference_size) / sqrt(S);
	cout << "distance" << distance << endl;

	return(distance);

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

	double d=0;

	/////距離の度合い/////
	double near_range = 2.3;
	double middle_range = 2.7;

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

		if (d <= near_range) {	//近いと赤色の四角
			cv::rectangle(img, r.tl(), r.br(), cv::Scalar(0, 0, 255), 3);
			cv::putText(img, "near_range", cv::Point(r.tl().x, r.tl().y), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2, CV_AA);
		}
		else if (d <=middle_range && d > near_range) {	//中間の距離は緑の四角
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

void opencv_detect_person_haarcascade(Mat img, cv::Rect &r)
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

	/////検出結果チェック/////
	int count = 0; //配列の番号を指す
	int i = 0;
	int n = 0;
	int flag = 0;
	int f_memory = 0;
	int k = 0;

	cv::Mat frame2;

	start = cv::getTickCount(); //fps計測基準時取得

	///distance////
	int l = 0;
	double distance[5] = { 100,100,100,100,100 };	//距離(m)

	cv::resize(frame1, frame2, cv::Size(), 0.6, 0.6);
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
		opencv_detect_person_hogcascade(frame2, result); //hog+cascade(青)

		opencv_detect_person(frame2, result, n);
		end = cv::getTickCount();
		fps = measure_fps(start, end, fps);
		std::cout << "::" << fps << "fps" << std::endl;


		if (flag == 0) {
			f_memory = fps;
			flag = 1;
		}

		list[k] = n;

		k++;

		if (k == 3 * f_memory) { //監視するフレーム数をfpsから考慮する必要がある
			double avg = accumulate(&list[0], &list[k - 1], 0.0) / (k - 1);
			std::cout << "avg:" << avg << std::endl;
			if (avg < 0.1)//ここの値を変えることで警告の出やすさを調節する
						  //	std::cout << "::" << avg << "avg" << std::endl;
				cout << "Stop Drone!!!" << std::endl;
			k = 0;
			flag = 0;
		}

		distance[l % 5] = distance_measurement(result.area());
		l++;

		double d_avg = accumulate(&distance[0], &distance[4], 0.0) / 4;
		//	cout << "avg:" << avg << endl;

		if (d_avg < 2.0)//ここの値を変えることで警告の出やすさを調節する
						//	std::cout << "::" << avg << "avg" << std::endl;
			cout << "Stop Drone!!!" << endl;


		if (frame2.cols / 2 < (result.br().x + result.tl().x) / 2) {
			//右に人がいるときは右に曲がる
		}if ((result.br().x + result.tl().x) / 2 < frame2.cols / 2) {
			//左に人がいるときは左に曲がる
		}

	}

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
	bool flag_measure_fps = false;
	bool flag_track_something = false;
	bool flag_detect_face = false;
	bool flag_tracking_something = false;

	vector<double> list(200,1); 

	cv::Rect result;	//人認識の領域
	cv::Rect_<float> r_keep;
	cv::Rect_<float> r_predict;
	cv::Rect_<float> rect_keep;
	
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

	///distance////
	int l = 0;
	double distance[5] = { 100,100,100,100,100 };	//距離(m)

	while (true)//無限ループ
	{
		cv::Mat frame1;
		cv::Mat frame2;

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
			opencv_detect_person_hogcascade(frame2, result); //hog+cascade(青)

			opencv_detect_person(frame2, result, n);
			end = cv::getTickCount();
			fps = measure_fps(start, end, fps);
			std::cout << "::" << fps << "fps" << std::endl;


			if (flag == 0) {
				f_memory = fps;
				flag = 1;
			}

			list[k] = n;
			
			k++;

		
			if (k == 3*f_memory){ //監視するフレーム数をfpsから考慮する必要がある
				double avg = accumulate(&list[0], &list[k -1], 0.0) / (k - 1);
				std::cout << "avg:" << avg << std::endl;
				if (avg < 0.1)//ここの値を変えることで警告の出やすさを調節する
				//	std::cout << "::" << avg << "avg" << std::endl;
					cout << "Stop Drone!!!" << std::endl;
				k = 0;
				flag = 0;
			}

			distance[l % 5] = distance_measurement(result.area());
			l++;

			double d_avg = accumulate(&distance[0], &distance[4], 0.0) / 4;
			//	cout << "avg:" << avg << endl;

			if (d_avg < 2.0)//ここの値を変えることで警告の出やすさを調節する
						  //	std::cout << "::" << avg << "avg" << std::endl;
				cout << "Stop Drone!!!" << endl;


			if (frame2.cols / 2 < (result.br().x + result.tl().x) / 2) {
				//右に人がいるときは右に曲がる
			}if ((result.br().x + result.tl().x) / 2 < frame2.cols / 2) {
				//左に人がいるときは左に曲がる
			}

		}

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
	cv::destroyAllWindows();
}

int main(void)
{
	process_bebop2();
	return 0;
}
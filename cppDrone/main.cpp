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



#include <opencv2\core.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\objdetect.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\features2d.hpp>
#include <opencv2\video.hpp>
#include <opencv2\videoio.hpp>
#include <opencv2\calib3d.hpp>
#include <opencv2\video\tracking.hpp>
#include "bebopCommand.h"

using namespace std;
using namespace cv;

void process_bebop()
{
	bebopCommand bebop;
	cv::Mat img;
	bebop.takeOff();

	while (1)
	{
		img = bebop.getImage();
		if (img.empty()) continue;
		cv::imshow("image", img);
		char key = cv::waitKey(10);
		int x, y, z, r = 0;
		if (key == 'q') break;
		if (key == 'w') x = 1;
		if (key == 'z') x = -1;
		if (key == 'a') y = 1;
		if (key == 's') y = -1;
		if (key == 'd') z = 1;
		if (key == 'x') z = -1;
		if (key == 'r') r = 1;
		bebop.move(x, y, z, r);
	}
	bebop.landing();
}

void opencv_detect_face(Mat img)
{
	// ref: http://opencv.jp/cookbook/opencv_img.html#id40

	double scale = 4.0;
	cv::Mat gray, smallImg(cv::saturate_cast<int>(img.rows / scale), cv::saturate_cast<int>(img.cols / scale), CV_8UC1);
	// グレースケール画像に変換
	cv::cvtColor(img, gray, CV_BGR2GRAY);
	// 処理時間短縮のために画像を縮小
	cv::resize(gray, smallImg, smallImg.size(), 0, 0, cv::INTER_LINEAR);
	cv::equalizeHist(smallImg, smallImg);

	// 分類器の読み込み
	std::string cascadeName = "./train/haarcascades/haarcascade_frontalface_alt.xml"; // Haar-like
	//std::string cascadeName = "./train/lbpcascades/lbpcascade_frontalface.xml"; // LBP
	cv::CascadeClassifier cascade;
	if (!cascade.load(cascadeName)) {
		cout << "fail to load cascade file" << endl;
		return;
	}

	std::vector<cv::Rect> faces;
	/// マルチスケール（顔）探索xo
	// 画像，出力矩形，縮小スケール，最低矩形数，（フラグ），最小矩形
	cascade.detectMultiScale(smallImg, faces,
	                         1.1, 2,
	                         CV_HAAR_SCALE_IMAGE,
	                         cv::Size(30, 30));

	// 結果の描画
	std::vector<cv::Rect>::const_iterator r = faces.begin();
	for (; r != faces.end(); ++r)
	{
		cv::Point center;
		int radius;
		center.x = cv::saturate_cast<int>((r->x + r->width * 0.5) * scale);
		center.y = cv::saturate_cast<int>((r->y + r->height * 0.5) * scale);
		radius = cv::saturate_cast<int>((r->width + r->height) * 0.25 * scale);
		cv::circle(img, center, radius, cv::Scalar(80, 80, 255), 3, 8, 0);
	}

	cv::namedWindow("result", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::imshow("result", img);
}

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

void opencv_detect_person(Mat img, cv::Rect &r,int &n)
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

double measure_fps(int64 start, int64 end, double fps)
{
	return fps = cv::getTickFrequency() / (end - start);
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
	bool flag_detect_face = false;
	bool flag_measure_fps = false;
	bool flag_tracking_something = false;

	vector<double> list(200,1); 

	cv::Rect result;	//人認識の領域
	
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
		else if (key == 'f') //顔認識
		{
			flag_detect_face = !flag_detect_face;
			cout << "Detect Face ON" << endl;
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


		}


		if (flag_tracking_something) //対象の追跡を開始
		{

		}

		if (flag_measure_fps) //fps計測と表示
		{
			end = cv::getTickCount();
			fps = measure_fps(start, end, fps);
			std::cout << "::" << fps << "fps" << std::endl;
			std::cout << "x = " << result.x << std::endl;
		}

	}
	cv::destroyAllWindows();
}

int main(void)
{
	process_opencv();
	// process_bebop();
	return 0;
}
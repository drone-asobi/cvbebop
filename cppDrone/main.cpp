#define DEBUG

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

extern "C" {
#include <libARSAL/ARSAL.h>
#include <libARDiscovery/ARDiscovery.h>
#include <libARController/ARController.h>
}

#include "BebopController.h"

#include <opencv2\core.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\objdetect.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\features2d.hpp>
#include <opencv2\video.hpp>
#include <opencv2\videoio.hpp>
#include <opencv2\calib3d.hpp>

using namespace std;
using namespace cv;
using namespace bebop_driver;

#define TAG "Main"

void key_control(ARCONTROLLER_Device_t *deviceController);
eARCONTROLLER_ERROR didReceiveFrameCallback(ARCONTROLLER_Frame_t *frame, void *customData);

void key_control(ARCONTROLLER_Device_t *deviceController)
{
	if (deviceController == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "DeviceController is NULL.");
		return;
	}

	cvNamedWindow("video");

	auto control = true;
	while (control) {
		char key = cv::waitKey(0);
		// Manage IHM input events
		eARCONTROLLER_ERROR error = ARCONTROLLER_OK;

		switch (key)
		{
		case 'q':
		case 'Q':
			// send a landing command to the drone
			error = deviceController->aRDrone3->sendPilotingLanding(deviceController->aRDrone3);
			control = false;
			break;
		case 'e':
		case 'E':
			// send a Emergency command to the drone
			error = deviceController->aRDrone3->sendPilotingEmergency(deviceController->aRDrone3);
			break;
		case 'l':
		case 'L':
			// send a landing command to the drone
			error = deviceController->aRDrone3->sendPilotingLanding(deviceController->aRDrone3);
			break;
		case 't':
		case 'T':
			// send a takeoff command to the drone
			error = deviceController->aRDrone3->sendPilotingTakeOff(deviceController->aRDrone3);
			break;
		case 'u': // UP
		case 'U':
			// set the flag and speed value of the piloting command
			// error = deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
			error = deviceController->aRDrone3->setPilotingPCMDGaz(deviceController->aRDrone3, 10);
			break;
		case 'j': // DOWN
		case 'J':
			// error = deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
			error = deviceController->aRDrone3->setPilotingPCMDGaz(deviceController->aRDrone3, -10);
			break;
		case 'k': // RIGHT
		case 'K':
			// error = deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
			error = deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, 10);
			break;
		case 'h':
		case 'H':
			// error = deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
			error = deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, -10);
			break;
		case 'r': //IHM_INPUT_EVENT_FORWARD
		case 'R':
			error = deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
			error = deviceController->aRDrone3->setPilotingPCMDPitch(deviceController->aRDrone3, 10);
			break;
		case 'f': //IHM_INPUT_EVENT_BACK:
		case 'F':
			error = deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
			error = deviceController->aRDrone3->setPilotingPCMDPitch(deviceController->aRDrone3, -10);
			break;
		case 'd': //IHM_INPUT_EVENT_ROLL_LEFT:
		case 'D':
			error = deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
			error = deviceController->aRDrone3->setPilotingPCMDRoll(deviceController->aRDrone3, -10);
			break;
		case 'g': //IHM_INPUT_EVENT_ROLL_RIGHT:
		case 'G':
			error = deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
			error = deviceController->aRDrone3->setPilotingPCMDRoll(deviceController->aRDrone3, 10);
			break;
		default:
			error = deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
			break;
		}

		Sleep(10);

		// This should be improved, here it just displays that one error occured
		if (error != ARCONTROLLER_OK)
		{
			printf("Error sending an event\n");
		}
	}
}

eARCONTROLLER_ERROR didReceiveFrameCallback(ARCONTROLLER_Frame_t *frame, void *customData)
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
				ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "frame decode has failed.");
			}
			else
			{
				CvSize size;
				size.width = decoder->GetFrameWidth();
				size.height = decoder->GetFrameHeight();

				Mat image(size, CV_8UC3, (void*)decoder->GetFrameRGBRawCstPtr());

				imshow("video", image);
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
	VideoDecoder* videoDecoder = new VideoDecoder();

	auto error = start_control_bebop2(&deviceController, videoDecoder, didReceiveFrameCallback);

	if (error == ARCONTROLLER_OK) {

		deviceController->aRDrone3->sendPilotingSettingsMaxAltitude(deviceController->aRDrone3, 2.0);

		key_control(deviceController);
	}

	error = finish_control_bebop2(deviceController);
}

//void process_bebop()
//{
//	bebopCommand bebop;
//	cv::Mat img;
//	bebop.takeOff();
//
//	while (1)
//	{
//		img = bebop.getImage();
//		if (img.empty()) continue;
//		cv::imshow("image", img);
//		char key = cv::waitKey(10);
//		int x, y, z, r = 0;
//		if (key == 'q') break;
//		if (key == 'w') x = 1;
//		if (key == 'z') x = -1;
//		if (key == 'a') y = 1;
//		if (key == 's') y = -1;
//		if (key == 'd') z = 1;
//		if (key == 'x') z = -1;
//		if (key == 'r') r = 1;
//		bebop.move(x, y, z, r);
//	}
//	bebop.landing();
//}

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

void opencv_detect_person(Mat img)
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
		cv::Rect r = *it;
		// 描画に際して，検出矩形を若干小さくする
		r.x += cvRound(r.width * 0.1);
		r.width = cvRound(r.width * 0.8);
		r.y += cvRound(r.height * 0.07);
		r.height = cvRound(r.height * 0.8);
		cv::rectangle(img, r.tl(), r.br(), cv::Scalar(0, 255, 0), 3);
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

	bool flag_detect_people = false;
	bool flag_detect_face = false;

	while (true)//無限ループ
	{
		cv::Mat frame;
		cap >> frame; // get a new frame from camera

		//
		//取得したフレーム画像に対して，クレースケール変換や2値化などの処理を書き込む．
		//

		cv::imshow("window", frame);//画像を表示．

		int key = cv::waitKey(1);
		if (key == 'q')//qボタンが押されたとき
		{
			break;//whileループから抜ける．
		}
		else if (key == 's')//sが押されたとき
		{
			//フレーム画像を保存する．
			cv::imwrite("img.png", frame);
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

		if (flag_detect_people)
		{
			opencv_detect_person(frame);
		}
		else if (flag_detect_face)
		{
			opencv_detect_face(frame);
		}
	}
	cv::destroyAllWindows();
}

int main(void)
{
	// process_opencv();
	// process_bebop();
	process_bebop2();
	return 0;
}

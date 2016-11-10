﻿#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <iostream>
#include <ostream>
#include <cmath>
#include <random>
#include <algorithm>
#include <set>
#include <time.h>
#include "semaphore.h"

extern "C" {
#include <libARSAL/ARSAL.h>
#include <libARDiscovery/ARDiscovery.h>
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
#include "bebopCommand.h"

using namespace std;
using namespace cv;

#ifdef _DEBUG
#	define DEBUG_LOG(stmt) cout << stmt << endl
#else
#	define DEBUG_LOG(stmt)
#endif
ARDISCOVERY_Device_t* create_discovery_device()
{
	DEBUG_LOG("called create_discovery_device()");
	auto errorDiscovery = ARDISCOVERY_OK;

	auto device = ARDISCOVERY_Device_New(&errorDiscovery);

	bool failed = false;
	if (errorDiscovery != ARDISCOVERY_OK)
	{
		failed = true;
		DEBUG_LOG("    ARDISCOVERY_Device_New != ARDISCOVERY_OK");
	}
	else
	{
		errorDiscovery = ARDISCOVERY_Device_InitWifi(device, ARDISCOVERY_PRODUCT_BEBOP_2, "bebop2", BEBOP_IP_ADDRESS, BEBOP_DISCOVERY_PORT);
		if (errorDiscovery != ARDISCOVERY_OK)
		{
			failed = true;
			DEBUG_LOG("    ARDISCOVERY_Device_InitWifi != ARDISCOVERY_OK");
		}
	}
	
	if(failed) {
		ARDISCOVERY_Device_Delete(&device);
		return nullptr;
	}

	return device;
}

ARCONTROLLER_Device_t* create_device_controller(ARDISCOVERY_Device_t* device)
{
	eARCONTROLLER_ERROR error = ARCONTROLLER_OK;
	ARCONTROLLER_Device_t *deviceController = ARCONTROLLER_Device_New(device, &error);

	error = ARCONTROLLER_Device_AddStateChangedCallback(
		deviceController,
		[](eARCONTROLLER_DEVICE_STATE newState, eARCONTROLLER_ERROR error, void *customData)
		{
			switch (newState)
			{
			case ARCONTROLLER_DEVICE_STATE_RUNNING:
				break;
			case ARCONTROLLER_DEVICE_STATE_STOPPED:
				break;
			case ARCONTROLLER_DEVICE_STATE_STARTING:
				break;
			case ARCONTROLLER_DEVICE_STATE_STOPPING:
				break;
			default:
				break;
			}
		},
		nullptr);

	error = ARCONTROLLER_Device_AddCommandReceivedCallback(
		deviceController,
		[](eARCONTROLLER_DICTIONARY_KEY commandKey, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary, void *customData)
		{
			if (elementDictionary != NULL)
			{
				// if the command received is a battery state changed
				if (commandKey == ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_BATTERYSTATECHANGED)
				{
					ARCONTROLLER_DICTIONARY_ARG_t *arg = NULL;
					ARCONTROLLER_DICTIONARY_ELEMENT_t *element = NULL;

					// get the command received in the device controller
					HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, element);
					if (element != NULL)
					{
						// get the value
						HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_BATTERYSTATECHANGED_PERCENT, arg);

						if (arg != NULL)
						{
							uint8_t batteryLevel = arg->value.U8;
							// do what you want with the battery level
						}
					}
				}
				// else if (commandKey == THE COMMAND YOU ARE INTERESTED IN)
			}
		},
		nullptr);

	error = ARCONTROLLER_Device_SetVideoStreamCallbacks(
		deviceController,
		[](ARCONTROLLER_Stream_Codec_t codec, void *customData)
		{
			// configure your decoder
			// return ARCONTROLLER_OK if configuration went well
			// otherwise, return ARCONTROLLER_ERROR. In that case,
			// configDecoderCallback will be called again

			return ARCONTROLLER_OK;
		},
		[](ARCONTROLLER_Frame_t *frame, void *customData)
		{
			// display the frame
			// return ARCONTROLLER_OK if display went well
			// otherwise, return ARCONTROLLER_ERROR. In that case,
			// configDecoderCallback will be called again

			return ARCONTROLLER_OK;
		},
		nullptr,
		nullptr);

	error = ARCONTROLLER_Device_Start(deviceController);
	
	return deviceController;
}


void process_bebop2()
{
	cout << "called process_bebop2()" << endl;

	auto device = create_discovery_device();
	if (device == nullptr)
	{
		cout << "Discovery Fail" << endl;
	}

	auto controller = create_device_controller(device);
}

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

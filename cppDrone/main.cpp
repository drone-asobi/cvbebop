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
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\features2d.hpp>
#include <opencv2\video.hpp>
#include <opencv2\videoio.hpp>
#include <opencv2\calib3d.hpp>
#include "bebopCommand.h"


void process_bebop() {
	bebopCommand bebop;
	cv::Mat img;
	bebop.takeOff();

	while (1) {
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

void process_opencv() {
	// ref: http://qiita.com/vs4sh/items/4a9ce178f1b2fd26ea30
	cv::VideoCapture cap(0);//デバイスのオープン
							//cap.open(0);//こっちでも良い．

	if (!cap.isOpened())//カメラデバイスが正常にオープンしたか確認．
	{
		//読み込みに失敗したときの処理
		return;
	}

	while (1)//無限ループ
	{
		cv::Mat frame;
		cap >> frame; // get a new frame from camera

					  //
					  //取得したフレーム画像に対して，クレースケール変換や2値化などの処理を書き込む．
					  //

		cv::imshow("window", frame);//画像を表示．

		int key = cv::waitKey(1);
		if (key == 113)//qボタンが押されたとき
		{
			break;//whileループから抜ける．
		}
		else if (key == 115)//sが押されたとき
		{
			//フレーム画像を保存する．
			cv::imwrite("img.png", frame);
		}
	}
	cv::destroyAllWindows();
}

int main(void)
{
	// process_opencv();
	process_bebop();
	return 0;
}
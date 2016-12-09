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
#include <fstream>

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
#include <opencv2/opencv.hpp>

#include "bebop2_device.h"
#include "bebop2_controller.h"

#include "Oni.h"
#include "FileIO.h"


using namespace std;
using namespace cv;
using namespace bebop_driver;
using namespace cv::ml;

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

	auto error = start_bebop2(&deviceController, command_received_callback, videoDecoder, receive_frame_callback);

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

void get_svm_detector(const Ptr<SVM>& svm, vector< float > & hog_detector)
{
	// get the support vectors
	Mat sv = svm->getSupportVectors();
	const int sv_total = sv.rows;
	// get the decision function
	Mat alpha, svidx;
	double rho = svm->getDecisionFunction(0, alpha, svidx);

	CV_Assert(alpha.total() == 1 && svidx.total() == 1 && sv_total == 1);
	CV_Assert((alpha.type() == CV_64F && alpha.at<double>(0) == 1.) ||
		(alpha.type() == CV_32F && alpha.at<float>(0) == 1.f));
	CV_Assert(sv.type() == CV_32F);
	hog_detector.clear();

	hog_detector.resize(sv.cols + 1);
	memcpy(&hog_detector[0], sv.ptr(), sv.cols * sizeof(hog_detector[0]));
	hog_detector[sv.cols] = (float)-rho;
}


/*
* Convert training/testing set to be used by OpenCV Machine Learning algorithms.
* TrainData is a matrix of size (#samples x max(#cols,#rows) per samples), in 32FC1.
* Transposition of samples are made if needed.
*/
void convert_to_ml(const std::vector< cv::Mat > & train_samples, cv::Mat& trainData)
{
	//--Convert data
	const int rows = (int)train_samples.size();
	const int cols = (int)std::max(train_samples[0].cols, train_samples[0].rows);
	cv::Mat tmp(1, cols, CV_32FC1); //< used for transposition if needed
	trainData = cv::Mat(rows, cols, CV_32FC1);
	vector< Mat >::const_iterator itr = train_samples.begin();
	vector< Mat >::const_iterator end = train_samples.end();
	for (int i = 0; itr != end; ++itr, ++i)
	{
		CV_Assert(itr->cols == 1 ||
			itr->rows == 1);
		if (itr->cols == 1)
		{
			transpose(*(itr), tmp);
			tmp.copyTo(trainData.row(i));
		}
		else if (itr->rows == 1)
		{
			itr->copyTo(trainData.row(i));
		}
	}
}


void load_pos_images(const string & filename, vector< Mat > & img_lst, bool debug)
{
	vector<string> imagelist;
	vector<vector<cv::Rect>> rectlists;
	if (!LoadAnnotationFile(filename, imagelist, rectlists))
	{
		cerr << "Unable to open the list of images from " << filename << " filename." << endl;
		exit(-1);
	}

	for (int i = 0; i < imagelist.size(); i++) {
		Mat img = imread(imagelist[i]);
		if (img.empty())
			continue;
		for (int j = 0; j < rectlists[i].size(); j++) {
			Mat part_img = img(rectlists[i][j]);
			if (debug) {
				imshow("image", part_img);
				waitKey(10);
			}
			img_lst.push_back(part_img.clone());
		}
	}
}



void load_images(const string & filename, vector< Mat > & img_lst, bool debug)
{
	vector<string> imagelist;
	if (!ReadList(filename, imagelist))
	{
		cerr << "Unable to open the list of images from " << filename << " filename." << endl;
		exit(-1);
	}

	for (int i = 0; i < imagelist.size(); i++) {
		Mat img = imread(imagelist[i]);
		if (img.empty())
			continue;

		if (debug) {
			imshow("image", img);
			waitKey(10);
		}
		img_lst.push_back(img);
	}
}


void sample_neg(const std::vector< cv::Mat > & full_neg_lst, std::vector< cv::Mat > & neg_lst, const cv::Size & size, bool debug)
{
	Rect box;
	box.width = size.width;
	box.height = size.height;

	const int size_x = box.width;
	const int size_y = box.height;

	srand((unsigned int)time(NULL));

	vector< Mat >::const_iterator img = full_neg_lst.begin();
	vector< Mat >::const_iterator end = full_neg_lst.end();
	for (; img != end; ++img)
	{
		box.x = rand() % (img->cols - size_x);
		box.y = rand() % (img->rows - size_y);
		Mat roi = (*img)(box);
		neg_lst.push_back(roi.clone());
		if (debug) {
			imshow("img", roi.clone());
			waitKey(10);
		}
	}
}


// From http://www.juergenwiki.de/work/wiki/doku.php?id=public:hog_descriptor_computation_and_visualization
Mat get_hogdescriptor_visu(const Mat& color_origImg, vector<float>& descriptorValues, const Size & size)
{
	const int DIMX = size.width;
	const int DIMY = size.height;
	float zoomFac = 3;
	Mat visu;
	resize(color_origImg, visu, Size((int)(color_origImg.cols*zoomFac), (int)(color_origImg.rows*zoomFac)));

	int cellSize = 8;
	int gradientBinSize = 9;
	float radRangeForOneBin = (float)(CV_PI / (float)gradientBinSize); // dividing 180 into 9 bins, how large (in rad) is one bin?

																	   // prepare data structure: 9 orientation / gradient strenghts for each cell
	int cells_in_x_dir = DIMX / cellSize;
	int cells_in_y_dir = DIMY / cellSize;
	float*** gradientStrengths = new float**[cells_in_y_dir];
	int** cellUpdateCounter = new int*[cells_in_y_dir];
	for (int y = 0; y<cells_in_y_dir; y++)
	{
		gradientStrengths[y] = new float*[cells_in_x_dir];
		cellUpdateCounter[y] = new int[cells_in_x_dir];
		for (int x = 0; x<cells_in_x_dir; x++)
		{
			gradientStrengths[y][x] = new float[gradientBinSize];
			cellUpdateCounter[y][x] = 0;

			for (int bin = 0; bin<gradientBinSize; bin++)
				gradientStrengths[y][x][bin] = 0.0;
		}
	}

	// nr of blocks = nr of cells - 1
	// since there is a new block on each cell (overlapping blocks!) but the last one
	int blocks_in_x_dir = cells_in_x_dir - 1;
	int blocks_in_y_dir = cells_in_y_dir - 1;

	// compute gradient strengths per cell
	int descriptorDataIdx = 0;
	int cellx = 0;
	int celly = 0;

	for (int blockx = 0; blockx<blocks_in_x_dir; blockx++)
	{
		for (int blocky = 0; blocky<blocks_in_y_dir; blocky++)
		{
			// 4 cells per block ...
			for (int cellNr = 0; cellNr<4; cellNr++)
			{
				// compute corresponding cell nr
				cellx = blockx;
				celly = blocky;
				if (cellNr == 1) celly++;
				if (cellNr == 2) cellx++;
				if (cellNr == 3)
				{
					cellx++;
					celly++;
				}

				for (int bin = 0; bin<gradientBinSize; bin++)
				{
					float gradientStrength = descriptorValues[descriptorDataIdx];
					descriptorDataIdx++;

					gradientStrengths[celly][cellx][bin] += gradientStrength;

				} // for (all bins)


				  // note: overlapping blocks lead to multiple updates of this sum!
				  // we therefore keep track how often a cell was updated,
				  // to compute average gradient strengths
				cellUpdateCounter[celly][cellx]++;

			} // for (all cells)


		} // for (all block x pos)
	} // for (all block y pos)


	  // compute average gradient strengths
	for (celly = 0; celly<cells_in_y_dir; celly++)
	{
		for (cellx = 0; cellx<cells_in_x_dir; cellx++)
		{

			float NrUpdatesForThisCell = (float)cellUpdateCounter[celly][cellx];

			// compute average gradient strenghts for each gradient bin direction
			for (int bin = 0; bin<gradientBinSize; bin++)
			{
				gradientStrengths[celly][cellx][bin] /= NrUpdatesForThisCell;
			}
		}
	}

	// draw cells
	for (celly = 0; celly<cells_in_y_dir; celly++)
	{
		for (cellx = 0; cellx<cells_in_x_dir; cellx++)
		{
			int drawX = cellx * cellSize;
			int drawY = celly * cellSize;

			int mx = drawX + cellSize / 2;
			int my = drawY + cellSize / 2;

			rectangle(visu, Point((int)(drawX*zoomFac), (int)(drawY*zoomFac)), Point((int)((drawX + cellSize)*zoomFac), (int)((drawY + cellSize)*zoomFac)), Scalar(100, 100, 100), 1);

			// draw in each cell all 9 gradient strengths
			for (int bin = 0; bin<gradientBinSize; bin++)
			{
				float currentGradStrength = gradientStrengths[celly][cellx][bin];

				// no line to draw?
				if (currentGradStrength == 0)
					continue;

				float currRad = bin * radRangeForOneBin + radRangeForOneBin / 2;

				float dirVecX = cos(currRad);
				float dirVecY = sin(currRad);
				float maxVecLen = (float)(cellSize / 2.f);
				float scale = 2.5; // just a visualization scale, to see the lines better

								   // compute line coordinates
				float x1 = mx - dirVecX * currentGradStrength * maxVecLen * scale;
				float y1 = my - dirVecY * currentGradStrength * maxVecLen * scale;
				float x2 = mx + dirVecX * currentGradStrength * maxVecLen * scale;
				float y2 = my + dirVecY * currentGradStrength * maxVecLen * scale;

				// draw gradient visualization
				line(visu, Point((int)(x1*zoomFac), (int)(y1*zoomFac)), Point((int)(x2*zoomFac), (int)(y2*zoomFac)), Scalar(0, 255, 0), 1);

			} // for (all bins)

		} // for (cellx)
	} // for (celly)


	  // don't forget to free memory allocated by helper data structures!
	for (int y = 0; y<cells_in_y_dir; y++)
	{
		for (int x = 0; x<cells_in_x_dir; x++)
		{
			delete[] gradientStrengths[y][x];
		}
		delete[] gradientStrengths[y];
		delete[] cellUpdateCounter[y];
	}
	delete[] gradientStrengths;
	delete[] cellUpdateCounter;

	return visu;

} // get_hogdescriptor_visu

void compute_hog(const vector< Mat > & img_lst, vector< Mat > & gradient_lst, const Size & size, bool debug)
{
	HOGDescriptor hog;
	hog.winSize = size;
	Mat gray;
	vector< Point > location;
	vector< float > descriptors;

	vector< Mat >::const_iterator img = img_lst.begin();
	vector< Mat >::const_iterator end = img_lst.end();
	for (; img != end; ++img)
	{
		cvtColor(*img, gray, COLOR_BGR2GRAY);
		hog.compute(gray, descriptors, Size(8, 8), Size(0, 0), location);
		gradient_lst.push_back(Mat(descriptors).clone());
		if (debug) {
			imshow("gradient", get_hogdescriptor_visu(img->clone(), descriptors, size));
			waitKey(10);
		}
	}
}

cv::Ptr<cv::ml::SVM> train_svm(const vector< Mat > & gradient_lst, const vector< int > & labels)
{

	Mat train_data;
	convert_to_ml(gradient_lst, train_data);

	clog << "Start training...";
	Ptr<SVM> svm = SVM::create();
	/* Default values to train SVM */
	svm->setCoef0(0.0);
	svm->setDegree(3);
	svm->setTermCriteria(TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 1000, 1e-3));
	svm->setGamma(0);
	svm->setKernel(SVM::LINEAR);
	svm->setNu(0.5);
	svm->setP(0.1); // for EPSILON_SVR, epsilon in loss function?
	svm->setC(0.01); // From paper, soft classifier
	svm->setType(SVM::EPS_SVR); // C_SVC; // EPSILON_SVR; // may be also NU_SVR; // do regression task
	svm->train(train_data, ROW_SAMPLE, Mat(labels));
	clog << "...[done]" << endl;

	//svm->save("my_people_detector.yml");
	return svm;
}


void draw_locations(Mat & img, const vector< Rect > & locations, const Scalar & color)
{
	if (!locations.empty())
	{
		vector< Rect >::const_iterator loc = locations.begin();
		vector< Rect >::const_iterator end = locations.end();
		for (; loc != end; ++loc)
		{
			rectangle(img, *loc, color, 2);
		}
	}
}


void get_hard_negatives(const std::vector<std::string>& test_imgs, const cv::Ptr<cv::ml::SVM>& svm, const cv::Size& size,
	std::vector<cv::Mat>& hard_negs, bool debug)
{
	HOGDescriptor my_hog;
	my_hog.winSize = size;

	// Set the trained svm to my_hog
	vector< float > hog_detector;
	get_svm_detector(svm, hog_detector);
	my_hog.setSVMDetector(hog_detector);
	//my_hog.setSVMDetector(my_hog.getDefaultPeopleDetector());

	vector<string>::const_iterator imf;
	vector<Rect>::const_iterator rect_i;
	hard_negs.clear();
	for (imf = test_imgs.begin(); imf != test_imgs.end(); imf++) {
		vector< Rect > locations;
		Mat im = imread(*imf);
		if (im.empty())
			continue;
		my_hog.detectMultiScale(im, locations);
		printf("%d hard negatives in %s.\n", locations.size(), imf->c_str());
		for (rect_i = locations.begin(); rect_i != locations.end(); rect_i++) {
			Mat resize_im;
			cv::resize(im(*rect_i), resize_im, size);
			hard_negs.push_back(resize_im);
		}
		if (debug) {
			draw_locations(im, locations, cv::Scalar(255, 0, 0));
			imshow("hard_negatives", im);
			waitKey(10);
		}
	}
}



void TrainHOGdetector(const std::string& pos, const std::string& neg, const std::string& val,
	const cv::Size& train_size, const std::string& save_svm, const std::string& save_old_svm, bool debug, bool restart_hns)
{
	vector< Mat > pos_lst;
	vector< Mat > neg_lst;
	vector< Mat > gradient_lst;
	vector< int > labels;

	load_pos_images(pos, pos_lst, debug);
	printf("Load %d positive images.\n", pos_lst.size());
	labels.assign(pos_lst.size(), +1);
	const unsigned int old = (unsigned int)labels.size();

	load_images(neg, neg_lst, debug);
	printf("Load %d negative images.\n", neg_lst.size());
	labels.insert(labels.end(), neg_lst.size(), -1);
	CV_Assert(old < labels.size());

	compute_hog(pos_lst, gradient_lst, train_size, debug);
	pos_lst.clear();
	printf("Compute positive samples' HOG.\n");
	compute_hog(neg_lst, gradient_lst, train_size, debug);
	neg_lst.clear();
	printf("Compute negative samples' HOG.\n");

	Ptr<SVM> svm;
	if (restart_hns) {
		printf("load %s\n", save_old_svm.c_str());
		svm = SVM::load<SVM>(save_old_svm);
	}
	else {
		svm = train_svm(gradient_lst, labels);
	}

	printf("Train support vector machine.\n");
	if (val.empty()) {
		svm->save(save_svm);
		printf("save %s as trained file.", save_svm.c_str());
		return;
	}
	else {
		svm->save(save_old_svm);
		printf("save %s as trained file.", save_old_svm.c_str());
	}

	std::vector<string> val_lst;
	if (!ReadList(val, val_lst)) {
		printf("Fail to read %s.\n", val.c_str());
		exit(-1);
	}

	std::vector<cv::Mat> false_pos_lst;
	get_hard_negatives(val_lst, svm, train_size, false_pos_lst, debug);
	svm->clear();

	val_lst.clear();
	printf("Got %d hard negative samples.\n", false_pos_lst.size());

	compute_hog(false_pos_lst, gradient_lst, train_size, debug);
	labels.insert(labels.end(), false_pos_lst.size(), -1);
	printf("Compute hard negative samples' HOG.\n");

	false_pos_lst.clear();
	svm = train_svm(gradient_lst, labels);
	svm->save(save_svm);
	printf("Train SVM and save as %s.\n", save_svm.c_str());
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
		opencv_detect_person_hogsvm(frame2, result); //hog+svm(緑)
		//opencv_detect_person_hogcascade(frame2, result); //hog+cascade(青)

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
			opencv_detect_person_hogsvm(frame2, result); //hog+svm(緑)
			//opencv_detect_person_hogcascade(frame2, result); //hog+cascade(青)

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

int main(int argc, char** argv)
{
	cv::CommandLineParser parser(argc, argv,
		"{help h|| show help message}"
		"{positive p|| positive image file}"
		"{negative n|| negative image list file}"
		"{validate v|| validate image list file for hard negative sampling}"
		"{debug d|| display train images and HOGS}"
		"{start_hns s|| restart from hard negative sampling}"
		"{width W|64| width of training image}"
		"{height H|128| height of training image}"
		"{@output|| output trained parameter file}");
	if (parser.has("help"))
	{
		parser.printMessage();
		exit(0);
	}
	string pos = parser.get<string>("p");
	string neg = parser.get<string>("n");
	string val = parser.get<string>("v");
	string outputf = parser.get<string>("@output");
	if (pos.empty() || neg.empty() || outputf.empty())
	{
		cout << "Wrong number of parameters." << endl;
		parser.printMessage();
		exit(-1);
	}

	int w = parser.get<int>("W");
	int h = parser.get<int>("H");
	bool debug = parser.has("d");
	bool restart_hns = parser.has("s");
	TrainHOGdetector(pos, neg, val, cv::Size(w, h), outputf, outputf + ".tmp", debug, restart_hns);

	//TestHOGdetector("testlist.txt", "test2", cv::Size(w, h), "my_people_detector.yml");
	//test_it(Size(96, 160)); // change with your parameters

	return 0;
}


/*
int main(void)
{
	//process_bebop2();

	//auto oni = Oni::createOni();

	//if(oni != nullptr)
	//{
	//	printf("Start oni!");
	//	oni->startOni();
	//}

	process_opencv();

	return 0;
}
*/
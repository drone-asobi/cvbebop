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
 * �l����苗���ȓ��ɂ��āA�S���߂܂����Ɣ��f����ꍇtrue��Ԃ��B
 * 
 * ���� image �̓J�����摜��\���B
 * ���� person �͒ǐՒ��̐l��\���B
 */
bool OniTracker::isPersonInBorder(cv::Mat image, cv::Rect person)
{

	/////�����v��/////
	double reference_d = 2.5;	//��̋���(m)
	double reference_size = 7938 * 8 / 3; //��̐l�̈�̑傫��
	double distance = 0;	//����(m)

							/////�����̓x����/////
	double near_range = 2.3;

	distance = reference_d*sqrt(reference_size) / sqrt(person.area());

	return distance <= near_range;
}

/**
 * �J�����摜�̒��ɐl�����邩�ǂ������f����B
 * �l������ꍇ�A���o���ꂽ���ׂĂ̐l��Ԃ��B
 * 
 * ���� image �̓J�����摜��\���B
 */
std::vector<cv::Rect> OniTracker::getPeople(cv::Mat image)
{
	HOGDescriptor hog;
	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

	std::vector<cv::Rect> found;
	// �摜�C���o���ʁC臒l�iSVM��hyper-plane�Ƃ̋����j�C
	// �T�����̈ړ������iBlock�ړ������̔{���j�C
	// �摜�O�ɂ͂ݏo���Ώۂ�T�����߂�padding�C
	// �T�����̃X�P�[���ω��W���C�O���[�s���O�W��
	hog.detectMultiScale(image, found, 0.2, cv::Size(8, 8), cv::Size(16, 16), 1.05, 2);

	std::cout << "found:" << found.size() << std::endl;
	return found;
}

void OniTracker::addCaptured(Mat image, Rect person)
{
	Mat region(person.height, person.width, image.type());
	Mat(image, person).copyTo(region);
	captured.push_back(region);
}

std::vector<Mat>& OniTracker::getCaptured()
{
	return captured;
}

void OniTracker::clearCaptured()
{
	for (auto person : captured)
	{
		person.release();
	}
	captured.clear();
}

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
	double reference_size = 7938; //��̐l�̈�̑傫��
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
	// ref: http://opencv.jp/cookbook/opencv_img.html#id43
	HOGDescriptor hog;
	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

	std::vector<cv::Rect> found;
	// �摜�C���o���ʁC臒l�iSVM��hyper-plane�Ƃ̋����j�C
	// �T�����̈ړ������iBlock�ړ������̔{���j�C
	// �摜�O�ɂ͂ݏo���Ώۂ�T�����߂�padding�C
	// �T�����̃X�P�[���ω��W���C�O���[�s���O�W��
	hog.detectMultiScale(image, found, 0.2, cv::Size(8, 8), cv::Size(16, 16), 1.05, 2);

	std::cout << "found:" << found.size() << std::endl;
	for (auto it = found.begin(); it != found.end(); ++it)
	{
		auto r = *it;
		// �`��ɍۂ��āC���o��`���኱����������
		r.x += cvRound(r.width * 0.1);
		r.width = cvRound(r.width * 0.8);
		r.y += cvRound(r.height * 0.07);
		r.height = cvRound(r.height * 0.8);
		cv::rectangle(image, r.tl(), r.br(), cv::Scalar(0, 255, 0), 3);
		r.area();
	}
	return found;
}

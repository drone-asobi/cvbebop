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

#include "OniTracker.h"

/**
 * �l����苗���ȓ��ɂ��āA�S���߂܂����Ɣ��f����ꍇtrue��Ԃ��B
 * 
 * ���� image �̓J�����摜��\���B
 * ���� person �͒ǐՒ��̐l��\���B
 */
bool OniTracker::isPersonInBorder(cv::Mat image, cv::Rect person)
{


	return false;
}

/**
 * �J�����摜�̒��ɐl�����邩�ǂ������f����B
 * �l������ꍇ�A���o���ꂽ���ׂĂ̐l��Ԃ��B
 * 
 * ���� image �̓J�����摜��\���B
 */
std::vector<cv::Rect> OniTracker::getPeople(cv::Mat image)
{


	return std::vector<cv::Rect>();
}

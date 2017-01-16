#define TAG "Oni"

#include "OniTracker.h"

using namespace cv;

/**
 * �l����苗���ȓ��ɂ��āA�S���߂܂����Ɣ��f����ꍇtrue��Ԃ��B
 * 
 * ���� image �̓J�����摜��\���B
 * ���� person �͒ǐՒ��̐l��\���B
 */
bool OniTracker::isPersonInBorder(const Mat& image, const Rect person)
{
	/////�����v��/////
	double reference_d = 2.5;	//��̋���(m)
	double reference_size = 7938 * 8 / 3; //��̐l�̈�̑傫��
	double distance;	//����(m)

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
std::vector<Rect> OniTracker::getPeople(const Mat& image)
{
	if (this->useCascade) {
		Mat gray_img; //�O���C�摜�ϊ�
		cvtColor(image, gray_img, CV_BGR2GRAY);
		equalizeHist(gray_img, gray_img);

		std::vector<cv::Rect> people;
		this->cascade.detectMultiScale(gray_img, people); //�����ݒ�
		gray_img.release();

		std::cout << "found:" << people.size() << std::endl;

		return people;
	}

	if (this->useHog)
	{
		std::vector<Rect> found;
		// �摜�C���o���ʁC臒l�iSVM��hyper-plane�Ƃ̋����j�C
		// �T�����̈ړ������iBlock�ړ������̔{���j�C
		// �摜�O�ɂ͂ݏo���Ώۂ�T�����߂�padding�C
		// �T�����̃X�P�[���ω��W���C�O���[�s���O�W��
		this->hog.detectMultiScale(image, found, 0.2, Size(8, 8), Size(16, 16), 1.05, 2);

		std::cout << "found:" << found.size() << std::endl;

		return found;
	}

	return std::vector<Rect>();
}

void OniTracker::addCaptured(const Mat& image, const Rect person)
{
	Mat region(person.height, person.width, image.type());
	Mat(image, person).copyTo(region);
	this->captured_mutex.lock();
	this->captured.push_back(region);
	this->captured_mutex.unlock();
}

std::vector<Mat>& OniTracker::getCaptured()
{
	this->captured_mutex.lock();
	auto clone = new std::vector<Mat>(this->captured);
	this->captured_mutex.unlock();
	return *clone;
}

void OniTracker::clearCaptured()
{
	this->captured_mutex.lock();
	for (auto person : this->captured)
	{
		person.release();
	}
	this->captured.clear();
	this->captured_mutex.unlock();
}

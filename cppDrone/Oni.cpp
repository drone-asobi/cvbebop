#define TAG "Oni"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <ctime>
#include <algorithm>

#include "Oni.h"

void Oni::oni_event_loop(eARCONTROLLER_DICTIONARY_KEY commandKey, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary, void *customData)
{
	auto *status = static_cast<DroneStatus*>(customData);

	if (status == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "DroneStatus is NULL!");
		return;
	}

	if (commandKey == ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_BATTERYSTATECHANGED && elementDictionary != nullptr)
	{
		ARCONTROLLER_DICTIONARY_ARG_t *arg = nullptr;
		ARCONTROLLER_DICTIONARY_ELEMENT_t *singleElement = nullptr;

		// get the command received in the device controller
		HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, singleElement);

		if (singleElement == nullptr)
		{
			return;
		}

		// get the value
		HASH_FIND_STR(singleElement->arguments, ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_BATTERYSTATECHANGED_PERCENT, arg);

		if (arg == nullptr)
		{
			return;
		}

		status->battery = arg->value.U8;

		// update UI
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Battery state has changed: %d %%", arg->value.U8);
	}

	if (commandKey == ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PICTURESETTINGSSTATE_VIDEOFRAMERATECHANGED && elementDictionary != nullptr)
	{
		ARCONTROLLER_DICTIONARY_ARG_t *arg = nullptr;
		ARCONTROLLER_DICTIONARY_ELEMENT_t *element = nullptr;

		HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, element);
		if (element == nullptr)
		{
			return;
		}

		HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PICTURESETTINGSSTATE_VIDEOFRAMERATECHANGED_FRAMERATE, arg);
		if (arg == nullptr)
		{
			return;
		}

		int res = -1;
		switch (eARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEOFRAMERATECHANGED_FRAMERATE(arg->value.I32))
		{
		case ARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEOFRAMERATECHANGED_FRAMERATE_24_FPS:
			res = 24;
			break;
		case ARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEOFRAMERATECHANGED_FRAMERATE_25_FPS:
			res = 25;
			break;
		case ARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEOFRAMERATECHANGED_FRAMERATE_30_FPS:
			res = 30;
			break;
		default:
			break;
		}

		status->framerate = res;
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Video frame rate is %d fps.", res);
	}

	if ((commandKey == ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PICTURESETTINGSSTATE_VIDEORESOLUTIONSCHANGED) && (elementDictionary != nullptr))
	{
		ARCONTROLLER_DICTIONARY_ARG_t *arg = nullptr;
		ARCONTROLLER_DICTIONARY_ELEMENT_t *element = nullptr;
		HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, element);
		if (element == nullptr)
		{
			return;
		}

		HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PICTURESETTINGSSTATE_VIDEORESOLUTIONSCHANGED_TYPE, arg);
		if (arg == nullptr)
		{
			return;
		}

		int res = -1;
		switch (static_cast<eARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEORESOLUTIONSCHANGED_TYPE>(arg->value.I32))
		{
		case ARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEORESOLUTIONSCHANGED_TYPE_REC1080_STREAM480:
			res = 1080;
			break;
		case ARCOMMANDS_ARDRONE3_PICTURESETTINGSSTATE_VIDEORESOLUTIONSCHANGED_TYPE_REC720_STREAM720:
			res = 720;
			break;
		default:
			break;
		}

		status->resolution = res;
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Video resolution is %dp", res);
	}

	if ((commandKey == ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_MEDIASTREAMINGSTATE_VIDEOSTREAMMODECHANGED) && (elementDictionary != nullptr))
	{
		ARCONTROLLER_DICTIONARY_ARG_t *arg = nullptr;
		ARCONTROLLER_DICTIONARY_ELEMENT_t *element = nullptr;
		HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, element);
		if (element == nullptr)
		{
			return;
		}

		HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_MEDIASTREAMINGSTATE_VIDEOSTREAMMODECHANGED_MODE, arg);
		if (arg == nullptr)
		{
			return;
		}

		char* mode = "unknown";

		switch (eARCOMMANDS_ARDRONE3_MEDIASTREAMINGSTATE_VIDEOSTREAMMODECHANGED_MODE(arg->value.I32))
		{
		case ARCOMMANDS_ARDRONE3_MEDIASTREAMINGSTATE_VIDEOSTREAMMODECHANGED_MODE_LOW_LATENCY:
			mode = "LOW_LATENCY";
			break;
		case ARCOMMANDS_ARDRONE3_MEDIASTREAMINGSTATE_VIDEOSTREAMMODECHANGED_MODE_HIGH_RELIABILITY:
			mode = "HIGH_RELIABILITY";
			break;
		case ARCOMMANDS_ARDRONE3_MEDIASTREAMINGSTATE_VIDEOSTREAMMODECHANGED_MODE_HIGH_RELIABILITY_LOW_FRAMERATE:
			mode = "HIGH_RELIABILITY_LOW_FRAMERATE";
			break;
		default:			
			break;
		}

		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Video stream mode is %s.", mode);
	}

	if ((commandKey == ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_CAMERASTATE_ORIENTATION) && (elementDictionary != nullptr))
	{
		ARCONTROLLER_DICTIONARY_ARG_t *arg = nullptr;
		ARCONTROLLER_DICTIONARY_ELEMENT_t *element = nullptr;
		HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, element);
		if (element != nullptr)
		{
			HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_CAMERASTATE_ORIENTATION_TILT, arg);
			if (arg != nullptr)
			{
				status->tilt = arg->value.I8;
			}
			HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_CAMERASTATE_ORIENTATION_PAN, arg);
			if (arg != nullptr)
			{
				status->pan = arg->value.I8;
			}
		}

		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Camera orientation is tilt=%d, pan=%d.", status->tilt, status->pan);
	}

	if ((commandKey == ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PILOTINGSTATE_SPEEDCHANGED) && (elementDictionary != nullptr))
	{
		ARCONTROLLER_DICTIONARY_ARG_t *arg = nullptr;
		ARCONTROLLER_DICTIONARY_ELEMENT_t *element = nullptr;
		HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, element);
		if (element != nullptr)
		{
			HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PILOTINGSTATE_SPEEDCHANGED_SPEEDX, arg);
			if (arg != nullptr)
			{
				status->speedX = arg->value.Float;
			}
			HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PILOTINGSTATE_SPEEDCHANGED_SPEEDY, arg);
			if (arg != nullptr)
			{
				status->speedY = arg->value.Float;
			}
			HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PILOTINGSTATE_SPEEDCHANGED_SPEEDZ, arg);
			if (arg != nullptr)
			{
				status->speedZ = arg->value.Float;
			}
		}

		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Speed is (%.3f, %.3f, %.3f).", status->speedX, status->speedY, status->speedZ);
	}

	if ((commandKey == ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PILOTINGSTATE_ATTITUDECHANGED) && (elementDictionary != nullptr))
	{
		ARCONTROLLER_DICTIONARY_ARG_t *arg = nullptr;
		ARCONTROLLER_DICTIONARY_ELEMENT_t *element = nullptr;
		HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, element);
		if (element != nullptr)
		{
			HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PILOTINGSTATE_ATTITUDECHANGED_ROLL, arg);
			if (arg != nullptr)
			{
				status->roll = arg->value.Float;
			}
			HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PILOTINGSTATE_ATTITUDECHANGED_PITCH, arg);
			if (arg != nullptr)
			{
				status->pitch = arg->value.Float;
			}
			HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PILOTINGSTATE_ATTITUDECHANGED_YAW, arg);
			if (arg != nullptr)
			{
				status->yaw = arg->value.Float;
			}
		}

		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Current (roll, pitch, yaw) is (%.3f, %.3f, %.3f).", status->roll, status->pitch, status->yaw);
	}

	if ((commandKey == ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PILOTINGSTATE_ALTITUDECHANGED) && (elementDictionary != nullptr))
	{
		ARCONTROLLER_DICTIONARY_ARG_t *arg = nullptr;
		ARCONTROLLER_DICTIONARY_ELEMENT_t *element = nullptr;
		HASH_FIND_STR(elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, element);
		if (element != nullptr)
		{
			HASH_FIND_STR(element->arguments, ARCONTROLLER_DICTIONARY_KEY_ARDRONE3_PILOTINGSTATE_ALTITUDECHANGED_ALTITUDE, arg);
			if (arg != nullptr)
			{
				status->altitude = arg->value.Double;
			}
		}

		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Current altitude is %.3lf.", status->altitude);
	}
}

eARCONTROLLER_ERROR Oni::oni_image_loop(ARCONTROLLER_Frame_t *frame, void *customData)
{
	auto *decoder = static_cast<bebop_driver::VideoDecoder*>(customData);

	if (decoder == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "VideoDecoder is NULL.");
		return ARCONTROLLER_ERROR_NO_ARGUMENTS;
	}

	if (frame == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "frame is NULL.");
		return ARCONTROLLER_ERROR_STREAMPOOL_FRAME_NOT_FOUND;
	}

	auto res = decoder->Decode(frame);
	if (!res)
	{
		ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "frame decode has failed.");
		return ARCONTROLLER_ERROR;
	}

	if (!true)
	{
		cv::Size size(decoder->GetFrameWidth(), decoder->GetFrameHeight());
		cv::Mat image(size, CV_8UC3, (void*)decoder->GetFrameRGBRawCstPtr());

		if (!image.empty()) {
			cv::imshow(MONITOR_WINDOW_NAME, image);
			cv::waitKey(1);
		}
	}

	return ARCONTROLLER_OK;
}

cv::Mat Oni::getCameraImage(double ratioX, double ratioY) const
{
	cv::Size size(mVideoDecoder->GetFrameWidth(), mVideoDecoder->GetFrameHeight());
	cv::Mat image(size, CV_8UC3, (void*)mVideoDecoder->GetFrameRGBRawCstPtr());
	cv::Mat resized;

	cv::resize(image, resized, cv::Size(), ratioX, ratioY);
	image.release();

	return resized;
}

void Oni::processCoolScreen(Oni* oni, const cv::Mat& wanted_base)
{
	using namespace cv;

	Mat origin = oni->getCameraImage(1.0, 1.0);
	Mat result;
	// Make image red tone.
	{
		int rows = origin.rows, cols = origin.cols;
		Mat hsv_orig, hsv_channel[3];
		cvtColor(origin, hsv_orig, CV_BGR2HSV);
		split(hsv_orig, hsv_channel);
		hsv_orig.release();
		hsv_channel[0].release();
		hsv_channel[0] = Mat::zeros(rows, cols, CV_8UC1);
		merge(hsv_channel, 3, hsv_orig);
		hsv_channel[0].release();
		hsv_channel[1].release();
		hsv_channel[2].release();
		cvtColor(hsv_orig, result, CV_HSV2BGR);
		hsv_orig.release();
	}

	// Print current date-time.
	{
		time_t rawtime;
		time(&rawtime);
		auto timeinfo = localtime(&rawtime);

		char buffer[80];
		strftime(buffer, sizeof(buffer), "%F %T %z", timeinfo);

		String text(buffer);
		int fontFace = CV_FONT_HERSHEY_DUPLEX;
		double fontScale = 0.6;
		int thickness = 1;
		int baseline = 0;
		Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
		baseline += thickness;
		Point textOrg(5, 20 + textSize.height / 2);

		putText(result, text, textOrg, fontFace, fontScale, Scalar::all(255), thickness, CV_AA);
	}

	// Print console output
	{
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
		SMALL_RECT srctReadRect;
		COORD coordBufSize = { 120, 120 };
		COORD coordBufCoord = { 0, 0 };
		CHAR_INFO chiBuffer[120 * 120];

		GetConsoleScreenBufferInfo(hStdout, &bufferInfo);

		srctReadRect.Top = bufferInfo.srWindow.Top - 90;
		srctReadRect.Left = 0;
		srctReadRect.Bottom = bufferInfo.srWindow.Bottom;
		srctReadRect.Right = 119;

		ReadConsoleOutput(hStdout, chiBuffer, coordBufSize, coordBufCoord, &srctReadRect);

		int console_face = CV_FONT_HERSHEY_PLAIN;
		double console_scale = 0.35;
		int console_thickness = 1;
		int offset = 40;
		for (int i = 0; i < coordBufSize.Y; ++i)
		{
			String console_text;
			for (int j = 0; j < coordBufSize.X; ++j)
			{
				console_text += chiBuffer[i * coordBufSize.X + j].Char.UnicodeChar;
			}

			int console_baseline = 0;
			Size textSize = getTextSize(console_text, console_face, console_scale, console_thickness, &console_baseline);
			console_baseline += console_thickness;

			Point console_textOrg(5, offset + textSize.height / 2);
			offset += textSize.height + console_baseline;

			putText(result, console_text, console_textOrg, console_face, console_scale, Scalar::all(255), console_thickness, CV_AA);
		}
	}

	// Print drone status
	{
		String text_arr[] = {
			format("Battery  : %8d %%", oni->mDroneStatus->battery),
			format("Roll     : %3.4f rad.", oni->mDroneStatus->roll),
			format("Pitch    : %3.4f rad.", oni->mDroneStatus->pitch),
			format("Yaw      : %3.4f rad.", oni->mDroneStatus->yaw),
			format("Altitude : %3.4lf m", oni->mDroneStatus->altitude),
			//format("Rel. dX  : %8d m", oni->mDroneStatus->dX),
			//format("Rel. dY  : %8d m", oni->mDroneStatus->dY),
			//format("Rel. dZ  : %8d m", oni->mDroneStatus->dZ),
			//format("Rel. dPsi: %8d rad", oni->mDroneStatus->dPsi),
			format("Speed-X  : %3.4f m/s", oni->mDroneStatus->speedX),
			format("Speed-Y  : %3.4f m/s", oni->mDroneStatus->speedY),
			format("Speed-Z  : %3.4f m/s", oni->mDroneStatus->speedZ),
			format("Tilt     : %8d", oni->mDroneStatus->tilt),
			format("Pan      : %8d", oni->mDroneStatus->pan) };

		int offset = 50;
		for (auto text : text_arr) {
			int fontFace = CV_FONT_HERSHEY_DUPLEX;
			double fontScale = 0.6;
			int thickness = 1;
			int baseline = 0;
			Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
			baseline += thickness;
			Point textOrg(result.cols - 300, offset + textSize.height / 2);
			offset += textSize.height + baseline;

			putText(result, text, textOrg, fontFace, fontScale, Scalar::all(255), thickness, CV_AA);
		}
	}

	// Print all captured
	{
		int offset = 40;
		for(auto captured : oni->mTracker->getCaptured())
		{
			Mat wanted(wanted_base.rows, wanted_base.cols, wanted_base.type());
			wanted_base.copyTo(wanted);
			Mat target;
			resize(captured, target, Size(325, 270));
			target.copyTo(wanted(Rect(Point(40, 140), Size(325, 270))));

			Mat channels[3];
			cvtColor(wanted, wanted, CV_BGR2HSV);
			split(wanted, channels);
			channels[0].release();
			channels[0] = Mat::zeros(wanted.rows, wanted.cols, CV_8UC1);
			merge(channels, 3, wanted);
			cvtColor(wanted, wanted, CV_HSV2BGR);

			resize(wanted, wanted, Size(135, 240));

			wanted.copyTo(result(Rect(Point(offset, result.rows - wanted.rows - 20), wanted.size())));
			offset += wanted.cols + 10;
		}
	}

	// Print main title
	if (oni->mStateController->getState() == StateController::STATE_READY) {
		String text = "DRONE_TAGGER";
		int fontFace = CV_FONT_HERSHEY_TRIPLEX;
		double fontScale = 3.5;
		int thickness = 3;
		int baseline = 0;
		Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
		baseline += thickness;
		Point textOrg((result.cols - textSize.width) / 2, result.rows / 2 + textSize.height / 2);

		rectangle(result, textOrg + Point(-40, thickness + 40), textOrg + Point(textSize.width + 40, -textSize.height - 40), Scalar(255, 255, 255), CV_FILLED);
		putText(result, text, textOrg, fontFace, fontScale, Scalar::all(0), thickness, CV_AA);
	}

	// Print taking off message
	if (oni->mStateController->getState() == StateController::STATE_TAKINGOFF)
	{
		String text = "> TAKING OFF_";
		int fontFace = CV_FONT_HERSHEY_TRIPLEX;
		double fontScale = 0.7;
		int thickness = 1;
		int baseline = 0;
		Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
		baseline += thickness;
		Point textOrg(result.cols - 300, 20 + textSize.height / 2);

		putText(result, text, textOrg, fontFace, fontScale, Scalar(255, 255, 255), thickness, CV_AA);
	}

	// Print ready message
	if (oni->mStateController->getState() == StateController::STATE_HOVERING)
	{
		String text = "> HOVERING_";
		int fontFace = CV_FONT_HERSHEY_TRIPLEX;
		double fontScale = 0.7;
		int thickness = 1;
		int baseline = 0;
		Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
		baseline += thickness;
		Point textOrg(result.cols - 300, 20 + textSize.height / 2);

		putText(result, text, textOrg, fontFace, fontScale, Scalar(255, 255, 255), thickness, CV_AA);
	}

	// Print searching target message
	if (oni->mStateController->getState() == StateController::STATE_SEARCHING)
	{
		String text = "> SEARCHING_";
		int fontFace = CV_FONT_HERSHEY_TRIPLEX;
		double fontScale = 0.7;
		int thickness = 1;
		int baseline = 0;
		Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
		baseline += thickness;
		Point textOrg(result.cols - 300, 20 + textSize.height / 2);

		putText(result, text, textOrg, fontFace, fontScale, Scalar(255, 255, 255), thickness, CV_AA);
	}

	// Print tracking message
	if (oni->mStateController->getState() == StateController::STATE_TRACKING ||
		oni->mStateController->getState() == StateController::STATE_MISSING)
	{
		String text = "> TRACKING_";
		int fontFace = CV_FONT_HERSHEY_TRIPLEX;
		double fontScale = 0.7;
		int thickness = 1;
		int baseline = 0;
		Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
		baseline += thickness;
		Point textOrg(result.cols - 300, 20 + textSize.height / 2);

		putText(result, text, textOrg, fontFace, fontScale, Scalar(255, 255, 255), thickness, CV_AA);

		auto target = oni->mDroneStatus->currentTarget;
		rectangle(result, target, Scalar(255, 255, 255, 255), 3);
	}

	// Print captured
	if (oni->mStateController->getState() == StateController::STATE_CAPTURED)
	{
		String text = "> CAPTURED_";
		int fontFace = CV_FONT_HERSHEY_TRIPLEX;
		double fontScale = 0.7;
		int thickness = 1;
		int baseline = 0;
		Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
		baseline += thickness;
		Point textOrg(result.cols - 300, 20 + textSize.height / 2);

		putText(result, text, textOrg, fontFace, fontScale, Scalar(255, 255, 255), thickness, CV_AA);

		auto captured = *oni->mTracker->getCaptured().rbegin();
		Mat wanted(wanted_base.rows, wanted_base.cols, wanted_base.type());
		wanted_base.copyTo(wanted);
		Mat target;
		resize(captured, target, Size(325, 270));
		target.copyTo(wanted(Rect(Point(40, 140), Size(325, 270))));

		Mat channels[3];
		cvtColor(wanted, wanted, CV_BGR2HSV);
		split(wanted, channels);
		channels[0].release();
		channels[0] = Mat::zeros(wanted.rows, wanted.cols, CV_8UC1);
		merge(channels, 3, wanted);
		cvtColor(wanted, wanted, CV_HSV2BGR);

		resize(wanted, wanted, Size(180, 320));

		wanted.copyTo(result(Rect(Point((result.cols - wanted.cols) / 2, (result.rows - wanted.rows) / 2), wanted.size())));
	}

	// Print landing message
	if (oni->mStateController->getState() == StateController::STATE_LANDING)
	{
		String text = "> LANDING_";
		int fontFace = CV_FONT_HERSHEY_TRIPLEX;
		double fontScale = 0.7;
		int thickness = 1;
		int baseline = 0;
		Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
		baseline += thickness;
		Point textOrg(result.cols - 300, 20 + textSize.height / 2);

		putText(result, text, textOrg, fontFace, fontScale, Scalar(255, 255, 255), thickness, CV_AA);
	}

	imshow(COOL_SCREEN_WINDOW_NAME, result);
	result.release();
}

DWORD WINAPI Oni::user_command_loop(LPVOID lpParam)
{
	auto oni = static_cast<Oni*>(lpParam);
	if (oni == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_FATAL, TAG, "Oni is NULL.");
		return -1;
	}

	Sleep(2000);

	auto wanted_base = cv::imread("tehai.png");

	while (oni->mStateController->getState() != StateController::STATE_FINISHED)
	{
		processCoolScreen(oni, wanted_base);

		auto commandKey = cv::waitKey(100);

		switch (commandKey)
		{
		case 'E': // Emergency
		case 'e':
		{
			oni->mReceivedCommand = OniCommand::Emergency;
		}
		break;

		case 'T': // Take Off
		case 't':
		{
			oni->mReceivedCommand = OniCommand::TakeOff;
		}
		break;

		case 'S': // Search
		case 's':
		{
			oni->mReceivedCommand = OniCommand::Search;
		}
		break;

		case 'L': // Land
		case 'l':
		{
			oni->mReceivedCommand = OniCommand::Land;
		}
		break;

		case 'D': // Disconnect
		case 'd':
		{
			oni->mReceivedCommand = OniCommand::Disconnect;
		}
		default:
			break;
		}
	}

	return 0;
}

void Oni::processStateStart(StateController::STATE_PARAMETER*& currentParameter)
{
	auto param = dynamic_cast<StateController::STATE_PARAMETER_START*>(currentParameter);
	if (param == nullptr)
	{
		delete currentParameter;
		currentParameter = new StateController::STATE_PARAMETER_START;
	}
	else
	{
		param->connected = true;
	}
}

void Oni::processStateReady(StateController::STATE_PARAMETER*& currentParameter, Oni::OniCommand command)
{
	auto param = dynamic_cast<StateController::STATE_PARAMETER_READY*>(currentParameter);
	if (param == nullptr)
	{
		param = new StateController::STATE_PARAMETER_READY;
		delete currentParameter;
		currentParameter = param;
	}

	switch (command)
	{
	case TakeOff:
		param->command = StateController::STATE_PARAMETER_READY::COMMAND_TAKEOFF;
		break;
	case Disconnect:
		param->command = StateController::STATE_PARAMETER_READY::COMMAND_DISCONNECT;
		break;
	case None:
	case Emergency:
	case Search:
	case Land:
	default:
		break;
	}
}

void Oni::processStateTakingOff(StateController::STATE_PARAMETER*& currentParameter)
{
	auto param = dynamic_cast<StateController::STATE_PARAMETER_TAKINGOFF*>(currentParameter);
	if (param == nullptr)
	{
		param = new StateController::STATE_PARAMETER_TAKINGOFF(GetTickCount64());
		delete currentParameter;;
		currentParameter = param;
	}
}

void Oni::processStateHovering(StateController::STATE_PARAMETER*& currentParameter, Oni::OniCommand command)
{
	auto param = dynamic_cast<StateController::STATE_PARAMETER_HOVERING*>(currentParameter);
	if (param == nullptr)
	{
		param = new StateController::STATE_PARAMETER_HOVERING(StateController::STATE_PARAMETER_HOVERING::COMMAND_NONE);
		delete currentParameter;
		currentParameter = param;
	}

	switch (command)
	{
	case Search:
		param->command = StateController::STATE_PARAMETER_HOVERING::COMMAND_SEARCH;
		break;
	case Land:
		param->command = StateController::STATE_PARAMETER_HOVERING::COMMAND_LAND;
		break;
	case None:
	case Emergency:
	case TakeOff:
	default:
		break;
	}
}

void Oni::processStateSearching(Oni* oni, StateController::STATE_PARAMETER*& currentParameter)
{
	auto param = dynamic_cast<StateController::STATE_PARAMETER_SEARCHING*>(currentParameter);
	if (param == nullptr)
	{
		param = new StateController::STATE_PARAMETER_SEARCHING(GetTickCount64(), false);
		delete currentParameter;
		currentParameter = param;
	}

	auto image = oni->getCameraImage(oni->mTracker->resize_rate, oni->mTracker->resize_rate);

	auto peopleList = oni->mTracker->getPeople(image);

	bool found = !peopleList.empty();

	param->found = found;
	if(found)
	{
		auto newRect = cv::Rect(
			peopleList[0].tl() / oni->mTracker->resize_rate,
			cv::Size(peopleList[0].width / oni->mTracker->resize_rate, peopleList[0].height / oni->mTracker->resize_rate));
		oni->mDroneStatus->currentTarget = newRect;
	}
	image.release();
}

void Oni::processStateTracking(Oni* oni, StateController::STATE_PARAMETER*& currentParameter)
{
	auto param = dynamic_cast<StateController::STATE_PARAMETER_TRACKING*>(currentParameter);
	if (param == nullptr)
	{
		param = new StateController::STATE_PARAMETER_TRACKING(oni->mTracker, StateController::STATE_PARAMETER_TRACKING::STATUS_NONE, StateController::STATE_PARAMETER_TRACKING::DIRECTION_NONE);
		delete currentParameter;
		currentParameter = param;
	}

	auto image = oni->getCameraImage(oni->mTracker->resize_rate, oni->mTracker->resize_rate);
	auto peopleList = oni->mTracker->getPeople(image);

	if(peopleList.empty())
	{
		param->status = StateController::STATE_PARAMETER_TRACKING::STATUS_MISSED;
	}
	else
	{
		int trackingPerson = 0;
		auto person = peopleList[trackingPerson];
		auto newRect = cv::Rect(person.tl() / oni->mTracker->resize_rate, cv::Size(person.width / oni->mTracker->resize_rate, person.height / oni->mTracker->resize_rate));
		oni->mDroneStatus->currentTarget = newRect;

		if (oni->mTracker->isPersonInBorder(image, person))
		{
			param->status = StateController::STATE_PARAMETER_TRACKING::STATUS_CAPTURED;
			oni->mTracker->addCaptured(image, person);
			printf("STATUS_CAPTURED\n");
		}
		else
		{
			param->status = StateController::STATE_PARAMETER_TRACKING::STATUS_FOUND;

			double leftBorder = image.cols / 3.0;
			double rightBorder = image.cols * 2.0 / 3.0;
			double personLocation = person.x + person.width/2;

			if (personLocation < leftBorder)
			{
				param->direction = StateController::STATE_PARAMETER_TRACKING::DIRECTION_LEFT;
				printf("STATUS_FOUND: DIRECTION_LEFT\n");
			}
			else if (rightBorder < personLocation)
			{
				param->direction = StateController::STATE_PARAMETER_TRACKING::DIRECTION_RIGHT;
				printf("STATUS_FOUND: DIRECTION_RIGHT\n");
			}
			else if (leftBorder <= personLocation && personLocation <= rightBorder)
			{
				param->direction = StateController::STATE_PARAMETER_TRACKING::DIRECTION_FORWARD;
				printf("STATUS_FOUND: DIRECTION_FORWARD\n");
			}
		}
	}
}

void Oni::processStateMissing(Oni* oni, StateController::STATE_PARAMETER*& currentParameter)
{
	auto param = dynamic_cast<StateController::STATE_PARAMETER_MISSING*>(currentParameter);
	if (param == nullptr)
	{
		param = new StateController::STATE_PARAMETER_MISSING(oni->mTracker, GetTickCount64(), false);
		delete currentParameter;
		currentParameter = param;
	}

	auto image = oni->getCameraImage(oni->mTracker->resize_rate, oni->mTracker->resize_rate);

	auto peopleList = oni->mTracker->getPeople(image);

	param->found = !peopleList.empty();
}

void Oni::processStateCaptured(Oni* oni, StateController::STATE_PARAMETER*& currentParameter)
{
	auto param = dynamic_cast<StateController::STATE_PARAMETER_CAPTURED*>(currentParameter);
	if (param == nullptr)
	{
		param = new StateController::STATE_PARAMETER_CAPTURED(oni->mTracker, GetTickCount64());
		delete currentParameter;
		currentParameter = param;
	}
}

void Oni::processStateLanding(Oni* oni, StateController::STATE_PARAMETER*& currentParameter)
{
	auto param = dynamic_cast<StateController::STATE_PARAMETER_LANDING*>(currentParameter);
	if (param == nullptr)
	{
		param = new StateController::STATE_PARAMETER_LANDING(GetTickCount64());
		delete currentParameter;
		currentParameter = param;

		oni->mTracker->clearCaptured();
	}
}

void Oni::processStateFinished(Oni* oni, StateController::STATE_PARAMETER*& currentParameter)
{
	auto param = dynamic_cast<StateController::STATE_PARAMETER_FINISHED*>(currentParameter);
	if (param == nullptr)
	{
		param = new StateController::STATE_PARAMETER_FINISHED;
		delete currentParameter;
		currentParameter = param;
	}

	oni->mStateController->processState(currentParameter);
}

DWORD WINAPI Oni::oni_state_loop(LPVOID lpParam)
{
	auto oni = static_cast<Oni*>(lpParam);
	if (oni == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_FATAL, TAG, "Oni is NULL.");
		return -1;
	}

	static StateController::STATE_PARAMETER* currentParameter = nullptr;

	while (true)
	{
		auto command = oni->mReceivedCommand;
		oni->mReceivedCommand = None;
		if(command == Emergency)
		{
			oni->mStateController->setState(StateController::STATE_EMERGENCY);
			oni->mStateController->processState(nullptr);
			continue;
		}

		auto state = oni->mStateController->getState();
		switch (state)
		{
		case StateController::STATE_EMERGENCY:
			delete currentParameter;
			currentParameter = nullptr;
		break;
		case StateController::STATE_START:
			processStateStart(currentParameter);
		break;
		case StateController::STATE_READY:
			processStateReady(currentParameter, command);
		break;
		case StateController::STATE_TAKINGOFF:
			processStateTakingOff(currentParameter);
		break;
		case StateController::STATE_HOVERING:
			processStateHovering(currentParameter, command);
		break;
		case StateController::STATE_SEARCHING:
		{
			if (command == Land)
			{
				oni->mStateController->setState(StateController::STATE_LANDING);
				delete currentParameter;
				currentParameter = nullptr;
				oni->mStateController->processState(currentParameter);
				continue;
			}

			processStateSearching(oni, currentParameter);
		}
		break;
		case StateController::STATE_TRACKING:
		{
			if (command == Land)
			{
				oni->mStateController->setState(StateController::STATE_LANDING);
				delete currentParameter;
				currentParameter = nullptr;
				oni->mStateController->processState(currentParameter);
				continue;
			}

			processStateTracking(oni, currentParameter);
		}
		break;
		case StateController::STATE_MISSING:
		{
			if (command == Land)
			{
				oni->mStateController->setState(StateController::STATE_LANDING);
				delete currentParameter;
				currentParameter = nullptr;
				oni->mStateController->processState(currentParameter);
				continue;
			}

			processStateMissing(oni, currentParameter);
		}
		break;
		case StateController::STATE_CAPTURED:
		{
			if (command == Land)
			{
				oni->mStateController->setState(StateController::STATE_LANDING);
				delete currentParameter;
				currentParameter = nullptr;
				oni->mStateController->processState(currentParameter);
				continue;
			}

			processStateCaptured(oni, currentParameter);
		}
		break;
		case StateController::STATE_LANDING:
			processStateLanding(oni, currentParameter);
			break;
		case StateController::STATE_FINISHED:
			processStateFinished(oni, currentParameter);
			return 0;
		default: break;
		}

		oni->mStateController->processState(currentParameter);
		Sleep(50);
	}

	return 0;
}

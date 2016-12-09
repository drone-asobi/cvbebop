#define TAG "Oni"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "Oni.h"

void Oni::oni_event_loop(eARCONTROLLER_DICTIONARY_KEY commandKey, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary, void *customData)
{
	auto *deviceController = static_cast<ARCONTROLLER_Device_t*>(customData);

	if (deviceController == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, " DeviceController is NULL!");
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

	return resized;
}

DWORD WINAPI Oni::user_command_loop(LPVOID lpParam)
{
	auto oni = static_cast<Oni*>(lpParam);
	if (oni == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_FATAL, TAG, "Oni is NULL.");
		return -1;
	}

	auto controllerImage = cv::imread(CONTROLLER_IMAGE_FILE);

	while (oni->mStateController->getState() != StateController::STATE_FINISHED)
	{
		cv::imshow(CONTROLLER_WINDOW_NAME, controllerImage);

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

DWORD WINAPI Oni::oni_state_loop(LPVOID lpParam)
{
	auto oni = static_cast<Oni*>(lpParam);
	if (oni == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_FATAL, TAG, "Oni is NULL.");
		return -1;
	}

	auto tick = GetTickCount64();

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
		{
			oni->mStateController->processState(nullptr);
		}
		break;
		case StateController::STATE_START:
		{
			oni->mStateController->processState(new StateController::STATE_PARAMETER_START(true));
		}
		break;
		case StateController::STATE_READY:
		{
			switch (command)
			{
			case TakeOff:
				oni->mStateController->processState(
					new StateController::STATE_PARAMETER_READY(
						StateController::STATE_PARAMETER_READY::COMMAND_TAKEOFF
					));
				tick = GetTickCount64();
				break;
			case Disconnect:
				oni->mStateController->processState(
					new StateController::STATE_PARAMETER_READY(
						StateController::STATE_PARAMETER_READY::COMMAND_DISCONNECT
					));
				break;
			case None:
			case Emergency:
			case Search:
			case Land:
			default:
				oni->mStateController->processState(
					new StateController::STATE_PARAMETER_READY()
				);
				break;
			}
		}
		break;
		case StateController::STATE_TAKINGOFF:
		{
			oni->mStateController->processState(
				new StateController::STATE_PARAMETER_TAKINGOFF(tick)
			);
		}
		break;
		case StateController::STATE_HOVERING:
		{
			switch (command)
			{
			case Search:
				oni->mStateController->processState(
					new StateController::STATE_PARAMETER_HOVERING(
						StateController::STATE_PARAMETER_HOVERING::COMMAND_SEARCH
					));
				tick = GetTickCount64();
				break;
			case Land:
				oni->mStateController->processState(
					new StateController::STATE_PARAMETER_HOVERING(
						StateController::STATE_PARAMETER_HOVERING::COMMAND_LAND
					));
				tick = GetTickCount64();
				break;
			case None:
			case Emergency:
			case TakeOff:
			default:
				oni->mStateController->processState(
					new StateController::STATE_PARAMETER_HOVERING()
				);
				break;
			}
		}
		break;
		case StateController::STATE_SEARCHING:
		{
			auto image = oni->getCameraImage(0.3, 0.3);

			// TODO: Trackerが人を検出したらfoundをtrueにする
			// 例えば、下のように実装する。
			auto peopleList = oni->mTracker->getPeople(image);

			bool found = !peopleList.empty();

			cv::imshow("debug_search", image);
			cv::waitKey(1);

			oni->mStateController->processState(
				new StateController::STATE_PARAMETER_SEARCHING(tick, found)
			);
		}
		break;
		case StateController::STATE_TRACKING:
		{
			auto image = oni->getCameraImage(0.3, 0.3);

			auto status = StateController::STATE_PARAMETER_TRACKING::STATUS_NONE;
			auto direction = StateController::STATE_PARAMETER_TRACKING::DIRECTION_NONE;

			// TODO: Trackerからの情報を用いてドローンをどのように動かすか決める
			// 例えば、下のように実装する。
			auto peopleList = oni->mTracker->getPeople(image);

			cv::imshow("debug_search", image);
			cv::waitKey(1);

			if(peopleList.empty())
			{
				status = StateController::STATE_PARAMETER_TRACKING::STATUS_MISSED;
			}
			else
			{
				int trackingPerson = 0;
				auto person = peopleList[trackingPerson];

				if (oni->mTracker->isPersonInBorder(image, person))
				{
					status = StateController::STATE_PARAMETER_TRACKING::STATUS_CAPTURED;

					cv::Rect rect(person.tl().x,person.tl().y,person.br().x - person.tl().x,person.br().y - person.tl().y);
					cv::Mat imgSub(image,rect);	//人領域
                    cvtColor(imgSub, imgSub, CV_RGB2GRAY);
					cv::Mat imgSub2;
					cv::resize(imgSub,imgSub2,cv::Size(325,270),0,0);
					cv::Mat base = cv::imread("tehai.png",1);
					cv::Mat comb(cv::Size(base.cols,base.rows),CV_8UC3);
					cv::Mat im1(comb,cv::Rect(0,0,base.cols,base.rows));
					cv::Mat im2(comb,cv::Rect(40,140,imgSub2.cols,imgSub2.rows));
					base.copyTo(im1);
					imgSub2.copyTo(im2);
					cv::resize(comb,comb,cv::Size(180,320));
					imshow("tehai",comb);
					printf("STATUS_CAPTURED\n");
				}
				else
				{
					status = StateController::STATE_PARAMETER_TRACKING::STATUS_FOUND;

					double leftBorder = image.cols / 3.0;
					double rightBorder = image.cols * 2.0 / 3.0;
					double personLocation = person.x + person.width/2;
					
					if (personLocation < leftBorder)
					{
						direction = StateController::STATE_PARAMETER_TRACKING::DIRECTION_LEFT;
						printf("STATUS_FOUND: DIRECTION_LEFT\n");
					}
					else if (rightBorder < personLocation)
					{
						direction = StateController::STATE_PARAMETER_TRACKING::DIRECTION_RIGHT;
						printf("STATUS_FOUND: DIRECTION_RIGHT\n");
					}
					else if (leftBorder <= personLocation && personLocation <= rightBorder)
					{
						direction = StateController::STATE_PARAMETER_TRACKING::DIRECTION_FORWARD;
						printf("STATUS_FOUND: DIRECTION_FORWARD\n");
					}
				}
			}

			oni->mStateController->processState(
				new StateController::STATE_PARAMETER_TRACKING(
					nullptr,
					status,
					direction
				));
			tick = GetTickCount64();
		}
		break;
		case StateController::STATE_MISSING:
		{
			auto image = oni->getCameraImage();

			// TODO: Trackerが人を検出したらfoundをtrueにする
			// 例えば、下のように実装する。
			auto peopleList = oni->mTracker->getPeople(image);

			bool found = !peopleList.empty();

			cv::imshow("debug_search", image);
			cv::waitKey(1);

			oni->mStateController->processState(
				new StateController::STATE_PARAMETER_MISSING(
					nullptr,
					tick,
					found
				));
		}
		break;
		case StateController::STATE_LANDING:
		{
			oni->mStateController->processState(
				new StateController::STATE_PARAMETER_LANDING(tick)
			);
		}
		break;
		case StateController::STATE_FINISHED:
		{
			return 0;
		}
		default: break;
		}

		Sleep(50);
	}

	return 0;
}

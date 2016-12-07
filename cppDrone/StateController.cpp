#define TAG "StateController"

#include "StateController.h"

void StateController::processState(STATE_PARAMETER* arg)
{
	static ULONGLONG startedTick = 0;

	auto state = getState();
	switch (state)
	{
	case STATE_EMERGENCY:
	{
		ARSAL_PRINT(ARSAL_PRINT_FATAL, TAG, "State[Emergency]: Stop!!!!!!!!!!!!!");
		deviceController->aRDrone3->sendPilotingEmergency(deviceController->aRDrone3);
		setState(STATE_FINISHED);
	}
	break;
	case STATE_START:
	{
		auto param = static_cast<STATE_PARAMETER_START*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Start]: No parameters!");
			break;
		}

		if (param->connected)
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Start => Ready]: Waiting for a take-off command...");
			setState(STATE_READY);
		}
	}
	break;
	case STATE_READY:
	{
		auto param = static_cast<STATE_PARAMETER_READY*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Ready]: No parameters!");
			break;
		}

		switch (param->command)
		{
		case STATE_PARAMETER_READY::COMMAND_NONE:
		{
			// Ready. do nothing.
		}
		break;
		case STATE_PARAMETER_READY::COMMAND_TAKEOFF:
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Ready => TakingOff]: Try to take off.");
			deviceController->aRDrone3->sendPilotingTakeOff(deviceController->aRDrone3);
			setState(STATE_TAKINGOFF);
		}
		break;
		case STATE_PARAMETER_READY::COMMAND_DISCONNECT:
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Ready => Finished]: Try to disconnect.");
			setState(STATE_FINISHED);
		}
		break;
		default: break;
		}
	}
	break;
	case STATE_TAKINGOFF:
	{
		auto param = static_cast<STATE_PARAMETER_TAKINGOFF*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[TakingOff]: No parameters!");
			break;
		}

		if (GetTickCount64() - param->takenOffTick >= TAKING_OFF_WAIT_TICK)
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[TakingOff => Hovering]: Completed taking off.");
			deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
			setState(STATE_HOVERING);
		}
		else
		{
			// Go up!
			deviceController->aRDrone3->setPilotingPCMDGaz(deviceController->aRDrone3, 100);
		}
	}
	break;
	case STATE_HOVERING:
	{
		auto param = static_cast<STATE_PARAMETER_HOVERING*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Hovering]: No parameters!");
			break;
		}

		switch (param->command)
		{
		case STATE_PARAMETER_HOVERING::COMMAND_NONE:
		{
			// waiting for commands...
			// ... and no move.
			deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
		}
		break;
		case STATE_PARAMETER_HOVERING::COMMAND_SEARCH:
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Hovering => Searching]: Start to search people...");
			setState(STATE_SEARCHING);
		}
		break;
		case STATE_PARAMETER_HOVERING::COMMAND_LAND:
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Hovering => Landing]: Try to land.");
			deviceController->aRDrone3->sendPilotingLanding(deviceController->aRDrone3);
			setState(STATE_LANDING);
		}
		break;
		default:
		{
		}
		break;
		}
	}
	break;
	case STATE_SEARCHING:
	{
		auto param = static_cast<STATE_PARAMETER_SEARCHING*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Searching]: No parameters!");
			break;
		}

		if (param->found)
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Searching => Tracking]: Found a person! Try to track.");
			deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
			setState(STATE_TRACKING);
		}
		else
		{
			if (GetTickCount64() - param->startedTick >= SEARCHING_WAIT_TICK)
			{
				ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Searching => Hovering]: No more people. Finish searching.");
				setState(STATE_HOVERING);
			}
			else
			{
				// Rotate to right
				deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, 100);
			}
		}
	}
	break;
	case STATE_TRACKING:
	{
		auto param = static_cast<STATE_PARAMETER_TRACKING*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Tracking]: No parameters!");
			break;
		}

		switch (param->status)
		{
		case STATE_PARAMETER_TRACKING::STATUS_NONE:
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Tracking]: Status is None!");
			deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
		}
		break;
		case STATE_PARAMETER_TRACKING::STATUS_FOUND:
		{
			switch (param->direction)
			{
			case STATE_PARAMETER_TRACKING::DIRECTION_NONE:
			{
				// do nothing
				ARSAL_PRINT(ARSAL_PRINT_DEBUG, TAG, "State[Tracking]: Found, but wait.");
				deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
			}
			break;
			case STATE_PARAMETER_TRACKING::DIRECTION_FORWARD:
			{
				// go forward
				ARSAL_PRINT(ARSAL_PRINT_DEBUG, TAG, "State[Tracking]: Found, go forward");
				deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
				deviceController->aRDrone3->setPilotingPCMDPitch(deviceController->aRDrone3, 100);
			}
			break;
			case STATE_PARAMETER_TRACKING::DIRECTION_LEFT:
			{
				// turn left
				ARSAL_PRINT(ARSAL_PRINT_DEBUG, TAG, "State[Tracking]: Found, turn left");
				deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, -100);
			}
			break;
			case STATE_PARAMETER_TRACKING::DIRECTION_RIGHT:
			{
				// turn right
				ARSAL_PRINT(ARSAL_PRINT_DEBUG, TAG, "State[Tracking]: Found, turn right");
				deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, 100);
			}
			break;
			default: break;
			}
		}
		break;
		case STATE_PARAMETER_TRACKING::STATUS_MISSED:
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Tracking => Missing]: Missed a person!");
			deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
			setState(STATE_MISSING);
		}
		break;
		case STATE_PARAMETER_TRACKING::STATUS_CAPTURED:
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Tracking => Searching]: Captured a person! Back to searching");
			// TODO: ’ÇÕ‚ªI‚í‚Á‚½‚çƒhƒ[ƒ“‚ð‚Ç‚Ì‚æ‚¤‚É“®‚©‚·‚©Œˆ‚ß‚é
			deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
			setState(STATE_SEARCHING);
		}
		break;
		default: break;
		}
	}
	break;
	case STATE_MISSING:
	{
		auto param = static_cast<STATE_PARAMETER_MISSING*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Missing]: No parameters!");
			break;
		}

		deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
		if (param->found)
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Missing => Tracking]: Found the person. Back to tracking");
			setState(STATE_TRACKING);
		}
		else
		{
			if (GetTickCount64() - param->missedTick >= MISSING_WAIT_TICK)
			{
				ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Missing => Searching]: Failed to find the person. Back to searching");
				setState(STATE_SEARCHING);
			}
			else
			{
				// wait for finding the person
			}
		}
	}
	break;
	case STATE_LANDING:
	{
		auto param = static_cast<STATE_PARAMETER_LANDING*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Landing]: No parameters!");
			break;
		}

		if (GetTickCount64() - param->landedTick >= LANDING_WAIT_TICK)
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Landing => Ready]: Completed landing. Get ready.");
			setState(STATE_READY);
		}
		else
		{
			// wait for landing...
		}
	}
	break;
	case STATE_FINISHED:
	{
		auto param = static_cast<STATE_PARAMETER_FINISHED*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Finished]: No parameters!");
			break;
		}

		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Finished]: Disconnected. Bye-bye!");
	}
	break;
	default:
	{
	}
	break;
	}
}

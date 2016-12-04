#define TAG "state_machine"

#include "state_machine.h"

void BEBOP2_STATE_MACHINE::process_bebop2_state(BEBOP2_STATE_PARAMETER* arg)
{
	static ULONGLONG startedTick = 0;

	auto state = get_bebop2_state();
	switch (state)
	{
	case BEBOP2_STATE_EMERGENCY:
	{
		ARSAL_PRINT(ARSAL_PRINT_FATAL, TAG, "State[Emergency]: Stop!!!!!!!!!!!!!");
		deviceController->aRDrone3->sendPilotingEmergency(deviceController->aRDrone3);
	}
	break;
	case BEBOP2_STATE_START:
	{
		auto param = static_cast<BEBOP2_STATE_PARAMETER_START*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Start]: No parameters!");
			break;
		}

		if (param->connected)
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Start => Ready]: Waiting for a take-off command...");
			set_bebop2_state(BEBOP2_STATE_READY);
		}
	}
	break;
	case BEBOP2_STATE_READY:
	{
		auto param = static_cast<BEBOP2_STATE_PARAMETER_READY*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Ready]: No parameters!");
			break;
		}

		switch (param->command)
		{
		case BEBOP2_STATE_PARAMETER_READY::BEBOP2_STATE_PARAMETER_READY_COMMAND_NONE:
		{
			// Ready. do nothing.
		}
		break;
		case BEBOP2_STATE_PARAMETER_READY::BEBOP2_STATE_PARAMETER_READY_COMMAND_TAKEOFF:
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Ready => TakingOff]: Try to take off.");
			deviceController->aRDrone3->sendPilotingTakeOff(deviceController->aRDrone3);
			set_bebop2_state(BEBOP2_STATE_TAKINGOFF);
		}
		break;
		case BEBOP2_STATE_PARAMETER_READY::BEBOP2_STATE_PARAMETER_READY_COMMAND_DISCONNECT:
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Ready => Finished]: Try to disconnect.");
			set_bebop2_state(BEBOP2_STATE_FINISHED);
		}
		break;
		default: break;
		}
	}
	break;
	case BEBOP2_STATE_TAKINGOFF:
	{
		auto param = static_cast<BEBOP2_STATE_PARAMETER_TAKINGOFF*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[TakingOff]: No parameters!");
			break;
		}

		if (GetTickCount64() - param->takenOffTick >= BEBOP2_TAKING_OFF_WAIT_TICK)
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[TakingOff => Hovering]: Completed taking off.");
			deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
			set_bebop2_state(BEBOP2_STATE_HOVERING);
		}
		else
		{
			// Go up!
			deviceController->aRDrone3->setPilotingPCMDGaz(deviceController->aRDrone3, 100);
		}
	}
	break;
	case BEBOP2_STATE_HOVERING:
	{
		auto param = static_cast<BEBOP2_STATE_PARAMETER_HOVERING*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Hovering]: No parameters!");
			break;
		}

		switch (param->command)
		{
		case BEBOP2_STATE_PARAMETER_HOVERING::BEBOP2_STATE_PARAMETER_HOVERING_COMMAND_NONE:
		{
			// waiting for commands...
			// ... and no move.
			deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
		}
		break;
		case BEBOP2_STATE_PARAMETER_HOVERING::BEBOP2_STATE_PARAMETER_HOVERING_COMMAND_SEARCH:
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Hovering => Searching]: Start to search people...");
			set_bebop2_state(BEBOP2_STATE_SEARCHING);
		}
		break;
		case BEBOP2_STATE_PARAMETER_HOVERING::BEBOP2_STATE_PARAMETER_HOVERING_COMMAND_LAND:
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Hovering => Landing]: Try to land.");
			deviceController->aRDrone3->sendPilotingLanding(deviceController->aRDrone3);
			set_bebop2_state(BEBOP2_STATE_SEARCHING);
		}
		break;
		default:
		{
		}
		break;
		}
	}
	break;
	case BEBOP2_STATE_SEARCHING:
	{
		auto param = static_cast<BEBOP2_STATE_PARAMETER_SEARCHING*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Searching]: No parameters!");
			break;
		}

		if (param->found)
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Searching => Tracking]: Found a person! Try to track.");
			deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
			set_bebop2_state(BEBOP2_STATE_TRACKING);
		}
		else
		{
			if (GetTickCount64() - param->startedTick >= BEBOP2_SEARCHING_WAIT_TICK)
			{
				ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Searching => Hovering]: No more people. Finish searching.");
				set_bebop2_state(BEBOP2_STATE_HOVERING);
			}
			else
			{
				// Rotate to right
				deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, 100);
			}
		}
	}
	break;
	case BEBOP2_STATE_TRACKING:
	{
		auto param = static_cast<BEBOP2_STATE_PARAMETER_TRACKING*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Tracking]: No parameters!");
			break;
		}

		switch (param->status)
		{
		case BEBOP2_STATE_PARAMETER_TRACKING::BEBOP2_STATE_PARAMETER_TRACKING_STATUS_NONE:
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Tracking]: Status is None!");
			deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
		}
		break;
		case BEBOP2_STATE_PARAMETER_TRACKING::BEBOP2_STATE_PARAMETER_TRACKING_STATUS_FOUND:
		{
			switch (param->direction)
			{
			case BEBOP2_STATE_PARAMETER_TRACKING::BEBOP2_STATE_PARAMETER_TRACKING_DIRECTION_NONE:
			{
				// do nothing
				ARSAL_PRINT(ARSAL_PRINT_DEBUG, TAG, "State[Tracking]: Found, but wait.");
				deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
			}
			break;
			case BEBOP2_STATE_PARAMETER_TRACKING::BEBOP2_STATE_PARAMETER_TRACKING_DIRECTION_FORWARD:
			{
				// go forward
				ARSAL_PRINT(ARSAL_PRINT_DEBUG, TAG, "State[Tracking]: Found, go forward");
				deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
				deviceController->aRDrone3->setPilotingPCMDPitch(deviceController->aRDrone3, 100);
			}
			break;
			case BEBOP2_STATE_PARAMETER_TRACKING::BEBOP2_STATE_PARAMETER_TRACKING_DIRECTION_LEFT:
			{
				// turn left
				ARSAL_PRINT(ARSAL_PRINT_DEBUG, TAG, "State[Tracking]: Found, turn left");
				deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, -100);
			}
			break;
			case BEBOP2_STATE_PARAMETER_TRACKING::BEBOP2_STATE_PARAMETER_TRACKING_DIRECTION_RIGHT:
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
		case BEBOP2_STATE_PARAMETER_TRACKING::BEBOP2_STATE_PARAMETER_TRACKING_STATUS_MISSED:
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Tracking => MISSING]: Missed a person!");
			deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
			set_bebop2_state(BEBOP2_STATE_MISSING);
		}
		break;
		case BEBOP2_STATE_PARAMETER_TRACKING::BEBOP2_STATE_PARAMETER_TRACKING_STATUS_CAPTURED:
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Tracking => Searching]: Captured a person! Back to searching");
			// TODO: ’ÇÕ‚ªI‚í‚Á‚½‚çƒhƒ[ƒ“‚ð‚Ç‚Ì‚æ‚¤‚É“®‚©‚·‚©Œˆ‚ß‚é
			deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
			set_bebop2_state(BEBOP2_STATE_SEARCHING);
		}
		break;
		default: break;
		}
	}
	break;
	case BEBOP2_STATE_MISSING:
	{
		auto param = static_cast<BEBOP2_STATE_PARAMETER_MISSING*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Missing]: No parameters!");
			break;
		}

		deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
		if (param->found)
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Missing => Tracking]: Found the person. Back to tracking");
			set_bebop2_state(BEBOP2_STATE_TRACKING);
		}
		else
		{
			if (GetTickCount64() - param->missedTick >= BEBOP2_MISSING_WAIT_TICK)
			{
				ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Missing => Searching]: Failed to find the person. Back to searching");
				set_bebop2_state(BEBOP2_STATE_MISSING);
			}
			else
			{
				// wait for finding the person
			}
		}
	}
	break;
	case BEBOP2_STATE_LANDING:
	{
		auto param = static_cast<BEBOP2_STATE_PARAMETER_LANDING*>(arg);
		if (param == nullptr)
		{
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Landing]: No parameters!");
			break;
		}

		if (GetTickCount64() - param->landedTick >= BEBOP2_LANDING_WAIT_TICK)
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Landing => Ready]: Completed landing. Get ready.");
			set_bebop2_state(BEBOP2_STATE_READY);
		}
		else
		{
			// wait for landing...
		}
	}
	break;
	case BEBOP2_STATE_FINISHED:
	{
		auto param = static_cast<BEBOP2_STATE_PARAMETER_FINISHED*>(arg);
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

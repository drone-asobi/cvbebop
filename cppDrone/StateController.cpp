#define TAG "StateController"

#include "StateController.h"

void StateController::processStateEmergency()
{
	ARSAL_PRINT(ARSAL_PRINT_FATAL, TAG, "State[Emergency]: Stop!!!!!!!!!!!!!");
	deviceController->aRDrone3->sendPilotingEmergency(deviceController->aRDrone3);
	setState(STATE_FINISHED);
}

void StateController::processStateStart(STATE_PARAMETER_START * param)
{
	if (param == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Start]: No parameters!");
		return;
	}

	if (param->connected)
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Start => Ready]: Oni is connected. Waiting for a command");
		setState(STATE_READY);
	}
}

void StateController::processStateReady(STATE_PARAMETER_READY * param)
{
	if (param == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Ready]: No parameters!");
		return;
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
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Ready => TakingOff]: Received the 'take-off' command.");
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

void StateController::processStateTakingOff(STATE_PARAMETER_TAKINGOFF * param)
{
	if (param == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[TakingOff]: No parameters!");
		return;
	}

	if (!param->status_takenOff)
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[TakingOff]: Try to take off...");
		deviceController->aRDrone3->sendPilotingTakeOff(deviceController->aRDrone3);
		param->status_takenOff = true;
	}

	if (GetTickCount64() - param->takenOffTick >= TAKING_OFF_WAIT_TICK)
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[TakingOff => Hovering]: Completed taking off.");
		setState(STATE_HOVERING);
	}
	else
	{
		// Go up!
		deviceController->aRDrone3->setPilotingPCMDGaz(deviceController->aRDrone3, 100);
	}
}

void StateController::processStateHovering(STATE_PARAMETER_HOVERING * param)
{
	if (param == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Hovering]: No parameters!");
		return;
	}

	if (!param->status_hovered)
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Hovering]: Hovering. Waiting for a command...");
		param->status_hovered = true;
	}

	switch (param->command)
	{
	case STATE_PARAMETER_HOVERING::COMMAND_NONE:
	{
		// Do nothing.
		// waiting for a command...
	}
	break;
	case STATE_PARAMETER_HOVERING::COMMAND_SEARCH:
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Hovering => Searching]: Received the 'search' command.");
		setState(STATE_SEARCHING);
	}
	break;
	case STATE_PARAMETER_HOVERING::COMMAND_LAND:
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Hovering => Landing]: Received the 'land' command.");
		setState(STATE_LANDING);
	}
	break;
	default:
	{
	}
	break;
	}
}

void StateController::processStateSearching(STATE_PARAMETER_SEARCHING * param)
{
	if (param == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Searching]: No parameters!");
		return;
	}

	if (param->found)
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Searching => Tracking]: Oni found a person! Try to track.");
		deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
		setState(STATE_TRACKING);
	}
	else
	{
		if (GetTickCount64() - param->startedTick >= SEARCHING_WAIT_TICK)
		{
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Searching => Hovering]: No more people. Stop searching.");
			deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
			setState(STATE_HOVERING);
		}
		else
		{
			if (!param->status_rotating)
			{
				ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Searching]: Started to rotate to find people...");
				param->status_rotating = true;
			}

			// Rotate to right
			// deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, 20);
		}
	}
}

void StateController::processStateTracking(STATE_PARAMETER_TRACKING * param)
{
	if (param == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Tracking]: No parameters!");
		return;
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
			ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Tracking]: Found, but no direction.");
			deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
		}
		break;
		case STATE_PARAMETER_TRACKING::DIRECTION_FORWARD:
		{
			// go forward
			ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Tracking]: Found, go forward");
			deviceController->aRDrone3->setPilotingPCMDFlag(deviceController->aRDrone3, 1);
			deviceController->aRDrone3->setPilotingPCMDPitch(deviceController->aRDrone3, 20);
		}
		break;
		case STATE_PARAMETER_TRACKING::DIRECTION_LEFT:
		{
			// turn left
			ARSAL_PRINT(ARSAL_PRINT_DEBUG, TAG, "State[Tracking]: Found, turn left");
			deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, -20);
		}
		break;
		case STATE_PARAMETER_TRACKING::DIRECTION_RIGHT:
		{
			// turn right
			ARSAL_PRINT(ARSAL_PRINT_DEBUG, TAG, "State[Tracking]: Found, turn right");
			deviceController->aRDrone3->setPilotingPCMDYaw(deviceController->aRDrone3, 20);
		}
		break;
		default: break;
		}
	}
	break;
	case STATE_PARAMETER_TRACKING::STATUS_MISSED:
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Tracking => Missing]: Missed a person!");
		// deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
		setState(STATE_MISSING);
	}
	break;
	case STATE_PARAMETER_TRACKING::STATUS_CAPTURED:
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Tracking => Captured]: Captured a person!");
		deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);

		setState(STATE_CAPTURED);
	}
	break;
	default: break;
	}
}

void StateController::processStateCaptured(STATE_PARAMETER_CAPTURED * param)
{
	if (param == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Captured]: No parameters!");
		return;
	}

	if (GetTickCount64() - param->capturedTick >= CAPTURED_WAIT_TICK)
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Captured => Hovering]: Captured timeout. Back to hovering.");
		setState(STATE_HOVERING);
	}
}

void StateController::processStateMissing(STATE_PARAMETER_MISSING * param)
{
	if (param == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Missing]: No parameters!");
		return;
	}

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
			// waiting for finding the person...
		}
	}
}

void StateController::processStateLanding(STATE_PARAMETER_LANDING * param)
{
	if (param == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Landing]: No parameters!");
		return;
	}

	if (!param->status_landed)
	{
		ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Landing]: Try to land...");
		deviceController->aRDrone3->sendPilotingLanding(deviceController->aRDrone3);
		param->status_landed = true;
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

void StateController::processStateFinished(STATE_PARAMETER_FINISHED * param)
{
	if (param == nullptr)
	{
		ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "State[Finished]: No parameters!");
		return;
	}

	ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "State[Finished]: Disconnected. Bye-bye!");
}

void StateController::processState(STATE_PARAMETER* arg)
{
	auto state = getState();
	switch (state)
	{
	case STATE_EMERGENCY:
	{
		processStateEmergency();
	}
	break;
	case STATE_START:
	{
		processStateStart(static_cast<STATE_PARAMETER_START*>(arg));
	}
	break;
	case STATE_READY:
	{
		processStateReady(static_cast<STATE_PARAMETER_READY*>(arg));

	}
	break;
	case STATE_TAKINGOFF:
	{
		processStateTakingOff(static_cast<STATE_PARAMETER_TAKINGOFF*>(arg));

	}
	break;
	case STATE_HOVERING:
	{
		// Stop Moving: Top priority behavior than any errors
		deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);

		processStateHovering(static_cast<STATE_PARAMETER_HOVERING*>(arg));
	}
	break;
	case STATE_SEARCHING:
	{
		processStateSearching(static_cast<STATE_PARAMETER_SEARCHING*>(arg));
	}
	break;
	case STATE_TRACKING:
	{
		processStateTracking(static_cast<STATE_PARAMETER_TRACKING*>(arg));
	}
	break;
	case STATE_MISSING:
	{
		// Stop: Top priority behavior than any errors.
		// deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);

		processStateMissing(static_cast<STATE_PARAMETER_MISSING*>(arg));
	}
	break;
	case STATE_CAPTURED:
	{
		// Stop: Top priority behavior than any errors.
		deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);

		processStateCaptured(static_cast<STATE_PARAMETER_CAPTURED*>(arg));
	}
	break;
	case STATE_LANDING:
	{
		deviceController->aRDrone3->setPilotingPCMD(deviceController->aRDrone3, 0, 0, 0, 0, 0, 0);
		processStateLanding(static_cast<STATE_PARAMETER_LANDING*>(arg));
	}
	break;
	case STATE_FINISHED:
	{
		processStateFinished(static_cast<STATE_PARAMETER_FINISHED*>(arg));
	}
	break;
	default:
	{
	}
	break;
	}
}

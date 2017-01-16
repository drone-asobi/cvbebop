#pragma once

extern "C" {
#include "libARController/ARCONTROLLER_Device.h"
}

#define TAKING_OFF_WAIT_TICK 10000
#define SEARCHING_WAIT_TICK 30000
#define MISSING_WAIT_TICK 5000
#define CAPTURED_WAIT_TICK 5000
#define LANDING_WAIT_TICK 5000

class StateController
{
public:
	enum STATE
	{
		STATE_EMERGENCY = -1,
		STATE_START = 0,
		STATE_READY,
		STATE_TAKINGOFF,
		STATE_HOVERING,
		STATE_SEARCHING,
		STATE_TRACKING,
		STATE_MISSING,
		STATE_CAPTURED,
		STATE_LANDING,
		STATE_FINISHED,
	};

	struct STATE_PARAMETER;
	struct STATE_PARAMETER_START;
	struct STATE_PARAMETER_READY;
	struct STATE_PARAMETER_TAKINGOFF;
	struct STATE_PARAMETER_HOVERING;
	struct STATE_PARAMETER_SEARCHING;
	struct STATE_PARAMETER_TRACKING;
	struct STATE_PARAMETER_MISSING;
	struct STATE_PARAMETER_CAPTURED;
	struct STATE_PARAMETER_LANDING;
	struct STATE_PARAMETER_FINISHED;

private:
	STATE currentState;
	ARCONTROLLER_Device_t* deviceController;

public:
	STATE getState() const { return this->currentState; }

	void setState(STATE state) { this->currentState = state; }

	void processState(STATE_PARAMETER* arg);

private:
	void processStateEmergency();
	void processStateStart(STATE_PARAMETER_START* param);
	void processStateReady(STATE_PARAMETER_READY* param);
	void processStateTakingOff(STATE_PARAMETER_TAKINGOFF* param);
	void processStateHovering(STATE_PARAMETER_HOVERING* param);
	void processStateSearching(STATE_PARAMETER_SEARCHING* param);
	void processStateTracking(STATE_PARAMETER_TRACKING* param);
	void processStateMissing(STATE_PARAMETER_MISSING* param);
	void processStateCaptured(STATE_PARAMETER_CAPTURED* param);
	void processStateLanding(STATE_PARAMETER_LANDING* param);
	void processStateFinished(STATE_PARAMETER_FINISHED* param);

public:
	StateController(ARCONTROLLER_Device_t* deviceController) : deviceController(deviceController)
	{
		currentState = STATE_START;
	}
};

struct StateController::STATE_PARAMETER
{
	virtual ~STATE_PARAMETER() { }
};

struct StateController::STATE_PARAMETER_START : STATE_PARAMETER
{
	bool connected;

	STATE_PARAMETER_START(bool connected = false) : connected(connected) { }
};

struct StateController::STATE_PARAMETER_READY : STATE_PARAMETER
{
	enum STATE_PARAMETER_READY_COMMAND
	{
		COMMAND_NONE = 0,
		COMMAND_TAKEOFF,
		COMMAND_DISCONNECT,
	};

	STATE_PARAMETER_READY_COMMAND command;

	STATE_PARAMETER_READY(STATE_PARAMETER_READY_COMMAND command = COMMAND_NONE) : command(command) { }
};

struct StateController::STATE_PARAMETER_TAKINGOFF : STATE_PARAMETER
{
	bool status_takenOff;
	ULONGLONG takenOffTick;

	STATE_PARAMETER_TAKINGOFF(ULONGLONG takenOffTick) : status_takenOff(false), takenOffTick(takenOffTick) { }
};

struct StateController::STATE_PARAMETER_HOVERING : STATE_PARAMETER
{
	enum STATE_PARAMETER_HOVERING_COMMAND
	{
		COMMAND_NONE = 0,
		COMMAND_SEARCH,
		COMMAND_LAND,
	};

	bool status_hovered;
	STATE_PARAMETER_HOVERING_COMMAND command;

	STATE_PARAMETER_HOVERING(STATE_PARAMETER_HOVERING_COMMAND command = COMMAND_NONE) : status_hovered(false), command(command) { }
};

struct StateController::STATE_PARAMETER_SEARCHING : STATE_PARAMETER
{
	bool status_rotating;
	ULONGLONG startedTick;
	bool found;

	STATE_PARAMETER_SEARCHING(ULONGLONG startedTick, bool found) : status_rotating(false), startedTick(startedTick), found(found) { }
};

struct StateController::STATE_PARAMETER_TRACKING : STATE_PARAMETER
{
	enum STATE_PARAMETER_TRACKING_STATUS
	{
		STATUS_NONE = 0,
		STATUS_FOUND,
		STATUS_MISSED,
		STATUS_CAPTURED,
	};

	enum STATE_PARAMETER_TRACKING_DIRECTION
	{
		DIRECTION_NONE = 0,
		DIRECTION_FORWARD,
		DIRECTION_LEFT,
		DIRECTION_RIGHT,
	};

	void* tracker;
	STATE_PARAMETER_TRACKING_STATUS status;
	STATE_PARAMETER_TRACKING_DIRECTION direction;

	STATE_PARAMETER_TRACKING(void* tracker, STATE_PARAMETER_TRACKING_STATUS status, STATE_PARAMETER_TRACKING_DIRECTION direction) : tracker(tracker), status(status), direction(direction) { }
};

struct StateController::STATE_PARAMETER_CAPTURED : STATE_PARAMETER
{
	void* tracker;
	ULONGLONG capturedTick;

	STATE_PARAMETER_CAPTURED(void* tracker, ULONGLONG capturedTick) : tracker(tracker), capturedTick(capturedTick) { }
};

struct StateController::STATE_PARAMETER_MISSING : STATE_PARAMETER
{
	void* tracker;
	ULONGLONG missedTick;
	bool found;

	STATE_PARAMETER_MISSING(void* tracker, ULONGLONG missedTick, bool found) : tracker(tracker), missedTick(missedTick), found(found) { }
};

struct StateController::STATE_PARAMETER_LANDING : STATE_PARAMETER
{
	bool status_landed;
	ULONGLONG landedTick;

	STATE_PARAMETER_LANDING(ULONGLONG landedTick) : status_landed(false), landedTick(landedTick) { }
};

struct StateController::STATE_PARAMETER_FINISHED : STATE_PARAMETER { };

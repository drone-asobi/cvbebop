#pragma once

extern "C" {
#include "libARController/ARCONTROLLER_Device.h"
}

#define BEBOP2_TAKING_OFF_WAIT_TICK 10000
#define BEBOP2_SEARCHING_WAIT_TICK 10000
#define BEBOP2_MISSING_WAIT_TICK 5000
#define BEBOP2_LANDING_WAIT_TICK 5000

class BEBOP2_STATE_MACHINE
{
public:
	enum BEBOP2_STATE
	{
		BEBOP2_STATE_EMERGENCY = -1,
		BEBOP2_STATE_START = 0,
		BEBOP2_STATE_READY,
		BEBOP2_STATE_TAKINGOFF,
		BEBOP2_STATE_HOVERING,
		BEBOP2_STATE_SEARCHING,
		BEBOP2_STATE_TRACKING,
		BEBOP2_STATE_MISSING,
		BEBOP2_STATE_LANDING,
		BEBOP2_STATE_FINISHED,
	};

	struct BEBOP2_STATE_PARAMETER;
	struct BEBOP2_STATE_PARAMETER_START;
	struct BEBOP2_STATE_PARAMETER_READY;
	struct BEBOP2_STATE_PARAMETER_TAKINGOFF;
	struct BEBOP2_STATE_PARAMETER_HOVERING;
	struct BEBOP2_STATE_PARAMETER_SEARCHING;
	struct BEBOP2_STATE_PARAMETER_TRACKING;
	struct BEBOP2_STATE_PARAMETER_MISSING;
	struct BEBOP2_STATE_PARAMETER_LANDING;
	struct BEBOP2_STATE_PARAMETER_FINISHED;

private:
	BEBOP2_STATE currentState;
	ARCONTROLLER_Device_t* deviceController;

public:
	BEBOP2_STATE get_bebop2_state() const { return this->currentState; }

	void set_bebop2_state(BEBOP2_STATE state) { this->currentState = state; }

	void process_bebop2_state(BEBOP2_STATE_PARAMETER* arg);

public:
	BEBOP2_STATE_MACHINE(ARCONTROLLER_Device_t* deviceController) : deviceController(deviceController)
	{
		currentState = BEBOP2_STATE_START;
	}
};

struct BEBOP2_STATE_MACHINE::BEBOP2_STATE_PARAMETER { };

struct BEBOP2_STATE_MACHINE::BEBOP2_STATE_PARAMETER_START : BEBOP2_STATE_PARAMETER
{
	bool connected;

	BEBOP2_STATE_PARAMETER_START(bool connected = false) : connected(connected) { }
};

struct BEBOP2_STATE_MACHINE::BEBOP2_STATE_PARAMETER_READY : BEBOP2_STATE_PARAMETER
{
	enum BEBOP2_STATE_PARAMETER_READY_COMMAND
	{
		BEBOP2_STATE_PARAMETER_READY_COMMAND_NONE = 0,
		BEBOP2_STATE_PARAMETER_READY_COMMAND_TAKEOFF,
		BEBOP2_STATE_PARAMETER_READY_COMMAND_DISCONNECT,
	};

	BEBOP2_STATE_PARAMETER_READY_COMMAND command;

	BEBOP2_STATE_PARAMETER_READY(BEBOP2_STATE_PARAMETER_READY_COMMAND command = BEBOP2_STATE_PARAMETER_READY_COMMAND_NONE) : command(command) { }
};

struct BEBOP2_STATE_MACHINE::BEBOP2_STATE_PARAMETER_TAKINGOFF : BEBOP2_STATE_PARAMETER
{
	ULONGLONG takenOffTick;

	BEBOP2_STATE_PARAMETER_TAKINGOFF()
	{
		this->takenOffTick = GetTickCount64();
	}

	BEBOP2_STATE_PARAMETER_TAKINGOFF(ULONGLONG takenOffTick) : takenOffTick(takenOffTick) { }
};

struct BEBOP2_STATE_MACHINE::BEBOP2_STATE_PARAMETER_HOVERING : BEBOP2_STATE_PARAMETER
{
	enum BEBOP2_STATE_PARAMETER_HOVERING_COMMAND
	{
		BEBOP2_STATE_PARAMETER_HOVERING_COMMAND_NONE = 0,
		BEBOP2_STATE_PARAMETER_HOVERING_COMMAND_SEARCH,
		BEBOP2_STATE_PARAMETER_HOVERING_COMMAND_LAND,
	};

	BEBOP2_STATE_PARAMETER_HOVERING_COMMAND command;

	BEBOP2_STATE_PARAMETER_HOVERING() : command(BEBOP2_STATE_PARAMETER_HOVERING_COMMAND_NONE) { }
};

struct BEBOP2_STATE_MACHINE::BEBOP2_STATE_PARAMETER_SEARCHING : BEBOP2_STATE_PARAMETER
{
	ULONGLONG startedTick;
	bool found;

	BEBOP2_STATE_PARAMETER_SEARCHING() : found(false)
	{
		this->startedTick = GetTickCount64();
	}

	BEBOP2_STATE_PARAMETER_SEARCHING(ULONGLONG startedTick, bool found) : startedTick(startedTick), found(found) { }
};

struct BEBOP2_STATE_MACHINE::BEBOP2_STATE_PARAMETER_TRACKING : BEBOP2_STATE_PARAMETER
{
	enum BEBOP2_STATE_PARAMETER_TRACKING_STATUS
	{
		BEBOP2_STATE_PARAMETER_TRACKING_STATUS_NONE = 0,
		BEBOP2_STATE_PARAMETER_TRACKING_STATUS_FOUND,
		BEBOP2_STATE_PARAMETER_TRACKING_STATUS_MISSED,
		BEBOP2_STATE_PARAMETER_TRACKING_STATUS_CAPTURED,
	};

	enum BEBOP2_STATE_PARAMETER_TRACKING_DIRECTION
	{
		BEBOP2_STATE_PARAMETER_TRACKING_DIRECTION_NONE = 0,
		BEBOP2_STATE_PARAMETER_TRACKING_DIRECTION_FORWARD,
		BEBOP2_STATE_PARAMETER_TRACKING_DIRECTION_LEFT,
		BEBOP2_STATE_PARAMETER_TRACKING_DIRECTION_RIGHT,
	};

	void* tracker;
	BEBOP2_STATE_PARAMETER_TRACKING_STATUS status;
	BEBOP2_STATE_PARAMETER_TRACKING_DIRECTION direction;

	BEBOP2_STATE_PARAMETER_TRACKING(void* tracker, BEBOP2_STATE_PARAMETER_TRACKING_STATUS status, BEBOP2_STATE_PARAMETER_TRACKING_DIRECTION direction) : tracker(tracker), status(status), direction(direction) { }
};

struct BEBOP2_STATE_MACHINE::BEBOP2_STATE_PARAMETER_MISSING : BEBOP2_STATE_PARAMETER
{
	void* tracker;
	ULONGLONG missedTick;
	bool found;

	BEBOP2_STATE_PARAMETER_MISSING(void* tracker, ULONGLONG missedTick, bool found) : tracker(tracker), missedTick(missedTick), found(found) { }
};

struct BEBOP2_STATE_MACHINE::BEBOP2_STATE_PARAMETER_LANDING : BEBOP2_STATE_PARAMETER
{
	ULONGLONG landedTick;

	BEBOP2_STATE_PARAMETER_LANDING()
	{
		this->landedTick = GetTickCount64();
	}

	BEBOP2_STATE_PARAMETER_LANDING(ULONGLONG landedTick) : landedTick(landedTick) { }
};

struct BEBOP2_STATE_MACHINE::BEBOP2_STATE_PARAMETER_FINISHED : BEBOP2_STATE_PARAMETER { };

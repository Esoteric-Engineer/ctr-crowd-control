#include <crowd/crowd_fx_input.h>

#include <crowd/crowd_runtime.h>

#include <common.h>

enum
{
	CROWD_INPUT_JUMP_BUTTON_MASK = BTN_L1 | BTN_R1, /* VEH_PHYS_PROC_JUMP_BUTTON_MASK, game/Vehicle/VehPhysProc.c:16 */
	CROWD_INPUT_REVERSE_CAM_MASK = BTN_R2, /* buttonsHeldCurrFrame, game/CAM.c:1082 */
	CROWD_INPUT_STEER_CENTER = 0x80, /* RACING_WHEEL_DEFAULT_CENTER, game/Vehicle/VehPhysJoystick.c:5 */
	CROWD_INPUT_STICK_MAX = 0xFF,
};

global_variable b32 s_jumpingDisabled = 0;
global_variable b32 s_reverseCameraActive = 0;
global_variable b32 s_constantJumpActive = 0;
global_variable b32 s_turnLeftDisabled = 0;
global_variable b32 s_turnRightDisabled = 0;
global_variable b32 s_reverseSteeringActive = 0;

enum CrowdEffectStatus Crowd_Fx_InputReverseCamera_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	s_reverseCameraActive = 1;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_InputReverseCamera_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	s_reverseCameraActive = 0;
}

enum CrowdEffectStatus Crowd_Fx_InputConstantJump_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	s_constantJumpActive = 1;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_InputConstantJump_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	s_constantJumpActive = 0;
}

enum CrowdEffectStatus Crowd_Fx_InputDisableTurnLeft_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	s_turnLeftDisabled = 1;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_InputDisableTurnLeft_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	s_turnLeftDisabled = 0;
}

enum CrowdEffectStatus Crowd_Fx_InputDisableTurnRight_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	s_turnRightDisabled = 1;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_InputDisableTurnRight_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	s_turnRightDisabled = 0;
}

enum CrowdEffectStatus Crowd_Fx_InputReverseSteering_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	s_reverseSteeringActive = 1;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_InputReverseSteering_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	s_reverseSteeringActive = 0;
}

enum CrowdEffectStatus Crowd_Fx_InputDisableJumping_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	if (s_jumpingDisabled)
	{
		return CROWD_STATUS_RETRY;
	}

	s_jumpingDisabled = 1;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_InputDisableJumping_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	s_jumpingDisabled = 0;
}

void CrowdFxInput_ApplyMask(void)
{
	if (!s_jumpingDisabled && !s_reverseCameraActive && !s_constantJumpActive &&
	    !s_turnLeftDisabled && !s_turnRightDisabled && !s_reverseSteeringActive)
	{
		return;
	}

	/* Skips the mask during pause/end-of-race/menus so they stay navigable. */
	if (!CrowdRuntime_IsReady())
	{
		return;
	}

	struct GamepadBuffer *pad = &sdata->gGamepads->gamepad[0];

	if (s_constantJumpActive)
	{
		/* Force a fresh jump input every frame. */
		pad->buttonsHeldCurrFrame |= CROWD_INPUT_JUMP_BUTTON_MASK;
		pad->buttonsTapped |= CROWD_INPUT_JUMP_BUTTON_MASK;
	}

	if (s_jumpingDisabled)
	{
		/* Disable wins over constant-jump if both happen to be active at once. */
		pad->buttonsHeldCurrFrame &= ~CROWD_INPUT_JUMP_BUTTON_MASK;
		pad->buttonsTapped &= ~CROWD_INPUT_JUMP_BUTTON_MASK;
	}

	if (s_reverseCameraActive)
	{
		pad->buttonsHeldCurrFrame |= CROWD_INPUT_REVERSE_CAM_MASK;
	}

	/* stickLX: 
			- 0x00 = left
			- 0x80 = center
			- 0xFF = right
		 */
	if (s_turnLeftDisabled)
	{
		pad->buttonsHeldCurrFrame &= ~BTN_LEFT;
		pad->buttonsTapped &= ~BTN_LEFT;
		pad->stickLX = max(pad->stickLX, CROWD_INPUT_STEER_CENTER);
	}

	if (s_turnRightDisabled)
	{
		pad->buttonsHeldCurrFrame &= ~BTN_RIGHT;
		pad->buttonsTapped &= ~BTN_RIGHT;
		pad->stickLX = min(pad->stickLX, CROWD_INPUT_STEER_CENTER);
	}

	if (s_reverseSteeringActive)
	{
		/* Applied after the disable-turn checks so "disable turn left/right" always blocks the raw physical direction. */
		int heldLeft = pad->buttonsHeldCurrFrame & BTN_LEFT;
		int heldRight = pad->buttonsHeldCurrFrame & BTN_RIGHT;
		pad->buttonsHeldCurrFrame &= ~(BTN_LEFT | BTN_RIGHT);
		pad->buttonsHeldCurrFrame |= (heldLeft ? BTN_RIGHT : 0) | (heldRight ? BTN_LEFT : 0);

		int tapLeft = pad->buttonsTapped & BTN_LEFT;
		int tapRight = pad->buttonsTapped & BTN_RIGHT;
		pad->buttonsTapped &= ~(BTN_LEFT | BTN_RIGHT);
		pad->buttonsTapped |= (tapLeft ? BTN_RIGHT : 0) | (tapRight ? BTN_LEFT : 0);

		pad->stickLX = CROWD_INPUT_STICK_MAX - pad->stickLX;
	}
}

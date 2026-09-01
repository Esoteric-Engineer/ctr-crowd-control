#include <crowd/crowd_fx_input.h>

#include <crowd/crowd_runtime.h>

#include <common.h>

enum
{
	CROWD_INPUT_JUMP_BUTTON_MASK = BTN_L1 | BTN_R1, /* VEH_PHYS_PROC_JUMP_BUTTON_MASK, game/Vehicle/VehPhysProc.c:16 */
	CROWD_INPUT_STEER_CENTER = 0x80, /* RACING_WHEEL_DEFAULT_CENTER, game/Vehicle/VehPhysJoystick.c:5 */
};

global_variable b32 s_engineDisabled = 0;
global_variable b32 s_steeringDisabled = 0;
global_variable b32 s_jumpingDisabled = 0;

enum CrowdEffectStatus Crowd_Fx_InputDisableEngine_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	if (s_engineDisabled)
	{
		return CROWD_STATUS_RETRY;
	}

	s_engineDisabled = 1;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_InputDisableEngine_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	s_engineDisabled = 0;
}

enum CrowdEffectStatus Crowd_Fx_InputDisableSteering_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	if (s_steeringDisabled)
	{
		return CROWD_STATUS_RETRY;
	}

	s_steeringDisabled = 1;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_InputDisableSteering_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	s_steeringDisabled = 0;
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
	if (!s_engineDisabled && !s_steeringDisabled && !s_jumpingDisabled)
	{
		return;
	}

	/* Skips the mask during pause/end-of-race/menus so they stay navigable. */
	if (!CrowdRuntime_IsReady())
	{
		return;
	}

	struct GamepadBuffer *pad = &sdata->gGamepads->gamepad[0];

	if (s_engineDisabled)
	{
		pad->buttonsHeldCurrFrame &= ~BTN_CROSS;
		pad->buttonsTapped &= ~BTN_CROSS;
	}

	if (s_jumpingDisabled)
	{
		pad->buttonsHeldCurrFrame &= ~CROWD_INPUT_JUMP_BUTTON_MASK;
		pad->buttonsTapped &= ~CROWD_INPUT_JUMP_BUTTON_MASK;
	}

	if (s_steeringDisabled)
	{
		pad->buttonsHeldCurrFrame &= ~(BTN_LEFT | BTN_RIGHT);
		pad->buttonsTapped &= ~(BTN_LEFT | BTN_RIGHT);
		pad->stickLX = CROWD_INPUT_STEER_CENTER;
	}
}

#include <crowd/crowd_fx_player_speed.h>

#include <crowd/crowd_runtime.h>

#include <common.h>

global_variable s32 s_playerSpeedDisableCount = 0;
global_variable s32 s_playerSpeedMaxDown50Count = 0;

enum CrowdEffectStatus Crowd_Fx_PlayerSpeedDisable_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	s_playerSpeedDisableCount++;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_PlayerSpeedDisable_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	if (s_playerSpeedDisableCount > 0)
	{
		s_playerSpeedDisableCount--;
	}
}

enum CrowdEffectStatus Crowd_Fx_PlayerSpeedMaxDown50_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	s_playerSpeedMaxDown50Count++;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_PlayerSpeedMaxDown50_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	if (s_playerSpeedMaxDown50Count > 0)
	{
		s_playerSpeedMaxDown50Count--;
	}
}

int CrowdFxPlayerSpeed_ScaleSpeed(int speed, int maxSpeed)
{
	if (s_playerSpeedDisableCount > 0)
	{
		/* Always wins over the cap below. */
		return 0;
	}

	if (s_playerSpeedMaxDown50Count > 0)
	{
		/* A ceiling, not a per-frame multiplier. */
		int cap = maxSpeed / 2;
		speed = clamp(speed, -cap, cap);
	}

	return speed;
}

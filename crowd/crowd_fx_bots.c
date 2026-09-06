#include <crowd/crowd_fx_bots.h>

#include <crowd/crowd_runtime.h>

#include <common.h>

/* One counter per simultaneously active redemption of each effect. */
global_variable s32 s_botSpeedDown50Count = 0;
global_variable s32 s_botSpeedUp50Count = 0;
global_variable s32 s_botSpeedDown100Count = 0;
global_variable s32 s_botSpeedUp100Count = 0;

enum CrowdEffectStatus Crowd_Fx_BotsSpeedDown50_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	s_botSpeedDown50Count++;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_BotsSpeedDown50_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	if (s_botSpeedDown50Count > 0)
	{
		s_botSpeedDown50Count--;
	}
}

enum CrowdEffectStatus Crowd_Fx_BotsSpeedUp50_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	s_botSpeedUp50Count++;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_BotsSpeedUp50_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	if (s_botSpeedUp50Count > 0)
	{
		s_botSpeedUp50Count--;
	}
}

enum CrowdEffectStatus Crowd_Fx_BotsSpeedDown100_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	s_botSpeedDown100Count++;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_BotsSpeedDown100_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	if (s_botSpeedDown100Count > 0)
	{
		s_botSpeedDown100Count--;
	}
}

enum CrowdEffectStatus Crowd_Fx_BotsSpeedUp100_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	s_botSpeedUp100Count++;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_BotsSpeedUp100_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	if (s_botSpeedUp100Count > 0)
	{
		s_botSpeedUp100Count--;
	}
}

/* -100% clamps to zero rather than reversing bot progress. */
int CrowdFxBots_ScaleDelta(int delta)
{
	s32 percent = (s_botSpeedUp50Count * 50) + (s_botSpeedUp100Count * 100) - (s_botSpeedDown50Count * 50) - (s_botSpeedDown100Count * 100);

	if (percent < -100)
	{
		percent = -100;
	}

	return (delta * (100 + percent)) / 100;
}

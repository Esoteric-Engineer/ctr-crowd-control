#include <crowd/crowd_fx_bots.h>

#include <crowd/crowd_runtime.h>

#include <common.h>

/* Only one bot-speed effect can be active at a time. */
enum CrowdBotSpeedMode
{
	CROWD_BOT_SPEED_MODE_NONE,
	CROWD_BOT_SPEED_MODE_DOWN_50,
	CROWD_BOT_SPEED_MODE_UP_50,
	CROWD_BOT_SPEED_MODE_DOWN_100,
	CROWD_BOT_SPEED_MODE_UP_100,
};

global_variable enum CrowdBotSpeedMode s_botSpeedMode = CROWD_BOT_SPEED_MODE_NONE;

internal enum CrowdEffectStatus CrowdFxBots_Start(enum CrowdBotSpeedMode mode)
{
	if (s_botSpeedMode != CROWD_BOT_SPEED_MODE_NONE)
	{
		return CROWD_STATUS_RETRY;
	}

	s_botSpeedMode = mode;
	return CROWD_STATUS_SUCCESS;
}

internal void CrowdFxBots_Stop(enum CrowdBotSpeedMode mode)
{
	if (s_botSpeedMode == mode)
	{
		s_botSpeedMode = CROWD_BOT_SPEED_MODE_NONE;
	}
}

enum CrowdEffectStatus Crowd_Fx_BotsSpeedDown50_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	return CrowdFxBots_Start(CROWD_BOT_SPEED_MODE_DOWN_50);
}

void Crowd_Fx_BotsSpeedDown50_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	CrowdFxBots_Stop(CROWD_BOT_SPEED_MODE_DOWN_50);
}

enum CrowdEffectStatus Crowd_Fx_BotsSpeedUp50_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	return CrowdFxBots_Start(CROWD_BOT_SPEED_MODE_UP_50);
}

void Crowd_Fx_BotsSpeedUp50_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	CrowdFxBots_Stop(CROWD_BOT_SPEED_MODE_UP_50);
}

enum CrowdEffectStatus Crowd_Fx_BotsSpeedDown100_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	return CrowdFxBots_Start(CROWD_BOT_SPEED_MODE_DOWN_100);
}

void Crowd_Fx_BotsSpeedDown100_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	CrowdFxBots_Stop(CROWD_BOT_SPEED_MODE_DOWN_100);
}

enum CrowdEffectStatus Crowd_Fx_BotsSpeedUp100_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	return CrowdFxBots_Start(CROWD_BOT_SPEED_MODE_UP_100);
}

void Crowd_Fx_BotsSpeedUp100_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	CrowdFxBots_Stop(CROWD_BOT_SPEED_MODE_UP_100);
}

/* -100% clamps to zero rather than reversing bot progress. */
int CrowdFxBots_ScaleDelta(int delta)
{
	switch (s_botSpeedMode)
	{
	case CROWD_BOT_SPEED_MODE_DOWN_50:
		return delta / 2;

	case CROWD_BOT_SPEED_MODE_UP_50:
		return delta + (delta / 2);

	case CROWD_BOT_SPEED_MODE_DOWN_100:
		return 0;

	case CROWD_BOT_SPEED_MODE_UP_100:
		return delta * 2;

	case CROWD_BOT_SPEED_MODE_NONE:
	default:
		return delta;
	}
}

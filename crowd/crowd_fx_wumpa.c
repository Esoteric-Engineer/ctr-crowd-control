#include <crowd/crowd_runtime.h>

#include <common.h>

/* Wumpa give/remove effects (all instant). */

internal enum CrowdEffectStatus CrowdFxWumpa_Give(void)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *driver = gGT->drivers[0];

	if ((gGT->gameMode2 & CHEAT_WUMPA) != 0)
	{
		return CROWD_STATUS_FAILURE;
	}

	if (driver->numWumpas >= DRIVER_WUMPA_MAX_COUNT)
	{
		return CROWD_STATUS_FAILURE;
	}

	RB_Player_ModifyWumpa(driver, 1);
	return CROWD_STATUS_SUCCESS;
}

internal enum CrowdEffectStatus CrowdFxWumpa_Remove(int wumpaDelta)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *driver = gGT->drivers[0];

	if ((gGT->gameMode2 & CHEAT_WUMPA) != 0)
	{
		return CROWD_STATUS_FAILURE;
	}

	if (driver->numWumpas <= 0)
	{
		return CROWD_STATUS_FAILURE;
	}

	/* Specifically doesn't go through RB_Player_ModifyWumpa, otherwise it would potentially fail if the player has a mask item equipped, which is part of retail, but feels weird in a Crowd Control setting. */
	driver->numWumpas = (s8)clamp(driver->numWumpas + wumpaDelta, 0, DRIVER_WUMPA_MAX_COUNT);
	return CROWD_STATUS_SUCCESS;
}

enum CrowdEffectStatus Crowd_Fx_WumpaGive_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	return CrowdFxWumpa_Give();
}

void Crowd_Fx_WumpaGive_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;
}

enum CrowdEffectStatus Crowd_Fx_WumpaRemove_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	return CrowdFxWumpa_Remove(-1);
}

void Crowd_Fx_WumpaRemove_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;
}

enum CrowdEffectStatus Crowd_Fx_WumpaRemoveAll_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	return CrowdFxWumpa_Remove(-sdata->gGT->drivers[0]->numWumpas);
}

void Crowd_Fx_WumpaRemoveAll_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;
}

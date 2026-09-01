#include <crowd/crowd_runtime.h>

#include <common.h>

/* 
- CHEAT_ICY/CHEAT_TURBOPAD are plain per-frame checks (game/COLL.c:1476, game/Vehicle/VehPhysForce.c:586), so Start/Stop just toggles the bit.
- CHEAT_WUMPA and CHEAT_MASK/TURBO/BOMBS only *freeze* numWumpas/numHeldItems (game/231/RB_Player.c:144, game/Vehicle/VehPhysProc.c:626). Start applies the one-shot grant VehBirth.c:406-434 makes at spawn, and Stop restores what was there before.
- CHEAT_INVISIBLE freezes invisibleTimer (game/Vehicle/VehPhysProc.c:218), so Stop can just clear the bit and let that same per-frame code count the timer down and restore draw flags. */

enum
{
	CROWD_CHEAT_WUMPA_GRANT = 99, /* VEH_BIRTH_CHEAT_WUMPA_COUNT */
	CROWD_CHEAT_ITEM_GRANT_COUNT = 9, /* VEH_BIRTH_CHEAT_ITEM_COUNT */
	CROWD_CHEAT_INVISIBLE_TIMER = 1, /* nonzero; frozen by CHEAT_INVISIBLE while the effect runs */
};

global_variable b32 s_wumpaActive = 0;
global_variable s8 s_wumpaPrevCount = 0;

global_variable b32 s_icyActive = 0;
global_variable b32 s_turbopadActive = 0;

/* Masks/Turbo/Bombs share the driver->heldItemID slot with ordinary items, so only one at a time. */
internal enum CrowdEffectStatus CrowdFxCheats_HeldItemStart(int cheatBit, DriverHeldItem itemId)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *driver = gGT->drivers[0];

	if (driver->heldItemID != HELD_ITEM_NONE)
	{
		return CROWD_STATUS_RETRY;
	}

	gGT->gameMode2 |= cheatBit;
	driver->heldItemID = itemId;
	driver->numHeldItems = CROWD_CHEAT_ITEM_GRANT_COUNT;
	return CROWD_STATUS_SUCCESS;
}

internal void CrowdFxCheats_HeldItemStop(int cheatBit, DriverHeldItem itemId)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *driver = gGT->drivers[0];

	gGT->gameMode2 &= ~cheatBit;

	if (driver->heldItemID == itemId) /* leave it alone if the player swapped it for something else */
	{
		driver->heldItemID = HELD_ITEM_NONE;
		driver->numHeldItems = 0;
	}
}

enum CrowdEffectStatus Crowd_Fx_CheatInfiniteWumpa_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	if (s_wumpaActive)
	{
		return CROWD_STATUS_RETRY;
	}

	struct GameTracker *gGT = sdata->gGT;
	struct Driver *driver = gGT->drivers[0];

	s_wumpaPrevCount = driver->numWumpas;
	s_wumpaActive = 1;

	gGT->gameMode2 |= CHEAT_WUMPA;
	driver->numWumpas = CROWD_CHEAT_WUMPA_GRANT;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_CheatInfiniteWumpa_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	struct GameTracker *gGT = sdata->gGT;

	gGT->gameMode2 &= ~CHEAT_WUMPA;
	gGT->drivers[0]->numWumpas = s_wumpaPrevCount;
	s_wumpaActive = 0;
}

enum CrowdEffectStatus Crowd_Fx_CheatInfiniteMasks_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	return CrowdFxCheats_HeldItemStart(CHEAT_MASK, HELD_ITEM_MASK);
}

void Crowd_Fx_CheatInfiniteMasks_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	CrowdFxCheats_HeldItemStop(CHEAT_MASK, HELD_ITEM_MASK);
}

enum CrowdEffectStatus Crowd_Fx_CheatInfiniteTurbo_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	return CrowdFxCheats_HeldItemStart(CHEAT_TURBO, HELD_ITEM_TURBO);
}

void Crowd_Fx_CheatInfiniteTurbo_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	CrowdFxCheats_HeldItemStop(CHEAT_TURBO, HELD_ITEM_TURBO);
}

enum CrowdEffectStatus Crowd_Fx_CheatInfiniteBombs_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	return CrowdFxCheats_HeldItemStart(CHEAT_BOMBS, HELD_ITEM_BOMB_1X);
}

void Crowd_Fx_CheatInfiniteBombs_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	CrowdFxCheats_HeldItemStop(CHEAT_BOMBS, HELD_ITEM_BOMB_1X);
}

enum CrowdEffectStatus Crowd_Fx_CheatInfiniteInvisibility_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	struct GameTracker *gGT = sdata->gGT;
	struct Driver *driver = gGT->drivers[0];

	if (driver->invisibleTimer == 0)
	{
		driver->instFlagsBackup = driver->instSelf->flags;
		driver->instSelf->flags = (driver->instSelf->flags & ~(u32)(DRAW_TRANSPARENT | GHOST_DRAW_TRANSPARENT)) | GHOST_DRAW_TRANSPARENT;
	}

	gGT->gameMode2 |= CHEAT_INVISIBLE;
	driver->invisibleTimer = CROWD_CHEAT_INVISIBLE_TIMER;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_CheatInfiniteInvisibility_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	sdata->gGT->gameMode2 &= ~CHEAT_INVISIBLE;
}

enum CrowdEffectStatus Crowd_Fx_CheatIcyTracks_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	if (s_icyActive)
	{
		return CROWD_STATUS_RETRY;
	}

	s_icyActive = 1;
	sdata->gGT->gameMode2 |= CHEAT_ICY;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_CheatIcyTracks_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	sdata->gGT->gameMode2 &= ~CHEAT_ICY;
	s_icyActive = 0;
}

enum CrowdEffectStatus Crowd_Fx_CheatSuperTurboPads_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	if (s_turbopadActive)
	{
		return CROWD_STATUS_RETRY;
	}

	s_turbopadActive = 1;
	sdata->gGT->gameMode2 |= CHEAT_TURBOPAD;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_CheatSuperTurboPads_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	sdata->gGT->gameMode2 &= ~CHEAT_TURBOPAD;
	s_turbopadActive = 0;
}

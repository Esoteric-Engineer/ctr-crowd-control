#include <crowd/crowd_runtime.h>

#include <common.h>

/* Race item effects (all instant). See include/crowd/crowd_effects.h for the effect table entries. */

/* Currently the 3x-stack variants (bomb / missile) and HELD_ITEM_ROULETTE aren't offered. Could be expanded later if those are requested. */
#define CROWD_ITEM_LIST(X) \
	X(Boost, HELD_ITEM_TURBO) \
	X(Bomb, HELD_ITEM_BOMB_1X) \
	X(Missile, HELD_ITEM_MISSILE_1X) \
	X(Tnt, HELD_ITEM_TNT) \
	X(Potion, HELD_ITEM_POTION) \
	X(Spring, HELD_ITEM_SPRING) \
	X(Shield, HELD_ITEM_SHIELD) \
	X(Mask, HELD_ITEM_MASK) \
	X(Clock, HELD_ITEM_CLOCK) \
	X(Warp, HELD_ITEM_WARPBALL) \
	X(Invisibility, HELD_ITEM_INVISIBILITY) \
	X(SuperTurbo, HELD_ITEM_SUPER_ENGINE)

internal enum CrowdEffectStatus CrowdFxItems_Add(DriverHeldItem itemId)
{
	struct Driver *driver = sdata->gGT->drivers[0];

	if (driver->heldItemID != HELD_ITEM_NONE)
	{
		return CROWD_STATUS_RETRY; /* retry when a held item is already present */
	}

	driver->heldItemID = itemId;
	driver->numHeldItems = 1;
	return CROWD_STATUS_SUCCESS;
}

/* itemId == HELD_ITEM_NONE means "remove whatever is held". Otherwise refuses (Failure) when the held item does not match. */
internal enum CrowdEffectStatus CrowdFxItems_Remove(DriverHeldItem itemId)
{
	struct Driver *driver = sdata->gGT->drivers[0];

	if (driver->heldItemID == HELD_ITEM_NONE)
	{
		return CROWD_STATUS_FAILURE;
	}
	if ((itemId != HELD_ITEM_NONE) && (driver->heldItemID != itemId))
	{
		return CROWD_STATUS_FAILURE;
	}

	driver->heldItemID = HELD_ITEM_NONE;
	driver->numHeldItems = 0;
	return CROWD_STATUS_SUCCESS;
}

#define CROWD_ITEM_ADD_FN(name, itemId) \
	enum CrowdEffectStatus Crowd_Fx_ItemAdd##name##_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request) \
	{ \
		(void)effect; \
		(void)request; \
		return CrowdFxItems_Add(itemId); \
	} \
	void Crowd_Fx_ItemAdd##name##_Stop(struct CrowdActiveEffect *effect) \
	{ \
		(void)effect; \
	}
CROWD_ITEM_LIST(CROWD_ITEM_ADD_FN)
#undef CROWD_ITEM_ADD_FN

#define CROWD_ITEM_REMOVE_FN(name, itemId) \
	enum CrowdEffectStatus Crowd_Fx_ItemRemove##name##_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request) \
	{ \
		(void)effect; \
		(void)request; \
		return CrowdFxItems_Remove(itemId); \
	} \
	void Crowd_Fx_ItemRemove##name##_Stop(struct CrowdActiveEffect *effect) \
	{ \
		(void)effect; \
	}
CROWD_ITEM_LIST(CROWD_ITEM_REMOVE_FN)
#undef CROWD_ITEM_REMOVE_FN

enum CrowdEffectStatus Crowd_Fx_ItemAddRandom_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	static const DriverHeldItem s_items[] = {
#define CROWD_ITEM_ARRAY_ENTRY(name, itemId) itemId,
	    CROWD_ITEM_LIST(CROWD_ITEM_ARRAY_ENTRY)
#undef CROWD_ITEM_ARRAY_ENTRY
	};
	enum
	{
		CROWD_ITEM_COUNT = (s32)(sizeof(s_items) / sizeof(s_items[0])),
	};

	const s32 index = MixRNG_Scramble() % CROWD_ITEM_COUNT;
	return CrowdFxItems_Add(s_items[index]);
}

void Crowd_Fx_ItemAddRandom_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;
}

enum CrowdEffectStatus Crowd_Fx_ItemRemoveRandom_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	return CrowdFxItems_Remove(HELD_ITEM_NONE);
}

void Crowd_Fx_ItemRemoveRandom_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;
}

#undef CROWD_ITEM_LIST

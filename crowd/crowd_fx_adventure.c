#include <crowd/crowd_runtime.h>

#include <common.h>

/* Adventure progress effects (all instant). Mutates sdata->advProgress.rewards directly, same as retail (game/222.c:602-611). */

struct CrowdAdvRange
{
	s32 first;
	s32 count;
};

enum
{
	CROWD_ADV_MAX_ELIGIBLE_BITS =
	    ADV_REWARD_TROPHY_TRACK_COUNT + (ADV_REWARD_RELIC_TRACK_COUNT * 3) + ADV_REWARD_PURPLE_TOKEN_COUNT + ADV_REWARD_BOSS_KEY_COUNT,
};

#define CROWD_ADV_CATEGORY_LIST(X)                                                  \
	X(Trophy, ADV_REWARD_FIRST_TROPHY, ADV_REWARD_TROPHY_TRACK_COUNT)               \
	X(SapphireRelic, ADV_REWARD_FIRST_SAPPHIRE_RELIC, ADV_REWARD_RELIC_TRACK_COUNT) \
	X(GoldRelic, ADV_REWARD_FIRST_GOLD_RELIC, ADV_REWARD_RELIC_TRACK_COUNT)         \
	X(PlatinumRelic, ADV_REWARD_FIRST_PLATINUM_RELIC, ADV_REWARD_RELIC_TRACK_COUNT) \
	X(Crystal, ADV_REWARD_FIRST_PURPLE_TOKEN, ADV_REWARD_PURPLE_TOKEN_COUNT)        \
	X(Key, ADV_REWARD_FIRST_BOSS_KEY, ADV_REWARD_BOSS_KEY_COUNT)

/* Arcade/Time Trial/Battle never read sdata->advProgress, so gate on the same ADVENTURE_MODE bit checked right before touching these reward bits (game/222.c:136, game/MAIN/MainGameEnd.c:169). */
internal b32 CrowdFxAdventure_IsReady(void)
{
	if (!CrowdRuntime_IsReady())
	{
		return 0;
	}

	return (sdata->gGT->gameMode1 & ADVENTURE_MODE) != 0;
}

/* Returns -1 when nothing in the given ranges currently matches wantSet. */
internal s32 CrowdFxAdventure_PickBit(const struct CrowdAdvRange *ranges, s32 rangeCount, b32 wantSet)
{
	s32 eligible[CROWD_ADV_MAX_ELIGIBLE_BITS];
	s32 numEligible = 0;

	for (s32 r = 0; r < rangeCount; r++)
	{
		for (s32 i = 0; i < ranges[r].count; i++)
		{
			const s32 bitIndex = ranges[r].first + i;
			const b32 isSet = CHECK_ADV_BIT(sdata->advProgress.rewards, bitIndex) != 0;

			if (isSet == wantSet)
			{
				eligible[numEligible++] = bitIndex;
			}
		}
	}

	if (numEligible == 0)
	{
		return -1;
	}

	return eligible[MixRNG_Scramble() % numEligible];
}

internal enum CrowdEffectStatus CrowdFxAdventure_ApplyBit(s32 bitIndex, b32 set)
{
	if (bitIndex < 0)
	{
		return CROWD_STATUS_RETRY;
	}

	if (set)
	{
		UNLOCK_ADV_BIT(sdata->advProgress.rewards, bitIndex);
	}
	else
	{
		sdata->advProgress.rewards[MEMCARD_BIT_WORD(bitIndex)] &= ~MEMCARD_BIT_MASK(bitIndex);
	}

	GAMEPROG_AdvPercent(&sdata->advProgress);
	return CROWD_STATUS_SUCCESS;
}

internal enum CrowdEffectStatus CrowdFxAdventure_AddCategory(s32 first, s32 count)
{
	const struct CrowdAdvRange range = {first, count};

	return CrowdFxAdventure_ApplyBit(CrowdFxAdventure_PickBit(&range, 1, 0), 1);
}

internal enum CrowdEffectStatus CrowdFxAdventure_RemoveCategory(s32 first, s32 count)
{
	const struct CrowdAdvRange range = {first, count};

	return CrowdFxAdventure_ApplyBit(CrowdFxAdventure_PickBit(&range, 1, 1), 0);
}

#define CROWD_ADV_ADD_FN(name, first, count) \
	enum CrowdEffectStatus Crowd_Fx_AdvAdd##name##_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request) \
	{ \
		(void)effect; \
		(void)request; \
		if (!CrowdFxAdventure_IsReady()) \
		{ \
			return CROWD_STATUS_RETRY; \
		} \
		return CrowdFxAdventure_AddCategory(first, count); \
	} \
	void Crowd_Fx_AdvAdd##name##_Stop(struct CrowdActiveEffect *effect) \
	{ \
		(void)effect; \
	}
CROWD_ADV_CATEGORY_LIST(CROWD_ADV_ADD_FN)
#undef CROWD_ADV_ADD_FN

#define CROWD_ADV_REMOVE_FN(name, first, count) \
	enum CrowdEffectStatus Crowd_Fx_AdvRemove##name##_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request) \
	{ \
		(void)effect; \
		(void)request; \
		if (!CrowdFxAdventure_IsReady()) \
		{ \
			return CROWD_STATUS_RETRY; \
		} \
		return CrowdFxAdventure_RemoveCategory(first, count); \
	} \
	void Crowd_Fx_AdvRemove##name##_Stop(struct CrowdActiveEffect *effect) \
	{ \
		(void)effect; \
	}
CROWD_ADV_CATEGORY_LIST(CROWD_ADV_REMOVE_FN)
#undef CROWD_ADV_REMOVE_FN

#undef CROWD_ADV_CATEGORY_LIST

enum CrowdEffectStatus Crowd_Fx_AdvAddRandom_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	if (!CrowdFxAdventure_IsReady())
	{
		return CROWD_STATUS_RETRY;
	}

	static const struct CrowdAdvRange s_addRanges[] = {
	    {ADV_REWARD_FIRST_TROPHY, ADV_REWARD_TROPHY_TRACK_COUNT},       {ADV_REWARD_FIRST_SAPPHIRE_RELIC, ADV_REWARD_RELIC_TRACK_COUNT},
	    {ADV_REWARD_FIRST_GOLD_RELIC, ADV_REWARD_RELIC_TRACK_COUNT},    {ADV_REWARD_FIRST_PLATINUM_RELIC, ADV_REWARD_RELIC_TRACK_COUNT},
	    {ADV_REWARD_FIRST_PURPLE_TOKEN, ADV_REWARD_PURPLE_TOKEN_COUNT}, {ADV_REWARD_FIRST_BOSS_KEY, ADV_REWARD_BOSS_KEY_COUNT},
	};
	enum
	{
		CROWD_ADV_ADD_RANGE_COUNT = (s32)(sizeof(s_addRanges) / sizeof(s_addRanges[0])),
	};

	return CrowdFxAdventure_ApplyBit(CrowdFxAdventure_PickBit(s_addRanges, CROWD_ADV_ADD_RANGE_COUNT, 0), 1);
}

void Crowd_Fx_AdvAddRandom_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;
}

/* Deliberately excludes Key (removing a key should be something done intentionally due to how much it can impact length of the game). */
enum CrowdEffectStatus Crowd_Fx_AdvRemoveRandom_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	if (!CrowdFxAdventure_IsReady())
	{
		return CROWD_STATUS_RETRY;
	}

	static const struct CrowdAdvRange s_removeRanges[] = {
	    {ADV_REWARD_FIRST_TROPHY, ADV_REWARD_TROPHY_TRACK_COUNT},       {ADV_REWARD_FIRST_SAPPHIRE_RELIC, ADV_REWARD_RELIC_TRACK_COUNT},
	    {ADV_REWARD_FIRST_GOLD_RELIC, ADV_REWARD_RELIC_TRACK_COUNT},    {ADV_REWARD_FIRST_PLATINUM_RELIC, ADV_REWARD_RELIC_TRACK_COUNT},
	    {ADV_REWARD_FIRST_PURPLE_TOKEN, ADV_REWARD_PURPLE_TOKEN_COUNT},
	};
	enum
	{
		CROWD_ADV_REMOVE_RANGE_COUNT = (s32)(sizeof(s_removeRanges) / sizeof(s_removeRanges[0])),
	};

	return CrowdFxAdventure_ApplyBit(CrowdFxAdventure_PickBit(s_removeRanges, CROWD_ADV_REMOVE_RANGE_COUNT, 1), 0);
}

void Crowd_Fx_AdvRemoveRandom_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;
}

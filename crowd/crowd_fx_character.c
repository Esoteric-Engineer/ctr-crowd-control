#include <crowd/crowd_fx_character.h>

#include <crowd/crowd_runtime.h>

#include <common.h>

#include <string.h>

/* Oxide is excluded due to VehBirth_TireSprites setting Oxide to wheelSize=0. This handler doesn't currently replicate that path. */
#define CROWD_CHARACTER_LIST(X) \
	X(Crash, CRASH_BANDICOOT) \
	X(Cortex, NEO_CORTEX) \
	X(Tiny, TINY_TIGER) \
	X(Coco, COCO_BANDICOOT) \
	X(Ngin, N_GIN) \
	X(Dingodile, DINGODILE) \
	X(Polar, POLAR) \
	X(Pura, PURA) \
	X(Pinstripe, PINSTRIPE) \
	X(Papu, PAPU_PAPU) \
	X(RipperRoo, RIPPER_ROO) \
	X(Komodo, KOMODO_JOE) \
	X(Tropy, N_TROPY) \
	X(Penta, PENTA_PENGUIN) \
	X(FakeCrash, FAKE_CRASH)

enum
{
	CROWD_CHARACTER_WHEEL_SIZE = 0xccc, /* VEH_BIRTH_WHEEL_SIZE, game/Vehicle/VehBirth.c:20 */
};

/* Characters not currently on the track wouldn't be cached for swapping, so they need to be cached here at HI LOD quality.  */
global_variable struct Model *s_crowdCharacterModelCache[NITROS_OXIDE + 1];

void CrowdFxCharacter_Tick(void)
{
	if (sdata->Loading.stage != LOAD_IDLE)
	{
		memset(s_crowdCharacterModelCache, 0, sizeof(s_crowdCharacterModelCache));
	}
}

internal struct Model *CrowdFxCharacter_GetModel(enum Characters characterID)
{
	struct Model *model = VehBirth_GetModelByName(data.MetaDataCharacters[characterID].name_Debug);
	if (model != NULL)
	{
		return model;
	}

	if (s_crowdCharacterModelCache[characterID] != NULL)
	{
		return s_crowdCharacterModelCache[characterID];
	}

	u32 size = 0;
	void *fileBase = LOAD_DramFile(NULL, BI_RACERMODELHI + characterID, NULL, &size, -1);
	if (fileBase == NULL)
	{
		return NULL;
	}

	model = (struct Model *)((u8 *)fileBase + LOAD_MODEL_FILE_HEADER_BYTES);
	s_crowdCharacterModelCache[characterID] = model;
	return model;
}

/* Mutates instSelf->model in place instead of re-birthing the Instance/Thread, so flags and untouched Driver fields survive the swap. */
internal enum CrowdEffectStatus CrowdFxCharacter_Swap(enum Characters characterID)
{
	struct GameTracker *gGT = sdata->gGT;

	if (gGT->gameMode1 & ADVENTURE_BOSS)
	{
		return CROWD_STATUS_RETRY;
	}

	struct Driver *driver = gGT->drivers[0];
	struct Model *model = CrowdFxCharacter_GetModel(characterID);

	if (model == NULL)
	{
		return CROWD_STATUS_RETRY;
	}

	struct Instance *inst = driver->instSelf;

	inst->animFrame = 0;
	inst->animIndex = 0;
	inst->model = model;

	data.characterIDs[driver->driverID] = (s16)characterID;
	driver->wheelSize = CROWD_CHARACTER_WHEEL_SIZE;

	VehBirth_SetConsts(driver);
	VehBirth_EngineAudio_AllPlayers();

	return CROWD_STATUS_SUCCESS;
}

#define CROWD_CHARACTER_SWAP_FN(name, characterID) \
	enum CrowdEffectStatus Crowd_Fx_CharacterSwap##name##_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request) \
	{ \
		(void)effect; \
		(void)request; \
		return CrowdFxCharacter_Swap(characterID); \
	} \
	void Crowd_Fx_CharacterSwap##name##_Stop(struct CrowdActiveEffect *effect) \
	{ \
		(void)effect; \
	}
CROWD_CHARACTER_LIST(CROWD_CHARACTER_SWAP_FN)
#undef CROWD_CHARACTER_SWAP_FN

enum CrowdEffectStatus Crowd_Fx_CharacterSwapRandom_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	static const enum Characters s_characters[] = {
#define CROWD_CHARACTER_ARRAY_ENTRY(name, characterID) characterID,
	    CROWD_CHARACTER_LIST(CROWD_CHARACTER_ARRAY_ENTRY)
#undef CROWD_CHARACTER_ARRAY_ENTRY
	};
	enum
	{
		CROWD_CHARACTER_COUNT = (s32)(sizeof(s_characters) / sizeof(s_characters[0])),
	};

	const s32 index = MixRNG_Scramble() % CROWD_CHARACTER_COUNT;
	return CrowdFxCharacter_Swap(s_characters[index]);
}

void Crowd_Fx_CharacterSwapRandom_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;
}

#undef CROWD_CHARACTER_LIST

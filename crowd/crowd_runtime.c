#include <crowd/crowd_runtime.h>

#include <common.h>
#include <crowd/crowd_protocol.h>

#include <platform/native_log.h>

#include <string.h>

/* Forward-declare every handler from the effect table. */
#define CROWD_EFFECT(code, kind, durationMs, category, handler)                                                                 \
	enum CrowdEffectStatus Crowd_Fx_##handler##_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request); \
	void Crowd_Fx_##handler##_Stop(struct CrowdActiveEffect *effect);
#include <crowd/crowd_effects.h>
#undef CROWD_EFFECT

#if defined(CTR_NATIVE) && defined(CTR_INTERNAL)
/* Matches the forward declaration the CROWD_EFFECT pass above generated. */
enum CrowdEffectStatus Crowd_Fx_DebugTest_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)request;

	Platform_Log("[CTR Crowd] debug_test_timed started (quantity=%d, duration=%dms)\n", (int)effect->quantity, (int)effect->totalMs);
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_DebugTest_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	Platform_Log("[CTR Crowd] debug_test_timed finished\n");
}
#endif

struct CrowdEffectDef
{
	const char *code;
	enum CrowdEffectKind kind;
	s32 defaultDurationMs;
	enum CrowdEffectCategory category;
	CrowdFxStartFn start;
	CrowdFxStopFn stop;
};

/* CTR_INTERNAL is unconditionally defined for ctr_native (see CMakeLists.txt), so this table is
never empty in practice. */
#define CROWD_EFFECT(code, kind, durationMs, category, handler) {code, kind, durationMs, category, Crowd_Fx_##handler##_Start, Crowd_Fx_##handler##_Stop},
global_variable const struct CrowdEffectDef s_crowdEffectDefs[] = {
#include <crowd/crowd_effects.h>
};
#undef CROWD_EFFECT

enum
{
	CROWD_EFFECT_DEF_COUNT = (s32)(sizeof(s_crowdEffectDefs) / sizeof(s_crowdEffectDefs[0])),
};

global_variable struct CrowdActiveEffect s_activeEffects[CROWD_MAX_ACTIVE_EFFECTS];
global_variable b32 s_wasPaused = 0;
global_variable s32 s_lastLevelId = -1;
global_variable b32 s_haveLastLevelId = 0;

internal const struct CrowdEffectDef *CrowdRuntime_FindEffect(const struct CrowdJsonSlice *codeSlice)
{
	for (s32 i = 0; i < CROWD_EFFECT_DEF_COUNT; i++)
	{
		const struct CrowdEffectDef *def = &s_crowdEffectDefs[i];
		const size_t codeLength = strlen(def->code);

		if ((codeLength == (size_t)codeSlice->length) && (memcmp(def->code, codeSlice->ptr, codeLength) == 0))
		{
			return def;
		}
	}

	return NULL;
}

internal struct CrowdActiveEffect *CrowdRuntime_AllocSlot(void)
{
	for (s32 i = 0; i < CROWD_MAX_ACTIVE_EFFECTS; i++)
	{
		if (!s_activeEffects[i].inUse)
		{
			return &s_activeEffects[i];
		}
	}

	return NULL;
}

/* Ensures effects only land mid-race by ruling out loading/menus/cutscenes/pause/etc. */
b32 CrowdRuntime_IsReady(void)
{
	if (sdata->mainGameState != 3)
	{
		return 0;
	}
	if (sdata->Loading.stage != LOAD_IDLE)
	{
		return 0;
	}

	struct GameTracker *gGT = sdata->gGT;

	if (gGT->levelID == MAIN_MENU_LEVEL)
	{
		return 0;
	}
	if (gGT->drivers[0] == NULL)
	{
		return 0;
	}
	if (gGT->gameMode1 & (PAUSE_ALL | MAIN_MENU | GAME_CUTSCENE | LOADING | END_OF_RACE))
	{
		return 0;
	}

	return 1;
}

internal void CrowdRuntime_FillFromRequest(struct CrowdActiveEffect *effect, const struct CrowdEffectDef *def, const struct CrowdJsonSlice *idRaw,
                                           const struct CrowdJsonObject *request)
{
	effect->inUse = 1;
	effect->effectIndex = (s32)(def - s_crowdEffectDefs);

	effect->requestIdLength = 0;
	if (idRaw != NULL)
	{
		effect->requestIdLength = (s32)((idRaw->length < (u32)(CROWD_REQUEST_ID_MAX - 1)) ? idRaw->length : (u32)(CROWD_REQUEST_ID_MAX - 1));
		memcpy(effect->requestId, idRaw->ptr, (size_t)effect->requestIdLength);
	}

	effect->quantity = 1;
	const struct CrowdJsonValue *quantityValue = CrowdJson_Get(request, "quantity");
	if ((quantityValue != NULL) && (quantityValue->type == CROWD_JSON_NUMBER))
	{
		effect->quantity = (s32)quantityValue->number;
	}

	/* SimpleTCP sends "duration" in seconds, which must then be converted to milliseconds for tables and timers. */
	effect->totalMs = def->defaultDurationMs;
	const struct CrowdJsonValue *durationValue = CrowdJson_Get(request, "duration");
	if ((durationValue != NULL) && (durationValue->type == CROWD_JSON_NUMBER) && (durationValue->number > 0.0))
	{
		effect->totalMs = (s32)(durationValue->number * 1000.0);
	}

	effect->remainingMs = 0;
	effect->paused = 0;
}

internal void CrowdRuntime_SendActiveEffectStatus(const struct CrowdActiveEffect *effect, enum CrowdEffectStatus status)
{
	struct CrowdJsonSlice idRaw;

	idRaw.ptr = effect->requestId;
	idRaw.length = (u32)effect->requestIdLength;

	CrowdProtocol_SendEffectStatus((effect->requestIdLength > 0) ? &idRaw : NULL, status, effect->remainingMs, 1);
}

/* Reverts (timed effects only) and reports Finished.
Crowd Control treats natural expiry, EffectStop, and forced level-transition expiry all the same. */
internal void CrowdRuntime_FinishEffect(struct CrowdActiveEffect *effect)
{
	const struct CrowdEffectDef *def = &s_crowdEffectDefs[effect->effectIndex];

	if (def->kind == CROWD_EFFECT_TIMED)
	{
		def->stop(effect);
	}

	CrowdRuntime_SendActiveEffectStatus(effect, CROWD_STATUS_FINISHED);
	effect->inUse = 0;
}

internal void CrowdRuntime_HandleStart(const struct CrowdJsonSlice *idRaw, const struct CrowdEffectDef *def, const struct CrowdJsonObject *request)
{
	if (def->kind == CROWD_EFFECT_INSTANT)
	{
		struct CrowdActiveEffect scratch;

		memset(&scratch, 0, sizeof(scratch));
		CrowdRuntime_FillFromRequest(&scratch, def, idRaw, request);

		const enum CrowdEffectStatus status = def->start(&scratch, request);
		CrowdProtocol_SendEffectStatus(idRaw, status, 0, 0);
		return;
	}

	struct CrowdActiveEffect *slot = CrowdRuntime_AllocSlot();
	if (slot == NULL)
	{
		CrowdProtocol_SendEffectStatus(idRaw, CROWD_STATUS_RETRY, 0, 0);
		return;
	}

	CrowdRuntime_FillFromRequest(slot, def, idRaw, request);

	const enum CrowdEffectStatus status = def->start(slot, request);
	if (status != CROWD_STATUS_SUCCESS)
	{
		slot->inUse = 0;
		CrowdProtocol_SendEffectStatus(idRaw, status, 0, 0);
		return;
	}

	slot->remainingMs = slot->totalMs;
	slot->paused = s_wasPaused;
	CrowdRuntime_SendActiveEffectStatus(slot, CROWD_STATUS_RUNNING);
}

internal void CrowdRuntime_HandleStop(const struct CrowdJsonSlice *idRaw, const struct CrowdEffectDef *def)
{
	const s32 defIndex = (s32)(def - s_crowdEffectDefs);
	b32 stoppedAny = 0;

	for (s32 i = 0; i < CROWD_MAX_ACTIVE_EFFECTS; i++)
	{
		struct CrowdActiveEffect *effect = &s_activeEffects[i];

		if (!effect->inUse || (effect->effectIndex != defIndex))
		{
			continue;
		}

		CrowdRuntime_FinishEffect(effect);
		stoppedAny = 1;
	}

	CrowdProtocol_SendEffectStatus(idRaw, stoppedAny ? CROWD_STATUS_SUCCESS : CROWD_STATUS_FAILURE, 0, 0);
}

void CrowdRuntime_HandleEffectRequest(enum CrowdRequestType requestType, const struct CrowdJsonObject *request, const struct CrowdJsonValue *idValue)
{
	const struct CrowdJsonSlice *idRaw = (idValue != NULL) ? &idValue->raw : NULL;

	const struct CrowdJsonValue *codeValue = CrowdJson_Get(request, "code");
	if ((codeValue == NULL) || (codeValue->type != CROWD_JSON_STRING))
	{
		CrowdProtocol_SendEffectStatus(idRaw, CROWD_STATUS_FAILURE, 0, 0);
		return;
	}

	const struct CrowdEffectDef *def = CrowdRuntime_FindEffect(&codeValue->string);
	if (def == NULL)
	{
		CrowdProtocol_SendEffectStatus(idRaw, CROWD_STATUS_UNAVAILABLE, 0, 0);
		return;
	}

	if (requestType == CROWD_REQUEST_EFFECT_STOP)
	{
		CrowdRuntime_HandleStop(idRaw, def);
		return;
	}

	/* EffectTest and EffectStart both need the game to be in a state that could actually run the effect. */
	if (!CrowdRuntime_IsReady())
	{
		CrowdProtocol_SendEffectStatus(idRaw, CROWD_STATUS_RETRY, 0, 0);
		return;
	}

	if (requestType == CROWD_REQUEST_EFFECT_TEST)
	{
		CrowdProtocol_SendEffectStatus(idRaw, CROWD_STATUS_SUCCESS, 0, 0);
		return;
	}

	CrowdRuntime_HandleStart(idRaw, def, request);
}

void CrowdRuntime_ForceExpireAll(void)
{
	for (s32 i = 0; i < CROWD_MAX_ACTIVE_EFFECTS; i++)
	{
		struct CrowdActiveEffect *effect = &s_activeEffects[i];

		if (effect->inUse)
		{
			CrowdRuntime_FinishEffect(effect);
		}
	}
}

void CrowdRuntime_Tick(struct GameTracker *gGT)
{
	if (s_haveLastLevelId && (s_lastLevelId != gGT->levelID))
	{
		CrowdRuntime_ForceExpireAll();
	}
	s_lastLevelId = gGT->levelID;
	s_haveLastLevelId = 1;

	const b32 isPaused = (gGT->gameMode1 & PAUSE_ALL) != 0;

	if (isPaused != s_wasPaused)
	{
		const enum CrowdEffectStatus pauseStatus = isPaused ? CROWD_STATUS_PAUSED : CROWD_STATUS_RESUMED;

		for (s32 i = 0; i < CROWD_MAX_ACTIVE_EFFECTS; i++)
		{
			struct CrowdActiveEffect *effect = &s_activeEffects[i];

			if (effect->inUse)
			{
				effect->paused = isPaused;
				CrowdRuntime_SendActiveEffectStatus(effect, pauseStatus);
			}
		}

		s_wasPaused = isPaused;
	}

	if (isPaused)
	{
		return;
	}

	for (s32 i = 0; i < CROWD_MAX_ACTIVE_EFFECTS; i++)
	{
		struct CrowdActiveEffect *effect = &s_activeEffects[i];

		if (!effect->inUse)
		{
			continue;
		}

		effect->remainingMs -= gGT->elapsedTimeMS;
		if (effect->remainingMs <= 0)
		{
			CrowdRuntime_FinishEffect(effect);
		}
	}
}

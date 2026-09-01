#ifndef CROWD_RUNTIME_H
#define CROWD_RUNTIME_H

#include <macros.h>
#include <crowd/crowd_json.h>
#include <crowd/crowd_protocol.h>

/* Effect registry, request dispatch and timed-effect scheduling. Table shape is in crowd_effects.h. */

struct GameTracker;

enum CrowdEffectKind
{
	CROWD_EFFECT_INSTANT,
	CROWD_EFFECT_TIMED,
};

enum CrowdEffectCategory
{
	CROWD_EFFECT_CATEGORY_DEBUG,
	CROWD_EFFECT_CATEGORY_ITEM,
	CROWD_EFFECT_CATEGORY_CHEAT,
	CROWD_EFFECT_CATEGORY_INPUT,
	CROWD_EFFECT_CATEGORY_TERRAIN,
	CROWD_EFFECT_CATEGORY_BOTS,
	CROWD_EFFECT_CATEGORY_CHARACTER,
	CROWD_EFFECT_CATEGORY_ADVENTURE,
};

enum
{
	CROWD_MAX_ACTIVE_EFFECTS = 16,
	CROWD_REQUEST_ID_MAX = 64, /* raw JSON bytes, quotes included; longer ids are truncated */
};

struct CrowdActiveEffect
{
	s32 inUse;
	s32 effectIndex; /* index into the effect table this instance came from */
	char requestId[CROWD_REQUEST_ID_MAX];
	s32 requestIdLength;
	s32 quantity;
	s32 totalMs;
	s32 remainingMs;
	s32 paused;
};

/* Returning CROWD_STATUS_SUCCESS has the runtime track the effect (timed) or consider it done (instant). 
Any other status is sent straight back to Crowd Control untracked. */
typedef enum CrowdEffectStatus (*CrowdFxStartFn)(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request);
typedef void (*CrowdFxStopFn)(struct CrowdActiveEffect *effect); /* timed effects only */

void CrowdRuntime_HandleEffectRequest(enum CrowdRequestType requestType, const struct CrowdJsonObject *request, const struct CrowdJsonValue *idValue);

/* Call once per frame. Advances timed-effect clocks, expires anything that ran out, and reports Paused/Resumed. */
void CrowdRuntime_Tick(struct GameTracker *gGT);

void CrowdRuntime_ForceExpireAll(void);

#endif

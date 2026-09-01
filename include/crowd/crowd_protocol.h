#ifndef CROWD_PROTOCOL_H
#define CROWD_PROTOCOL_H

#include <macros.h>

struct CrowdJsonSlice;

/* SimpleTCP request/response enums. */

enum CrowdRequestType
{
	CROWD_REQUEST_EFFECT_TEST = 0x00,
	CROWD_REQUEST_EFFECT_START = 0x01,
	CROWD_REQUEST_EFFECT_STOP = 0x02,
	CROWD_REQUEST_DATA = 0x20,
	CROWD_REQUEST_GAME_UPDATE = 0xFD,
	CROWD_REQUEST_KEEP_ALIVE = 0xFF,
};

enum CrowdResponseType
{
	CROWD_RESPONSE_EFFECT_REQUEST = 0x00,
	CROWD_RESPONSE_EFFECT_STATUS = 0x01,
	CROWD_RESPONSE_GAME_UPDATE = 0xFD,
	CROWD_RESPONSE_KEEP_ALIVE = 0xFF,
};

enum CrowdEffectStatus
{
	CROWD_STATUS_SUCCESS = 0x00,
	CROWD_STATUS_FAILURE = 0x01,
	CROWD_STATUS_UNAVAILABLE = 0x02,
	CROWD_STATUS_RETRY = 0x03,
	CROWD_STATUS_QUEUE = 0x04,
	CROWD_STATUS_RUNNING = 0x05,
	CROWD_STATUS_PAUSED = 0x06,
	CROWD_STATUS_RESUMED = 0x07,
	CROWD_STATUS_FINISHED = 0x08,
};

/* Decodes one NUL-delimited frame's JSON body, dispatches it, and queues a response via CrowdNet_SendFrame. */
void CrowdProtocol_HandleFrame(const char *json, u32 length);

/* Encodes and sends one EffectStatus response.
- idRaw may be NULL to omit the "id" field. 
- Pass hasTimeRemaining=0 to omit the "timeRemaining" field. */
void CrowdProtocol_SendEffectStatus(const struct CrowdJsonSlice *idRaw, enum CrowdEffectStatus status, s32 timeRemainingMs, s32 hasTimeRemaining);

#endif

#include <crowd/crowd_protocol.h>

#include <crowd/crowd_json.h>
#include <crowd/crowd_net.h>
#include <crowd/crowd_runtime.h>

#include <platform/native_log.h>

void CrowdProtocol_SendEffectStatus(const struct CrowdJsonSlice *idRaw, enum CrowdEffectStatus status, s32 timeRemainingMs, s32 hasTimeRemaining)
{
	char scratch[256];
	struct CrowdJsonWriter writer;

	CrowdJsonWriter_Begin(&writer, scratch, sizeof(scratch));
	if (idRaw != NULL)
	{
		CrowdJsonWriter_RawField(&writer, "id", idRaw);
	}
	CrowdJsonWriter_IntField(&writer, "type", CROWD_RESPONSE_EFFECT_STATUS);
	CrowdJsonWriter_IntField(&writer, "status", (s32)status);
	if (hasTimeRemaining)
	{
		CrowdJsonWriter_IntField(&writer, "timeRemaining", timeRemainingMs);
	}

	const u32 total = CrowdJsonWriter_End(&writer);
	if (total == 0)
	{
		Platform_LogWarn("[CTR Crowd] response scratch buffer overflowed\n");
		return;
	}

	CrowdNet_SendFrame(scratch, total - 1);
}

internal void CrowdProtocol_SendGameUpdateAck(const struct CrowdJsonValue *idValue)
{
	char scratch[128];
	struct CrowdJsonWriter writer;

	CrowdJsonWriter_Begin(&writer, scratch, sizeof(scratch));
	if (idValue != NULL)
	{
		CrowdJsonWriter_RawField(&writer, "id", &idValue->raw);
	}
	CrowdJsonWriter_IntField(&writer, "type", CROWD_RESPONSE_GAME_UPDATE);

	const u32 total = CrowdJsonWriter_End(&writer);
	if (total == 0)
	{
		return;
	}

	CrowdNet_SendFrame(scratch, total - 1);
}

internal s32 CrowdProtocol_GetRequestType(const struct CrowdJsonObject *object, s32 *outType)
{
	const struct CrowdJsonValue *typeValue = CrowdJson_Get(object, "type");

	if ((typeValue == NULL) || (typeValue->type != CROWD_JSON_NUMBER))
	{
		return 0;
	}

	*outType = (s32)typeValue->number;
	return 1;
}

void CrowdProtocol_HandleFrame(const char *json, u32 length)
{
	struct CrowdJsonObject object;

	if (!CrowdJson_Parse(json, length, &object))
	{
		Platform_LogWarn("[CTR Crowd] dropped malformed request\n");
		return;
	}

	s32 requestType = 0;
	if (!CrowdProtocol_GetRequestType(&object, &requestType))
	{
		Platform_LogWarn("[CTR Crowd] dropped request with missing/invalid type\n");
		return;
	}

	const struct CrowdJsonValue *idValue = CrowdJson_Get(&object, "id");

	switch (requestType)
	{
	case CROWD_REQUEST_KEEP_ALIVE:
		/* No response expected. */
		break;

	case CROWD_REQUEST_EFFECT_TEST:
	case CROWD_REQUEST_EFFECT_START:
	case CROWD_REQUEST_EFFECT_STOP:
		CrowdRuntime_HandleEffectRequest((enum CrowdRequestType)requestType, &object, idValue);
		break;

	case CROWD_REQUEST_DATA:
	case CROWD_REQUEST_GAME_UPDATE:
		CrowdProtocol_SendGameUpdateAck(idValue);
		break;

	default:
		Platform_LogWarn("[CTR Crowd] dropped request with unknown type %d\n", (int)requestType);
		break;
	}
}

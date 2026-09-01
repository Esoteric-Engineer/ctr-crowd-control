#include <crowd/crowd_protocol.h>

#include <crowd/crowd_json.h>
#include <crowd/crowd_net.h>

#include <platform/native_log.h>

internal void CrowdProtocol_SendStatus(const struct CrowdJsonValue *idValue, enum CrowdEffectStatus status)
{
	char scratch[256];
	struct CrowdJsonWriter writer;

	CrowdJsonWriter_Begin(&writer, scratch, sizeof(scratch));
	if (idValue != NULL)
	{
		CrowdJsonWriter_RawField(&writer, "id", &idValue->raw);
	}
	CrowdJsonWriter_IntField(&writer, "type", CROWD_RESPONSE_EFFECT_STATUS);
	CrowdJsonWriter_IntField(&writer, "status", (s32)status);

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
	{
		const struct CrowdJsonValue *codeValue = CrowdJson_Get(&object, "code");

		if ((codeValue == NULL) || (codeValue->type != CROWD_JSON_STRING))
		{
			CrowdProtocol_SendStatus(idValue, CROWD_STATUS_FAILURE);
			break;
		}

		CrowdProtocol_SendStatus(idValue, CROWD_STATUS_UNAVAILABLE);
		break;
	}

	case CROWD_REQUEST_DATA:
	case CROWD_REQUEST_GAME_UPDATE:
		CrowdProtocol_SendGameUpdateAck(idValue);
		break;

	default:
		Platform_LogWarn("[CTR Crowd] dropped request with unknown type %d\n", (int)requestType);
		break;
	}
}

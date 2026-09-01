#include <crowd/crowd_json.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct CrowdJsonCursor
{
	const char *ptr;
	const char *end;
};

internal s32 CrowdJson_IsSpace(char c)
{
	return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
}

internal void CrowdJsonCursor_SkipSpace(struct CrowdJsonCursor *cur)
{
	while ((cur->ptr < cur->end) && CrowdJson_IsSpace(*cur->ptr))
	{
		cur->ptr++;
	}
}

internal s32 CrowdJsonCursor_Peek(const struct CrowdJsonCursor *cur, char *outChar)
{
	if (cur->ptr >= cur->end)
	{
		return 0;
	}

	*outChar = *cur->ptr;
	return 1;
}

internal s32 CrowdJsonCursor_Consume(struct CrowdJsonCursor *cur, char expected)
{
	char c;

	if (!CrowdJsonCursor_Peek(cur, &c) || (c != expected))
	{
		return 0;
	}

	cur->ptr++;
	return 1;
}

internal s32 CrowdJson_ParseString(struct CrowdJsonCursor *cur, struct CrowdJsonSlice *outRaw, struct CrowdJsonSlice *outContent)
{
	const char *start = cur->ptr;

	if (!CrowdJsonCursor_Consume(cur, '"'))
	{
		return 0;
	}

	const char *contentStart = cur->ptr;

	while (cur->ptr < cur->end)
	{
		const char c = *cur->ptr;

		if (c == '\\')
		{
			cur->ptr += 2; /* skip the escape pair */
			continue;
		}

		if (c == '"')
		{
			break;
		}

		cur->ptr++;
	}

	if ((cur->ptr >= cur->end) || (*cur->ptr != '"'))
	{
		return 0;
	}

	outContent->ptr = contentStart;
	outContent->length = (u32)(cur->ptr - contentStart);

	cur->ptr++; /* closing quote */

	outRaw->ptr = start;
	outRaw->length = (u32)(cur->ptr - start);
	return 1;
}

/* Finds end of an array/object at the cursor by tracking bracket depth.
Skips string contents so quoted brackets do not confuse the count. */
internal s32 CrowdJson_SkipRaw(struct CrowdJsonCursor *cur, struct CrowdJsonSlice *outRaw)
{
	const char *start = cur->ptr;
	char open;

	if (!CrowdJsonCursor_Peek(cur, &open) || ((open != '[') && (open != '{')))
	{
		return 0;
	}

	s32 depth = 0;

	while (cur->ptr < cur->end)
	{
		const char c = *cur->ptr;

		if (c == '"')
		{
			struct CrowdJsonSlice raw, content;
			if (!CrowdJson_ParseString(cur, &raw, &content))
			{
				return 0;
			}
			continue;
		}

		if ((c == '[') || (c == '{'))
		{
			depth++;
		}
		else if ((c == ']') || (c == '}'))
		{
			depth--;
		}

		cur->ptr++;

		if (depth == 0)
		{
			break;
		}
	}

	if (depth != 0)
	{
		return 0;
	}

	outRaw->ptr = start;
	outRaw->length = (u32)(cur->ptr - start);
	return 1;
}

internal s32 CrowdJson_ParseNumber(struct CrowdJsonCursor *cur, struct CrowdJsonSlice *outRaw, f64 *outNumber)
{
	const char *start = cur->ptr;

	if ((cur->ptr < cur->end) && (*cur->ptr == '-'))
	{
		cur->ptr++;
	}

	while (cur->ptr < cur->end)
	{
		const char c = *cur->ptr;
		const s32 isNumberChar = ((c >= '0') && (c <= '9')) || (c == '.') || (c == 'e') || (c == 'E') || (c == '+') || (c == '-');

		if (!isNumberChar)
		{
			break;
		}

		cur->ptr++;
	}

	if (cur->ptr == start)
	{
		return 0;
	}

	outRaw->ptr = start;
	outRaw->length = (u32)(cur->ptr - start);

	/* the buffer this cursor scans is always NUL-terminated */
	*outNumber = strtod(start, NULL);
	return 1;
}

internal s32 CrowdJson_MatchLiteral(struct CrowdJsonCursor *cur, const char *lit, u32 litLen)
{
	if ((u32)(cur->end - cur->ptr) < litLen)
	{
		return 0;
	}

	if (memcmp(cur->ptr, lit, litLen) != 0)
	{
		return 0;
	}

	cur->ptr += litLen;
	return 1;
}

internal s32 CrowdJson_ParseValue(struct CrowdJsonCursor *cur, struct CrowdJsonValue *outValue)
{
	char c;

	if (!CrowdJsonCursor_Peek(cur, &c))
	{
		return 0;
	}

	if (c == '"')
	{
		outValue->type = CROWD_JSON_STRING;
		return CrowdJson_ParseString(cur, &outValue->raw, &outValue->string);
	}

	if ((c == '[') || (c == '{'))
	{
		outValue->type = CROWD_JSON_RAW;
		return CrowdJson_SkipRaw(cur, &outValue->raw);
	}

	if (c == 't')
	{
		const char *start = cur->ptr;
		if (!CrowdJson_MatchLiteral(cur, "true", 4))
		{
			return 0;
		}
		outValue->type = CROWD_JSON_BOOL;
		outValue->boolean = 1;
		outValue->raw.ptr = start;
		outValue->raw.length = 4;
		return 1;
	}

	if (c == 'f')
	{
		const char *start = cur->ptr;
		if (!CrowdJson_MatchLiteral(cur, "false", 5))
		{
			return 0;
		}
		outValue->type = CROWD_JSON_BOOL;
		outValue->boolean = 0;
		outValue->raw.ptr = start;
		outValue->raw.length = 5;
		return 1;
	}

	if (c == 'n')
	{
		const char *start = cur->ptr;
		if (!CrowdJson_MatchLiteral(cur, "null", 4))
		{
			return 0;
		}
		outValue->type = CROWD_JSON_NULL;
		outValue->raw.ptr = start;
		outValue->raw.length = 4;
		return 1;
	}

	if ((c == '-') || ((c >= '0') && (c <= '9')))
	{
		outValue->type = CROWD_JSON_NUMBER;
		return CrowdJson_ParseNumber(cur, &outValue->raw, &outValue->number);
	}

	return 0;
}

s32 CrowdJson_Parse(const char *json, u32 length, struct CrowdJsonObject *outObject)
{
	struct CrowdJsonCursor cur;
	cur.ptr = json;
	cur.end = json + length;

	outObject->fieldCount = 0;

	CrowdJsonCursor_SkipSpace(&cur);
	if (!CrowdJsonCursor_Consume(&cur, '{'))
	{
		return 0;
	}

	CrowdJsonCursor_SkipSpace(&cur);

	char c;
	if (CrowdJsonCursor_Peek(&cur, &c) && (c == '}'))
	{
		return 1;
	}

	for (;;)
	{
		CrowdJsonCursor_SkipSpace(&cur);

		struct CrowdJsonSlice keyRaw, keyContent;
		if (!CrowdJsonCursor_Peek(&cur, &c) || (c != '"') || !CrowdJson_ParseString(&cur, &keyRaw, &keyContent))
		{
			return 0;
		}

		CrowdJsonCursor_SkipSpace(&cur);
		if (!CrowdJsonCursor_Consume(&cur, ':'))
		{
			return 0;
		}
		CrowdJsonCursor_SkipSpace(&cur);

		struct CrowdJsonValue value;
		if (!CrowdJson_ParseValue(&cur, &value))
		{
			return 0;
		}

		if (outObject->fieldCount >= CROWD_JSON_MAX_FIELDS)
		{
			return 0; /* well under 1 KiB requests should never have this many keys */
		}

		outObject->fields[outObject->fieldCount].key = keyContent;
		outObject->fields[outObject->fieldCount].value = value;
		outObject->fieldCount++;

		CrowdJsonCursor_SkipSpace(&cur);
		if (!CrowdJsonCursor_Peek(&cur, &c))
		{
			return 0;
		}

		if (c == ',')
		{
			cur.ptr++;
			continue;
		}

		if (c == '}')
		{
			break;
		}

		return 0;
	}

	return 1;
}

const struct CrowdJsonValue *CrowdJson_Get(const struct CrowdJsonObject *object, const char *key)
{
	const u32 keyLen = (u32)strlen(key);

	for (s32 i = 0; i < object->fieldCount; i++)
	{
		const struct CrowdJsonField *field = &object->fields[i];

		if ((field->key.length == keyLen) && (memcmp(field->key.ptr, key, keyLen) == 0))
		{
			return &field->value;
		}
	}

	return NULL;
}

internal s32 CrowdJsonWriter_Append(struct CrowdJsonWriter *writer, const char *data, u32 length)
{
	if (writer->overflow)
	{
		return 0;
	}

	if ((writer->length + length + 1) > writer->capacity) /* +1 reserves room for the closing NUL */
	{
		writer->overflow = 1;
		return 0;
	}

	memcpy(writer->buf + writer->length, data, length);
	writer->length += length;
	return 1;
}

internal s32 CrowdJsonWriter_AppendChar(struct CrowdJsonWriter *writer, char c)
{
	return CrowdJsonWriter_Append(writer, &c, 1);
}

internal s32 CrowdJsonWriter_AppendCStr(struct CrowdJsonWriter *writer, const char *s)
{
	return CrowdJsonWriter_Append(writer, s, (u32)strlen(s));
}

internal void CrowdJsonWriter_BeginField(struct CrowdJsonWriter *writer, const char *key)
{
	if (writer->fieldCount > 0)
	{
		CrowdJsonWriter_AppendChar(writer, ',');
	}

	writer->fieldCount++;
	CrowdJsonWriter_AppendChar(writer, '"');
	CrowdJsonWriter_AppendCStr(writer, key);
	CrowdJsonWriter_AppendCStr(writer, "\":");
}

void CrowdJsonWriter_Begin(struct CrowdJsonWriter *writer, char *buf, u32 capacity)
{
	writer->buf = buf;
	writer->capacity = capacity;
	writer->length = 0;
	writer->fieldCount = 0;
	writer->overflow = 0;
	CrowdJsonWriter_AppendChar(writer, '{');
}

void CrowdJsonWriter_RawField(struct CrowdJsonWriter *writer, const char *key, const struct CrowdJsonSlice *rawValue)
{
	CrowdJsonWriter_BeginField(writer, key);
	CrowdJsonWriter_Append(writer, rawValue->ptr, rawValue->length);
}

void CrowdJsonWriter_StringField(struct CrowdJsonWriter *writer, const char *key, const char *value)
{
	CrowdJsonWriter_BeginField(writer, key);
	CrowdJsonWriter_AppendChar(writer, '"');

	for (const char *p = value; *p != '\0'; p++)
	{
		const char c = *p;

		if ((c == '"') || (c == '\\'))
		{
			CrowdJsonWriter_AppendChar(writer, '\\');
			CrowdJsonWriter_AppendChar(writer, c);
		}
		else if (c == '\n')
		{
			CrowdJsonWriter_AppendCStr(writer, "\\n");
		}
		else if (c == '\r')
		{
			CrowdJsonWriter_AppendCStr(writer, "\\r");
		}
		else if (c == '\t')
		{
			CrowdJsonWriter_AppendCStr(writer, "\\t");
		}
		else
		{
			CrowdJsonWriter_AppendChar(writer, c);
		}
	}

	CrowdJsonWriter_AppendChar(writer, '"');
}

void CrowdJsonWriter_IntField(struct CrowdJsonWriter *writer, const char *key, s32 value)
{
	char tmp[16];
	snprintf(tmp, sizeof(tmp), "%d", (int)value);

	CrowdJsonWriter_BeginField(writer, key);
	CrowdJsonWriter_AppendCStr(writer, tmp);
}

u32 CrowdJsonWriter_End(struct CrowdJsonWriter *writer)
{
	if (!CrowdJsonWriter_AppendChar(writer, '}'))
	{
		return 0;
	}

	if (writer->overflow || (writer->length >= writer->capacity))
	{
		writer->overflow = 1;
		return 0;
	}

	writer->buf[writer->length] = '\0';
	return writer->length + 1;
}

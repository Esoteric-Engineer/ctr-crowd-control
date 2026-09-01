#ifndef CROWD_JSON_H
#define CROWD_JSON_H

#include <macros.h>

/* Scanner for the flat JSON objects SimpleTCP uses. Note that string values are NOT unescaped: `string` is the raw source bytes between the quotes. */

struct CrowdJsonSlice
{
	const char *ptr;
	u32 length;
};

enum CrowdJsonValueType
{
	CROWD_JSON_STRING,
	CROWD_JSON_NUMBER,
	CROWD_JSON_BOOL,
	CROWD_JSON_NULL,
	CROWD_JSON_RAW, /* array or object */
};

struct CrowdJsonValue
{
	enum CrowdJsonValueType type;
	struct CrowdJsonSlice raw;
	struct CrowdJsonSlice string;
	f64 number;
	s32 boolean;
};

enum
{
	CROWD_JSON_MAX_FIELDS = 16,
};

struct CrowdJsonField
{
	struct CrowdJsonSlice key;
	struct CrowdJsonValue value;
};

struct CrowdJsonObject
{
	struct CrowdJsonField fields[CROWD_JSON_MAX_FIELDS];
	s32 fieldCount;
};

/* Parses one flat top-level JSON object from [json, json + length).
Returns 0 on malformed input or if the object has more than CROWD_JSON_MAX_FIELDS keys */
s32 CrowdJson_Parse(const char *json, u32 length, struct CrowdJsonObject *outObject);

/* Returns the field's value, or NULL if `key` is not present. */
const struct CrowdJsonValue *CrowdJson_Get(const struct CrowdJsonObject *object, const char *key);

/* Fixed-buffer append-only writer for responses. */
struct CrowdJsonWriter
{
	char *buf;
	u32 capacity;
	u32 length;
	s32 fieldCount;
	s32 overflow;
};

void CrowdJsonWriter_Begin(struct CrowdJsonWriter *writer, char *buf, u32 capacity);
/* Writes `key` with rawValue's bytes spliced in verbatim - used to echo a request's `id` back */
void CrowdJsonWriter_RawField(struct CrowdJsonWriter *writer, const char *key, const struct CrowdJsonSlice *rawValue);
void CrowdJsonWriter_StringField(struct CrowdJsonWriter *writer, const char *key, const char *value);
void CrowdJsonWriter_IntField(struct CrowdJsonWriter *writer, const char *key, s32 value);
/* Closes the object and NUL-terminates. Returns the total byte length written including the NUL, or 0. */
u32 CrowdJsonWriter_End(struct CrowdJsonWriter *writer);

#endif

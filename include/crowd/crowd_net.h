#ifndef CROWD_NET_H
#define CROWD_NET_H

#include <macros.h>

/* Non-blocking TCP client for Crowd Control's SimpleTCP connector.
Everything here is meant to be injected once per game frame (no background thread). */

enum CrowdNetState
{
	CROWD_NET_DISCONNECTED,
	CROWD_NET_CONNECTING,
	CROWD_NET_CONNECTED,
};

/* Records the target and resets to a fresh, disconnected state. */
void CrowdNet_Init(const char *host, s32 port);
void CrowdNet_Shutdown(void);

/* Drives the connect/reconnect state machine and pumps send/recv on an established connection. */
void CrowdNet_Pump(void);

enum CrowdNetState CrowdNet_GetState(void);

/* Pops the oldest complete NUL-delimited frame into outBuf.
Returns the frame length excluding the NUL, or 0 if no complete frame is buffered yet, in which case outBuf is left untouched.
An oversized frame is dropped with a log line rather than truncated, so framing can't desync. */
u32 CrowdNet_PopFrame(char *outBuf, u32 outCapacity);

/* Queues `data` followed by a NUL for sending on the next Pump(). */
s32 CrowdNet_SendFrame(const char *data, u32 length);

#endif

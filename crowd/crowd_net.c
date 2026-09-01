#include <crowd/crowd_net.h>

#include <platform/native_log.h>

#include <string.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET CrowdSocket;
typedef int CrowdSockLen;
#define CROWD_INVALID_SOCKET INVALID_SOCKET
#else
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int CrowdSocket;
typedef socklen_t CrowdSockLen;
#define CROWD_INVALID_SOCKET (-1)
#endif

enum
{
	CROWD_NET_HOST_MAX = 128,
	CROWD_NET_RING_CAPACITY = 4096,
	CROWD_NET_RECONNECT_DELAY_FRAMES = FPS,
	CROWD_NET_CONNECT_TIMEOUT_FRAMES = FPS * 5,
};

struct CrowdNetRing
{
	u8 data[CROWD_NET_RING_CAPACITY];
	u32 head;
	u32 count;
};

internal u32 CrowdNetRing_FreeSpace(const struct CrowdNetRing *ring)
{
	return CROWD_NET_RING_CAPACITY - ring->count;
}

internal u32 CrowdNetRing_Write(struct CrowdNetRing *ring, const u8 *data, u32 length)
{
	const u32 freeSpace = CrowdNetRing_FreeSpace(ring);
	const u32 writable = (length < freeSpace) ? length : freeSpace;
	const u32 tail = (ring->head + ring->count) % CROWD_NET_RING_CAPACITY;
	const u32 tailRoom = CROWD_NET_RING_CAPACITY - tail;
	const u32 firstChunk = (writable < tailRoom) ? writable : tailRoom;

	memcpy(ring->data + tail, data, firstChunk);
	if (writable > firstChunk)
	{
		memcpy(ring->data, data + firstChunk, writable - firstChunk);
	}

	ring->count += writable;
	return writable;
}

/* Byte at logical offset `offset` from head; offset must be < ring->count. */
internal u8 CrowdNetRing_At(const struct CrowdNetRing *ring, u32 offset)
{
	return ring->data[(ring->head + offset) % CROWD_NET_RING_CAPACITY];
}

internal void CrowdNetRing_Discard(struct CrowdNetRing *ring, u32 length)
{
	ring->head = (ring->head + length) % CROWD_NET_RING_CAPACITY;
	ring->count -= length;
}

/* Copies `length` bytes from head into outBuf and pops them. */
internal void CrowdNetRing_Read(struct CrowdNetRing *ring, u8 *outBuf, u32 length)
{
	if (outBuf != NULL)
	{
		for (u32 i = 0; i < length; i++)
		{
			outBuf[i] = CrowdNetRing_At(ring, i);
		}
	}

	CrowdNetRing_Discard(ring, length);
}

/* Offset of the first occurrence of `target` from head, or -1 if not buffered. */
internal s32 CrowdNetRing_Find(const struct CrowdNetRing *ring, u8 target)
{
	for (u32 i = 0; i < ring->count; i++)
	{
		if (CrowdNetRing_At(ring, i) == target)
		{
			return (s32)i;
		}
	}

	return -1;
}

global_variable CrowdSocket s_sock = CROWD_INVALID_SOCKET;
global_variable enum CrowdNetState s_state = CROWD_NET_DISCONNECTED;
global_variable char s_host[CROWD_NET_HOST_MAX];
global_variable s32 s_port;
global_variable u32 s_frameCounter;
global_variable u32 s_reconnectAtFrame;
global_variable u32 s_connectStartFrame;
global_variable struct CrowdNetRing s_recvRing;
global_variable struct CrowdNetRing s_sendRing;

internal s32 CrowdNet_SetNonBlocking(CrowdSocket sock)
{
#if defined(_WIN32)
	u_long mode = 1;
	return ioctlsocket(sock, FIONBIO, &mode) == 0;
#else
	const int flags = fcntl(sock, F_GETFL, 0);
	if (flags < 0)
	{
		return 0;
	}
	return fcntl(sock, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
}

internal void CrowdNet_CloseSocket(CrowdSocket sock)
{
#if defined(_WIN32)
	closesocket(sock);
#else
	close(sock);
#endif
}

internal s32 CrowdNet_WouldBlock(void)
{
#if defined(_WIN32)
	return WSAGetLastError() == WSAEWOULDBLOCK;
#else
	return (errno == EWOULDBLOCK) || (errno == EAGAIN);
#endif
}

internal s32 CrowdNet_ConnectInProgress(void)
{
#if defined(_WIN32)
	return WSAGetLastError() == WSAEWOULDBLOCK;
#else
	return errno == EINPROGRESS;
#endif
}

internal void CrowdNet_ResetBuffers(void)
{
	s_recvRing.head = 0;
	s_recvRing.count = 0;
	s_sendRing.head = 0;
	s_sendRing.count = 0;
}

internal void CrowdNet_ScheduleReconnect(void)
{
	if (s_sock != CROWD_INVALID_SOCKET)
	{
		CrowdNet_CloseSocket(s_sock);
		s_sock = CROWD_INVALID_SOCKET;
	}

	s_state = CROWD_NET_DISCONNECTED;
	s_reconnectAtFrame = s_frameCounter + CROWD_NET_RECONNECT_DELAY_FRAMES;
}

internal void CrowdNet_HandleDisconnect(void)
{
	const s32 wasConnected = (s_state == CROWD_NET_CONNECTED);

	CrowdNet_ScheduleReconnect();
	CrowdNet_ResetBuffers();

	if (wasConnected)
	{
		Platform_Log("[CTR Crowd] disconnected, retrying shortly\n");
	}
}

internal void CrowdNet_BeginConnect(void)
{
	s_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s_sock == CROWD_INVALID_SOCKET)
	{
		s_reconnectAtFrame = s_frameCounter + CROWD_NET_RECONNECT_DELAY_FRAMES;
		return;
	}

	if (!CrowdNet_SetNonBlocking(s_sock))
	{
		CrowdNet_CloseSocket(s_sock);
		s_sock = CROWD_INVALID_SOCKET;
		s_reconnectAtFrame = s_frameCounter + CROWD_NET_RECONNECT_DELAY_FRAMES;
		return;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons((u16)s_port);

	if (inet_pton(AF_INET, s_host, &addr.sin_addr) != 1)
	{
		Platform_LogWarn("[CTR Crowd] invalid --crowd-host address: %s\n", s_host);
		CrowdNet_CloseSocket(s_sock);
		s_sock = CROWD_INVALID_SOCKET;
		s_reconnectAtFrame = s_frameCounter + CROWD_NET_RECONNECT_DELAY_FRAMES;
		return;
	}

	const int rc = connect(s_sock, (struct sockaddr *)&addr, sizeof(addr));
	if (rc == 0)
	{
		/* the handshake can complete synchronously for loopback */
		s_state = CROWD_NET_CONNECTED;
		Platform_Log("[CTR Crowd] connected to %s:%d\n", s_host, (int)s_port);
		return;
	}

	if (!CrowdNet_ConnectInProgress())
	{
		CrowdNet_CloseSocket(s_sock);
		s_sock = CROWD_INVALID_SOCKET;
		s_reconnectAtFrame = s_frameCounter + CROWD_NET_RECONNECT_DELAY_FRAMES;
		return;
	}

	s_state = CROWD_NET_CONNECTING;
	s_connectStartFrame = s_frameCounter;
}

internal void CrowdNet_PumpConnecting(void)
{
	fd_set writeSet, errorSet;
	FD_ZERO(&writeSet);
	FD_ZERO(&errorSet);
	FD_SET(s_sock, &writeSet);
	FD_SET(s_sock, &errorSet);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	const int rc = select((int)(s_sock + 1), NULL, &writeSet, &errorSet, &tv);

	if (rc > 0)
	{
		if (FD_ISSET(s_sock, &errorSet))
		{
			CrowdNet_HandleDisconnect();
			return;
		}

		if (FD_ISSET(s_sock, &writeSet))
		{
			int soError = 0;
			CrowdSockLen soErrorLen = sizeof(soError);

			if ((getsockopt(s_sock, SOL_SOCKET, SO_ERROR, (char *)&soError, &soErrorLen) != 0) || (soError != 0))
			{
				CrowdNet_HandleDisconnect();
				return;
			}

			s_state = CROWD_NET_CONNECTED;
			Platform_Log("[CTR Crowd] connected to %s:%d\n", s_host, (int)s_port);
			return;
		}
	}

	if ((s_frameCounter - s_connectStartFrame) >= CROWD_NET_CONNECT_TIMEOUT_FRAMES)
	{
		Platform_LogWarn("[CTR Crowd] connect to %s:%d timed out, retrying shortly\n", s_host, (int)s_port);
		CrowdNet_HandleDisconnect();
	}
}

internal void CrowdNet_PumpRecv(void)
{
	u8 buf[512];

	for (;;)
	{
		const u32 available = CrowdNetRing_FreeSpace(&s_recvRing);
		if (available == 0)
		{
			break;
		}

		const int wanted = (int)((available < sizeof(buf)) ? available : sizeof(buf));
		const int received = recv(s_sock, (char *)buf, wanted, 0);

		if (received > 0)
		{
			CrowdNetRing_Write(&s_recvRing, buf, (u32)received);
			if (received < wanted)
			{
				break; /* drained the socket for this frame */
			}
			continue;
		}

		if (received == 0)
		{
			CrowdNet_HandleDisconnect();
			return;
		}

		if (!CrowdNet_WouldBlock())
		{
			CrowdNet_HandleDisconnect();
			return;
		}

		break;
	}
}

internal void CrowdNet_PumpSend(void)
{
	u8 buf[512];

	while (s_sendRing.count > 0)
	{
		const u32 chunk = (s_sendRing.count < sizeof(buf)) ? s_sendRing.count : (u32)sizeof(buf);

		for (u32 i = 0; i < chunk; i++)
		{
			buf[i] = CrowdNetRing_At(&s_sendRing, i);
		}

		const int sent = send(s_sock, (const char *)buf, (int)chunk, 0);

		if (sent > 0)
		{
			CrowdNetRing_Discard(&s_sendRing, (u32)sent);
			if ((u32)sent < chunk)
			{
				break; /* socket send buffer is full for this frame */
			}
			continue;
		}

		if ((sent < 0) && !CrowdNet_WouldBlock())
		{
			CrowdNet_HandleDisconnect();
		}
		break;
	}
}

void CrowdNet_Init(const char *host, s32 port)
{
#if defined(_WIN32)
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

	strncpy(s_host, host, sizeof(s_host) - 1);
	s_host[sizeof(s_host) - 1] = '\0';
	s_port = port;

	s_sock = CROWD_INVALID_SOCKET;
	s_state = CROWD_NET_DISCONNECTED;
	s_frameCounter = 0;
	s_reconnectAtFrame = 0;
	CrowdNet_ResetBuffers();
}

void CrowdNet_Shutdown(void)
{
	if (s_sock != CROWD_INVALID_SOCKET)
	{
		CrowdNet_CloseSocket(s_sock);
		s_sock = CROWD_INVALID_SOCKET;
	}
	s_state = CROWD_NET_DISCONNECTED;

#if defined(_WIN32)
	WSACleanup();
#endif
}

void CrowdNet_Pump(void)
{
	s_frameCounter++;

	if (s_state == CROWD_NET_DISCONNECTED)
	{
		if (s_frameCounter >= s_reconnectAtFrame)
		{
			CrowdNet_BeginConnect();
		}
		return;
	}

	if (s_state == CROWD_NET_CONNECTING)
	{
		CrowdNet_PumpConnecting();
		return;
	}

	CrowdNet_PumpSend();
	if (s_state == CROWD_NET_CONNECTED)
	{
		CrowdNet_PumpRecv();
	}
}

enum CrowdNetState CrowdNet_GetState(void)
{
	return s_state;
}

u32 CrowdNet_PopFrame(char *outBuf, u32 outCapacity)
{
	const s32 nulOffset = CrowdNetRing_Find(&s_recvRing, 0x00);
	if (nulOffset < 0)
	{
		return 0;
	}

	const u32 frameLength = (u32)nulOffset;

	if ((frameLength + 1) > outCapacity)
	{
		CrowdNetRing_Discard(&s_recvRing, frameLength + 1);
		Platform_LogWarn("[CTR Crowd] dropped oversized frame (%u bytes)\n", frameLength);
		return 0;
	}

	CrowdNetRing_Read(&s_recvRing, (u8 *)outBuf, frameLength);
	outBuf[frameLength] = '\0';
	CrowdNetRing_Discard(&s_recvRing, 1); /* the NUL delimiter itself */

	return frameLength;
}

s32 CrowdNet_SendFrame(const char *data, u32 length)
{
	if ((length + 1) > CrowdNetRing_FreeSpace(&s_sendRing))
	{
		Platform_LogWarn("[CTR Crowd] send ring full, dropping a %u-byte response\n", length);
		return 0;
	}

	CrowdNetRing_Write(&s_sendRing, (const u8 *)data, length);

	const u8 nul = 0x00;
	CrowdNetRing_Write(&s_sendRing, &nul, 1);
	return 1;
}

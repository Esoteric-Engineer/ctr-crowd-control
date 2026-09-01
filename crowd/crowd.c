#include <crowd/crowd.h>
#include <crowd/crowd_config.h>
#include <crowd/crowd_fx_input.h>
#include <crowd/crowd_net.h>
#include <crowd/crowd_protocol.h>
#include <crowd/crowd_runtime.h>

#include <platform/native_log.h>

global_variable s32 s_crowdRunning = 0;

void Crowd_Init(void)
{
	const struct CrowdConfig *config = Crowd_GetConfig();

	if (!config->enabled)
	{
		return;
	}

	CrowdNet_Init(config->host, config->port);

	s_crowdRunning = 1;

	Platform_Log("[CTR Crowd] enabled, target %s:%d\n", config->host, (int)config->port);
}

void Crowd_Shutdown(void)
{
	if (!s_crowdRunning)
	{
		return;
	}

	CrowdNet_Shutdown();

	s_crowdRunning = 0;

	Platform_Log("[CTR Crowd] shut down\n");
}

int Crowd_IsEnabled(void)
{
	return s_crowdRunning;
}

void Crowd_Tick(struct GameTracker *gGT)
{
	if (!s_crowdRunning)
	{
		return;
	}

	CrowdNet_Pump();

	char frame[1024];
	u32 frameLength;
	while ((frameLength = CrowdNet_PopFrame(frame, sizeof(frame))) > 0)
	{
		CrowdProtocol_HandleFrame(frame, frameLength);
	}

	CrowdRuntime_Tick(gGT);
}

void Crowd_ApplyInputMask(void)
{
	if (!s_crowdRunning)
	{
		return;
	}

	CrowdFxInput_ApplyMask();
}

u8 Crowd_MapTerrain(u8 terrainType)
{
	if (!s_crowdRunning)
	{
		return terrainType;
	}

	// TODO(crowd): remap terrain here
	return terrainType;
}

int Crowd_ScaleBotDelta(int delta)
{
	if (!s_crowdRunning)
	{
		return delta;
	}

	// TODO(crowd): scale AI nav progress here
	return delta;
}

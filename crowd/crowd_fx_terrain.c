#include <crowd/crowd_fx_terrain.h>

#include <crowd/crowd_runtime.h>

#include <common.h>

/* Only one terrain effect can be active at a time. */
enum CrowdTerrainMode
{
	CROWD_TERRAIN_MODE_NONE,
	CROWD_TERRAIN_MODE_NO_ROUGH,
	CROWD_TERRAIN_MODE_ALL_DIRT,
};

global_variable enum CrowdTerrainMode s_terrainMode = CROWD_TERRAIN_MODE_NONE;

enum CrowdEffectStatus Crowd_Fx_TerrainNoRough_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	if (s_terrainMode != CROWD_TERRAIN_MODE_NONE)
	{
		return CROWD_STATUS_RETRY;
	}

	s_terrainMode = CROWD_TERRAIN_MODE_NO_ROUGH;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_TerrainNoRough_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	if (s_terrainMode == CROWD_TERRAIN_MODE_NO_ROUGH)
	{
		s_terrainMode = CROWD_TERRAIN_MODE_NONE;
	}
}

enum CrowdEffectStatus Crowd_Fx_TerrainAllDirt_Start(struct CrowdActiveEffect *effect, const struct CrowdJsonObject *request)
{
	(void)effect;
	(void)request;

	if (s_terrainMode != CROWD_TERRAIN_MODE_NONE)
	{
		return CROWD_STATUS_RETRY;
	}

	s_terrainMode = CROWD_TERRAIN_MODE_ALL_DIRT;
	return CROWD_STATUS_SUCCESS;
}

void Crowd_Fx_TerrainAllDirt_Stop(struct CrowdActiveEffect *effect)
{
	(void)effect;

	if (s_terrainMode == CROWD_TERRAIN_MODE_ALL_DIRT)
	{
		s_terrainMode = CROWD_TERRAIN_MODE_NONE;
	}
}

u8 CrowdFxTerrain_MapTerrain(u8 terrainType)
{
	switch (s_terrainMode)
	{
	case CROWD_TERRAIN_MODE_NO_ROUGH:
		switch (terrainType)
		{
		case TERRAIN_DIRT:
		case TERRAIN_SLOWDIRT:
		case TERRAIN_GRASS:
		case TERRAIN_SLOWGRASS:
		case TERRAIN_MUD:
			return TERRAIN_ASPHALT;
		default:
			return terrainType;
		}

	case CROWD_TERRAIN_MODE_ALL_DIRT:
		if (terrainType == TERRAIN_NONE)
		{
			return terrainType;
		}
		return TERRAIN_DIRT;

	case CROWD_TERRAIN_MODE_NONE:
	default:
		return terrainType;
	}
}

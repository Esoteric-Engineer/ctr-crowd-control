#ifndef CROWD_H
#define CROWD_H

#include <macros.h>

/*
 * Crowd Control integration.
 *
 * NOTE(crowd): the prefix is `Crowd_`/`CROWD_`, never `CC_`.
 * `CC_` already means Crystal Challenge (see tools/matching/overlays/221/abi.h).
 */

struct GameTracker;

/* Parses --crowd-control, --crowd-host and --crowd-port.
Returns 0 on success, non-zero when an arg is malformed. */
int Crowd_ConfigureFromArgs(int argc, char **argv);

/* Brings up the socket layer. No-op unless --crowd-control was passed. */
void Crowd_Init(void);
void Crowd_Shutdown(void);

int Crowd_IsEnabled(void);

/* Handles network I/O, request dispatch and timed-effect record keeping. */
void Crowd_Tick(struct GameTracker *gGT);

/* Applies the gamepad mask for input-denial effects on player 0. */
void Crowd_ApplyInputMask(void);

/* Terrain remap choke point, called from VehAfterColl_GetTerrain.
Returns terrainType unchanged when no terrain effect is active. */
u8 Crowd_MapTerrain(u8 terrainType);

/* Scales one frame of AI nav-path progress. Returns delta unchanged if no effect is active. */
int Crowd_ScaleBotDelta(int delta);

#endif

#ifndef CROWD_FX_PLAYER_SPEED_H
#define CROWD_FX_PLAYER_SPEED_H

#include <macros.h>

/* maxSpeed is the caller's own 100% speed reference, used for capping thresholds.
Returns speed unchanged when no player-speed effect is active. */
int CrowdFxPlayerSpeed_ScaleSpeed(int speed, int maxSpeed);

#endif

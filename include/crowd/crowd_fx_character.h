#ifndef CROWD_FX_CHARACTER_H
#define CROWD_FX_CHARACTER_H

#include <macros.h>

/* Call every frame regardless of effect state. Drops the off-roster model cache across load boundaries. */
void CrowdFxCharacter_Tick(void);

#endif

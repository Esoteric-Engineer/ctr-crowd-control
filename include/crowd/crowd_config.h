#ifndef CROWD_CONFIG_H
#define CROWD_CONFIG_H

#include <macros.h>

enum CrowdConfigConstants
{
	/* Crowd Control's default SimpleTCP port.
	    Note that The C# pack must declare the same value in its `Port` property. */
	CROWD_DEFAULT_PORT = 58430,

	CROWD_HOST_MAX = 128,
};

struct CrowdConfig
{
	s32 enabled;
	s32 port;
	char host[CROWD_HOST_MAX];
};

int Crowd_ConfigureFromArgs(int argc, char **argv);
const struct CrowdConfig *Crowd_GetConfig(void);

#endif

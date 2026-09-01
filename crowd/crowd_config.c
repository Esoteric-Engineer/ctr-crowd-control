#include <crowd/crowd_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

global_variable struct CrowdConfig s_crowdConfig = {
    .enabled = 0,
    .port = CROWD_DEFAULT_PORT,
    .host = "127.0.0.1",
};

internal const char *Crowd_ArgValue(int argc, char **argv, const char *name)
{
	for (s32 i = 1; i < argc - 1; i++)
	{
		if (strcmp(argv[i], name) == 0)
		{
			return argv[i + 1];
		}
	}

	return NULL;
}

internal s32 Crowd_ArgPresent(int argc, char **argv, const char *name)
{
	for (s32 i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], name) == 0)
		{
			return 1;
		}
	}

	return 0;
}

internal s32 Crowd_SetHost(const char *host)
{
	const size_t length = strlen(host);

	if ((length == 0) || (length >= sizeof(s_crowdConfig.host)))
	{
		return 0;
	}

	memcpy(s_crowdConfig.host, host, length + 1);
	return 1;
}

internal s32 Crowd_SetPort(const char *port)
{
	char *end = NULL;
	const long value = strtol(port, &end, 10);

	if ((end == port) || (*end != '\0') || (value < 1) || (value > 65535))
	{
		return 0;
	}

	s_crowdConfig.port = (s32)value;
	return 1;
}

int Crowd_ConfigureFromArgs(int argc, char **argv)
{
	const char *host = Crowd_ArgValue(argc, argv, "--crowd-host");
	const char *port = Crowd_ArgValue(argc, argv, "--crowd-port");
	const s32 enabled = Crowd_ArgPresent(argc, argv, "--crowd-control") || (host != NULL) || (port != NULL);

	if (!enabled)
	{
		return 0;
	}

	if ((host != NULL) && !Crowd_SetHost(host))
	{
		fprintf(stderr, "[CTR Crowd] invalid --crowd-host: %s\n", host);
		return 1;
	}

	if ((port != NULL) && !Crowd_SetPort(port))
	{
		fprintf(stderr, "[CTR Crowd] invalid --crowd-port: %s\n", port);
		return 1;
	}

	s_crowdConfig.enabled = 1;
	return 0;
}

const struct CrowdConfig *Crowd_GetConfig(void)
{
	return &s_crowdConfig;
}

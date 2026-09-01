# Crowd Control

`ctr_native` can accept effect requests from the
[Crowd Control](https://developer.crowdcontrol.live/) desktop app, letting viewers spend channel
points on in-game effects.

## Running

```
ctr_native --crowd-control
ctr_native --crowd-host 127.0.0.1 --crowd-port 58430
```

`--crowd-control` enables the integration. `--crowd-host` and `--crowd-port` override the target and
imply `--crowd-control`. The defaults are `127.0.0.1` and `58430`. Without any of these flags the
integration is completely inert — no socket is opened and every hook is a no-op.

The build can also be excluded entirely:

```
cmake --preset linux-gcc-i686-release -DCTR_CROWD_CONTROL=OFF
```

## How it connects

Crowd Control's SimpleTCP connector runs in *server mode*: the desktop app listens and the game
connects to it as a client. Messages are NUL-terminated (`0x00`) UTF-8 JSON in both directions.

The socket is non-blocking and pumped once per frame from the main gameplay loop, so no background
thread ever touches game state.

## Effect catalogue

The authoritative list lives in `include/crowd/crowd_effects.h`, from which
the C# pack's effect declarations are generated.

## Naming note for contributors

The prefix for this module is `Crowd_` / `CROWD_`, **never** `CC_`. In this codebase `CC_` already
means *Crystal Challenge* (see `tools/matching/overlays/221/abi.h`).

# Crowd Control

This `ctr_native` fork can accept effect requests from the [Crowd Control](https://crowdcontrol.live/) desktop app, letting viewers spend coins on effects.<br />
Effect feedback lives entirely in Crowd Control's own overlay to keep the in-game HUD free of clutter.

Table of Contents:
- [Running](#running)
- [Loading the Pack](#loading-the-pack)
- [Redeemable Effects](#redeemable-effects)
- [Current Limitations](#current-limitations)
- [Developers & Contributors](#developers--contributors)
  - [Architecture](#architecture)
  - [Naming](#naming)
  - [Connection Info](#connection-info)
  - [Testing the Pack](#testing-the-pack)

## Running

```bash
ctr_native --crowd-control
```
***or***
```bash
ctr_native --crowd-host 127.0.0.1 --crowd-port 58430
```

Flags:
- `--crowd-control` enables the integration. 
- `--crowd-host` and `--crowd-port` override the target and imply `--crowd-control`.

The defaults are `127.0.0.1` and `58430`, matching `Host`/`Port` in [`pack/CrashTeamRacingNativePack.cs`](../pack/CrashTeamRacingNativePack.cs) (so if you change one, change the other). Without any of these flags, the integration won't do anything.

To actually receive the effects in-game, you'll also need the Crowd Control desktop app running before you launch ctr_native (see below).

## Loading the Pack

1. Install the [Crowd Control desktop app](https://crowdcontrol.live/) and the [Crowd Control SDK](https://github.com/WarpWorld/CrowdControl.SDK/releases) (to load local/unpublished packs).
2. Using the SDK, load [`pack/CrashTeamRacingNativePack.cs`](../pack/CrashTeamRacingNativePack.cs) as a local pack.
3. Launch `ctr_native --crowd-control` and start a race or adventure. The pack connects automatically once both sides are up.

## Redeemable Effects

The authoritative list of effect codes, types (instant vs timed), durations, and categories live in [`include/crowd/crowd_effects.h`](../include/crowd/crowd_effects.h).

Run `python3 tools/crowdcontrol/gen_pack.py` any time you want to confirm the header and the pack file match.<br />
This is the check that `ctest` runs as `ctr_crowd_control_pack_sync` to ensure that the diff between the two files match.

Each of the following categories listed in this doc exist to specifically call out any weirdness / edge cases / design choices that may not be clearly readable from the code. If any of these change in the future, please update this document, but don't feel the need to update this document for every minor change or addition. **Code should always be clearly-documented and the authoritative source where possible.**

Every effect is gated on **`CrowdRuntime_IsReady()`** to confirm player is actually in a race (not loading, not paused, not in a cutscene, not in a menu, not at the end-of-race screen).<br />
Requests that don't arrive at a valid time answer `Retry` (Crowd Control's standard "try again shortly" response) rather than being silently dropped.

### Race items, `crowd/crowd_fx_items.c`

- Operates on the player 0's own kart (`gGT->drivers[0]`).
- "Add" sets the held-item slot, or answers `Retry` if an item is already there.
- "Remove" clears the slot, taking whatever is currently held, or answers `Failure` if nothing is held.
- The 3x Bomb and 3x Missile variants aren't currently present. They can be added later if requested, but didn't feel really interesting to add.

### Cheats, `crowd/crowd_fx_cheats.c`

- Setting one sets the persistent `gGT->gameMode2` cheat bit, and applies the immediate grant.
- Expiry clears the bit and undoes the grant.
- The infinite-item cheats share the held-item slot with the race-item effects above, so redeeming one while one of those is already running answers `Retry`.

### Controller effects, `crowd/crowd_fx_input.c`

Masks/rewrites bits directly in player 0's gamepad buffer every frame the effect is active, so there's nothing to clean up on expiration (or in case of a crash).

- `input_reverse_steering` is applied after the disable-turn-left/right checks, so those always block the player's raw physical steering direction, not whichever direction currently steers left/right once reversed.

### Player speed, `crowd/crowd_fx_player_speed.c`

- Effects act directly on `driver->speed` in `VehPhysForce_OnApplyForces`, right before it's converted to the velocity vector.
- `player_speed_disable` forces speed to exactly zero every frame while active, vs `player_speed_max_down_50` is a ceiling of the max speed a kart can hit.

### Terrain, mutually exclusive, `crowd/crowd_fx_terrain.c`

- Remaps how the track surface handles for the duration of the effect. Only one can run at a time.
- Note that this also applies to AI/bot racers, since bots resolve terrain the same way.

### Enemy racer speed, `crowd/crowd_fx_bots.c`

- Multiple redemptions stack additively. Floored at -100% to freeze bots rather than having them reverse.
- This is technically based on nav-path progress, so bots running under `BOT_FLAG_FREE_PHYSICS` are unaffected.<br />
  *I haven't noticed this causing any weirdness in the game, but may need to adjust later if it does.*

### Character swap, `crowd/crowd_fx_character.c`

- One `character_swap_<name>` variant per selectable character, plus a random option.
- Swaps the model, stats, and audio on player 0's live kart in place.<br />
  *This appears to be the same technique used normally for character-select and ghost replay, which ends up preserving the position/speed, held item, etc.*

**Note on Oxide:**
Oxide isn't currently available because the swap handler doesn't replicate his special-cased wheel size (see the Oxide branch of `VehBirth_TireSprites`).
This could potentially be tackled with a little time, but the safer option for now was to just go with the characters that use the standard kart / wheel size.

### Adventure progress, `crowd/crowd_fx_adventure.c`

- A random add/remove pair, plus an add/remove pair per progress category.
- It was a deliberate choice for now to not have key removal be part of the random remove option.<br />
  *Will need to get feedback from CTR streamers to see if this should be adjusted.*
- If not in Adventure mode, everything here answers `Retry`.
- If supplies run out (everything in a category already unlocked, or none left to remove from the player), this answers `Retry`.

### Wumpa, `crowd/crowd_fx_wumpa.c`

- Wumpa removal must manually set `driver->numWumpas`. Using `RB_Player_ModifyWumpa` (retail method) results in an active mask item blocking Wumpa removal, so only adding Wumpa uses that.

## Current Limitations

**Single-player only.** Every effect targets `gGT->drivers[0]` / player 0. There's no reason why this couldn't be expanded later if there's demand for it, just felt out of scope for the time being.


## Developers & Contributors

### Architecture

```
include/crowd/crowd_effects.h   Single source of truth: code, kind, default duration, category, handler
crowd/crowd_runtime.c           Registry + dispatch + timed-effect scheduler
crowd/crowd_protocol.c          Request decode / response encode
crowd/crowd_json.c              Flat-JSON parser/writer
crowd/crowd_net.c               Networking code: non-blocking TCP, NUL-framed, reconnect backoff
crowd/crowd_fx_*.c              One file per effect category, called by crowd_runtime.c
```

`crowd_runtime.c` includes `crowd_effects.h` twice, allowing the effect list to expand into two different things:
- once into the forward declarations for every `Crowd_Fx_<Handler>_Start`/`_Stop` pair.
- once into the dispatch table that wires them up.

Adding a new entry to `crowd_effects.h` is enough to register an effect with the new runtime. The handler functions still need to be written in the matching `crowd_fx_*.c` file, however.


### Naming
The prefix for this module is `Crowd_` / `CROWD_`, **never** `CC_`.<br />
`CC_` in the code already refers to "Crystal Challenge" (see `tools/matching/overlays/221/abi.h`).

### Connection Info

Crowd Control's SimpleTCP connector runs in *server mode*: the desktop app listens and the game connects to it as a client.
Messages are NUL-terminated (`0x00`) UTF-8 JSON in both directions:

- Requests: `{id, type, code, quantity?, duration?, viewer?, parameters?}`
- Responses: `{id, type, status, timeRemaining?, message?}`

The `RequestType`, `ResponseType`, and `EffectStatus` enum values are defined in [`include/crowd/crowd_protocol.h`](../include/crowd/crowd_protocol.h).

Every request needs a response within 5 seconds. Note that `duration` is in **seconds**, and `crowd_runtime.c` converts it to milliseconds to match the millisecond durations in `include/crowd/crowd_effects.h`.

### Testing the Pack
For development, it's usually faster to use [`tools/crowdcontrol/fake_cc_server.py`](../tools/crowdcontrol/fake_cc_server.py) instead of the SDK. It allows you to see the JSON data being sent/received, which can be helpful for troubleshooting.

```python
python3 tools/crowdcontrol/fake_cc_server.py
ctr_native --crowd-control
```

```
> start item_add_boost
>> {"id": "1", "type": 1, "code": "item_add_boost"}
<< {"id": "1", "type": 1, "status": 0}
> start cheat_icy_tracks 10
>> {"id": "2", "type": 1, "code": "cheat_icy_tracks", "duration": 10.0}
<< {"id": "2", "type": 1, "status": 5, "timeRemaining": 10000}
...
<< {"id": "2", "type": 1, "status": 8}
```

If needed, the build can also exclude the Crowd Control integration:

```bash
cmake --preset linux-gcc-i686-release -DCTR_CROWD_CONTROL=OFF
```
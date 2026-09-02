/*
 * Single source of truth for Crowd Control effect codes.
 *
 * CROWD_EFFECT(code, kind, durationMs, category, handler)
 *   code        - string sent in a request's "code" field.
 *   kind        - CROWD_EFFECT_INSTANT or CROWD_EFFECT_TIMED.
 *   durationMs  - default duration for CROWD_EFFECT_TIMED effects; write 0 for instant effects
 *                 (a request's own "duration" field, when present, overrides this).
 *   category    - a CrowdEffectCategory, purely descriptive (C# pack folder grouping).
 *   handler     - bare identifier used to build the Crowd_Fx_<handler>_Start / _Stop function
 *                 names crowd_runtime.c dispatches through. Instant effects still need a _Stop 
 *                 function to exist for the table to build, even though it's never called.
 */

#if defined(CTR_NATIVE) && defined(CTR_INTERNAL)
/* Just for runtime testing. Not part of the C# pack. */
CROWD_EFFECT("debug_test_timed", CROWD_EFFECT_TIMED, 5000, CROWD_EFFECT_CATEGORY_DEBUG, DebugTest)
#endif

/* Race items (instant). "Random" options picks across the 12 HELD_ITEM_* values. See crowd/crowd_fx_items.c. */
CROWD_EFFECT("item_add_random", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemAddRandom)
CROWD_EFFECT("item_add_boost", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemAddBoost)
CROWD_EFFECT("item_add_bomb", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemAddBomb)
CROWD_EFFECT("item_add_missile", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemAddMissile)
CROWD_EFFECT("item_add_tnt", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemAddTnt)
CROWD_EFFECT("item_add_potion", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemAddPotion)
CROWD_EFFECT("item_add_spring", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemAddSpring)
CROWD_EFFECT("item_add_shield", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemAddShield)
CROWD_EFFECT("item_add_mask", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemAddMask)
CROWD_EFFECT("item_add_clock", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemAddClock)
CROWD_EFFECT("item_add_warp", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemAddWarp)
CROWD_EFFECT("item_add_invisibility", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemAddInvisibility)
CROWD_EFFECT("item_add_super_turbo", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemAddSuperTurbo)
CROWD_EFFECT("item_remove_random", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemRemoveRandom)
CROWD_EFFECT("item_remove_boost", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemRemoveBoost)
CROWD_EFFECT("item_remove_bomb", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemRemoveBomb)
CROWD_EFFECT("item_remove_missile", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemRemoveMissile)
CROWD_EFFECT("item_remove_tnt", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemRemoveTnt)
CROWD_EFFECT("item_remove_potion", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemRemovePotion)
CROWD_EFFECT("item_remove_spring", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemRemoveSpring)
CROWD_EFFECT("item_remove_shield", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemRemoveShield)
CROWD_EFFECT("item_remove_mask", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemRemoveMask)
CROWD_EFFECT("item_remove_clock", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemRemoveClock)
CROWD_EFFECT("item_remove_warp", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemRemoveWarp)
CROWD_EFFECT("item_remove_invisibility", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemRemoveInvisibility)
CROWD_EFFECT("item_remove_super_turbo", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_ITEM, ItemRemoveSuperTurbo)

/* Cheat-flag timed effects (all 30s). See crowd/crowd_fx_cheats.c. */
CROWD_EFFECT("cheat_infinite_wumpa", CROWD_EFFECT_TIMED, 30000, CROWD_EFFECT_CATEGORY_CHEAT, CheatInfiniteWumpa)
CROWD_EFFECT("cheat_infinite_masks", CROWD_EFFECT_TIMED, 30000, CROWD_EFFECT_CATEGORY_CHEAT, CheatInfiniteMasks)
CROWD_EFFECT("cheat_infinite_turbo", CROWD_EFFECT_TIMED, 30000, CROWD_EFFECT_CATEGORY_CHEAT, CheatInfiniteTurbo)
CROWD_EFFECT("cheat_infinite_invisibility", CROWD_EFFECT_TIMED, 30000, CROWD_EFFECT_CATEGORY_CHEAT, CheatInfiniteInvisibility)
CROWD_EFFECT("cheat_infinite_bombs", CROWD_EFFECT_TIMED, 30000, CROWD_EFFECT_CATEGORY_CHEAT, CheatInfiniteBombs)
CROWD_EFFECT("cheat_icy_tracks", CROWD_EFFECT_TIMED, 30000, CROWD_EFFECT_CATEGORY_CHEAT, CheatIcyTracks)
CROWD_EFFECT("cheat_super_turbo_pads", CROWD_EFFECT_TIMED, 30000, CROWD_EFFECT_CATEGORY_CHEAT, CheatSuperTurboPads)

/* Input denial (all 10s). Masks player 0's GamepadBuffer. See crowd/crowd_fx_input.c. */
CROWD_EFFECT("input_disable_engine", CROWD_EFFECT_TIMED, 10000, CROWD_EFFECT_CATEGORY_INPUT, InputDisableEngine)
CROWD_EFFECT("input_disable_steering", CROWD_EFFECT_TIMED, 10000, CROWD_EFFECT_CATEGORY_INPUT, InputDisableSteering)
CROWD_EFFECT("input_disable_jumping", CROWD_EFFECT_TIMED, 10000, CROWD_EFFECT_CATEGORY_INPUT, InputDisableJumping)

/* Terrain remaps (both 30s, mutually exclusive). See crowd/crowd_fx_terrain.c. */
CROWD_EFFECT("terrain_no_rough", CROWD_EFFECT_TIMED, 30000, CROWD_EFFECT_CATEGORY_TERRAIN, TerrainNoRough)
CROWD_EFFECT("terrain_all_dirt", CROWD_EFFECT_TIMED, 30000, CROWD_EFFECT_CATEGORY_TERRAIN, TerrainAllDirt)

/* Enemy racer speed. Bots running under BOT_FLAG_FREE_PHYSICS are unaffected. See crowd/crowd_fx_bots.c. */
CROWD_EFFECT("bots_speed_down_50", CROWD_EFFECT_TIMED, 20000, CROWD_EFFECT_CATEGORY_BOTS, BotsSpeedDown50)
CROWD_EFFECT("bots_speed_up_50", CROWD_EFFECT_TIMED, 20000, CROWD_EFFECT_CATEGORY_BOTS, BotsSpeedUp50)
CROWD_EFFECT("bots_speed_down_100", CROWD_EFFECT_TIMED, 10000, CROWD_EFFECT_CATEGORY_BOTS, BotsSpeedDown100)
CROWD_EFFECT("bots_speed_up_100", CROWD_EFFECT_TIMED, 10000, CROWD_EFFECT_CATEGORY_BOTS, BotsSpeedUp100)

/* Swap character (instant). Refused when the target's model pack was never streamed for this race.
Note that Oxide isn't currently available. See crowd/crowd_fx_character.c. */
CROWD_EFFECT("character_swap_random", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapRandom)
CROWD_EFFECT("character_swap_crash", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapCrash)
CROWD_EFFECT("character_swap_cortex", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapCortex)
CROWD_EFFECT("character_swap_tiny", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapTiny)
CROWD_EFFECT("character_swap_coco", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapCoco)
CROWD_EFFECT("character_swap_ngin", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapNgin)
CROWD_EFFECT("character_swap_dingodile", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapDingodile)
CROWD_EFFECT("character_swap_polar", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapPolar)
CROWD_EFFECT("character_swap_pura", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapPura)
CROWD_EFFECT("character_swap_pinstripe", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapPinstripe)
CROWD_EFFECT("character_swap_papu", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapPapu)
CROWD_EFFECT("character_swap_ripper_roo", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapRipperRoo)
CROWD_EFFECT("character_swap_komodo", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapKomodo)
CROWD_EFFECT("character_swap_tropy", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapTropy)
CROWD_EFFECT("character_swap_penta", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapPenta)
CROWD_EFFECT("character_swap_fake_crash", CROWD_EFFECT_INSTANT, 0, CROWD_EFFECT_CATEGORY_CHARACTER, CharacterSwapFakeCrash)

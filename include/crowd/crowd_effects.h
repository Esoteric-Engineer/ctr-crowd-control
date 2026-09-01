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

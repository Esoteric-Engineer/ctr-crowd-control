// Crowd Control effect pack.
// 
// Loaded by the Crowd Control desktop app, not by this repo's own build.
//
// Effect codes, kind, and durations must match include/crowd/crowd_effects.h. After editing either file, run `tools/crowdcontrol/gen_pack.py --check`.
// 
// TODO(crowd): Prices will be adjusted later. Just testing to make sure they work for now.

using ConnectorLib.SimpleTCP;
using CrowdControl.Common;
using JetBrains.Annotations;
using ConnectorType = CrowdControl.Common.ConnectorType;

namespace CrowdControl.Games.Packs.CrashTeamRacingNative;

[UsedImplicitly]
public class CrashTeamRacingNative : SimpleTCPPack<SimpleTCPServerConnector>
{
    public override string Host => "127.0.0.1";

    public override ushort Port => 58430;

    public override ISimpleTCPPack.MessageFormatType MessageFormat => ISimpleTCPPack.MessageFormatType.CrowdControlLegacy;

    public CrashTeamRacingNative(UserRecord player, Func<CrowdControlBlock, bool> responseHandler, Action<object> statusUpdateHandler) : base(player, responseHandler, statusUpdateHandler) { } // don't convert this to a primary constructor; CrowdControl SDK gets angry

    // Numeric ID and slug are placeholders until Crowd Control assigns permanent ones on submission.
    public override Game Game { get; } = new("Crash Team Racing (Native)", "CrashTeamRacingNative", "PC", ConnectorType.SimpleTCPServerConnector);

    public override EffectList Effects { get; } = new Effect[]
    {
        // Race items: "Add" fails if a held item is already present; "Remove" fails if it doesn't match.
        new("Give Random Item", "item_add_random") { Category = "Give Item", Price = 10, Description = "Give the player a random held item. Fails if they're already holding one." },
        new("Give Turbo Boost", "item_add_boost") { Category = "Give Item", Price = 10, Description = "Give the player a Turbo Boost. Fails if they already have an item." },
        new("Give Bomb", "item_add_bomb") { Category = "Give Item", Price = 10, Description = "Give the player a Bowling Bomb. Fails if they already have an item." },
        new("Give Missile", "item_add_missile") { Category = "Give Item", Price = 10, Description = "Give the player a Tracking Missile. Fails if they already have an item." },
        new("Give TNT", "item_add_tnt") { Category = "Give Item", Price = 10, Description = "Give the player a TNT Crate. Fails if they already have an item." },
        new("Give Beaker", "item_add_potion") { Category = "Give Item", Price = 10, Description = "Give the player an N. Brio Beaker (potion). Fails if they already have an item." },
        new("Give Spring", "item_add_spring") { Category = "Give Item", Price = 10, Description = "Give the player a Spring. Fails if they already have an item." },
        new("Give Shield", "item_add_shield") { Category = "Give Item", Price = 10, Description = "Give the player a Power Shield. Fails if they already have an item." },
        new("Give Mask", "item_add_mask") { Category = "Give Item", Price = 10, Description = "Give the player an Aku Aku/Uka Uka Mask. Fails if they already have an item." },
        new("Give Clock", "item_add_clock") { Category = "Give Item", Price = 10, Description = "Give the player an N. Tropy Clock. Fails if they already have an item." },
        new("Give Warp Orb", "item_add_warp") { Category = "Give Item", Price = 10, Description = "Give the player a Warp Orb. Fails if they already have an item." },
        new("Give Invisibility", "item_add_invisibility") { Category = "Give Item", Price = 10, Description = "Give the player an Invisibility item. Fails if they already have an item." },
        new("Give Super Engine", "item_add_super_turbo") { Category = "Give Item", Price = 10, Description = "Give the player a Super Engine. Fails if they already have an item." },
        new("Remove Random Item", "item_remove_random") { Category = "Remove Item", Price = 10, Description = "Take away whatever item the player is currently holding." },
        new("Remove Turbo Boost", "item_remove_boost") { Category = "Remove Item", Price = 10, Description = "Take away the player's Turbo Boost. Fails if that isn't what they're holding." },
        new("Remove Bomb", "item_remove_bomb") { Category = "Remove Item", Price = 10, Description = "Take away the player's Bowling Bomb. Fails if that isn't what they're holding." },
        new("Remove Missile", "item_remove_missile") { Category = "Remove Item", Price = 10, Description = "Take away the player's Tracking Missile. Fails if that isn't what they're holding." },
        new("Remove TNT", "item_remove_tnt") { Category = "Remove Item", Price = 10, Description = "Take away the player's TNT crate. Fails if that isn't what they're holding." },
        new("Remove Beaker", "item_remove_potion") { Category = "Remove Item", Price = 10, Description = "Take away the player's N. Brio Beaker (potion). Fails if that isn't what they're holding." },
        new("Remove Spring", "item_remove_spring") { Category = "Remove Item", Price = 10, Description = "Take away the player's Spring. Fails if that isn't what they're holding." },
        new("Remove Shield", "item_remove_shield") { Category = "Remove Item", Price = 10, Description = "Take away the player's Power Shield. Fails if that isn't what they're holding." },
        new("Remove Mask", "item_remove_mask") { Category = "Remove Item", Price = 10, Description = "Take away the player's Aku Aku/Uka Uka Mask. Fails if that isn't what they're holding." },
        new("Remove Clock", "item_remove_clock") { Category = "Remove Item", Price = 10, Description = "Take away the player's N. Tropy Clock. Fails if that isn't what they're holding." },
        new("Remove Warp Orb", "item_remove_warp") { Category = "Remove Item", Price = 10, Description = "Take away the player's Warp Orb. Fails if that isn't what they're holding." },
        new("Remove Invisibility", "item_remove_invisibility") { Category = "Remove Item", Price = 10, Description = "Take away the player's Invisibility item. Fails if that isn't what they're holding." },
        new("Remove Super Engine", "item_remove_super_turbo") { Category = "Remove Item", Price = 10, Description = "Take away the player's Super Engine. Fails if that isn't what they're holding." },

        // Cheats
        new("Infinite Wumpa", "cheat_infinite_wumpa") { Category = "Cheats", Duration = 30, Price = 40, Description = "The player's Wumpa fruit is maxed out for 30 seconds." },
        new("Infinite Masks", "cheat_infinite_masks") { Category = "Cheats", Duration = 30, Price = 40, Description = "The player has infinite Aku Aku/Uka Uka masks for 30 seconds." },
        new("Infinite Turbo", "cheat_infinite_turbo") { Category = "Cheats", Duration = 30, Price = 40, Description = "The player has infinite Turbo Boost items for 30 seconds." },
        new("Infinite Invisibility", "cheat_infinite_invisibility") { Category = "Cheats", Duration = 30, Price = 40, Description = "The player stays invisible for 30 seconds." },
        new("Infinite Bombs", "cheat_infinite_bombs") { Category = "Cheats", Duration = 30, Price = 40, Description = "The player has infinite Bowling Bombs for 30 seconds." },
        new("Icy Tracks", "cheat_icy_tracks") { Category = "Cheats", Duration = 30, Price = 40, Description = "Every terrain is ice for 30 seconds." },
        new("Super Turbo Pads", "cheat_super_turbo_pads") { Category = "Cheats", Duration = 30, Price = 40, Description = "Every turbo pad gives the maximum speed boost for 30 seconds." },

        // Input
        new("Disable Engine", "input_disable_engine") { Category = "Input Denial", Duration = 10, Price = 20, Description = "The player can't accelerate for 10 seconds." },
        new("Disable Steering", "input_disable_steering") { Category = "Input Denial", Duration = 10, Price = 20, Description = "The player can't steer left or right for 10 seconds." },
        new("Disable Jumping", "input_disable_jumping") { Category = "Input Denial", Duration = 10, Price = 20, Description = "The player can't hop or power-slide for 10 seconds." },

        // Terrain: all mutually exclusive with each other
        new("No Rough Terrain", "terrain_no_rough") { Category = "Terrain", Duration = 30, Price = 25, Description = "Dirt, grass, and mud all handle like asphalt for 30 seconds." },
        new("Rough Terrain", "terrain_all_dirt") { Category = "Terrain", Duration = 30, Price = 25, Description = "Every surface handles like dirt for 30 seconds." },

        // Enemy racer speed
        new("Slow Enemies 50%", "bots_speed_down_50") { Category = "Enemy Racers", Duration = 20, Price = 20, Description = "Every AI racer moves 50% slower for 20 seconds." },
        new("Speed Up Enemies 50%", "bots_speed_up_50") { Category = "Enemy Racers", Duration = 20, Price = 25, Description = "Every AI racer moves 50% faster for 20 seconds." },
        new("Freeze Enemies", "bots_speed_down_100") { Category = "Enemy Racers", Duration = 10, Price = 35, Description = "Every AI racer is frozen for 10 seconds." },
        new("Speed Up Enemies 100%", "bots_speed_up_100") { Category = "Enemy Racers", Duration = 10, Price = 40, Description = "Every AI racer moves twice as fast for 10 seconds." },

        // Character swap
        new("Random Character", "character_swap_random") { Category = "Character Swap", Price = 10, Description = "Swap the player to a random character." },
        new("Become Crash", "character_swap_crash") { Category = "Character Swap", Price = 15, Description = "Swap the player to Crash Bandicoot." },
        new("Become Cortex", "character_swap_cortex") { Category = "Character Swap", Price = 15, Description = "Swap the player to Dr. Neo Cortex." },
        new("Become Tiny", "character_swap_tiny") { Category = "Character Swap", Price = 15, Description = "Swap the player to Tiny Tiger." },
        new("Become Coco", "character_swap_coco") { Category = "Character Swap", Price = 15, Description = "Swap the player to Coco Bandicoot." },
        new("Become N. Gin", "character_swap_ngin") { Category = "Character Swap", Price = 15, Description = "Swap the player to Dr. N. Gin." },
        new("Become Dingodile", "character_swap_dingodile") { Category = "Character Swap", Price = 15, Description = "Swap the player to Dingodile." },
        new("Become Polar", "character_swap_polar") { Category = "Character Swap", Price = 15, Description = "Swap the player to Polar." },
        new("Become Pura", "character_swap_pura") { Category = "Character Swap", Price = 15, Description = "Swap the player to Pura." },
        new("Become Pinstripe", "character_swap_pinstripe") { Category = "Character Swap", Price = 15, Description = "Swap the player to Pinstripe Potoroo." },
        new("Become Papu Papu", "character_swap_papu") { Category = "Character Swap", Price = 15, Description = "Swap the player to Papu Papu." },
        new("Become Ripper Roo", "character_swap_ripper_roo") { Category = "Character Swap", Price = 15, Description = "Swap the player to Ripper Roo." },
        new("Become Komodo Joe", "character_swap_komodo") { Category = "Character Swap", Price = 15, Description = "Swap the player to Komodo Joe." },
        new("Become N. Tropy", "character_swap_tropy") { Category = "Character Swap", Price = 15, Description = "Swap the player to Dr. N. Tropy." },
        new("Become Penta Penguin", "character_swap_penta") { Category = "Character Swap", Price = 15, Description = "Swap the player to Penta Penguin." },
        new("Become Fake Crash", "character_swap_fake_crash") { Category = "Character Swap", Price = 15, Description = "Swap the player to Fake Crash." },

        // Adventure progress
        new("Give Random Adventure Item", "adv_add_random") { Category = "Add Adventure Progress", Price = 20, Description = "Unlock a random Trophy, Relic, Crystal, or Boss Key. Requires an active Adventure mode race." },
        new("Give Random Trophy", "adv_add_trophy") { Category = "Add Adventure Progress", Price = 30, Description = "Unlock a random track's Trophy. Requires an active Adventure mode race." },
        new("Give Random Sapphire Relic", "adv_add_sapphire_relic") { Category = "Add Adventure Progress", Price = 30, Description = "Unlock a random track's Sapphire relic. Requires an active Adventure mode race." },
        new("Give Random Gold Relic", "adv_add_gold_relic") { Category = "Add Adventure Progress", Price = 30, Description = "Unlock a random track's Gold relic. Requires an active Adventure mode race." },
        new("Give Random Platinum Relic", "adv_add_platinum_relic") { Category = "Add Adventure Progress", Price = 30, Description = "Unlock a random track's Platinum relic. Requires an active Adventure mode race." },
        new("Give Random Crystal", "adv_add_crystal") { Category = "Add Adventure Progress", Price = 30, Description = "Unlock a random hub's Purple CTR Token. Requires an active Adventure mode race." },
        new("Give Random Key", "adv_add_key") { Category = "Add Adventure Progress", Price = 30, Description = "Unlock a random hub's Boss Key. Requires an active Adventure mode race." },
        new("Remove Random Adventure Item", "adv_remove_random") { Category = "Remove Adventure Progress", Price = 15, Description = "Revoke a random already-unlocked Trophy, Relic, or Crystal (never a Key). Requires an active Adventure mode race." },
        new("Remove Random Trophy", "adv_remove_trophy") { Category = "Remove Adventure Progress", Price = 25, Description = "Revoke a random already-unlocked Trophy. Requires an active Adventure mode race." },
        new("Remove Random Sapphire Relic", "adv_remove_sapphire_relic") { Category = "Remove Adventure Progress", Price = 25, Description = "Revoke a random already-unlocked Sapphire relic. Requires an active Adventure mode race." },
        new("Remove Random Gold Relic", "adv_remove_gold_relic") { Category = "Remove Adventure Progress", Price = 25, Description = "Revoke a random already-unlocked Gold relic. Requires an active Adventure mode race." },
        new("Remove Random Platinum Relic", "adv_remove_platinum_relic") { Category = "Remove Adventure Progress", Price = 25, Description = "Revoke a random already-unlocked Platinum relic. Requires an active Adventure mode race." },
        new("Remove Random Crystal", "adv_remove_crystal") { Category = "Remove Adventure Progress", Price = 25, Description = "Revoke a random already-unlocked Purple CTR Token. Requires an active Adventure mode race." },
        new("Remove Random Key", "adv_remove_key") { Category = "Remove Adventure Progress", Price = 25, Description = "Revoke a random already-unlocked Boss Key. Requires an active Adventure mode race." },
    };
}
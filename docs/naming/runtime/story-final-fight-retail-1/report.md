# Runtime trace evidence: capture.jsonl

- recorded 2026-10-25 22:56, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace\build\bb2.map`, 951 events in 2722.68 s
- manifest `capture.manifest.json` (SHA1 16306b52e3832b7d10ad915cc85ddaf6a78d5e24), capture SHA1 fbff35093e8e2adb1ddb22e8a70d4e2f481a35f0
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- phases covered: boot_intro (frames 0-8900), story (frames 8900-126600), katze (frames 126600-127900), strat (frames 127900-129300), after_katze (frames 129300-153700), final_fight (frames 153700-158000); script reached its end

## fighter_Init: 166 hits

Hits per phase: boot_intro 4, story 118, katze 1, strat 2, after_katze 33, final_fight 8

Call sites (ra-8):
- `fight_Init+0xE0` x118
- `func_8001C51C+0x78` x27
- `func_8003AF40+0x68` x21

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 2 | 0x00000001, 0x00000000 |
| a1 | 2 | 0x00000001, 0x00000000 |
| a2 | 12 | 0x0000001D, 0x00000000, 0x0000001B, 0x00000010 |
| a3 | 6 | 0x00000000, 0x00000005, 0x00000002, 0x00000001 |

## func_80032854: 773 hits

Hits per phase: story 300, katze 28, strat 25, after_katze 300, final_fight 120
At most 300 hit(s) recorded per phase (per_phase): counts and the tables below are capped.

Call sites (ra-8):
- `object_UpdatePool+0x824` x110
- `fighter_DispatchBoneHitSoundEvent+0xB4` x76
- `func_80026DA4+0x51C` x72
- `fighter_ResolveBoneHit+0x2CC` x57
- `fighter_ResolveWeaponContact+0x15C` x52
- `fighter_ResolveWeaponContact+0x170` x52
- `fighter_ResolveBoneHit+0x364` x48
- `func_800204C0+0x1B4` x39
- `fighter_ResolveWeaponContact+0x258` x36
- `fighter_ResolveWeaponContact+0x26C` x36

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 2 | 0x00000001, 0x00000000 |
| a1 | 20 | 0x00000025, 0x00000029, 0x00000026, 0x0000002D |
| a2 | 40 | 0x1F8003F4, 0x80106AA4 `g_object_pool+0x2C`, 0x1F8001BC, 0x801FFE78 |
| a3 | 3 | 0x00000000, 0x801FFC60, 0x1F8003B0 |

## fighter_ResolveBoneHit+0x6B0: 12 hits

Hits per phase: final_fight 12

Call sites (ra-8):
- `fighter_ProcessWeaponBodyHits+0x1108` x12

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00000001 |
| a1 | 1 | 0x00000012 |
| a2 | 8 | 0x1F800234, 0x1F8001D4, 0x1F800228, 0x1F80021C |
| a3 | 11 | 0x000007FA, 0x000007F3, 0x000007F5, 0x00000744 |


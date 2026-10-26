# Runtime trace evidence: capture.jsonl

- recorded 2026-10-26 06:12, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace\build\bb2.map`, 16 events in 208.625 s
- scenario: Versus 2P on Kimigaura Coast, lib-versus-2p-kimigaura-effects-1 script cut at 11300 (two code-3 rose-light starts, 9885 and 11144); eff_StartRecycledCommandWithRoseLight 0x80061280 poked to b 0x800612DC / addiu a0, v1, 1 at t_load so every start takes the second slot (status 10, command 0xA)
- manifest `capture.manifest.json` (SHA1 1ce484fa2baf71f36168666981b1e9c021a2b7d2), capture SHA1 feaa8107bd1e396bf1570f1f1f4bb05fd40d1105
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x80061280` (0x80061280) set to 1600001001006424 (was 0b00401000000000) when t_load started (frame 7450): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6260), t_char (frames 6260-6550), char_select (frames 6550-6700), weapon_select (frames 6700-7000), t_stage (frames 7000-7175), stage_select (frames 7175-7450), t_load (frames 7450-8230), fight (frames 8230-11300); script reached its end

## eff_StartRecycledCommandWithRoseLight: 2 hits

Hits per phase: fight 2

Call sites (ra-8):
- `func_80032854+0x180` x2

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 2 | 0x1F800144, 0x1F8000FC |
| a1 | 1 | 0x00000002 |
| a2 | 2 | 0x1F800144, 0x1F8000FC |
| a3 | 1 | 0x00000000 |

## func_80064FB4: 2 hits

Hits per phase: fight 2

Call sites (ra-8):
- `eff_StartCommand+0xBC` x2

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x0000000A |
| a1 | 1 | 0x00000000 |
| a2 | 2 | 0x1F800144, 0x1F8000FC |
| a3 | 1 | 0x00000000 |

## func_80065434: 12 hits

Hits per phase: fight 12

Call sites (ra-8):
- `eff_StepCommand+0xCC` x12

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x1F800030 |
| a1 | 1 | 0x8009BB74 `D_8009BB74` |
| a2 | 1 | 0x1F800030 |
| a3 | 5 | 0x8010EA80, 0x801130D0, 0x8010EAD0, 0x80113120 |

## Armed, never hit in this scenario

- `func_80064F68`
- `func_800653E4`


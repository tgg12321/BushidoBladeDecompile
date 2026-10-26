# Runtime trace evidence: capture.jsonl

- recorded 2026-10-26 02:01, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace\build\bb2.map`, 48 events in 194.377 s
- scenario: versus-2p-kimigaura-kick-2p (spec only): versus-2p-kimigaura-shore-kick's boot..fight; walks 0-3 both pads DOWN 60 then R2+CROSS on the same frames; walks 4-7 P1 only, R2+CROSS at +80 and +180; retail, no pokes or stubs
- manifest `capture.manifest.json` (SHA1 bd6014c81e69daf6e30e7be29be513cbae239f72), capture SHA1 1e9765aab3ec63afeba544624e28ae4283fe56f1
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6260), t_char (frames 6260-6550), char_select (frames 6550-6700), weapon_select (frames 6700-7000), t_stage (frames 7000-7175), stage_select (frames 7175-7450), t_load (frames 7450-8230), fight (frames 8230-8430), walk_0 (frames 8430-8510), kick2p_0 (frames 8510-8630), walk_1 (frames 8630-8710), kick2p_1 (frames 8710-8830), walk_2 (frames 8830-8910), kick2p_2 (frames 8910-9030), walk_3 (frames 9030-9110), kick2p_3 (frames 9110-9230), walk_4 (frames 9230-9310), kick_4 (frames 9310-9490), walk_5 (frames 9490-9570), kick_5 (frames 9570-9750), walk_6 (frames 9750-9830), kick_6 (frames 9830-10010), walk_7 (frames 10010-10090), kick_7 (frames 10090-10330); script reached its end

## func_80032064: 16 hits

Hits per phase: kick2p_0 2, kick2p_1 2, kick2p_2 2, kick2p_3 2, kick_4 2, kick_5 2, kick_6 2, kick_7 2

Call sites (ra-8):
- `fighter_Update+0x2D30` x16

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 2 | 0x80101EC8 `D_80101EC8`, 0x80102314 `D_801021E2+0x132` |
| a1 | 2 | 0x00000002, 0x00000001 |
| a2 | 1 | 0x00000002 |
| a3 | 1 | 0x00000001 |

## func_80061C00: 6 hits

Hits per phase: kick2p_0 2, kick2p_1 2, kick2p_2 2

Call sites (ra-8):
- `func_80032854+0x2D8` x6

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 2 | 0x80104E8C `D_80104E88+0x4`, 0x80104EB8 `D_80104E88+0x30` |
| a1 | 5 | 0x00000001, 0x00000000, 0x000007FC, 0x000007FD |
| a2 | 2 | 0x00000066, 0x00000075 |
| a3 | 1 | 0x00000000 |

## func_80067060: 3 hits

Hits per phase: kick2p_0 1, kick2p_1 1, kick2p_2 1

Call sites (ra-8):
- `eff_StartCommand+0xBC` x3

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00000016 |
| a1 | 1 | 0x00000A00 |
| a2 | 1 | 0x801FFC10 |
| a3 | 1 | 0x00000000 |

## func_800670C8: 3 hits

Hits per phase: kick2p_0 1, kick2p_1 1, kick2p_2 1

Call sites (ra-8):
- `eff_StartCommand+0xBC` x3

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00000017 |
| a1 | 1 | 0x0000F600 |
| a2 | 1 | 0x801FFC10 |
| a3 | 1 | 0x00000000 |

## func_80061D74: 10 hits

Hits per phase: kick2p_3 2, kick_4 2, kick_5 2, kick_6 2, kick_7 2

Call sites (ra-8):
- `func_80032854+0x2F8` x10

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 2 | 0x80104E8C `D_80104E88+0x4`, 0x80104EB8 `D_80104E88+0x30` |
| a1 | 6 | 0x00000770, 0x000006CC, 0x0000062F, 0x0000059F |
| a2 | 7 | 0x00000074, 0x00000071, 0x00000070, 0x00000066 |
| a3 | 1 | 0x00000000 |

## func_80067130: 9 hits

Hits per phase: kick2p_3 1, kick_4 2, kick_5 2, kick_6 2, kick_7 2

Call sites (ra-8):
- `eff_StartCommand+0xBC` x9

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00000018 |
| a1 | 5 | 0x0000F63E, 0x0000F718, 0x0000F870, 0x0000FA0E |
| a2 | 1 | 0x801FFC10 |
| a3 | 1 | 0x00000000 |

## func_80067198: 1 hits

Hits per phase: kick2p_3 1

Call sites (ra-8):
- `eff_StartCommand+0xBC` x1

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00000019 |
| a1 | 1 | 0x0000F600 |
| a2 | 1 | 0x801FFC10 |
| a3 | 1 | 0x00000000 |


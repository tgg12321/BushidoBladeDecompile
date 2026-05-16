# Data Symbol Naming Quick Wins

Scan of `undefined_syms_auto.txt` (1735 D_* labels) for cases where the access pattern in code gives away the underlying type or purpose. These are PROPOSALS, not renames. Apply with care.


Total D_* symbols analyzed: 1505; with usage patterns: 1467

## GTE matrix/vector candidates (direct lwc2/swc2 references) (0)

(none)

> NOTE: GTE access usually goes through a pointer-loaded base address rather than a direct D_<addr> reference. So this section is sparse; most GTE matrices live behind indirect pointer chains and require body inspection to identify.

## Function pointer / callback candidates (12)

| current | address | proposed | usage |
|---|---|---|---|
| `D_80015470` | `0x80015470` | `fp_80015470` | {'lui': 1, 'lw': 1, 'fp_call': 1} |
| `D_8008D090` | `0x8008D090` | `fp_8008D090` | {'lui': 1, 'lw': 1, 'fp_call': 1} |
| `D_8009BC1C` | `0x8009BC1C` | `fp_8009BC1C` | {'lui': 1, 'lw': 1, 'fp_call': 1} |
| `D_800A26D4` | `0x800A26D4` | `fp_800A26D4` | {'lui': 4, 'lw': 4, 'fp_call': 2} |
| `D_800A2D14` | `0x800A2D14` | `fp_800A2D14` | {'lui': 10, 'lw': 6, 'fp_call': 1, 'sw': 4} |
| `D_800A362C` | `0x800A362C` | `fp_800A362C` | {'lui': 3, 'lw': 2, 'fp_call': 1, 'sw': 1} |
| `D_800F3340` | `0x800F3340` | `fp_800F3340` | {'lui': 2, 'lw': 2, 'fp_call': 1} |
| `D_800F3348` | `0x800F3348` | `fp_800F3348` | {'lui': 1, 'lw': 1, 'fp_call': 1} |
| `D_800F66A0` | `0x800F66A0` | `fp_800F66A0` | {'lui': 11, 'addiu': 2, 'lw': 8, 'fp_call': 8, 'sw': 1} |
| `D_801027E8` | `0x801027E8` | `fp_801027E8` | {'lui': 1, 'lw': 1, 'fp_call': 1} |
| `D_80102BF8` | `0x80102BF8` | `fp_80102BF8` | {'lui': 1, 'lw': 1, 'fp_call': 1} |
| `D_80103680` | `0x80103680` | `fp_80103680` | {'lui': 4, 'sw': 1, 'lw': 2, 'fp_call': 1, 'addiu': 1} |

> NOTE: Same caveat as above -- function-pointer tables are typically loaded into a register first and then `jalr`'d, so the direct-reference pattern misses most of them.

## Word-table candidates (lw/sw only access) (267)

| current | address | proposed | usage |
|---|---|---|---|
| `D_80094B88` | `0x80094B88` | `wtbl_80094B88` | {'lui': 5, 'lw': 2, 'sw': 2, 'addiu': 1} |
| `D_8009BD44` | `0x8009BD44` | `wtbl_8009BD44` | {'lui': 12, 'lw': 12} |
| `D_8009BD6C` | `0x8009BD6C` | `wtbl_8009BD6C` | {'lui': 4, 'lw': 4} |
| `D_8009BD80` | `0x8009BD80` | `wtbl_8009BD80` | {'lui': 5, 'sw': 4, 'lw': 1} |
| `D_8009BF60` | `0x8009BF60` | `wtbl_8009BF60` | {'lui': 4, 'lw': 4} |
| `D_8009BF64` | `0x8009BF64` | `wtbl_8009BF64` | {'lui': 4, 'lw': 4} |
| `D_800A11B4` | `0x800A11B4` | `wtbl_800A11B4` | {'lui': 20, 'lw': 8, 'sw': 12} |
| `D_800A11B8` | `0x800A11B8` | `wtbl_800A11B8` | {'lui': 8, 'lw': 5, 'sw': 3} |
| `D_800A11DC` | `0x800A11DC` | `wtbl_800A11DC` | {'lui': 8, 'lw': 7, 'addiu': 1} |
| `D_800A147C` | `0x800A147C` | `wtbl_800A147C` | {'lui': 27, 'lw': 27} |
| `D_800A14C0` | `0x800A14C0` | `wtbl_800A14C0` | {'lui': 5, 'lw': 5} |
| `D_800A14D8` | `0x800A14D8` | `wtbl_800A14D8` | {'lui': 7, 'sw': 3, 'lw': 4} |
| `D_800A14DC` | `0x800A14DC` | `wtbl_800A14DC` | {'lui': 5, 'sw': 2, 'lw': 2, 'addiu': 1} |
| `D_800A14E0` | `0x800A14E0` | `wtbl_800A14E0` | {'lui': 6, 'sw': 1, 'lw': 5} |
| `D_800A14E4` | `0x800A14E4` | `wtbl_800A14E4` | {'lui': 20, 'sw': 10, 'lw': 10} |
| `D_800A14EC` | `0x800A14EC` | `wtbl_800A14EC` | {'lui': 5, 'sw': 2, 'addiu': 1, 'lw': 2} |
| `D_800A14F0` | `0x800A14F0` | `wtbl_800A14F0` | {'lui': 8, 'sw': 3, 'addiu': 1, 'lw': 4} |
| `D_800A14F4` | `0x800A14F4` | `wtbl_800A14F4` | {'lui': 4, 'sw': 1, 'lw': 3} |
| `D_800A14F8` | `0x800A14F8` | `wtbl_800A14F8` | {'lui': 4, 'sw': 1, 'lw': 3} |
| `D_800A14FC` | `0x800A14FC` | `wtbl_800A14FC` | {'lui': 4, 'sw': 1, 'lw': 3} |
| `D_800A1500` | `0x800A1500` | `wtbl_800A1500` | {'lui': 9, 'lw': 5, 'addiu': 4} |
| `D_800A1510` | `0x800A1510` | `wtbl_800A1510` | {'lui': 3, 'lw': 3} |
| `D_800A151C` | `0x800A151C` | `wtbl_800A151C` | {'lui': 3, 'lw': 2, 'sw': 1} |
| `D_800A2604` | `0x800A2604` | `wtbl_800A2604` | {'lui': 8, 'lw': 8} |
| `D_800A260C` | `0x800A260C` | `wtbl_800A260C` | {'lui': 4, 'lw': 4} |
| `D_800A2634` | `0x800A2634` | `wtbl_800A2634` | {'lui': 9, 'lw': 7, 'sw': 2} |
| `D_800A263C` | `0x800A263C` | `wtbl_800A263C` | {'lui': 7, 'lw': 7} |
| `D_800A2668` | `0x800A2668` | `wtbl_800A2668` | {'lui': 3, 'lw': 2, 'sw': 1} |
| `D_800A26CC` | `0x800A26CC` | `wtbl_800A26CC` | {'lui': 6, 'lw': 2, 'sw': 4} |
| `D_800A26D0` | `0x800A26D0` | `wtbl_800A26D0` | {'lui': 4, 'lw': 1, 'addiu': 1, 'sw': 2} |
| `D_800A26D8` | `0x800A26D8` | `wtbl_800A26D8` | {'lui': 5, 'lw': 2, 'sw': 3} |
| `D_800A2870` | `0x800A2870` | `wtbl_800A2870` | {'lui': 6, 'sw': 1, 'lw': 5} |
| `D_800A2874` | `0x800A2874` | `wtbl_800A2874` | {'lui': 7, 'sw': 2, 'lw': 5} |
| `D_800A2878` | `0x800A2878` | `wtbl_800A2878` | {'lui': 3, 'sw': 2, 'lw': 1} |
| `D_800A287C` | `0x800A287C` | `wtbl_800A287C` | {'lui': 5, 'sw': 4, 'lw': 1} |
| `D_800A2880` | `0x800A2880` | `wtbl_800A2880` | {'lui': 3, 'lw': 2, 'sw': 1} |
| `D_800A2884` | `0x800A2884` | `wtbl_800A2884` | {'lui': 5, 'lw': 3, 'sw': 2} |
| `D_800A288C` | `0x800A288C` | `wtbl_800A288C` | {'lui': 9, 'sw': 2, 'lw': 7} |
| `D_800A2894` | `0x800A2894` | `wtbl_800A2894` | {'lui': 5, 'sw': 5} |
| `D_800A2898` | `0x800A2898` | `wtbl_800A2898` | {'lui': 5, 'sw': 5} |
| `D_800A289C` | `0x800A289C` | `wtbl_800A289C` | {'lui': 5, 'sw': 3, 'lw': 2} |
| `D_800A28A0` | `0x800A28A0` | `wtbl_800A28A0` | {'lui': 11, 'lw': 5, 'sw': 6} |
| `D_800A2CD4` | `0x800A2CD4` | `wtbl_800A2CD4` | {'lui': 7, 'lw': 6, 'sw': 1} |
| `D_800A2CD8` | `0x800A2CD8` | `wtbl_800A2CD8` | {'lui': 4, 'lw': 2, 'sw': 2} |
| `D_800A2CDC` | `0x800A2CDC` | `wtbl_800A2CDC` | {'lui': 106, 'lw': 106} |
| `D_800A2CEC` | `0x800A2CEC` | `wtbl_800A2CEC` | {'lui': 4, 'lw': 4} |
| `D_800A2CF8` | `0x800A2CF8` | `wtbl_800A2CF8` | {'lui': 7, 'sw': 5, 'lw': 2} |
| `D_800A2D04` | `0x800A2D04` | `wtbl_800A2D04` | {'lui': 14, 'lw': 13, 'sw': 1} |
| `D_800A2D0C` | `0x800A2D0C` | `wtbl_800A2D0C` | {'lui': 3, 'lw': 2, 'sw': 1} |
| `D_800A2D10` | `0x800A2D10` | `wtbl_800A2D10` | {'lui': 8, 'sw': 6, 'lw': 2} |
| ... 217 more | | | |

## Short-table candidates (lh/sh only access) (137)

| current | address | proposed | usage |
|---|---|---|---|
| `D_800963EE` | `0x800963EE` | `stbl_800963EE` | {'lui': 4, 'lh': 4} |
| `D_80099478` | `0x80099478` | `stbl_80099478` | {'lui': 6, 'lh': 3, 'sh': 2, 'lhu': 1} |
| `D_8009947A` | `0x8009947A` | `stbl_8009947A` | {'lui': 9, 'sh': 6, 'lh': 3} |
| `D_80099D88` | `0x80099D88` | `stbl_80099D88` | {'lui': 32, 'lhu': 32} |
| `D_8009B2BC` | `0x8009B2BC` | `stbl_8009B2BC` | {'lui': 16, 'lh': 12, 'lhu': 4} |
| `D_8009B2BE` | `0x8009B2BE` | `stbl_8009B2BE` | {'lui': 9, 'lh': 7, 'lhu': 2} |
| `D_8009BE78` | `0x8009BE78` | `stbl_8009BE78` | {'lui': 9, 'sh': 1, 'lh': 8} |
| `D_8009BE7A` | `0x8009BE7A` | `stbl_8009BE7A` | {'lui': 9, 'sh': 1, 'lh': 8} |
| `D_800A157A` | `0x800A157A` | `stbl_800A157A` | {'lui': 3, 'lhu': 1, 'sh': 2} |
| `D_800A2890` | `0x800A2890` | `stbl_800A2890` | {'lui': 3, 'sh': 3} |
| `D_800A2892` | `0x800A2892` | `stbl_800A2892` | {'lui': 3, 'sh': 3} |
| `D_800A2CF4` | `0x800A2CF4` | `stbl_800A2CF4` | {'lui': 9, 'lhu': 5, 'sh': 4} |
| `D_800A3074` | `0x800A3074` | `stbl_800A3074` | {'lui': 3, 'lh': 3} |
| `D_800A3174` | `0x800A3174` | `stbl_800A3174` | {'lh': 12, 'lui': 1, 'addiu': 1} |
| `D_800A3176` | `0x800A3176` | `stbl_800A3176` | {'lh': 4} |
| `D_800A32B4` | `0x800A32B4` | `stbl_800A32B4` | {'sh': 3, 'lhu': 1, 'lh': 1} |
| `D_800A32B6` | `0x800A32B6` | `stbl_800A32B6` | {'sh': 3, 'lhu': 1, 'lh': 1} |
| `D_800A3310` | `0x800A3310` | `stbl_800A3310` | {'sh': 2, 'lhu': 1, 'lh': 1} |
| `D_800A33E8` | `0x800A33E8` | `stbl_800A33E8` | {'lui': 1, 'sh': 2, 'lh': 2} |
| `D_800A33EA` | `0x800A33EA` | `stbl_800A33EA` | {'lh': 3, 'sh': 1} |
| `D_800A345C` | `0x800A345C` | `stbl_800A345C` | {'sh': 2, 'lh': 1} |
| `D_800A345E` | `0x800A345E` | `stbl_800A345E` | {'sh': 2, 'lh': 1} |
| `D_800A3528` | `0x800A3528` | `stbl_800A3528` | {'sh': 4, 'lh': 3, 'lhu': 3} |
| `D_800A3530` | `0x800A3530` | `stbl_800A3530` | {'lui': 3, 'sh': 2, 'lh': 1} |
| `D_800A3544` | `0x800A3544` | `stbl_800A3544` | {'lui': 3, 'sh': 2, 'lh': 1} |
| `D_800A3550` | `0x800A3550` | `stbl_800A3550` | {'lhu': 4, 'lh': 1, 'sh': 6} |
| `D_800A3580` | `0x800A3580` | `stbl_800A3580` | {'sh': 4, 'lhu': 2, 'lh': 17} |
| `D_800A3588` | `0x800A3588` | `stbl_800A3588` | {'sh': 1, 'lh': 5, 'lui': 5, 'addiu': 1} |
| `D_800A358A` | `0x800A358A` | `stbl_800A358A` | {'sh': 3, 'lh': 2} |
| `D_800A358C` | `0x800A358C` | `stbl_800A358C` | {'sh': 1, 'lh': 2, 'lui': 3, 'addiu': 2} |
| `D_800A358E` | `0x800A358E` | `stbl_800A358E` | {'sh': 2, 'lh': 1} |
| `D_800A3590` | `0x800A3590` | `stbl_800A3590` | {'lui': 10, 'addiu': 7, 'lh': 2, 'sh': 1} |
| `D_800A3594` | `0x800A3594` | `stbl_800A3594` | {'lui': 4, 'sh': 1, 'lh': 3} |
| `D_800A35C8` | `0x800A35C8` | `stbl_800A35C8` | {'lui': 1, 'addiu': 1, 'sh': 3} |
| `D_800A35CA` | `0x800A35CA` | `stbl_800A35CA` | {'sh': 3} |
| `D_800A36A4` | `0x800A36A4` | `stbl_800A36A4` | {'lui': 23, 'lh': 14, 'sh': 6, 'lhu': 3} |
| `D_800A36C6` | `0x800A36C6` | `stbl_800A36C6` | {'lui': 3, 'lh': 3} |
| `D_800A36CA` | `0x800A36CA` | `stbl_800A36CA` | {'lui': 3, 'sh': 2, 'lhu': 1} |
| `D_800A36F6` | `0x800A36F6` | `stbl_800A36F6` | {'lui': 11, 'sh': 3, 'lh': 8} |
| `D_800A36FC` | `0x800A36FC` | `stbl_800A36FC` | {'lui': 3, 'sh': 2, 'lh': 1} |
| `D_800A3710` | `0x800A3710` | `stbl_800A3710` | {'lui': 9, 'lhu': 2, 'sh': 5, 'lh': 3} |
| `D_800A3714` | `0x800A3714` | `stbl_800A3714` | {'sh': 2, 'lh': 2} |
| `D_800A376E` | `0x800A376E` | `stbl_800A376E` | {'lui': 7, 'sh': 5, 'lh': 2} |
| `D_800A379E` | `0x800A379E` | `stbl_800A379E` | {'sh': 11, 'lh': 1} |
| `D_800A37C4` | `0x800A37C4` | `stbl_800A37C4` | {'lhu': 2, 'sh': 3} |
| `D_800A37C8` | `0x800A37C8` | `stbl_800A37C8` | {'sh': 4, 'lh': 3} |
| `D_800A3814` | `0x800A3814` | `stbl_800A3814` | {'sh': 5, 'lhu': 1} |
| `D_800A381C` | `0x800A381C` | `stbl_800A381C` | {'lui': 4, 'sh': 2, 'lh': 2} |
| `D_800A3824` | `0x800A3824` | `stbl_800A3824` | {'lui': 6, 'sh': 2, 'lh': 2, 'lhu': 2} |
| `D_800A382E` | `0x800A382E` | `stbl_800A382E` | {'lui': 5, 'sh': 3, 'lhu': 1, 'lh': 1} |
| ... 87 more | | | |

## Byte-table candidates (lb/sb only access) (169)

| current | address | proposed | usage |
|---|---|---|---|
| `D_8008D118` | `0x8008D118` | `btbl_8008D118` | {'lui': 58, 'lbu': 58} |
| `D_80094E08` | `0x80094E08` | `btbl_80094E08` | {'lui': 4, 'lbu': 4} |
| `D_80099D8B` | `0x80099D8B` | `btbl_80099D8B` | {'lui': 4, 'lbu': 3, 'sb': 1} |
| `D_8009A853` | `0x8009A853` | `btbl_8009A853` | {'lui': 3, 'lbu': 3} |
| `D_8009AD18` | `0x8009AD18` | `btbl_8009AD18` | {'lui': 5, 'addiu': 2, 'lbu': 3} |
| `D_8009BC0C` | `0x8009BC0C` | `btbl_8009BC0C` | {'lui': 4, 'lbu': 4} |
| `D_8009BC0D` | `0x8009BC0D` | `btbl_8009BC0D` | {'lui': 5, 'lbu': 5} |
| `D_8009BC40` | `0x8009BC40` | `btbl_8009BC40` | {'lui': 14, 'lbu': 14} |
| `D_8009BC7C` | `0x8009BC7C` | `btbl_8009BC7C` | {'lui': 35, 'lbu': 23, 'sb': 7, 'addiu': 5} |
| `D_8009BCE4` | `0x8009BCE4` | `btbl_8009BCE4` | {'lui': 10, 'lbu': 5, 'sb': 4, 'addiu': 1} |
| `D_8009BD24` | `0x8009BD24` | `btbl_8009BD24` | {'lui': 7, 'lbu': 3, 'addiu': 4} |
| `D_8009BD3C` | `0x8009BD3C` | `btbl_8009BD3C` | {'lui': 4, 'sb': 2, 'lbu': 2} |
| `D_8009BD42` | `0x8009BD42` | `btbl_8009BD42` | {'lui': 4, 'lbu': 2, 'sb': 2} |
| `D_8009BD8D` | `0x8009BD8D` | `btbl_8009BD8D` | {'lui': 5, 'lbu': 5} |
| `D_8009BE74` | `0x8009BE74` | `btbl_8009BE74` | {'lui': 11, 'addiu': 2, 'lbu': 9} |
| `D_8009BE76` | `0x8009BE76` | `btbl_8009BE76` | {'lui': 15, 'lbu': 11, 'addiu': 4} |
| `D_8009BE77` | `0x8009BE77` | `btbl_8009BE77` | {'lui': 6, 'addiu': 1, 'lbu': 5} |
| `D_800A11D5` | `0x800A11D5` | `btbl_800A11D5` | {'lui': 10, 'lbu': 8, 'sb': 2} |
| `D_800A1495` | `0x800A1495` | `btbl_800A1495` | {'lui': 8, 'sb': 6, 'lbu': 1, 'addiu': 1} |
| `D_800A1496` | `0x800A1496` | `btbl_800A1496` | {'lui': 6, 'sb': 3, 'lbu': 3} |
| `D_800A26DC` | `0x800A26DC` | `btbl_800A26DC` | {'lui': 4, 'lbu': 2, 'sb': 1, 'addiu': 1} |
| `D_800A26DD` | `0x800A26DD` | `btbl_800A26DD` | {'lui': 3, 'sb': 2, 'lbu': 1} |
| `D_800A26DE` | `0x800A26DE` | `btbl_800A26DE` | {'lui': 8, 'lbu': 4, 'sb': 4} |
| `D_800A31D8` | `0x800A31D8` | `btbl_800A31D8` | {'lbu': 1, 'sb': 2} |
| `D_800A31DA` | `0x800A31DA` | `btbl_800A31DA` | {'lui': 4, 'sb': 5, 'lbu': 5} |
| `D_800A31FC` | `0x800A31FC` | `btbl_800A31FC` | {'lbu': 5, 'sb': 13} |
| `D_800A3203` | `0x800A3203` | `btbl_800A3203` | {'lbu': 1, 'sb': 2} |
| `D_800A3204` | `0x800A3204` | `btbl_800A3204` | {'lbu': 8, 'sb': 15} |
| `D_800A3208` | `0x800A3208` | `btbl_800A3208` | {'lbu': 1, 'sb': 2} |
| `D_800A320C` | `0x800A320C` | `btbl_800A320C` | {'sb': 2, 'lbu': 2} |
| `D_800A32E9` | `0x800A32E9` | `btbl_800A32E9` | {'sb': 2, 'lbu': 1} |
| `D_800A3318` | `0x800A3318` | `btbl_800A3318` | {'sb': 3, 'lbu': 2} |
| `D_800A331C` | `0x800A331C` | `btbl_800A331C` | {'sb': 2, 'lbu': 1} |
| `D_800A3320` | `0x800A3320` | `btbl_800A3320` | {'sb': 2, 'lbu': 1} |
| `D_800A3324` | `0x800A3324` | `btbl_800A3324` | {'sb': 2, 'lbu': 1} |
| `D_800A3328` | `0x800A3328` | `btbl_800A3328` | {'sb': 2, 'lbu': 1} |
| `D_800A332C` | `0x800A332C` | `btbl_800A332C` | {'sb': 2, 'lbu': 1} |
| `D_800A3330` | `0x800A3330` | `btbl_800A3330` | {'sb': 3, 'lbu': 1} |
| `D_800A3338` | `0x800A3338` | `btbl_800A3338` | {'sb': 2, 'lbu': 1} |
| `D_800A333C` | `0x800A333C` | `btbl_800A333C` | {'sb': 2, 'lbu': 1} |
| `D_800A3340` | `0x800A3340` | `btbl_800A3340` | {'lbu': 1, 'sb': 2} |
| `D_800A3344` | `0x800A3344` | `btbl_800A3344` | {'lbu': 1, 'sb': 2} |
| `D_800A3348` | `0x800A3348` | `btbl_800A3348` | {'lbu': 1, 'sb': 2} |
| `D_800A334C` | `0x800A334C` | `btbl_800A334C` | {'sb': 10, 'lbu': 3} |
| `D_800A3350` | `0x800A3350` | `btbl_800A3350` | {'sb': 8, 'lbu': 4} |
| `D_800A3354` | `0x800A3354` | `btbl_800A3354` | {'sb': 6, 'lbu': 1} |
| `D_800A3561` | `0x800A3561` | `btbl_800A3561` | {'sb': 6, 'lui': 13, 'lbu': 15} |
| `D_800A3562` | `0x800A3562` | `btbl_800A3562` | {'lui': 15, 'lbu': 9, 'sb': 11} |
| `D_800A3563` | `0x800A3563` | `btbl_800A3563` | {'sb': 3, 'lbu': 2} |
| `D_800A3565` | `0x800A3565` | `btbl_800A3565` | {'sb': 4, 'lbu': 1} |
| ... 119 more | | | |

---

These are NAMING HINTS based on access width. Width tells you the element type; the actual semantic name still needs caller study. For example, an `lw`-only table could be `int[]`, a function-pointer table, or a packed RGB array.

Do NOT bulk-apply these names. The width classifier has many false positives (e.g., a struct field accessed with `lw` but actually a `char[4]`). Use them as suggestions for decomp-time naming, not as a rename batch.

# Hypothesis ledger — func_80063084

## Ruled out (2026-09-29 manual session)
- Inline `D_800A3448 & (1 << i)` at the loop top: combine -> srav/andi (9). Needs the `bit` local.
- Size clamps through a product local (`w`, `h`): 7 (C0 sra, C8 value seat); through s16/s32 result locals: 14; `(u32)w >> 8`: 6 and a cast.
- `-*fade * 255 / 5`: folded to *51 (fold-const multiple-of rule). `-*fade * 128`: negu-then-sll (target sll-then-negu).
- Flare colour as a ternary: shared-ori layout differs (hunk 31/32 of v1).
- Pool accessed through a cast view `((Rec *)&D_800F0E38)[i]`: 16 — loop.c makes the full address a giv; target keeps only i*12 in s6.

## Open
- (none at sandbox level; landing verification pending)

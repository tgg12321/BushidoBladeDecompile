"""wave72 rows: the unnamed functions of 51268.c that Codex researched (tmp/codex/w72-research.md:
6 RENAME leads, 69 NO_NAME), re-derived from the code at main c844b59a7. Codex names were leads only.
No kept address is another tmp/p3w* draft's (tmp/p3w58 only mentions 0x800644FC in its prose as
neutral; it holds no row for it). 3 RENAME. No new prefix (eff_ is admitted and in use).

Pre-apply review pending (VERDICT / VERIFIER_NOTE empty).

Not named (dropped Codex leads):
- 0x8006288C (eff_SpawnQuadCluster): the noun fits its twins. func_80062FEC and func_800645B0
  also claim a slot of a bitmask-managed table whose draw pass emits textured quads (51268.c:896-1024
  draws three per slot, 1058-1236 two per slot), so "QuadCluster" does not separate it from them,
  and what the three rotating, scaling quads depict is not established. Neutral.
- 0x80063BD0 (eff_StoreQuadSlot): same generic-noun problem (every spawner stores a quad slot); what
  sets it apart (the lane argument idx, the 10-entry fill then cyclic overwrite, 51268.c:1273-1299)
  is mechanism, and no role for the two lanes is shown (callers 0x80063AF0 / 0x80063B34 only differ
  by idx). Neutral.
- 0x80065800 (eff_DrawAnimatedQuadVariant): "Variant" restates a selector (the 16 call sites pass
  0-0x11 and mean nothing the code names), and the body (51268.c:1918-2287) builds differently
  shaped quads per selector, so a noun beyond "quad" guesses; wave58 already left it neutral.
- The 69 NO_NAME rows stay neutral (not re-mined).
"""

VERDICT = "CONFIRM"
VERIFIER_NOTE = 'naming-reviewer pre-apply (round 1, 3/3 CONFIRM). Apply notes: substitute old names in include/m2c_context.h:853/861/862 too; named_syms.txt:994 by exact line substitution; do not attach the flare/halo wording (51268.c:1033/1058) to the new names.'

FUNCS = [
    ('0x80062FEC', 'eff_SpawnBillboardPair', 'RENAME',
     'BODY 51268.c:1037-1056: sets the command phase D_800F10D0[8] = 1, claims the first free of 12 '
     'bits of D_800A3448, copies the position at *D_800A347C into D_800F0E38[i] and zeroes the age '
     'D_800F0BEC[i], returns 1. Its only use is the command-handler table (asm/data/7D920.data.s:23320, '
     '.word func_80062FEC); no jal site in src/ or asm/funcs. Consumer func_80063084 (51268.c:1065-1236) '
     'loops j = 0..1 per live slot and for each j projects the slot position (RotTransPers) and builds '
     'one screen-aligned textured POLY_FT4 whose corners are offsets from the projected centre, sized by '
     'depth (1099-1213): so each claimed slot is drawn as two billboards. WHY: eff_ is the textured-quad / '
     'OT effect system (the draw writes g_eff_quad_cursor and links into g_gpu_ot_ptr); Billboard is the '
     'screen-facing sprite quad, Pair the two quads per slot, Spawn the slot claim. COUNTER-EVIDENCE: '
     'the two quads are not always both emitted (j == 1 uses another texture and depth gate; a slot '
     'past age 16 draws neither, 1105/1229), so Pair names what a slot is drawn as, not a guarantee per '
     'frame; with all 12 bits taken the loop leaves i == 12 and writes past the table (comment '
     '1033-1036), the name does not promise a checked allocation; no flare/halo reading from the comment '
     'is used. Twins: func_800645B0 (one quad per slot, jittered) and func_8006288C (three rotating '
     'quads) are distinguished by Pair vs Jittered/neutral. "Billboard" is also in eff_DrawBillboardList '
     '(g_eff_billboards, a separate list), which this function does not touch. FRESH: eff_SpawnBillboardPair '
     'absent from src/, include/, named_syms.txt, symbol_addrs.txt and every tmp/p3w* draft. Comments to '
     'update: 51268.c:1058, include/game.h:107. CENSUS NAME (draft time): func_80062FEC (AUTO, '
     'splat-auto).'),
    ('0x800645B0', 'eff_SpawnJitteredBillboards', 'RENAME',
     'BODY 51268.c:1438-1460: sets D_800F10D0[7] = 1; for each of the four groups of four bits of '
     'D_800A3444 (i = 0, 4, 8, 12) it claims the first free slot, writes D_800F0D78[idx] = the position '
     'at D_800A347C plus (rand() & 0xFF) - 0x7F on each of x, y, z, sets the animation counter '
     'D_800F0BCC[idx] = rand() & 7, sets the bit, then moves to the next group; returns 1. Only use: '
     'the command-handler table (asm/data/7D920.data.s:23318); no jal site. Consumer func_800646E8 '
     '(51268.c:1469-1594) projects each live slot and emits ONE screen-aligned, depth-sized, '
     'semi-transparent POLY_FT4 per slot (1543-1575), picks its frame by counter / 4 and retires the '
     'slot at 7. WHY: eff_ as for func_80062FEC (same quad cursor and OT link); Billboards the '
     'screen-facing quads of the consumer; Jittered because every path offsets all three axes by '
     'rand() (no deterministic branch); plural because up to four slots (one per group) are claimed per '
     'call. COUNTER-EVIDENCE: a full group claims nothing for that group and the call still returns 1; '
     'the random counter start is not in the name (only the position jitter is); no dust/smoke reading '
     'taken from the textures. Twin func_80062FEC is told apart by Jittered vs Pair. FRESH: '
     'eff_SpawnJitteredBillboards absent from src/, include/, named_syms.txt, symbol_addrs.txt and '
     'every tmp/p3w* draft. Comments to update: 51268.c:1434, 51268.c:1462, include/bb2.h:134, '
     'named_syms.txt:994 (D_800A3444 note). CENSUS NAME (draft time): func_800645B0 (AUTO, splat-auto).'),
    ('0x800644FC', 'eff_BuildUsedSlotRotMatrices', 'RENAME',
     'BODY 51268.c:1419-1432 (s32 *count, MATRIX *m, s32 idx): for i < *count, when bit i of '
     'D_800A3454[idx] is set, RotMatrix(&D_800F1000[idx][i], &m[i]); nothing else is written. Only '
     'caller: func_80063E10 (51268.c:1339; asm/funcs/func_80063E10.s:36 is the only jal), which passes '
     'its work-area matrix array and then fills m[i].t from D_800F0EC8[idx][i] (1370-1372), composes with '
     'the camera matrix and projects a textured quad (1373-1397). func_80063BD0 sets those same bits and '
     'writes D_800F1000[idx][slot] (the SVECTOR it rotates, 1282-1283). WHY: eff_ because its data (the '
     'D_800A3454 slot mask, D_800F1000 angles) is the slot table that func_80063BD0 fills from the effect '
     'command block and func_80063E10 draws into the eff quad buffer; Build the matrices, Rot because '
     'RotMatrix only sets the rotation part (the translation is set by the caller), UsedSlot because the '
     'mask bit is the in-use bit that func_80063BD0 sets. COUNTER-EVIDENCE: wave58 left it neutral since '
     'its role comes from a caller that is itself unnamed; the name here claims only what the body does '
     '(matrices for set slots), not what they are used for, and avoids Quad. Same-shape functions: none '
     'found (grep RotMatrix in 51268.c: only this loop builds a matrix array from a slot mask). FRESH: '
     'eff_BuildUsedSlotRotMatrices absent from src/, include/, named_syms.txt, symbol_addrs.txt and every '
     'tmp/p3w* draft. Comments to update: 51268.c:1416; 51268.c:1339 is a call; named_syms.txt:3099 is a '
     'RESET note naming the address only. CENSUS NAME (draft time): func_800644FC (AUTO, splat-auto).'),
]

DATA = []

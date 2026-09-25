# func_8001F2E4 — evidence (manual session 2026-09-24)

First session on this function (no prior ledger). Queue distance 347 (INCLUDE_ASM
stub) -> 0 in one manual session. Census member of the 2026-08-17 cop2
addressing-preamble cluster (`.claude/rules/cop2-addressing-preamble-cluster.md`
row :65, LZCS/LZCR sub-family, 2 idiom sites).

## What it does

Called from func_80023F08 with (obj, set_a, set_b). Eases obj's bone-angle
targets toward its partner (`*(u8 **)obj`): heading (obj+0x1D8 - obj+0x1CA,
wrapped, clamped ±0x1FF) and elevation (0x400 - ratan2(ground distance, height
delta), clamped ±0xFF) while obj+0x6A is 0x15/0x25 (state 0x1F holds elevation
0x100); each is eased by 1/8 of the wrapped difference into obj+0x1E6/0x1E8 and
applied to set_a+0x36 / set_b+0x36 via func_8002F770(angles, z, y, x). Twist
obj+0x1EA is set (states 0x1D/0xE) or eased (obj+0xE in 6..7, obj+0x6A == 2).
Random jitter into the four angle fields when obj+0x26E is set.
Ground distance: D_8008D118 byte-LUT isqrt with the GTE LZC for inputs >= 0x400
(same spelling as the matched func_8002E838, code6cac_b.c).

## Measurements (sandbox --disable all, --candidate; 347 target insns)

| step | score | note |
|---|---|---|
| first natural transcription | 33 | 346/347 insns |
| else-arm order `tgt_x = 0` first | 28 | fixes the 0x6A-branch delay slot |
| block-3 angle in the delta var + block-4 angle in the elevation var | 10 | reuse, see below |
| `(ratan2(..) - 0x400) & 0xFFF` as one expression | 6 | target masks the `addiu -0x400` result directly |
| block-4 delta sharing a pseudo with the rng jitter | **0** | 347/347 |

Full-build: `verify-oracle --rebuild` build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa,
build_matches true (2026-09-24). Frame 0x48 matched without any frame construct.

## Variable-reuse ablations (final chassis, one reuse separated at a time)

Rejected variants were banked pre-rename (`d` = the final `ang`, `ang_y` = `tgt_y`).

| variant | score | file |
|---|---|---|
| block-3 twist in its own local | 11 | rejected/split-twist-11.c |
| block-4 twist target in its own local | 9 | rejected/split-twist-target-9.c |
| rng jitter in its own local | 6 | rejected/split-jitter-6.c |
| all three separated | 26 | rejected/split-all-26.c |
| block-4 delta in a fresh block-local | 11 | (tmp only) |
| block-4 delta in `dist` / `dist_sq` | 3 / 6 | (tmp only) |

What each reuse fixes (read from the --diff): block-3 twist lands in $a0 (a
separate local gets $v1); block-4 twist target lands in $a1 (separate: $v1); with
the jitter sharing the pseudo, the block-4 `/8` divides in place in $a0 instead
of through a copied $v0 temp. Allocation-level effects observed in the diff, not
dump-traced.

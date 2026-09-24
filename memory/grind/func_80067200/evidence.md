# func_80067200 — evidence (manual session 2026-09-24)

Result: candidate.c scores 0 on `sandbox --disable all` (298/298, rules_dropped 0)
and, spliced into src/text1b.c, the full build SHA1 == oracle
62efab4f73f992798c43e8c730aa43baa10bb4fa (verify-oracle --rebuild --allow-dirty).

## What the function is
Particle/debris spawner. arg0 = effect kind (0..7), arg1 = lane (0..3),
arg2 = which half of the lane's 48 records (0/1). Resets D_800A3438[arg1],
loads the camera rotation (gte_SetRotMatrix(D_800A3474)); on arg2 == 0 snapshots
the source position (D_800A347C -> D_800F0C10[arg1][0]) and scale vector
(D_800A3478 -> D_800F0B78[arg1]). Then for 24 records: a 6-word rand() XOR chain,
three random angles (masked/offset by kind), three randomly scaled components
of the lane scale vector, RotMatrix(angle) and a GTE rotate of the scaled vector
into the record (+8/+A/+C). Returns 1.

## Load-bearing facts (measured)
1. `mask`/`base` must be `s16`. With s32 `mask` the top-of-function constants and
   the loop divisor share one callee-saved reg (s3 vs target s5->s2 copy) — score 34.
   The target's `move s2,s5` in the loop preheader is the hoisted
   (s32)(s16)mask sign-extend that combine reduced to a move: mask is always
   assigned a small positive constant on every path, so combine knows its sign
   bits. amask/aoff are NOT assigned on the arg0 >= 8 path (live at entry), so
   their hoisted extends stay sll/sra (target fp/s7). Mechanism read from the
   instrumented cc1 .loop/.greg dumps (tools/ra_solver extract via
   tmp/f67200/ra.py): movables 398/399, 403/404 = the amask/aoff extends.
2. `s16 base` too: with s32 base (and s16 mask) score 11.
3. The angle block must precede the mask/scale block in the loop body
   (ang-after-scale with s16 base/mask: 25). The division trap branches are
   inside one RTL insn, so sched1 reorders freely; the order matters through
   loop.c movable order (the mask extend must be hoisted AFTER the amask/aoff
   extends).
4. Angle statement order r[0] (vy), r[1] (vx), r[2] (vz) — vx-first scores 6
   (stack store order of r[0]/r[1]).
5. `u8 count = 0x30; (count >> 1) * arg2`: the unfolded constant is rematerialized
   by reload as `li t7,0x30; srl` at both uses (pseudo spilled, REG_EQUIV const).
   u8 makes the shift logical (srl) while keeping the int compare signed (slt).
6. Declaration order of the scalar locals is inert (6 orders, all 7 on the
   pre-s16 chassis).

## GTE islands
Five inline islands, each a PsyQ Release 4.3 inline_c.h macro body:
gte_SetRotMatrix :297-310 (x2), gte_ldv0 :16-20, gte_rtv0 :499-502,
gte_stlvnl :1111-1117. Headers pinned: silent-hill-decomp@a1f407cb sha256
de1a70ef..., xenogears-decomp@54d7ef3e sha256 2f1261e5... (fetched to
tmp/f67200/hdr/). gte_rtv0's header word is the DMPSX placeholder 0x0000013f;
the island carries the post-DMPSX word 0x4A486012 (target 0x80067630). Owner
ruling 2026-09-24 (inline-asm-policy.md § Extension) admits exactly that swap.

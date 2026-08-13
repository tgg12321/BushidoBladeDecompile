# Hypotheses — func_800470B0

## Session 1 (2026-08-12, recon) — all resolved; function MATCHED

### H1 — CONFIRMED. The pre-existing pins/pad were costing instructions, not buying them.
Statement: the `register void *var_s1 asm("s1")` / `register s32 var_s2 asm("s2")`
pins and the `volatile s32 _sp_pad[2]` local in the HEAD body were not load-bearing;
removing them would lower the honest floor rather than raise it.
Mechanism: the sandbox strips cheat-asm before scoring, so pins can never lower the
`--disable all` distance, but the volatile pad emits real frame arithmetic that the
stripped build still pays for.
Probe: delete all three, re-run `sandbox --disable all`.
Result: 22 -> 17, and build_insns 92 -> 88 (exact match with target's 88).
Verdict: CONFIRMED.

### H2 — CONFIRMED. The 8-byte frame shortfall is a TYPE error, not a padding problem.
Statement: target's 32-byte locals region holds a PsyQ `MATRIX` (s16 m[3][3] +
u16 pad + s32 t[3] = 32 bytes), not the `s16 sp10[9]` (24 bytes rounded) the
decompilation declared.
Mechanism: `sizeof(MATRIX) == 32` exactly accounts for sp+0x10..sp+0x2F, and
`func_80052930` is the matrix-transform leaf that `camera_Transform` calls with
`&g_cam_matrix`, so a MATRIX is the object the callee expects. The unwritten
`pad`/`t[3]` tail is genuine dead space in a correctly-typed local.
Probe: `#include "gte.h"`, declare `MATRIX sp10;`, rewrite `sp10[k]` as
`sp10.m[r][c]`, pass `&sp10`.
Result: 17 -> 7; frame 56 -> 64, all prologue/epilogue save offsets aligned.
Verdict: CONFIRMED.

### H3 — CONFIRMED. The residual s1/s2 swap is a dropped call argument.
Statement: arg2 landing in `$s2` and arg3 in `$s1` (target wants the reverse) is
caused by the call to `func_80052930` passing two arguments to a three-parameter
function; restoring the destination argument fixes both the correctness defect and
the allocation.
Mechanism (diagnosis): `.greg` showed pseudo 74 (arg2) -> hard reg 18, pseudo 75
(arg3) -> hard reg 17. Equal n_refs (4 each), but arg3 dies six insns earlier, so
`global.c`'s `allocno_compare` — priority ~ `floor_log2(n_refs)*n_refs*size /
live_length` — ranked arg3 first. The extra `arg2` reference at the call raises its
n_refs to 5 (`floor_log2(5)*5 = 10` vs `8`), flipping the rank.
Mechanism (justification, independent of GCC): `src/sound.c:46` declares the callee
with three parameters; `camera_Transform` (`src/sound.c:424`) passes (matrix, src,
dst); `src/text1a_post.c:166` passes three; `include/m2c_context.h:738` prototypes
three. The two-argument call was simply wrong.
Probe: `func_80052930(&sp10, var_s0, arg2)`, re-run sandbox, then full build.
Result: 7 -> **0**; full build SHA1 == oracle. Zero extra instructions (arg2 is
already live in `$a2` at the call, so GCC coalesces the copy).
Verdict: CONFIRMED.

### H4 — KILLED. Statement reordering cannot equalise the two live ranges.
Statement: moving `arg2[6] = arg3;` to the end (or the start) of the three
destination stores would extend arg3's live range to arg2's death point, equalising
`live_length` so `allocno_compare`'s tie-break (lower allocno number wins) would
hand `$s1` to arg2.
Mechanism: `allocno_compare` returns `v1 - v2` on a priority tie, and arg2's pseudo
(74) precedes arg3's (75).
Probe: both reorderings, sandbox each.
Result: 8 and 8 — WORSE than the 7 baseline. sched1 re-hoists the `sw` into the same
slot, so post-schedule live_length is unchanged and the swap persists; the only
effect is a store-order mismatch. Store order 5,6,7 is load-bearing.
Verdict: KILLED. Forms banked in `rejected/store-order-z-last.c` and
`rejected/store-order-z-first.c`.

### H5 — KILLED (as a lever; kept as cleanup). Pointer typing is byte-neutral here.
Statement: retyping `arg1`/`arg2` from `void *` to `s32 *` (with `[]` indexing
replacing the `*(s32 *)((s8 *)p + 0xNN)` casts) would perturb the allocation.
Probe: apply, sandbox.
Result: 7 -> 7. No codegen effect whatsoever.
Verdict: KILLED as a lever. The change was RETAINED anyway, purely for readability
and parity with the sibling `camera_Transform(s32 *a0, s32 *a1, s32 a2)`.

## Frontier
Empty — the function is matched and byte-verified against the oracle. No open
hypotheses remain.

# Evidence bank — func_80033D38

## Session s1 (2026-07-29, modality: recon) — CLOSED AT DISTANCE 0

### Baseline
- `canonical func_80033D38` → verdict **C**, asm_insns 0, total 47, distance 17
  ("pure-C distance 17 <= 50 — pure-C target"). Not an asm-routing candidate.
- `sandbox func_80033D38 --disable all` on the inherited HEAD body → **17**
  (50 build insns vs 47 target), 6 regfix rules dropped. The inherited body was
  a pinned goto bridge (`register u8 *t1 asm("t1")`, `register s32 a3 asm("a3")`,
  `register s32 v1 asm("v1")`, plus `t0`/`a2`/`v1` pins in the two inner blocks);
  the sandbox strips all of it, so 17 was the honest floor of the un-pinned
  goto form, not of the pins.
- regfix.txt carries 6 legacy bridge rules for this function at lines 3447-3452
  (2 substs, 1 delete, insert_label + label re-point, 1 reorder), commented
  "retire bridge -> pure C".

### Semantics recovered from asm/funcs/func_80033D38.s (47 insns, no calls)
`D_80106A50` is an array of 8-byte records `{ u8 x; u8 y; s32 t; }` — `t` at
offset +4, stride 8. Slots 1..3 are a 3-deep event history ordered oldest-last;
slot 0 overlaps the bit-flags word the rest of code6cac_b.c reads as
`D_80106A50 & mask`, and this function never touches it. The body: scan
n = 3,2,1 for the highest slot whose `recs[n].t` is older than the current time
`D_800A3858`; publish n to the u8 `D_800A38E9`; if n < 3, shift `recs[k+1] =
recs[k]` for k = 2 down to n+1 (an 8-byte struct copy, emitted lw/lw/sw/sw);
then write `recs[n+1] = { (u8)D_80101ED2, (u8)D_80101ED6, D_800A3858 }`.

Two provenance details read off the target, both load-bearing:
- The final `.t` store RELOADS `D_800A3858` (`lui/lw` at the tail) even though
  the loop already holds it in `a0`. That is only consistent with the source
  reading the GLOBAL in both places: the shift loop's stores kill the cached
  memory value, forcing a reload. Hoisting it into a local `s32 now =
  D_800A3858;` keeps it in a register and measured 11, worse than 9.
- The `lhu` (not `lh`) loads of D_80101ED2/D_80101ED6 fall out of a plain
  `(u8)` narrowing cast on the existing `s16` declarations; no header change
  was needed.

### The measured floor ladder
17 (inherited pinned goto bridge) → **9** (natural single-variable record-insert)
→ **4** (two-variable scan, `while (1)`) → **1** (`j = n - 1` moved to the loop
top) → **0** (element addresses via named pointer variables). Each step is a
distinct mechanism; all four are written up as levers in `candidate.c`.

### Tooling built this session (both reusable)
- `tmp/grind/func_80033D38/s1/cmp.py` — **the important one.** `sandbox
  --disable all` reports a REGISTER-MASKED score. cmp.py canonicalizes both
  sides (pseudo-ops li/move/b/negu expanded, immediates to signed decimal,
  `%hi`/`%lo` and R_MIPS_HI16/LO16 relocs to H/L placeholders, branch targets to
  relative insn indices) and reports a register-INCLUSIVE differing-instruction
  count plus a unified diff. Use it to confirm any masked 0 before claiming a
  match; it confirmed the final form at 47/47 instructions, 0 differing.
- `tmp/grind/func_80033D38/s1/sweep{,2,3,4,5}.py` — patch/build/score harness
  that swaps only the function body and reports masked + true distance per
  variant. 60 forms measured this session in 5 batches; this is the reason the
  session closed in one pass and is worth copying for the next function.

### Negative result worth keeping
`diagnose func_80033D38 --detail` is MISLEADING for this function: it builds
with the 6 legacy regfix rules APPLIED, so against any new body it reported
d22 with a fabricated per-instruction diff (a two-walking-pointer shift loop
that the actual sandbox object does not contain). Use the sandbox object plus
cmp.py, never `diagnose --detail`, while stale rules exist for the function.

Three forms compiled UNSCORABLE (`"scorable": false`) rather than producing a
score: `while (recs[i].t >= now) { i--; if (i <= 0) break; }`,
`while (recs[i].t >= now) { if (--i <= 0) break; }`, and
`do { if (recs[i].t < now) break; i--; } while (i > 0);`. Not investigated —
they are single-variable forms, and the whole single-variable family is dead for
the structural reason in `rejected/single-var-for-strength-reduced.c`.

### Cheat-policy self-check on the final form
Zero register-asm pins, zero `__asm__`, zero volatile, zero alias renames, zero
dead stores or self-assigns, zero unused declarations, no `do {} while (0)`.
Every local (`recs`, `n`, `j`, `k`, `p`, `ins`) is written and read on a live
path. `n` and `j` are both genuine program values (insertion slot and scan
index); `p`/`ins` are ordinary named element addresses. Nothing in the body
needs a GCC-internals justification to explain what it COMPUTES — the levers
explain only why one of several behaviourally identical spellings was chosen,
which is ordinary matching work, not coercion.

### Outstanding integration step (NOT a blocker on the C)
The 6 regfix rules at regfix.txt:3447-3452 were written for the old bridge body
and are now wrong. They are outside this session's allowed surface. The
operator/driver must run `retire func_80033D38` (drops the rules, full-build
SHA1 verify) before `queue done`. The candidate does not depend on them — the
sandbox scored it 0 with all 6 dropped.

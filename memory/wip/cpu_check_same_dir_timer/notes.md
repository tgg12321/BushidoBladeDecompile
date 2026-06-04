# cpu_check_same_dir_timer — WIP resume notes

## TL;DR

- HEAD byte-matches via **3 cheat-asm constructs** (a1val `register asm("a1")`
  pin, arg0 `register asm("a0")` + `asm("" : "=r"(arg0) : "0"(arg0))` barrier,
  trailing `asm volatile("")` scheduling barrier). All forbidden per current
  policy (`inline-asm-policy.md`, `register-asm-pins.md`,
  `inline-asm-injection.md`).
- Cheat-free pure-C floor (in `candidate.c`): **2 actual byte diffs** — a
  single branch-sense flip + its jump-target swap. Masked sandbox score: 1.
- The original commit (`9490fd3a`, 2026-04-05) explicitly identifies this as
  *"GCC's branch-inversion peephole"* and used the `asm volatile("")` barrier
  to defeat it. That barrier IS the cheat — a pure-C lever to defeat the
  peephole has not been found.
- 8 structural variants tested this session, all converge on the same 2-insn
  diff (see `meta.json.sessions[0].levers_tested`).

## How to resume in one read

1. Read `meta.json` — full lever ledger + next_hypotheses.
2. `cp candidate.c`'s body into `src/code6cac_b.c` (replace the existing
   `cpu_check_same_dir_timer` body).
3. `& tools/eng.ps1 sandbox cpu_check_same_dir_timer --disable all` →
   expect score 1 / 2 masked-insn diffs.
4. `python3 tmp/diff_csmdt.py` (already written) shows the diff. Insns 48
   (`bnez` vs `beqz`) + 50 (`j` target) are the residual.
5. Pick from `next_hypotheses`. Most promising: permuter from `candidate.c`
   base (~1000+ iters), OR instrumented cc1 (BB2_REORG_DEBUG /
   BB2_JUMP_DEBUG) to dump jump.c's `invert_jump` decision and identify the
   source-driven trigger.

## The residual diff in detail

Target asm at maspsx insn 47-50 (the `dir < val ? done : next` branch):
```
slt   v0, v1, v0    ; v0 = (dir < val)
bnez  v0, .L80033480  ; conditional to done (function-exit path)
nop                   ; delay slot
j     .L80033470      ; unconditional to next (loop-continuation path)
nop
```

My cheat-free build emits at the same position:
```
slt   v0, v1, v0
beqz  v0, .L80033470  ; conditional to next (loop continuation)
nop
j     .L80033480      ; unconditional to done (function exit)
nop
```

Semantically identical (`if (dir<val) goto done; else goto next;`); GCC
chose opposite polarity. This is jump.c's `invert_jump` reordering blocks
based on a heuristic (likely "branch-to-loop-target predicted taken"
favoring next as the conditional-true target, then inverting).

The barrier between the goto's prevents whichever jump.c pass does the
swap from seeing the two gotos as a fold-candidate. Without the barrier,
GCC inverts.

## Why pure-C levers tested didn't help

The branch-sense decision happens in jump.c's `invert_jump` pass AFTER
the C structure has been lowered to RTL. By that point, statement order,
goto label positions, if/else-if structure are all flattened. The RTL has
two unconditional jumps and one conditional jump, and the pass picks which
to be conditional based on block layout — which is driven by the same
"most likely path" heuristic regardless of source spelling.

The only sources that changed score:
- `do { } while (...)` form: +4 entry-test insns (score 19, regression).
- Inverted outer-if (`val != dir` first): BB layout shuffle (score 7,
  regression).

Both are worse, not better.

## Why this is parked, not committed

Current HEAD: byte-matches via cheat-asm. SHA1 = oracle. INCOMPLETE under
expanded cheat catalog (the 3 cheat-asm constructs).

Cheat-free candidate.c: 2 byte diffs. NOT byte-matched.

Committing the candidate would break the oracle. So src stays at HEAD; the
WIP entry records the legitimate floor + resume avenues. The next session
takes one of the `next_hypotheses` to close the 2-insn gap, then commits a
`cheat-cleanup:` if it lands.

## Related rules
- `inline-asm-policy.md` — the 3 HEAD cheats are all in the forbidden category
- `register-asm-pins.md` — the a1val + arg0 pins are diagnostic-only
- `inline-asm-injection.md` — the `asm("" : "=r"(x) : "0"(x))` barrier is in
  the cheat catalog
- `difficult-is-not-impossible.md` — keep grinding; the matching C exists

## Session 5 (2026-06-04) — root cause FULLY diagnosed at GCC-source level

Diagnostic path:
1. Dumped cc1's RTL through every pass via `-da` (see `tmp/rtl_dump.sh`,
   outputs at `tmp/rtl/code6cac_b.i.{rtl,jump,cse,loop,cse2,flow,combine,sched,lreg,greg,jump2,sched2,dbr}`).
2. Extracted per-pass `jump_insn` for the function via `tmp/rtl_extract.py`.
3. Located the flip: jump_insn 130 = `(ne v0 0) target=L168(done)` is stable through
   `sched2`, becomes `(eq v0 0) target=L146(next)` in `dbr` — and jump_insn 138's
   target swaps inversely (146 ↔ 168). The flip is from `dbr` (reorg.c).
4. Read `tools/gcc-2.7.2/reorg.c:3829-3851` — the unconditional-jump-after-
   conditional invert peephole. The swap fires iff
   `0 < mostly_true_jump(other, condition)`.
5. Read `mostly_true_jump` (reorg.c:1335-1428): `case NE: return 1` at line 1407
   ("NE tests are usually true"). Our condition `(ne v0 0)` hits this. Swap fires.

cc1psx confirmation (`tools/cc1psx_wrapper.sh` on `tmp/rtl/code6cac_b.i`,
output at `tmp/rtl/code6cac_b.psx.s`): cc1psx produces the SAME swap from
candidate.c. NOT a fork divergence per [[cc1psx-calibration-only]] — target
was compiled from a C source different from candidate.c.

## Session 5 — permuter found score 0, REJECTED by cheat-reviewer

Permuter setup at `permuter/csmdt/`:
- `target.s`: `tools/decomp-permuter/prelude.inc` (with `.set gp=64` removed) +
  `asm/funcs/cpu_check_same_dir_timer.s`. Function at offset 0 in target.o.
- `base.c`: preprocessed `src/code6cac_b.c` (with WIP candidate body applied),
  then `tools/decomp-permuter/strip_other_fns.py` to keep only this function.
- `compile.sh`: copied from `permuter/dbe4/compile.sh` (cc1 → prologue_fix →
  maspsx → as pipeline).

3-min 4-thread run found `permuter/csmdt/output-0-1/` at score 0. The find:
wraps the dispatch block in `do { ... } while (0);`. Mechanism: the C
front-end emits NOTE_INSN_LOOP_BEG for any `do-while` (regardless of
`while`-condition value), which makes `loop.c` set `LABEL_OUTSIDE_LOOP_P` on
`done:` (outside the do-while), which makes `mostly_true_jump` return -1 at
`reorg.c:1349` instead of reaching the NE-fallback at line 1407, suppressing
the swap.

**Cheat-reviewer verdict (2026-06-04): FAIL.** The do-while-0 wrap has zero
semantic purpose; would not be written by a human; justification is purely
GCC-internal; same intent as the original `asm volatile("")` barrier
(forbidden cheat-asm) with only the spelling changed. Form preserved at
`rejected/do_while_zero_wrapping.c` with full mechanism + reviewer evidence.

## What this means for the remaining gap

The 2-byte residual at masked insns 48 + 50 is structurally bounded by
this dilemma:

| Desired form | Required compile-time path |
|---|---|
| `bnez done; j next` (target's bytes — NE condition) | mostly_true_jump must return ≤ 0 for NE → requires LABEL_OUTSIDE_LOOP_P on `done` |
| LABEL_OUTSIDE_LOOP_P on done | loop.c must see a loop covering the dir-vs-val test → requires NOTE_INSN_LOOP_BEG |
| NOTE_INSN_LOOP_BEG from C | requires `for`/`while`/`do` construct; do-while-0 is a cheat (no semantic purpose); real do-while/while/for triggers LICM hoist of `0xFF` and `1` constants (+4 insns regression) |

The genuine search continues per `next_hypotheses[1]` (instrumented cc1 to
identify what makes target's reorg take a DIFFERENT path — maybe a different
condition shape that bypasses the NE heuristic without changing bytes), or
`next_hypotheses[2]` (subu+bltz form combined with combine-fold back to
slt+bnez, if combine reaches that depth on this RTL).

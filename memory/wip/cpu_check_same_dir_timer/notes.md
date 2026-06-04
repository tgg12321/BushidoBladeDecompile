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

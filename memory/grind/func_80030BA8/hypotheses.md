# Hypothesis ledger — func_80030BA8

## s2 [structural] — the two flagged constructs, resolved by measurement

| # | Hypothesis | Probe | Score | Verdict |
|---|---|---|---|---|
| H1 | Literal `0xF423F` in the compare still matches; sched1 lifts the lui/ori into the mult window on its own | drop `new_var`, inline the literal | **6** (87/87) | **KILLED** |
| H2 | A meaningfully-named *block-local* const reproduces the scheduling | `s32 range_sq = 0xF423F;` in the block that uses it | **0** | **CONFIRMED** — `new_var` fully retired |
| H3 | Literal `-1` at both sites breaks (holder is load-bearing) | drop `neg1`, inline `-1` | **14** (85/87) | **CONFIRMED** — the 2 missing insns are `sw`/`lw $s3` |
| H4 | A *block-scoped* named const reproduces the `$s3` allocation | `s32 empty_slot = -1;` inside the loop body | **4** (87/87) | **KILLED** — init re-executes per iteration; `li $s3,-1` lands in the `bnez` delay slot + prologue save order flips |
| H5 | A real `for` loop lets LICM hoist a body-local `-1` to the preheader, removing the function-scope holder entirely | rewrite goto-loop as `for (i=0;i<12;i++,p+=0x64)` with `continue` | **33** (89/87) | **KILLED** — LICM *does* hoist the `-1` (verified in the disasm), but it also hoists the `0xF423F` lui/ori OUT of the mult latency window (+ a base-address). Loop notes enable LICM for *all* invariants at once. |
| H6 | The sentinel and the `return -1;` value are the same -1; one variable for both is unimpeachably semantic | `s32 none = -1;` also used at both returns | **1** (87/87) | **KILLED** — shared exit emits `move $v0,$s3`; target emits `addiu $v0,$zero,-1`. The original's sentinel is DISTINCT from its return value. |

## The structural fixpoint (why the two constructs are coupled)

The goto-loop carries no `NOTE_INSN_LOOP`, so LICM never runs. That is *load-bearing
in both directions*:
  - it is why `0xF423F` stays inside the loop, in the mult/mflo latency window (target);
  - it is also why there is no preheader for a `-1` to be hoisted into, which forces
    the sentinel to be a **function-scope** local initialized once before the loop.
Any real loop construct (`for`, `while(1)`+break) re-enables LICM and breaks the
first to fix the second. H5 measured exactly this trade.

## Where this leaves the audit findings

- **`new_var` — RESOLVED, no exception needed.** Retired to `s32 range_sq = 0xF423F;`,
  block-local, meaningfully named, initialized at its declaration, read by the very
  next statement. Score 0. The function-scope lifetime the audit flagged was never
  load-bearing; only the single-pseudo identity is.
- **`neg1` — renamed to `empty_slot`, still function-scope. Score 0.** Both of the
  Judge's prescribed remediations (literal / block-scoped const) are measured
  IMPOSSIBLE (H3, H4), as are the two levers derived this session (H5, H6). The
  surviving form is a constant-holder local living across a `jal`, i.e. the
  [[named-local-fake-exception]] family. **Not self-approved — ruling requested.**

# func_8003B10C — WIP (CSE-defeat plateau; identical-symbol read can't duplicate in pure C)

## TL;DR (2026-06-14)
HEAD carries 12 regfix rules (4 deletes + 6 inserts + 1 subst) whose net effect
is to INJECT a duplicated `lh %lo(D_80101ED6)($1)` (with `lui %hi`/`addu $1,$16`)
into BOTH arms of the `if (D_800A38DC == 5)` branch. Unpinned/un-ruled masked
floor = **10** and NO pure-C form found this session beats it. This is a
[[split-read-defeats-hoist]] case where the dup-read lever does NOT apply
cleanly: the shared read is the SAME symbol (`D_80101ED6`) in both arms, so
there is no "symbol-known arm" to index directly — GCC CSE-merges the identical
memory read no matter where it is written, and the only thing that would force
the duplication is the FORBIDDEN volatile coercion. Per the rule, this makes
the function not pure-C-matchable; block for canonical-asm authorization or a
confirmed plateau decision.

## The gap (the 10)
Target duplicates `*(s16*)((u8*)&D_80101ED6 + s0)` inside each branch (leaves it
in v1, caller-saved) and the shared tail is just `addu v0,v0,a0; addu v0,v0,v1;
lbu`. Pure-C GCC HOISTS that identical read to ONE shared load after the merge
(CSE), changing the branch shapes + the tail. Also a 1-insn operand-order diff
(`addu v0,a0,v0` vs target `addu v0,v0,a0`).

## Ruled out this session (all >= floor 10, none a cheat)
- per-branch `ed6` variable (separate read in each arm) — floor 32; GCC
  promotes ed6 to a callee-saved reg (sp grows -32 -> -40, extra s-reg save).
- per-branch full pointer `p = &TBL + idx*N + ed6read` — floor 26.
- per-branch pointer `p = &TBL + idx*N`, ed6 read in shared tail — floor 19
  (still hoists; basically baseline + a spill).
- fold ed6 into v0 per branch (`v0 = idx*N + ed6read`) — floor 26.

## Why this needs volatile (forbidden) or canonical
The dup-read structural lever defeats hoisting only when the duplicated reads
target DIFFERENT bases (direct symbol vs pointer) per arm. Here both arms read
the SAME `D_80101ED6` global, so CSE re-merges; the regfix inserts exist
precisely because pure C cannot keep the read duplicated. Marking D_80101ED6
volatile would force re-reads but is a forbidden codegen-coercion cheat
(volatile_cheats detector + split-read-defeats-hoist #3 FORBIDDEN 2026-05-31).

## Next hypotheses
- Canonical-asm authorization (the 6 inserts reconstruct duplicated absolute
  loads GCC's CSE removes — genuine no-pure-C-form construct for this shape).
- Permuter from baseline (unlikely: CSE on identical reads is a fixed-shape
  artifact the permuter can't restructure away).

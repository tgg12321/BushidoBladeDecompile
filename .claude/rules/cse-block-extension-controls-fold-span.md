---
name: cse-block-extension-controls-fold-span
description: "cse1's basic block does NOT stop at a join label — cse_end_of_basic_block (cse.c:8102-8184) extends through a conditional branch whose target label has LABEL_NUSES==1, so CSE merges address pseudos and forwards stores across joins the target keeps separate. There are exactly three escapes; the only free one is an if/else whose arm ends in a jump+BARRIER."
paths: [".claude/rules/cse-block-extension-controls-fold-span.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
metadata:
  type: reference
---

# CSE folds across join labels — `cse_end_of_basic_block`'s extension test

## Symptom (two shapes, same mechanism)

- **You have too FEW instructions.** The target rematerializes an address /
  reloads a value in each of several arms; your build computes it once and
  forwards it. Your `.cse` dump shows the per-arm pseudos merged into one
  (`insn 27/55/83 DELETED`, all four pseudos merged into `reg 74`) and the
  per-arm reloads deleted or rewritten.
- **You have too MANY instructions.** A value the target folds is kept separate
  in your build because a branch shape you chose ended the cse block early.

Both are the same knob: **where cse1's basic block ends**. The near-universal
wrong assumption — worth naming because three functions each burned a session on
it — is that *a join label ends the block*. It does not.

## Mechanism (`tools/gcc-2.7.2/cse.c:8102-8184`, read verbatim)

`cse_end_of_basic_block` does not stop at a `CODE_LABEL`. It **extends** the
block past a conditional branch (the follow-jumps / skip-blocks arm) whenever
`LABEL_NUSES (JUMP_LABEL (p)) == 1` — a singly-referenced target label is
followed, not treated as a boundary. Under a target-shaped layout **both**
branches get followed (`Processing block from 2 to 94` in the dump), so
everything in both arms and past the join is one CSE region.

Its escapes are **exactly three** (enumerated verbatim at cse.c:8092-8184,
func_80034F88 [s9]):

- **(a)** `LABEL_NUSES (JUMP_LABEL (p)) != 1` — the label is referenced more than
  once (e.g. the `&&` short-circuit boundary);
- **(b)** the backward scan (8109-8114) hits a `CODE_LABEL` with
  `LABEL_NUSES != 0` immediately before the join. (The scan skips ordinary NOTEs
  and NUSES-0 labels, and breaks on `NOTE_INSN_LOOP_END` / `NOTE_INSN_SETJMP`.)
- **(c)** the forward `no_labels_between_p` check (8168-8172) finds any
  `CODE_LABEL` between the branch and the arm's last insn.

## The one escape that is free — jump+BARRIER-terminated arm

Every one of (a)/(b)/(c) as usually reached costs **at least one extra jump or
branch instruction**, or an unreferenced label — which is the forbidden dead-goto
label pad by another name. A C construct that emits a `CODE_LABEL` with no jump
does not exist outside the computed-goto (`&&label`) family, which is the same
pad again.

The exception, and the lever to reach for first:

> Spell the conditional as a real **`if/else`** rather than "initialise to a
> default, then conditionally overwrite".

An `else` arm ends in an unconditional jump followed by a **BARRIER**, and
`cse_end_of_basic_block` cannot around-extend past the join in that shape — the
block is *followed* rather than *ended*, and the fold stops. This is ordinary C
with a semantic reading ("one of two values"), not a coercion.

**func_8003B9D0, region B: a 7-instruction shortfall closed entirely** by
respelling two flag-selected argument initialisations from
`x = -1; if (c) x = v;` to `if (c) x = v; else x = -1;`. With the same code but
the test left as a plain `if`, the fold comes straight back (measured 10,
build_insns 187).

## What is measured dead (do not re-probe)

- **The `&&` boundary** (raising `LABEL_NUSES` to 2) is the cheapest reachable
  spelling of escape (a); it is measured and **net-negative at every count of
  boundaries** (+2, +13, +15 instructions) — func_80034F88.
- **`thread_jumps` cannot supply a second `LABEL_REF`** when the join labels are
  followed by different code; no redirection is available — func_80034F88.
- **Carrier copies** (a same-region `SImode` copy of the value, hoping to break
  the merge) are reverted by `cse1`'s `canon_reg` **before combine** ever runs —
  func_80048AD0 [s4], banked at `rejected/carrier-copy-cse-canon-reverted.c`.

## A neighbouring trap: the fold may not be cse's at all

func_80048AD0 [s4] measured cse1/cse2 folding a truncation in **no** layout
(P1 and P8 both). The fold was **combine's**:
`expand_compound_operation -> simplify_and_const_int -> nonzero_bits`. Confirm
which pass owns the fold in the `.cse` vs `.combine` dumps before choosing a
lever — a cse-shaped lever cannot move a combine fold.

## Diagnosis recipe

```
cc1 ... -da        # gives <file>.i.cse and <file>.i.combine
```

Look for `Processing block from N to M` in the `.cse` dump: the span tells you
directly whether the block extended past your join. Then check whether the
insns you expect are `DELETED` in `.cse` (cse's fold) or survive to `.combine`.

## Confirmed occurrences

| function | shape | ledger |
|---|---|---|
| `func_80034F88` | three join labels, address pseudos merged; escape enumeration + `&&`/`thread_jumps` kills | `memory/grind/func_80034F88/evidence.md:205-215, 1060-1075` |
| `func_8003B9D0` | region B **closed** by the if/else + BARRIER lever; region A residual is the same mechanism at a hoisted pointer | `memory/grind/func_8003B9D0/evidence.md:120-132, 236-246` |
| `func_80048AD0` | both branches followed under target layout (overturns an earlier "call block is off cse1's path" assumption); fold turns out to be combine's | `memory/grind/func_80048AD0/evidence.md` [s4] |

## Related
- [[exit-path-return-set-cse-join]] — the return-value case of the same join
  question.
- [[param-reuse-base-copy-cse-canon]] — `canon_reg`'s copy reversion, the
  mechanism behind the dead carrier-copy family above.
- [[split-read-defeats-hoist]] / [[defeat-combine-symbol-fold]] — the loop.c and
  combine.c folds; distinguish by which dump deletes the insn.
- [[duplicated-statement-into-arms]] — the sanctioned way to put work back into
  both arms once the block boundary is where you want it.

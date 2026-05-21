---
name: regfix-subst-multi-splice
paths: ["regfix.txt", "asmfix.txt", "regfix_stage2.txt", "tools/regfix*.py"]
description: "Two regfix ops added 2026-05-13 for K-to-N instruction transformations. `subst_multi` does 1-to-N regex-driven; `splice` does K-to-N range replacement. Closes the previous gap where `subst` (1-to-1) + `insert_after` couldn't reliably handle multi-instruction GCC-vs-target divergences like `blez` vs `slt+beq`."
metadata:
  type: reference
---


# regfix subst_multi and splice — K-to-N instruction transformations

> **TIER-4 note ([[tier4-sota-standard]], 2026-05-21):** these ops are debt — **do
> not author new `subst_multi`/`splice`/regfix rules.** `audit_asm_cheats.py
> --check-new` hard-blocks net-new regfix at commit. This doc is for reading and
> RETIRING existing rules; the end state is pure C (or canonical asm).

## When to reach for these

Both ops solve cases where `subst` (1-to-1) and `insert_after` chains can't cleanly express a multi-line transformation. Common signatures:

| Pattern | Op | Example |
|---|---|---|
| GCC emits 1 insn, target emits 2+ for same semantic | `subst_multi` | `blez $rN, .L` (mine) vs `slt $rD, $rA, $rN; beq $rD, $zero, .L` (target) |
| GCC's 3-insn `result-via-tN` idiom collapses to 2-insn target form | `splice` | mine's `li $tN, 1; addu $vM, $tN, $0; addu $vM, $tN, $0` → target's `addu $vM, $0, $0; addiu $vM, $0, 1` |
| Specific load-delay nop needs to move position relative to substituted block | `subst_multi` (regex-driven) | mine's `lw; nop; bgez` vs target's `lw; nop; bgez` where the nop's slot semantics differ |

**Trigger for tool use:** you're chaining `subst @ N` + `insert_after @ N` (or `delete @ N` + `insert @ N+1`) and the result races with a maspsx-emitted `nop  # DEBUG: branch/jump` or `nop  # DEBUG: Reuse of $X` line. The composed rules can't address the debug nop's position. `subst_multi`/`splice` do the transformation atomically, leaving the debug nops in their original positions relative to surrounding instructions.

## subst_multi

```
func_NAME: subst_multi "pattern" "line1" "line2" [...] @ idx
```

- **Semantics:** at maspsx idx N, regex-match `pattern`. If matched, replace the line with multiple output lines. First output line keeps idx=N; subsequent outputs get idx=None (synthesized, can't be regfix targets).
- **Indentation:** taken from the original line. Replacement strings are `lstrip`ped; whitespace is owned by regfix.
- **Length:** adds `(K - 1)` lines to the maspsx output, where K is the number of replacement strings. Account for this in your balance — typically pair with a `delete @ M` elsewhere if the function size matters.
- **Pattern tip:** use `.*` to consume any trailing label/operand if you want a clean replacement. A pattern that only consumes part of the line will leave the unmatched portion in place, which usually breaks the assembler.

CLI:
```
bash tools/dc.sh add-regfix subst_multi <func> "<pattern>" "<line1>" "<line2>" [...] --idx N
```

Pre-validates that the pattern matches the line at idx N before append.

## splice

```
func_NAME: splice start..end "line1" "line2" [...]
```

- **Semantics:** removes every instruction-bearing line in `[start, end]` (inclusive) and emits the listed replacement strings in their place. K input lines → N output lines; K and N may differ.
- **Non-instruction lines preserved:** labels (`.L<N>:`), directives (`.set noat`), and comments within the range are left in their original positions. Only lines with a non-None idx are touched.
- **Indentation:** taken from the line at `start`.
- **No regex:** splice is positional. Use this when the WHOLE block needs to go and a regex per line would be tedious.

CLI:
```
bash tools/dc.sh add-regfix splice <func> "<line1>" "<line2>" [...] --range N-M
```

## Phase ordering

Both ops run during the regfix phase pipeline:

```
1.   register swaps (original idx)
1.5  subst        (original idx, 1-to-1)
1.55 subst_multi  (original idx, 1-to-N)   <-- NEW
1.65 splice       (original idx, K-to-N)   <-- NEW
1.7  fill_delay
1.8  drain_delay
2.   delete       (original idx)
3.   insert / insert_after (shifts idx)
3.6  insert_label
4.   reorder
```

Other rules at @ M for M > N continue to use ORIGINAL maspsx indices — subst_multi/splice don't renumber prior indices. The output stream gets more/fewer lines, but rule indexing is stable.

## Worked example: func_8002D320 (action_defense_success_normal)

This is the recipe that drove the design. After 13 prior regfix rules converted mine's `$t0`-based result flow to target's `$v0`-direct flow, the function had 6 remaining diffs concentrated at maspsx idx 94, where mine emits `blez $v1, .L273` (1 insn) and target emits `slt $v0, $a2, $v1; beq $v0, $0, .L8002D4B8` (2 insns).

Replacing the v13 rule `subst "blez\s+\$2," "blez\t$3," @ 94` with:

```
func_8002D320: subst_multi "blez\s+\$2,\.L273" "slt\t$2,$6,$3" "beq\t$2,$0,.L273" @ 94
```

plus a balancing `delete @ 88` (since `subst_multi` adds +1 line) brought the function from 6 diffs to MATCH on first build. See `tmp/add_regfix_8002D320_subst_multi_v2.py` for the full recipe at the time of commit.

## Gotcha: length accounting

`subst_multi` and `splice` change the line count of the maspsx output. Watch for cascade branch-offset diffs (mine's `bne` targets shifting by ±1 relative to target's). Common patterns:

- **subst_multi @ N (1 → K)**: adds `K - 1` lines. Pair with `delete @ M` to balance if downstream branches use absolute labels.
- **splice start..end (K → N)**: net change is `N - K`. May be negative (collapse) or positive (expand).

The build trajectory (`dc.sh build-active <func>` prints recent diff counts) will surface a length-mismatch cascade quickly — if you go from 0 structural to 5+ branch-offset diffs after adding a `subst_multi`/`splice` rule, you have a length imbalance.

## Tests

`tmp/test_subst_multi.py` contains 7 unit tests covering:
- Basic 1-to-2 substitution
- Non-matching pattern (warns and leaves line alone)
- 1-to-3 substitution
- splice 3-to-1 collapse
- splice 1-to-3 expansion
- subst_multi composed with existing subst
- subst_multi composed with insert_after (the insert lands AFTER the first output line of the multi-block — to insert after the whole block, use `insert_after @ (next original idx)`)

Run via `python3 tmp/test_subst_multi.py`. Add to this test file when you discover new edge cases.

---
name: lost-codegen-insert-cheat
paths: ["regfix.txt", "src/*.c"]
description: "`insert_after \\"addu $sN, $0, $zero\\" @ idx` and similar single-instruction insert/insert_after/insert_before rules in regfix.txt are asm injection — they restore instructions GCC's constant-prop / dead-store-eliminator ate. The audit_asm_cheats hook now blocks new ones at commit time. Fix the C structure instead (usually via [[shared-end-label]])."
metadata:
  type: audit
---


## Why: detected as cheat
**Rule:** any new `insert`/`insert_after`/`insert_before` regfix rule whose body matches `addu $rN, $0, $zero`, `addu $rN, $rM, $zero`, or `addu $rN, $zero, $rM` is blocked by the commit hook (`audit_asm_cheats.py --check-new`).

**How to apply:** if you see one of these rules as the only way to close a 1-2-instruction shortfall, the C source is wrong, not the regfix mechanism. Restructure the C.

## What it looks like

```
func_XXX: insert_after "addu\t$18,$0,$zero" @ 52    # restore s2 = 0
func_XXX: insert_after "addu\t$18,$0,$zero" @ 80    # restore s2 = 0
```

These two rules inject `addu $s2, $zero, $zero` (= `s2 = 0;` in C) at maspsx idx 53 and 81. The instructions are missing because GCC's constant-propagation removed them — every path returning $s2 in those cases returned 0, so GCC folded `return s2;` to `return 0;` and dead-stored the assignment.

The instructions ARE in target.s. The C source ASKED for them (`s2 = 0;` is written explicitly). GCC dropped them. Injecting them via regfix is asm-from-the-rule, not asm-from-C-compilation.

## Why this counts as a cheat (the project's pure-C bar)

The project's standing rule (commit `4937f0f` revert message): "no asm workarounds that bypass the canonical pure-C source." A 38-line splice and a 2-line `insert_after` are different in scale but identical in kind — bytes that came from rule text, not from compilation.

Threshold rationale: `splice >= 30 lines` catches the bridge-equivalent case; `cumulative >= 60 lines` catches split-evasion. Single-instruction lost-codegen inserts are below both thresholds and slipped past the audit until 2026-05-16. They're now tracked individually.

## How to fix in C instead

The most common cause is multi-return functions where GCC constant-folds per-case returns. The fix is documented in [[shared-end-label]] — restructure to `goto end; ... end: return s2;` so GCC can't see each case's return as a constant.

Other techniques that have NOT worked for this pattern (verified in func_80077B30 session, 2026-05-16):

- `register T x asm("$N")` register pin alone — GCC still constant-folds (pin doesn't prevent value analysis)
- `__asm__ volatile ("" : "=r"(s2) : "0"(s2))` no-op barrier — emits 1 extra instruction even with the pin (constraint-driven move)
- `__asm__ volatile ("" ::: "memory")` memory clobber — disturbs register allocation, breaks other things
- Per-case `do { } while (0)` wrapping — collapses to nothing useful
- Order-shuffling if-checks within cases — doesn't change the constant-fold

The shared-end-label restructure is the one that worked. Try it FIRST before any other approach when you see this pattern.

## Audit output

```
$ python3 tools/audit_asm_cheats.py --summary
Cheats:   N unauthorized (..., M func(s) w/ lost-codegen inserts (K insn(s)))
```

```
$ python3 tools/audit_asm_cheats.py --func func_XXX
UNAUTHORIZED: func_XXX has 2 lost-codegen single-instruction insert rule(s) in regfix.txt:
  line  5060  insert_after    'addu\t$18,$0,$zero'
  line  5061  insert_after    'addu\t$18,$0,$zero'
These inject MIPS instructions that didn't come from C compilation ...
```

```
$ git commit -m "..."
ASM-CHEAT GUARD: new unauthorized asm-cheat patterns since HEAD:
  Lost-codegen insert(s) for func_XXX: HEAD had 0, now 2 (+2). ...
```

## Existing pre-cleanup population

29 functions had lost-codegen inserts at the time the audit was added (commit `cc46451`, 2026-05-16) — 39 total instructions. These are grandfathered: `--check-new` only blocks NEW additions. They're tracked for eventual cleanup via the shared-end-label recipe or other structure changes.

`audit_asm_cheats.py --all` lists them with their regfix line numbers.

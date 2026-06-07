---
name: canonical-asm-authorization-recipe
description: How to author a whole-body __asm__("glabel ...") block for canonical-asm authorization — three lessons from the 2026-06-07 A-category pass (commit 6e0476f0 documents the retry after a failed first attempt at 8719adea)
paths: ["src/*.c", "inline_asm_canonical.txt", "regfix.txt", "asmfix.txt"]
---

# Writing a whole-body `__asm__("glabel ...")` block

This rule captures three concrete failures-and-fixes from the 2026-06-07
A-category authorization pass (5 functions: `func_8004DDB4`, `func_8006BD28`,
`save_vc_ctrl`, `InitFadePanel`, `func_80044010`). One of the five required a
revert + retry; the other four worked first-try by following these rules.

## The three load-bearing technical rules

### 1. Use SYMBOL form for globals, NOT `%gp_rel(X)($gp)`

**`asm/funcs/*.s` files use `lw $v0, %gp_rel(D_xxx)($gp)`** — that's splat's
DISASSEMBLY output, derived from byte-decoding the linked binary. It is NOT
directly assemblable by `as` inside an `__asm__` block:

```
{standard input}: Error: invalid operands `lui $at,%hi(%gp_rel)'
{standard input}: Error: operand 3 must be constant `addu $at,$at,D_800A34FC'
```

**The fix**: write the symbol-form load — `lw $v0, D_xxx`. Maspsx converts
this to `%gp_rel(D_xxx)($gp)` if the symbol is in `sdata_syms.txt` for
your function's file, producing the same target bytes:

```c
/* ❌ WRONG — verbatim splat output, doesn't assemble */
"    lw     $v0, %gp_rel(D_800A34FC)($gp)\n"

/* ✓ RIGHT — maspsx converts to gp_rel for sdata symbols */
"    lw     $v0, D_800A34FC\n"
```

For symbols NOT in `sdata_syms.txt`, use the 2-insn `lui %hi/%lo` form:
`lui $at, %hi(D_xxx); lw $v0, %lo(D_xxx)($at)`. Verify per-symbol BEFORE
writing — wrong choice produces wrong bytes.

The 2026-06-07 A2 failure (`a4b1fe2f`, reverted by `8719adea`) was caused by
copying the `%gp_rel` form directly from `asm/funcs/func_8006BD28.s`. The
retry (`6e0476f0`) used symbol form and matched oracle first-build.

### 2. Use NAMED LABELS for intra-function branch targets, NOT `.L<N>` numbers

cc1 maintains a global per-TU counter for auto-generated `.L<N>` labels.
Numbers are assigned in source order, top to bottom. Writing literal
`.L<N>:` in your `__asm__` block risks colliding with a number cc1 emits
for another function in the same `.c` file.

**The fix**: use a function-scoped descriptive label name:

```c
/* ❌ RISKY — collides with cc1's auto-allocation */
".L987:\n"  /* hopes cc1 didn't already emit this number elsewhere */

/* ✓ SAFE — unique by function name + role */
".L_func_8006BD28_loop:\n"
".L_func_8006BD28_end:\n"
```

A5's worker (`19d75899`, `func_80044010`) used this pattern and matched
first-try. A4's worker (`81fe5275`, `InitFadePanel`) had to fix 4 sibling
asmfix rules because removing the C body shifted cc1's `.L<N>` counter — but
the authorized function's own labels were named, so they weren't part of the
cascade.

### 3. Check + fix the LABEL-SHIFT CASCADE in sibling rules

Removing a function's C body drops the auto-`.L<N>` labels cc1 used to emit
for it (typically 1-15 labels for a non-trivial function). This shifts EVERY
subsequent function's `.L<N>` numbers DOWN by the dropped count in the SAME
`.c` file.

**Any sibling whose asmfix/regfix rule anchors on a hardcoded `.L<N>`
literal will BREAK** when the labels shift. Confirmed examples from the A-pass:
- A3 (save_vc_ctrl removal, text1a.c): broke `saTan4FireDisp`'s asmfix
  anchor `.L279`; fixed to `.L272` (delta −7).
- A4 (InitFadePanel removal, text1b.c): broke 4 siblings —
  `func_80056CB8` (renames `.L321→.L320`, `.L320→.L319`),
  `func_8006BD28` (7 rename source labels each −1),
  `func_80070C70` (2 renames),
  `func_80074B18` (1 regfix subst replacement).
- A2 (func_8006BD28 removal, text1b.c): broke 2 siblings — `func_80070C70`
  (`.L1001→.L987`, `.L992→.L978`, delta −14), `func_80074B18`
  (`.L1090→.L1076`).

**Process**:
1. After writing the `__asm__` block and removing rules, run
   `verify-oracle --rebuild`.
2. If SHA1 mismatches, diff your `build/src/<file>.o` against oracle to
   find which function's bytes drifted. The drift will be in a sibling
   function later in source order than the authorized one.
3. Use `bash tmp/probe_func_labels.sh <sibling>` to find the new
   `.L<N>` mapping vs the old.
4. Update affected sibling asmfix/regfix rules with new label numbers
   (or migrate to `{lbl#N}` function-local slots per
   `[[global-label-drift-sibling-cheat]]` — drift-robust solution).
5. Re-verify oracle until SHA1 matches.

**Do NOT commit a broken oracle.** If you cannot get SHA1 to match within
your effort budget, REVERT all edits and report honestly. A2's first
attempt committed broken bytes; the retry succeeded only after explicit
cascade-handling.

## Supporting rules

- **Decimal offsets in base-displacement**: `0(sp)`, `4(sp)` — NOT
  `0x0($sp)`. Maspsx parses base 10 for `disp($reg)` form.
  Hex IMMEDIATES are fine (`0xFF`). See `[[packed-multiply-cluster]]`.

- **BOTH TAB and SPACE-form `.set` directives** per
  `[[maspsx-noreorder-stripping]]`: maspsx silently drops TAB form; SPACE
  form reaches `as`. You need both.

```c
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"            /* SPACE form reaches as */
    ".set noreorder\n"
    "glabel func_name\n"
    "    ...body...\n"
    "    jr     $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
```

- **Strip ALL cheats from C-source decls** alongside the body replacement:
  register-asm pins, `volatile s32 sp_pad` frame coercion, function-specific
  typedefs that are now unused, extern declarations only the removed body
  referenced.

- **Remove ALL `func_NAME:` rules from regfix.txt + asmfix.txt** when
  authorizing — whole-body canonical-asm REPLACES the rule-driven bridge.

## Add-to-inline_asm_canonical.txt format

Match the existing entries. Include a 1-2 line rationale citing:
- The specific hand-coded signal(s) per `[[hand-coded-asm-recognition]]`
  (S1/S2/S6/S7/S8) — e.g. "vestigial frame + unfilled jr ra delay slot"
- User authorization date + category — "User authorized 2026-06-07
  (canonical-asm A category)"

## Verification gate (mandatory before commit)

```pwsh
& tools/eng.ps1 verify-oracle --rebuild
```

Must return `build_matches: True` with SHA1 ==
`62efab4f73f992798c43e8c730aa43baa10bb4fa`. If not — fix or revert.

## Then queue done + commit

```pwsh
& tools/eng.ps1 queue done <func>
git commit -F tmp/msg.txt  # subject: auth: <func> (<file>.c) — COMPLETED-INLINE-ASM-CANONICAL
```

## Related

- `[[hand-coded-asm-recognition]]` — the signals that qualify a function
  for canonical-asm authorization in the first place
- `[[maspsx-noreorder-stripping]]` — the TAB+SPACE `.set` directive pattern
- `[[global-label-drift-sibling-cheat]]` — `{lbl#N}` migration path for
  drift-robust sibling rules (preferable to manual `.L<N>` updates)
- `[[inline-asm-allowed]]` — categorization of what counts as canonical
  vs cheat-asm
- `[[review-discipline-before-commit]]` — cheat-reviewer required for
  `auth:` commits

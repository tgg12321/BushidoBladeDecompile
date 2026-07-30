# Evidence bank — mk_g2l

Function: `mk_g2l` (aka camera_SetMatrix per the regfix comment), `src/code6cac_b.c:402`.
Target asm: `asm/funcs/mk_g2l.s` (51 instructions, one jal to `func_80032854`).
Canonical gate: **C** (asm_insns 0, distance 17 <= 50 — pure-C target).

## Session 1 (recon, 2026-07-29) — floor 17 -> 7

### The gap is 100% register allocation
`sandbox --disable all`: target_insns 51, build_insns 51 at every floor measured
this session. No control-flow difference, no insn-count difference — every diff
is a register name (plus, at floor 7, one 2-insn load-ORDER swap). The 10 regfix
rules for this function (regfix.txt:242-257) are all register-rename /
operand-order paperwork (`$3 <-> $8`, `$3 <-> $4 @ 13-29`, four
`subst "$4," "$2,"`, `subst "4($4)" "4($2)"`, `reorder 4,3 @ 3-4`) — consistent:
the rules ARE this register rotation.

### Diff harness (reusable)
`tmp/grind/mk_g2l/s1/dis.py [obj]` — objdumps the sandbox object, extracts
`mk_g2l`, index-aligns it against `asm/funcs/mk_g2l.s`, normalizes
`move`/`li` <-> `addu`/`addiu` and decimal-vs-hex immediates, prints starred
diffs. KNOWN COSMETIC BUG: its hex regex is lowercase-only, so an operand with
an uppercase hex digit (`0x2B`, `0x8E`) is reported as a diff when it is not.
The engine score is authoritative (harness says 9, engine says 7 — the two extra
lines are exactly those false positives).

### Baseline register map (floor 17 = the committed HEAD form)
| value | our build | target |
|---|---|---|
| `ptr` (2nd param) | `$v1` | `$t0` |
| `stack_v1` (6th param, `0x2C(sp)`) | `$t0` | `$v1` |
| obj (`*ptr`), field-check region | `$a0` | `$v1` |
| obj (`*ptr`), the four call_with_field4 paths | `$a0` | `$v0` |
| `f86` (the `0x86`-field compare value) | `$t0` | `$a0` |
| `cmd` / `arg_a2` / `a3` / `f8E` / `f88` | a1 / a2 / a3 / v0 / v0 | same (match) |

### THE LEVER (17 -> 7): hold the 0x86 compare value in the dead `a0` param
`s16 f86 = *(s16 *)(obj + 0x86);` -> `a0 = *(s16 *)(obj + 0x86);`. The parameter
`a0` is dead after the `if (a0 == 1) return;` early-out, and the function already
assigns the call's first argument into `a0`. **Floor 17 -> 7 in one edit**, and it
fixed FOUR register families at once: `ptr` v1 -> t0 (match), field-check obj
a0 -> v1 (match), `stack_v1` -> v1 (match — it shares `$v1` with the field-check
obj exactly as target does, disjoint live ranges), and the compare value itself
-> a0 (match). This cascade is what the 10 regfix rules were papering over.
POLICY: the spelling is parameter reuse — see candidate.c's header and
hypotheses.md H1. It is NOT a dead store and NOT a pin, but the only reason to
write it this way is codegen, so it needs either a more natural equivalent
spelling (H1) or the variable-reuse carve-out's exhaustion + `/* FAKE */`
annotation before it can be accepted.

### Residual at floor 7 (two families)
1. **2 diffs — stack-param load order.** Target `lw $a2,0x28($sp)` then
   `lw $v1,0x2C($sp)` (stack_a2 before stack_v1); we emit stack_v1 first.
   Both registers are already correct; only the order differs.
2. **5 diffs — the call-path obj register.** Target `lw $v0,0($t0)` x4 plus the
   shared `lh $a0,0x4($v0)`; we put that pseudo in `$v1` (single-variable form)
   or `$a0` (two-local form). Everything else in the function matches.

### RTL ground truth (cc1 `-da` — the mandated Step-0)
Recipe: `tmp/grind/mk_g2l/s1/greg.sh` (cpp with the Makefile's CPP_FLAGS +
CPP_DEFS, then `cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1
-mno-abicalls -fno-builtin -w -da`). Artifacts in `tmp/grind/mk_g2l/s1/`:
`base.i.greg`, `base.i.lreg`, `base.i.sched`, plus the extracts `greg_mk.txt`
and `lreg_mk.txt`. NB the TU emits many benign "conflicting types" warnings —
pre-existing, not caused by this work.

For the two-local (`chk_obj` split) form at floor 7, the `.greg` section reads:

```
;; 8 regs to allocate: 72 79 74 78 73 77 75 80      <- priority order
;; 78 conflicts: 74 78 80 29
;; 78 preferences: 4 5
;; Register dispositions: 72 in 4  73 in 8  74 in 5  75 in 7  77 in 3
;;                        78 in 4  79 in 3  80 in 6  82/86/91/95 in 2
```

Pseudo map (from `lreg_mk.txt`): 72 = the a0-held compare value + the call's
first arg; 73 = `ptr`; 74 = `cmd`; 75 = `a3`; 77 = `arg_v1`; 78 = the call-path
obj; 79 = the field-check obj; 80 = `arg_a2`; 82/86/91/95 = f8E / f88 / slti
temps (all in `$v0`, matching target).

**Why 78 misses `$v0`:** 78 does NOT conflict with hard reg 2 (`$v0`) — v0 is
available to it — but 78 carries a hard-reg *preference* for 4/5 (`$a0`/`$a1`),
and a preference beats the alloc-order fallback. With no preference it would take
the first free register in the default REG_ALLOC_ORDER (mips.h defines no
REG_ALLOC_ORDER, so the order is 0,1,2,... and the first allocatable is
`$v0` = 2) — exactly the target register.

**Where the preference comes from:** `global.c:797-840` `expand_preferences()`
IORs the hard-reg preference sets of two allocnos together whenever an insn's
`single_set` DEST is one allocno and the insn carries a `REG_DEAD` note for the
other, provided the two do not conflict. Insn 153 is
`(set (reg 72) (sign_extend (mem (plus (reg 78) 4))))` with `REG_DEAD (reg 78)`
— the shared `call_with_field4` load. 72 legitimately prefers `$a0` (it is the
a0 param AND the call's first argument) and that {4,5} set is merged straight
into 78. So the call-path obj inherits the ARGUMENT register's preference from
the very insn that consumes it.

Allocno priority (`global.c:604-625 allocno_compare`) is
`floor_log2(n_refs) * n_refs / live_length * 10000 * size`, descending. Useful
for reasoning about who picks first, but note the residual is NOT an ordering
problem: 78 takes `$a0` because it *prefers* it, not because `$v0` was taken.

### Measured KILLS this session (do not re-run)
- **Two-local obj split** (`chk_obj` for the field-check region, `v1_obj` for the
  call paths): at floor 17 it produced BYTE-IDENTICAL output (fully inert). At
  floor 7 it is also score-inert (still 7) but changes WHICH wrong register the
  call-path obj gets — split -> `$a0`, single-variable -> `$v1`. Target wants
  `$v0`, so neither is right; the single-variable form is kept as the candidate
  because it is the simpler and more natural C. Splitting is not the lever.
- **Dropping the `arg_a2` / `arg_v1` param-alias locals** (using `stack_a2` /
  `stack_v1` directly, per [[drop-param-alias-local]]): floor 17 -> **18**,
  strictly worse. The alias locals are load-bearing; keep them.

- [s1] canonical: verdict C, asm_insns 0, total 51, distance 17 (pure-C target).

- [s1] Honest floor moved 17 -> 7 this session; build_insns == target_insns == 51 throughout, so the residual is purely register naming plus one 2-insn load-order swap.

- [s1] Residual family A (2 diffs): target loads stack_a2 from 0x28(sp) BEFORE stack_v1 from 0x2C(sp); we emit the reverse order. Both registers already match.

- [s1] Residual family B (5 diffs): the call-path obj pseudo must land in $v0 (target 'lw $v0,0($t0)' x4 + shared 'lh $a0,0x4($v0)'); we get $v1 (single-variable form) or $a0 (two-local form).

- [s1] cc1 -da RTL dump for the TU is reproducible via tmp/grind/mk_g2l/s1/greg.sh; pseudo map from base.i.lreg: 72 = a0-held compare value + call's first arg, 73 = ptr, 74 = cmd, 75 = a3, 77 = arg_v1, 78 = call-path obj, 79 = field-check obj, 80 = arg_a2, 82/86/91/95 = f8E/f88/slti temps (all $v0, matching).

- [s1] base.i.greg (two-local form, floor 7): ';; 8 regs to allocate: 72 79 74 78 73 77 75 80', ';; 78 conflicts: 74 78 80 29', ';; 78 preferences: 4 5', dispositions '72 in 4  73 in 8  74 in 5  75 in 7  77 in 3  78 in 4  79 in 3  80 in 6'.

- [s1] ROOT CAUSE of residual family B: pseudo 78 does NOT conflict with hard reg 2 ($v0) — v0 is available — but it carries a hard-reg PREFERENCE for 4/5 ($a0/$a1) which beats the alloc-order fallback. mips.h defines no REG_ALLOC_ORDER, so with no preference the fallback order is 0,1,2,... and 78 would take $v0 = exactly target.

- [s1] The preference is injected by global.c:797-840 expand_preferences(), which IORs the hard-reg preference sets of a single_set's DEST allocno and any REG_DEAD allocno of the same insn when the two do not conflict. Insn 153 '(set (reg 72) (sign_extend (mem (plus (reg 78) 4))))' with REG_DEAD (reg 78) — the shared call_with_field4 load — hands 72's legitimate {a0,a1} preference to 78.

- [s1] Allocno priority is floor_log2(n_refs)*n_refs/live_length*10000*size descending (global.c:604-625 allocno_compare); the residual is NOT an ordering problem — 78 takes $a0 because it prefers it, not because $v0 was taken.

- [s1] OPEN POLICY QUESTION recorded in candidate.c + hypotheses.md H1: the floor-7 lever is spelled as reuse of the dead a0 PARAMETER. Not a dead store, not a pin, but codegen-motivated, so it belongs to the SOTN variable-reuse family (exhaustion + /* FAKE */ + layer-2) UNLESS the equivalent named-local spelling (H1) measures the same, which would make it ordinary C. H1 is the next probe and was deliberately left for the next session rather than self-approved.

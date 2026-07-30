
## Session 2 (structural, 2026-07-29) - floor 7 -> **0** (MATCH)

Starting point: src was still at the committed HEAD form (floor 17); session
1's floor-7 lever lived only in `candidate.c`.

### H1 CONFIRMED - the compare-value holder does NOT have to be the `a0` parameter
Replacing the `s16 f86` local with an ordinary named `s32 id;` local that
serves both the `0x86` compare and the call's first argument measures the SAME
floor 7 as session 1's parameter-reuse spelling. The parameter identity is
NOT load-bearing, so the SOTN variable-reuse / `/* FAKE */` policy question
session 1 flagged is RETIRED - it never needed a carve-out.

### The floor-7 wall, fully diagnosed (greg dump, split form)
Split form (`chk_obj` for the check region, `v0_obj` for the four call paths,
named `id` for the arg) at floor 7:

```
;; 8 regs to allocate: 79 80 74 78 73 77 75 81
;; 78 conflicts: 74 78 81 29          ;; 78 preferences: 4 5
;; 79 conflicts: 73 74 75 79 80 81 2 29
;; 80 preferences: 4 5                ;; 81 preferences: 6
;; Register dispositions:
73 in 8  74 in 5  75 in 7  77 in 3  78 in 4  79 in 3  80 in 4  81 in 6
```
Map: 73=ptr($t0 ok), 74=cmd($a1 ok), 75=a3($a3 ok), 77=arg_v1($v1 ok),
79=chk_obj($v1 ok), 80=id($a0 ok), 81=arg_a2($a2 ok) - **every allocno matches
target except 78, the call-path obj, which takes $a0 where target wants $v0.**
78 does NOT conflict with hard reg 2, so $v0 is legally available; it loses it
only to the merged {4,5} preference.

The single-variable (un-split) form is structurally excluded for a different
reason: there the one obj pseudo is live across the f8E/f88 temps that
local-alloc parks in `$v0`, so it CONFLICTS with hard reg 2 and can never take
it (`;; 78 conflicts: ... 2 ...`, disposition `78 in 3`). Splitting the obj is
therefore NECESSARY but (per session 1's K1) not SUFFICIENT.

### THE LEVER (7 -> 2): do not give the call's first argument a C variable
`expand_preferences()` (global.c:797-840) only merges when
`reg_allocno[REGNO (SET_DEST (set))] >= 0` - i.e. when the single_set's
destination is a PSEUDO. A shared `id` local across a shared `do_call:` label
forces that pseudo to exist. Writing the call at each site instead
(`func_80032854(*(s16 *)(*ptr + 0x4), 0x2B, ...); return;` per arm, no `id`,
no `goto`) lets the field-4 load combine straight into the argument HARD
register, so the merge condition fails, no preference reaches the obj, and the
obj falls to the alloc-order first-free register = `$v0` = target.
**Floor 7 -> 2 in one restructure**, and `build_insns` stays 51: jump2's
cross-jumping re-merges the duplicated `lh $a0,0x4($vN)` / `jal` suffixes after
allocation, rebuilding target's shared `.L80027970` / `.L80027974` tails.

### THE LEVER (2 -> 0): drop the param-alias locals - in the NEW structure
Session 1's K2 (dropping `arg_a2`/`arg_v1` costs a point) was measured on the
OLD shared-arg structure and does NOT survive the restructure. Measured sweep
on the duplicated-call form (all at build_insns 51):

| variant | score |
|---|---|
| both aliases kept (`arg_a2`, `arg_v1`) | 2 |
| alias declaration order swapped | 2 |
| drop `arg_v1` alias only | 2 |
| **drop `arg_a2` alias only** | **0** |
| **drop both aliases** | **0** |

So the `arg_a2` copy is the single cause of residual family A: with it, the
scheduler emits `lw 0x2C($sp)` before `lw 0x28($sp)`; target is a2-first.
Declaration order is inert. The committed form drops BOTH (equally 0, simpler C).

### Final state
`sandbox mk_g2l --disable all` => **score 0**, target_insns 51, build_insns 51.
Form in `candidate.c` and in place at `src/code6cac_b.c:402`. Pure C: no dead
stores, no pins, no inline asm, no volatile, no parameter reuse, no alias
locals, no `goto`, no unused declarations. The 10 regfix rules at
regfix.txt:242-257 are now dead paperwork awaiting operator `retire mk_g2l`
(this session may not touch regfix.txt).

- [s2] H1 CONFIRMED: an ordinary named local serving both the 0x86 compare and the call's first argument measures floor 7, identical to session 1's dead-a0-parameter spelling. Parameter identity is not load-bearing; the variable-reuse policy question is retired without needing a carve-out.

- [s2] Split-form greg dump proves the floor-7 residual is ONE allocno: 78 (call-path obj) takes $a0 from a merged {4,5} preference while every other allocno already matches target (73 ptr->$t0, 74 cmd->$a1, 75 a3->$a3, 77 arg_v1->$v1, 79 chk_obj->$v1, 80 id->$a0, 81 arg_a2->$a2).

- [s2] The un-split single-obj form cannot reach $v0 at all: its obj pseudo is live across the f8E/f88 temps that local-alloc puts in $v0, so it carries a HARD conflict with reg 2. Splitting the obj is necessary; K1's finding that it is not sufficient is confirmed and now explained.

- [s2] expand_preferences() requires reg_allocno[REGNO(SET_DEST(set))] >= 0 - the merge only fires when the single_set writes a PSEUDO. Removing the shared arg variable (duplicate the call into each arm, write the field-4 read inline) makes the load write the argument HARD reg, the merge is skipped, and the obj falls to alloc-order $v0. Floor 7 -> 2.

- [s2] Duplicating the call into 5 arms does NOT change instruction count: build_insns stays 51 because jump2 cross-jumps the identical `lh $a0,0x4($vN)` / `jal` suffixes back together AFTER register allocation, reproducing target's shared .L80027970 / .L80027974 tails.

- [s2] Session 1's K2 (param-alias locals are load-bearing, dropping them costs a point) is structure-specific and does NOT survive the restructure. On the duplicated-call form: both aliases = 2, swapped declaration order = 2, drop arg_v1 only = 2, drop arg_a2 only = 0, drop both = 0. The arg_a2 copy alone caused the inverted stack-param load order (residual family A).

- [s2] FINAL: sandbox mk_g2l --disable all => score 0, target_insns 51, build_insns 51, with pure-C edits in place at src/code6cac_b.c:402. Zero cheat constructs. regfix.txt:242-257 (10 rules) are now dead paperwork awaiting operator retire.

# Evidence — func_800174F4 (src/ings.c)

## Session 1 (recon, 2026-08-13) — floor 25 -> 14

### Function shape
136 target instructions. `void func_800174F4(void)` — the per-frame draw
dispatcher: early-returns when `g_disp_enable == 0xFF`, otherwise builds a
DRAWENV in a 0x68-byte stack buffer (`sp+0x20`), `SetDefDrawEnv` /
`PutDrawEnv`, `ClearOTagR(sp+0x18, 2)`, then a 4-way `switch (g_disp_enable)`
(cases 1/2, 10, 20, default), then `DrawOTag(D_800A374C + 4)` + `DrawSync(0)`.
Carries exactly ONE regfix rule: `func_800174F4: $3 <-> $5 @ 27-41`
(regfix.txt:11) — i.e. the cheat covers only the switch-selector register.

Target register roles (from asm/funcs/func_800174F4.s):
  $s0 = &sp20 (drawenv buffer address), later the case-1/2 loop COUNTER.
  $s1 = 0xF0 (drawenv height), later the case-1/2 loop LIMIT, the case-20
        `div` dividend, and the `D_800A37A8[]` table value. One register,
        four roles.
  $s2 = the prim pointer (`&D_800F33D8` -> func_8005D46C -> func_8005D554 ...).

### The cheat scaffold in the inherited body was worth ZERO distance
The pre-session src/ings.c body carried `register s32 s0_var asm("s0")`,
`register s32 s2_var asm("s2")`, `register s32 a1_val asm("a1")`, an
empty-body dead-read `if ((a1_val && a1_val) && a1_val) { }`, and the
`new_var` / `new_var2` constant-holder locals. A from-scratch pure-C rewrite
with ALL of that removed measured the SAME honest floor (25). MEASURED, not
inferred. There is therefore no reason for any future session to keep or
re-derive any of it.

### Measured floor ladder (all `sandbox func_800174F4 --disable all`)
| form | score | insns |
|---|---|---|
| inherited body (pins + dead-read + constant holders) | 25 | 136 |
| clean pure-C rewrite, same structure | 25 | 135 |
| + drop the `a0_temp` copy local from the case-1/2 loop | 25 | 135 |
| + split the loop counter out of `env` into its own `s32 i` | **18** | 135 |
| + move `i++` to AFTER the call inside the loop | **14** | **136** |

At 14 the instruction SEQUENCE matches target exactly (136 vs 136, zero
insert/delete/reorder in the aligned diff). All 14 residual points are
register-NAME substitutions.

### The two residual clusters at floor 14
(A) switch-selector web — 6 diffs, insns 28/30/31/33/35/41:
    ours `lbu v1,%gp_rel(D_800A3768)` + `beq v1,..` / `slti v0,v1,11` /
    `slti v0,v1,3` / `beq v1,..` / `beqz v1,..`; target uses `$a1` throughout.
    This is precisely what the surviving regfix rule `$3 <-> $5 @ 27-41`
    papers over.
(B) callee-save web split — 8 diffs, insns 50/52/53/58/60 and 116/119/120:
    ours  i->$s1, loop-limit->$s0, table-value->$s0
    target i->$s0, loop-limit->$s1, table-value->$s1.
    GCC splits the single C variable `h` into three independent allocnos
    (0xF0/div web, loop-limit web, table-value web). Ours places web #1
    correctly in $s1 but webs #2/#3 in $s0; target keeps all three in $s1.

### Mechanism reference (read from the frozen toolchain source)
`tools/gcc-2.7.2/global.c:allocno_compare` — allocation order is by
    priority = floor_log2(n_refs) * n_refs / live_length   (× 10000 × size),
ties broken by allocno number (i.e. first-use order). Higher priority is
allocated first and takes the first free reg in `reg_alloc_order`, so for the
callee-saves the higher-priority allocno gets $s0. This is the lever surface
for cluster (B): the split is decided by refs/live-length of the individual
webs, not by anything the C names.

### Artifacts
`tmp/grind/func_800174F4/s1/` — `diffdump.py` (aligned normalized-insn diff
tool built on `engine.score.normalized_insns`, ours vs `build/src/ings.o`),
`diff.txt` (baseline 25), `diff_goto_notemp.txt`, `diff_18.txt`, `diff_14.txt`,
`ings.i` + `ings.i.greg` (cc1 `-dg` RTL/global-alloc dump of the baseline
form; `;; Register dispositions` for this function starts at greg line ~5945),
plus snapshots `ings_clean25.c`, `ings_split_i_18.c`, `ings_incr_after_14.c`.

- [s1] Baseline honest floor was 25 (sandbox func_800174F4 --disable all); best form this session is 14 with build_insns == target_insns == 136.

- [s1] MEASURED: the inherited body's entire cheat scaffold is worth ZERO honest distance -- a from-scratch pure-C rewrite that removes all three `register T x asm(...)` pins, the empty-body dead-read `if ((a1_val && a1_val) && a1_val) { }`, and the `new_var`/`new_var2` constant-holder locals scored the SAME 25. No future session should keep or re-derive any of it.

- [s1] func_800174F4 carries exactly one regfix rule: `func_800174F4: $3 <-> $5 @ 27-41` (regfix.txt:11), whose scope is precisely the switch-selector register cluster.

- [s1] Target register roles: $s0 = &sp20 drawenv buffer then the case-1/2 loop counter; $s1 = 0xF0 height, then the loop limit, then the case-20 `div` dividend, then the D_800A37A8[] table value (one register, four roles); $s2 = the prim pointer chain.

- [s1] At floor 14 the residual is exactly two register-name clusters: (A) switch selector ours $v1 vs target $a1 at insns 28/30/31/33/35/41 (6 pts); (B) callee-save webs ours counter->$s1 + limit->$s0 + table-value->$s0 vs target counter->$s0 + limit->$s1 + table-value->$s1 at insns 50/52/53/58/60/116/119/120 (8 pts).

- [s1] GCC 2.7.2 splits the single C variable `h` into three independent allocnos (0xF0/div web, loop-limit web, table-value web); our build places web #1 correctly in $s1 but webs #2/#3 in $s0.

- [s1] Mechanism reference read from the frozen toolchain: tools/gcc-2.7.2/global.c:allocno_compare -- allocation order is priority = floor_log2(n_refs) * n_refs / live_length (x10000 x size), ties broken by allocno number; higher priority is allocated first and takes the first free register in reg_alloc_order, so for the callee-saves the higher-priority allocno gets $s0. Declaration order/naming only enters the tie-break, so it is NOT a lever for these (untied) webs.

- [s1] The cc1 -dg dump of the BASELINE body (tmp/grind/func_800174F4/s1/ings.i.greg, function section at ~line 5945) shows `;; Register dispositions: 72 in 17  73 in 16  74 in 18 ...` -- i.e. the two callee-save pseudos were swapped relative to target at baseline. This dump predates the floor-14 form and must be re-taken before it is used for cluster (B).

## Session 2 (structural, 2026-08-13) — floor 14 -> 8

### The move that did it
The ONLY difference between the session-1 floor-14 form and the session-2
floor-8 form is WHERE `i = 0;` sits. Session 1 had it in the loop pre-header
(before the `rand()` call); session 2 puts it INSIDE the `if (h != 0)` guard
block, immediately before the loop label. That single statement move resolved
the whole callee-save cluster (B): counter -> $s0, loop-limit web -> $s1,
D_800A37A8[] table-value web -> $s1, all matching target.

### Measured ladder (all `sandbox func_800174F4 --disable all`, insns 136
### unless noted)
| form | score |
|---|---|
| s1 floor-14 base (`i = 0;` first statement of the fade block) | 14 |
| `i = 0;` moved after `v0 = rand();` | 14 |
| `i = 0;` moved after `h = v0 + 4;` (still pre-guard) | 14 |
| **`i = 0;` moved INSIDE `if (h != 0) { ... }`** | **8** |
| loop tail respelled `if (i < h) goto inner_loop;` | 14 |
| guard tests `(v0 + 4)` instead of `h` | 15 |
| counter doubles as the rand() temp | 12 (137 insns) |

### The residual at floor 8 — two clusters, both fully characterised
(A) switch-selector web, 6 pts, insns 28/30/31/33/35/41 — unchanged from s1:
    ours does the whole dispatch in `$v1`, target in `$a1`. Exactly the scope
    of the surviving regfix rule `$3 <-> $5 @ 27-41`.
(B') delay-slot placement, 2 pts, insns 50 and 54 — NEW, and it is the
    residue of the fix above. Target fills the `rand()` jal delay slot with
    `move s0,zero` and leaves a `nop` in the guard branch's delay slot; ours
    is the exact mirror (nop at 50, `move s0,zero` at 54).

### Why (B') is a real tension, not an oversight
Read from `tools/gcc-2.7.2/reorg.c:fill_simple_delay_slots`: a CALL_INSN's
delay slot is filled by the BACKWARD search over preceding insns; the
forward "insns beyond the insn needing the delay slot" search is blocked here
because every insn between the call and the guard branch (`andi`, `addiu`)
references the call's `$v0` result, and the search cannot cross the
`beqz`. Therefore target's `move s0,zero` came from BEFORE the call — i.e.
target's `i = 0;` is pre-call, like our floor-14 spellings. But every
pre-call spelling measured loses $s0 for the counter (back to 14). Getting
both simultaneously is the open problem.

### The $s0/$s1 flip is NOT decided by global.c (measured, kills the s1 F1
### mechanism as stated)
The session-1 frontier attributed cluster (B) to `global.c:allocno_compare`
priority. Instrumented cc1 (`BB2_ALLOC_DEBUG`, hooks at global.c:379/605)
was run on five variants. The floor-8 form and the floor-14 pre-call forms
b1/b2 produce **byte-identical allocno tables** (same order, same
n_refs/live_length/priority for every allocno) yet differ by 6 points of
$s0/$s1 naming. Only the s1-baseline form differs (pseudo 85 livelen 13 ->
pri 6153 / ord 3 vs livelen 9 -> pri 8888 / ord 2). Conclusion: the global
allocno priority is not the discriminator between the 14 and the 8 forms —
the flip happens downstream, in `local-alloc.c` quantity assignment /
reload. The next mechanism probe belongs on `BB2_QTY_DEBUG` /
`BB2_SUGG_DEBUG` (local-alloc.c:1518/1585 and 1437/1447/2215), not on
global.c.

### Cluster (A): the F2 mechanism is measured DEAD as stated
`global.c:expand_preferences` only propagates preferences BETWEEN two pseudo
allocnos; the hard-reg preference that would pull the selector into `$a1`
requires a surviving `(set (reg a1) (reg <selector>))` copy insn at global-
alloc time. Every honest spelling that creates one was measured:
  - cache the dispatch value in a local, `switch (mode)`, keep the loop's
    per-iteration re-read of the global: **8** (no change, 136 insns) —
    also with the local `u8`-typed, with the local re-read at the loop site,
    at the else-call site, at all three sites, and with no switch-local at
    all. TEN spellings, ALL exactly 8 and all the same instruction count:
    the copy is folded into the load by combine before global alloc, so the
    preference never exists.
  - actually PASS the cached local as func_8005D554's 2nd argument: **29**
    (138 insns) — `mode` must then survive func_8005D46C and rand(), so it
    goes callee-save and target's per-iteration `lbu a1` becomes a move.
  - one read serving both the `== 0xFF` early-out and the switch: **21**
    (137 insns) — target reads the global twice (insns 4 and 28).
There is no REG_ALLOC_ORDER override in `config/mips/mips.h`, so find_reg
hands out the lowest-numbered free register in the class: $v0(2) is taken by
an earlier allocno, so the preference-less selector takes $v1(3). For target
to reach $a1(5), regs 2/3/4 must all be excluded by conflict, or a copy
preference must exist. Neither is reachable by any statement-order or
locals spelling measured this session.

### Artifacts (session 2)
`tmp/grind/func_800174F4/s2/` — `sweep.py` (splice-a-variant-and-score
harness, restores src/ings.c in a finally block), `gen.py`/`gen2.py`/
`gen3.py`/`gen4.py` (variant generators), `variants/*.c` (23 measured forms),
`allocsweep.sh` + `dump.sh` (BB2_ALLOC_DEBUG cc1 dumps), `probe.sh`
(apply + score + aligned diff), `bank.py`, `alloc_*.txt` (the allocno
priority tables), `diff_b3.txt` (the floor-8 aligned diff),
`diff_b1.txt` (the pre-call-`i=0` aligned diff).

- [s2] Floor 14 -> 8 by moving `i = 0;` from the loop pre-header INTO the `if (h != 0)` guard block; 136 == 136 insns; the entire callee-save cluster (counter->$s0, loop-limit->$s1, table-value->$s1) now matches target.

- [s2] The residual 8 = 6 pts switch-selector ($v1 vs $a1, insns 28/30/31/33/35/41 — the sole regfix rule's exact scope) + 2 pts delay-slot placement (target puts `move s0,zero` in the rand() jal delay slot and a nop in the guard-branch slot; ours is the mirror).

- [s2] MEASURED KILL of the session-1 F1 mechanism as stated: the cc1 BB2_ALLOC_DEBUG allocno table (order, n_refs, live_length, priority) is BYTE-IDENTICAL between the floor-8 form and the floor-14 pre-call forms, so global.c:allocno_compare is NOT what decides the $s0/$s1 flip. The discriminator is downstream (local-alloc quantities / reload) — instrument BB2_QTY_DEBUG / BB2_SUGG_DEBUG next, not global.c.

- [s2] MEASURED KILL of the h-live-range axis: hoisting `h = 0xF0;` to the top of the function / before `env` / to just before SetDefDrawEnv, on two different bases (six combinations), scores 14 every time and does not move the `h` allocno's live_length in the cc1 dump at all.

- [s2] MEASURED KILL of the session-1 F2 mechanism as stated: ten honest selector-local spellings (cache-and-switch, u8 local, local re-read at the loop site / the else-call site / all sites, no switch-local) ALL score exactly 8 with 136 insns — combine folds the pseudo-to-$a1 copy into the load before global alloc, so expand_preferences never sees a copy and no $a1 preference can exist. Passing the cached local as the call's 2nd argument costs 2 insns (29/138); a single read serving both the 0xFF early-out and the switch costs 1 insn (21/137).

- [s2] config/mips/mips.h defines NO REG_ALLOC_ORDER, so global.c:find_reg hands out the lowest-numbered available register of the class. The selector takes $v1(3) because $v0(2) is held by an earlier-allocated allocno. Reaching target's $a1(5) requires either conflicts covering 2/3/4 or a hard-reg copy preference — neither is produced by any statement-order or named-local spelling.

- [s2] Read from tools/gcc-2.7.2/reorg.c:fill_simple_delay_slots — a CALL_INSN's delay slot is filled by the backward search; the forward "beyond the insn" search cannot reach the guard-block `i = 0;` here because the intervening `andi`/`addiu` both reference the call's $v0 result and the search cannot cross the `beqz`. So target's `i = 0;` is PRE-call, which is exactly the placement that loses $s0 for the counter — the two remaining goals are in direct tension and that tension IS the frontier.

- [s1] tmp/grind/func_800174F4/s1/diffdump.py is a reusable aligned normalized-instruction differ (ours vs build/src/ings.o) built on engine.score.normalized_insns; it is what turned an opaque score into the per-insn cluster map above.

- [s2] [s2] Floor 14 -> 8 by moving `i = 0;` from the loop pre-header INTO the `if (h != 0)` guard block; build_insns == target_insns == 136; the whole callee-save cluster (counter->$s0, loop-limit->$s1, D_800A37A8[] table-value->$s1) now matches target.

- [s2] [s2] The residual 8 points are exactly two clusters: 6 pts switch-selector ($v1 vs target $a1 at insns 28/30/31/33/35/41 - the sole surviving regfix rule's exact scope, `func_800174F4: $3 <-> $5 @ 27-41`) and 2 pts delay-slot placement (target has `move s0,zero` in the rand() jal delay slot and a nop in the guard-branch delay slot; ours is the mirror, insns 50 and 54).

- [s2] [s2] MEASURED KILL of session-1's F1 mechanism: the cc1 BB2_ALLOC_DEBUG allocno table is byte-identical between the floor-8 form and the floor-14 pre-call forms, so global.c:allocno_compare is NOT what decides the $s0/$s1 flip. Instrument local-alloc next (BB2_QTY_DEBUG at local-alloc.c:1518/1585, BB2_SUGG_DEBUG at :1437/:1447/:2215).

- [s2] [s2] MEASURED KILL of session-1's F2 mechanism: ten honest selector-local spellings all score exactly 8 with 136 insns because combine folds the pseudo-to-$a1 copy into the load before global alloc; the two spellings that keep a real copy cost 1-2 instructions (21/137 and 29/138).

- [s2] [s2] MEASURED KILL of the h-live-range axis: six hoist positions for `h = 0xF0;` all score 14 and the `h` allocno's live_length does not move in the cc1 dump.

- [s2] [s2] config/mips/mips.h defines NO REG_ALLOC_ORDER, so global.c:find_reg hands out the lowest-numbered free register of the class; the selector allocno (nrefs 6, live_length 10, allocated 2nd overall) takes $v1(3) because $v0(2) is already held by an earlier allocno. Reaching target's $a1(5) requires conflicts covering regs 2, 3 AND 4 - a copy preference is not available (K6).

- [s2] [s2] Read from tools/gcc-2.7.2/reorg.c:fill_simple_delay_slots - a CALL_INSN's delay slot is filled by the BACKWARD search over preceding insns; the forward 'beyond the insn' search cannot reach our guard-block `i = 0;` because the intervening `andi`/`addiu` both reference the call's $v0 result and the search cannot cross the `beqz`. Target's `i = 0;` is therefore PRE-call, which is exactly the placement that costs the counter $s0. The two remaining goals are in direct tension and that tension is the frontier.

- [s2] [s2] tmp/grind/func_800174F4/s2/sweep.py is a reusable splice-a-variant-and-score harness (restores src/ings.c in a finally block); allocsweep.sh + dump.sh produce per-variant BB2_ALLOC_DEBUG allocno priority tables; probe.sh applies a variant and prints the aligned normalized-insn diff. 23 variants measured this session.

## Session 3 (structural, 2026-08-13) � floor stays 8; the s2 K4 kill is CORRECTED

### THE HEADLINE: session 2's K4 ("global.c is inert; the flip is a
### local-alloc question") is WRONG, and the frontier it produced was
### misdirected. The $s0/$s1 flip IS decided by global.c:allocno_compare.

Session 2 compared BB2_ALLOC_DEBUG output between the floor-8 form and the
floor-14 pre-call forms and read the tables as byte-identical. They are not:
the per-allocno STATS are the same shape but the ORDER of two allocnos
(pseudo 85 = the case-1/2 loop counter, pseudo 73 = `h`) swaps, and that swap
is exactly the $s0/$s1 flip. Measured this session with `-dg` + BB2_ALLOC_DEBUG
on both forms (artifacts `tmp/grind/func_800174F4/s3/ings_b3.i.greg`,
`ings_b12.i.greg`, `alloc_b12.txt`):

| form | counter (pseudo 85) | `h` (pseudo 73) | order | result |
|---|---|---|---|---|
| floor 8 (`i = 0;` inside the `if (h != 0)` guard) | nrefs 4, livelen **9**, pri **8888** | nrefs 7, livelen 16, pri **8750** | 85 then 73 | 85 -> $s0(16), 73 -> $s1(17) � TARGET |
| floor 14 (`i = 0;` after `h = v0 + 4;`, pre-guard) | nrefs 4, livelen **10**, pri **8000** | nrefs 7, livelen 16, pri **8750** | 73 then 85 | 73 -> $s0(16), 85 -> $s1(17) � WRONG |

The conflict lists and every other allocno's stats are IDENTICAL between the
two forms. The ONLY difference in the whole `.greg` header is the two-element
order swap. The margin at floor 8 is **8888 vs 8750 � 1.6%**, i.e. ONE unit of
the counter's live length. This is the sharpest possible statement of the
lever and it is a *global.c* lever, not a local-alloc one.

### What that means quantitatively for the 2-point delay-slot residue (F4)
Target fills the `rand()` jal delay slot with `move s0,zero`, which requires
`i = 0;` to be emitted in the basic block that ENDS with that call � i.e. the
first statement of the `if (g_disp_fade != 0)` block, before `v0 = rand();`.
Measured this session (`rejected/i-init-first-stmt-of-fade-block.c`): score 14,
136 insns. With that placement the counter's live range spans `jal rand` +
`andi` + `addiu` + `beqz` (livelen ~13), so
  priority = floor_log2(4)*4/13*10000 = 6153  <  h's 8750,
and `h` takes $s0. To win with a pre-call init the counter allocno needs
either n_refs >= 6 (2*6/13 = 9230 > 8750) or `h` pushed below 6153
(n_refs 7 needs livelen > 22.7, vs its actual 16). Both were attacked
structurally this session and both are dead (see the ladder below). THAT
inequality is the whole of F4, written as arithmetic.

### Measured ladder (all `sandbox func_800174F4 --disable all`)
| form | score | insns |
|---|---|---|
| **s2 floor-8 base (`i = 0;` inside the guard)** | **8** | 136 |
| guard inverted to an early-exit `if (h == 0) break;` | **8** | 136 |
| `i = 0;` first statement of the fade block (pre-`rand()`) | 14 | 136 |
| `i = 0;` after `h = v0 + 4;` (pre-guard) | 14 | 136 |
| `i = 0;` duplicated: pre-`rand()` AND inside the guard | 14 | 136 |
| `i = 0;` at the top of case 1/2 (before func_8005D46C) | 15 | 136 |
| `i = 0;` before the `g_disp_fade` test (block-scoped `i`) | 18 | 137 |
| `for (i = 0; ; ) { ...; if (i >= h) break; }` | 33 | 136 |
| natural `for (i = 0; i < h; i++)` (no explicit guard) | 39 | 135 |
| natural `i = 0; while (i < h) { ...; i++; }` | 39 | 135 |
| `env` (the drawenv-buffer variable) doubles as the counter, init in guard | 22 | 136 |
| `env` doubles as the counter, init pre-`rand()` / fade-top / after-`h` | 21 | 136 |
| counter shared with the case-20 `D_800A37C0` counter (7 refs), init in guard | 18 | 136 |
| counter shared with case 20, init pre-`rand()` | 24 | 136 |
| case-20 `h = D_800A37A8[a0_val];` hoisted to the top of its else block | 14 | 136 |
| that hoist + pre-`rand()` init | 19 | 136 |

Seventeen forms, none below 8. Floor 8 stands.

### Cluster (A), the 6-point switch selector: the exact find_reg state
Instrumented `BB2_FINDREG_DEBUG=120` on the floor-8 form
(`tmp/grind/func_800174F4/s3/findreg_b3_120.txt`) prints the selector
allocno's complete decision state:

    FINDREGDBG func=func_800174F4 pseudo=120 alt=0 acc=0 retry=0
    FINDREGDBG  conflicts: 2 29
    FINDREGDBG  someone_prefers:            <- EMPTY
    FINDREGDBG  own_copy_prefs:             <- EMPTY
    FINDREGDBG  own_full_prefs:             <- EMPTY
    FINDREGDBG  pass0_used: 0 1 2 17..23 26..31
    FINDREGDBG  class=1 mode=4 size=1

So `find_reg` takes reg 3 because 3 is simply the lowest register of the class
that is neither a conflict nor preferred-by-a-conflicting-lower-priority
allocno. To land on $a1(5) the selector needs **3 AND 4 in
`hard_reg_conflicts` U `regs_someone_prefers`**. In the same dump the
mechanism is visible working for another allocno: pseudo 99 has hard conflicts
{2,3} and still skips 4 to take 5, because the lower-priority allocno 101
*prefers* 4 (`;; 101 preferences: 4`) and `prune_preferences` therefore puts 4
into `regs_someone_prefers[99]`. That is the ONLY route to $a1 for the
selector, and it needs (a) a lower-priority allocno preferring 3 or 4 that
CONFLICTS with the selector, plus (b) a hard-reg conflict at the other one.
The selector's conflict set is `{74, 81}` (the two callee-save pseudos) plus
hard {2, 29} � it conflicts with nothing that prefers 3 or 4, and it cannot
without living across the `func_8005D46C` call region, which forces it
callee-save instead (that is the measured 29/138 form from s2's K6).

### Artifacts (session 3)
`tmp/grind/func_800174F4/s3/` � `apply.py` (persistently splice a variant into
src/ings.c), `greg.sh` (cc1 `-dg` dump), `alloc.sh` (BB2_ALLOC_DEBUG allocno
table), `findreg.sh` (BB2_FINDREG_DEBUG per-pseudo find_reg state),
`gen.py`/`gen2.py`/`gen3.py`/`gen4.py` (variant generators), `variants/*.c`
(17 measured forms + `base8.c`), `ings_b3.i.greg`, `ings_b12.i.greg`,
`alloc_b12.txt`, `findreg_b3_120.txt`, `diff_base8.txt` (the floor-8 aligned
diff, showing only insns 28/30/31/33/35/41 and the 50/54 delay-slot swap).

- [s3] CORRECTION to [s2] K4: global.c is NOT inert for the $s0/$s1 flip. The cc1 `-dg` headers of the floor-8 and floor-14 forms differ in exactly one place - the allocno ORDER list (`;; 9 regs to allocate: 84 120 85 73 ...` vs `84 120 73 85 ...`). Conflict lists are identical. The counter (pseudo 85) has pri 8888 at livelen 9 in the floor-8 form and pri 8000 at livelen 10 in the floor-14 form, against `h` (pseudo 73) at a constant pri 8750. The lever is global.c:allocno_compare after all; session 2 read the BB2_ALLOC_DEBUG tables as identical and missed the order swap.

- [s3] The floor-8 margin is 1.6% (8888 vs 8750) - ONE unit of the loop counter's live length. Any C shape that lengthens the counter's live range by a single insn loses $s0 to `h` and costs 6 points.

- [s3] QUANTIFIED F4: to fill the `rand()` jal delay slot with `move s0,zero`, `i = 0;` must be the first statement of the `if (g_disp_fade != 0)` block (livelen ~13, pri 6153). To still win $s0 from there the counter allocno needs n_refs >= 6 (2*6/13 = 9230 > 8750) or `h` must drop under 6153 (with n_refs 7 that needs livelen > 22.7 vs its actual 16). Both requirements were attacked structurally and neither is reachable: raising the counter's refs by sharing it with the case-20 counter gives 18/24, and lengthening `h` by hoisting its case-20 table load gives 14/19.

- [s3] MEASURED KILL of the natural-loop hypothesis: `for (i = 0; i < h; i++)` and `i = 0; while (i < h) { ...; i++; }` both score 39 with 135 insns - GCC 2.7.2's loop rotation emits a two-instruction entry test (`slt` + branch) instead of target's single `beqz $s1`, so the goto-form loop with an explicit `if (h != 0)` guard is load-bearing, not a decompiler artifact.

- [s3] MEASURED KILL of the env-reuse hypothesis (target reuses $s0 for the drawenv-buffer address AND the loop counter, which suggested one C variable): making `env` double as the counter scores 21-22 on all four init placements. The $s0 sharing in target is RA reusing a dead register, not a shared C variable.

- [s3] MEASURED: `if (h == 0) break;` as an early-exit guard (instead of `if (h != 0) { ... }`) is exactly equivalent - 8, 136 insns, byte-identical output. Two spellings of the floor-8 form now exist.

- [s3] MEASURED KILL of the duplicated-init form: `i = 0;` written BOTH before `v0 = rand();` and inside the guard scores 14 - the dead pre-store still lengthens the counter allocno's live range, so the sanctioned dead-store-to-a-local family buys nothing here.

- [s3] Cluster (A) find_reg state read from the instrumented cc1 (BB2_FINDREG_DEBUG=120, floor-8 form): the selector allocno has conflicts {2, 29}, EMPTY someone_prefers, EMPTY own_copy_prefs, EMPTY own_full_prefs. It takes reg 3 because 3 is the lowest class register outside pass0_used. Reaching $a1(5) needs BOTH 3 and 4 in conflicts-union-someone_prefers. The same dump shows the mechanism working for pseudo 99 (hard conflicts {2,3}, skips 4 because lower-priority allocno 101 prefers 4, lands on 5) - so the route exists in principle but requires a conflicting lower-priority allocno that prefers 3 or 4, which the selector cannot acquire without living across the func_8005D46C call and going callee-save (s2's measured 29/138).

- [s3] [s3] CORRECTION to [s2] K4: global.c is NOT inert for the $s0/$s1 flip. The cc1 `-dg` headers of the floor-8 and floor-14 forms differ in exactly one place — the allocno ORDER list (`;; 9 regs to allocate: 84 120 85 73 ...` vs `84 120 73 85 ...`). All nine conflict lists are byte-identical. The lever is global.c:allocno_compare; session 2 read the BB2_ALLOC_DEBUG stats as identical and missed the order swap.

- [s3] [s3] The floor-8 register match rests on a 1.6% priority margin: counter pseudo 85 = n_refs 4, live_length 9, pri 8888 vs `h` pseudo 73 = n_refs 7, live_length 16, pri 8750. ONE extra unit of the counter's live length flips it (the floor-14 form measures live_length 10, pri 8000). Any future form that lengthens the counter's live range by a single insn costs 6 points.

- [s3] [s3] QUANTIFIED F4 (the 2-point delay-slot residue): to fill the `rand()` jal delay slot with `move s0,zero`, `i = 0;` must be the first statement of the `if (g_disp_fade != 0)` block (reorg.c's backward search), which puts the counter at live_length ~13 / pri 6153. Winning $s0 from there needs counter n_refs >= 6 (2*6/13 = 9230 > 8750) or `h` under 6153 (n_refs 7 needs live_length > 22.7 vs its actual 16). Both requirements were attacked structurally this session and both are dead.

- [s3] [s3] Cluster (A) find_reg state, read from the instrumented cc1 (BB2_FINDREG_DEBUG=120, floor-8 form, tmp/grind/func_800174F4/s3/findreg_b3_120.txt): the selector allocno has conflicts {2, 29}, EMPTY someone_prefers, EMPTY own_copy_prefs, EMPTY own_full_prefs, and takes reg 3 as the lowest class register outside pass0_used. Reaching $a1(5) needs BOTH 3 and 4 in conflicts-union-someone_prefers.

- [s3] [s3] The exclusion mechanism that cluster (A) needs is visible working elsewhere in the same function: pseudo 99 has hard conflicts {2,3} and still skips reg 4 to take 5, because lower-priority conflicting allocno 101 carries `;; 101 preferences: 4` and prune_preferences therefore puts 4 into regs_someone_prefers[99]. So the $a1 route requires a lower-priority argument-preferring allocno that CONFLICTS with the selector.

- [s3] [s3] MEASURED KILL of the natural-loop hypothesis: `for (i = 0; i < h; i++)` and `i = 0; while (i < h) { ...; i++; }` both score 39 with 135 insns — GCC 2.7.2's loop rotation emits a two-instruction entry test (`slt` + branch) instead of target's single `beqz $s1`. The goto-form loop with the explicit `if (h != 0)` guard is load-bearing.

- [s3] [s3] MEASURED KILL of the env-reuse hypothesis: making `env` (the drawenv-buffer address) double as the loop counter scores 21-22 across all four init placements. Target's $s0 sharing between the buffer address and the counter is the allocator reusing a dead register, not a shared C variable.

- [s3] [s3] MEASURED: `if (h == 0) break;` as an early-exit guard is exactly equivalent to `if (h != 0) { ... }` — 8, 136 insns, byte-identical output. Two spellings of the floor-8 form now exist; noted in candidate.c.

- [s3] [s3] MEASURED KILL of the duplicated-init form: `i = 0;` written BOTH before `v0 = rand();` and inside the guard scores 14 — the dead pre-store still starts the counter allocno's live range, so the sanctioned dead-store-to-a-local family buys nothing here.

- [s3] [s3] Seventeen structural forms measured this session, none below 8: guard early-exit 8; i-init pre-rand 14; i-init after h 14; i-init duplicated 14; i-init top of case 15; i-init before the fade test 18 (137 insns); for-header hybrid 33; natural for 39 (135); natural while 39 (135); env-as-counter 22/21/21/21; counter shared with case 20 18/24; case-20 h hoist 14; that hoist plus pre-call init 19.

- [s3] [s3] Reusable tooling added: tmp/grind/func_800174F4/s3/alloc.sh (BB2_ALLOC_DEBUG per-allocno priority table for the current src/ings.c), greg.sh (cc1 `-dg` dump with the allocno order list and conflict lists), findreg.sh <tag> <pseudo> (BB2_FINDREG_DEBUG dump of one allocno's complete find_reg decision state), apply.py (persistently splice a variant body into src/ings.c). Reading the priority/exclusion sets directly is far cheaper than inferring them from the score.

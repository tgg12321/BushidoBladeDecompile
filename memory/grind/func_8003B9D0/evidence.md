# Evidence bank — func_8003B9D0

## Session 1 (recon, 2026-08-11)

### Baseline
- `canonical`: verdict **C** (pure-C target), asm_insns 0, total 185, distance 21.
- `sandbox --disable all`: **score 21**, target_insns 185, **build_insns 178** (we are
  SEVEN instructions SHORT — our build is MORE optimized than target, not less).
- HEAD source carries 3 cheat-asm constructs (all sandbox-stripped, all score-inert):
  one identity-reload barrier `__asm__ __volatile__("" : "=r"(eda) : "0"(eda))` at
  src/code6cac_c2.c:211 and two `__asm__ __volatile__("" ::: "memory")` scheduling
  barriers at :223 and :226. Plus ONE regfix rule (`func_8003B9D0: fill_delay @ 49 <- 52`,
  regfix.txt:1116). All four exist to compensate for the two residual regions below.
- `diagnose` says LARGE / 31 differing insns — that classification is *stale/coarse*;
  the real normalized diff is TWO tight regions totalling 7 missing insns + 6 respelled.

### The residual is exactly two independent regions
Measured with `tmp/grind/func_8003B9D0/s1/diff.py` (normalized-insn side-by-side of
`tmp/sandbox/func_8003B9D0/code6cac_c2.o` vs `build/src/code6cac_c2.o` via
`engine.score.normalized_insns`). Everything outside these two regions is byte-equal
(58 equal head, 88 equal tail).

**REGION A — displaced access respelled (3 sites, +3 insns of ours, 0 net length change
in the region but 6 wrong insns).**
Target keeps the symbol address live in `$s0` and uses register+displacement for the
`+0x44C` accesses:
```
tgt 58  lh s2,1100(s0)          tgt 67  sh v0,1100(s0)      tgt 71  sh s2,1100(s0)
```
Ours re-materializes the symbol for every displaced access:
```
mine 58 lui s2,0x0 / lh s2,1100(s2)      (same for the two sh sites, via $at)
```
i.e. cc1 substituted the symbol-valued pseudo into the MEM address and folded to
`%lo(D_80101EDA+0x44C)` (== `%lo(D_80102326)`; note 0x80101EDA + 0x44C = 0x80102326,
so `eda[0x226]` and the global `D_80102326` are the SAME halfword).
The **zero-offset** accesses (`lh s1,0(s0)`, `sh v0,0(s0)`) already match — the fold
bites ONLY the non-zero displacement.

**REGION B — target re-loads the flag word twice more than we do (the 7 missing insns).**
Target reads `((u8 *)D_800A3878)[3]` with a FULL reload (lui/lw/nop/lbu/nop) at each of
three sites — tgt[72..77], tgt[82..87], tgt[92..96] — even though nothing writes memory
between them. We emit the reload ONCE (mine[75..79]) and reuse `$v1` (and the pointer
`$a1`) for all three reads plus the later `p[0]`:
```
mine 80 andi v0,v1,0x1 ... 85 andi v0,v1,0x2      (10 insns total)
tgt  72..91                                        (21 insns total)
```
That is the entire 178-vs-185 shortfall.

### Measured probes this session
1. **Displaced-pointer-into-its-own-local** ([[defeat-combine-symbol-fold]] lever;
   `s16 *eda_alt = eda + 0x226;` declared at block top, all three displaced accesses via
   `*eda_alt`): sandbox **21, build_insns 178, region A byte-identical to before** — the
   lever does NOT apply here. The rule's confirmed case (func_80082C58) had a *call*
   between the pointer def and the displaced use; here two of the three displaced
   accesses precede the call, and the extra local is folded away before combine runs.
2. **Re-basing the pointer at the other end** (`s16 *eda = &D_80102326;` with
   `eda[-0x226]` for the low access): sandbox **24** (WORSE — now the *other* three sites
   fold). Proves the fold follows the NON-ZERO DISPLACEMENT, not the symbol identity and
   not the sign of the offset. Any pointer-rebasing spelling just moves which sites fold.
3. **Separate scoped read-locals** (`{ u8 *r1 = (u8*)D_800A3878; ... }` / `{ u8 *r2 = ... }`
   around each flag test): region B **unchanged** — one load, `$v1` reused. Textual
   re-reads in distinct scopes are not enough; cc1's CSE crosses the join labels
   (LABEL_NUSES==1 at both `.L8003BB18` / `.L8003BB40`, so the extended basic block is
   not broken there).
4. **Intervening global store** (moved the already-present `D_800A390F = 0;` statement
   between the two flag `if`s): **build_insns 178 → 180**, a second `lbu v0,3(a1)` reload
   APPEARED (mine[87]). Score stayed 21 because the `sb` landed in the wrong place and the
   pointer `lw` was still shared. **This is the mechanism confirmation for region B:** a
   memory write invalidates the *dereference* MEM (`3(ptr)`, unknown alias) and forces the
   re-read, but a store to a DIFFERENT named symbol (`D_800A390F`) does NOT invalidate the
   symbol MEM `D_800A3878`, so its `lw` stays CSE'd. Target reloads BOTH the `lw` and the
   `lbu` at all three sites — so whatever the original C did between those reads
   invalidated *everything*, i.e. it was a store through a pointer (unknown address), not
   a store to a named global.

### Facts worth not re-deriving
- `D_80101EDA[0x226] == D_80102326` (same halfword, two splat names).
- `D_800A3878` is `extern s32` (include/code6cac.h:180) used as a pointer-to-byte-struct;
  every read in this function is `lw` of the symbol + `lbu 3(ptr)`.
- The 6-line block guarded by `qf & 0x30` saves/sets-to-0x32/restores that pair around
  `func_8003AFFC()`.
- Our build being SHORTER than target means the search direction is "make cc1 optimize
  LESS", not "find a cheaper form" — every candidate must ADD 7 honest instructions.

- [s1] canonical: verdict C, asm_insns 0, total 185, distance 21 -- pure-C target, no canonical-asm question.

- [s1] sandbox --disable all: score 21, target_insns 185, build_insns 178. We are SEVEN instructions SHORT -- our build is MORE optimized than target, so every candidate must ADD honest instructions, not find a cheaper form.

- [s1] HEAD src carries 3 score-inert cheat-asm constructs (identity-reload barrier on `eda` at src/code6cac_c2.c:211; two `__asm__ __volatile__("" ::: "memory")` at :223 and :226) plus one regfix rule (`func_8003B9D0: fill_delay @ 49 <- 52`, regfix.txt:1116). All four exist to compensate for the two residual regions; a COMPLETED-C form must delete all four.

- [s1] The residual is exactly two independent regions; everything else is byte-equal (58 equal insns head, 88 equal tail).

- [s1] REGION A (6 wrong insns, net length 0): target keeps the symbol address in $s0 and uses register+displacement -- `lh s2,1100(s0)`, `sh v0,1100(s0)`, `sh s2,1100(s0)`. We re-materialize the symbol at each of those three sites (`lui` + `%lo(D_80101EDA+0x44C)`). The ZERO-offset accesses already match, so the fold bites only the non-zero displacement.

- [s1] REGION B (the entire 7-insn shortfall): target reads ((u8 *)D_800A3878)[3] with a full lui/lw/nop/lbu/nop reload at each of three sites (tgt 72..77, 82..87, 92..96) with nothing writing memory between them; we emit that block once and reuse $v1 (and pointer $a1) for all three reads plus the later p[0]. 10 insns of ours vs 21 of target.

- [s1] 0x80101EDA + 0x44C == 0x80102326 -- `eda[0x226]` and the global D_80102326 are the SAME halfword under two splat names.

- [s1] D_800A3878 is `extern s32` (include/code6cac.h:180) used as a pointer to a byte struct; every read is `lw` of the symbol + `lbu 3(ptr)`.

- [s1] diagnose's 'LARGE / 31 differing insns' classification is coarse/misleading for this function -- the true normalized diff is the two tight regions above.

- [s1] src/code6cac_c2.c was restored to HEAD at end of session (git status clean apart from metrics/events.jsonl and the new memory/grind/func_8003B9D0/ ledger).

## Session 2 (structural, 2026-08-11)

### Floor
- **sandbox --disable all: 6** (target_insns 185, build_insns 188), down from
  session 1's 21. Measured THIS session with the candidate applied to
  `src/code6cac_c2.c`; re-measured a second time after all probes were reverted,
  same result (6 / 188 / rules_dropped 1).
- The function's THREE cheat-asm constructs from session-1 HEAD are GONE: the
  `__asm__ __volatile__("" : "=r"(eda) : "0"(eda))` identity-reload barrier and
  both `__asm__ __volatile__("" ::: "memory")` scheduling barriers are deleted and
  are no longer needed — the honest `if/else` C reproduces what they were faking.
  The one regfix rule (`func_8003B9D0: fill_delay @ 49 <- 52`, regfix.txt:1116)
  is untouched (grind sessions may not edit regfix.txt) and is still counted by
  `rules_dropped: 1` in the sandbox output.

### REGION B IS CLOSED
The whole 7-instruction shortfall closed by spelling the two flag-selected
argument initialisations as `if/else` rather than "init to -1, then conditionally
overwrite". See hypotheses.md H1 for the mechanism (cse_end_of_basic_block's
follow-jumps/skip-blocks extension at cse.c:8102-8184). Session-1 frontier items
F1 (intervening pointer store) and F2 (volatile-qualified reads) are both moot —
region B needed neither, so the volatile census that F2 asked for does not need to
be run for region B.

### The residual is now REGION A ONLY — three sites, 2 insns vs 1
Normalized diff (`tmp/grind/func_8003B9D0/s2/region_a_diff_floor6.txt`):
58 equal head insns, then exactly three isolated replacements, then a 113-insn
byte-equal tail.
```
  mine 58 lui s2,0x0 / 59 lh s2,1100(s2)   | tgt 58  lh s2,1100(s0)
  mine 68 lui at,0x0 / 69 sh v0,1100(at)   | tgt 67  sh v0,1100(s0)
  mine 73 lui at,0x0 / 74 sh s2,1100(at)   | tgt 71  sh s2,1100(s0)
```
Both builds materialise the base identically — `lui s0` / `addiu s0` at insns
55-56 (byte-equal) — and both use `0($s0)` for the zero-offset accesses. The ONLY
difference is that ours re-materialises the symbol for the `+0x44C` displacement.

### Mechanism for region A, read out of the frozen compiler source
- `fold_rtx`'s MEM case calls `find_best_addr` (cse.c:5029-5034).
- `find_best_addr` returns early only for frame/arg-pointer addresses and
  `CONSTANT_ADDRESS_P` (cse.c:2646-2657). For any address that is **not** a bare
  REG it then does `validate_change (insn, loc, fold_rtx (addr, insn), 0)`
  (cse.c:2659-2663) — **unconditionally, with no cost test.**
- `fold_rtx` substitutes a register operand's `qty_const`, but ONLY when that
  `qty_const` is neither `REG` nor a bare `PLUS` (cse.c:5171-5176).
- `qty_const` is recorded when a constant enters the register's equivalence class
  (cse.c:1377-1397).
- The `.cse` dump matches exactly: insn 89 `(set (reg/v:SI 94) (symbol_ref "D_80101EDA"))`
  + `REG_EQUAL` note; insn 92 `(mem:HI (reg 94))` survives (addr IS a REG, so the
  fold branch at cse.c:2661 is skipped); insn 97 becomes
  `(mem/s:HI (const:SI (plus:SI (symbol_ref "D_80101EDA") (const_int 1100))))`.

Consequences (all now settled, do not re-derive):
1. There is no cost-based route to suppress the fold — it is not a cost decision.
2. Volatile cannot suppress it either: the fold at cse.c:2661 runs BEFORE the
   `addr_volatile` bail-out at cse.c:2668-2675. The volatile axis is dead for
   region A on mechanism grounds, independent of any policy gate.
3. A cse basic-block boundary between the base definition and the first displaced
   use is geometrically impossible: in target the `la` is at tgt[55..56] and the
   first displaced use at tgt[58], with only `lh s1,0(s0)` in between.
4. GCC 2.7.2 has no pass that hoists a repeated symbolic address into a shared
   base register, so target's `la` + register-displacement shape can ONLY come
   from a pointer variable in the C source (proved by the plain-globals probe).

### Probes measured this session
| Form | sandbox | build_insns | Verdict |
|---|---|---|---|
| candidate (`if/else` region-B form) | **6** | 188 | current floor |
| two independent pointers (`eda` @ D_80101EDA, `edb` @ D_80102326, all offset 0) | 17 | 189 | KILLED (K4) |
| no pointer variable, six plain global accesses (the m2c shape) | 23 | 187 | KILLED (K5) |

### Facts worth not re-deriving
- m2c reconstructs the block as `D_80101EDA.unk0` / `D_80101EDA.unk44C`, but m2c
  renders any register-based struct access as `SYM.unkNNN` regardless of whether
  the C used a pointer — so that rendering is NOT evidence for the plain-global
  spelling (measured 23, K5).
- m2c also infers `func_8003AFFC(D_800A3878)`: `$a0` holds the `D_800A3878` object
  pointer live from tgt[48] through the `jal` at tgt[68] with no reload. Our C
  calls it with no arguments. Byte-neutral here, but it is a real semantic
  discrepancy in the decompilation and is recorded as frontier F3.
- `cheat_asm_stripped: 69` in the sandbox output is FILE-wide (other functions in
  code6cac_c2.c), not this function — func_8003B9D0's own body now has zero
  `__asm__`.

- [s2] [s2] sandbox --disable all == 6 (target_insns 185, build_insns 188), down from session 1's 21. Measured twice this session: once on applying candidate.c to src/code6cac_c2.c, and again after all probes were reverted.

- [s2] [s2] The candidate carries ZERO cheat-asm for this function. Session-1 HEAD's identity-reload barrier `__asm__ __volatile__("" : "=r"(eda) : "0"(eda))` and both `__asm__ __volatile__("" ::: "memory")` scheduling barriers are deleted and no longer needed -- the honest if/else C reproduces what they were faking. The one regfix rule (func_8003B9D0: fill_delay @ 49 <- 52, regfix.txt:1116) is untouched, as grind sessions may not edit regfix.txt; it is what `rules_dropped: 1` reports.

- [s2] [s2] REGION B IS CLOSED. The whole 7-instruction shortfall came from cc1 CSE-ing the ((u8 *)D_800A3878)[3] reload across all three flag tests; the if/else spelling of the two argument initialisations ends the CSE basic block at each join and restores target's three full lui/lw/nop/lbu/nop reloads (tgt[72..77] / tgt[82..87] / tgt[92..96]).

- [s2] [s2] The residual is REGION A ONLY -- three isolated sites, ours 2 insns vs target's 1: mine[58] lui s2 / mine[59] lh s2,1100(s2) vs tgt[58] lh s2,1100(s0); mine[68..69] vs tgt[67] sh v0,1100(s0); mine[73..74] vs tgt[71] sh s2,1100(s0). Everything else is byte-equal (58 head + 113 tail).

- [s2] [s2] Both builds materialise the base IDENTICALLY -- `lui s0` / `addiu s0` at insns 55-56 are byte-equal -- and both use `0($s0)` for the zero-offset accesses. The only divergence is that ours re-materialises the symbol for the +0x44C displacement.

- [s2] [s2] Region A mechanism, read out of the frozen compiler source and confirmed in the .cse dump: fold_rtx's MEM case calls find_best_addr (cse.c:5029-5034); find_best_addr returns early only for frame/arg-pointer addresses and CONSTANT_ADDRESS_P (cse.c:2646-2657) and otherwise folds any non-REG address unconditionally with no cost test (cse.c:2659-2663); fold_rtx substitutes the base register's qty_const only when that qty_const is neither REG nor a bare PLUS (cse.c:5171-5176). `.cse` insn 89 sets qty_const from `(set (reg/v:SI 94) (symbol_ref "D_80101EDA"))`; insn 92 survives as `(mem:HI (reg 94))`; insn 97 becomes `(mem/s:HI (const:SI (plus:SI (symbol_ref "D_80101EDA") (const_int 1100))))`.

- [s2] [s2] CLOSED AXIS -- cost. The region-A fold is not a cost decision (no ADDRESS_COST or rtx_cost test guards it), so no spelling that makes the symbolic form 'look more expensive' can suppress it.

- [s2] [s2] CLOSED AXIS -- volatile. The fold at cse.c:2661 executes BEFORE find_best_addr's addr_volatile bail-out at cse.c:2668-2675, so volatile-qualifying the base pointer or the accesses cannot suppress it. Region A's volatile axis is dead on mechanism grounds, independently of the [[legitimate-volatile-interrupt-touched]] two-prong policy gate.

- [s2] [s2] CLOSED AXIS -- cse basic-block boundary. In target the base materialisation (tgt[55..56]) and the first displaced use (tgt[58]) are separated only by `lh s1,0(s0)`, so no boundary can exist there. 'Separate the pointer definition from the displaced access by control flow' is geometrically impossible, not merely unmeasured.

- [s2] [s2] CLOSED AXIS -- shared-base creation by the compiler. With no pointer variable in the C, cc1 emits six independent lui/%lo pairs and never creates a shared base register (measured 23). GCC 2.7.2 has no pass that hoists a repeated symbolic address into a pseudo, so target's `la` + register-displacement shape can only originate from a pointer variable in the source.

- [s2] [s2] m2c infers `func_8003AFFC(D_800A3878)`: $a0 holds the D_800A3878 object pointer live from tgt[48] through the `jal` at tgt[68] with no reload. Our C calls it with no arguments. Byte-neutral for this function at the current floor, but it is a genuine semantic discrepancy in the decompilation and should be revisited when func_8003AFFC itself is worked.

- [s2] [s2] `cheat_asm_stripped: 69` in the sandbox output is FILE-wide (other functions in code6cac_c2.c), not this function -- func_8003B9D0's own candidate body contains zero `__asm__`.

- [s2] [s2] src/code6cac_c2.c was restored to HEAD at end of session; the improved form lives in memory/grind/func_8003B9D0/candidate.c. Working tree carries only the ledger updates and metrics/events.jsonl.

- [s2] [s2] PROVENANCE: an earlier session-2 attempt was discarded by the driver for never writing its outcome JSON, but left candidate.c and two rejected/ forms on disk uncommitted. This session re-applied and independently re-measured that candidate before continuing, so the floor-6 claim rests on a measurement taken this session.


## Session 3 (structural, 2026-08-11)

### Floor
- **sandbox --disable all: 6** (target_insns 185, build_insns 188) - UNCHANGED.
  Re-measured this session with `candidate.c` applied to `src/code6cac_c2.c`
  before any probing; `src/` was restored to HEAD at the end of the session.

### THE BIG FINDING - region A IS closable; the first form that closes it
`memory/grind/func_8003B9D0/rejected/cse-boundary-diamond-closes-region-a-but-moves-magic-and-la.c`
scores **11 with build_insns 185 == target_insns 185**, and objdump of
`tmp/sandbox/func_8003B9D0/code6cac_c2.o` shows target's exact region-A shape at
all three sites:
```
lh  s1,1100(s2)     sh  v0,1100(s2)     sh  s1,1100(s2)
```
Both ingredients are required (measured separately - see hypotheses H3):
1. the pointer `eda` is assigned BEFORE the `((u8*)D_800A3878)[3] & 0x80` test,
   so its definition sits in an EARLIER cse basic block than the displaced uses; and
2. that test is spelled `if (...) { ... } else { magic = 0x80190800; }` - the
   `else` arm is what ENDS the cse basic block (the arm ends in an unconditional
   jump + BARRIER, so `cse_end_of_basic_block` cannot AROUND-extend past the join
   label, cse.c:8102-8184).  With the same hoist but the test left as a plain
   `if`, the fold comes straight back (measured 10 / build_insns 187 - which
   reproduces AND explains session-2's K6 number).

### Why it still scores 11 instead of 0 - exactly two residual clusters
Normalized diff (185 vs 185, **128-insn byte-equal tail**):
- **magic placement, ~7 points.**  Target materialises `magic = 0x80190800` in
  the PROLOGUE - tgt[1..3] `sw s3,44(sp)` / `lui s3,0x8019` / `ori s3,s3,0x800` -
  so the original C initialises `magic` before everything and the 0x80 test only
  OVERWRITES it, i.e. a PLAIN `if`.  The if/else needed for ingredient 2 sinks
  the lui/ori into the else arm (mine[47..48]) and adds the `j` around it
  (mine[45]).
- **`la` placement, ~4 points.**  Target materialises the base INSIDE the
  `qf & 0x30` block (tgt[55..56]); ingredient 1 forces it to mine[38..39], where
  cc1's scheduler drops it into the load-delay slot target leaves as a `nop`
  (tgt[40]).

Those two costs are exactly the price of the boundary, and they are what the next
session has to buy more cheaply.  **Region A itself is no longer the open
problem - its placement side-effects are.**

### The proven-spelling sibling census (new tool)
`tmp/grind/func_8003B9D0/s3/find_sibling.py` scans every `asm/funcs/*.s` for a
symbol address materialised into a register (`lui %hi` + `addiu %lo`) that is
then used with a NON-ZERO displacement, and intersects with the unqueued +
ruleless (= MATCHED) set.  **60 functions have the shape; 30 of them are
matched.**  The closest analogue is `func_8002BC68` (`src/code6cac_b.c:745`),
which uses the SAME 0x44C displacement off the neighbouring symbol D_80101EC8:
```c
t2_base = &D_80101EC8;
t3_base = t2_base + 0x44C;
...
*((s32 *) (t2_base + 0x134)) = ...;      /* emits sw $x,308($base) - NOT folded */
```
Its pointer is defined at the top of the function and its uses come after a large
if/else diamond - i.e. it is an instance of the SAME cse-boundary mechanism, not
a different C spelling.  This kills the "there is a magic pointer spelling"
theory outright.

### Mini-TU sweep (the cheap gradient - region A reproduces in ~20 lines)
`tmp/grind/func_8003B9D0/s3/mini_*.c` + `sweep.sh` (cpp -> cc1 with the exact
build flags -> count `%lo(D_80101EDA+1100)` folds vs `1100($reg)`).  Every
SPELLING variant folds; only the GEOMETRY variant does not:

| variant | fold? |
|---|---|
| `s16 *eda = &D_80101EDA;` + `eda[0x226]` (candidate shape) | folded |
| `s16 *far = eda + 0x226;` (K1 shape) | folded |
| struct pointer `T *t` with the member at +1100 | folded |
| pointer declared at function top, assigned in the block | folded |
| `s32 i = 0x226; eda[i]` | folded |
| `*(s16 *)((u8 *)eda + 0x44C)` byte-cast | folded |
| `u8 *` base + byte displacement (the func_8002BC68 spelling) | folded |
| `u8 *` base + derived `u8 *edf = eda + 0x44C` | folded |
| pointer defined before the block, uses after a CALL | folded |
| pointer defined before the block, no diamond | folded |
| **pointer defined before an if/ELSE diamond, uses after** | **NOT folded - `1100($17)`** |
| (control) base arrives as a function PARAMETER | not folded |

So the fold is decided by ONE thing only: whether the base pseudo still carries a
`qty_const` in the cse basic block where the displaced MEM lives.  No respelling
of the ACCESS changes that; only a cse basic-block boundary between the
definition and the use does.

- [s3] sandbox --disable all == 6 re-measured this session on candidate.c applied to src/code6cac_c2.c (target 185, build 188, rules_dropped 1); src restored to HEAD at end of session.

- [s3] REGION A IS CLOSABLE IN PURE C. Measured form (rejected/cse-boundary-diamond-closes-region-a-but-moves-magic-and-la.c): sandbox 11, build_insns 185 == target 185, and objdump shows target's exact shape `lh s1,1100(s2)` / `sh v0,1100(s2)` / `sh s1,1100(s2)` at all three displaced sites. Two required ingredients: (1) `eda` assigned BEFORE the 0x80 test, (2) that test spelled as if/ELSE. Neither alone works.

- [s3] The if/ELSE is load-bearing, not incidental: the same hoist with the 0x80 test left as a plain `if` measures 10 / build_insns 187 and region A folds again. This reproduces and EXPLAINS session-2's K6 number (10) -- K6 failed because it lacked the boundary, not because hoisting the pointer is inherently wrong.

- [s3] The residual 11 of the region-A-closing form is exactly two placement clusters and NOTHING else (128-insn byte-equal tail): ~7 points because target materialises `magic = 0x80190800` in the PROLOGUE (tgt[1..3] sw s3,44(sp) / lui s3,0x8019 / ori s3,s3,0x800), which means the original C used a PLAIN `if` there and the if/ELSE sinks the constant into the else arm plus a `j`; and ~4 points because target materialises the base INSIDE the qf&0x30 block (tgt[55..56]) while the hoist puts it at mine[38..39], filling the load-delay slot target leaves as a nop (tgt[40]).

- [s3] SIBLING CENSUS (tmp/grind/func_8003B9D0/s3/find_sibling.py): 60 functions in asm/funcs emit symbol-base + non-zero-displacement addressing; 30 of them are MATCHED (unqueued and ruleless). The closest analogue, func_8002BC68 (src/code6cac_b.c:745), uses the SAME 0x44C displacement off the neighbouring symbol D_80101EC8 via `u8 *t2_base = &D_80101EC8; *((s32 *)(t2_base + 0x134))` -- and its pointer is defined before a large if/else diamond. It is an instance of the same cse-boundary mechanism, NOT a different access spelling. There is no spelling to copy.

- [s3] MINI-TU REPRODUCTION (tmp/grind/func_8003B9D0/s3/mini_*.c + sweep.sh, cpp -> cc1 with exact build flags): region A reproduces in a ~20-line TU, so the gradient is readable without sandbox round-trips. Ten access/declaration spellings ALL fold (array index, derived pointer, struct member, u8* + byte offset, byte-cast, index variable, declaration-order, intervening call, plain hoist); only the if/ELSE-diamond geometry does not fold, and the function-parameter control also does not fold. CLOSED AXIS: no respelling of the ACCESS or of the pointer's TYPE can suppress the fold.

- [s3] sandbox --disable all == 6 re-measured this session with candidate.c applied to src/code6cac_c2.c (target_insns 185, build_insns 188, rules_dropped 1). src/ was restored to HEAD at end of session; the working tree carries only the ledger updates, the new rejected/ form and metrics/events.jsonl.

- [s3] REGION A IS CLOSABLE IN PURE C. The banked form rejected/cse-boundary-diamond-closes-region-a-but-moves-magic-and-la.c measures sandbox 11 with build_insns 185 == target_insns 185, and objdump of tmp/sandbox/func_8003B9D0/code6cac_c2.o shows target's exact shape at all three displaced sites: lh s1,1100(s2) / sh v0,1100(s2) / sh s1,1100(s2).

- [s3] Two ingredients are BOTH required for that closure: (1) `eda` assigned BEFORE the ((u8*)D_800A3878)[3] & 0x80 test, so its definition is in an earlier cse basic block than the displaced uses; (2) that test spelled as if/ELSE, whose arm ends in an unconditional jump + BARRIER so cse_end_of_basic_block cannot AROUND-extend past the join label. With the hoist but a plain `if`, the fold returns (measured 10 / build_insns 187) -- which reproduces and explains session-2's K6 number.

- [s3] The residual 11 of that form is exactly two placement clusters and nothing else (128-insn byte-equal tail): ~7 points because target materialises `magic = 0x80190800` in the PROLOGUE (tgt[1..3] sw s3,44(sp) / lui s3,0x8019 / ori s3,s3,0x800), proving the original C used a PLAIN `if` there while the if/ELSE sinks the lui/ori into the else arm plus a `j`; and ~4 points because target materialises the base INSIDE the qf&0x30 block (tgt[55..56]) while the hoist puts it at mine[38..39], filling the load-delay slot target leaves as a nop (tgt[40]).

- [s3] CLOSED AXIS -- access spelling and pointer type. Ten spellings measured in the mini TU (array index, derived pointer, struct member, u8* + byte offset, byte-cast, index-in-a-local, declaration-order, intervening call, plain hoist, plus the K1 shape) ALL fold. The lever is geometry only.

- [s3] CLOSED AXIS -- copy a matched sibling. 60 functions in asm/funcs emit symbol-base + non-zero-displacement addressing and 30 of them are matched (unqueued and ruleless), but the closest analogue func_8002BC68 (src/code6cac_b.c:745, SAME 0x44C displacement off the neighbouring symbol D_80101EC8) owes its shape to the same cse-boundary geometry, not to a special spelling.

- [s3] NEW REUSABLE INSTRUMENT -- region A reproduces in a ~20-line TU (tmp/grind/func_8003B9D0/s3/mini_*.c + sweep.sh), so the fold gradient is readable directly from cc1 output in seconds instead of via sandbox round-trips. Any future boundary candidate should be screened there first.

- [s3] NEW REUSABLE TOOL -- tmp/grind/func_8003B9D0/s3/find_sibling.py (symbol-base + non-zero-displacement census across asm/funcs, filtered to the matched set). Generally useful for locating proven-spelling siblings for any addressing-shape residual.

- [s3] The three cheat-asm constructs deleted in session 2 stayed deleted; no __asm__ and no new construct of any kind was introduced this session. src/code6cac_c2.c is byte-identical to HEAD.


## Session 4 (permuter, 2026-08-11)

### FLOOR: 0 — the honest pure-C distance is CLOSED
`sandbox func_8003B9D0 --disable all` == **0** (target_insns 185,
build_insns 185, rules_dropped 1, cheat-asm stripping on), measured this
session with the body in `memory/grind/func_8003B9D0/candidate.c` applied to
`src/code6cac_c2.c`.  Session floors: s1 21 -> s2 6 -> s3 6 -> **s4 0**.

### What closed region A
One statement pair, replacing `saved_44c = eda[0x226];`:

```c
p = (u8 *)&eda[0x226];      /* the function's existing u8 * scratch local */
saved_44c = *(s16 *)p;
```

With that, ALL THREE displaced sites keep target's register+displacement
addressing (`lh s2,1100(s0)` / `sh v0,1100(s0)` / `sh s2,1100(s0)` — the two
stores still spell `eda[0x226]`), the `la` stays INSIDE the `qf & 0x30` block
where target puts it (tgt[55..56]), `magic` stays in the prologue, and
build_insns drops 188 -> 185 == target.  The construct is `/* FAKE */`-
annotated; the vet is `memory/grind/func_8003B9D0/self_vet.md`.

### Provenance (permuter finds are PROPOSALS)
Two campaigns ran from two chassis:
  * wsA = the floor-6 candidate (permuter base score 330), and
  * wsB = the session-3 region-A-closing if/ELSE form (base score 748).
wsA produced `output-200-1` and `output-200-2` (score 330 -> 200) within ~5
minutes; both were REJECTED as submitted forms — `output-200-1` moved
`eda = &D_80101EDA;` to AFTER the block (uninitialised read = UB) and
`output-200-2` read a BYTE through `p` (`saved_44c = *p;`) where target does
`lh`.  Their shared, extractable signal was that routing the far address
through `p` unfolds region A.  The halfword-correct derivation was written by
hand and re-measured on the real pipeline.  wsB produced nothing better than
578 (base 748) in ~21 minutes and was harvested + stopped; the region-A-closing
diamond chassis is a WORSE permuter basin than the floor-6 chassis.

### Fold-suppression and instruction count are TWO separate effects
Measured over 13 spellings this session (full-TU compile, exact Makefile
flags; `tmp/grind/func_8003B9D0/s4/sweep2.py`, `sweep3.py`):
  * ANY derived-address pointer local suppresses the cse symbol fold — all
    three sites come out as `1100($base)` — including a block-scope
    `s16 *far`, an init-at-declaration `s16 *far = &eda[0x226];`, a
    `u8 *far` + `(s16 *)` cast, and a function-scope `s16 *far`.
  * But every DEDICATED local costs 3 extra instructions (188 vs target 185)
    and sandboxes at 6, no better than the session-3 floor.
  * ONLY staging the address through the function's already-live `u8 *p`
    scratch local reaches 185 == target and sandbox 0.
This corrects the session-1 K1 reading: K1's `s16 *eda_alt = eda + 0x226;`
did not fail because the fold survived, it failed on instruction count.

### The session-3 F1 lever is dead on target's own evidence
Session 3 proposed hunting for a cse basic-block boundary produced by a join
label with `LABEL_NUSES != 1` (a short-circuit `&&`/`||`).  Counting every
label reference in `asm/funcs/func_8003B9D0.s`: all 13 labels appear exactly
twice (one definition + one use), so target has NO multi-use join label
anywhere.  Any form introducing one emits control flow target does not have.
KILLED — and moot, since region A closed without any boundary at all.

- [s4] sandbox --disable all == 0 (target_insns 185, build_insns 185, rules_dropped 1, cheat_asm_stripped 69 file-wide, zero __asm__ in this function's body). Floor history 21 -> 6 -> 6 -> 0.

- [s4] REGION A CLOSED by staging the +0x44C address through the function's existing `u8 *p` scratch local (`p = (u8 *)&eda[0x226]; saved_44c = *(s16 *)p;`) in place of `saved_44c = eda[0x226];`. All three displaced sites keep target's `1100($s0)` addressing, the `la` stays inside the qf&0x30 block, `magic` stays in the prologue, and build_insns goes 188 -> 185 == target. FAKE-annotated in src; sanctioned family = variable reuse for codegen control (.claude/rules/no-new-park-categories.md:170).

- [s4] Fold suppression and instruction count are INDEPENDENT effects. Thirteen spellings measured on the full-TU pipeline: every derived-address pointer local (block-scope split decl/assign, init-at-decl, assign-before-the-zero-read, u8* + cast, function-scope) suppresses the cse fold and yields 3 register+displacement sites, but all of them compile to 188 insns and sandbox 6. Only reuse of the already-live scratch pointer reaches 185/0. This corrects session-1 K1's diagnosis: K1 failed on length, not on the fold.

- [s4] Target has NO multi-use join label: every one of the 13 `.L8003B*` labels in asm/funcs/func_8003B9D0.s appears exactly twice (definition + single use), so `LABEL_NUSES != 1` (the short-circuit `&&`/`||` boundary the session-3 frontier proposed) cannot exist in the original control flow. That frontier item is KILLED, and moot: region A closed with no cse basic-block boundary at all, with the `la` in target's own position.

- [s4] The permuter campaign on the region-A-closing if/ELSE chassis (wsB, base score 748) is a strictly WORSE basin than the floor-6 chassis (wsA, base 330): ~21 minutes and ~30k iterations produced nothing better than 578. Harvested and stopped. Both campaigns are stopped; no campaign outlived the session.

- [s4] Both permuter finds were rejected AS WRITTEN and re-derived by hand: output-200-1 relocated `eda = &D_80101EDA;` to after its uses (undefined behaviour) and output-200-2 read a byte where target reads a halfword. The usable signal was the p-staging shape, not either form.

- [s4] REMAINING FOR THE OPERATOR (integration, not a matching problem): regfix.txt:1116 still carries `func_8003B9D0: fill_delay @ 49 <- 52`. The sandbox drops it (rules_dropped 1) and still scores 0, so it is now dead weight; retiring it is a regfix.txt edit, which grind sessions may not make.

## SESSION 4 RE-RUN (permuter modality, 2026-08-11) — validator bounce, re-verified

The first session-4 attempt was DISCARDED by the driver validator on a
MECHANICAL self-vet defect, not on its work: `grindlib._FAMILY_BLOCK`
(`^\s*FAMILY\s*:`) matched a wrapped prose line inside the T5 answer
("It is NOT any forbidden\nfamily: no register pin, ..."), so the file was read
as claiming THREE sanctioned families while quoting only two SCOPE sentences.
The lesson generalises: in self_vet.md never let a line begin with `FAMILY:`,
`SCOPE:` or `PRECEDENT:` unless it IS one of those fields — the validator is a
line-anchored regex and prose wrapping can forge a field.

This session restored `memory/grind/func_8003B9D0/candidate.c` into
`src/code6cac_c2.c` (HEAD still carried the three cheat-asm constructs and the
`x = -1; if (...)` region-B form — the discarded session's src edits had been
reverted) and INDEPENDENTLY RE-MEASURED it:

    sandbox func_8003B9D0 --disable all
    -> score 0, target_insns 185, build_insns 185, rules_dropped 1,
       strip_cheat_asm true, cheat_asm_stripped 69 (all in OTHER functions of
       the TU; func_8003B9D0's own body carries zero cheat-asm)

So the honest, cheat-free distance is 0 with the candidate body in place, and
the H4 finding is now confirmed by two independent measurements taken in
separate sessions. `self_vet.md` was rewritten (same substance, prose rewrapped
so no forged field lines remain, second family's SCOPE sentence quoted verbatim
from `.claude/rules/pointer-alias-fake-exception.md:13`) and now passes
`grindlib.validate_self_vet` -> `(True, '')`.

Integration state unchanged: `regfix.txt:1116` still carries
`func_8003B9D0: fill_delay @ 49 <- 52`, which the sandbox drops. Retiring it +
a full-build SHA1 verify is an operator step (grind sessions may not edit
regfix.txt).

## SESSION 4-FORENSICS (2026-08-11) — the region-A fold is named, and session-4's K10 is RETRACTED

### Starting state and floor
The layer-1 cheat-reviewer FAILED the previous session's D3 construct
(`p = (u8 *)&eda[0x226]; saved_44c = *(s16 *)p;`) and the driver BANNED it.
This session started from candidate-minus-D3 (keep D1 = the three cheat-asm
deletions, keep D2 = the if/else region-B form) applied to
`src/code6cac_c2.c` and measured `sandbox func_8003B9D0 --disable all`:

    score 6, target_insns 185, build_insns 188, rules_dropped 1,
    strip_cheat_asm true

So the honest LEGAL floor is **6**, unchanged from sessions 2/3. `src/` was
restored to HEAD at end of session; the legal body is `candidate.c` and the
banned body is `rejected/p-staging-layer1-cheat-banned.c`.

### RETRACTION — session 4's K10 rested on a broken detector
Session 4's sweep scripts (`tmp/grind/func_8003B9D0/s4/sweep2.py`,
`sweep3.py`) classified a site as "unfolded register+displacement" by counting
`1100\(s?[0-9a-z]+\)` in `objdump -d` of an UNLINKED `.o`.  That regex cannot
distinguish the two forms: an unrelocated `lh $2,%lo(D_80101EDA+1100)($2)`
prints as `lh s2,1100(s2)` exactly like the wanted `lh s2,1100(s0)`.  The
distinguishing feature is the PRECEDING `lui`, which the regex never looked at.

Re-dumped this session with `tmp/grind/func_8003B9D0/s4b/dumpfn.sh` (full
mnemonic listing of the function):
  * `B_fnscope_far_read` (dedicated function-scope `s16 *far`, read only):
    **188 insns, ALL THREE SITES FOLDED** — `lui s2,0x0 / lh s2,1100(s2)`,
    `lui at,0x0 / sh v0,1100(at)`, `lui at,0x0 / sh s2,1100(at)`.
  * `A_fnscope_far_all3` (dedicated `s16 *far` used at all three sites):
    **188 insns, ALL THREE SITES FOLDED** — byte-identical region to B.
  * `C_p_reuse` (the banned D3 form): 185 insns, all three sites
    `1100($s0)` with no `lui`.

So the dedicated-pointer variants do NOT "suppress the fold at a 3-instruction
cost"; they simply fold, exactly like the plain `eda[0x226]` baseline, and the
188 is the baseline 185 + 3 folds.  Session-1 K1's original diagnosis was
right; session-4's "correction" of it was the artefact.
**Generalisable tooling lesson: never test for a %lo symbol fold by looking at
the displacement in an unlinked object — look for the `lui`, or read the `.s`.**

### THE PASS AND THE DECISION (the mandate of this modality)
Per-pass RTL dumps for B (folds) and C (does not) were produced with the exact
build flags plus `-da` (`tmp/grind/func_8003B9D0/s4b/da_B_fnscope_far_read/`,
`.../da_C_p_reuse/`) and compared with `firstdiv.py` (normalises pseudo numbers
and insn UIDs so pure renumbering is not a difference):

    rtl SAME | jump SAME | cse DIVERGE | loop DIVERGE | cse2 DIVERGE | ...

1. **The initial RTL of B and C is IDENTICAL modulo pseudo numbering.**  Both
   emit `(set (reg/v P) (plus (reg/v eda) (const_int 1100)))` followed by
   `(mem:HI (reg/v P))`.  The C-source difference (dedicated local vs reuse of
   the existing `u8 *p`) is invisible to the front end.
2. **cse1 does NOT fold, in either variant.**  In `.cse` all three sites are
   `(mem (plus (reg eda) (const_int 1100)))` in B and in C.  The one and only
   difference at `.cse` is that C still contains
   `(insn 97 (set (reg/v 78) (plus (reg/v 94) (const_int 1100)))
        (expr_list:REG_EQUAL (const:SI (plus (symbol_ref "D_80101EDA") 1100))))`
   while B does not.
3. **`loop` changes nothing** for either variant (`.loop` == `.cse` at these
   sites).
4. **cse2 (the SECOND cse pass, toplev.c:2926) performs the fold.**  At `.cse2`
   B's three sites have become
   `(mem (const (plus (symbol_ref "D_80101EDA") (const_int 1100))))`
   while C's are still `(mem (plus (reg 94) (const_int 1100)))`.  Nothing after
   cse2 (flow / combine / sched / lreg / greg / jump2 / sched2 / dbr) changes
   either shape — `.dbr` still shows `(mem (const (plus sym 1100)))` for B and
   `(mem (plus (reg 16 s0) 1100))` for C.
5. **What decides cse2's behaviour is whether insn 97 still exists**, and that
   is decided by **`delete_dead_from_cse` (cse.c:8683), called at toplev.c:2867
   immediately after cse1 and NOT after cse2.**  It builds a WHOLE-FUNCTION
   reference count (`count_reg_usage`, cse.c:8595) and deletes any single-SET
   insn whose destination pseudo has `counts[REGNO] == 0` (cse.c:8731-8734).
   * In B, cse1 substituted `far`'s value into all three MEMs, `far`'s count
     fell to 0, the set was deleted, and cse2 therefore saw a bare
     `(plus (reg eda) 1100)` with no equivalence class — `find_best_addr`
     (cse.c:2659-2663) folded it unconditionally through `fold_rtx`'s
     `qty_const` substitution (cse.c:5171-5179).
   * In C the destination is `p`, which is ASSIGNED AGAIN later in the function
     (`p = (u8 *)D_800A3878;`) and read there, so its whole-function count is
     non-zero, the dead set survives cse1, and in cse2 it re-establishes the
     equivalence that keeps the cheap register+displacement address at all
     three sites.
6. **The surviving insn emits no bytes.**  `flow` deletes it (the `const_int
   1100` count in C's dumps goes 5 -> 3 between `.cse2` and `.flow`), and it is
   absent from `.dbr`.  So the banned construct is, mechanically, "dead in the
   emitted output but its existence in source changed the codegen decisions
   upstream of DCE" — the policy's own definition of a cheat-by-spelling
   (.claude/rules/no-new-park-categories.md).  The layer-1 FAIL is confirmed on
   mechanism, not merely on style.

### What this closes and what it opens
CLOSED: "a dedicated pointer local for the derived address" — every spelling of
it (block scope, function scope, read-only, all-three-sites, `u8 *` + cast) is
dead for the same reason, and the reason is now known rather than measured
one variant at a time: cse1 propagates it, `delete_dead_from_cse` deletes it,
cse2 folds.  No further dedicated-pointer variants need to be measured.

OPEN (see hypotheses.md F1/F2): target's shape requires ONE of
  (a) a pseudo holding `(plus eda 1100)` whose whole-function reference count
      is non-zero for a reason a programmer would write — i.e. genuinely
      multi-assigned or used as a value, not as an address (this is the
      "variable reuse for codegen control" family, and the one instance tried
      so far was FAILed at layer 1); or
  (b) a cse2 basic-block boundary between `eda`'s definition and the displaced
      uses (session-3's if/ELSE diamond: measured 11, costs `magic` and `la`
      placement); or
  (c) some construct that removes `eda`'s `qty_const` in cse2 without moving
      its `la` — no candidate identified yet.

- [s4-forensics] LEGAL floor re-measured this session: sandbox --disable all == 6 (target 185, build 188, rules_dropped 1) with candidate-minus-D3 in src/code6cac_c2.c. src restored to HEAD at end of session.

- [s4-forensics] RETRACTION of session-4 K10: the sweep2/sweep3 fold detector counted `1100($reg)` in an UNLINKED objdump, which is identical text for a folded `%lo(D_80101EDA+1100)($reg)` access and an unfolded register+displacement access. Re-dumping the full mnemonic listing shows BOTH dedicated-pointer variants (function-scope `s16 *far` read-only, and `s16 *far` used at all three sites) FOLD at all three sites and are 188 insns = 185 + 3 folds. Dedicated pointers never suppressed the fold; session-1 K1's original diagnosis was correct.

- [s4-forensics] THE FOLD IS PERFORMED BY cse2, NOT cse1. Per-pass `-da` dumps (tmp/grind/func_8003B9D0/s4b/da_*/): `.rtl` and `.jump` are identical between the folding and non-folding variants modulo pseudo numbering; `.cse` and `.loop` leave all three sites as `(mem (plus (reg eda) (const_int 1100)))` in BOTH; `.cse2` is where the folding variant becomes `(mem (const (plus (symbol_ref "D_80101EDA") (const_int 1100))))`; every later pass preserves whichever shape cse2 left.

- [s4-forensics] THE DECIDING DECISION IS `delete_dead_from_cse` (cse.c:8683, called from toplev.c:2867 right after cse1 and NOT after cse2). It builds a WHOLE-FUNCTION reference count via count_reg_usage (cse.c:8595) and deletes any single-SET insn whose destination pseudo has count 0 (cse.c:8731-8734). cse1 propagates the derived-address pointer into the three MEMs; a DEDICATED pointer then has count 0 and its set is deleted, so cse2 sees a bare (plus (reg) 1100) with no equivalence class and folds it via find_best_addr (cse.c:2659-2663) + fold_rtx's qty_const substitution (cse.c:5171-5179). A pointer that is ASSIGNED AGAIN later in the function keeps a non-zero count, its dead set survives into cse2, and the cheap register+displacement address is retained at all three sites.

- [s4-forensics] The insn that does the work emits NO BYTES: it is deleted by `flow` (const_int 1100 occurrences go 5 -> 3 between .cse2 and .flow) and is absent from .dbr. So the banned D3 construct is mechanically "dead in the emitted output, but its existence in source changed codegen upstream of DCE" -- the policy's own definition of a cheat-by-spelling. The layer-1 FAIL is confirmed on mechanism.

- [s4-forensics] TOOLING LESSON (generalises beyond this function): never test for a %lo symbol fold by looking at the displacement in an UNLINKED objdump -- the addend prints identically for the folded and unfolded forms. Test for the preceding `lui`, or read the cc1 `.s` output.

- [s4] LEGAL floor re-measured this session: `sandbox func_8003B9D0 --disable all` == 6 (target_insns 185, build_insns 188, rules_dropped 1, strip_cheat_asm true) with candidate-minus-D3 applied to src/code6cac_c2.c. src/ was restored to HEAD at end of session; the working tree carries only the ledger updates, the new rejected/ form and metrics/events.jsonl.

- [s4] The banned D3 body is preserved verbatim at memory/grind/func_8003B9D0/rejected/p-staging-layer1-cheat-banned.c; candidate.c is now the D3-free floor-6 body (zero __asm__, region B still closed by the session-2 if/else form); self_vet.md has been voided so its D3 claims cannot be reused.

- [s4] THE PASS: region A's fold is done by cse2 (the second cse pass, toplev.c:2926). cse1 leaves all three +0x44C sites as `(mem (plus (reg eda) (const_int 1100)))` in BOTH a folding and a non-folding variant; the `loop` pass changes nothing; the fold first appears in `.cse2` and no later pass alters it.

- [s4] THE DECISION: delete_dead_from_cse (tools/gcc-2.7.2/cse.c:8683), called at toplev.c:2867 right after cse1 and NOT after cse2, deletes any single-SET insn whose destination pseudo has a WHOLE-FUNCTION reference count of zero (count_reg_usage, cse.c:8595; test at cse.c:8731-8734). That deletion is what removes the derived-address set and hands cse2 an address with no equivalence class, which find_best_addr (cse.c:2659-2663) then folds unconditionally by substituting the base pseudo's qty_const in fold_rtx (cse.c:5171-5179).

- [s4] The insn that decides the outcome emits NO BYTES: in the non-folding variant it is deleted by `flow` (const_int 1100 occurrences drop 5 -> 3 between .cse2 and .flow) and is absent from .dbr. The banned construct therefore matches the policy's own definition of a cheat-by-spelling - dead in the emitted output, but its existence in source changed codegen upstream of DCE - so the layer-1 FAIL is confirmed on mechanism, not merely on style.

- [s4] CLOSED AXIS (on mechanism, not one probe at a time): a dedicated pointer local for the derived address, in ANY spelling (block scope, function scope, read-only, used at all three sites, u8* + cast, init-at-declaration). cse1 propagates it, delete_dead_from_cse deletes it, cse2 folds. No further dedicated-pointer variant needs to be measured.

- [s4] TOOLING LESSON, generalises beyond this function: never test for a %lo symbol fold by looking at the displacement in an UNLINKED objdump - the addend prints identically for the folded and unfolded forms. Test for the preceding `lui`, or read the cc1 `.s`. This artefact is what produced session-4's false K10.

- [s4] New reusable instruments: s4b/dumpfn.sh (function-scoped mnemonic listing from a .o), s4b/dump.sh (per-variant -da dumps with the exact build flags), s4b/firstdiv.py (first-diverging-pass finder with pseudo/UID normalisation - directly reusable for any two-variant codegen forensic), s4b/sites.sh + loopchk.sh + allpass.sh + trace1100.py (per-pass extraction of specific insns).

## SESSION 5 (forensics, 2026-08-11) — the fold is a COST/LOOKUP decision, and s4's mechanism is corrected for the baseline

### Floor
`sandbox func_8003B9D0 --disable all` == **6** (target_insns 185, build_insns 188,
rules_dropped 1, strip_cheat_asm true), measured this session with candidate.c
(the D3-free floor-6 body) applied to `src/code6cac_c2.c`.  `src/` was restored to
HEAD at the end of the session.

### CORRECTION to session-4-forensics H5 — for the CANDIDATE body the fold is cse1, not cse2
Session 4-forensics dumped only the two DERIVED-POINTER variants (B = dedicated
`s16 *far`, C = the banned `p`-reuse) and concluded "cse1 does not fold;
delete_dead_from_cse deletes the address set; cse2 folds".  That is true for those
two variants and FALSE for the actual floor-6 candidate.  Dumped this session
(`tmp/grind/func_8003B9D0/s5/da_base/`, derived from `s4/sw3/B_fnscope_far_read.i`
by editing the pointer out, so the cc1 flags and TU context are provably identical
to the s4b dumps):

* `.rtl` — the baseline emits **no address pseudo at all**: all three sites are
  `(mem/s:HI (plus:SI (reg/v:SI 94) (const_int 1100)))` directly (insns 97, 120, 130).
  H5's "the front end always emits `(set (reg P) (plus (reg eda) 1100))` +
  `(mem (reg P))`" holds only for spellings that NAME a pointer variable.
* `.cse` — **all three sites are ALREADY folded** to
  `(mem/s:HI (const:SI (plus:SI (symbol_ref:SI ("D_80101EDA")) (const_int 1100))))`.
* `.cse2`, `.flow`, `.combine`, `.dbr` — unchanged thereafter.

So for the candidate body `delete_dead_from_cse` plays NO role at all (there is
nothing to delete); cse1's `find_best_addr` folds directly.

### THE UNIFIED MECHANISM (one rule covering every variant measured in sessions 1-5)
Read out of the frozen compiler source (`tools/gcc-2.7.2/cse.c`,
`config/mips/mips.h`, `config/mips/mips.c`):

1. `find_best_addr` (cse.c:2621) folds any non-REG address unconditionally
   (cse.c:2663-2665); `fold_rtx` substitutes the base pseudo's `qty_const`
   (cse.c:5170-5180) giving `(const (plus sym 1100))`.
2. It then does `lookup (addr, hash, Pmode)` (cse.c:2680) and, under `ADDRESS_COST`,
   walks the equivalence class choosing the member with the LOWEST `ADDRESS_COST`,
   tie-broken by the HIGHEST `rtx_cost` (cse.c:2698-2739).
3. MIPS costs (`mips.h:2897` — `ADDRESS_COST(A) = REG_P(A) ? 1 : mips_address_cost(A)`;
   `mips_address_cost` in mips.c):
   * `(plus (reg) (const_int 1100))` — SMALL_INT → **1**
   * bare `(reg)` — **1**
   * `(const (plus (symbol_ref) (const_int 1100)))` — SMALL_INT offset falls through
     to the SYMBOL_REF case → `SYMBOL_REF_FLAG ? 1 : 2` → **2** (our symbols are not
     small-data under `-G0`).
   The register+displacement form is therefore STRICTLY CHEAPER than the folded
   symbolic constant and `find_best_addr` RESTORES it — but only if that expression
   is in the cse hash table.  **The fold is not sticky; it is a lookup miss.**
4. Consequences, one rule for every variant:
   * baseline (`eda[0x226]`, no pointer): nothing ever puts `(plus (reg eda) 1100)`
     in the table → cse1 folds → 3 folds, 188 insns, sandbox 6.
   * dedicated pointer (`s16 *far = &eda[0x226]`, any scope or spelling): its SET
     puts the expression in the table, so cse1 RESTORES register+displacement at all
     three MEMs — which drops `far`'s whole-function reference count to 0,
     `delete_dead_from_cse` (cse.c:8683, called at toplev.c:2867) deletes the set
     (cse.c:8730-8734 — the only escapes are a non-pseudo dest, a non-zero count, or
     `side_effects_p`), and cse2 re-runs with an empty table → folds again.
   * the banned `p`-reuse: `p` is assigned again later, its count stays non-zero, the
     set survives, cse2's table still carries the expression → restored, 185 insns,
     sandbox 0.  The surviving set emits no bytes (flow deletes it) — exactly why
     layer 1 FAILed it.
   * a base pseudo with NO `qty_const` (a cse basic-block boundary, or a function
     parameter): step 1 cannot fold at all, and no table entry is needed.

### MEASURED PROBE — the session-4 F1 frontier item, KILLED with a number
Form: `s16 *eda` at FUNCTION scope, assigned unconditionally at the top, used by the
`qf & 0x30` block AND by the two later reads (`a3_arg = eda[0];`,
`a0_arg = eda[0x226];` — legitimate, since `D_80102326 == &D_80101EDA[0x226]`).  This
is the honest "one pointer genuinely serves both" shape the frontier asked for,
spelled so that it is NOT the undefined-behaviour form permuter output-200-1 produced.
**Result: sandbox 16 (build_insns 186) versus the floor of 6.  KILLED.**
objdump of `tmp/sandbox/func_8003B9D0/code6cac_c2.o` shows why:
```
   e8: lui  s1,0x0          \
   ec: lh   s1,1100(s1)      |  the three IN-BLOCK sites still FOLD
  110: lui  at,0x0           |
  114: sh   v0,1100(at)      |
  124: lui  at,0x0           |
  128: sh   s1,1100(at)     /
  170: lh   a0,1100(s2)     <-- register+displacement appears at the ONE site
                                target does NOT want it (target: lui a0,%hi(D_80102326))
```
A non-zero whole-function reference count is NOT sufficient: the reference must keep
the address expression in the cse hash table AT THE POINT the displaced MEM is
processed.  The later read sits past the region-B `if/else` join labels, which end the
cse basic block — so there `eda` has no `qty_const` and the fold cannot fire (mechanism
bullet 4, observed live), while inside the block `eda`'s constant is still current and
it folds.  Form banked at `rejected/fnscope-shared-eda-pointer-later-reads.c`.

### The two-route partition, now stated on mechanism
Target's three-site shape requires, at the cse pass that first sees a `+0x44C` MEM,
EITHER
  (A) `(plus (reg eda) (const_int 1100))` present in the cse hash table — which needs
      an address-valued SET whose destination pseudo survives `delete_dead_from_cse`,
      i.e. is referenced again while the same cse table is live.  Every such construct
      measured so far emits no bytes of its own (target is 185 insns and the fold-free
      forms are also 185), which is the policy's definition of dead-in-output codegen
      steering; OR
  (B) `eda` carrying no `qty_const` — a cse basic-block boundary between the `la` and
      the first displaced use — which target's own instruction stream forbids
      (`lui/addiu $s0` at tgt[55..56], `lh $s1,0($s0)` at tgt[57],
      `lh $s2,0x44C($s0)` at tgt[58]; no label, no barrier, no call in between).
Session-3's if/ELSE diamond buys (B) by moving the `la` OUT of the block (measured 11:
`magic` placement ~7 + `la` placement ~4).  No third route has been identified:
`use_related_value` (cse.c:1781) is called ONLY at cse.c:6535, for a SET's source
constant, never for a MEM address, and `find_best_addr` returns immediately for
`CONSTANT_ADDRESS_P` addresses (cse.c:2656) — which is also the mechanism proof of
session-2's K5 (plain global accesses can never become `1100($s0)`).

- [s5] LEGAL floor re-measured this session: sandbox --disable all == 6 (target_insns 185, build_insns 188, rules_dropped 1, strip_cheat_asm true) with candidate.c applied to src/code6cac_c2.c. src/ restored to HEAD at end of session.

- [s5] CORRECTION to s4-forensics H5: for the ACTUAL candidate body the region-A fold is performed by cse1, not cse2, and delete_dead_from_cse is irrelevant to it. The baseline .rtl emits (mem/s:HI (plus:SI (reg/v:SI 94) (const_int 1100))) with NO address pseudo (insns 97/120/130) and the .cse dump already shows all three sites as (mem/s:HI (const:SI (plus:SI (symbol_ref "D_80101EDA") (const_int 1100)))). H5's "the front end always emits a (set (reg P) (plus (reg eda) 1100))" holds only for spellings that NAME a pointer variable. Dumps: tmp/grind/func_8003B9D0/s5/da_base/tu.i.{rtl,cse,cse2,flow,combine,dbr}.

- [s5] THE FOLD IS A COST/LOOKUP DECISION, NOT A ONE-WAY TRANSFORM. find_best_addr folds the address (cse.c:2663) and then looks it up (cse.c:2680) and picks the equivalence-class member with the lowest ADDRESS_COST, tie-broken by highest rtx_cost (cse.c:2698-2739). On MIPS (mips.h:2897 + mips_address_cost): (plus reg small_int) costs 1, a bare reg costs 1, and (const (plus symbol_ref small_int)) costs 2 for a non-small-data symbol. Register+displacement is therefore STRICTLY cheaper and IS restored whenever (plus (reg eda) 1100) is in the cse hash table. Region A folds only because nothing in the honest C ever puts that expression in the table.

- [s5] ONE RULE EXPLAINS EVERY VARIANT MEASURED IN SESSIONS 1-5. Baseline (no pointer): expression never in the table, cse1 folds, 188/6. Dedicated pointer (any scope or spelling): its SET puts the expression in the table, cse1 restores register+displacement at all three MEMs, the pointer's whole-function reference count falls to 0, delete_dead_from_cse (cse.c:8683; escapes at cse.c:8730-8734 are only a non-pseudo dest, a non-zero count, or side_effects_p) deletes the set, cse2 re-folds with an empty table, 188/6. Banned p-reuse: the pointer is assigned again later, count stays non-zero, the set survives, cse2's table still carries the expression, restored, 185/0, and the surviving set emits no bytes. No qty_const on the base (cse basic-block boundary, or a function parameter): the fold cannot fire at all.

- [s5] KILLED with a measurement -- the s4-forensics F1 frontier item. A function-scope `s16 *eda` assigned unconditionally at the top and genuinely used by BOTH the qf&0x30 block and the two later reads (a3_arg = eda[0], a0_arg = eda[0x226], legitimate because D_80102326 == &D_80101EDA[0x226]) measures sandbox 16 / build_insns 186 versus the floor of 6. objdump shows the three IN-BLOCK sites still folded (lui + 1100) while `lh a0,1100(s2)` appears at the later read -- the one site where target re-materialises lui %hi(D_80102326). A non-zero whole-function reference count is NOT sufficient: the reference must keep the address expression in the cse hash table at the point the displaced MEM is processed, and the later read lives past the region-B if/else join labels where eda has no qty_const. Banked at rejected/fnscope-shared-eda-pointer-later-reads.c.

- [s5] MECHANISM PROOF for session-2's K5 (plain global accesses can never produce 1100($s0)): find_best_addr returns immediately for CONSTANT_ADDRESS_P addresses (cse.c:2656), and the only consumer of cse's related-value chains, use_related_value (cse.c:1781), is called ONLY at cse.c:6535 for a SET's source constant -- never for a MEM address. So no amount of la-holding elsewhere in the function can make a plain D_80102326 access reuse a base register.

- [s5] THE PARTITION, ON MECHANISM. Target's three-site register+displacement shape requires, at the cse pass that first sees a +0x44C MEM, EITHER (A) (plus (reg eda) 1100) present in the cse hash table -- which needs an address-valued SET whose destination survives delete_dead_from_cse, and every such construct measured emits no bytes of its own (target 185 == the fold-free forms 185), i.e. dead-in-output codegen steering; OR (B) eda carrying no qty_const, i.e. a cse basic-block boundary between the la and the first displaced use -- which target's own bytes forbid (lui/addiu $s0 at tgt[55..56], lh $s1,0($s0) at tgt[57], lh $s2,0x44C($s0) at tgt[58]: no label, no barrier, no call in between). Session-3's diamond buys (B) by moving the la out of the block, priced at 11.

- [s5] LEGAL floor re-measured this session: sandbox func_8003B9D0 --disable all == 6 (target_insns 185, build_insns 188, rules_dropped 1, strip_cheat_asm true, cheat_asm_stripped 69 file-wide) with memory/grind/func_8003B9D0/candidate.c applied to src/code6cac_c2.c. src/ was restored to HEAD at the end of the session; the working tree carries only the ledger updates, the new rejected/ form and metrics/events.jsonl.

- [s5] CORRECTION to session-4-forensics H5: for the actual candidate body the region-A fold is performed by cse1, not cse2, and delete_dead_from_cse is irrelevant to it. The baseline .rtl emits (mem/s:HI (plus:SI (reg/v:SI 94) (const_int 1100))) with NO address pseudo (insns 97/120/130) and .cse already shows all three sites folded. H5's premise that the front end always emits a (set (reg P) (plus (reg eda) 1100)) holds only for spellings that NAME a pointer variable.

- [s5] THE FOLD IS A COST/LOOKUP DECISION. find_best_addr folds the address (cse.c:2663-2665) and then looks it up (cse.c:2680) and picks the equivalence-class member with the lowest ADDRESS_COST, tie-broken by the highest rtx_cost (cse.c:2698-2739). On MIPS (mips.h:2897 + mips_address_cost) (plus reg small_int) costs 1, a bare reg costs 1, and (const (plus symbol_ref small_int)) costs 2 for a non-small-data symbol. Register+displacement is strictly cheaper and IS restored whenever (plus (reg eda) 1100) is in the cse hash table -- region A folds only because nothing in the honest C puts it there.

- [s5] ONE RULE NOW EXPLAINS EVERY VARIANT MEASURED IN SESSIONS 1-5. Baseline (no pointer): the expression is never in the table, cse1 folds, 188/6. Dedicated pointer (any scope or spelling): its SET puts the expression in the table, cse1 restores register+displacement at all three MEMs, the pointer's whole-function reference count falls to 0, delete_dead_from_cse deletes the set, cse2 re-folds with an empty table, 188/6. Banned p-reuse: the pointer is assigned again later, count stays non-zero, the set survives, cse2's table still carries the expression, restored, 185/0, and the surviving set emits no bytes. Base pseudo with no qty_const (cse basic-block boundary, or a function parameter): the fold cannot fire at all.

- [s5] KILLED with a measurement -- the session-4-forensics F1 frontier item. A function-scope s16 *eda assigned unconditionally at the top and genuinely used by BOTH the qf&0x30 block and the two later reads measures sandbox 16 / build_insns 186 against the floor of 6; the three in-block sites still fold while lh a0,1100(s2) appears at the later read, the one site target does not want it. A non-zero whole-function reference count is NOT sufficient -- the reference must keep the address expression in the cse hash table at the point the displaced MEM is processed.

- [s5] MECHANISM PROOF for session-2's K5: find_best_addr returns immediately for CONSTANT_ADDRESS_P addresses (cse.c:2656), and use_related_value (cse.c:1781) is called only at cse.c:6535 for a SET's source constant, never for a MEM address. Plain global accesses can never become 1100($s0).

- [s5] THE PARTITION, ON MECHANISM. Target's three-site register+displacement shape requires, at the cse pass that first sees a +0x44C MEM, EITHER (A) (plus (reg eda) 1100) present in the cse hash table -- which needs an address-valued SET whose destination survives delete_dead_from_cse, and every such construct measured so far emits no bytes of its own (target is 185 insns and the fold-free forms are also 185), i.e. dead-in-output codegen steering; OR (B) eda carrying no qty_const, i.e. a cse basic-block boundary between the la and the first displaced use -- which target's own bytes forbid (lui/addiu $s0 at tgt[55..56], lh $s1,0($s0) at tgt[57], lh $s2,0x44C($s0) at tgt[58]: no label, no barrier, no call in between). Session-3's if/ELSE diamond buys (B) by moving the la out of the block, priced at 11 (magic ~7 + la ~4).

- [s5] NEW REUSABLE INSTRUMENT: tmp/grind/func_8003B9D0/s5/mk.py derives a variant preprocessed TU by textual surgery on an existing .i, which guarantees byte-identical cc1 flags and TU context between the variant and any earlier dump -- much safer than re-running cpp with hand-reconstructed flags, and directly reusable for any two-variant codegen forensic.

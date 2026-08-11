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

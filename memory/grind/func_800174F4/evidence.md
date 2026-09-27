# Evidence — func_800174F4 (src/ings.c)

## Session 7 (manual, slotP, 2026-09-26) — stock-compiler floor 1 -> 0 (sandbox)

candidate.c = the new body; sandbox --disable all = 0, 136/136, 0 hunks
(stock build cc1, 2026-09-26). Not yet landed; two Ruling-11 locals pending.

- The ori is a SPELLING problem, not a compiler one. cc1psx (calibration,
  tools/cc1psx_wrapper.sh via tmp/func_800174F4/psx.sh) on the old body emits
  the same `ori $17,$2,0x0004` as our stock cc1; with an int loop limit both
  emit `addu $17,$2,4`. So the original did not narrow the loop limit to u16.
- Mechanism (stock cc1 -dc): `h = v0 + 4` into `unsigned short h` expands to
  117 `(set r88:HI (subreg:HI r86))`, 119 `(set r89 (plus (subreg:SI r88) 4))`,
  121 `(set h:HI (subreg:HI r89))`; combine merges 117+119+121 into
  `(set (subreg:SI h) (ior r86 4))` because nonzero_bits(r86) = 3 via
  reg_last_set_value (combine.c:3623 PLUS->IOR, :6887 last-set nonzero bits).
  With an int limit the plus has no narrowing partner: the only combine
  attempt (plus <- and, 2 insns, no split without i1, combine.c:1737) yields an
  unrecognisable IOR and is discarded, so the addiu survives. Every
  u16/u8/s16 spelling of the limit or of v0 measured ori (15 spellings,
  tmp/func_800174F4/v/v*.c).
- Frame: the natural `for (i = 0; i < n; i++)` leaves one combine-orphan
  reload slot (pseudo 124: the inverted entry test `(lt i n)` 3->2-combined
  with `i = 0`, newi2pat kept, its REG_DEAD note becomes `(use (reg 124))`;
  BB2_FRAME_DEBUG ctx=spill_new_p124 size=8). cc1psx gives the same 160 frame
  with the old 0x68 buffer. The old `u8 sp20[0x68]` was 0x5C (sizeof
  DRAWENV; isbg at +0x18) + 8 + align; with the real 0x5C buffer the orphan
  slot fills exactly that gap: frame 152 = target.
- Allocation: with the for loop, i gets $s0 and the limit $s1 with NO wraps
  (BB2_ALLOC_DEBUG: i p87 nrefs 8 ll 9 pri 26666 > limit p74 nrefs 8 ll 15
  pri 16000). The goto/do-while forms give i 7 refs (no entry-test ref) and
  lose $s0 when `i = 0` precedes the guard (15555 < 16000), or lose the rand
  jal delay slot when it follows (score 2). This is what the old
  do-while(0) wraps were papering over.
- 240: no local needed. `SetDefDrawEnv(env, 0, ..., 0x280, 0xF0)` and
  `0xF0 / (sel + 1)` both literal still seat 240 in $s1 (score 0); u16, s32,
  s16 and u8 `h` locals are all 0 too.
- Cleanups, each measured 0: `(D_800A36AC & 1) ? 0xF0 : 0` replaces the
  two-statement mask; the `s32 env` alias dropped; `u8 *prim`; case 20
  `u32 idx = cur` replaces the `(u32)` casts and `& 0xFF`;
  `if (0xF0 / (sel + 1) >= ++D_800A37C0)` replaces div_result/counter;
  `next == sel`. `next` itself is needed (dropping it: 1, 135 insns).
- Remaining multi-value locals (both measured necessary so far):
  `temp` = loop limit + case-20 table code. Split (G02): 5, 134 insns; the
  table code takes $a0 directly (it crosses no call, so it is never seated
  callee-saved) and the `move a0,s1` disappears. `sel` = switch selector +
  D_800A37A0. Split (G01): 6; the selector lands in $v1 (target $a1). Both
  split: 11. Ruling 11 (D) proofs not yet banked.

## Session 7b (2026-09-27) — landing-form candidate (still sandbox 0)

candidate.c = tmp/func_800174F4/v/M2.c: `DrawEnv env` (PsyQ DRAWENV layout,
typedef placed before the function) and `u32 ot[2]` replace the byte
buffers (0); the case-20 `u32 idx` copy dropped for `(u32)temp2 < cur` (0);
the two reused locals renamed `temp`/`temp2` with Ruling 11 (F) comments;
the both-arms `D_800A38F8 = cur + 1` store FAKE-annotated as F7 (hoisted
store: 6, 131 insns; `next` hoisted with a store per arm: 1, 135 insns).
Frame: vars = 8 (ot) + 96 (env) + 8 (reload slot of the for loop's
combined entry test, BB2_FRAME_DEBUG spill_new_p128) = 112 = target.
Other probes this session, all 0: `idx` kept as `u32 idx = cur`; the case-20
divide/counter spelled with locals; `& 0xFF` masks. Not 0: `u32 temp2` 10
(130 insns, dispatch turns unsigned), `u8 temp2` 6, `u32 cur` 10, `idx` only
10.

## Ruling 11 proof (session 7, 2026-09-27) — `temp` and `temp2`

Reuse spelling = candidate.c (tmp/func_800174F4/v/M2.c). One-variable-per-value
spelling = tmp/func_800174F4/v/M2_split.c: `temp` -> `count` (declared in the
case-1/2 fade block) + `code` (declared in the case-20 else arm); `temp2` ->
`mode` (function scope, the switch) + `limit` (declared in the case-20 block).
Same statement list; only declarations and identifiers differ.

Dumps (banked verbatim in r11_dumps.txt next to this file). Commands, repo
root, WSL: `bash tmp/func_800174F4/alloc.sh <cand>` (BB2_ALLOC_DEBUG=1,
tools/gcc-2.7.2/cc1, build CC_FLAGS, -dg -dl), `bash
tmp/func_800174F4/findreg.sh <cand> <pseudo>` (BB2_FINDREG_DEBUG=<pseudo>),
`bash tmp/func_800174F4/rtl.sh <cand> -dl -dg -df` (.lreg "Register N ..."
lines). The candidate is spliced over the INCLUDE_ASM line of src/ings.c.

### (A) fresh locals
Both are plain function-scope locals of func_800174F4 (the innermost scope
enclosing all their writes: `temp` is written in case 1/2 and in case 20,
`temp2` before the switch and in case 20). No `&temp`/`&temp2`. No other
declaration moved.

### (B) every write live, none redundant
`temp = (rand() & 3) + 4` is read by the loop test; `temp = D_800A37A8[cur]`
is read by `|=`/the call; `temp |= 0x8000` is read by the call.
`temp2 = g_disp_enable` is read by the switch; `temp2 = D_800A37A0` by the
compares and the divide. The two values of each variable sit on disjoint
paths (case 1/2 vs case 20; dispatch vs case 20), so no write stores a value
the variable already holds on every path: on the case-20 path `temp2` holds
g_disp_enable (== 20) before `temp2 = D_800A37A0`, and `temp` is
uninitialised on entry to case 20.

### (C) same statements, real values
(C)(2): M2.c and M2_split.c differ only in declarations and identifiers.
(C)(3): every value is a load or arithmetic present in the target bytes
(`andi/addiu s1` for the count, `lbu s1`/`ori s1` for the code, `lbu a1` for
each selector byte).

### (D)(1)-(2) the allocator decision, both spellings
`temp`:
- reuse: pseudo 72, `.lreg`: "used 8 times across 15 insns; crosses 1 call".
  find_reg conflicts {2,4,5,16,29}; allocated $17 (s1).
- split: `count` p88 "crosses 1 call" -> $17 (same as reuse); `code` p117
  "used 4 times across 5 insns" (no call crossed), conflicts {2,29},
  own_copy_prefs {4} -> $4 (a0).
- decision: global.c find_reg 970-975. An allocno with
  allocno_calls_crossed != 0 starts from call_used_reg_set, so only
  callee-saved registers are candidates (s0 is taken by `i` -> s1). An
  allocno crossing no call starts from fixed_reg_set, and its copy preference
  (hard_reg_copy_preferences, global.c 1097-1110; the `move a0,<code>`
  argument copy) seats it in $a0. The now-redundant argument move is then a
  no-op and is deleted: 134 insns, `lbu a0`/`ori a0` in place of target's
  `lbu s1`/`ori s1`/`move a0,s1`.
`temp2`:
- reuse: p74 "used 11 times across 28 insns", conflicts {2,3,17,29},
  someone_prefers {4} (the conflicting zero-extended `cur`, p101 = `andi a0,a2,0xff`, the table index, prefers a0) ->
  pass0 excludes 0-4 -> $5 (a1), target's register at the switch AND in case 20.
- split: `mode` p73 "used 6 times across 10 insns", conflicts {2,29},
  someone_prefers {} -> first free hard reg in numeric order (mips.h has no
  REG_ALLOC_ORDER) = $3 (v1); `limit` p99 conflicts {2,3,4,29},
  someone_prefers {4} -> $5 (a1). Case 20 is unchanged; only the dispatch
  selector moves to v1 (score 6, insns 28/30/31/33/35/41).
- decision: global.c find_reg 985-1001 (used1 |= hard_reg_conflicts; pass 0
  also excludes regs_someone_prefers).
Both values are global allocnos (live in more than one basic block: the
dispatch compare chain; the case-20 compare/divide/branches; the loop), so
local-alloc does not allocate them and its suggestion passes
(qty_phys_sugg / qty_phys_copy_sugg) never see them.

### (D)(3) necessity: the property, and why every split spelling lacks it
`temp`: the property is "the table code's pseudo crosses a call". The code
value's reads are the `|= 0x8000` and the argument of func_80060414; its
writes are the table load and the `|=`, all after the last call before
func_80060414 (case 20 has no earlier call). So a variable holding only that
value is live only between the load and the argument copy, whatever its
declaration order, scope or type, and allocno_calls_crossed is 0. It
therefore starts from fixed_reg_set, not call_used_reg_set, and its argument
copy gives it $a0 (or, with no preference, the lowest free call-clobbered
register). It can reach $s1 only by sharing a pseudo with a call-crossing
value, i.e. by the reuse.
`temp2`: the property is "the selector's pseudo is also live in case 20,
where v1 (limit+1, the divisor) is live and the cur allocno prefers a0".
A variable holding only the g_disp_enable byte is read only by the dispatch
compare chain (the cases re-load g_disp_enable into a1 themselves, as the
target does), so its live range is the dispatch block chain, where the only
other live values are the compare constants/flags in v0. Its conflict set is
{v0} and no conflicting allocno prefers a register, so find_reg returns $v1
regardless of priority order, declaration order, scope or type (u8: 6,
tmp/func_800174F4/v/X_sel_u8.c).

Sanctioned families, by name (each acts on the wrong quantity or is removed
before flow; measured where applicable; the X_* probes were measured on the
session's K8 twin, whose case 20 differs from M2 only by an extra `u32 idx`
copy of `cur`):
- dead store: a store never read is deleted by flow (life analysis), adds no
  liveness or conflict.
- self-assign: `code = code;` after the call / `mode = mode;` in case 20 is a
  no-op move deleted before flow: X_code_selfassign 5, X_sel_selfassign 6.
- cancelling pair: `code++; code--;` after the call, `mode++; mode--;` in
  case 20: both sets dead, deleted by flow: X_code_cancel 5, X_sel_cancel 6.
- chain-extender: adds reg_n_refs (priority) only; neither decision depends
  on priority (calls-crossed and conflicts/preferences decide), and the
  selector gets v1 at any priority because nothing conflicting holds v1.
- pointer alias: `u8 *tbl = D_800A37A8` 5 (X_code_tblalias); `*penable`
  for the selector 6 (X_sel_alias).
- do {} while (0): changes loop-depth ref weighting only: X_code_dowhile 7,
  X_sel_dowhile 6.
- duplicated-into-arms + cross-jump: the code value split into per-arm
  writes (`code = tbl|0x8000` / `code = tbl`) 8, 139 insns
  (X_code_arms); the selector has one write, nothing to duplicate.
- hoisting/sinking writes across branches: moving the D_800A37A0 load or
  the table load changes the instruction stream (a load in the dispatch
  block / before the stores), which the target does not have.
- post-allocation: jump2 cross-jump, sched2 and reorg do not change
  register assignment; the only post-allocation effect is the deletion of
  the no-op `move a0,a0` in the split `code` spelling (above).

### (D)(4) measured alternatives (sandbox --disable all)
- full one-variable-per-value (M2_split.c): 11, 134 insns.
- ablation (each variable split alone; each has two values): `temp` split
  5, 134 insns (M2_split_temp.c); `temp2` split 6 (M2_split_temp2.c).
- structural respellings of the split body: case 20 as nested ifs 11
  (Y_split_nestedif.c); switch as an if/else chain 21, 132 insns
  (Y_split_ifchain.c).
- permuter: campaign `split-all-s7` from the split body (the K8 twin): no
  zero in 18,209 iterations; its finds re-merge values into one variable.
  See "Ruling 11 (D)(4) permuter harvest".

### (E)/(F)
Names `temp`, `temp2` (Ruling 11 (E)(i)). Declaration comments name both
values of each and cite Ruling 11 and this section.

## Ruling 11 (D)(4) permuter harvest (2026-09-27)

Campaign `split-all-s7`: tmp/func_800174F4/perm_split (minimal-TU workspace,
build cc1 + maspsx recipe for src/ings.c, verified byte-identical to target
with the K8 reuse body), seeded from the one-variable-per-value body (K8
split twin), -j2, stack diffs on. Base 445 (permuter score). Stopped after
18,209 iterations; last new find 26 min after launch, then none for the rest
of the run (it overran: 39,825 s wall, harvested with --stop). No find
reached 0. Finds:
- 205: moves `D_800A37C0 = 0` below the `|=` (store reorder, not target
  order); 215: a `new_var` copy of the table load; 335/355: reorder the
  call or the D_800A38F8 store; 345: `short code` (changes the value passed).
- REUSE finds: 415 `mode = sel;` and 425 `sel = (mode = D_800A37A0)` (the
  selector variable re-used for D_800A37A0 — the `temp2` reuse),
  445-3 `mode = idx`, 445-2 `mode = 0x8000` (constant holder), 430 a
  `new_var` + do-while(0).
The permuter's own improvements on the split body come from merging values
back into one variable, never from a split spelling.

## FRONTIER (session 7 close, 2026-09-27) — ready for layer-2, not landed

State: candidate.c is sandbox 0 (136/136). It was spliced over the
INCLUDE_ASM line of src/ings.c under the landing lock, and the full build
matched the oracle (build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa).
The session closed before layer-2, so the splice was reverted: main still
carries INCLUDE_ASM, and the rebuild after the revert matches the oracle.
The commit message is drafted in tmp/func_800174F4/msg_match.txt and the
precheck output (mechanically clean) is in tmp/func_800174F4/precheck.txt.

Next worker:
1. Take the lock.
2. Splice candidate.c's typedef + function over the INCLUDE_ASM line.
3. Run `lock.ps1 rebuild`, then the sandbox.
4. Stage src/ings.c and run the precheck.
5. Send it to layer-2 with the Ruling 11 proof above.

What layer-2 must judge:
- `temp` and `temp2`, two values each, under Ruling 11.
- The F7 both-arms store.
- The `DrawEnv` typedef.
- `(u32)temp2 < cur`.

Mechanism (addiu vs ori): combine.c:3623 rewrites a PLUS whose operands
share no nonzero bits into IOR. That rule only fires when some combination
simplifies the plus. An `unsigned short` loop count supplies such a
combination: the subreg narrowing chain 3-merges into
`(set (subreg:SI h) (ior v0 4))`, recognised as iorsi3. cc1psx does the
same, so the original was not written that way. An int count leaves the
plus with only a 2-insn and->plus attempt, which is unrecognised and
discarded, so the addiu survives.

Spellings tried for the loop count / v0 (all emit ori):
- u16, u8 and s16 types for v0.
- One expression `(rand()&3)+4`.
- The count staged through `h`.
- `4 + v0`.
- `(u32)rand()%4+4`.
- Casts on the sum.

Spellings that keep the addiu: s32/u32 `h`, or an int split limit.

## CHEATED — reopened 2026-09-26 (owner ruling Q17)

CHEATED — matched only under the PLUS->IOR cc1 patch (owner Q17 2026-09-26);
honest floor under stock cc1 = 1 (136 vs 136 insns).

- Committed COMPLETED-C by d9660d9b5 (2026-08-14, session 6), ledger closed by
  336826804 and restored from 336826804^ at this reopen (sessions 1-6 below
  are unchanged; they were all measured under a PATCHED build/cc1 — the
  no-rewrite PLUS->IOR patch, which agrees with the later narrow patch here).
- Owner ruling 2026-09-26 Q17: "consider a compiler patch a cheat". The build
  compiler is now pinned upstream decompals/mips-gcc-2.7.2 @43d1cdb6 + the
  host-only reorg.c crash fix; tools/cc1-plus-to-ior-narrow.patch is retired.
  This was the only function whose bytes depended on the patch (census,
  tmp/patchcensus/, 2026-09-26).
- The single residual under stock: case 1/2, `h = v0 + 4;` with
  `v0 = rand() & 3`. nonzero_bits(v0) = 3 and 4 share no bits, so combine.c's
  simplify_rtx rewrites the PLUS to an IOR — we emit `ori s1,v0,0x4`, the
  target has `addiu s1,v0,4`. Register allocation, scheduling and every other
  instruction are unchanged.
- The body is banked unchanged as rejected/compiler-patch-dependent.c and is
  also the working candidate.c (best-known form). Main carries
  `INCLUDE_ASM("asm/funcs", func_800174F4);` (asm-until-matched).
- Open question for the next session: what C spelling makes the original
  compiler keep an add here — i.e. what stops nonzero_bits from proving the
  operands disjoint at combine time (the value's provenance, its type/width,
  or where the `+ 4` is formed), without changing the program's meaning.


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

## Session 4 (permuter, 2026-08-13/14) - floor 8 -> 2; cluster (A) CLOSED

### THE HEADLINE
The 6-point switch-selector cluster (A) - the residue three sessions could not
move, and the exact scope of the sole surviving regfix rule
`func_800174F4: $3 <-> $5 @ 27-41` - is CLOSED in pure C. Floor 8 -> 2.
The move: give the switch-selector local a SECOND live range in case 20, so one
C variable `mode` holds the dispatch value AND the case-20 limit:

    mode = g_disp_enable;
    switch (mode) {
    ...
    case 20:
        mode = D_800A37A0;
    {   u8 a2_val = D_800A38F8;
        s32 a0_val = a2_val & 0xFF;                 /* a1_val local is GONE */
        if (((u32)mode) < (u32)a0_val) break;
        div_result = h / (mode + 1);
        ... if (a0_val == mode) ... if ((new_val & 0xFF) == mode) ...

Sessions 2 and 3 measured TEN spellings of a selector local, all 8; every one
of them gave the local exactly one def (the `g_disp_enable` read). What was
never tried was a second def. Target's own register economy is the tell: target
keeps the dispatch value and the case-20 limit in the SAME register ($a1).

### Why it works (mechanism, consistent with the s3 find_reg reading)
s3 measured the selector allocno's complete find_reg state: hard conflicts
{2,29}, EMPTY someone_prefers / own_copy_prefs / own_full_prefs, so find_reg
handed it $v1(3) as the lowest free class register. Adding the case-20 def
extends the same pseudo's live range across the case-20 argument setup, which
is what finally puts regs 3 and 4 into its exclusion union and lets find_reg
walk to $a1(5) - the state s3 predicted was required ("to land on $a1(5) the
selector needs 3 AND 4 in hard_reg_conflicts U regs_someone_prefers"). s3's
route hypothesis (a lower-priority CONFLICTING allocno that prefers 3 or 4) was
therefore right about the requirement and wrong about the only way to reach it:
a second def of the selector itself does it without the selector going
callee-save (the measured 29/138 failure mode of s2's K6). The instrumented
confirmation (BB2_FINDREG_DEBUG on the floor-2 form, to read the new conflict /
someone_prefers sets directly) was NOT run this session and is a cheap,
high-value first probe for the next one.

### Measured ladder (all `sandbox func_800174F4 --disable all`, 136 insns)
| form | score |
|---|---|
| s3 floor-8 base (early-exit guard spelling) | 8 |
| + `s32 mode` selector local, single def (s2's K6 shape, re-measured) | 8 |
| + `mode` re-defined in case 20, `s32 a1_val = mode;` kept | **2** |
| + that, plus the `(s32)` cast dropped on the func_8005D554 result | **2** |
| + `a1_val` local deleted, case 20 reads `mode` directly | **2** |
| floor-2 base, `i = 0;` moved before `v0 = rand();` | 8 |
| floor-2 base, `i = 0;` moved between `rand()` and `v0 &= 3;` | 8 |
| floor-2 base, `i = 0;` moved after `h = v0 + 4;` | 8 |

The last three re-measure F4' on the NEW base: closing cluster (A) does NOT
relax the s3 priority inequality. The 2-point delay-slot residue is unchanged
and is now the ONLY thing between this function and a byte match.

### The permuter campaigns (the mandated modality)
Three fresh-seed campaigns, all launched/waited/harvested in-session via
`tools/permuter_campaign.py`, all against a hand-built single-function
workspace (see Artifacts):

| chassis | label | iters | elapsed | finds |
|---|---|---|---|---|
| 1: floor-8 candidate (`if (h != 0) {...}` guard) | goto-loop-floor8 | 46045 | 21 min | 1 (score 110) |
| 2: floor-8 early-exit (`if (h == 0) break;`) | earlyexit-guard-chassis | 41859 | 20 min | 1 (score 110) |
| 3: chassis 2 + `s32 mode` selector local | selector-local-chassis | ~8000 | 5 min | 3 (145, 110, 175) |

Chassis 1 and 2 both converged within 100 s to the SAME form - `i = 0;` hoisted
to just after `v0 = rand();` - and then produced nothing new for 20 more
minutes across 87k combined iterations. That form is s2/s3's already-rejected
pre-call-init attractor, measured at sandbox 8 (permuter 110). This is a
measured statement about the two metrics: decomp-permuter weights a reordering
at 60 and a register substitution at 5, so in THIS function its gradient points
away from the honest sandbox optimum (fixing the 2-point delay-slot reordering
"gains" 65 permuter points while losing 6 sandbox points). Seeding a permuter
campaign here and trusting its score would walk the function backwards.

Chassis 3 is where the modality paid: giving the permuter a selector local to
mutate, it produced `mode = D_800A37A0; s32 a1_val = mode;` (output-145-1) 74 s
after seed. That find is a PROPOSAL - it was then measured in the honest
sandbox (2), reduced by hand (the `a1_val` intermediate is unnecessary), and
re-spelled into the simpler one-variable form now in candidate.c.

### Artifacts (session 4)
`tmp/grind/func_800174F4/s4/` - `mkws.sh` (build the single-function permuter
workspace: full-TU preprocessed base.c, real cheat-free pipeline in compile.sh,
target.o assembled from asm/funcs/func_800174F4.s + prelude.inc with
`.set gp=64` stripped), `mkbase.sh` (trim base.c to the declaration prologue +
this one function so pycparser can parse it - PROVEN byte-identical to the
full-TU compile), `mkws2.sh <chassis.c> <dir>` (reusable: build a workspace from
an arbitrary chassis body), `chassis2.c`, `chassis3.c`, `variants/*.c` (9
measured forms). Campaign workspaces + logs: `tmp/perm_ings/`, `tmp/perm_ings2/`,
`tmp/perm_ings3/` (each with campaign.log, campaign_meta.json, output-*/source.c).

- [s4] FLOOR 8 -> 2. Cluster (A), the 6-point switch-selector residue that is the exact scope of the sole surviving regfix rule (`func_800174F4: $3 <-> $5 @ 27-41`), is CLOSED in pure C by giving the switch-selector local a SECOND live range: `mode = g_disp_enable; switch (mode) { ... case 20: mode = D_800A37A0; ... }` with every case-20 use of the old `a1_val` local reading `mode` instead (the `a1_val` local is deleted outright, so the body is one local SIMPLER than the floor-8 form). 136 == 136 insns.

- [s4] WHY sessions 2 and 3 missed it: their ten selector-local spellings all gave the local exactly ONE def (the `g_disp_enable` read), and s2's K6 correctly measured that shape at 8. The lever is not "cache the selector in a local", it is "the selector local also carries a second, later value". Target's register economy is the tell - target holds the dispatch value and the case-20 limit in the SAME register ($a1).

- [s4] The s3 find_reg requirement is satisfied exactly as s3 predicted ("to land on $a1(5) the selector needs 3 AND 4 in hard_reg_conflicts U regs_someone_prefers"); s3's assumed ROUTE (needing a lower-priority conflicting allocno that prefers 3 or 4, unreachable without the selector going callee-save at 29/138) was not the only route - a second def of the selector pseudo itself reaches the same exclusion state without crossing func_8005D46C. The BB2_FINDREG_DEBUG confirmation on the floor-2 form was not run this session.

- [s4] MEASURED: the 2-point delay-slot residue (F4') is UNCHANGED by the cluster-(A) win. All three pre-call `i = 0;` placements re-measured on the new floor-2 base (pre-`rand()`, between `rand()` and `v0 &= 3;`, after `h = v0 + 4;`) score 8 with 136 insns - still exactly the 6-point callee-save loss. The s3 priority inequality (counter 6153 vs `h` 8750 once the counter's live range crosses the call) still governs.

- [s4] MEASURED: dropping the `(s32)` cast on `prim = func_8005D554(...)` is codegen-neutral (2 either way), so the explicit cast stays.

- [s4] METRIC ANTI-ALIGNMENT, measured: decomp-permuter's weighted score and the engine's honest sandbox distance point in OPPOSITE directions for this function. Two independent fresh-seed campaigns (chassis 1 = the floor-8 candidate, chassis 2 = its early-exit spelling) each converged within 100 s to the same form - `i = 0;` hoisted to just after `v0 = rand();`, permuter 110 vs base 175 - and then produced nothing novel for 20 more minutes over 87k combined iterations. That form is the already-rejected pre-call-init attractor and measures sandbox 8. The cause is the weighting: a reordering costs 60 and a register substitution 5, so trading the 2-point delay-slot reordering for the 6-point register cluster looks like a 65-point win to the permuter and is a 6-point loss honestly. Never seed a campaign here and read its score as progress.

- [s4] The modality PAID once the chassis carried the lever: chassis 3 (chassis 2 + a `s32 mode` selector local) produced `mode = D_800A37A0; s32 a1_val = mode;` 74 s after seed (tmp/perm_ings3/output-145-1, permuter 145). Treated as a PROPOSAL per policy: measured honestly (2), then hand-reduced (the `a1_val` intermediate is unnecessary - deleting it scores the same 2 with one fewer local) into the form now in candidate.c.

- [s4] Reusable tooling added: tmp/grind/func_800174F4/s4/mkws2.sh builds a complete single-function decomp-permuter workspace for func_800174F4 from ANY chassis body file in one command (declaration-prologue-trimmed preprocessed base.c, real cheat-free compile pipeline, offset-0 target.o) and prints the base-vs-target insn diff as validation.

- [s4] PROVEN context-neutrality: a base.c trimmed to src/ings.c's declaration prologue (lines 1-90) plus func_800174F4 alone compiles to output BYTE-IDENTICAL to the full-TU compile for this function. That is what makes the workspace pycparser-parseable at all (the rest of ings.c is full of `register T x asm("...")` declarations and file-scope `__asm__` blobs pycparser cannot read).

- [s4] VETTING STATUS of the floor-2 form: `mode` carrying two values is the SOTN-sanctioned "variable reuse for codegen control" family (frozen list, .claude/rules/no-new-park-categories.md), and the resulting body has one FEWER local than the floor-8 form. It has NOT been through a cheat-reviewer, because the floor is 2 and not 0 - it is banked as the measured best form, not as a submission. Whichever session reaches 0 must self-vet it.

- [s4] [s4] FLOOR 8 -> 2. Cluster (A), the 6-point switch-selector residue that is the exact scope of the sole surviving regfix rule (`func_800174F4: $3 <-> $5 @ 27-41`, regfix.txt:11), is CLOSED in pure C by giving the switch-selector local a SECOND live range: `mode = g_disp_enable; switch (mode) { ... case 20: mode = D_800A37A0; ... }` with every case-20 use of the old `a1_val` local reading `mode` instead. The a1_val local is deleted outright, so the body is one local SIMPLER than the floor-8 form. 136 == 136 insns.

- [s4] [s4] WHY sessions 2 and 3 missed it: their ten selector-local spellings all gave the local exactly ONE def (the g_disp_enable read), and s2's K6 correctly measured that shape at 8. The lever is not 'cache the selector in a local', it is 'the selector local also carries a second, later value'. Target's own register economy is the tell - target holds the dispatch value and the case-20 limit in the SAME register ($a1).

- [s4] [s4] The s3 find_reg requirement is satisfied exactly as s3 predicted ('to land on $a1(5) the selector needs 3 AND 4 in hard_reg_conflicts U regs_someone_prefers'); s3's assumed ROUTE (a lower-priority conflicting allocno preferring 3 or 4, unreachable without the selector going callee-save at 29/138) was not the only route - a second def of the selector pseudo itself reaches the same exclusion state without crossing func_8005D46C. The BB2_FINDREG_DEBUG confirmation on the floor-2 form was NOT run this session.

- [s4] [s4] MEASURED: the 2-point delay-slot residue is UNCHANGED by the cluster-(A) win. All three pre-call `i = 0;` placements re-measured on the floor-2 base score 8 with 136 insns - still exactly the 6-point callee-save loss. The s3 priority inequality (counter 6153 vs h 8750 once the counter's live range crosses the call) still governs.

- [s4] [s4] MEASURED: dropping the (s32) cast on `prim = func_8005D554(...)` is codegen-neutral (2 either way), so the explicit cast stays in candidate.c.

- [s4] [s4] METRIC ANTI-ALIGNMENT, measured: decomp-permuter's weighted score and the engine's honest sandbox distance point in OPPOSITE directions for this function. Two independent fresh-seed campaigns each converged within 100 s to the pre-call-init attractor (permuter 110 vs base 175; sandbox 8 vs base 8-form's 8 -> it is the floor-14-class form) and then produced nothing novel over 87k combined iterations. Cause: reordering weight 60 vs register weight 5. Never read a permuter score here as progress.

- [s4] [s4] The modality PAID once the chassis carried the lever: chassis 3 (early-exit floor-8 chassis + a `s32 mode` selector local) produced `mode = D_800A37A0; s32 a1_val = mode;` 74 s after seed (tmp/perm_ings3/output-145-1, permuter 145). Treated as a PROPOSAL per policy: measured honestly (2), then hand-reduced into the simpler one-variable form now in candidate.c.

- [s4] [s4] Reusable tooling: tmp/grind/func_800174F4/s4/mkws2.sh builds a complete single-function decomp-permuter workspace for func_800174F4 from ANY chassis body file in one command (declaration-prologue-trimmed preprocessed base.c, real cheat-free compile pipeline with regfix/asmfix excluded, offset-0 target.o) and prints the base-vs-target insn diff as validation.

- [s4] [s4] PROVEN context-neutrality: a base.c trimmed to src/ings.c's declaration prologue (lines 1-90) plus func_800174F4 alone compiles to output BYTE-IDENTICAL to the full-TU compile for this function. That is what makes the workspace pycparser-parseable at all (the rest of ings.c is full of `register T x asm(...)` declarations and file-scope __asm__ blobs pycparser cannot read).

- [s4] [s4] VETTING STATUS of the floor-2 form: `mode` carrying two values is the SOTN-sanctioned 'variable reuse for codegen control' family (frozen list, .claude/rules/no-new-park-categories.md), and the resulting body has one FEWER local than the floor-8 form. It has NOT been through a cheat-reviewer, because the floor is 2 and not 0 - it is banked as the measured best form, not as a submission. Whichever session reaches 0 must self-vet it.

- [s4] [s4] All three campaigns were launched, waited on in-turn (`permuter_campaign.py wait`), harvested and --stop'd inside this session; zero permuter processes survive it.

## Session 5 (permuter, 2026-08-14) - floor stays 2; the permuter modality is now measured exhausted

### THE HEADLINE
Two more fresh-seed campaigns (both launched, waited on in-turn, harvested and
--stop'd inside the session; zero permuter processes survive it) close the
permuter question for this function, and the F4'' arithmetic is attacked from
its last untried direction (h's n_refs) and killed with an instrumented reading.

### Campaign 5b - the ALIGNED basin, and the decisive negative result
Chassis: the floor-2 body with `i = 0;` hoisted to the FIRST statement of the
`if (g_disp_fade != 0)` block (the pre-call placement reorg.c needs). Validated
before launch with mkws2.sh: the base-vs-target insn diff of that chassis is
NOTHING but the $s0/$s1 register flip (insns 51/53/54/59/61/117/120/121) - the
2-point delay-slot residue is already GONE in it, and sandbox reads it at 8.
This is the one chassis on this function where decomp-permuter's weighted score
is ALIGNED with the honest metric: every remaining diff is a register
substitution (weight 5), base score 80, and a permuter score of 0 would be an
honest 0. Result: 37,035 iterations / 19 minutes, ZERO finds. The permuter
cannot reach the register flip from that basin by any mutation in its
repertoire. That is the strongest available statement that this residual is not
a search-depth problem in permuter space.

### Campaign 5a - the floor-2 chassis (the inherited F6' probe)
Chassis: candidate.c verbatim; mkws2.sh confirms the base-vs-target diff is
exactly the two delay-slot lines (nop vs `move s0,zero` and its mirror).
Permuter base score 145, i.e. the honest floor-2 form scores WORSE in permuter
space than the honest floor-8 pre-call form (80) - a third independent
confirmation of the metric anti-alignment s4 measured. 43,747 iterations /
~48 min, ONE find: `output-80-1`, permuter 80, the form
`if (h == (i = 0)) { break; } i = 0;` - an embedded assignment that starts the
counter's live range before the guard branch. Measured honestly: 8. Two
hand-reductions of it (`if (h == (i = 0))` alone; `h = (i = 0) + v0 + 4;`) are
also 8. Exactly the known anti-aligned attractor in a new spelling.

### The floor-2 allocno table (the ledger's next-probe #1, now measured)
`tmp/grind/func_800174F4/s3/alloc.sh s5base` on the floor-2 base:

| ord | pseudo | role | hardreg | nrefs | livelen | pri |
|---|---|---|---|---|---|---|
| 0 | 86 | (short-lived temp) | 2 | 4 | 3 | 26666 |
| 1 | 76 | the switch selector `mode` | 5 ($a1) | 11 | 28 | 11785 |
| 2 | 87 | the case-1/2 loop counter | 16 ($s0) | 4 | 9 | 8888 |
| 3 | 73 | `h` (loop-limit + table-value web) | 17 ($s1) | 7 | 16 | 8750 |
| 6 | 74 | `prim` | 18 ($s2) | 8 | 76 | 3157 |
| 8 | 82 | `h`'s 0xF0/div web | 17 ($s1) | 4 | 58 | 1379 |

Cluster (A)'s win is visible: the selector is now an 11-ref / 28-length allocno
(the second live range) and lands on $a1 directly. The counter-vs-`h`
inequality is UNCHANGED by it: 8888 vs 8750, the same 1.6% margin s3 measured
on the floor-8 base. s3's arithmetic therefore still governs verbatim.

### Why the tie is decided by ORDER and nothing else (new, from the -dg dump)
`;; 8 regs to allocate: 86 76 87 73 101 102 74 82` with
`;; 73 conflicts: 73 74 76 87 2 4 5 29` and `;; 87 conflicts: 73 74 87 2 4 5 29`.
Neither `h` nor the counter has a CALLEE-SAVE hard conflict, and they conflict
with each other - so whichever global-alloc reaches first takes $s0(16) and the
other takes $s1(17). `;; Register dispositions` shows `72 in 16` as well: the
drawenv-address pseudo 72 is assigned $s0 by LOCAL-alloc and conflicts with
neither, which is exactly how target shares $s0 between the buffer address and
the counter. There is no conflict-level route to force `h` off $s0; the ONLY
lever is allocno_compare priority, i.e. the s3 inequality.

### F4''-(a) KILLED: `h`'s n_refs cannot be lowered
s3 left "make h's n_refs drop from 7 to 4 without splitting the variable" as the
untried surface (at livelen 16 that gives 5000 < the pre-call counter's 6153).
Four case-20 tail spellings measured on BOTH bases:

| case-20 tail form | floor-2 base | pre-call base |
|---|---|---|
| `h = tbl[a0] | ((cond) ? 0x8000 : 0);` (one def, one use) | 21 | 25 |
| same with `<< 15` instead of the ternary | 21 | 25 |
| `h` removed from the tail entirely (expression argument) | 41 (137 insns) | 39 (137) |
| two-step `h = tbl[a0]; h = h | (...);` | 26 | 25 |

Instrumented (`alloc.sh s5ab1`, the ternary fold on the floor-2 base): the fold
moves `h` from nrefs 7 / livelen 16 / pri 8750 to nrefs 6 / livelen 15 /
pri 8000 - it removes ONE ref, not two, and 8000 is still far above 6153.
Worse, it introduces a NEW allocno (pseudo 115, the fold temp, nrefs 3 livelen 4
pri 7500) that takes reg 3, which pushes the selector from reg 5 ($a1) to reg 4
($a0) and DESTROYS cluster (A) - that is where the 21 comes from. Reaching
pri < 6153 needs nrefs <= 4, i.e. `h` out of the case-20 tail altogether, which
is the 137-insn form. The axis is dead from this end too, joining K5 (h's
definition end) and K11 (the case-20 hoist end).

### Case-20 local merging KILLED (the ledger's low-priority frontier item 2)
Measured on the floor-2 base / the pre-call base: `new_val` merged into
`a2_val` 6 / 12; `div_result` folded into its compare 2 / 8 (exactly
neutral - a second equivalent spelling of the floor-2 form); `env` reused as
case-20's `div_result` 23 / 29; `env` reused as case-20's `counter` 12 / 18.
The `env`-reuse probes were mechanism-motivated (a case-20 local landing in
$s0 would put reg 16 in `h`'s conflict set) and they are dead: reusing `env`
that late degrades case 20 far more than the conflict change could buy.

### Artifacts (session 5)
`tmp/grind/func_800174F4/s5/` - `chassisA.c` (the floor-2 body), `chassisB.c`
(the pre-call-init body), `genc.py`/`gend.py`/`bank.py`/`ledger.py` (variant
generators + bookkeeping), `variants/*.c` (19 measured forms). Campaign
workspaces + logs: `tmp/perm_ings5a/` (campaign.log, campaign_meta.json,
output-80-1/source.c), `tmp/perm_ings5b/`. Instrumented dumps:
`tmp/grind/func_800174F4/s3/alloc_s5base.txt`, `alloc_s5ab1.txt`,
`ings_s5b2.i.greg`.

- [s5] The floor-2 allocno arithmetic is IDENTICAL to s3's floor-8 arithmetic: counter pseudo 87 = nrefs 4 / livelen 9 / pri 8888 -> $s0(16); `h` pseudo 73 = nrefs 7 / livelen 16 / pri 8750 -> $s1(17). Closing cluster (A) did not perturb the inequality at all, so every F4' number s3 derived carries over verbatim to the floor-2 base.

- [s5] The switch selector is now pseudo 76 with nrefs 11 / livelen 28 / pri 11785 and lands on reg 5 ($a1) directly - the cluster-(A) fix is visible in the allocno table as the second live range, not as a preference.

- [s5] From the floor-2 `-dg` dump: `h`(73) conflicts {73,74,76,87} + hard {2,4,5,29}; the counter(87) conflicts {73,74} + hard {2,4,5,29}. NEITHER has a callee-save hard conflict and they conflict with each other, so whichever global-alloc reaches FIRST takes $s0. `;; Register dispositions` also shows `72 in 16`: the drawenv-address pseudo is LOCAL-alloc'd into $s0 and conflicts with neither - which is exactly how target shares $s0 between the buffer address and the loop counter. There is no conflict-level lever; allocno_compare order is the only one.

- [s5] MEASURED KILL of F4''-(a) (s3's last untried arithmetic surface): lowering `h`'s n_refs by folding the case-20 tail. Four spellings measured on two bases (21/21/41/26 on floor-2; 25/25/39/25 on the pre-call base). Instrumented, the ternary fold takes `h` from nrefs 7 / livelen 16 / pri 8750 only to nrefs 6 / livelen 15 / pri 8000 - one ref, not two, still far above the pre-call counter's 6153 - and it creates a new allocno (pseudo 115) that steals reg 3 and pushes the selector off $a1, destroying cluster (A). Getting `h` under 6153 requires nrefs <= 4, i.e. removing `h` from the case-20 tail, which costs an instruction (137 insns).

- [s5] MEASURED KILL of case-20 local merging: new_val-into-a2_val 6/12, div_result folded into its compare 2/8 (neutral), `env` reused as case-20 div_result 23/29, `env` reused as case-20 counter 12/18. The env-reuse probes were mechanism-motivated (a case-20 local in $s0 would put reg 16 into `h`'s conflict set) and are dead.

- [s5] MEASURED: `if (div_result >= counter)` written with the divide inline (`if (h / (mode + 1) >= counter)`, one local fewer) is exactly neutral at 2 - a third equivalent spelling of the floor-2 form.

- [s5] DECISIVE permuter negative: a campaign seeded from the pre-call-init chassis - the one basin on this function where the permuter's weighted score is ALIGNED with the honest metric (the delay slot already matches; every remaining diff is a register substitution; base 80, and permuter 0 would be honest 0) - produced ZERO finds in 37,035 iterations / 19 minutes. The register flip is not reachable by decomp-permuter mutation.

- [s5] The floor-2 form's permuter base score is 145, WORSE than the honest-8 pre-call form's 80. Third independent confirmation of the metric anti-alignment; the only find from a 43,747-iteration campaign on the floor-2 chassis was `if (h == (i = 0)) { break; } i = 0;` at permuter 80, which measures sandbox 8 (as do its two hand-reductions). Never read a permuter score on this function as progress.

- [s5] Read from tools/gcc-2.7.2/flow.c: `reg_n_refs[regno] += loop_depth` at :2081/:2329/:2515/:2725, with loop_depth driven by basic_block_loop_depth / NOTE_INSN_LOOP_BEG (:1385/:1401/:1447). Our loop is GOTO-formed, so the front end emits no loop note and every ref is weighted 1 - which is why the counter reads exactly nrefs 4 (i=0, i++ def+use, the compare use). A form that makes the front end emit real loop notes would re-weight IN-LOOP refs for the counter AND for `h` simultaneously; the arithmetic has NOT been worked out and the only measured loop-note forms so far (K8's natural for/while) cost an instruction.

- [s5] All campaigns launched this session were harvested and --stop'd in-session; `permuter_campaign.py status` shows every workspace (including s4's) with alive:false at session end.

- [s5] The floor-2 allocno arithmetic is IDENTICAL to s3's floor-8 arithmetic: counter pseudo 87 = n_refs 4 / live_length 9 / pri 8888 -> $s0(16); h pseudo 73 = n_refs 7 / live_length 16 / pri 8750 -> $s1(17). Closing cluster (A) did not perturb the inequality at all, so every number s3 derived for F4' carries over verbatim to the floor-2 base (the ledger's next-probe #1, now answered).

- [s5] The switch selector is now pseudo 76 with n_refs 11 / live_length 28 / pri 11785 and lands on reg 5 ($a1) directly - s4's cluster-(A) fix is visible in the allocno table as a second live range, not as a preference.

- [s5] From the floor-2 cc1 -dg dump: `;; 8 regs to allocate: 86 76 87 73 101 102 74 82`, `;; 73 conflicts: 73 74 76 87 2 4 5 29`, `;; 87 conflicts: 73 74 87 2 4 5 29`. NEITHER h nor the counter has a callee-save hard conflict and they conflict with each other, so whichever global-alloc reaches FIRST takes $s0. `;; Register dispositions` also shows `72 in 16`: the drawenv-address pseudo is LOCAL-alloc'd into $s0 and conflicts with neither - which is exactly how target shares $s0 between the buffer address and the loop counter. There is no conflict-level lever on this residual; allocno_compare ORDER is the only one.

- [s5] MEASURED KILL of s3's F4''-(a): folding the case-20 tail lowers h only to n_refs 6 / pri 8000 (not the required <= 4 / 5000) and its fold temp (pseudo 115) steals reg 3, knocking the selector off $a1 - 21/21/41/26 on the floor-2 base, 25/25/39/25 on the pre-call base.

- [s5] MEASURED KILL of case-20 local merging incl. the mechanism-motivated env-reuse probes: 6/12, 2/8, 23/29, 12/18.

- [s5] MEASURED: `if (h / (mode + 1) >= counter)` with the divide inline (dropping the div_result local) is exactly neutral at 2 - a third equivalent spelling of the floor-2 form.

- [s5] DECISIVE permuter negative: from the ALIGNED pre-call-init basin (base 80; only register substitutions separate it from a byte match) decomp-permuter found NOTHING in 37,035 iterations / 19 minutes. The register flip is not reachable by permuter mutation, so this residual is not a search-depth problem in permuter space.

- [s5] The floor-2 form's permuter base score is 145 while the honest-8 pre-call form's is 80 - a third independent confirmation of the metric anti-alignment. Never read a permuter score on this function as progress.

- [s5] Read from tools/gcc-2.7.2/flow.c: `reg_n_refs[regno] += loop_depth` at :2081/:2329/:2515/:2725, with loop_depth driven by basic_block_loop_depth / NOTE_INSN_LOOP_BEG (:1385/:1401/:1447). Our loop is GOTO-formed, so the front end emits no loop note and every ref is weighted 1 - which is exactly why the counter reads n_refs 4 and h reads 7. Loop-note re-weighting is the one un-worked surface of the inequality.

- [s5] All campaigns launched this session were harvested and --stop'd in-session; `permuter_campaign.py status` reports alive:false for every workspace (including session 4's) at session end.

## Session 6 (forensics, 2026-08-14) - floor 2 -> 0. THE FUNCTION MATCHES IN PURE C.

### THE HEADLINE
`sandbox func_800174F4 --disable all` prints **score 0**, build_insns 136 ==
target_insns 136, rules_dropped 1 - i.e. the honest, cheat-invisible distance is
ZERO with the sole surviving regfix rule (`func_800174F4: $3 <-> $5 @ 27-41`,
regfix.txt:11) DISABLED. That rule is now redundant; retiring it is an
operator-side step (regfix.txt is outside a grind session's allowed surface).

### The forensic reading that unlocked it (the mandated modality, in order)

**(1) reorg.c cannot fill the `jal rand` delay slot from anywhere except the
call's own basic block, and in every floor-2-class form that block is EMPTY.**
Instrumented cc1 (`BB2_DBR_DEBUG=1` + `-dd`) on the floor-2 body
(`tmp/grind/func_800174F4/s6/dbr_f2.txt`, `ings_f2.i.dbr`): the trace contains
ZERO `DBRDBG simp insn=95 ...` lines for the rand CALL_INSN (uid 95), i.e.
`fill_simple_delay_slots`' backward scan terminated on its first `stop_search_p`
step - the only thing before the call in that block is `(insn 345 (use (reg
v0)))` and then the guard branch. The `.dbr` dump shows `(call_insn 95 ...)`
with no SEQUENCE (a bare nop) while `(insn 388 (sequence[ (jump_insn 110 ...)
(insn 119 (set (reg 16 s0) (const_int 0))) ]))` shows OUR `i = 0;` was taken by
`fill_slots_from_thread` into the guard branch's slot (`DBRDBG thr WINNER
insn=110 trial=119 annul=0`). So the ledger's F6' question - "does target's
`move s0,zero` have to be `i = 0;` at all, could reorg have pulled a different
insn?" - is answered NO: nothing else can reach that slot, and target's own asm
confirms the block is not empty there (asm/funcs/func_800174F4.s:51-53 shows the
guard branch's slot holding `addu $s2,$v0,$zero` - the `prim = func_8005D46C()`
result copy - and the `jal rand` slot holding `addu $s0,$zero,$zero`). The
requirement is exact: `i = 0;` must be the first statement of the
`if (g_disp_fade != 0)` block.

**(2) The priority inequality on that base is SHARPER than s3 stated, and s3's
"n_refs >= 6" target was wrong.** Measured allocno tables (BB2_ALLOC_DEBUG, this
session, on the floor-2 base with the `mode` lever):

| form | counter (pseudo 87) | `h` (pseudo 73) | score |
|---|---|---|---|
| floor-2 (init inside the guard) | nrefs 4 / livelen 9 / pri **8888** -> $s0 | nrefs 7 / livelen 16 / pri 8750 -> $s1 | 2 |
| pre-call init (first stmt of the fade block) | nrefs 4 / livelen 13 / pri **6153** -> $s1 | nrefs 7 / livelen **15** / pri **9333** -> $s0 | 8 |
| K12 duplicated init (pre-call + in-guard) | IDENTICAL to the pre-call row | IDENTICAL | 8 |

`h`'s priority in the pre-call form is 9333, not the 8750 of the in-guard form
(its live_length drops 16 -> 15 when the counter's init moves out of the guard).
So the counter needs pri > 9333 at live_length 13, i.e. weighted n_refs >= 7
(2*7/13 = 10769); n_refs 6 gives 9230 and LOSES BY 1.1%. Every session before
this one was aiming at the wrong number.

**(3) flow.c's loop-note ref weighting (the ledger's F7) is REAL, measurable,
and supplies exactly the missing refs.** `reg_n_refs[regno] += loop_depth`
(flow.c:2081/2329/2515/2725) with `loop_depth` from `basic_block_loop_depth` /
`NOTE_INSN_LOOP_BEG` (flow.c:1385/1401/1447); base depth is 1 (flow.c aborts on
depth 0), so refs inside a note-delimited loop weigh 2. Our loop is goto-formed
and emits no notes, which is exactly why the counter reads n_refs 4. Measured,
each `do { ... } while (0);` wrap re-weighting only the refs it spans:

| form | counter n_refs / pri | `h` n_refs / pri | score |
|---|---|---|---|
| pre-call base, no wrap | 4 / 6153 | 7 / 9333 | 8 |
| + `do { i = 0; } while (0);` only | 5 / 7692 | 7 / 9333 | 8 |
| + `do { i++; } while (0);` only | 6 / 9230 | 7 / 9333 | 11 |
| + whole loop body wrapped only | 6 / 9230 | 7 / 9333 | 8 |
| + init wrap AND `i++` wrap | **7 / 10769** | 7 / 9333 | 4 |
| + init wrap AND WHOLE-BODY wrap | **7 / 10769** | 7 / 9333 | **0** |

The wraps re-weight the COUNTER's refs without touching `h` because `h`'s only
in-loop reference is the `slt` in the exit test, which sits OUTSIDE both wraps.
That is the asymmetry s5 doubted existed ("only if a shape exists that
re-weights the COUNTER's in-loop refs without equally re-weighting h's in-loop
`slt` use") - it exists, and the exit test staying outside the wrap is what
creates it.

**(4) Why the wrap must span the whole loop body, not just `i++`.** With
`do { i++; } while (0);` the registers are already correct (10769 > 9333) but
the score is 4, not 0: the wrap's trailing CODE_LABEL lands between the
`func_8005D554` call and `i++`, and reorg.c's backward search stops at labels,
so `addiu s0,s0,1` can no longer reach that call's delay slot - ours emits
`move a0,s2` there and a nop in the back-edge `j` slot, where target has
`addiu s0,s0,1` in the call slot and `move a0,s2` duplicated into both the
pre-header and the `j` slot. Wrapping the whole body puts the label after the
increment and restores target's loop verbatim.

### Also measured (a correction to a session-3 kill)
K12's duplicated `i = 0;` (pre-call dead store + the in-guard live one) does NOT
lengthen the counter's live range by one unit as s3's arithmetic assumed: the
redundant store is folded away before flow analysis and the allocno table is
byte-identical to the single-pre-call form (n_refs 4, live_length 13, pri 6153).
The kill stands; the stated reason was wrong.

### Artifacts (session 6)
`tmp/grind/func_800174F4/s6/` - `dbr.sh` (BB2_DBR_DEBUG + cc1 `-dd` reorg dump
of the current src/ings.c), `alloc6.sh` (BB2_ALLOC_DEBUG allocno table),
`probe.py` (apply a variant -> honest sandbox score + allocno table -> restore),
`gen.py`/`gen2.py`/`gen3.py` (variant generators), `bank.py`, `ledger.py`,
`dbr_f2.txt` + `ings_f2.i.dbr` (the floor-2 reorg forensics),
`diff_wboth.txt` (the 4-point aligned diff that isolated the loop delay slot),
`variants/*.c` (9 measured forms incl. `final.c`, the matching body).

- [s6] MATCH: `sandbox func_800174F4 --disable all` = score 0, build_insns 136 == target_insns 136, rules_dropped 1. The honest cheat-free pure-C distance is ZERO and the sole regfix rule (`func_800174F4: $3 <-> $5 @ 27-41`, regfix.txt:11) is redundant - retiring it plus `queue done` is an operator-side step, outside a grind session's allowed surface.

- [s6] The matching form = the session-4/5 floor-2 body with `i = 0;` moved to the FIRST statement of the `if (g_disp_fade != 0)` block (reorg.c's requirement) plus TWO sanctioned `do { ... } while (0);` wraps: one around that init, one around the WHOLE case-1/2 loop body. Both are FAKE-annotated inline per .claude/rules/do-while-zero-exception.md.

- [s6] FORENSIC KILL of the F6' question "could reorg.c have pulled a different insn into the `jal rand` delay slot?": NO. BB2_DBR_DEBUG on the floor-2 body produces ZERO `DBRDBG simp insn=95` lines for the rand CALL_INSN - fill_simple_delay_slots' backward scan hits stop_search_p immediately because the call is the first real insn of its basic block. Target's own asm (asm/funcs/func_800174F4.s:51-53) confirms the shape: the guard branch's slot holds the `prim = func_8005D46C()` result copy and the `jal rand` slot holds `addu $s0,$zero,$zero`.

- [s6] CORRECTION to s3's F4' arithmetic: on the pre-call-init base `h` (pseudo 73) measures n_refs 7 / live_length 15 / priority 9333, NOT the 8750 of the in-guard base (its live_length shrinks by one when the counter's init leaves the guard). The counter therefore needs weighted n_refs >= 7 at live_length 13 (10769), not the n_refs >= 6 (9230) s3 derived - 6 loses by 1.1%.

- [s6] CONFIRMED with instrumented tables: flow.c's loop-note ref weighting (`reg_n_refs[regno] += loop_depth`, :2081/:2329/:2515/:2725; loop_depth from NOTE_INSN_LOOP_BEG at :1385/:1401/:1447, base depth 1) is a real and controllable lever. `do { i = 0; } while (0);` takes the counter to n_refs 5 (7692); a wrap covering the two `i++` refs takes it to 6 (9230); both together reach 7 (10769) and beat `h`. `h` is NOT re-weighted because its only in-loop reference is the exit test's `slt`, which stays outside both wraps.

- [s6] MEASURED: a wrap around `i++` ALONE fixes the registers but scores 4, because the wrap's trailing CODE_LABEL sits between the func_8005D554 call and the increment and reorg.c's backward search stops at labels, costing `addiu s0,s0,1` the jal delay slot. Wrapping the WHOLE loop body puts the label past the increment and restores target's loop (score 0). Wrap SCOPE is load-bearing, not just wrap presence.

- [s6] MEASURED CORRECTION to s3's K12: the duplicated `i = 0;` (pre-call + in-guard) produces an allocno table byte-identical to the single pre-call form (counter n_refs 4 / live_length 13 / pri 6153) - the redundant store is folded away before flow analysis, so it does not lengthen the live range at all. The kill stands; its stated reason did not.

- [s6] Reusable tooling: tmp/grind/func_800174F4/s6/dbr.sh (BB2_DBR_DEBUG=1 + cc1 `-dd`: per-candidate delay-slot trace plus the post-reorg RTL, which is how "the block is empty" was proven rather than inferred) and probe.py (one command: splice a variant, print honest score AND the allocno priority table, restore src).

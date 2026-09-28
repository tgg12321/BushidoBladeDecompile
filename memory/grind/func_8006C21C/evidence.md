# Evidence bank — saTan4GaugeMain

- [s1] [fable-blitz 2026-07-07] Queue: distance 620, ASM-STRUCTURAL, parked (canonical audit 2026-06-09 REJECTED: standard compiled C). Single rule = asmfix.txt:183 replace_with_asmfile. Stub at src/text1b.c:15912; caller at :15973 passes one arg (`saTan4GaugeMain(sp10)`). True signature: void f(Ctx *ctx) -- only a0, no return. Frame 0xC0, saves ra+fp+s0..s7 (ALL of them). No WIP, no near-dup lead, no GTE/cop2 anywhere.

- [s1] [fable-blitz 2026-07-07] SIGNATURE CODEGEN FACT: the ctx param is HOMED to sp+0x48 in insn 2 (`sw a0,0x48(sp)` s:3) and reloaded via `lw $t0,0x48(sp)` ~17 times -- after every jal. The param pseudo got NO callee-saved register because all 9 are claimed by hotter locals (s3 data ptr, s5/s2/s1/s0/fp/s4/s6/s7 loop state). Same for the inner 2-count loop counters (sp+0x50) and the three row-offset accumulators (sp+0x80/0x88/0x90, decremented -4/-8/-2 per bar row via lw/addiu/sw read-modify-write). Reproducing this pressure profile is THE register-allocation risk: the draft must keep the same set of long-lived locals so global.c spills exactly ctx + counters (rule refs: register-alloc-pure-c; contrast fake-varargs-explicit-homing -- NOT that family, this is plain RA spill).

- [s1] [fable-blitz 2026-07-07] Phase map: (1) frame sprite: desc(sp+0x18, S60C8 family from COMPLETED func_800600C8 text1b.c:12949) p0=*(ctx->4)->0x30->0 (s3=gauge data), otIdx field=0xA, func_8007352C, ctx->0x14=ret; saMotionSet(s3->0,0); initTexPage(ctx->0x1C,1,0,ret); ot_Link(D_800A374C+0x28); ctx->0x1C+=0xC. (2) otIdx=9; unlock icons: players s2=0,1 (s1=s2*2, x offset s4 += 0x118 into desc +0x30 field... sp+0x30): if ((s16)(D_800A34FC+s1)->0x28 < 3): bits fp=0..3: if (D_800A3524[level]->0x17 & ((1<<fp)<<(s2*4))): p0 = s3->0x34[fp], func_8007352C. (3) saMotionSet(s3->0x34,0)+initTexPage+ot_Link(+0x24). (4) otIdx=0xA; 6 sprites fp=0..5 x 2 copies (x=0/0x118 via `sw $s1(0x118)` vs 0 -- an if(counter)-else on the field, s:170-177): p0 = (s3+fp*4)->8, func_8007352C. (5) saMotionSet(s3->4, 0)+initTexPage+ot_Link(+0x28).

- [s1] [fable-blitz 2026-07-07] Phase (6) tiles: s2 = ctx->0x18 (TILE prims, stride 0x10; s0 = s2+0xE store cursor with negative offsets); table s1 = *(D_800A34FC->0x24)->0x44 (sp+0x58 home, stride 0xC): 11 rows (fp<0xB) x 2 players (x += 0x118 via s3): initTile(s2); rgb = s1->8/9/A bytes -> s0-0xA/-9/-8; x = s1->0 + s3 -> s0-6; y = s1->2 -> s0-4; w/h = s1->4/6 -> s0-2/+0; gpu_SetSemiTransp(s2, 0); ot_Link(D_800A374C+0x30, s2); write-back ctx->0x18 = s2 after. NOTE the h store `sh v0,0(s0)` rides in the gpu_SetSemiTransp jal DELAY SLOT (s:266-267) -- classic store-before-jal placement, comes free from source order.

- [s1] [fable-blitz 2026-07-07] Phase (7) pulse color: s4 = ((math_Cos((D_800A3518 << 7) & 0xF80) << 5) >> 12) + 0xD0 -- note the & 0xF80 angle mask BEFORE the call (s:287-288). Phase (8) gauge bars: s5 = ctx->0x10 (POLY_G4, stride 0x24, s0 = s5+0x22 cursor); per player (sp+0x50 = 0,1): s6=0 (row), s7=-1 (row-1), sp80/88/90 = 0; level = (s16)(D_800A34FC + player*2)->0x28; s1 = s2 = sp58_table + level*12 + 0xC; TWO initPolyG4 per row iteration (two bars), row loop s6<2.

- [s1] [fable-blitz 2026-07-07] Bar color/vertex arms: each initPolyG4 is followed by `if (level == 5) { gpu_SetSemiTransp(prim,1); all-4 colors = s4 pulse (or left-pair pulse for bar1); vertex block A } else { gpu_SetSemiTransp(prim,0); colors r-only s4/0x80 pattern; vertex block B }` -- the two arms end in IDENTICAL trailing stores and GCC 2.7.2 jump.c cross_jump MERGED the tails: joins .L8006C888 / .L8006CAB8 execute `addu v0,v0,t0; sh v0,...` where each arm loaded a DIFFERENT t0 (sp+0x80 in one, sp+0x90 in the other, s:358 vs s:412) before jumping in. Write both arms out in full in C and let cross-jump do the merge -- do NOT hand-factor the tail (rule ref: cross-jump-store-tail-merge).

- [s1] [fable-blitz 2026-07-07] Bar vertex math (arm bodies): bar1 x-coords = lhu s2->0 + s3(player x) with +0/+width(s2->4)/+2 combos, y = lhu s2->2 - s7 (+1 row0, +0 row1) +0/+1/+2 biases + sp90 accumulator; bar2 uses `lh s1->4 * s6` (mult by row index, mflo) + lhu s1->0 + s3 and y from lhu s1->0xE - 1, x biases +2/+4 + sp80/sp88 accumulators; per row: sp80 -= 4, sp88 -= 8, sp90 -= 2, s7 += 1, s6 += 1, s2 += 0xC, s5/s0 += 0x24 x2. After both rows: initTexPage(ctx->0x1C, 1, 0, 0x40) (a3 = CONSTANT 0x40, not a saMotionSet result -- different from phases 1/3/5), ot_Link(+0x20), ctx->0x1C += 0xC, s3 += 0x118. Epilogue: ctx->0x10 = s5 write-back.

- [s1] [fable-blitz 2026-07-07] ot_Link OT-index constants differ per phase: +0x28 (frame, bars-tail), +0x24 (icons), +0x30 (tiles), +0x20 (each POLY_G4) -- all as addiu on the loaded D_800A374C base, i.e. C `ot_Link(D_800A374C + N, prim)` with N in words (0x28/4=10, matching the desc otIdx field values 0xA/9 -- the desc field and the direct addiu spellings coexist; in THIS function ot_Link args are computed from the constant directly, NOT from the desc field like func_800720FC does).

- [s1] [fable-blitz 2026-07-07] Descriptor field-store pattern per phase: otIdx(sp+0x2C) rewritten 0xA -> 9 -> 0xA; flag byte sp+0x40 zeroed FOUR times (once per phase, redundant re-stores -- GCC keeps all, no MEM dead-store elim; write each phase's assignments literally as the source's copy-paste blocks); x/y fields sp+0x30/0x34 zeroed then per-loop `sw s4/s1/0`. The 2-copy sprite loop chooses `sw s1(=0x118)` vs `sw zero` via a 2-arm if on the copy counter with an empty-looking join (s:170-177) -- an `x = copy ? 0x118 : 0;` conditional that GCC turned into branch+two-store arms (NOT movz -- R3000).

- [s1] [fable-blitz 2026-07-07] All callees known + completed-precedented in TU: func_8007352C (25 sites), saMotionSet, initTexPage, ot_Link, initTile, initPolyG4, gpu_SetSemiTransp, math_Cos. Family neighbor saTan3GaugeMain_80073200 is named (called from func_800720FC) -- check its src state for a style template before drafting: it is the closest sibling by name and role.

- [s1] [fable-blitz 2026-07-07] m2c reference: tmp/blitz/m2c_saTan4GaugeMain.c (329 lines, valid syntax, exit 0) -- clean decode, good for cross-checking the bar vertex arithmetic signs/biases.

## s2 [manual slotO 2026-09-26] — first full draft, 620 -> 49 (sandbox --disable all)
Names now: saMotionSet=func_8006E480, initTexPage=SetDrawMode, ot_Link=AddPrim, initTile=SetTile,
initPolyG4=SetPolyG4, gpu_SetSemiTransp=SetSemiTrans, math_Cos=rcos; ctx is `s32 *arg0` (caller
passes its s32 sp10[] — landing must change the later `extern void func_8006C21C(s32);` at
text1b.c ~10232 to `(s32 *)`; sandbox candidates mask it with a trailing #define, NOT for landing).
- Frame forensics: target vars=128 = Env(0x2C->48) + TEN 8-byte reload slots (reload1.c alter_reg,
  align -1 -> BIGGEST_ALIGNMENT). Slots in regno order: 0x48 arg0, 0x50 j, 0x58 recs, 0x60-0x78 FOUR
  untouched (phantom), 0x80/0x88/0x90 the three bevel accumulators. Slot 0x50 is ONE pseudo used as the
  2-count counter in phase 4 inner, phase 6 inner AND phase 8 outer (distinct spilled pseudos never
  share a from_reg==-1 slot) -> one `j` variable across those loops (ordinary counter reuse).
- `y - k + (2 + d4)` grouping: fold-const.c associate (VAR+CON)+ARG1 -> VAR+(ARG1+CON) re-groups
  `y - k + 2 + d4` into `(y-k) + (d4+2)`; target is ((y-k)+2)+d4 which only `y - k + (2 + d4)` yields
  (mini-TU m1.c proof: fa/fb match, fc/fd/fe/ff don't). Same for (1+d2), (4+d8), (2+d4).
- k (s7, -1 then ++) must be its own variable: `y - (row - 1)` reassociates to `(y+1)-row` (fe).
- Bevel accumulators d4/d8/d2 (0, -=4/-8/-2 per row) must be explicit; `- row*4` forms make loop.c
  givs with +4 stride and subu (v1).
- Phase 6: `rec = &recs[i];` at top of the i loop -> loop.c DEST_REG giv replaced in place (s1, no
  copy). `recs[i].f` gives an inner-loop invariant copy (move s1,s4); `rec++` biv splits an
  offset-6 giv (v13).
- Phase 8 uses `rec[row]` (address giv = move s2,s1), not a separate `next++` pointer (extra
  offset-2 giv, v4).
- `cells` (header+0xC) seats a1 ONLY as one function-scope pseudo written at all 4 sites (global.c:
  conflicts with v0/v1/a0 across sites) — per-site expressions get v1/v0. POLICY: Ruling 5 ext.
  fails (B)/(C) here (table re-assigned between sites; 2 function-scope sites) -> needs Ruling 11
  proof or another form. OPEN.
- level must share `i`'s pseudo (fp) for global.c priority to spill j and seat level in fp:
  separate `level` pri 1487 < j 5182 (level spilled); merged i/level 40 refs/353 livelen pri 5665 >
  j 5105 -> j spilled, fp = i/level (v8: 131 -> 68). POLICY: multi-role local -> Ruling 11 needed. OPEN.

## s2 (cont.) [manual slotO 2026-09-27] — 49 -> 47; the three remaining gaps, measured
All scores `sandbox --disable all` against build/ at 55d9efbbd-era main; candidate = w4 form.
- **Loop-tail/init order (fixed, 49->47):** `for (row = 0, k = -1; row < 2; row++, k++)` puts
  row=0 before k=-1 (target 8006C68C/690) and k++ after the rec[row] giv add (8006CAE0). k MUST be
  a user variable: `k = row - 1;` per iteration is a DEST_REG giv loop.c refuses ("giv of insn
  754 not worth while, 0 vs 259", add-giv benefit 0), so it stays an in-loop addiu (g1, 103).
  Residual: target `lw t0,0x80; addiu s6` (row++ fills the d4 load delay), ours has row++
  before the load. 360 tail permutations of {d4,d8,d2 updates, k++} x body/for-header
  (tmp/func_8006C21C/gen_tail.py + tailscan.py) never produce giv,k++,lw,row++.
- **0x80 hoist (4 hunks):** target keeps `li v0,0x80` in each else arm; ours hoists it and
  reloads into spill reg t0. Mechanism (loop.c scan_loop/combine_movables/move_movables,
  dump tmp/.../dump_v14 F_loop): bar1-else 0x80 (life 4) and bar2-else 0x80 (life 4) are
  matched (same QImode const, both set once) -> savings 2, life 8 -> 58*2*8=928 >= 259 insns
  -> moved twice (out of row loop, then p loop), spilled, reg_equiv_constant. Unmatched, each
  is 58*1*4=232 < 259 (not moved) -> the target's arms did NOT match. Only measured form that
  reproduces it: ONE function-scope u8 written `dim = 0x80` in both else arms (n_times_set=2 ->
  not a movable): score 39, 2 source hunks left (rejected/dim-constant-holder-written-twice-39.c).
  Policy: a constant written twice fails R5 1(e)/R11 (C)(3) (not per-branch-different);
  named-local-fake-exception covers once-initialized holders -> needs an owner ruling.
  do-while(0) wraps (80 placements, tmp/.../dx/, gen_dw2.py + armscan.py) never seat v0 in
  both arms (best dw2 45: bar1 right, bar2 still t0). cc1psx-check on w4: psx 128 vs ours 47
  -> SOURCE-SIDE (cc1psx hoists too, frame 96 too).
- **Frame (largest remaining block):** target 0x60-0x78 never touched (spcensus of t.s);
  FRAMEDBG on ours: spill_new p72(arg0) p73(j) p75(recs) p209-211(d4,d8,d2) — target has four
  more spill_new slots with regnos between recs and d4. Mechanism class confirmed in-TU: combine
  distribute_notes plants `(use (reg N))` after a CODE_LABEL for a dead i2dest (orphans in
  func_8005BA8C: `for (i=0;i<n;...)` entry test slt+branch -> blez; func_80048BA4/80057CC8:
  `Judge[idx]` sym+idx*2; func_80049584: HImode var sign-extension). Probed and ruled out (all
  vars=96, identical insns): 6 global-access respellings (p1-p4, `((s16*)(D+0x28))[pl]`,
  `((u8*)(D3524+0x17))[lv]`, `recs = ((Rec**)..)[17]`, rcos `*128`/`<<5`), arg0[7] = arg0[7]+0xC,
  `1 << i << pl*4`, if/else for `j ? 280 : 0`, `recs + i + 1`, `(table+13)[i]`, inline helper for
  the 4 SetDrawMode/AddPrim/+=0xC triples, s16/u8 typing of k/x/pulse/d4-d2/level/i, s16 280
  holder, s16 rcos-arg and phase-2 level temps. `s16 mode` DOES add 2 orphan slots (vars 112,
  p145/p173 between recs and d's = the target's slot pattern) but combine then folds the arg to
  `move a1,zero` (target keeps `addu a1,s5,zero`) -> wrong mechanism, rejected/s16-mode-*.
- **Permuter** (tmp/func_8006C21C/perm, standalone TU == full-TU codegen, --stack-diffs, 5501
  iterations, 2 workers): best finds `short mode` (above) and a do-while(0) around bar2 r3
  stores; `new_var = arg0` copies. No closing form.
- **Ablation receipts:** level as its own variable 114 (rejected/ablation-level-own-variable-114.c);
  per-site `s.table = (s32)s.header + 0xC` (no cells) 59 (rejected/ablation-cells-per-site-expr-59.c).
- **Sibling precedent found:** func_8007636C (landed; Ruling 9 re-audit PASS 71b14499d) uses the
  same `cells` and a FAKE `s32 mode` holder for func_8006E480's 2nd arg; func_800753D8/800759D0
  use a FAKE `zero` holder (same target shape `addu a1,s5/fp,zero`). Mirror their annotations.

## s3 [manual slotO 2026-09-27, stock cc1 d94fef9a0] — frame phantoms: mechanism narrowed, not found
Re-measured on stock cc1: candidate 47 (622/622), t1 39 — unchanged.
- **Expand-time locals are excluded by frame order (measured).** A block-local `s32 probe[4]`
  lands at 0x48 and pushes arg0's reload slot to 0x58 (FRAMEDBG stack_temp before every
  spill_new). Assignment-as-value and struct locals likewise allocate at expand time. The
  target keeps arg0 at 0x48 directly after Env, so the four untouched 0x60-0x78 slots are
  reload `alter_reg` spill_new slots of pseudos whose regno lies between recs and d4. A
  local array / VECTOR-sized struct / address-taken local cannot produce them.
- **Caller-save save areas ruled out for ours:** `-fno-caller-saves` gives byte-identical
  output, and BB2_FINDREG_DEBUG shows no acc=1 find_reg retry for d4 (nrefs 20 vs 8 calls
  crossed, so CALLER_SAVE_PROFITABLE fails).
- **Orphan census with a private logging cc1** (/tmp/gccdbg, a copy of tools/gcc-2.7.2 with
  COMBDBG/ORPHAN fprintf in combine.c; output verified byte-identical to build/cc1 on the TU;
  never a build path; tmp/func_8006C21C/patch_comb.py + comb.sh + combsum.py). Our function
  has 28 combinations and ZERO 3->2 (newi2pat) ones, so no orphans are possible. Orphan-
  producing shapes elsewhere in this TU: (a) global ARRAY indexed by a register (`Judge[idx]`,
  newi2 keeps the symbol pseudo); (b) a loop entry test against a non-constant bound (newi2
  keeps `i = 0`; func_8005BA8C/80063E10/8006F100); (c) an s16 LOCAL loaded from an s16 field and
  then sign-extended into an int context while the HImode local survives (func_8006CCC8's
  `field`, func_800646E8/80049584). None fits the target cheaply: (a) no global arrays here
  (all globals are loaded pointers); (b) every target loop test is an slti immediate; (c) the
  only lh reads are phase-2 lv (used once), phase-8 level (merged with the int counter i;
  s16 i adds visible sll/sra, i16.c), bar-2 w (re-read at each vertex).
- **Sweeps with no frame change and identical code:** 138 single-site spelling mutations
  (mut.py), 427 single/pair retypings of every scalar local (typesweep.py; only `mode` s16/s8
  (+2 slots, but a1 folds to a constant) and `x` u8/s8 change the frame, all with different
  code), every for-loop as do-while/while (loopsweep.sh, 18 variants), do-while(0) around every
  statement and statement pair (wrapsweep.py), a pointer-to-field DR_MODE spelling, a
  descriptor-through-pointer spelling, -G8, an inline helper, and separate s16/u16/s32 level.
  Second permuter campaign: 3.5k iterations, no frame-changing find.
- **Sibling datum:** func_800720FC (INCLUDE_ASM, same family) has 2 untouched slots after its
  one spill slot, with 2 SetDrawMode/func_8006E480 pairs. Ours has 4 SetDrawMode (3
  func_8006E480) and 4 phantoms. The correlation is suggestive but no mechanism is found.
- **Gap 2 policy check (lead's Q20 pointer):** both else-arm writes store the SAME constant
  (0x80), so R11 (C)(3) plus the Q20 per-branch clause still refuses the twice-written holder.
  It is not yet proven to be the only closing form (80 do-while(0) placements fail; other
  families remain untried), so no policy question has been filed.

## s4 [Codex manual lane 2026-09-27] — source-natural follow-ups, floor remains 47
- A single-use `s16 zero` passed to the first `func_8006E480` call compiled byte-identically;
  it did not allocate either of the missing reload slots. Four typed primitive-cursor advances
  (`arg0[7] = (s32)((u8 *)arg0[7] + 0xC)`) were also byte-identical.
- Replacing the four header-plus-12 intermediates with typed `Sheet *` / `Cell *` expressions
  kept vars=96 and regressed 47 -> 59: each store used the header result register instead of
  the target's shared `a1` pseudo. Byte-offset spellings for the draw-mode cursor regressed to
  51 / 624 instructions. Both were reverted.
- A once-initialized `s16 player_count = 2` used as the first player-loop bound regressed to
  65. A reusable `s16 level` assigned at both phase-2 loads and copied into `i` in phase 8
  compiled byte-identically to the 47 baseline when only `i` was consumed.
- Consuming that `s16 level` in the first bar comparison confirmed the earlier orphan mechanism:
  frame vars grew from 96 to 112 (two 8-byte reload slots), but the build grew to 627 instructions
  and score 57 due an `lhu`/stack store/reload/sign-extension sequence absent from the target.
  It is not a viable half-solution and was reverted.
- Recasting the second `if (i == 5)` as a `switch`, and as the equivalent unsigned range test,
  compiled byte-identically. Neither prevents loop.c from matching and hoisting the two 0x80
  constants. The best honest candidate therefore remains 47 / 622 with equal instruction count.

## s5 [manual, cloud Linux container 2026-09-28] — 47 -> 41; frame mechanism identified
Environment: no WSL/pwsh; cc1 rebuilt per tools/build_oracle_cc1.sh recipe (pinned upstream
43d1cdb + crash fix, `--host=i386-pc-linux`, all -O0 -g, combine.o -O; simplify_rtx 0x3a11).
Reproduced the banked candidate at 47/622 before changing anything. No disc/ EXE here, so the
full-SHA1 oracle was not run; every score is `engine sandbox --disable all`.
- **0x80 hoist (fixed, -4):** loop.c move_movables moves the matched else-arm constants when
  `threshold * savings * lifetime >= insn_count`; the row loop has calls, so threshold = 29
  (not 58), insn_count 259. Base form: life 4+4, savings 2 -> 464 >= 259 -> hoisted.
  `poly->r3 = poly->r2 = 0x80;` (bar 1) and `poly->r3 = poly->r1 = 0x80;` (bar 2) make each
  arm's life 2 -> 232 < 259 -> `li v0,0x80` stays in both arms. Both chains are needed (one
  alone: 47). Residual: the r3 store now follows r2 directly (target: after b2) -- 2 hunks.
  100-combo sweep of chain direction x position (tmp/sweep_chain.py): best 43 on the old base,
  i.e. no position fixes the order. Any target-order form with one pseudo per arm has life 4
  (-> hoisted), so the target's arms are either unmatched or shorter-lived by a mechanism not
  yet found. Swapping arms (`i != 5`): 61-81.
- **Tail order (fixed, -2):** sched's tie-break is insn order (rank_for_schedule -> LUID), and
  loop.c inserts a giv increment right before its biv's increment. `for (...; k++, row++)` plus
  bar 1 addressed as `next[k]` with `next = rec + 1` (same address as rec[row]) ties the giv to
  k, giving target order `s2 += 12; k++; lw d4; row++`. `rec[k + 1]` also fixes the order but
  splits the +12 into the displacement (`addiu s2,s1,-12`, 62).
- **Bevel accumulators as givs (neutral, 41 = 41):** `(2 + row * -4)`, `(1 + row * -2)`,
  `(4 + row * -8)` in place of d4/d2/d8 is byte-identical to the explicit accumulators;
  `2 - row * 4` / `<< 2` / unparenthesised forms give subu (69-80). As givs they are created by
  loop.c, so they get the highest regnos -> their reload slots land LAST in the frame.
- **Frame phantoms: mechanism found (not closed).** Confirmed via -dg: an orphan
  `(use (reg N))` pseudo (combine distribute_notes, REG_DEAD note with no home before a label)
  sits in global's allocno list with no conflicts, is never allocated, and alter_reg(i, -1)
  gives it a slot nothing touches. A function-scope `s32 n = 2;` used as the bound of all five
  2-count loops gives EXACTLY four orphans: the duplicated loop-entry tests (jump.c
  duplicate_loop_exit_test) of phase-4 inner j, phase-6 inner j, phase-8 j and the row loop.
  Phase 2's pl loop gets none: its entry test is in the same CSE block as `n = 2`, so CSE1
  folds it. With the giv bevel form the slots land at 0x60/0x68/0x70/0x78 and the givs at
  0x80/0x88/0x90 = the target layout exactly (rejected/nbound-frame-exact-entrytests-remain-117.c,
  vars=128). Blocker: combine turns each entry test into `(eq n 0)` (nonzero_bits proves n >= 0,
  not n != 0), so `li t0,2; beqz t0` survives and the allocation shifts (score 117).
  The target needs a bound/init form where CSE cannot fold the entry test but combine folds it
  completely (3->2 with i1 `j = 0` kept as newi2pat). CSE leaves `(lt j n)` alone because
  slt_si needs a register first operand.
- **Ruled out this session (all vars=96):** s16 level locals in phase 2 (outer, inner, both,
  block-scope) and phase 8 (`lv` then `i = lv`) -- byte-identical; s16/u16/s8/u8 `mode` with
  1-3 assignments (49-52); zero-holder loop inits `j = mode` / `j = z` (166-169, no orphans:
  combine does not fold `(lt z 2)` via nonzero_bits here); loop-form variants of the phase-6 j
  loop (`<= 1`, `!= 2`, `2 > j`, `2u`, casts, while, do-while, `&& j >= 0`): no slot, 41-47;
  bound types s32/u32/s16/u16/s8/u8 with `< n`, `<= n`, `< n + 1`, `<= n - 1`, `< n - 1`:
  the `< n`-style forms give the four slots but keep entry tests (117-140); `<= n` forms give
  no orphans (CSE substitutes j into the second slt operand).

## s6 [manual, cloud Linux container 2026-09-28] — phantom-slot mechanism survey (floor 41 unchanged)
Tooling (scratch only, never a build path): a copy of tools/gcc-2.7.2 in the session scratchpad
with SLOTDBG in reload1.c alter_reg (prints every stack slot's pseudo, from_reg, refs, live,
and whether any insn still mentions it) and COMBDBG in combine.c (prints every 3->2 combine
whose i2dest vanishes). Output verified byte-identical to build/cc1 on text1b. Scripts:
tmp/s6/{patch_alter2,patch_comb,patch_la}.py, mkcc1.sh, slots.sh, comball.sh, combcat.py, phx.py.
- **Ours:** exactly 6 slots, all `from -1` with live code: arg0 p72, j p73, recs p75, and the
  three row-loop givs (d4/d8/d2, loop-created, highest regnos). Zero 3->2 vanishes.
- **Project-wide catalog** (every src/*.c TU, 183 vanish events, 60 untouched slots): every
  untouched slot in a matched function comes from combine, except 3 from `/ 255` division
  expansion (func_80041E10). The combine families are:
  (1) loop entry test against a non-constant bound (41×): the branch survives as `blez`/`beqz`;
  (2) s16 memory value shared by a sign-extended use and a narrow use (64×): `lh` + HI copy;
  (3) global array `SYM[reg]` addressing (40×): `$at` macro forms;
  (4) decrement-and-test (`n - 1 != -1`, `while (n--)`) (7×);
  (5) 3->1 fold of a narrow local's extension where the i1 death note becomes an orphan USE
  (the `s16 mode` case: +2 slots, but combine folds the arg to `move a1,zero`).
  The target body has no `blez/bgtz`, no `$at`, no `lh` whose value also feeds a narrow
  store/add, and no decrement tests, so families 1-4 cannot supply its four slots at zero
  code cost.
- **Loop-entry route ruled out by exhaustion** (mini-TU harness tmp/s6/mini/, nested 2-count
  loop, n set once outside): 1458 + 182 bound/init/type/test forms. Every form that leaves an
  orphan costs +4 insns (the entry branch survives). Combine can fold a comparison only against
  0 via nonzero_bits/sign bits, never `0 < n` or `z < 2`.
- **0x80 hoist mechanics pinned:** loop.c move_movables needs `threshold*savings*lifetime <
  insn_count` to keep the constant in-loop; the row loop has 264 real insns, threshold 29
  (loop has calls), and the two else-arm constants always match in combine_movables (same
  const, either mode order: the wider one absorbs the narrower). lifetime counts NOTE_INSN_DELETED
  luids. Not moved requires life ≤ 4 total, or ≥5 other movables moved first in the same loop
  (threshold -= 3 each), or m1->global. None of these has a natural source form yet.
- **PsyQ setRGBn / setXY4 macro spellings** of the bar arms compile byte-identically to the
  statement form (45 = natural-order baseline): comma expressions do not change the RTL.
- **Other probes, no frame change:** s16/u16/s8/u8 holders for the rcos angle and the first
  func_8006E480 zero (CSE folds them in-block); phase-8 `s16 lv` level with `i = lv` (41,
  byte-identical); lv used directly in the `== 5` tests (605 insns: jump threading merges the
  bar1/bar2 tests); lv loaded in a separate block-scope decl (same).
- Sibling func_800720FC's two untouched slots are explained by family (2): its `lh` of a
  `D_8009BCD0[]` element feeds both an abs() compare and a narrow `*p + d` store. The earlier
  "SetDrawMode count" correlation is coincidental.
- **Frame reproduced (policy-blocked).** s16 zero holders read only after block 0 give one
  untouched slot per folded read: `Z` as SetDrawMode's 5th arg in phases 3/5/8 (+3, vars 120,
  622 insns) plus a second holder at the phase-4 `s.x = 0` (or `s.y = 0`, `s.semi = 0`, or a
  phase-3/5 dtd arg) (+1) = vars 128, EXACT target layout (arg0, j, recs, 4 phantoms, givs).
  Score 6 alone, 2 with `col` (rejected/s16-zero-holders-frame-exact-2.c). Residual 2 =
  phase-8 `sw zero,16(sp)` scheduled early because that call's tw is the holder; with a
  literal there, no pair of extra holder sites reaches 128 at 622 (78-combo sweep,
  tmp/s6/mkcomb2.py; several dtd3 pairs hang cc1). Reusing ONE holder at every same-meaning
  site costs code (block-0 reads fold in CSE; natural `s16 px, py` for all Env x/y stores:
  96-104, +1..+8 insns).
- **`col` per-branch colour (41 -> 37, zero source-level hunks):** see candidate header.

## s7 [Codex local WSL, 2026-09-28] — adversarial correction and cluster ablation

Started after owner pulled main e6feac99c. queue next still selects this function;
canonical routes C. Reproduced s6 at 37/622. A fresh independent checkpoint
review FAILed the s6 col reuse under R11's common-read definition/C(3) and
redundant same-value write B(2); see review-s7.md. No completion PASS was obtained.
The rejected body is retained as rejected/s6-col-carrier-37.c, not a best admitted
candidate. Its numeric score is real; its claimed policy admission is not.

Measurements (all sandbox --disable all, project compiler):
- Remove col and use literals: 45/622. Genuine runtime color selection once per
  row: 186/629; once per player: 59/632. Neither closes.
- WHOLE next/k cluster removed together (rec[row], y + 1 - row): 45/622.
- Split tile rec from gauge rec: 45/622. Byte-pointer header/cells and explicit
  descriptor color bytes: 45/622. Combine all these simplifications: 45/622.
- On that simplified body, chain only bar 1: 45/622; only bar 2: 45/622; both:
  41/622. candidate.c now contains the simplified two-chain checkpoint.
  Thus the old claim that next/k is needed for score 41 is disproven by cluster
  ablation. No new no-semantic-purpose construct was added.
- Direct cells expression without carrier: 57/622; literal mode: 48/622;
  separate level: 112/605 (each on the direct-color s6 base). On the combined
  simplified body, direct cells plus literal mode: 60/622; also split level:
  123/605. This final whole-cluster ablation is banked as plain-s7.c.
- Color/pulse narrowing to s16/u16: unchanged 37/622 on s6; u8 col unchanged,
  u8 pulse 38/622. These were diagnostics on rejected s6, not admissions.
  Naming coordinate fields within the four bar arms: scores 47-197, no closer
  candidate. The local-only x s16/u16 forms: 53/622 on the simplified base.
  See probes/s7/results.json and README for negative tests and script-error caveat.

Open: 32-byte frame gap, two color-store orderings, and full policy evidence for
i/cells/mode. The changed pointer typing fixes cells' cast-form defect but does
not itself supply R9's other prerequisites. Neither numerical closeness nor
these ablations constitutes R11 necessity proof. Do not rotate or mark done.
No src, asm, build configuration, compiler, gate list, or queue edit was made.

Verification of the UNCHANGED main source (not of the checkpoint as a landed C body):
`verify-oracle --rebuild` returned ok=true, build_matches=true, artifact_matches=true,
fresh=true; SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa. The function remains
INCLUDE_ASM in that build. `tools/audit_asm_cheats.py --check-new` exited 0 silently.
Re-scored the exact saved s7 candidate: 41/622, four source-level diff hunks
(two moved stores), fourteen operand-only hunks, fifteen masked cascade hunks.
`tools/check_completion_integrity.py` was first started during the rebuild and
exited 1 because build/verified-inputs.json was temporarily absent. Re-run after
the successful rebuild: exit 0, "OK: all completed functions satisfy their
category's invariants." This is repository validation, not candidate approval.

## s8 [manual, Claude, local WSL, 2026-09-28] — mechanisms pinned for both gaps; floor 41 unchanged
Resumed the open manual session; re-scored the s7 candidate at 41/622 (4 source-level hunks =
the two early r3 stores; 14 operand-only = frame offsets). Receipts in probes/s8/.
- **0x80 hoist, loop.c arithmetic (dumps, tmp/c21c/loops.sh).** Natural per-arm literals: each
  else-arm constant is a movable of life 4, the two arms MATCH in combine_movables (life 8,
  savings 2), threshold 29 (loops with calls): 29*2*8 = 464 >= 264 (row loop) and >= 312 (j loop),
  so it is hoisted twice. Matching needs n_times_set == 1 on both, same set_src, m1 not global;
  the only escapes are (a) one variable written in both arms (n_times_set 2 -> not movable: the
  refused dim/col shapes), (b) a user variable read outside its basic block, (c) threshold cut
  by >= 5 earlier moves in the SAME loop (loop.c:1719 `threshold -= 3`), (d) insn_count > 464.
  Measured: (c) `rec = &recs[i + 1]` inside the row loop is not hoisted (first chain insn life 2,
  116 < 269); indexing `recs[i + 1 (+row)]` inside the loop moves the chain but AFTER the 0x80 in
  list order, and CSE folds i == 5 into the if-arm addresses (190-238/609-639). (d) s16 k / x /
  row do not change the loop's pre-combine insn count (264). No natural (a)/(b) found.
- **Frame orphans, exact mechanism (private logging cc1, output byte-identical to build cc1).**
  Candidate: 28 combines, zero 3->2, zero orphans. The s6 holder file's orphans come from 2->1
  combines, not 3->2: i2 = (ashiftrt t1 16), i3 = store; t1 = (ashift Z 16) with Z known zero, so
  the store folds to 0; distribute_notes (combine.c REG_DEAD case) deletes t1's now-dead setter
  and STILL plants `(use t1)` after the block's label (the backward scan continues past the
  deleted TEM). t1 is then used-never-set -> live from entry -> unallocated -> untouched slot.
  So a slot needs a narrow value whose known bits make its extension fold to a CONSTANT, read
  after a label, outside any loop (inside a loop, loop.c hoists the extension pair: life 2,
  savings 2 -> 116 >= loop size, and the fold is lost, +insns).
- **Zero-cost reproduction of 3 of the 4 slots:** `s16 xpos = 0, ypos = 0, semi = 0` read only at
  the phase-1, phase-2-head and phase-4-head descriptor x/y/semi stores: vars 96 -> 120, cc1 code
  identical (probes/s8/zero-s16-descriptor-locals-vars120.c). Reading them at the in-loop sites
  too (the consistent, natural form) costs 4-34 lines and loses the slots. This is the refused
  holder class (named-local-fake-exception: frame-reservation mechanisms stay forbidden); banked
  as mechanism evidence, not as a candidate.
- **Other sweeps, all vars 96 / no orphans:** 40 single-site respellings (rcos angle, pulse,
  bit mask, level reads, rec base, x offsets, w*row order, OT adds) — probes/s8/tools/sweep1.py;
  19 struct field signedness variants (Rec x/y/w/h, PolyG4 coords/colours, Env bytes) — sweep2.py.
- **Naturalness gain (byte-neutral):** bar 1's vertex block is exactly PsyQ `setXYWH(p, x, y, w, h)`
  — `setXYWH(poly, rec[row].x + x, rec[row].y + 1 - row, rec[row].w, -4 * row + 2)` (else arm
  `-2 * row + 1`) compiles identically. The macro's `(_y0)+(_h)` is the grouping s2 found
  necessary, and its re-evaluated arguments explain the per-coordinate reloads. `2 - row * 4`
  costs +4 insns. Bar 2 is not setXYWH (y2/y3 read rec[1].y).
- **Dead ends:** Kengo's `saTan4GaugeMain` (src/sato/sa_tan4.c, 0x1504C0) is an unrelated float
  state machine; Kengo gives nothing for this function. Caller-save save areas cannot explain the
  32 bytes (the target has one spill reg, t0, and no saves). -fno-caller-saves irrelevant.

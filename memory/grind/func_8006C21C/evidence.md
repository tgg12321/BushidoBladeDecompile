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

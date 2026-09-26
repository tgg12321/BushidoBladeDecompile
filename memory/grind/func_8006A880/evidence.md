# Evidence bank — func_8006A880

- [s1] [fable-blitz 2026-07-07] Queue state: distance 550, ASM-STRUCTURAL, PARKED (2026-06-09 audit REJECTED canonical-asm). 1 rule: asmfix.txt:181 replace_with_asmfile. src/text1b.c:15668 stub: void f(s32,s32,s32,s32) - actual signature is TWO args: a0 = ctx struct (s32* with prim-chain cursors: [1]=+0x4 asset-root, [2]=+0x8 scaled-sprite chain, [5]=+0x14 sprite chain, [6]=+0x18 tile cursor, [7]=+0x1C texpage cursor, [8]=+0x20 drawarea cursor, [9]=+0x24 drawoffset cursor), a1 = u16[6] rect/offset block (x,y,x2,y2,ofsx,ofsy). Returns void. 550 = whole unwritten body.

- [s1] [fable-blitz 2026-07-07] FAMILY JACKPOT: the function's own tail calls func_8006A494(ctx, &block) - COMPLETED at distance 0, WRITTEN C at src/text1b.c:15645 taking (s32 *arg0, u8 *arg1) - which documents the EXACT param-block field map used here (block base = sp+0x18): +0x00 anim ptr, +0x04 glyph (anim+0xC), +0x08 chain-prev, +0x10 flag(sp28), +0x14 otIdx(sp2C), +0x18 x(sp30), +0x1C y(sp34), +0x20/+0x24 w/h scale (sp38/sp3C, only Phase G sets 0x200/0x100), +0x28 byte flag(sp40), +0x29/+0x2A/+0x2B rgb(sp41-43). Also proves cursor idioms: 'arg0[2] = func_80073728(&blk, 0)', 'initTexPage(arg0[7], 1, 0, saMotionSet(blk.anim,0), 0); ot_Link(D_800A374C+4, arg0[7]); arg0[7] += 0xC;' - reuse these spellings VERBATIM. Note A494 reads blk.anim back through a VOLATILE s32 cast (v = *(volatile s32*)(arg1+0)) - the family tolerates that spelling in completed code.

- [s1] [fable-blitz 2026-07-07] Also called: saTan3GaugeMain_8006A564(ctx, &block, i) - active dist 197 rules 1, its own asmfix stub sits directly ABOVE this function at text1b.c:15663 (4-arg stub); called 8 times here (7 loop + 1 phase-D with a2=7). func_80073728 (scaled-sprite emitter, active 339). math_Sin, initDrawArea, initDrawOffset, initTile, initTexPage, ot_Link, saMotionSet, gpu_SetSemiTransp, func_8007352C all COMPLETED or near-0. func_8006A3CC (sibling, has grind ledger) - same subsystem.

- [s1] [fable-blitz 2026-07-07] Phase A prologue (L14-59): s2 = ctx[1]->0x18 (body-part asset table, 8 entries); s6 = D_8009BC08 (bit mask); emit texpage bracket: saMotionSet(s2[0],0) with sw a0,0x18(sp) homing the FIRST table cell into the block; initTexPage(ctx[7], 1, 0, ret, 0); ot_Link(D_800A374C+0x30, ctx[7]); ctx[7]+=0xC; drawarea rect sp+0x48 = {a1[0], a1[1]+0x3A, 0xF6, 0x92} (2 lhu + 2 constants, sh order 0x48,0x4C,0x4E,0x4A - the +0x3A add stored LAST); initDrawArea(ctx[8], sp+0x48); ot_Link(+0x30); ctx[8]+=0xC.

- [s1] [fable-blitz 2026-07-07] Bit-scan (L60-69): yofs = 0; if (!(D_8009BC08 & 1)) { i=0; do { i++; yofs -= 0x18; } while (!(s6 & (1 << i))); } - first-set-bit scan of D_8009BC08 accumulating -0x18 per skipped slot; the sllv uses a CONSTANT 1 in a0 (addiu a0,zero,1 hoisted above the loop). i (s1) is reset to 0 immediately after (only yofs survives into the drawoffset: sp+0x50 = {a1[4], a1[5] + yofs}; initDrawOffset(ctx[9], sp+0x50); ot_Link(+0x30); ctx[9]+=0xC).

- [s1] [fable-blitz 2026-07-07] Main loop (L96-200, i=0..6): tri-state per body part. Constants pre-hoisted into callee-saves: s4=0x30 (rgb.b everywhere), s7=1 (flag byte + sp28), s2=0x3F (y accumulator, +=0x18/iter), s6 RELOADED = D_8009BC04 (alive mask, L92), sp2C=0xB (otIdx, set ONCE before loop); per-iter: sp30=0x76 (x, re-stored EVERY iteration), sb s7->0x40 in the branch delay slot. FLASH arm ((D_800A34F8 & 0xF) == i): sp28=0; r=math_Sin(((D_800A3514 & 0x1F) << 7) + 0x1FF); blk.r = (r*63 >> 12) - 0x61 (sll6-sub = *63); r2=math_Sin(SAME recomputed arg - the (D_800A3514&0x1F)<<7+0x1FF expression is RE-EVALUATED with a fresh lw, not cached); blk.g = (r2*47 >> 12) - 0x7D (sll1+add=*3, sll4-sub=*47); blk.b=s4; D_800A3514++; y = ((s16*)D_800A34FC)[7] + s2 (lh +0xE). ALIVE arm ((D_8009BC04 >> i) & 1): rgb={0x80,0x6C,0x30}; sp28=1; y=s2; if (D_800A3524->0x20 & 1) blk.flag40=0. DEAD arm: rgb={0x30,0x30,0x30} (all from s4); sp28=1; y=s2. Then: blk.anim=*s5++ (walking asset ptr); blk.glyph=anim+0xC; blk.chain=ctx[5]; ctx[5]=func_8007352C(&blk); saMotionSet(blk.anim,0) via lw 0x18(sp) reload; initTexPage(ctx[7],1,0,ret,0); ot_Link(D_800A374C + blk.otIdx*4, ctx[7]) - otIdx READ BACK from sp+0x2C; ctx[7]+=0xC; saTan3GaugeMain_8006A564(ctx, &blk, i) with sb 1(fp const),0x40(sp) in its delay slot.

- [s1] [fable-blitz 2026-07-07] Phase B counter icon (L201-243): s2 = ctx[1]->0x40 (second asset table); blk: otIdx=0xA, y=0x41, x=0(sw zero sp30), flag40=0, sp28=0; anim = s2[D_800A34F8 & 0xF] (frame-animated - table indexed by the global counter nibble); emit + saMotionSet/texpage/ot_Link(otIdx from sp2C) + ctx[7]+=0xC. Phase C (L244-285): re-emit drawarea with the FULL rect {a1[0],a1[1],a1[2],a1[3]} (4 lhu, NO constants) at ot_Link+0x28, and drawoffset {a1[4],a1[5]} (no yofs) at +0x28; both cursors +=0xC; s2 = ctx[1]->0x18 RELOADED (asset table 1 again).

- [s1] [fable-blitz 2026-07-07] Phase D 8th part (L286-398): same tri-state selector but vs literal 7 ((D_800A34F8&0xF)==7 / (D_8009BC04>>7)&1) and y=0xCF (+ D_800A34FC[7] lh in flash arm); rgb.b re-materialized as literal 0x30 addiu (s4 usage stops - GCC did NOT reuse s4 here; write literal 0x30, cse decides); blk.otIdx=9; anim = assetTbl1->0x1C (8th entry, direct lw 0x1C(s2)); emit + bracket + saTan3GaugeMain(ctx,&blk,7) + func_8006A494(ctx,&blk). Phase E (L399-435): TWO bare saMotionSet(blk.anim,0)+initTexPage+ot_Link(D_800A374C+4) brackets with NO sprite emit between (blk.flag40 zeroed via sb zero in first jal's delay); each ctx[7]+=0xC.

- [s1] [fable-blitz 2026-07-07] Phase F-G (L436-499): s2 = ctx[1]->0x24 (third asset table); F: blk {y=0x19, x=0, flag40=0, sp28=0, otIdx=0}; anim = s2[0x20/4 + (D_800A34F8&0xF)] (lw 0x20(v0) - table +0x20 base) - emit func_8007352C + bracket ot_Link(D_800A374C+0, no offset add). G: s2 reloaded; anim = s2[D_800A34F8&0xF] (base +0); blk gains SCALE fields sp38=0x200, sp3C=0x100, x=y=0; chain = ctx[2] (+0x8 - the SCALED-sprite chain, different cursor!); ctx[2] = func_80073728(&blk, 0) - the scaled emitter, exactly as in func_8006A494's completed body.

- [s1] [fable-blitz 2026-07-07] Phase H tile (L497-532): initTile(ctx[6], 0); then SEVEN field stores each preceded by a FRESH 'lw v0,0x18(s0)' reload of the tile pointer (sb 0->+4,+5,+6; sh 0x142->+8, 0x51->+0xA, 0x100->+0xC, 0x59->+0xE) - the C writes through ctx->tileCursor->field per statement (member pointer re-read each time because the sh through it may alias ctx; do NOT cache in a local); gpu_SetSemiTransp(ctx[6], 0); ot_Link(D_800A374C+4, ctx[6]); ctx[6] += 0x10. Phase I: final bare saMotionSet+initTexPage+ot_Link bracket; ctx[7]+=0xC; return.

- [s1] [fable-blitz 2026-07-07] Codegen inventory: NO jump table, NO GTE, 4 mults-by-shift only (the *63 and *47 sin-scale chains x2 phases), one do-while bit scan, 12 saved regs. GP-REL globals: D_800A34F8, D_800A3514, D_800A34FC, D_800A3524 all accessed %gp_rel (sdata) - declare them so they land in sdata (they already exist in src; check existing extern decls in text1b.c). The texpage bracket 'saMotionSet -> initTexPage(ctx[7],1,0,ret,0) -> ot_Link -> ctx[7]+=0xC' repeats NINE times - write it as repeated open-coded statements (the target repeats the full sequence; no helper existed).

- [s1] [fable-blitz 2026-07-07] m2c reference captured at tmp/blitz/m2c_func_8006A880.c (clean, 9.6KB). Templates ranked: (1) func_8006A494 completed body (block layout + emit idioms, same file), (2) this blitz's func_8005F1C8 recon (same block family, S60C8 typedef precedent at text1b.c:12949), (3) sibling ledgers memory/grind/func_8006A3CC + func_8006A494 (same saTan3 subsystem). saTan3GaugeMain_8006A564 (dist 197) should be drafted AFTER this lands - it shares the block and will inherit the struct typedef.

- [s2 manual 2026-09-26] FIRST FULL C BODY: floor 550 -> 0 (sandbox --disable all, 552/552) with candidate.c. Signature `void func_8006A880(u8 *arg0, u16 *arg1, s32 arg2)` (caller func_800693CC passes a2 = lbu D_8009BC0D[..]; unused here). Levers that each moved the floor (measured): (1) ctx accessed as `*(s32 *)(arg0 + off)` with u8 *arg0 (the func_8006A564 sibling spelling) — NOT in-struct, so the `arg0+0x24 += 0xC` store blocks the D_800A34F8 load (Phase D keeps its load-delay nop): 56 -> 39; (2) `u32 mask` + literal `1 << i`: the u32 promotion wraps the shift in a NOP_EXPR so fold-const's (1<<n)&x -> (x>>n)&1 rewrite does not fire -> target's hoisted `li a0,1; sllv; and` (no opaque `one` needed); (3) `cells = s.p0 + 0xC; s.p1 = cells;` as ONE function-scope local at all 5 sites -> the value seats in $a1 at every site (per-site/direct spelling puts it in v0: 86-era hunks 22/23); (4) the loop's sheet pointer and y are GIVs: `s.p0 = tbl[i]` and `y = i * 0x18 + 0x3F` in the body -> loop.c strength-reduction inits land after the hoisted 0x30/1 movables = target's pre-loop order (cursor/`y += 0x18` explicit bivs: 25); (5) Phase G store order w, x, y, h: 2 -> 0.
- [s2] ALLOCATION FACT (BB2_ALLOC_DEBUG, tmp/func_8006A880/rtl/*.alloc): target seats the D_8009BC08 scan mask AND the D_8009BC04 alive mask in the SAME callee-save $s6. As two C locals they are two pseudos; the scan mask (4 refs, livelen 44) gets pass-0 $s3/$s2 and the rest cascades: g2_sep (separate `alive`) = 46/551. One variable holding BC08 then BC04 = 0. global.c find_reg (pass 0 = used-so-far minus conflicts minus someone_prefers; prefs only from hard-reg copies) gives a separate non-overlapping pseudo the lowest free reg ($s4), so no two-local spelling can reach $s6 — the shared pseudo IS the target's shape. Policy: that is a multi-write local; admissible only as a staged-value borrow (alive = the real variable) or not at all — see hypotheses.md.

## s2 (manual lane, 2026-09-26, continued) — landing form and admission evidence
- **Landing form** = candidate.c (tmp/func_8006A880/v/final.c): sandbox --disable all 0/552.
  Full-build SHA1 == oracle 62efab4f… was proven 2026-09-26 with the equivalent pend.c body
  (same code; `i` shared by both loops) spliced over INCLUDE_ASM, then reverted. The landing
  form gives the row scan its own counter `bit`, initialised at the top with `row_mask`
  (`bit = 0` before the first call): score 0 (g2_bit3). Initialising it in the `for` header
  instead scores 7 (553 insns): the counter then crosses no call, so sched1 cannot lift
  `move s1,zero` above SetDrawArea and local-alloc gives it $v1 (target: $s1 at 0x8006A914).
- **Ruling 11** governs `sheets` (5 values) and `row_mask` (2 values): full (D) proof,
  dumps, mechanism, necessity and ablations in ruling11.md.
- **Ruling 9 (`cells = s.header + 0xC; s.table = cells;`, 5 sites, K = 0xC at every site)**:
  (Line numbers: src/text1b.c with this function's body spliced, as staged for the
  2026-09-26 landing.) (b) layout: SprtHdrA (text1b.c:11455, 12 bytes) / SprtEntA (8 bytes)
  walked by func_8007352C, and Ft4Sheet (text1b.c:11521, 12 bytes) / Ft4Cell (8 bytes)
  walked by func_80073728: a sheet is header(s) followed by cells, and `.table` (+4 of the
  descriptor) is the first cell. Other readers that store `sheet + 0xC` to `.table` the same
  way: func_8006A3CC (text1b.c:8300) and func_8006A494 (:8317)
  (`*(s32 *)(arg1 + 4) = *(s32 *)(arg1 + 0) + 0xC;`), func_8006B120 (:8717
  `p1 = s.p0 + 0xC; s.p1 = p1;`), func_80069F80 (:8155) / func_8006A1A0 (:8258)
  (`tbl = s.sp18 + 0xC; s.sp1C = tbl;`), func_8006A564 (:8424 `v1 += 0xC;` on
  `*(s32 *)(arg1 + 0)`), and the Ruling 9
  precedents func_8007636C / func_800759D0 (`cells = s.sp18 + 0xC`). Data: the root at
  ctx[1] (= D_800A34FC[9] = the func_8006E950(2, …) buffer, func_80068F70) is
  disc/TIM2D/MOD.BIN (sha256 9ef17b95…d140): it is the only TIM2D resource whose
  root+0x18 / +0x40 / +0x24 tables hold 8 / 8 / 16 sheets; SEL/SEL1/SEL2/NAR fail the
  census and STAFF/D_SEL do not fit (memory/grind/func_8006A880/census.py). Every sheet a
  write reaches (root+0x18 [0..7], +0x40 [0..7], +0x24 [8..15] SPRT, +0x24 [0..7] FT4 — the
  nibble D_800A34F8 & 0xF is kept 0..7 by func_800693CC: +1 wraps at 8, -1 wraps 0 -> 7) has
  exactly ONE 12-byte header, confirmed by the packed layout for ALL 32 sheets. Sorted by
  address, the 32 sheets form two runs (0xDB8..0x104C: 16 sheets, count 4; 0x121C..0x13F0:
  16 sheets, counts 2-4). Each of 30 ends exactly at the next reached sheet
  (s + 12 + 8*count). The two run-ends end at a header-shaped slot of another sheet:
  0x104C + 0x2C = 0x1078 {tp 0x1F/0, count 2, pad 0, cy 481} and 0x13F0 + 0x24 = 0x1414
  {tp 0x0A/0, count 3, pad 0, cy 484}. (census_MOD.txt's per-table "next sheet" column
  compares against the next TABLE entry, which for root+0x18 is not in address order; the
  address-order check is this paragraph, re-run 2026-09-26.) So +0xC is the first cell at every
  write; no anomaly path, no (b′) needed. The Phase G sheets are exactly the FT4-shaped ones
  (tp1 = 1), matching func_80073728 — independent corroboration of the slot reading.
  (i) receipts: per-write `s.table = s.header + 0xC` 15/552 (final_cdirect), one block-local
  `cells` per site 15/552 (final_cblock); dumps: final.alloc `ord=0 pseudo=77 hardreg=5`
  (one pseudo, $a1 at all five sites as in the target: `addiu a1,v0,0xC`); per-site pseudos
  are local-alloc'd into $v0 (p0 dies at the add). Structural respellings (explicit bivs 25,
  arg0[N] in-struct 39, s32 mask 86-era) and the permuter below.
- **Permuter** (tmp/perm_a880, per-value/carrier-free body ord0 = v/ord0.c, 2 workers,
  --stack-diffs, 13:16-13:38 CDT, 14,393 iterations, harvested + stopped): base 1840 ->
  best 1334 (found 13:20; no novel find in the last 18 min). Every find below base adds a
  shared `new_var = s.p0 + 0xC; s.p1 = new_var;` carrier — the Ruling 9 `cells` shape —
  and none shares the sheet tables or the masks (variable identity is out of the
  permuter's mutation reach, as in the func_8003800C finding).

# Evidence bank — func_8006F97C

## s2 (manual lane slotC3, 2026-09-26): 513 -> 0 (sandbox --disable all, 515/515)

candidate.c = the sandbox-0 body (s32-field descriptor, func_8007636C style). Ladder (every
number from `sandbox --disable all`, variants in tmp/func_8006F97C/):
- 115 first draft (v1). 95: player-count bound spelled `1 + D_800A35B0 + D_800A3554` (the
  func_80070C70 spelling; the other 4 orders 105-134).
- Header advance: target computes `(i*2 + i)*4 + 12` THEN adds s.header (asm:66-72). fold's
  associate (fold-const.c:3703-3757) turns `hdr + (i*12 + 12)` into `(hdr+12) + i*12` (95),
  and `(i+1)*12` keeps the +1 inside (79, 518 insns). `s.header + 12 + i * 12` (the
  func_8007636C spelling, text1b.c:12765) splits arg0 -> `hdr + (i*12 + 12)` = target. A
  12-byte-record pointer `+= i + 1` also works (pointer_int_sum distribution) — 71.
- 41: `s32 rec = i * 3; D_800A3560[rec]`. Mechanism (loop.c dump, tmp/func_8006F97C/dump):
  the inline index `D_800A3560[i * 3]` expands as sym-load THEN index insns (expr.c:4659
  INDIRECT_REF + EXPAND_SUM MULT), so the symbol pseudo has life 3 and loop.c hoists it
  (`Insn 809: regno 356 (life 3), move-insn savings 1 moved`; threshold*savings*life >=
  insn_count, loop.c:1631), leaving the register form `lui t1; addiu t1; addu; lbu 0()`. With
  the index already in a pseudo the sym set sits right before the plus (life 1, not moved) and
  combine folds it into `lbu %lo(D_800A3560)(at)` = target. D_800A358C keeps life 2 -> hoisted
  and rematerialized in t0, exactly as the target (asm:370-372).
- 40/37: `col == D_800A358C[i] && row == D_800A3588[i]` (target `bne a3,v0`).
- 7: grid arms each end with their own `s.header = ...; cells = s.header + 0xC; s.table = cells;
  s.out; s.ot_idx = 0xA; call`. Target proof: the static arm ends `lw v0,84(fp); addiu a0,sp,24`
  and the label .L80070014 is AFTER that addiu (asm:444-447), while the animated arm reaches it
  by `j .L80070014` with `addiu a0,sp,24` in the delay slot (asm:409-410). That is jump2
  cross-jumping two identical tails + reorg stealing the tail's first insn; a join-block tail
  cannot put `addiu a0` before the label (7 -> the nop/addiu hunk).
- 0: `s.x = row * 116 + (row >> 1) * 20;` (the (5*(row>>1) + 29*row)*4 forms 7-11).
- `cells` MUST be one variable spanning block 1 and the grid/tail: with a separate block-1
  local, the grid/tail pseudo is single-block -> $v1, the row counter gets $s8 instead of the
  target's spill at sp+0x58, frame 0x80 not 0x88 (96/510, tmp/func_8006F97C/v8.c).

**Sheet census (Ruling 9 (b) data).** root = D_800A35A8 = the buffer func_8006E950(3|4|5) loads
(text1b.c:9815-9838); ctx = *(root+0x60). disc/TIM2D/SEL.BIN / SEL1.BIN / SEL2.BIN (root word
[3]-[2] = 0x5FB80, the same LoadImage size as D_SEL.BIN): root+0x60 -> table @0x1C0, entry [0]
= 3 SprtHdrA headers (counts 2,2,2, cells at +0x24), entries [1..22] = 1 header each (cells at
+0xC). tmp/func_8006F97C/census.py (sha256s in its output). So K = 12 x header count at every
write: +0x24 on ctx[0], +0xC on ctx[1+idx], ctx[21], ctx[22] — always the first SprtEntA cell.
idx = D_8009BC40[col][row].value for rows 0..3 is 0..19 (EXE .data 0x8009BC40, read-only: only
`lbu` references in asm/funcs), so ctx[1+idx] ranges over [1..20]; all 1-header sheets.

## Layer-2 FAIL (2026-09-26, manual lane) — rejected/ruling9-block1-break-span-0.c
Sandbox 0/515, full-build SHA1 == oracle with it spliced (reverted, oracle re-confirmed).
Objection (sole ground): block 1's `cells` fails Ruling 9 prong (c) — loop 1, which contains
`break;`, sits between `cells = s.header + 0x24;` and `s.table = cells;`, and the text makes no
nested-loop exception; that loop-spanning site is what makes the pseudo callee-saved ($s1),
i.e. allocator effect. Ruling 5 1(c) (write and read in one block) fails too. Reviewer: a
borderline owner-policy question, not a hard ban. ACCEPTED (do not relitigate): `rec` named
intermediate, the duplicated grid draw tail (cross-jump byte-neutral; decisions.md:1425
precedent), the void* casts on D_800A35C4, `shift[2]`, the ordinary spellings.

Frontier after the FAIL — break-free block-1 forms (v11 chassis, only loop 1 changed):
- L1 `for (i = 0; i < n && D_800A3588[i] != 5; i++) {}` + `if (i < n) {...}`: 55/520.
- L2 same as a `while`: 55/520.
- L3 found-path exits by `i = n;` (no break): 6/519 — the target's found path is a direct
  `j .L8006FBB4` (asm:135/105); `i = n` must reload n and fall through the increment/compare
  (4 extra insns). Every exact form needs a jump out of loop 1 from the found path, i.e.
  break/goto/return inside the write->consume span.
- Moving the block-1 write after loop 1 is impossible: loop 1 advances s.header in memory and
  calls rsin/rcos, so `ctx[0] + 0x24` after the loop needs an extra `lw 0(fp)`; the target
  computes it at 0x8006FA40 before the loop.
- Per-site at block 1 only (grid+tail still sharing `cells`): 96/510 (ps3.c) — the grid/tail
  pseudo is callee-saved only because block 1 makes it live across loop 1's calls.
- Permuter from L3 (tmp/perm_f97c_l3, 2 workers, ~6,800 iterations, stopped): no find below
  base 600 (permuter-weighted); the base-equal finds only respell the `i = n` bound.
- candidate.c is now L3 (best body with no break in the span, 6/519); the 0/515 body is
  rejected/ruling9-block1-break-span-0.c. Owner policy-question filed in
  docs/grind/borderline.md 2026-09-26 "func_8006F97C".

## Ruling 9 prong walk for `cells` (v11 = candidate.c, 2026-09-26)
Sites: block 1 `cells = s.header + 0x24;` (ctx[0]); grid animated arm and grid static arm
`cells = s.header + 0xC;` (ctx[1+idx], ctx[21]); tail `cells = s.header + 0xC;` (ctx[22]).
- (a) every write feeds `s.table = cells;` (identical text), the descriptor's `.table` member.
- (b) every write is `cells = s.header + K;`, K in {0x24, 0xC}; BASE `s.header` (member of the
  local `s`) is assigned the sheet address immediately before each write (block 1 before the
  highlight advance). Sub-object: the first SprtEntA cell of the sheet (census above). Layout
  in this function's own bytes: header stride 12 (`s.header + 12 + i * 12`, asm:66-71
  `addu v1,a1,a0; sll v1,v1,2; addiu v1,v1,0xC`), cell stride 8 (`s.table += count * 8`,
  asm:261-265 `lbu v0,2(v0); sll v0,v0,3`). Other readers of `.table` as cells: func_8007352C
  (src/text1b.c:11242 as of 65de4bb16, `e = (SprtEntA *)env->table + i`); the same K scheme on D_SEL.BIN sheets in
  func_8007636C (text1b.c:12745/12763/12786/12798/12808) and func_800759D0.
- (c) grid arms and tail: write, then the consumer as the next statement. Block 1: write and
  consumer both at function scope, write first; between them sits loop 1 (the `== 5` scan),
  whose body contains a `break` that exits THAT loop and lands before the consumer — no
  statement between them can bypass the consumer. OPEN READING: whether a nested loop's own
  `break` counts as "a break between them" under Ruling 9 (c). The value must be computed
  before loop 1 (loop 1 advances s.header in memory; target `addiu s1,v0,36` asm:40, store
  `sw s1,0x1C(sp)` asm:148 after the loop), and the store must follow the loop.
- (d) target adds: 8006FA40 `addiu s1,v0,0x24`; 80070014 `addiu s1,v0,0xC` (shared by both
  cross-jumped grid arms); 8007008C `addiu s1,v0,0xC`.
- (e) every consumer store is followed, unconditionally in its block, by
  `arg0[4] = func_8007352C((s32)&s);` before the next consumer store or the exit.
- (f) `cells`: true of all four writes (same name as func_8007636C's re-audited carrier).
- (g) same statement list as ps1.c (declarations/identifiers only differ); every write is
  consumed before the next; s.header is re-assigned before every write; no split computation.
- (h) one declaration at function scope, the innermost scope enclosing all four writes.
- (i) receipts: hypotheses.md s2 (per-site 98, partial 96/98, allocation dump, permuter).

**permuter** (tmp/perm_f97c, per-site chassis ps1.c, 2 workers, --stack-diffs, 17:07-17:23 UTC,
~9,000 iterations, harvested + stopped): base 1800 (permuter-weighted) -> best 500, no zero.
The best finds all press a per-site cells local into service as a carrier of an unrelated
value: 500 `cells0 = i` (loop index), 515 `cells3 = col`, 650 `cells0 = shift[i]`, 665
`cells0 = col` — banned multi-role reuse. They confirm the target needs one callee-saved
pseudo live from block 1 through the grid; no legal per-site form appeared.

- [s1] [fable-blitz 2026-07-07] Rule inventory: ONE rule -- asmfix.txt:180 replace_with_asmfile; stub src/text1b.c:16465 `void func_8006F97C(s32 arg0, ...)` (only arg0=s4 is used -- real signature is (s32 arg0) or (GameObj*)). Distance 513; floor 513. Park = rejected distance>500 canonical misroute.

- [s1] [fable-blitz 2026-07-07] PRIMARY TEMPLATE: func_80070C70 (src/text1b.c:16512, INCOMPLETE-with-cheats but structurally proven) declares the exact request struct PrimC70 (:16492): p_geom(+0)/p_static(+4)/link(+8)/zero10/code(+0x14)/mode(+0x18)/zero1C(+0x1C)/width(+0x20)/height(+0x24)/byte28(+0x28), lives at sp+0x18 so code=sp+0x2C, mode=sp+0x30(x), zero1C=sp+0x34(y), width/height=sp+0x38/0x3C(=0x100 scale), byte28=sp+0x40(flag). THIS function extends it identically to func_8005D814's needs: u8 rgb triplet at +0x29..0x2B (sp+0x41..0x43, init 0x70,0x70,0x70; wobble writes sin-derived value to all three) and an s16 tail array at +0x30.. (sp+0x48+k*2, per-slot glyph codes 7 or 9). CROSS-LINK: func_8005D814's ledger (same blitz batch) needs the same extended struct -- solving either pins the shared typedef.

- [s1] [fable-blitz 2026-07-07] WARNING on the template: func_80070C70 carries cheat-asm (`register s32 c60 asm("$20")` constant pin and `__asm__("addiu %0,$0,1")` for prim.code=1, src/text1b.c:16513,16533) -- it is itself queue item (engine/queue.json:2623). Copy the STRUCTURE only; the constants here are plain (0xC/0xA/1 into code) and must be written as plain C. If the same reg-alloc friction appears, consult register-asm-pins rule for the legit ladder, not the pins.

- [s1] [fable-blitz 2026-07-07] Context/resource plumbing: ctx = *(s32*)(D_800A35A8 + 0x60) (fp reg; template uses +0x64 -- DIFFERENT slot, same pattern). Resource words: ctx+0 (main geom, used twice), ctx+4[idx] (per-icon geom array, lw ctx+4+idx*4 at :407-408), ctx+0x54 (static cell geom), ctx+0x58 (final overlay geom). arg0 fields: +0x10 = prim packet cursor (link chain through func_8007352C), +0x18 = texpage packet cursor (+0xC per initTexPage/ot_Link pair). OT rows: ot_Link(D_800A374C + 0x34 / + 0x2C / + 4).

- [s1] [fable-blitz 2026-07-07] Loops 1 & 2 (:53-147, :159-256) are source-level DUPLICATED blocks differing only in {match constant 5 vs 4; initial x 0x82 vs 0x17E} (y=0x86 both): scan i over D_800A3588[] (s16 state array, bound = D_800A35B0 + D_800A3554 + 1, blez pre-test = signed compare) for the first slot in state N; on hit: prim.p_geom += i*12 + 0xC (skip i icons); if (D_800A35C4[i*2] != 0 -- wait, hit-path tests lh (D_800A35C4 + i*2), the SLOT halfword) { t = *(s32*)(D_800A35C4+8): x += math_Sin((t*3)<<6 & 0xFC0)*5 >> 12; y += math_Cos(t<<7 & 0xF80)*3 >> 12; } else prim.byte28 = 1; then re-read slot s16 at D_800A35C4[i*2]: if == 0x1E exit, else y += (slot * math_Sin((t*9)<<5 & 0xFE0)) >> 12 (mult/mflo, NOT magic -- plain s16*s32 product); exit loop either way. Loop counter is s16 with sll16/sra16 re-extension; the i*2 index also appears as sra 15 of (i<<16) = i*2 -- write `(s16)i * 2`-shaped indexing.

- [s1] [fable-blitz 2026-07-07] Between scans: func_8007352C chains prim (link = arg0->0x10, result stored back); second block re-inits p_geom = ctx+0 AND FIRST does p_static = ctx->0 + 0x24 + (lbu p_geom[2])*8 update (:258-265: p_static(sp+0x1C) += *(u8*)(p_geom+2) * 8 after the first 7352C call, s1 initial = ctx0+0x24).

- [s1] [fable-blitz 2026-07-07] Grid loop (:292-472): for (row u16 sp+0x58 = 0; row < 4; row++) { s2=row(s16), s7=row*2, s6=row/2; for (col s3 = 0; col < 5; col++) { if ((D_800A35BC < 2 || D_800A35BC >= 4) && col == 4 && (row == 1 || row == 3)) continue; idx = D_8009BC40[col*12 + row*2] (u8 grid LUT); if (D_8009BC7C[idx] & 1) { ANIMATED: prim.zero1C=prim.mode=0 (y=x=0); scan k=0..bound: glyph[k](sp+0x48+k*2) = (D_800A3560[k*3] == 0xFF) ? 7 : 9; if (D_800A358C[k] == col && D_800A3588[k] == row && D_800A35C4[k*2] != 0) { byte28=1; v = ((*(s32*)(D_800A35C4+8)) & 0x1F) << glyph[k]; c = math_Sin(v + k*511)*63 >> 12 - 0x40; rgb[0..2] = c; break; } ; p_geom = *(s32*)(ctx+4 + idx*4); code=0xA; } else { STATIC: y = col<<4; x = (s6*5 + row*29)*4; byte28=0; p_geom = ctx->0x54; code=0xA; } p_static = p_geom+0xC; link = arg0->0x10; arg0->0x10 = func_8007352C(&prim); } }. NOTE k*511 spelled (k<<9)-k; x formula (5*s6 + 29*s2)*4 -- verify grouping from m2c.

- [s1] [fable-blitz 2026-07-07] Tail (:473-527): final overlay p_geom = ctx->0x58, x=y=0, flag=0, code=1, func_8007352C; saMotionSet(p_geom, 0x60); initTexPage(arg0->0x18, 1, 0, ret, 0); ot_Link(D_800A374C+4, arg0->0x18); arg0->0x18 += 0xC; icon quad sp+0x50 = {0xC6, 0x25, 0xF3, 1} (sh order 0x54,0x50,0x52,0x56 -- template's IconC70 init order is 4C,48,4A,4E: same shuffled store order, keep source order c,a,b,d); func_80069898(arg0, &icon, 1); replay_camera_attack(arg0); func_8006ECF4(arg0); D_800A32E8 = D_800A3564 (u8 gp-rel); D_800A32E9 = (u8)D_800A3554 (lhu->sb narrowing); func_80072E10(arg0); saTan3GaugeMain_80073200(arg0). All tail callee externs already declared at src/text1b.c:16482-16490.

- [s1] [fable-blitz 2026-07-07] The three saMotionSet+initTexPage+ot_Link triples all take the form initTexPage(arg0->0x18, 1, 0, saMotionSet(geom, mode), 0) -- the template at :16536 proves GCC nests the saMotionSet call INSIDE the initTexPage arg list (a3 = v0 addu pattern). saMotionSet modes: 0, 0x60, 0x60.

- [s1] [fable-blitz 2026-07-07] gp-rel/global inventory: D_800A35A8(s32 ptr), D_800A3554(s16, read lh AND lhu at :521 -- the lhu is for the u8 narrowing store), D_800A35B0(s32), D_800A35BC(u32 -- sltiu unsigned compares), D_800A35C4(s32 ptr to s16[] with a word at +8), D_800A3564(u8), D_800A32E8/E9(u8, gp-rel sb), arrays D_800A3588/D_800A358C(s16), D_800A3560(u8 stride 3); rodata LUTs D_8009BC40/D_8009BC7C(u8). Most already extern'd for the template at src/text1b.c:16475-16481.

- [s1] [fable-blitz 2026-07-07] m2c reference: tmp/blitz/m2c_func_8006F97C.c (274 lines, clean, no jtbl). Control-flow notes: the two scans exit via break-on-found (j to the common resume label); the animated-cell scan's found-path RE-ENTERS the post-scan code via j .L8006FF54 (loop-with-break, then shared tail); grid-cell skip uses the two-level sltiu window test on D_800A35BC -- expect `if (v >= 2 && v < 4) draw-all else if (col==4 && (row&1)) skip` sense inversion; check switch-vs-ifchain-branch-sense if the beqz/bnez senses fight.

## INHERITED FROM func_8006DD94 (written by func_8006DD94's s5/enumerate session, 2026-09-10)

Filed here on the Judge's explicit instruction in the 2026-09-10 07:42 ruling on func_8006DD94
("Rotate func_8006DD94 (and transplant these kills into func_8006F97C's ledger before it repeats
the search)"). func_8006F97C has func_8006DD94's EXACT stack layout - the same 0x2C descriptor at
sp+0x18..0x43, the same 12 untouched bytes at sp+0x44..0x4F, the same func_80069898 rect at
sp+0x50 - plus one extra u16 local at sp+0x58 (mapped in
tmp/grind/func_8006DD94/s2/spmap.txt, seven family members). Do NOT re-derive any of the below.

1. THE RESIDUAL IS ONE 8-BYTE FRAME DISPLACEMENT, not an expression problem. On func_8006DD94
   the honest body and the target are both 117 instructions and differ ONLY in `addiu sp,sp,-112`
   vs `-120`, the seven register saves, `addiu a1,sp,72` vs `80`, and the four rect `sh` offsets.
   Frame equation (mips.c compute_frame_size): ALIGN8(vars)+ALIGN8(args)+ALIGN8(gp_regs);
   target 64+24+32 = 120, honest body 56+24+32 = 112.

2. NO SPILL AND NO ALIGNMENT TRICK CAN FILL THE HOLE (class kill, predicate
   tools/gcc-2.7.2/function.c:724). MIPS leaves FRAME_GROWS_DOWNWARD undefined
   (tools/gcc-2.7.2/config/mips/mips.h:1645), so assign_stack_local runs `frame_offset += size`
   in ALLOCATION order; the rect's slot comes from expand_decl (tools/gcc-2.7.2/stmt.c:3392)
   during RTL expansion and every reload/global-alloc spill home is allocated afterwards, so a
   spill can only ever land ABOVE the rect. Measured on func_8006DD94: register-pressure probes
   p2_hoist6/p3_hoist10 do reach vars= 64 with a genuine sw/lw spill pair - and the rect never
   moves off sp+0x48. Alignment is closed too: BIGGEST_ALIGNMENT is 64 bits (mips.h:1082) and
   expand_decl clamps every BLKmode automatic to it (stmt.c:3419), so sp+0x48 is the first legal
   slot after a descriptor ending at 0x44.

3. THE DESCRIPTOR TYPE IS NOT LARGER THAN 0x2C. func_8006BB68 is COMPLETED-C and byte-matches on
   main with `S69E18 s; u16 rect[4];` and its rect at sp+0x48 (src/text1b.c:5754-5806); a 0x34
   descriptor would break it. A 35-caller census of func_8007352C found no caller anywhere that
   reads or writes descriptor-relative 0x2C..0x2F.

4. THE PHANTOM-FRAME-SLOT ROUTE DOES NOT EXIST HERE. The byte-verified witness
   (src/code6cac_c2.c:1290-1296, `s16 v1 = ...; s16 mask = ...; if ((v1 & ~mask) & 1)`)
   transplanted VERBATIM into func_8006DD94 reserves nothing (vars= 56), as do six further
   truthful HImode spellings, a long long multiply, a long long divide and a soft-float double.
   The witness's mechanism is register pressure, and this family already saves seven registers.

5. THE BLKmode keep-temp ROUTE IS EMPTY. It is the only slot mechanism ordered BEFORE a later
   expand_decl, but it needs a struct-valued expression; every callee in this family
   (func_8007352C, func_8006E480, func_8006D808, func_80069898, rsin, SetDrawMode, AddPrim)
   returns a scalar or void, and src/text1b.c declares no function with a non-scalar return type.

6. WHAT IS LEFT IS A POLICY QUESTION, ALREADY ANSWERED FAIL FOR func_8006DD94. The source must
   declare, between the descriptor and the rect, a stack-homed object (BLKmode, volatile or
   address-taken per tools/gcc-2.7.2/stmt.c:3357-3364) that no surviving instruction touches.
   Every spelling is either stripped by the sandbox - so the honest floor does NOT move even
   though the linked binary matches the oracle SHA1 (measured on func_8006DD94:
   `u16 rect0[4]; u16 rect[4];` gives sandbox 21 AND build_sha1
   62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches true) - or it is a construct the Judge
   has FAILed: trailing struct pads (layer-1 FAIL 2026-09-10 05:42), the merged frame-block
   struct (Judge FAIL 05:59), `u16 rects[2][4]` (layer-1 FAIL 06:36), the interior `volatile`
   pad (Judge FAIL 05:59 and 07:42). The frozen phantom-frame-slot pad family requires
   FIRST-DECL position (.claude/rules/no-new-park-categories.md:422-431) and first-decl position
   is measured to give the WRONG layout here (it displaces the descriptor from sp+0x18 to
   sp+0x20, sandbox 45).

7. DO NOT SPEND SESSIONS ON SPELLING SEARCH. Two exhaustive enumerations on func_8006DD94's
   chassis (the rect block, 65 spellings; the loop-tail descriptor-fill block, 973 spellings)
   found nothing below the floor. The residual is invariant to statement spelling by
   construction: frame offsets are handed out by DECLARATION order in assign_stack_local, not by
   how the consuming statements are written.

Full ledger: memory/grind/func_8006DD94/{evidence.md,hypotheses.md}; artifacts under
tmp/grind/func_8006DD94/.

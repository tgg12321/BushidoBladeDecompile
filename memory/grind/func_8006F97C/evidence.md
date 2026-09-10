# Evidence bank — func_8006F97C

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

# Evidence bank — func_8005F1C8

- [s1] [fable-blitz 2026-07-07] Queue state: distance 562, ASM-STRUCTURAL, PARKED (2026-06-09 audit REJECTED canonical-asm). 1 rule: asmfix.txt:182 replace_with_asmfile. src/text1b.c:12866 stub: void func_8005F1C8(s32,s32,s32,s32) - arg COUNT is right but return type wrong: function RETURNS s32 = (arg2+0x304) - arg2 = 0x304, the fixed byte size of the record it emits (computed at L582-585 from two SPILLED pointer locals, lw 0x80(sp) - lw 0x60(sp), NOT constant-folded - keep 'end - start' pointer arithmetic in C, don't return a literal).

- [s1] [fable-blitz 2026-07-07] Caller context (from THIS blitz's camera_set_target_zoom recon, memory/grind/camera_set_target_zoom/): called once per frame in mode 5 as func_8005F1C8(timeBuf(sp local, bytes +2=secs +3=centisecs), packedWins = wins0 | winsAlt0<<8 | wins1<<4 | winsAlt1<<12, D_800A38B4 cursor, 1); the caller advances its cursor by ret/4*4. arg0 = time buffer ptr, arg1 = packed win nibbles, arg2 = output buffer, arg3 = OT index (ot_Link uses D_800A374C[arg3]).

- [s1] [fable-blitz 2026-07-07] ENTRY SIGNATURE (distinctive): all four args are HOMED immediately - sw a2,0x60(sp) then lw t2,0x60(sp) back-to-back (L3-4), sw a0,0x50 / a1,0x58 / a3,0x68 - and thereafter accessed ONLY from the home slots (a1 reloaded at L38/336 for digit extraction, a0 at L336/427 for time bytes, a3 at L24 etc). Derived pointers precomputed at entry into MORE sp slots: 0x70=arg2, 0x78=arg2+0x2F8 (texpage cursor, ADVANCED +0xC/+0x10), 0x80=arg2+0x304 (end), fp=arg2+0xA0 (sprite-packet chain cursor, REASSIGNED to each func_8007352C return). 12 saved regs incl fp - max pressure; the homing falls out of it, don't force with volatile/aliasing tricks.

- [s1] [fable-blitz 2026-07-07] The param block at sp+0x18 is ONE aggregate local passed by address to every func_8007352C call (a0 = sp+0x18): +0x00(sp18) p0 anim-set ptr, +0x04(sp1C) glyph ptr, +0x08(sp20) chain-prev (fp written before each call), +0x10(sp28) 0, +0x14(sp2C) arg3, +0x18(sp30) x, +0x1C(sp34) y, +0x28..0x2B(sp40-43) rgba bytes (0x40=1 flag/0, 0x41=0xFF, 0x42/0x43=0x10), +0x30(sp48) s16 digit scratch. FAMILY PRECEDENT: completed func_800600C8 (src/text1b.c:12949) declares exactly this pattern as 'typedef struct {...} S60C8; S60C8 s; s.p0 = &...; cur = func_8007352C((s32)&s);' - reuse that spelling with THIS function's field set.

- [s1] [fable-blitz 2026-07-07] P1 win-mark pips (L30-133): triple loop, ALL counters s16 (every increment is followed by sll16/sra16 - declare s16 i,j,k): for (i=0..1 players) { count = (i==0) ? ((D_8009BD38 >> 14) & 1) + 1 : 2; byte = (arg1 >> (i*8)) & 0xFF; for (j=0..1 rows) { for (k=0; k < count; k++) { blk.p0 = &D_8009B5A0[i*12bytes]; x = (j!=0) ? i*8 - ((0x1C - i*8)*k - 0x1C2) : (0x1C - i*8)*k; nibble = (byte >> (j*4)) & 0xF; blk.glyph = (k < nibble) ? D_8009B5B8 + i*16 + 8 : D_8009B5B8 + i*16 (filled vs empty pip); blk.chain = fp; fp = func_8007352C(&blk); }}} - the k<nibble compare is 'slt k, nibble' with the FILLED glyph chosen on true (sw s0 default in delay, overwritten by sw s7).

- [s1] [fable-blitz 2026-07-07] AROUND the P1 inner call, five loop-carried temps (a1=j*4, a2=i-outer, a3=i-s16, t0=count>0 precomputed bool, t1=byte) are spilled to sp+0x88..0x98 BEFORE the jal and reloaded AFTER (L101-117) - GCC 2.7.2 caller-save.c behavior when temps live across a call outrank free callee-saves. Also note t0 = (0 < count) is computed ONCE before the j loop and tested at the loop HEAD (beqz t0 skips the k loop entirely) - C shape: the inner for's 'k < count' guard hoisted as a precomputed 'count > 0' for the first entry (loop_optimize guard hoisting on a loop-invariant bound).

- [s1] [fable-blitz 2026-07-07] P2/P7 texture bracket (identical twice): tex = saMotionSet(D_8009B5A0 [P7: D_8009B3A4], 0); initTexPage(texCursor, 1, 0, tex, 0-as-5th-stack-arg); [P2 only: texCursor += 0xC BEFORE ot_Link via the t3=a1+0xC/sw dance]; ot_Link(D_800A374C + arg3*4, texCursor-old). P2 then RE-ZEROES blk fields (sb 0x40, sw 0x34, sw 0x28, sw 0x2C=arg3, sp18=D_8009B3A4) - the block is re-initialized per phase, field by field.

- [s1] [fable-blitz 2026-07-07] P3 (L144-199): 2x2 loop: blk.p0=D_8009B3A4 (set once); glyph = D_8009B5D8 + i*8; y(sp30) = j*170 (the sll4/add/sll2/add/sll2/sub/sll1 chain = *170); fp-chained func_8007352C x4. P4 (L200-298): 2x2 initTile strip: prim cursor s1 = arg2+0xE walking +0x10/iter; initTile(arg2cursor, 0); rgb bytes 0xFF/0x10/0x10 at s1-0xA/-9/-8; s1-0x2 = 0x42 - i*16; s1+0 = 1; s1-0x4 = i*20+0x24; s1-0x6 = j*431 + j*(i*16) + 0x48 (mult s0,s5 for the cross term - check m2c for exact expr); gpu_SetSemiTransp(cursor); ot_Link(D_800A374C[arg3], cursor); cursor+=0x10; then TWO chained sprite calls: blk.p0=D_8009B3B0, glyph=D_8009B5F0+j... (s0<<4 both), blk.y(sp34)=i*20+0x24; then blk.p0=D_8009B3BC, glyph=D_8009B5F8+s0<<4; the SECOND call's chain arg is the FIRST call's return passed via sw v0,0x20(sp) in the delay slot (not through fp).

- [s1] [fable-blitz 2026-07-07] P5 timer digits (L299-530): consts s6=1, s4=0x2000, s0=0x66666667 (div-10 magic) held in callee-saves for the whole phase; blk.p0=D_8009B398, blk.y(sp34)=0x16; digit array = sp+0x30-relative s16[] at 0x30(sp)+k*2 (elements sp30/sp32? NO - reads/writes 'sh 0x30(v0)' with v0=k*2+s1base where s1=sp+0x18, so the array is blk fields +0x18.. = the x-slot REUSED as s16[2] digits + sp+0x48 = third digit slot). Digit flow per row j: k==0: digits[0] = arg0[2+j] (lbu secs/centisecs byte); if (mode==0x2000 && j==0): sp48 = sp48/100 leftover (0x51EB851F /100 magic); k>0 or general: digits[k] = digits[k]/10, remainder chain via mod-10: d = v - (v/10)*10 (the sll2/add/sll1 *10 reconstruction). Glyph = D_8009B400 + digit*8; digit==1 kerning: x += 3 (addiu v0,+0x3 arms at L415/500); x-base = (mode==0x2000) ? k*20+0x48 : k*20+0x34; glyph id override at L505-518: ((D_8009BD38 & 0x3000) == 0x2000) ? *(s16*)glyph = 0x109 : 0x113 (STORES INTO the glyph table entry through blk.p1!). Row1 (j==1) draws only k<2 digits (beq s5,s6 branch to the 2-digit path). Loop k<3 rows j<2. P6 separators: glyph=D_8009B5E8; x = j*6 + (mode2000 ? 0x145 : 0x13B); one call per j.

- [s1] [fable-blitz 2026-07-07] Mode test '(D_8009BD38 & 0x3000) == 0x2000' appears SIX times, each a FRESH lw+andi against s4=0x2000 held in a callee-save (constant-in-s-reg like camera_set_target_zoom's s0=2) - write each test as the literal expression; the 0x2000 pseudo unifies via cse. Same global's bit 14 gates P1's pip count ((>>14)&1).

- [s1] [fable-blitz 2026-07-07] Callee status: func_8007352C active dist-54 rules-11 (near-done), all others COMPLETED (saMotionSet, initTexPage, ot_Link, initTile, gpu_SetSemiTransp). Same-family siblings for later: func_8005D814 (PARKED 544 - the mode-3 record painter, likely same phases), func_8005E098 (active 288). The COMPLETED small siblings func_8005E51C (text1b.c:12857) and func_800600C8 (12967) establish the local-struct + chained-func_8007352C idiom AND the div/mod-10 digit decomposition spelling ('s.d0 = arg % 10; hi = arg / 10; s.d1 = hi % 10') - the divmod-coalesce-reuse-var reference memory documents the quotient/remainder register pattern.

- [s1] [fable-blitz 2026-07-07] m2c reference captured at tmp/blitz/m2c_func_8005F1C8.c (clean, 10.1KB). No jump table, no GTE, ~12 mults (all magic-div/mod-10 or small-constant multiplies by shift-add). The block-struct offsets MUST be pinned by a struct typedef (like S60C8) since its address escapes to func_8007352C.

## s2 (manual lane, slotJ, 2026-09-26) — first full C body: 562 -> 0

candidate.c reaches sandbox --disable all 0/564 (564/564 insns). Measured ladder
(scores are sandbox --disable all): first draft 268 -> 247 (x formula `i*8 + 0x1C2 -
step*k`: fold-const.c associate keeps `s5 - (t3 - 450)` only from this operand order;
`i*8 - (step*k - 0x1C2)` is re-associated to `(i*8+450) - step*k`) -> 217 (P3 x is
j*550 not j*170; tile stores in the func_8005E098 order r0,g0,b0,x0,y0,w,h so loop.c's
giv base is &tile->h = target's s1=tile+0xE) -> 214 (case-0 /100 vs /10 as ONE
if/else-if, so the /10 block is shared by the k==0 and k==1 paths = target's L740 with a
generic &d[k] address) -> 186/182 (s16 count: its HImode conversion gives target's
`move s6,v0` + hoisted `slt t0,zero,s6`) -> 78 (P1 outer/middle counters i,row distinct
from the later phases' counters: one `i` spanning P1..P4 crossed 5 calls and was
spilled to a stack slot; target keeps P1.i in caller-saved a2) -> 61 (prologue order
tile, cur, mode_off, end = increasing offsets; reload then inherits t2 exactly as
target) -> 6 (x0 = 0x48 + j*431 + j*(k << 4): `j*(k*16)` is re-associated by fold to
(j*16)*k = 61) -> 0 (wins s16: target masks at the assignment, andi t1 at i-level; s32
wins lets loop.c hoist the srav chain out of the pip loop (109), u8 wins re-extends in
the loop (6)).
- Empty pip = `s.p1++` after `s.p1 = &D_8009B5B8[i][0]`: target forms s7 = s0 + 8 from
  the filled-pip address (cse sees the stored s.p1 and adds 8); `&[i][1]` directly
  gives i*16 + (sym+8) instead (247 -> 230 before the frame fix).
- Counters: target register map P1 (i=a2, row=s4, k=s2), P3/P4 (outer s2, inner s3),
  P5 (outer s3, inner s2). Candidate uses k for {P1 inner, P3/P4 outer, P5 inner} and j
  for {P3/P4 inner, P5 outer}. Fully separate counters per phase measured 73 (sep_all),
  38 (P5 only separate), 73 (P3/P4 separate).
- count if/else (two writes, one value in Ruling 11's sense: both reach the same reads):
  the ternary `count = (i != 0) ? 2 : ...` measures 5 because fold swaps the constant arm
  first (branch layout flips). `x` (case 1) as a single ternary write = 0.
- Data: D_8009B5A0 = 2 x 12-byte Unk8009B398Record; D_8009B5B8 = [2][2] 8-byte records
  (single label, 0x20 bytes); D_8009B5D8 = 2 records; D_8009B5E8 = 1 record;
  0x8009B5F0..0x8009B60F = [2][2] records reached as 5F0 + j*16 and 5F8 + j*16 (the
  ORIGINAL binary adds one `sll s0,4` stride to both %lo(D_8009B5F0) and %lo(D_8009B5F8))
  -> needs the per-word-label aggregate merge of D_8009B5F8 into D_8009B5F0[2][2].

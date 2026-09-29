# Evidence bank — func_800720FC

- [s1] [fable-blitz 2026-07-07] Queue: distance 688, ASM-STRUCTURAL, parked (canonical audit 2026-06-09 REJECTED). Single rule = asmfix.txt:184 replace_with_asmfile. Empty stub in src/text1b.c (grep 'void func_800720FC'). No WIP, no near-dup lead. Tail calls func_80072E10 + saTan3GaugeMain_80073200 + func_8005C6D0 place it in the saTan3 (replay-mode gauge/menu) family; caller passes (ctx, texArray, mode) per src/text1b.c:16621-16624 `func_800720FC(a0, v0[0x1A], 0)`.

- [s1] [fable-blitz 2026-07-07] Signature: void func_800720FC(ctx *s4, s32 *texArr fp, s32 mode s7). Frame 0x98, saves ra+fp+s0..s7. Locals: descriptor struct at sp+0x18 (S60C8 family from COMPLETED func_800600C8, text1b.c:12949; also used by 25 func_8007352C + multiple func_80073728 completed call sites); RECT at sp+0x50 (halfwords); rect2 at sp+0x48; menu-data base ptr sp+0x58 = *(D_800A35A8->0x80) cached ONCE in prologue, consumed at s:423 and s:676 (lw 0x58(sp) reloads -- a stack slot, NOT a callee-saved reg).

- [s1] [fable-blitz 2026-07-07] Preamble: OT index 0x11 stored to descriptor field +0x14 (sp+0x2C) and REUSED as the ot_Link index by loading it back (lw $a0, 0x2C($sp)) at all 5 early ot_Link sites -- `ot_Link(D_800A374C + desc.otIdx*4, ...)`; rect = 4 lhu/sh pairs from *D_800A35C0 (u16 fields, no struct copy); initDrawArea(s4->0x1C, &rect); s4->0x1C += 0xC; D_800A35C4->0x10/0x12 = D_800A35C0->8/0xA; initDrawOffset(s4->0x20, D_800A35C4+0x10); s4->0x20 += 0xC.

- [s1] [fable-blitz 2026-07-07] Cursor-highlight block gated on D_800A3578 == 0 (s:67-169): entries 8 bytes at *(D_800A35A8->0x74)->0x10 + 0x18; selected index = (D_800A359C + mode*3)*2 + D_800A3598; skip when (D_800A359C*2 + D_800A3598 == 3 && D_800A35BC == 6 && mode == 2); then 6-cell loop s1=0..5 drawing non-selected cells with the SAME skip rule per cell (s1==3 && ...) and skip-if-selected (D_800A359C*2+D_800A3598 == s1); then saMotionSet(base->0x10, 0) -> initTexPage -> ot_Link -> s4->0x18 += 0xC.

- [s1] [fable-blitz 2026-07-07] CODEGEN TRAP #1 (must reproduce): descriptor fields +0x20/+0x24 (sp+0x38/0x3C) are stored TWICE back-to-back -- 0x100,0x100 (s:356-358) then immediately 0x180,0x120 (s:364-367). GCC 2.7.2 does NO dead-store elimination on stack MEM, so the original C literally contains both assignment pairs (copy-paste residue). Similarly sp+0x30/0x34 get -0xA/-0x5 early (s:70-73) then 0x90/0x28 at the join (s:171-174) -- the -0xA/-0x5 pair is conditionally consumed only in the D_800A3578==0 arm.

- [s1] [fable-blitz 2026-07-07] CODEGEN TRAP #2: two GCC magic-constant signed divisions in the smooth-scroll block (s:231-288): (a) magic 0x4325C53F + sra 7 + sign-fix subu = division by 488; m2c decodes the full expr as `*p = cur + ((target-cur)*0x3C0)/488` (tmp/blitz/m2c_func_800720FC.c:180); (b) magic 0x88888889 + sra 4 + sign-fix = /30 family on the s16-reloaded value, consumed as `viewportOfs[i] -= val/30`. Write plain C `/488` and `/30` and let expmed.c expand_divmod synthesize; do NOT hand-shape.

- [s1] [fable-blitz 2026-07-07] Smooth-scroll gating: D_800A3578 == 1 || == 3, then BOTH D_800A3584 and D_800A3580 in [4,6] via the sltiu range idiom `(u32)(x - 4) < 3` (s:210-218, addiu -4 / sltiu 3 on u16 loads); target/current rows = &D_8009BCB4[(s16)val * 2] (sll 16/sra 14 = s16*4 byte offset); loop exactly 2 iterations over D_8009BCD0[0..1] with 3 walking pointers (a2, t0, t1, +2 each) and t2 = 0,2 indexing D_800A35C4->0x10/0x12. else-arm zeroes D_8009BCD2 THEN D_8009BCD0 (that store order, s:292-295).

- [s1] [fable-blitz 2026-07-07] Pad input uses DOUBLED both-player masks as 32-bit literals -- 0xA000A000 (any dpad L/R?), 0x20002000, 0x80008000, 0x40004000, 0x10001000, 0x400040 -- via lui/ori + and (contrast replay_camera_attack's per-player sllv shifts); the per-player cancel loop DOES use sllv: 0x10 << (s1*16) with s3=0x10, s2=0xFF held in callee-saved regs (s:610-611). Cursor wrap: D_800A359C++ / -- then clamp (>=3 -> 0; <0 -> 2, s:473-484). Confirm path: action byte = lbu at menuBase + D_800A3580*8 + D_800A3598 - 0x1A, low nibble -> D_800A3584, ==0xF gate, high nibble -> D_800A3578, sfx 6.

- [s1] [fable-blitz 2026-07-07] Grid draw: sin pulse color identical shape to replay_camera_attack (math_Sin((D_800A35C4->8 & 0x1F)*128 + 0x1FF), rgb = (sin*63>>12)-0x40 into bytes +0x29..0x2B); func_80069898(s4, &rect2{0xB7,0x25,0x111,1}, 1); nested rows s1=0..3 x cols s0=0..1, fp advances +8/row (s6), cell ptr s3 +4; flag byte +0x28 = (s1==D_800A359C && s0==D_800A3598 && D_800A3578==0); cell (s1*2+s0)==3 && D_800A35BC==6 && mode==2 redirects src ptr to fp->0x30; each cell = func_80073728(&desc, 0). Then flag=0, fp->0x2C drawn. Second draw uses func_80073728(&desc, 1) after advancing p1 by *(u8*)(fp[1]+2) * 8 (s:376-386).

- [s1] [fable-blitz 2026-07-07] Exit/select block (D_800A3578==0, s:601-714): per-player loop while s1 < D_800A35B0+1 (bound reloaded per iteration), s0 += 3: on cancel button: sfx 2; D_800A3562[s0]=0xFF; if (D_800A3560[s0] == 5 || == 0x10 (s2/s3 const regs)) { D_800A3580=0; D_800A3562[s1==0 ? 3 : 0]=0xFF (the sltiu/negu/andi 3 idiom, s:635-637); D_800A3560[s0]=0xFF; } else D_800A3580 = 1; D_800A35C8=0xF; D_800A35CA=0x14; D_800A35C4->6=0; ->4=0; then jump PAST the confirm block (j .L80072B78). Confirm (pad & 0x400040): action = lbu(menuBase + D_800A359C*2 + D_800A3580*8 + D_800A3598 - 0x20); sfx 1; if (action==0xD && D_800A35BC==6) D_800A3568->0x14 = (w & ~0x3F0) | 0x250; else ... | ((action&0x3F)<<4); D_800A35A0 = 1.

- [s1] [fable-blitz 2026-07-07] Register plan observed: s4=ctx, fp=texArr, s7=mode live whole body; loop regs recycled per phase (s1/s0/s3/s5/s6 in grid loop; s1/s0/s2/s3 in player loop with 0xFF/0x10 as REGISTER constants -- loop.c invariant motion again); scratch t-regs only inside the call-free scroll loop. The heavy jal density (18 calls) forces everything live-across-call into s-regs/stack -- low RA risk compared to leaf decoders.

- [s1] [fable-blitz 2026-07-07] m2c reference: tmp/blitz/m2c_func_800720FC.c (369 lines, valid syntax, exit 0) -- good semantic decode including both magic divisions.

## Manual session 2026-09-29 (s2) — from scratch, 175 -> 25

- [s2] First full body (m2c-guided, tmp/func_800720FC/m2c.c) scored 175/690. Levers that moved it, each measured:
  - `D_8009BCC4[mode][0|1]` as its OWN extern (`extern s16 D_8009BCC4[][2];`), not `D_8009BCB4[mode + 4]`: the ARRAY_REF
    index `(mode+4)*4` is not distributed (only c-typeck pointer_int_sum distributes a constant term), and
    `(D_8009BCB4 + 4)[mode]` distributes but cse.c use_related_value then relates the scroll loop's D_8009BCB4 base to
    the +16 register (same symbol base). A separate symbol base is the only form with neither effect (103 -> 94).
    The asm/data labels agree: D_8009BCB4 (4 pairs) and D_8009BCC4 (3 pairs) are separate dlabels; the scroll loop
    reads D_8009BCB4[page] with page in [4,6], i.e. past its 4 entries into D_8009BCC4 (original out-of-range read).
  - grid loop: a per-row local `row = i * 2` + `((s32 *)arg1 + i * 2)[j + 3]` (pointer giv s3 walks from the row
    pointer, +12 stays in the lw offset) + the `!=`/`||` condition form (normal arm first, `beq s7,2 -> alt`): 93 -> 44.
  - headers read back through the descriptor (`s.header = X; cells = s.header + 0xC; s.cells = cells;`), one
    function-scope `cells` (s2 at every site, as the target): 44 -> 30. NOTE: `cells` is a multi-write local — the
    admission ruling must be settled before landing (Ruling 9 fails prong (c) because the first block reads the
    +0x18 write many times; see hypotheses.md).
  - confirm: separate `action` local from the L/R `code` local (target: a0 vs s0) -> first lookup matches.
  - confirm write as a 6-bit BITFIELD store (`Cfg720FC.sel` at +0x14 bits 4..9): the target's in-place
    `andi s0,s0,0xff` + `andi 0x3f; sll 4` order is exactly store_bit_field of a u8 value (zero_extend shared with
    the `!= 0xD` compare). Explicit-mask forms: `(action & 0x3F) << 4` is shortened to QImode by c-typeck
    (andi into a new reg); `(action << 4) & 0x3F0` keeps the in-place andi but emits sll-then-andi. 27 -> 25.
  - player loop: `slot = D_800A3560[ctx]` (s32, block-local) and the `[ctx]` store spelled `D_800A3560[i * 3]`:
    stops loop.c combining the two sym+ctx address givs (combined benefit > threshold -> pointer reduction); the
    target keeps `sym(s0)` addressing for all three. `other = i == 0 ? 3 : 0;` gives the target's sltiu/negu/andi.
- [s2] Remaining at 25 (sandbox --diff): (1) scroll-loop ABS registers (v0/v1 swapped, source-level class by
  alignment only); (2) rsin block `li 256` register/schedule; (3) `sll s5` vs `move s0,zero` order (row local is
  source-ordered before j=0; target has loop.c's hoist order); (4) D_800A35C8/CA: -G0 prices a symbol address 2
  insns, so cse relates `D_800A35C8+2` to the `D_800A35C8` register and loop.c hoists it (life 4, 29*1*4 >= 42);
  target has both stores direct gp_rel (the original's small-data pricing, cost 1, no relation). Open.
- [s2] 25 -> 6 (one declaration of D_800A35C8) / 0 (probe with a second scalar handle `extern s16 D_800A35CA;`):
  - rsin block: color stores before the scale stores (sched1 order puts 256 in v0 after the color math): 25 -> 19ish.
  - scroll ABS as `(d >= 0 ? d : -d) > (cur >= 0 ? cur : -cur)` (fold -> ABS_EXPR, operand order d first).
  - grid: `for (j = 0, row = i * 2; j < 2; j++)` gives the target's j=0 / sll / giv-init order (the target's i*2
    sits after j=0, i.e. loop.c hoist order); `i + i` shared between address and condition does the same.
  - scroll: ONE variable `d = to - from; d *= 30;` (Ruling 4 compound split) with `cur += d * 32 / 488` — the
    target's `subu a0` + `move a0,v0` is one global pseudo; a separate `diff` local is tied to the `to` load's
    dying register by local-alloc combine_regs (v1). Combine folds d*32 of d=(x*15)<<1 into (x*15)<<6.
  - candidate.c = one-declaration form, sandbox 6 (only D_800A35C8/CA). probes/alias_D_800A35CA_sandbox0.c =
    the same body with `D_800A35C8[0] = 0xF; D_800A35CA = 0x14;` + `extern s16 D_800A35CA;` -> sandbox 0/690.
- [s2] D_800A35C8/CA mechanism (measured): array element stores force their constant address into a pseudo
  (explow.c memory_address force_reg, via change_address for any ARRAY_REF/COMPONENT_REF; only a scalar VAR_DECL
  keeps (mem (symbol_ref))). cse.c use_related_value then rewrites the second address as (plus reg 2) — same
  symbol base, and -G0 prices a symbol 2 insns (mips.h CONST_COSTS, no SYMBOL_REF_FLAG) vs 1 for reg+off. The
  pseudo is used twice, so loop.c's "large loop" single-usage substitution (loop.c:721-765, which would put the
  symbol back into the MEM) does not apply and move_movables hoists it (life 4 * threshold 29 >= 42 insns).
  Both store orders relate (bare sym second is related through the related_value chain insert() builds).
  cc1psx (original compiler, -G0) on the same body ALSO hoists (tmp/cc1psx/func_800720FC/psx.o: lui s5/addiu
  s5,2 before the loop) — so the original TU did not see this array form: it saw two symbols (scalars) or a
  sized small-data array at -G8. func_8006F100 (completed) indexes D_800A35C8[i] with a lui/addiu base in its
  target (non-small array in ITS original TU) — evidence of different declarations in different original TUs.
- [s2] D_800A3578: the target reads it with `lh` here (and in func_80070188 / func_80070F78), `lhu`+`srl` only in
  func_8006EC0C (through its `u16 word` local). Retyping BOTH text1b.c declarations `extern u16 D_800A3578;` ->
  `extern s16 D_800A3578;` is byte-neutral for the whole current text1b.o (tmp/func_800720FC/retype.py: objdump
  -s -r identical, 20283 lines) and lets this body test `if (D_800A3578 == 0)` plainly (lh). Every access stays
  ordinary C (EC0C's `u16 word = D_800A3578;` zero-extends). candidate.c: 7 with u16, 6 with the retype.
  (`(s16)D_800A3578 == 0`, `!(s16)x`, `(s32)(s16)x`, `x << 16` all fold to lhu; only a separate s32 local or the
  retype give lh.)
- [s2] PROOF FORM (probes/qform_alias_CA_retype3578_oracle.c): candidate + `extern s16 D_800A35CA;` +
  `D_800A35C8[0] = 0xF; D_800A35CA = 0x14;` + the s16 retype: sandbox 0/690 AND full-build verify-oracle
  --rebuild --allow-dirty SHA1 62efab4f... == oracle (2026-09-29; src reverted and rebuilt green afterwards).
- [s2] Access-form census (asm/funcs, address order): D_800A3578 — 8006E534 sh; 8006EC0C lbu/lhu/sh (u16);
  8006F528 lbu; 80070188 lh/sh; 80070F78 lh x5 / sh x3; 80071C4C sh; 800720FC lbu x4 / lh / sh.
  D_800A35C8/CA — 8006F100 `lui/addiu %hi/%lo(D_800A35C8)` array base (non-small array in its TU);
  80070188, 80070F78, 800720FC direct gp_rel stores to D_800A35C8 AND D_800A35CA (two symbols / small data).
  Both flips fall between 0x8006F100 and 0x80070188: evidence the original had a TU boundary there.
- [s2] BLOCKER (policy): with the single `extern s16 D_800A35C8[]` no C spelling reaches the target (floor 6):
  measured both store orders, pointer, comma/duplicated forms reasoned out (cse relation within one EBB; jump2
  cross-jump cannot produce the target's `lw D_800A35C4` -> C8 -> CA order); cc1psx hoists the same way. A second
  declaration of the same bytes is allowed only per FILE (no-new-park-categories aggregate-merge exception to
  prongs (c)/(d), Q21). Filed as a QUESTION to the orchestrator 2026-09-29.
- [s2] Same class, inside this function: D_8009BCB4 (4 pairs) is indexed [page], page 4..6, which reads the bytes
  D_8009BCC4 names; the single-base form `(D_8009BCB4 + 4)[mode]` costs 9 (cse relates the scroll base), so the
  two symbol bases are required too.
- [s2] Still to settle before landing (independent of the blocker):
  - `cells` (6 writes: s.header+0x18 once, s.header+0xC x5) -> Ruling 11 package (dumps, ablations, permuter
    from the split body). Split measured: one-local-per-value (first block `tbl`, rest `cells`) = 119.
  - the dead 0x100 scale stores before the 0x180/0x120 override (asm 356-367: both pairs are in the bytes;
    GCC 2.7.2 keeps dead stores to the escaping stack struct) -> dead-store family FAKE annotation + receipts.

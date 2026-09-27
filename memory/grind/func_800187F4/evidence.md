# Evidence bank — single_game_setModeRequest

- [s1] [fable-blitz 2026-07-07] Queue: distance 642, ASM-PARTIAL, ACTIVE. Single rule = asmfix.txt:104 replace_with_asmfile. Empty stub src/code6cac.c:604 `void single_game_setModeRequest(s32 arg0, s32 *arg1) {}`. NAME IS WRONG: named_syms.txt:1563 assigns the Kengo name, but Kengo's single_game_setModeRequest (0x13c670, src/numata/nm_single_game.c, dumped via `bash tools/wsl.sh 'python3 tools/kengo_ref.py single_game_setModeRequest'`) is a 9-case jr-jump-table mode dispatcher over lbu mode bytes -- ZERO structural overlap with BB2 0x800187F4 (no switch, no dispatcher; calls marionation_camera_Exec then runs particle physics). The kengo:HIGH '+1 near-exact' lead is a FALSE MATCH from the old naming pass; do NOT use the Kengo source as a template. (Candidate real kin: Kengo himo/rope/cloth sim functions -- unverified.)

- [s1] [fable-blitz 2026-07-07] True signature: void f(Ctx *s5, Desc *s2). Calls marionation_camera_Exec() FIRST (jal in prologue shadow, s:14). count = (s16)desc->0x4 (lh -> $s4, immediately spilled to sp+0x48 and RELOADED at the loop bottom s:677 -- s4 is recycled as a GTE staging temp inside the loop, so the count lives in a STACK slot); particles = desc->0xC, stride 0x40 (t8 walks +0x40, t0 = t8+8 walks with it); anchor pairs = desc->0x0 + i*8 (s0 += 8).

- [s1] [fable-blitz 2026-07-07] Scratchpad workspace map (all 0x1F80xxxx absolute, $fp pinned to 0x1F800000 base and $s3 = 0x1F8000BC held all loop): +0x00..0x14 collision deltas/temps; +0x18/1C/20 velocity vector; +0x24/28/2C prev-frame pos delta; +0x30/34/38 SQR MAC outputs; +0x3C/40/44 position; +0x48/4C/50 pos>>5; +0x60 sphere count; +0x70/74/78 sphere prev/cur tables (stride 0x18); +0xAC radius table (t3 += 4); +0xB8 ground Y; +0xBC/C0/C4 impulse force tables (stride 12). Register constants held across loop: $s7=3 (byte-in-word counter limit), $s6=-2, $t9=0x16 (LZC shift bias), $s3, $fp.

- [s1] [fable-blitz 2026-07-07] Per-particle state = t0[0x10]: (A) state >= 0: anchor path -- lwc2 VXY0/VZ0 from anchor pair, `mvmva 1,0,0,0,0` (encoding 0x4A484812... target word 1200484A = .word 0x4A480012? verify from bytes: insn 0x4A480012? s:51 shows 1200484A LE = 0x4A480012), MAC1/2/3 (swc2 $25/$26/$27) -> scratch 0x18/1C/20 = rotated target dir; if state > 0: per-axis spring integrate: vel = vel/2 (srl-sign-fix sra 1) + ((target<<7 - pos_updated)*state)>>8; pos += vel; Y axis adds gravity bias 25*(0x100-state) into the mflo sum (s:103-116); if state == 0: pos = target<<7 snap (s:148-162). (B) -0x100 <= state ... wait, state in [-0xFF,-1]: REVERSE damp -- same mvmva, then pos -= (target<<7 - scratchpos)*state>>8 per axis (s:163-221, subu variants). (C) state <= -0x100: skip straight to free physics.

- [s1] [fable-blitz 2026-07-07] Free-physics common block (s:222-297): vel triple from t0[4/8/C]; TWO impulse loops (t9 = t0[0x14] count then t0[0x18] count): per impulse, index byte pulled from packed words t0[0x1C]+t0[0x20] (first loop, ADD forces) / t0[0x24]+t0[0x28] (second loop, SUB forces) -- byte extraction via `andi 0xFF` then `sra 8` each iteration EXCEPT every 4th (t2==3 -> load next word, $s7 compare); force tables at scratch 0xBC/0xC0/0xC4 indexed by idx*12 (synth chain x3<<2). Then vel -> scratch 0x18/1C/20.

- [s1] [fable-blitz 2026-07-07] Collision (only if ctx->0xC != 0): ground: dy = y - *(0x1F8000B8); 0 < dy <= 0x3200 -> vel.y -= dy/8 (bgez/addiu 7/sra 3 = C `/8`); dy > 0x3200 -> vel.y -= 0x400 (s:302-323). Then pos>>5 triple -> 0x48/4C/50. Sphere loop over *(0x1F800060) spheres: per-axis early-reject |d| > r (slt pairs vs negu r, 6 reject branches); dist = isqrt(dx^2+dy^2+dz^2) via GTE SQR (lwc2 IR1/2/3, `sqr 0`, MACs -> 0x30/34/38, sum) then: sum < 0x400 -> D_8008D118[sum]>>3 table lookup ELSE LZC-normalized: mtc2 sum -> $30, swc2 $31 -> stack slot sp+0x10/0x14, shift = 0x16 - (lzc & ~1), table[sum >> shift], result = (table<<16) >> (0x13 - shift/2) (s:410-438 and the second copy s:506-534 -- TWO open-coded instances with different stack slots sp+0x10 vs sp+0x14).

- [s1] [fable-blitz 2026-07-07] Sphere response (s:535-616): needs dist1 (to sphere cur) + dist2 (to sphere prev) < r; pen = r - (dist1+dist2), v1 = min(pen<<17, 0x400000); mtc2 1 -> $8 (IR0=1); lwc2 vel; `gpf 0` (encoding .word 0x4B90003D); scale1 = v1/dist1 (runtime div + break 7/6 guards -- plain C `/`), lwc2 delta1 from 0x0, mtc2 scale1 -> IR0, `gpl 1` (.word 0x4BA8003E) accumulate; scale2 = v1/dist2, lwc2 delta2 from 0xC/0x10/0x14, gpl again; swc2 $9/$10/$11 (IR1/2/3) -> vel 0x18/1C/20.

- [s1] [fable-blitz 2026-07-07] Writeback (s:626-673): t0[4] = vel.x*7/8 (sll3/subu/sra3); *t8 = pos.x + prevdelta(0x24) + t0[4]; t0[8] = vel.y*7/8 + 0x190 (GRAVITY constant); t8[-4... t0[-4]] = pos.y + 0x28-delta + t0[8]; t0[0xC] = vel.z*7/8; *t0 = pos.z + 0x2C-delta + t0[0xC] via the SHARED join .L800191F4 (the state>=0 paths jump here with their own final z in $v0 -- one sw serves 3 paths, cross-jump-friendly tail).

- [s1] [fable-blitz 2026-07-07] EVERY GTE idiom needed has a COMPLETED same-file precedent: mvmva as `.word 0x4A486012`-style with pinned t-regs (src/code6cac.c:2312, code6cac_b.c:1599/2156/2218), mtc2 %0 placeholders (code6cac.c:2242), the EXACT LZC mtc2 $30 idiom `.word 0x488CF000` (code6cac_b.c:1525 -- same isqrt-with-table pattern), and the D_8008D118 sqrt-table + <0x400 direct-lookup shape in plain C at func_8001A67C (src/code6cac.c:975-990, COMPLETED). The swc2/lwc2 groups match func_80019310 (code6cac.c:931, the immediately-adjacent completed GTE trampoline sharing the same caller func_8001924C).

- [s1] [fable-blitz 2026-07-07] Scratchpad access shape: absolute-address globals (lui 0x1F80/lw pattern with SEPARATE lui per access, and $at-relative forms) -- these are `*(s32 *)0x1F80xxxx`-style or extern-scratchpad-symbol accesses; see .claude/rules/scratchpad-gte (auto-scoped to text1b/display) for the established declaration recipe; note $fp holds 0x1F800000 for ONE store only (s:375 sw v1, 0(fp)) while everything else re-materializes lui -- a base-reg CSE quirk to watch in the first diff.

- [s1] [fable-blitz 2026-07-07] m2c reference: tmp/blitz/m2c_single_game_setModeRequest.c (303 lines, valid syntax; cop2 ops become M2C_ERROR stubs). No memory/wip. No near-dup lead in tmp/duplicates_leads.txt (the GTE siblings above are better leads than any dup-scan hit).

- [s1] [fable-blitz 2026-07-07] Naming action for the closing session: the function should eventually be renamed (marionation rope/cloth particle sim -- e.g. mario_himo/cloth_Exec family); renaming touches named_syms.txt (build input) so it belongs to the implementing session with a full oracle rebuild, NOT recon; until then keep the symbol as-is to avoid oracle churn.

## [s2] manual worker slotQ, 2026-09-26/27 — chassis to 2 insns (real pipeline)

Name note: `single_game_setModeRequest` above is the retired name; the symbol is
func_800187F4 (rope/cloth particle integrator; caller func_8001924C).

Measurement: tmp/func_800187F4/fast.sh = the Makefile's exact per-file recipe
(cpp | build cc1 | prologue_fix | maspsx | multu_pad | as) on code6cac.c with the
body spliced, objdump'd and difflib-compared to build/src/code6cac.o (NO cheat
stripping). `sandbox --disable all` cannot score the verbatim form yet: the
engine keeps only PINNED header units (engine/gtemacro.py) and strips the
`move $12,%0` / `nop` statements of ldlvl, stlvl, lddp, sqr0, gpf0, gpl12,
rtv0tr (not pinned).

candidate.c (= template.c through gen.py, verbatim inline_o.h statements):
real-pipeline diff = 2 lines, 662/662 insns: ONLY the LZC-1 input copy
(target `beqz v0; move a0,a1` [delay] ... `move t4,a0`; ours `nop` ...
`move t4,a1`). Frame, every s-register, every loop, both LZC blocks otherwise exact.

Load-bearing findings (each measured, fast.sh):
1. VERBATIM header statements are load-bearing, not cosmetic: joined islands
   (one __asm__ per macro) are ~39 RTL insns short inside the sphere loop, so
   loop.c's desirability test (threshold*savings*lifetime >= insn_count,
   threshold -3 per move, loop.c:1631/1719) hoists the `19` constant (target
   keeps `li v0,19` in-loop). Separate statements: sphere loop N=199, 22 moves
   (50*4=200>=199), 19 does not (47*4=188<199) — exactly the target. Same fact
   func_800288C8/func_8002A458 found (+RTL insns fix seats).
2. Scratchpad as `#define SCR ((RopeScratch *)0x1F800000)` constant-pointer
   struct (all accesses fold to constant addresses; a local pointer var puts
   everything through a register — t1 score 413). The `sw v1,0(fp)` d0[0]
   store is cse's canonical 0x1F800000 temp from `SCR->rad[j]` indexing,
   hoisted twice by loop.c (fp) — falls out once allocation matches.
3. Counts: `n = node[7]` / `n = node[8]` locals (else loop.c adds a move).
4. ONE counter `j` for both force loops AND the sphere loop: global.c
   priority floor_log2(45)*45/244 = 9221 < node+8 giv (10135) and n (10000) so
   the counter lands in $t2 after them (target). Separate counters: 34042 ->
   $a0, shifts vx/vy/vz/bits by one reg (diff 152 -> 85).
5. dist1/dist2 hold the squared length in place (`dist1 = sq0+sq1+sq2; if
   (dist1 < 0x400) dist1 = LUT[dist1] >> 3; else {...}`): r -> $a2, dist -> $a1
   / $a0 as target (diff 85 -> 66).
6. LZC arm: `shift = lz[k]; shift = 0x16 - (shift & ~1); byte =
   LUT[dist >> shift]; dist = (byte << 16) >> (0x13 - (shift >> 1));`
   (func_80018300's `len = lz[0]; len = 0x16 - (len & ~1);` split). Every other
   spelling of the and/shift measured worse (tmp/func_800187F4/mklzc.py sweep:
   no-half/half/byte/in-place/`&-2`//2 variants 30-395).
7. Frame 0x78: `s32 lz[6]` (lz[0]/lz[1] = the two LZC outputs, sp+0x10/0x14);
   lz[5]/[6] match, lz[2] or two scalars leave vars=48 (count slot 0x38 vs
   0x48). Phantom 8-byte spill slots come from the 4 loop-guard orphan-USE
   pseudos (combine distribute_notes, combine.c ~10839) — target = 4 + a
   24-byte locals object, the same oversized LZC-output object func_80018300
   (lz[6]) and func_80018094 (sp_tmp[4]) carry.

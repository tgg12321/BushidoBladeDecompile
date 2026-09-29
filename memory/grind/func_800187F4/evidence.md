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

## [s2 cont.] REAL-PIPELINE BYTE MATCH (diff 0/662) — candidate.c = template g2
Added to the 2-insn chassis:
8. LZC-1 input copy staged in `byte` (`byte = dist1;` before the if; gte_Lzc(byte,
   ...); byte later = LUT byte) — the func_800288C8 `tbl` / func_8002A458
   `lzc_in` shape (Ruling 11 (C)(3) names those as FAILED bare copies). A FRESH
   single-write copy (`lzin = dist1`) survives cse (asm operands are not
   canonicalized when the arm is a separate path) but global.c seats it in $v1
   (conflicts only $v0/$t4-7; lowest free), target $a0 = 2 insns.
9. The copy adds 1 RTL insn in the particle loop, which pushes the hoisted `-2`
   (pri 140000/len) from 157 to 156 = tie with `3` -> s6/s7 swap. Fixed by
   ONE ground-collision store: `if (d > 0x3200) t = vy - 0x400; else t = vy -
   d / 8; SCR->vel[1] = t;` (target's single `sw` at 0x80018CBC, after the join label .L80018CB8, is shared;
   two stores = one extra RTL insn that jump2 cross-jumps away). A `?:` into
   the store keeps two stores (expand stores per arm).
Policy exposure still to reduce before any landing: `byte` copy (banned
shape), shared counter j (3 loops), n/bits/d/dist rewrites, `t`, lz[6],
shift split; islands need PINNED entries + DMPSX words (4 command words).

## [s2 close] slotQ 2026-09-27 — honest floor, load-bearing reuses, frontier
Two banked bodies (both real-pipeline, tools/fast.sh; generate with gen.py):
- template.c / candidate.c (= g2): 662/662 BYTE MATCH, but carries the LZC-1
  input copy in `byte` (banned tbl shape) + a `t` single ground store.
- template_copyfree.c / candidate_copyfree.c (= h5): copy-free, real-pipeline
  2 lines off (only the LZC-1 copy: target `move a0,a1` [beqz delay] and
  `move t4,a0`; ours nop / `move t4,a1`). `sandbox --disable all` = 147/644
  (570 build insns): the sandbox strips the 41 non-PINNED `move $12,%0`/`nop`
  header statements (ldlvl/stlvl/lddp/sqr0/gpf0/gpl12/rtv0tr), so it is NOT
  the honest measure until those macros are PINNED (engine: commit + layer-2).
Load-bearing reuses measured on the copy-free chassis (each split alone):
  counter j shared by force loops + sphere loop (split 85->152 at s2 start);
  n (node[7], node[8]) split 21; d (ground delta + six sphere-axis deltas):
  ground-only or axis-only split 6; `byte` shared by both LUT roots split 42;
  dist1/dist2 in place (squared length -> root -> pen/dist scale): root split
  85 vs 66, scale split 53; pen compute-then-clamp (?: clamp 57).
Byte-neutral splits (use them): bits/bits2 per force loop (0), shift/shift2
  per root block (2 = unchanged).
Every one of j, n, d, byte, dist1, dist2 needs a Ruling 11 (D) dump proof +
honest generic/kind name + annotation, or an equivalent single-role form.
Borderline question filed 2026-09-27 (docs/grind/borderline.md): the LZC-input
copy (shared idiom with func_800288C8/func_8002A458; func_80018094 landed it).
Islands: verbatim inline_o.h statements are REQUIRED (joined -> 19 hoisted).
Admission needs PINNED entries for 7 macros and the 4 command words are DMPSX
placeholders (0x4A480012 rtv0tr, 0x4AA00428 sqr0, 0x4B90003D gpf0, 0x4BA8003E
gpl12) -> class route excludes them; needs per-function owner row (func_8002DE20
Q11 precedent) with independent word sources (nugget inline_n.h / PSn00bSDK).
cc1psx-check (s2): ours 2, cc1psx 113 -> SOURCE-SIDE.

## [s3] manual 2026-09-28 — owner rulings Q28/Q29, recognizer, landing body 0/644

- **Owner rulings** (record 206e77db8, layer-2 PASS round 2): Q28 — the LZC-input copy counts as a
  Ruling 11 value under the new (C)(3) "GTE-macro input copies" clause (borderline 2026-09-27 option A);
  Q29 — per-function grant for the four DMPSX command words (rtv0tr 0x4A480012, sqr0 0x4AA00428,
  gpf0 0x4B90003D, gpl12 0x4BA8003E; sources nugget inline_n.h + PSn00bSDK inline_c.h).
- **Recognizer** (engine 21b9bbebd, layer-2 PASS): PINNED gte_ldlvl/lddp/rtv0tr/sqr0/gpf0/gpl12/stlvl
  + the four DMPSX_WORDS. Sandbox now scores the verbatim body as written: candidate 147 -> 0,
  copy-free 147 -> 2 (recognizer_treewide.md: no src function moves).
- **Declaration** (65f4f1730, oracle-green): `func_800187F4(s16 *arg0, s32 *arg1)`, the caller's type.
- **Landing body** = candidate.c (template.c through gen.py): sandbox **0 (644/644)**; the spliced
  code6cac.o is byte-identical to the INCLUDE_ASM build in .text/.rodata/relocations. Versus the s2 g2
  template: `s16 *arg0` + sibling-style offset casts; `force[0][3]`; per-loop `f_add`/`f_sub` and
  `bits`/`bits2`, `shift`/`shift2` split (all byte-identical); `dist2` split into `sq2` + `dist2`
  (byte-identical); `d` reduced to `delta` = ground depth + Y delta to focus 0 (the 127-partition
  ablation: 0 iff the ground depth shares with at least one sphere delta; the other five deltas are
  own locals); every reused local renamed and scoped for Ruling 11 (idx, nforce, temp, work, delta,
  nbits, nbits2), each annotated.
- **Ruling 11 package**: r11/proof.md — per variable the dumps (then r11/dumps_table.txt; since v5 r11/dumps_table_landing.txt), the mechanism
  (global.c allocno priority for idx/nforce; local-alloc single-block quantities for temp's table
  bytes; cse.c make_regs_eqv class head for work and delta; local-alloc combine_regs tie for nbits),
  the necessity argument, the measured alternatives (r11/measurements_v1.md: per-value 67/21/40/26/4/2/2 sandbox,
  ablations, 14 structural respellings, 17 FAKE-family probes), two permuter campaigns (25,116
  iterations, no find reaches 0; campaign 2's gains re-create reuses).
- **Disclosed alternative**: per-value `dg` + FAKE dead store `dg = 0;` after the ellipsoid loop also
  reaches 0 (cse counts the dead use before flow deletes it); not landed (adds a FAKE construct, the
  reuse adds none; Ruling 1(4), func_8008B488 precedent).
- **lz[6]**: lz[5]/[6] frame 0x78 (0), lz[2] 0x68, lz[3]/[4] 0x70 and lz[7]/[8] 0x80 (24 each) on the landing
  body (corrected 2026-09-29 by the sixth layer-2: lz[3]/[4] had been recorded as 0x68; builds
  r11/reviewer_probes/rv6_lz2..rv6_lz7, tmp/func_800187F4/fast_lzc5_*).

## [s3 cont.] 2026-09-28 — layer-2 FAIL of the first landing, v2 chassis c6

- The first landing (637f8431b's body) FAILed a fresh layer-2 (rejected/r11-work-delta-reuse-layer2-fail-0.md):
  `work` and `delta` each byte-match as one-variable-per-value spellings plus one sanctioned FAKE dead store,
  so their Ruling 11 (D)(3) necessity fails (regno_last_uid counts sets, regclass.c:1763-1764; my v1 work
  probe had put the store before temp's last use). Nothing was committed; src reverted.
- **v2 (template.c = c6)**: `sq1` + `dist1` and `depth` + `dy0` single-value locals, each closed by a
  FAKE-annotated dead store (`sq1 = 0;` before `tot`, `depth = 0;` after the ellipsoid loop;
  dead-store-fake-exception): 0/662, sandbox 0 (644/644); each store load-bearing (4 / 26; both 30).
  Scratchpad type renamed Scr1F800000; header text: the state -0xFF..-1 path pulls then integrates.
- Five Ruling 11 locals remain (idx, nforce, temp incl. the Q28 copy, nbits, nbits2), re-measured on c6
  (r11/measurements.md: 67 / 21 / 40 (27, 33, 29 ablations) / 2 / 2; all 98) with systematic FAKE sweeps:
  dead stores at six anchors (72 variants + 5 per-value baselines) and self-assigns + chain-extenders (132):
  none reaches 0.
- lz[6]: phantom-slot producer census (r11/frame_census.md): 14 ordinary spellings of the lz[2] form,
  none gives frame 0x78 at zero cost.

## [s3 cont. 2] 2026-09-28 — owner Q30 (45ecac2fc); v3 = v1 code (c7)
- Owner ruled FAKE-construct spellings do not count against Ruling 11 necessity (set-aside test in
  Ruling 11 (D)). v3 lands the v1 code (seven reused locals, no FAKE construct for them) with v2 text fixes
  (Scr1F800000, header, lz[6] annotation + census): template.c = r11/variants_v3/c7.c, 0/662. proof.md v3
  banks every set-aside closer (dead stores for work/delta, six do-while(0) wraps for nforce) and the sweeps.

## [s3 cont. 3] 2026-09-28 — v4 layer-2 FAIL (ledger only) fixed
- Banked the landing-chassis dumps (r11/dumps_table_landing.txt, r11/lreg_excerpts_landing.txt), rewrote proof §3
  as mechanism + record under Q31, banked all 29 tmp-only reviewer probes (r11/reviewer_probes/, none counting
  reaches 0), corrected figures (2,071 FAKE-free per-value reviewer probes + 99 reuse bodies; 10 reuse zeros: 8 loop, 2 order).

## [s3 cont. 4] 2026-09-29 — layer-2 v5, v6, v7 FAILs (records only) fixed; author-side audit
- Three more fresh layer-2 reviews passed the bytes, the islands and the seven Ruling 11 locals and FAILed
  record text only (rejected/r11-lz6-frame-text-v5-layer2-fail-0.md, r11-lz6-measured-line-v6-layer2-fail-0.md,
  r11-frame-probe-record-v7-layer2-fail-0.md): the lz[6] 0x40 sum (24 bytes of the object, not 16), the
  "Measured:" line (lz[3]/lz[4] give 0x70), template.c stale, §5 frame-probe sentence, probes.md zero count,
  uncommitted ledger. Their probes are banked (r11/reviewer_probes/rv5_*, rv6_*, rv7_*); none counting reaches 0.
- An author-side audit then checked every figure against the banked files (proof v9 notes the corrections;
  r11/fake_scan.md; tmp-only evidence banked in r11/banked/).

# Evidence bank — func_8005C8A8

- [s1] Queue entry: distance=752, verdict=ASM-STRUCTURAL, rules=1, status=parked (2026-06-09 audit REJECTED auto-auth: standard GCC-compiled C). Single rule: asmfix.txt:78 replace_with_asmfile; src/text1b.c body is a stub. The regfix.txt:1751 mention of func_8005C8A8 is only a COMMENT on func_80016E60's rule (a caller-side path note), not a rule on this function.

- [s1] Structure (tmp/blitz/func_8005C8A8_map.txt): 753 insns @0x8005C8A8-0x8005D468, frame 0xB8, saves s0-s7+fp+ra. Single jr $ra. Returns constant 0x4F0 (sp70 set once at entry — prim-buffer bytes consumed). ONE mult, no div insns (all /2 spelled as sign-fixed shifts), no GTE, no jump tables (mode dispatch is an if/else ladder on arg0: 0, 1, 2, >2).

- [s1] Signature: s32 func_8005C8A8(s32 mode, u16 sel, void *primBuf, s32 otDepth). primBuf walks +0x10 per tile prim (var_s3 cursor); prim area at +0x4D8 gets the final texpage (sp68); text prim cursor var_s5 starts at primBuf+0xF0 and is threaded through every func_8007352C return.

- [s1] THE CENTRAL PATTERN: a draw-request param block on the stack (sp+0x18..0x43) rewritten field-subset-wise before EACH of the 11 func_8007352C calls. (s2: settled — it is the 0x2C-byte EnvA descriptor passed by address, `func_8007352C((s32)&s)`, the same as every completed text1b.c caller, e.g. func_8005D814.)

- [s1] Mode ladder, loops, layout tables: see the s1 recon notes in git history (commit of this file before 2026-09-29); superseded by the s2 draft below.

## s2 (2026-09-29, manual lane laneB) — full-body draft, 752 -> 42

Candidate: memory/grind/func_8005C8A8/candidate.c. Measured with `pwsh tmp/orch/sbx.ps1` (sandbox --disable all).
Floor trail: first full draft 319 -> 167 (arg1 addressable + split cell arrays) -> 119 (tile field order x0,y0,w,h; hdr split
at 0x8009B14C) -> 90 (return value spelled `end_off = arg2 + 0x4F0; size = end_off - arg2;`) -> 77 (declaration order
sel, y_base, mode_off, end_off, size) -> 75 (`top + 0x73 + i`) -> 59 (tail loop: `s.table = D_8009B2AC[j]` right after
`s.header`) -> 42 (`top = ...` computed before the colour stores).

Settled facts (each measured):
- Dispatch is `switch (mode)` with cases in source order 2 (falls through), 0 (break), 1 (break): GCC's 3-case
  decision tree (beq 1; slti 2; beqz 0; bne 2) is exactly the target's ladder. The `li s6,2` in three delay slots is
  reorg stealing the post-switch `for (j = 2; ...)` init.
- arg1 is an `s32` whose address is taken: the prologue `sw a1,0xBC(sp); lhu t0,0xBC(sp)` and the in-loop
  `lw v1,0xBC(sp)` (arg1 >> (j+16)) are reads of the parameter's own stack home. `sel = *(s16 *)&arg1;` reproduces
  both (narrow-stack-param-subword-offset family; SOTN sub-word param read).
- Return: the target keeps 0x4F0 in a spill slot (li t0,1264; sw t0,0x70). A literal `size = 0x4F0` is
  REG_EQUIV-constant (cse adds REG_EQUAL) and local-alloc substitutes it at the return (li v0,1264 at the end).
  `end_off = arg2 + 0x4F0; size = end_off - arg2;` folds in COMBINE (no REG_EQUAL -> no REG_EQUIV) and gives the
  target's spilled pseudo. Dump: tmp/func_8005C8A8/dump (f.cse insn 35 carries REG_EQUAL for the literal form).
- Spill-slot order = pseudo creation order: mode 0x48, ot 0x50, sel 0x58, y_base 0x60, mode_off 0x68, size 0x70 ->
  locals declared `s16 sel; u16 y_base; s32 mode_off; s32 end_off; s32 size;`. y_base is u16 (lhu + addu, no sext).
- Data: sprite headers 0x8009B0E0..0x8009B14B are ONE 12-byte-record array (target forms &D_8009B110 once and reaches
  -0x30/-0x24/-0x18/-0xC; &D_8009B0F8 reaches +0x48). D_8009B14C and D_8009B158 are separate. Cell tables are separate
  per sprite (D_8009B184 is NOT related to D_8009B20C: one big array made cse relate them, +136, which the target does
  not); D_8009B164 is [2][2] (&D_8009B164[1] = +0x10 related in target). D_8009B2BC is {s16 w, h}[3] (D_8009B2C4 =
  [2].w). D_8009B2AC is a 4-entry pointer table.
- Tile loops: field stores in source order x0, y0, w, h (loop.c giv base = last field, +0xE, as target).
- Tail loop: loop.c threshold T=19 in this function (bounded by the f.loop decisions: life 2 * 19 >= 38 moved, < 40
  not). The target hoists &D_8009B2AC into s4; ours only does when `s.table = D_8009B2AC[j]` directly follows
  `s.header` (life 3). That also restores `li s6,2` as the first insn after the switch, so reorg fills
  `j tail` and does NOT invert the case-0 loop branch (reorg.c:3923 reversal only fires on an unfilled jump).

- (42 -> 30) Icon loop: `s.table = &D_8009B23C[j * s.header->count];` (count read THROUGH the descriptor field just
  stored). cse folds (plus <reg known = sym> 2) to the constant address sym+2 without making a register for it, so the
  header's symbol pseudo is used once (life 1, not hoisted) and (s16)sel keeps fp. `D_8009B14C.count` instead made cse
  relate the two addresses (use_related_value: 154 = 146 - 2 in the i3 dump) and loop.c hoisted them.
- Case-2 first draw: `s.header`/`s.table` assigned before `y_base = 0x33; s.y = ...` (c1). Final pair: `s.y` assigned
  right after `s.x` (first draw) and right after `s.table` (second draw) (f1/cf2).

## At 30 (2026-09-29): every remaining scored hunk is a frame offset
target frame 0xB8, ours 0xB0: the target has one extra 8-byte spill slot at 0x78 that NO instruction references
(between size@0x70 and the hoisted ot*4@0x80; (s16)k@0x88). Reload slots are allocated in pseudo order for the
initially-unallocated pseudos (BB2_FRAME_DEBUG: p72,p75,p78,p79,p80,p82,p490,p537 in ours), so the target has one more
MEM pseudo X with 82 < X < 490 whose every reference vanished. Known mechanism that leaves such a hole:
reload1.c delete_output_reload "forget we had a stack slot" (a LOCAL pseudo, one death, completely replaced by
inherited reload regs) — the slot's frame space stays allocated. The only t0-computed-and-consumed value in the target
is `mflo t0; sll v0,t0,3` (the icon-loop product j*count), whose code is identical whether the product pseudo is in LO
(ours: global gives pseudo 150 LO, pref LO_REG else GR_REGS) or MEM (target?). Not yet reproduced; ruled out so far:
size spellings e1-e5 (tmp/func_8005C8A8/v), named product intermediate m1, pointer-arith m2, operand order m3.

Hole investigation (s2, 2026-09-29), all measured with tmp/orch/sbx.ps1 / tools/sandbox_sweep.ps1 on the committed
candidate (variants in tmp/func_8005C8A8/v/):
- cc1psx calibration (tmp/func_8005C8A8/psx.sh): the ORIGINAL compiler on this exact C also emits .frame 176
  (vars=112) - the missing slot is in the source, not a toolchain difference.
- A copy of mode (`m2 = mode;`, declared last, regno between size and the loop temps) that the FINAL dim tile reads
  reproduces the slot at exactly 0x78 and every other offset: score 2 (tmp/func_8005C8A8/v/nv2.c). The two remaining
  diffs are the copy's own `sw t0,0x78` and the final tile reading 0x78 instead of mode's 0x48. 15-variant matrix
  (copy placed at 5 points x 3 use-sets, mx_*.c): every variant whose final tile reads the copy scores 2; every
  variant where the final tile reads mode folds the copy away in cse (score 30). The copy is a no-op copy (refused
  construct), so nv2 is diagnostic only, NOT a landing form.
- Decomp-permuter (2 campaigns: tmp/perm_c8a8 from the candidate, 4.7k iterations, best 338 = the same mode copy;
  tmp/perm_c8a8_nv2 from nv2, 4.5k iterations, nothing below its base).
- Killed (frame stays 0xB0): inline helper set_pair(...) for the centred pairs (ih1-4, also +2..4 insns);
  Size5C8A8 *sz pointer (sz1-3, loses insns); param types s16/u16/u8 mode, s16 ot (pt1-4); extra `start` base
  variable for size (z1, z3), `size = end_off - (s32)tile` (z2), `size = arg2 + 0x4F0; size -= arg2` (z4);
  redundant k-diamond in the first border loop (hk1, +9 insns).

## s3 (2026-09-29, manual lane laneB, modality: compiler mechanism) — 30 -> 0 (Q27 (A) always-zero narrow local)

Base for every s3 probe: the s2 floor-30 body, kept as memory/grind/func_8005C8A8/v/s2_floor30.c. Probe tools
(tmp/func_8005C8A8/): fpmod.py/fp.py (splice into src/text1b.c, instrumented cc1 tools/gcc-2.7.2/cc1 with the
engine/buildconfig.py CC_FLAGS + BB2_FRAME_DEBUG, `.frame vars=` + FRAMEDBG slot list + asm diff with sp offsets
masked), sweep.py / gen.py / gen2.py / gen3.py / gen4.py / pairs.py / subsets.py / place.py (variant generators),
holes.py / holecause.py (in-file census). Receipts: probes/s3/ (regenerate with tmp/func_8005C8A8/receipts.sh).

What the hole is (settled). The target's 0x78 slot is a reload `alter_reg` first-pass slot: first-pass slots are
assigned in pseudo order, ours are p72 mode, p75 ot, p78 sel, p79 y_base, p80 mode_off, p82 size, p490, p537, and
p490/p537 are EXPAND pseudos (f.rtl max pseudo 619, loop.c adds 620-653; p490 = ot*4 of the first AddPrim in the
final double loop, p537 = the `(s16)j` extension tested by the second inner loop). So the target has one more
never-allocated expand pseudo created between `size` and that AddPrim, with no instruction referencing it.
Stack temps allocated at expand (assignment used as a value) are placed BEFORE the reload slots (FRAMEDBG
ctx=stack_temp at frame_offset 48, then spill_new_p72 at 56), so they would move mode off 0x48: that family
cannot give a hole at 0x78.

In-file census (probes/s3/text1b_untouched_slot_census.txt, text1b_orphan_families.txt): 34 C functions in
src/text1b.c list a frame slot no emitted instruction names (the census also counts address-taken struct temps,
false positives); of the 49 untouched reload slots classified, 41 are distribute_notes `(use (reg))` orphans and 8
are other. The orphans come in four kinds:
(1) HI load + sll/sra extension whose loaded value is used again (func_80070188 p454/p484, func_800620B8 ...);
(2) reg+reg address with the symbol in a register folded into the access (`(&Judge)[i]`, func_80048BA4 p91,
func_800493E4 p98, func_80057CC8 p171); (3) loop entry test `i = 0; i < n` with a variable bound
(compare-then-branch on a bound held in a register; e.g. func_80071C4C p315/p322, func_800720FC p577:
combine folds `i = 0; i < n` to `n <= 0` and keeps i's set); (4) always-zero narrow locals whose extension
combine folds (func_8006C21C p162/p164/p170/p172 = the Q27 (A) first application, dtd/xpos/ypos/tw).

Ordinary forms and the other orphan families, measured negative on the s2 body (condition 2 of Q27):
- 363 single-site semantics-preserving respellings (operand commutation, `&a[i]`/`a + i`/`(&a[i])->f`/`(a + i)->f`,
  `(*tile).f`, `(&s)->f`, s32 casts on table reads, `!= 0`/truthiness, `tile += 1`, pre-increment, `(s32)(...) / 2`,
  `<` vs `<=`, argument casts): frame stays vars=112 in every one (neg_respelling_catalog.txt).
- 139 targeted rewrites (sweeps r1-r5: `j * 15` forms, compare forms, bit-test forms, header/table index forms,
  ot*4 forms, colour chains, semi ternaries, size/mode_off/cur forms, size-table as s16[3][2] / s16[6] / incomplete
  array / scalar with `(&D)[mode]` or `(&D + mode)->`, `s.table[k].unk0` writes, incomplete extern arrays,
  assignment used as a value): no variant gives vars=120 with the target's code (neg_sweeps_r1-r5.txt; the only
  vars=120 hit, s16[6] indexing, changes 167 lines).
- Family (1): an s16/s32/u16 local for every D_8009B2BC[mode].w/.h read, centred x/y, and &D_8009B2BC[mode],
  over every window of 1-6 statements (193 variants, neg_hi_extension_locals.txt): the only vars=120 hits are s16
  w/h locals that keep the loaded value in a register where the target reloads it (8-38 changed lines; the final-
  tile ones also put the new slot after the ot*4 pseudo).
- Family (2): the size table spelled through pointer arithmetic (scalar `(&D)[mode]`, `(&D + mode)->`, per-site
  `(D + mode)->f`, `(&D[mode])->f`) never yields the slot; the whole-table forms change 143 lines.
- Family (3): every loop here has constant bounds, so cse folds the entry test; there is no variable-bound loop in
  the target to spell.
- s2 (above): named product intermediate, pointer/operand-order product forms, size pointer, inline helpers, param
  types, `start` base variable, two permuter campaigns.

Q27 (A) reproduces the hole exactly (condition 1). One `s16 xpos`, written `xpos = 0;` once at entry and read as
`s.x = xpos;` at case 0's first draw (the site after the case-0 label): vars=120 (frame 0xB8), code identical
(asm diff 0 with sp offsets masked), slot list (first s3 body) p72,p75,p78,p79,p80,p82,p226,p493,p540; on the s3b landed body p72,p75,p78,p79,p80,p81,
p226,p493,p540 (end_off gone) — the new p226 lands between
size and the ot*4 pseudo, i.e. exactly the target's 0x78 (probes/s3/landed_frame.txt). Mechanism, dumped on the
landed body (probes/s3/landed_orphan.txt, regenerated on the s3b body): xpos is reg/v:HI 87 (88 on the first s3
body); case 0's `s.x = xpos` expands to
p226 = (ashift (subreg xpos) 16), p225 = (ashiftrt p226 16); combine folds the extension of the once-set-to-0
pseudo and distribute_notes plants `(use (reg:SI 226))` + REG_DEAD at code_label 566 (567 on the first s3 body; the case-0 label); p226 is
never allocated and reload gives it the first-pass slot. Sandbox (tmp/orch/sbx.ps1, --disable all): score 0,
0 source-level, 0 operand-only hunks (58 not-scored relocation/branch-target artifacts).

Site matrix (which reads give the slot): single reads (q27a_single_sites.txt, s16/u16/s8/u8 x 25 literal-0 sites of
s.x/s.y/s.semi/s.has_color): with s16 or s8, seven writes after the case-0 label each give vars=120 with 0 changed
lines (s.x at case 0 first draw, case 0 second draw, case 1, the tail loop; s.semi at case 0's `sel == 0` arm,
case 0 second draw, case 1); u16/u8 never do (zero extension, no fold); the case-2 reads are folded by cse (no slot);
the final-section s.x reads cost 2-7 lines. Reading xpos at every s.x write: vars=136, 127 changed lines; all s.x + s.y: vars=144
(q27a_combos.txt). Pairs (q27a_pairs.txt) and subsets (q27a_subsets.txt): at most one read after the case-0 label
can be taken; two such reads either add a second slot (vars=128) or cost 92-136 lines. The landed body takes the
first such site. Declaration/initialisation placement is neutral (q27a_placement.txt, 24/24 placements identical).

## s3b (2026-09-29, laneB) — layer-2 FAIL round 1 fixes (bytes unchanged: SHA1 == oracle, sandbox 0)

Layer-2 round 1 (orchestrator relay, 2026-09-29) passed xpos (every Q27 (A) prong), the case 2->0 fallthrough,
`*(s16 *)&arg1`, `j * s.header->count` and `D_8009B2BC[2]`, and FAILed two things; both are fixed here.

1. Aggregate merges were TU-local. The five merges now sit in include/game.h after D_8009B490, each with its
   object-model evidence (base+offset / one-stride addressing in asm/funcs/func_8005C8A8.s):
   - Unk8009B0E0Record D_8009B0E0[9] (merges D_8009B0F8 / D_8009B110 / D_8009B11C);
   - D_8009B14C (merges its count byte D_8009B14E);
   - D_8009B164[2][2] (D_8009B16C / D_8009B17C);
   - D_8009B184[2] (D_8009B18C);
   - Unk8009B2BCRecord D_8009B2BC[3] (D_8009B2BE / D_8009B2C4).
   D_8009B158 (a single header, no merge) moved with its siblings. The retired labels' undefined_syms_auto.txt rows
   (D_8009B14E, D_8009B2BE, D_8009B16C, D_8009B17C, D_8009B18C, D_8009B2C4) are removed: after the landing only the
   no-longer-assembled asm/funcs/func_8005C8A8.s names them. The dlabels stay in asm/data/7D920.data.s. No other C
   consumer of any merged byte exists (grep of src/ and include/).
   Naming, for a tools/naming_wave.py reset, not hand-edited: named_syms.txt `g_cpu_no_action_table_24x3 = 0x8009B0E0`
   ("3 x 24-byte records") and `g_cpu_no_action_table_16x3 = 0x8009B1AC` ("3 x 16-byte records") contradict the
   evidenced 9 x 12-byte sheet headers at 0x8009B0E0 and the 2 x 8-byte cell records at 0x8009B1AC (and D_8009B1BC
   right after). No code names either symbol.
2. `end_off = arg2 + 0x4F0; size = end_off - arg2;` was a round trip that is not in the target's bytes (the target
   only has `li t0,0x4F0; sw t0,0x70(sp)` at entry and `lw v0,0x70(sp)` at the return). It is now claimed under the
   named-local-fake-exception constant-holder family: `size` holds the constant 0x4F0 in a frame slot across every
   call instead of being rematerialized. The spelling is one statement, `size = (s32)tile + 0x4F0 - arg2;` (the
   buffer's end minus its start), with no second local. The `/* FAKE: */` annotation is at size's declaration.
   Mechanism (probes/s3b/size_dumps.txt, dumps of the landed body and of the literal form with command lines):
   - landed: f.cse insn 40 is `(minus (reg/v 76) (reg 89))` with no note; combine folds it to `(const_int 1264)`
     with no REG_EQUAL; local-alloc leaves it; p81 is spilled (slot 0x70) and the return loads it: `li $8,0x4f0`
     at entry, `lw $2,112($sp)`, frame 184 (vars 120).
   - literal `size = 0x4F0;`: f.cse insn 38 carries REG_EQUAL 1264, which local-alloc.c update_equiv_regs turns
     into REG_EQUIV (1024-1032). Since p81 is read once (reg_n_refs == 2, 1078-1081), it rewrites the return to
     `(set v0 (const_int 1264))` and deletes the set (1103-1110). Result: no slot, frame 176 (vars 112).
   Literal-form failure and the alternatives (probes/s3b/engine_sandbox_scores.txt; engine sandbox --disable all,
   variant files alongside):
   - literal 33 (2 source-level + 11 operand-only hunks);
   - `(arg2 + 0x4F0) - arg2` 33 (cse folds it: the same REG_EQUAL path);
   - `mode_off + 0x18 - arg2` 30 (a second orphan slot shifts every offset);
   - `arg2 + 0x4F0 - (s32)tile` 0;
   - the old end_off form 0.
   Boundaries: size is s32, not a narrow always-zero local (Q27 (A) does not apply). It is read once, as the
   return value, never as an array subscript or pointer offset, so Q22's dummy-local refusal does not reach it.
   The spelling's only cast is `(s32)tile`, a pointer converted to an integer for arithmetic, not a memory access.
Verification on the spliced src (lock held): `tmp/orch/lock.ps1 rebuild laneB` build_sha1
62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches true; `sandbox func_8005C8A8 --disable all --diff` 0,
0 source-level / 0 operand-only (39 not-scored); engine test 862 passed. probes/s3/landed_frame.txt and
landed_orphan.txt are regenerated on this body.

## s3c (2026-09-29, laneB) — layer-2 round 2 FAIL on `size`; BANKED (admissible floor 33 = literal size, with fix1 applied)

Layer-2 round 2 PASSED Fix 1 (the five merges in include/game.h, the six retired undefined_syms_auto.txt rows). It
FAILED `size = (s32)tile + 0x4F0 - arg2;`. The line comes right after `tile = (Tile5C8A8 *)arg2;`, so the value is
always 0x4F0: it hides a constant by cancelling a value against a second name for it (the extra-handle device the
named-intermediate entry forbids). The constant-holder family admits only the canonical `s32 size = 0x4F0;`
(sandbox 33, probes/s3b/engine_sandbox_scores.txt). The body is kept as rejected/size-tile-cancel-0.c (sandbox 0,
SHA1 == oracle with fix1 applied).

Bounded (a) attempt, all the literal form in effect, frame vars 112, 3 changed lines each
(tmp/func_8005C8A8/r8.py on the s3b body): `register s32 size`; `u32 size`; the set moved after `sel`/`top`.
update_equiv_regs keys only on reg_n_sets == 1 plus a cse REG_EQUAL constant (local-alloc.c 1024-1032), so any
once-set literal is rematerialized. No end-of-buffer value or 0x4F0 advance exists in the target to derive it from:
the only +0x4D8/+0xF0 pointers are mode_off and cur, and `mode_off + 0x18 - arg2` / `cur + 0x400 - arg2` both cancel
arg2 against itself and add a second orphan slot anyway (30).

State banked:
- candidate.c is the admissible literal-size body. It REQUIRES fix1-merges.patch (include/game.h + the
  undefined_syms_auto.txt rows) applied, and scores 33 with it.
- fix1-merges.patch is the layer-2-PASSED Fix 1, re-apply with `git apply`.
- rejected/size-tile-cancel-0.c is the 0-scoring cancellation form.
- Owner question: docs/grind/borderline.md 2026-09-29 func_8005C8A8 policy-question. Tree reverted; lock.ps1 rebuild
  laneB == oracle.

## s3d (2026-09-30) — owner ruling Q45: option B, cancellation spelling REFUSED

Owner, twenty-third batch Q45 (docs/grind/owner-rulings-2026-09-26.md, d023686ea): "Don't allow it (option B).
`size = (s32)tile + 0x4F0 - arg2`, where tile was just set to arg2, is a fancy way of writing 0x4F0. It exists only
to hide the constant from the compiler. [...] The function stays unfinished, 33 instructions off, and keeps being
worked for another spelling." Resolution line: docs/grind/borderline.md 2026-09-29 func_8005C8A8 entry.

Consequence for the next session: rejected/size-tile-cancel-0.c is permanently out, along with every variant that
cancels a live value against a second name for it (`mode_off + 0x18 - arg2`, `cur + 0x400 - arg2`, and the like).
Do not resubmit one. The open problem is unchanged: keep the 0x70($sp) slot for a once-set 0x4F0 without
update_equiv_regs rematerializing it (local-alloc.c 1024-1032), in a form the target's own dataflow supports.
Admissible floor stays 33 (candidate.c + fix1-merges.patch). Status: INCLUDE_ASM/active, no rotation.

## s4 (2026-09-30, laneC) — the `size` slot: what the target's dataflow requires; SOTN precedent search negative

Floor unchanged: 33 (candidate.c + fix1-merges.patch). No src change this session. Receipts: probes/s4/scores.txt
(volatile / s16 / one-member struct / one-element array / `return 0x4F0;` all measured, none keeps the slot).

What the 0x70 slot proves about the original source (tools/gcc-2.7.2, read on this session):
1. A pseudo gets a reload stack slot only when it has no hard reg AND no equivalence:
   reload1.c:2381-2385 (`reg_equiv_constant[i] == 0 && reg_equiv_memory_loc[i] == 0`).
   reg_equiv_constant is filled from any REG_EQUIV constant note (reload1.c:567-586).
2. local-alloc.c update_equiv_regs turns a REG_EQUAL constant into REG_EQUIV for every
   pseudo with reg_n_sets == 1 (1019-1032), whatever its number of uses (the n_refs == 2
   test at 1079 only decides the extra init-deletion seen for the literal form).
3. cse.c:6918-6934 adds that REG_EQUAL note to every single-SET insn whose source cse
   knows to be a constant (src_const), including a plain literal (probes/s3b/size_dumps.txt:
   `size = 0x4F0` gets REG_EQUAL 1264 at f.cse insn 38).
4. So the target's size pseudo had either (a) reg_n_sets >= 2 -- excluded: a spilled
   pseudo stores to its slot at every set, and the target has exactly one store to
   0x70($sp) (asm/funcs/func_8005C8A8.s:28) -- or (b) a set whose constant value cse could
   not know and only combine later folded. Every (b) form measured on this function is a
   cancellation of a start pointer against a second name for it (s3b/s3c), which owner
   ruling Q45 refused. A self-cancellation without a second name (`(arg2 + 0x4F0) - arg2`,
   probes/s3b/paren.c) is folded before the note is written (33).
   Also excluded here: a `volatile` or any other memory-resident local (its slot is
   allocated at expand, before the reload slots, i.e. in the sp+0x44 alignment gap after
   `s`, not at 0x70 between mode_off's 0x68 and xpos's 0x78); `u16`/`s16` size (the target
   loads it with `lw`); a one-element array or one-member struct (SImode set, same note).

SOTN precedent search (Q50/Q55 route, tmp/sotn-decomp @aa53500, PS1-build files only;
Explore agent, very thorough): no matched PS1 C computes a byte count as a buffer end
pointer minus the start it was derived from, or cancels a variable against a copy of
itself to leave a constant. Closest hits, none the same construct when read:
- src/dra/7879C.c:3040-3043 (EntityPlayerOutline; also 7879C.c:2466-2471, 7E4BC.c:1250-1255,
  ric/pl_blueprints.c:1795-1800, bo4/rbo5 copies): `four = 4; spriteX = four + p[0];
  width = spriteX - four;` -- u8 truncation, a data-dependent value, not a constant.
- src/st/e_skelerang.h:244-245 `(Random() & 3) + 1 - 1` (comment: "required to align PSP")
  and src/dra/7E4BC.c:298 `selfYPos - 1 + 1 - (rand() & 0x1F)`: constant pairs cancelled
  around a variable expression; the value stays variable. They fold nothing into a
  constant and carry no second name for a live value.
- src/dra/4AEA4.c:239-240 DecompressData `return g_DecDstPtr - dst + 0x2000;`: a real
  end-minus-start difference, computed at the return from a pointer advanced in a
  callee -- data-dependent, not a constant computed at entry.
None of these is a citation for keeping a constant out of cse by cancelling a live value.

Frontier (unchanged in substance): a once-set 0x4F0 whose source cse cannot evaluate,
without the Q45-refused cancellation. The tree-level evidence above leaves no ordinary
C form in the families measured; the item stays active (no rotation).

## s5 (2026-10-01, laneA) — re-baseline 33; the two remaining mechanisms measured; owner question filed

Receipts: probes/s5/scores.txt (+ the variant files, setup.py / sbx.py / dump.sh harness). Re-baseline on the
current tree (function back in src/text1b.c since 5c543ce1d): candidate.c + the fix1 game.h hunk = 33, as banked.

The slot needs a size pseudo with no REG_EQUIV constant (s4). Two ways a once-stored value escapes it:
1. A set whose constant cse cannot see and combine folds. Every such source is the constant computed by
   cancellation (`(s32)tile + 0x4F0 - arg2`, `end_off - arg2` at entry = probes/s5/e2.c, 0) or a bit trick;
   Q45 refused the class ("a fancy way of writing 0x4F0").
2. reg_n_sets >= 2 with one emitted store. (a) A running offset accumulator (a1/a2/a3, a real
   buffer-layout reading: 0xF0 tiles, +0x3E8 sprites, +0x18 modes) does not do it: cse substitutes each
   intermediate constant into its user, the earlier sets go dead, flow deletes them before counting (a2 dump:
   f.flow `(note 35 ... NOTE_INSN_DELETED)`, "Register 81 used 2 times"), 33. (b) A union constructor's
   `(clobber (reg))` counts as a set (tools/gcc-2.7.2/flow.c:1930 -> 2079; expr.c:2996) and leaves REG_EQUAL un-promoted: u2/u4 keep
   the slot (frame 184), 4 off on placement only. A one-member union wrapper has no semantic purpose and is in
   no frozen-list family: inadmissible, banked as mechanism evidence only.
Sibling evidence (new since Q45): eight finished functions in src/text1b.c (func_8005D814, func_8005E098,
func_8005E54C, func_8005F1C8, func_8005FC9C, func_800600C8, func_80060414, func_80060768) set
`end_off = <buffer start> + K` at entry and `return end_off - <start>;`, and every one of their targets
computes it at the return (`subu $v0, ...` before `jr $ra`). The sibling-faithful spelling here
(probes/s5/e1.c) scores 110: func_8005C8A8's original did the subtraction at entry, not at the return, so the
idiom supports an end-pointer local but not by itself the entry-time subtraction.
Status: every admissible spelling found is at 33; the closing forms (e2 / the tile cancellation) need an owner
ruling. Question: docs/grind/borderline.md 2026-10-01 func_8005C8A8. Item stays active, no rotation.

## s5b (2026-10-01, laneA) — the deciding pass, named from dumps; typed end-of-object forms

Dumps (probes/s5/dump.sh: the build cc1 with -da on the spliced TU, fix1 game.h; function-only sections):
- 33 body (candidate.c): f.rtl insn 38 `(set (reg/v:SI 81) (const_int 1264))` (literal from expand);
  f.cse insn 38 gains `REG_EQUAL (const_int 1264)` (cse.c:6918-6934); f.combine unchanged; f.lreg insn 38 is
  `NOTE_INSN_DELETED` and the return insn 1911 is `(set (reg/i:SI 2 v0) (const_int 1264))`: local-alloc.c
  update_equiv_regs (1019-1032 REG_EQUAL -> REG_EQUIV for reg_n_sets == 1; one use -> substituted into the
  use and the init deleted) removes the pseudo; f.greg/func.s `li $2,0x4f0` at the return, frame 176.
- 0 body (probes/s5/e2.c): f.rtl/f.cse/f.cse2 insn 41 `(set (reg/v:SI 82) (minus (reg/v 81) (reg/v 76)))`
  with r81 = r76 + 1264 and NO note (cse does not simplify (minus (plus a c) a) through the two pseudos);
  f.combine insn 38 deleted, insn 41 `(set (reg/v:SI 82) (const_int 1264))` with no REG_EQUAL (combine's
  substitution + simplify); update_equiv_regs finds no note, r82 stays a global pseudo, gets no hard reg and
  reload gives it the 0x70 slot: func.s `li $8,0x4f0; sw $8,112($sp)` at entry, `lw $2,112($sp)` at the
  return, frame 184. The deciding pass is CSE: a source form whose constant cse can see loses the slot.
- t2 (typed end-of-object pointer over an s32 address, `(u8 *)((Buf5C8A8 *)arg2 + 1) - (u8 *)arg2`): the
  int->pointer casts keep fold-const from seeing the same operand, the tree becomes arg2 - (arg2 - 0x4F0),
  and the RTL goes the e2 way (cse no note, combine folds): 0. With arg2 typed `Buf5C8A8 *` (p1/p2/p3) the
  same subtraction folds at tree level and scores 33. probes/s5/scores.txt.
What object is 0x4F0: the prim chunk this call fills from the prim-pool cursor D_800A38B4 (callers advance it
by the return: src/code6cac_c2.c:748, src/code6cac_tu2.c:2196); laid out as 15 TILEs (0xF0), the sprite area
func_8007352C writes (0x3E8), 0x18 of draw-mode space (one DR_MODE used at +0x4D8). No such type, header field
or caller-side size exists in the code or data; callers pass the bare cursor. sizeof of a declared layout is a
literal (t1, 33). So every 0-form found computes the size as end minus start of the same address: t2 has no
second NAME (Q45's literal boundary) but is still a cancellation that works only because the s32 parameter
hides the identity from fold. Not landed; added to the borderline.md 2026-10-01 entry as option (C).
Permuter not run this round: every byte-exact route found needs a cse-invisible constant (above), which a
permuter mutation of the 33 body cannot introduce except as such a cancellation; two earlier campaigns
(s2, 9.2k iterations) found nothing below their bases.

## s6 (2026-10-01, laneC) — Q77 refused A/C; re-baseline 33; the slot's mechanism space closed at pass level

Owner Q77 (rules commit 30a3e2d2d, `.claude/rules/ordinary-c-judge-decidable.md` § Owner rulings on constant
spellings): Q45 stands for the sibling end-pointer form (e2) and the layout-struct end-minus-start form (t2).
Re-baseline on main eb1c75fca: candidate.c + fix1's game.h hunk (tmp harness = probes/s5 setup.py/sbx.py) = 33
(2 source-level, 11 operand-only); `git apply --check fix1-merges.patch` clean; no new src/ or include/ consumer of
any merged symbol (grep of src/, include/, *.txt). Receipts: probes/s6/ (scores.txt, variant files, psx.sh/run.sh).

New this session:
- cc1psx calibration on the literal body (probes/s6/psx_literal.txt): the ORIGINAL compiler also emits frame 176
  and `li $2,0x4f0` at the return. So the original source was not a once-set literal either; whatever it wrote
  kept 0x4F0 out of cse. No fidelity lead (rotation Ruling 2 check done by hand).
- No-op self-copy (`size = size;` after the set, n1, or before the return, n2): 33 both. The copy is gone before
  flow counts sets, so reg_n_sets stays 1 (and it would be a refused no-op copy anyway).
- `const s32 size = 0x4F0;` (c1): 33 (decl_constant_value folds the read to the literal at tree level).

Why no admissible spelling exists (tools/gcc-2.7.2, read this session; extends s4/s5b):
1. The slot needs a size pseudo with no REG_EQUIV constant: reload1.c:563-586 (equivalence scan) and 2381-2385
   (alter_reg allocates a slot only without one). update_equiv_regs promotes any REG_EQUAL constant when
   reg_n_sets == 1 (local-alloc.c:1019-1032, no other condition).
2. cse.c:6918-6934 writes REG_EQUAL on every single-SET insn into a REG whose source cse evaluates to a constant,
   and the literal itself is such a source. cse2 runs after loop, so every set whose RTL source IS 0x4F0 before
   flow carries the note. The target's `li 0x4F0` must therefore be created by COMBINE (the only later pass
   that rewrites set sources; flow and local-alloc do not), and combine only gets it by algebraic
   simplification of non-constant operands that are all set in the entry block, where cse already knows every
   constant. Such an expression is an identity equal to 0x4F0 for every input: a cancellation (`a + K - a`,
   Q45/Q77) or a bit/zero trick (`x & 0`, `x ^ x`, `(ior x K)` with nonzero_bits(x) inside K,
   combine.c simplify_logical). Q45's reasoning ("a fancy way of writing 0x4F0 ... only to hide the constant")
   covers every member.
3. The other route is reg_n_sets >= 2 with one emitted store. Sets that reach flow but emit no store after
   reload are: a dead set (flow deletes it before counting, flow.c:1490), a set combine merges away (combine.c
   2305-2312 decrements reg_n_sets), a no-op copy (removed before flow, n1/n2), a CLOBBER (counted; emits
   nothing) and a REG_UNUSED side output of a multi-SET insn. CLOBBERs of a user pseudo come only from
   union/struct constructors, multi-word moves and BLKmode returns (u2/u4 union 4, s64 88, u3 struct 33);
   multi-SET insns here would need a div/mod pair (the target has none). None has a semantic reading for a
   byte count. A real second write would emit a second store (the target has exactly one,
   asm/funcs/func_8005C8A8.s:28) unless jump2 cross-jumps it into the first, which needs a join before the
   entry store (there is none: the first branch is at 0x8005C948).
So, under Q45/Q77 and the no-no-op / no-purpose-wrapper rules, no spelling of this function keeps the 0x70
slot; the admissible floor is 33 and it is a true floor for the current rulings, not an unexplored gap.
The item stays INCLUDE_ASM/active (no rotation). Re-open trigger: a ruling change, or new evidence that the
original wrote a different construct (e.g. a PsyQ/Square header macro computing the size).

## s7 (2026-10-02, oct2-b2) — Q91 clause D re-judgment; 33 -> 0 with ONE FAKE (xpos retired)

Q91 (`.claude/rules/completion-bar.md`, item 3 + §Application clause D) re-opens Q45/Q77: a codegen-only
construct with a measured `/* FAKE */` label is admissible unless the wall or item 3's refused list (fabricated
calls/side effects, cross-symbol address derivation, volatile outside its catalog) covers it. A cancellation
spelling of the size is none of those. Item 5 then wants the fewest-FAKE byte-exact form. Receipts: probes/s7/.
- `size = mode_off + 0x18 - arg2;` (the draw-mode area ends 0x18 past mode_off; the same 0x18 tail as the
  completed siblings' end_off minus draw-mode offset in src/text1b.c func_80060414 (`dist_off = new_var +
  0x14; end_off = new_var + 0x2C;`) and func_80060544 (`arg0 + 0x5DC` / `arg0 + 0x5F4`))
  scores 0 WITHOUT the Q27 (A) xpos local: one construct gives both slots. Dump (probes/s7/scores.txt): the tree
  becomes mode_off - (arg2 - 0x18); cse writes no note; combine folds size to 1264 with no REG_EQUAL and deletes
  the temp's set, leaving r88's stale count, so size spills to 0x70 and r88 takes 0x78 (frame 184).
- With xpos kept, the same line scores 30 (one slot too many); the s3b tile form without xpos scores 30 (0x78
  missing). The tile form needs two FAKEs (size + xpos), the mode_off form one: item 5 picks mode_off.
- `cur + 0x400 - arg2` also scores 0 without xpos; 0x400 names nothing in the layout, so not chosen.
- `sel = *(s16 *)&arg1;` is load-bearing ((s16)arg1 / plain arg1: 58); now carries a FAKE label.
- AddPrim's OT argument spelled `(s32)g_gpu_ot_ptr + ot * 4` like the completed siblings (g_gpu_ot_ptr is
  `u8 *` in text1b.c): byte-neutral (0).
- Literal size on this body: 33, frame 168 (both slots gone).
candidate.c = v2 (sandbox 0 with fix1's game.h hunk).

## LANDED (2026-10-02, oct2-b2) — COMPLETED-C; ledger closed
Layer-2 round 3 (Q91 rubric): cheat-reviewer rv2-5C8A8-1 PASS, scope match, body_hash a2903df592e9960a, no
required fixes (both FAKEs labelled, measured and load-bearing; nothing false; merged tables via game.h;
fewest-FAKE known form). Recorded in layer2.jsonl. Landed body == candidate.c (s7 v2) in Match commit ddeda2901
(src/text1b.c + include/game.h fix1 hunk + six undefined_syms_auto.txt rows); queue done e3f1bb1ed. Oracle SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa; check_completion_integrity OK. Hygiene debt is listed in the Match
commit body (shared TILE/descriptor types, libgpu extern prototypes, asm/text1b.s labels, named_syms misnomers).

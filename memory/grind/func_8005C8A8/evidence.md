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
(asm diff 0 with sp offsets masked), slot list p72,p75,p78,p79,p80,p82,p226,p493,p540 — the new p226 lands between
size and the ot*4 pseudo, i.e. exactly the target's 0x78 (probes/s3/landed_frame.txt). Mechanism, dumped on the
landed body (probes/s3/landed_orphan.txt): xpos is reg/v:HI 88; case 0's `s.x = xpos` expands to
p226 = (ashift (subreg xpos) 16), p225 = (ashiftrt p226 16); combine folds the extension of the once-set-to-0
pseudo and distribute_notes plants `(use (reg:SI 226))` + REG_DEAD at code_label 567 (the case-0 label); p226 is
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

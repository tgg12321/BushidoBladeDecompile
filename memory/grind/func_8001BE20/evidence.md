# func_8001BE20 — evidence (manual s1, 2026-09-25)

## What the function is
Per-player pad fetch (called for player 0 and 1 by func_8001E878 into a local
0x18-byte PadState). If the player's practice record (g_practice_menu_table
[arg0].unk_06) is set, input comes from func_80055B60 instead. Otherwise it
copies the global pad record D_80102788 to the caller's buffer, extracts this
player's u16 half of held/pressed/released/unheld, remaps face-button bits
0x10/0x20/0x40/0x80 onto 0x20/0x40/0x80 by the per-player colour-config nibbles
(D_80106A70[0..2] >> arg0*4, or D_800A3912..14 in mode 6 for the non-local
player), folds 0x200 -> 0x8 and 0x400 -> 0x20, clears the words in mode 5 when
any of six busy flags is set, and for player 1 (outside mode 6) swaps bits
15<->13 and 14<->12. It also stores into the OTHER player's record +0x34E a
flag = mode 2 && !D_800A389A && arg0 == 0 && bit 8 of pressed.

## Data model (aggregate merge, prongs (a)-(e))
- 0x80102788 is a 0x18-byte record: this function block-copies it
  (lw/lw/lw sw/sw/sw x2 from `addiu a1, %lo(D_80102788)`, i.e. base+offset
  addressing in the original binary); func_800194F4 initialises all six fields;
  func_80019568 writes unk_00[0..3] pairwise and held/pressed/released/unheld as
  new, new&~old, ~new&old, ~new; func_8001BE08 clears the four words of a
  caller buffer of the same shape. Declared once in include/code6cac.h as
  `PadState D_80102788` (u32 words: every consumer bit-tests them and this
  function needs logical shifts; srl vs sra measured, s32 fields = 8 source-level
  hunks).
- D_80101ECE / D_80102216 are +0x6 / +0x34E of g_practice_menu_table record 0
  (indexed by arg0*0x44C and (arg0==0)*0x44C in this function): added as
  PracticeMenuRec.unk_06 / unk_34E.
- LANDED as byte-neutral prep, commit 91448d225 (2026-09-25): header merge,
  sibling-TU conversions, func_8001BE08 retype, undefined_syms rows, and
  func_8001BE20.s respelled base+offset. The chassis is now main; candidate
  bodies splice straight in.

## Floors (chassis = main since 91448d225)
- Stub: 393/393.
- rejected/shift-two-roles-ruling5-0.c: **0/393**, full-build SHA1 == oracle
  62efab4f73f992798c43e8c730aa43baa10bb4fa with the body spliced.
- candidate.c (landable if the reuse is refused): **5/393** = one missing
  `move a3,a2` + the four srlv using a2 instead of a3. Everything else matches.
- Without the chassis (clean main) the same bodies read +14..+16 higher: the
  target .s spells D_80102790..9C / D_80101ECE / D_80102216 / D_80106A71..72 as
  separate symbols, so every struct-member access differs by relocation addend
  only (scorer artifact).

## Load-bearing constructs (each measured)
1. Single-expression store `g_practice_menu_table[arg0 == 0].unk_34E = a && b &&
   c && ((pressed >> 8) & 1);` — expand_assignment expands the LHS offset first,
   giving the target's early sltiu/negu/andi 0x44C. Separate `flag` + if: 33->45;
   ternary rhs: 44 (old chassis, v2-v4). Combine narrows `(pressed >> 8) & 1`
   to the target's `lbu +0xD`.
2. Index loop `for (i = 0; i < 4; i++) buf[i]` (biv eliminated -> pointer +
   signed `slt` against base+16, as target). Pointer loop `p < &buf[4]` gives
   sltu + hoisted constant (v1 57 vs v2 45).
3. One block-scoped `u8 r/g/b` per colour test (each written once). One shared
   `col` written 6 times: 6-arm register swap (v7 28 vs v13 16); arm-scope
   `u8 r, g, b;` declarations: 43 (v17); direct global reads: 73-79 (v10/v11).
4. **`shift` reused**: `shift = arg0 * 16;` for the four pad extractions, then
   `if (D_800A38DC == 6) shift = 0; else shift = arg0 * 4;` for the nibble
   selects. Mechanism: cse.c make_regs_eqv makes the copy `shift <- P(arg0<<4)`
   (P is the arg0*0x44C multiply's intermediate) canonical only if `shift` is
   mentioned beyond the CSE block; the reuse is the only mention that does it,
   so the copy survives as `move a3,a2`, and the pseudo, live into the loop, is
   seated in a3 by global alloc. Separate `half`: no move (5). Ternary for the
   nibble write: 20 (L5); with separate half + ternary: 8 (L4).
   A dead defensive init `s32 half = 0;` (L3) proves the mechanism (brings the
   move back) but the pseudo is then block-local and gets a1: still 5.
   **Admissibility: under ordinary-c-judge-decidable Ruling 5 this reuse fails
   prong 1(a) (the writes feed different consumers: pad words vs colour
   nibbles) and 1(e) (`shift = 0` is a constant write).** It is the author's
   reading that no current ruling admits it. **Layer-2 FAIL 2026-09-25**
   (orchestrator-spawned): Ruling 5 1(a)/(b)/(c)/(e) (different consumers and
   templates, four readers on the first write, a literal-0 write), 1(f)
   (generic role name) and Ruling 6 (sequential, not exclusive writes); the
   CSE effect is the allocator-only effect Ruling 5 excludes. Every other
   construct in the body was ruled SOUND (merge, one-expression flag store,
   block-scoped r/g/b, duplicated `& ~0xF0`).

## cc1psx self-disproof
candidate (honest, v15 shape): ours 5, cc1psx 33 -> SOURCE-SIDE (not a
fidelity lead).

## Manual s4 (2026-09-28): Ruling 11 submission — 0/393, oracle match
Picked from the rotated tail: the 2026-09-25 rotation reason ("two-role shift form inadmissible")
predates Ruling 11 (owner 2026-09-26, reused local admitted on allocator-necessity proof).
- Chassis drift since s3: 0x80106A70..72 became `D_80106A50.color[3]` (FileRecord merge,
  65897593b); bodies respelled `D_80106A50.color[k]`. Re-measured on the current (stock + crash
  fix) compiler: reuse form 0/393, one-var-per-value form 5/393 (unchanged).
- Proof package: r11/proof.md (every prong A-H), r11/dumps.txt (f.cse / f.lreg / f.greg /
  ALLOCDBG / FINDREGDBG for both spellings; stock and instrumented cc1 emit identical asm),
  r11/*.c both spellings. Variable renamed `temp` (R11 (E)(i)); codegen name-independent.
- Fresh permuter from the one-var body on the current compiler (tmp/perm_1be20_s4, 9,412 iters):
  3 finds, all invalid (read `half` before its write).
- Landing: body spliced + D_80106A70 alias row retired; verify-oracle --rebuild --allow-dirty
  == 62efab4f73f992798c43e8c730aa43baa10bb4fa.
- Layer-2 (fresh cheat-reviewer, 2026-09-28): **PASS** on every Ruling 11 prong (A)-(H) and on
  ordinary review of the rest; D_80106A70 row removal verified (no linked referrer). Noted
  weakness: the s4 permuter ran ~6 min, judged adequate together with s2's ~16 min campaign.
- LANDED f7fe651e9 `Match: func_8001BE20 — COMPLETED-C (manual)`; `queue done` OK;
  check_completion_integrity OK.

# func_800290B8 — evidence

## s1 (manual, 2026-09-28, parallel lane beside the func_8006C21C session)

Picked from the rotated tail: rotated 2026-09-21 with no ledger ("28-branch nested collision/
audio search ... selecting the next C candidate with only five branches") — i.e. never worked.
`canonical`: verdict C, hand_coded_tier LOW, 231 insns.

What the function is (read from asm/funcs/func_800290B8.s): `tbl` is 2 × 4 LeafPos points
(caller func_80029454 passes idx 0 or 1). It copies point idx*4 into the scratchpad record at
0x1F8002B8 as both max (+0x84) and min (+0x78), folds points 1..3 in (x/z min+max, y max
only), then walks the 16-byte entry list from func_8004678C (s16 type, s16 used, s32 x/y/z;
func_8002906C clears `used`) for unused in-bounds entries, puts x/z at +0x100 and tests the
two triangles (0,1,2) / (1,2,3) with func_8002E6B0. See the function comment in candidate.c.

Score trail (`sandbox --disable all`, candidate):
- 71/231 — first transcription (inline `(i / 2) * 2 + (i & 1)`, hit block after the loop).
- 52 — `i / 2` and `i & 1` written into the list-index / triangle-number variables (the target
  holds them in t0 and s0, the registers of those two variables), `rec->y > max.y` order.
- 5 — hit block placed at the outer loop's tail (`if (flag == 0 || idx == 0) continue; hit:`)
  — the target lays it out inline before the `j++` continue; `return 0` vs `goto end` and
  `k = 0` placement measured neutral.
- **0** — head loop `tbl[n].y > max.y` operand order. Every remaining hunk is `not-scored`
  (masked branch displacement).

The 0 needs the two reused locals (`temp`: row then list index; `temp2`: column then triangle
number). No earlier ruling fits (not one role/meaning — Rulings 5/6/9; not SOTN/original source
— 8/10), so they are submitted under Ruling 11: r11/proof.md (all prongs, dumps, mechanism,
measured alternatives, permuter).

Mechanisms (r11/dumps.txt):
- list index: crosses 2 calls, so global.c's caller-save retry needs refs > 8
  (`CALLER_SAVE_PROFITABLE`, regs.h:164). Reuse 12 refs → t0 + caller-save (target). Split 8
  refs → no register → stack slot (`sw $0,24($sp)`).
- column: split `col` is a single-block pseudo → local-alloc → v1; target's `andi s0` needs the
  multi-block global pseudo (temp2) seated in s0.

## s1 layer-2 round 1 (2026-09-28): FAIL(EVIDENCE) — ledger only
Fresh cheat-reviewer confirmed the staged body == r11/final.c, the oracle, the behaviour trace
against the asm, every (D)(2) citation in tools/gcc-2.7.2, and both necessity arguments in
substance, and passed (H). It failed three things, all fixed without touching the body:
1. r11/proof.md + dumps.txt named pseudo 98 as the split `col` (a number from an earlier split
   variant's dump); the banked excerpt contradicted it. Now the mapping is computed from each
   dump by r11/pmap.py (split: row 93 → v0, col 94 → v1 in block 1, triangle number 176 → s0,
   list index 82 → no register).
2. (C)(1): the one-var form left `temp2`'s second value at function scope; it is now declared
   in the record-loop body (byte-identical asm, still 21). Permuter campaign 3 re-run from it.
3. The commit message pre-claimed a layer-2 PASS and said "below" for "not above" the
   caller-save threshold.
Also taken from the review: global.c's kick-out path cannot hand out t0 (used2 ⊇
call_used_reg_set on the first pass), now stated in (D)(2); function comment's "mid y" reworded.

## s1 layer-2 round 2 (2026-09-28): PASS
Fresh cheat-reviewer re-derived the package from scratch (raw dumps, pmap mapping read from the
insns, gcc citations, permuter logs) and passed both variables on every Ruling 11 prong plus (H).
Non-blocking notes applied before commit: campaign-1 tally 28 finds (2 unscored, both invalid),
iteration counts per the logs, commit message cites regs.h for CALLER_SAVE_PROFITABLE.

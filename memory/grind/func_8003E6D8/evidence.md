# func_8003E6D8 — evidence (manual session 2026-09-23)

State at 2026-09-23: COMPLETED-INLINE-ASM-CANONICAL candidate = `candidate.c`
(sandbox --disable all 0 at 297/297; full-build SHA1 == oracle 62efab4f with
the body + aggregate merges applied).

## Floor trajectory (sandbox --disable all, candidate)
- 31  first draft (sibling func_8003EB84's inner loop reused verbatim)
- 21  scratchpad loop: src2 / mask / dst assigned in target order
- 14  pos[] stored vx, vy, vz through one base pointer (the call arg is base+0x1C)
-  9  ratan2 result kept in `ang`, index in a separate `idx` (no move a2,v0)
-  0  column shift written in the `col < 0` arm and at the loop tail (FAKE,
      duplicated-statement-into-arms) instead of in the for-increment

## The last 9 points: one global-alloc priority pair
tools/ra_solver (extract.py on a spliced copy under tmp/, simulate.py
29/29 dispositions): bits = pseudo 81 (nrefs 17, livelen 78, pri 8717),
a3 = pseudo 87 (nrefs 15, livelen 51, pri 8823). Target needs bits first
($t1) and a3 second ($t2). inverse.py (--swap 81,87): minimal single atoms are
bits refs 17->18, bits livelen -2, a3 refs -1, a3 livelen +2.
- The duplicated shift lifts bits to nrefs 23 (pri 11500); jump2 cross-jumps
  the copies back into the single `sll` in the loop branch's delay slot.

## Aggregates (per-word splat symbol -> aggregate merge)
- 0x80101DF0 / 0x800FF638 = two Unk80101DF0Record (0x58 bytes, include/code6cac.h):
  unk0 +0, unk1 +1, unk8 +8 (g_anim_func_table index), unkC +0xC,
  xf {rot +0x10, mat +0x18}, work +0x38 (MATRIX; t = +0x4C/50/54).
  First landed attempt (layer-2 round 3 FAIL, 2026-09-23) declared only
  {rot, mat} at 0x80101E00 -- mid-record; the census (named_syms 2026-05-17
  target A/B) and func_80049E4C's two parallel setups show the real base.
  Evidence: func_80049E4C sets both instances at identical offsets and stores
  the bases to D_800A3708/D_800A370C; func_800418D0 passes a negated stack
  copy of +0x10..0x14 and &+0x38 to g_anim_func_table[+8], copies +0x38 -> +0x18; func_800475A4 uses
  +0x10 (rot), +0x18 (ApplyMatrix), +0x2C..0x34 (mat.t); this target forms one
  base at +0x10 and passes base+0x1C, hence the nested xf member.
  Consumers rewritten (byte-identical): sound.c func_80046BF4 (the
  `(char *)rot_base - 0x10` arithmetic became &D_80101DF0), camera_InitBoneData,
  func_800475A4; text1b.c func_80049E4C, func_8005508C; code6cac_c2.c
  func_8003E6A0. func_8005490C (INCLUDE_ASM) keeps E00/E02/E04/E08/E3C/E40/E44
  rows with alias suffixes.
- 0x800948BC = StageFuncEntry[] {init, unk4} (include/game.h). config.c's
  stage_ExecInitFunc indexed it with an explicit `id << 3` byte pun; the
  .data shows 8-byte fn-pointer pairs (entry 13 = camera_InitBone2 /
  func_800475A4). D_800948C0 (the +4 field) removed from code6cac.h.

## Island provenance
Four islands, spelled as func_80019310 / func_800203B4: gte_SetRotMatrix,
gte_ldv0 (lwc2 pair + 2 nops), MVMVA .word 0x4A486012, gte_stlvnl. Covered by
the 2026-09-01 widened-anchor owner grant (decisions.md:18082 names this
function); registry row 1de410a11 (owner-instructed 2026-09-23).

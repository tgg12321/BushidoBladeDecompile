# special_camera_get_rot_dir (code6cac_b2_post.c) — WIP, BLOCKED (4-reg rotation)

## TL;DR (2026-06-14)
Camera rotation-direction reader. HEAD matches only via 5 register pins
(index s2, cam_base s3, constant_80 s4, copy_end s5). Removing them leaves a
pure **4-register rotation** vs target — instruction count + structure already
match (72==72), ONLY the s2-s5 assignment differs:
- mine:   index->s3, cam_base->s4, constant_80->s5, copy_end->s2
- target: index->s2, cam_base->s3, constant_80->s4, copy_end->s5  (decl order)

## Critical: the masked sandbox is BLIND here
`sandbox --disable all` masks register names, so it scores 12 with OR without
the pins. Iterating against it is futile — the rotation is invisible. The FULL
build SHA1 is the only signal. (This is the masked-0/masked-N caveat in CLAUDE.md.)

## Resume steps
1. Paste candidate.c (pins removed); structure/count already match.
2. Reshape live ranges so GCC's allocno priority gives index->s2 (lowest reg).
   index is the most-used (index+cam_base twice in the retry loop). Try
   register-alloc-pure-c Lever A (block-local splits / use-order). VERIFY each
   attempt with a full build (verify-oracle/retire), NOT the masked sandbox.
3. Modality: decomp-permuter (its RA mutation pass targets this rotation).

## Ruled out (do not re-derive)
- Reordering the assignments (index/cam_base before constant_80): breaks the
  prologue (target assigns constant_80->s4=128 FIRST). Keep that first.

## This is the documented hard wall-class
Same as marionation_Exec — a coupled register-rotation behind GCC 2.7.2's
allocno-priority tiebreaker (global.c:624). See
memory/project/register-alloc-deep-dive.md BEFORE deep work. May be a genuine
plateau needing canonical-asm authorization.

## Pointers
- `.claude/rules/register-alloc-pure-c.md` (Lever A; the confirmed-limits section)
- `memory/project/register-alloc-deep-dive.md`

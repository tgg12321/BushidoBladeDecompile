# func_800200DC — coupled register-rotation wall (code6cac.c, projectile angle)

## TL;DR (2026-06-14)
Honest distance 14; both builds 168 insns (structure already correct). The 14
regfix rules are ALL `subst` register renames (NO insert/delete/reorder) — a
dense coupled $v0/$v1/$t1/$a1/$a2 rotation across the projectile-launch-angle
compute chain. A mid-function CALL (`func_8007E11C`, the sqrt) forces
callee-save decisions that couple the whole allocation, so every statement
reorder I tried cascaded WORSE (15 / 38 / 72), never better. No floor
improvement; blocked. Same plateau class as func_80072CD4 / D_80083418 this
session (but heavier — 14 coupled renames vs their handful).

## Resume steps
1. `sandbox func_800200DC --disable all` on HEAD = 14 (14 rules stripped).
2. The diff is pure register-rename: first/cleanest is the `dy = arg1[1] -
   arg0[1]` subu landing in $v1 (target $v0); then the disc/sq/a0 chain mflo
   pairs and the `move v1,v0` / `addu v0,a2,v1` cluster swap $v0/$v1/$t1/$a1.

## Live hypotheses
- Instrumented cc1 ALLOCDBG/PRIODBG ([[register-alloc-deep-dive]]) — hand
  levers cascade because of the sqrt call's callee-save coupling; need to read
  the allocno-priority tiebreak directly.
- Flip ONLY `dy`'s register without disturbing the call save/restore (root of
  the rotation) — not found by reorder.
- Likely register-alloc-pure-c "confirmed limits" -> canonical-asm review.

## Ruled out (do not re-derive)
- a2 = arg2<<5 before the sqrt call -> 15 (callee-save +1).
- disc operand-order swap (arg3*dy2 + arg2*arg2) -> 38 (reassociation, 167 insns).
- hoist dy before the dist==0 return -> 72 (dy survives the call, -5 insns).

## Pointers
- `.claude/rules/register-alloc-pure-c.md` (confirmed-limits class).
- Siblings on the same wall this session: memory/wip/func_80072CD4/,
  memory/wip/D_80083418/. Contrast mot_data_set (text1a_c) which WAS closable —
  a single clean swap fixed by reordering chroma before the copy; this one's
  rotation is coupled through a call and does not yield to the same trick.

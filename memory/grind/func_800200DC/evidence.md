# Evidence bank — func_800200DC

## s1 recon (2026-07-27)
- Baseline re-verified at HEAD: canonical verdict C (168 insns, distance 14);
  `sandbox --disable all` = 14, build 168 / target 168, 14 rules stripped.
- Exact residual map (artifact tmp/grind/func_800200DC/s1/diff_named.txt):
  14 = Rotation A (6: dy v0-vs-v1 through the disc chain) + Rotation B
  (8: sq/quotient $v1<->$a1 swap after the second sqrt call). No insertion/
  deletion/reorder diffs — pure renames, consistent with all-14-subst rules.
- Target burns $t1 for the arg3*dy2 mflo while our build reuses the freed
  v-reg — the one asymmetry that is NOT a pure rename; suggests a liveness or
  allocno-order difference rooted at dy (Rotation A root).
- m2c reference shape confirms our C structure exactly (a0=300 preset hoisted
  into the bltz delay slot; shared arg4[2] tail store via cross-jump). Artifact:
  tmp/grind/func_800200DC/s1/m2c.c.
- No duplicate/sibling analog: find_duplicates.py full scan, 0 entries for this
  function (tmp/grind/func_800200DC/s1/dups_all.txt, 206 pairs).

- WIP rejected_form: {'form': 'compute a2 = arg2<<5 BEFORE sq=func_8007E11C(...) (across the call)', 'score': 15, 'reason': 'Forces a2 callee-save across the call; +1 worse.'}

- WIP rejected_form: {'form': 'disc = arg3*dy2 + arg2*arg2 (operand order swap)', 'score': 38, 'reason': 'Reassociates the mult/add chain; insn count drops to 167, large divergence.'}

- WIP rejected_form: {'form': 'hoist dy = arg1[1]-arg0[1] before the dist==0 early-return', 'score': 72, 'reason': 'dy must survive the func_8007E11C(dx*dx+dz*dz) call -> callee-save cascade, -5 insns, severe divergence.'}

- == imported from memory/wip notes.md ==
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


- [s1] Baseline at HEAD: canonical=C, sandbox --disable all = 14, build 168 / target 168, 14 subst rules stripped (matches ledger import).

- [s1] Exact diff map artifact: Rotation A = subu dest + mflo/sll/addu operand renames in the disc chain; Rotation B = move-sq/mflo-quotient/bgez-sra-addiu-sra/subu cluster, a pure $v1<->$a1 swap.

- [s1] Asymmetry: target uses $t1 for the arg3*dy2 mflo (`mflo $t1; addu $v1,$v1,$t1`) where ours reuses the freed v-reg (`mflo $v1; addu $v1,$v0,$v1`) — with low-first alloc order this implies $v0/$a0/$a1/$t0 were occupied in target's RTL there, i.e. dy possibly live past the sll; Rotation A is the likely rotation root.

- [s1] m2c confirms structural completeness (a0=300 delay-slot preset, cross-jumped shared arg4[2] store).

- [s1] No duplicate/sibling analog exists (full find_duplicates.py scan).

---
name: endgame-lock-disposition
paths: [".claude/rules/endgame-lock-disposition.md"]
# on-demand: surfaced via codegen-technique-index; read when an escalation
# proposes canonical-asm OR a new coercion family for an RA/scheduler-locked fn.
description: "OWNER STANDING POLICY 2026-07-20 — disposition of RA/scheduler-locked endgame functions that byte-match only via a cheat and are a few insns short in pure C: canonical-asm ONLY with hand-coded-asm evidence (scan_hand_coded S1/S2/S6-class signals); coercion families ONLY with SOTN-master precedent; absent both, the function is an owner-accepted INCOMPLETE exception — cheat retained to hold the match, never sanctioned, never closed."
metadata:
  type: rules
---

# Endgame RA/scheduler-lock disposition (OWNER POLICY, 2026-07-20)

Standing owner ruling (Trenton, 2026-07-20), issued while disposing three
escalations at once (func_80045294, cpu_side_move_dir_4, func_80057CC8 — all
REFUSED, docs/grind/decisions.md 2026-07-20). It generalizes the
motion_SetMotion (2026-07-18, refused) and hirahira_w_frie (2026-07-17,
sanctioned) precedents into a decision rule so this species is not
re-litigated one function at a time.

## The species
A function that: (1) byte-matches on main **only via a cheat**
(prologue_fix, or regfix/asmfix rules); (2) is a small number of instructions
short of a byte match in honest pure C; and (3) has had its sanctioned pure-C
levers exhausted (multi-session grind, mechanism named at GCC-pass level). The
residual is almost always a register-allocation or instruction-scheduling
tiebreak.

## The two owner criteria (both are AND-gates, default = refuse)
1. **Canonical-asm is allowed ONLY with evidence the original was hand-written
   assembly.** Operationally: `python3 tools/scan_hand_coded.py --single <fn>`
   must show real signals (S1 multu pacing / S2 empty branch / S6 BIOS
   jumptable-class — the STRONG tier). A LOW score (0–2/8, "no strong hand-coded
   indicators") is dispositive: **refuse asm.** A compiler-scheduling or
   register-allocation artifact is by definition ordinary GCC output, not a
   hand-coded signature — never authorize asm to paper over one.
2. **A coercion / spelling family is sanctioned ONLY with SOTN-master (or
   equivalent community: Vagrant Story / ESA) precedent.** The burden of proof
   is on the mechanism to exhibit the precedent; "believed community-viable",
   "the only lever left", and "measured to work" do NOT lower the bar. A
   spelling unique to this repo, or a novel extension of an existing sanctioned
   family to a shape that family's evidence does not cover, is **refused.**

## Disposition when both criteria fail (the common case)
No clean COMPLETED state is reachable. Then:
- **Keep the existing cheat on main** so the full-build oracle stays green
  (the byte-identical build is the project's foundation; removing the cheat to
  get "honest but N-off" C breaks the match and defeats a matching decomp).
- **Classify INCOMPLETE-owner-accepted**: NOT COMPLETED-C, NOT
  COMPLETED-INLINE-ASM-CANONICAL. The retained cheat is *not* sanctioned as a
  technique — it survives only to hold the match, and the function is openly
  flagged unresolved.
- **Park out of active grind, but eligible for re-attempt** if a genuine
  pure-C lever or new tooling emerges. This is the owner-granted exception
  that [[no-park-permanently]] reserves to the owner; it is not a grinder-
  initiated permanent park and not a new "done" category
  ([[no-new-park-categories]]).

## For grind sessions
If you reach this state, do NOT propose asm without STRONG scan_hand_coded
signals, and do NOT propose a coercion family without exhibiting the SOTN
precedent in-hand. File the OWNER-ESCALATION honestly (both options, the
evidence, and "no precedent found" stated plainly if so) and stop — the owner
applies the criteria above. Members of this species so far: motion_SetMotion,
func_80045294, cpu_side_move_dir_4, func_80057CC8.

## Related
- [[no-new-park-categories]] — no new cheat-tolerant categories; register-
  rotation/prologue-order are pure-C-reachable, not carve-outs
- [[no-park-permanently]] — owner-only exception grant
- [[hand-coded-asm-recognition]] / [[canonical-asm-retirement]] — the asm
  evidence bar this policy points at
- [[community-standard]] — the SOTN bar for mechanisms

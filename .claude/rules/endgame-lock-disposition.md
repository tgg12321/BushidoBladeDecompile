---
name: endgame-lock-disposition
paths: [".claude/rules/endgame-lock-disposition.md"]
# on-demand: surfaced via codegen-technique-index; read when an escalation

# proposes canonical-asm OR a new coercion family for an RA/scheduler-locked fn.
description: "OWNER POLICY 2026-07-20 + AUTO-RULING 2026-07-27 + judge-sole-gate 2026-08-18 for RA/scheduler-locked endgame functions: canonical-asm ONLY with STRONG scan_hand_coded evidence (pipeline-executed grant, no owner wait); coercion families ONLY with in-hand SOTN precedent (borderline-logged, refused under the current frozen list); absent both, REFUSED / OWNER-ACCEPTED INCOMPLETE applies AUTOMATICALLY. NOTHING escalates to a waiting owner."
metadata:
  type: rules
---

# Endgame RA/scheduler-lock disposition (OWNER POLICY, 2026-07-20)

> **Historical framing note (2026-08-30):** this rule predates the removal of the
> regfix/asmfix rule system (retired at zero rules; machinery deleted). Where the
> symptom text says a function "carries a rule", read it as "the honest build shows
> this diff shape vs target". The technique itself is unchanged.

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

## STANDING AUTO-RULING (owner, 2026-07-27) — both-gates-fail is pre-decided

After 20 consecutive option-(b) refusals (3 on 2026-07-20, 7 on 2026-07-22,
13 on 2026-07-27) the owner declared the criteria permanent — *"My standards
will never change. No cheats, SOTN standard, 100% C or hard evidence for
inline asm"* — and directed that both-gates-fail cases **no longer wait on an
owner ruling**. The disposition below is applied IMMEDIATELY by the pipeline
(grind session or driver backstop): the decisions.md entry is titled
`OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED /
OWNER-ACCEPTED INCOMPLETE` and records both failed gates with evidence; the
queue park reason is the terminal `OWNER-ACCEPTED INCOMPLETE (standing ruling
2026-07-27): …` form. There is no pending state and nothing for the owner to
answer.

**Gate-passing cases (UPDATED by owner ruling 2026-08-18, [[judge-sole-gate]] —
NOTHING waits on the owner anymore):**
1. **Gate 1 passes** — `scan_hand_coded` shows STRONG S1/S2/S6-class signals:
   the pipeline canonical-asm grant path applies. The function stays ACTIVE;
   the whole-body canonical form is authored per
   [[canonical-asm-authorization-recipe]], the Judge makes the final call, and
   the DRIVER writes the `inline_asm_canonical.txt` entry and logs the grant
   to `docs/grind/borderline.md` for later owner audit. No owner sign-off wait.
2. **Gate 2 passes** — a genuinely in-hand SOTN-master (or VS/ESA) precedent
   is EXHIBITED (file + line / commit citation, not "believed viable" or
   "same spirit"): the packet is logged to `docs/grind/borderline.md` as a
   `family-candidate` entry and the function takes the terminal refusal under
   the CURRENT frozen list. The list remains owner-only to extend; the owner
   batch-reviews the ledger and a later landed ruling can spend the entry.

Owner directives verbatim: (2026-07-27) *"I only ever want to be presented
with new techniques that DO have SOTN precedence and are not an explicit
cheat."* — (2026-08-18) *"Anything borderline can be logged somewhere that we
can evaluate later down the line, but i dont want work to just pile up or be
'parked' pending my decisions anymore."* Filing ANY escalation as "awaiting
owner ruling" is a process error — no pending state exists.

## Disposition when both criteria fail (the common case, auto-applied)
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
precedent in-hand. When both gates fail, APPLY THE STANDING RULING yourself
(see the 2026-07-27 section above): file the RESOLVED-BY-STANDING-RULING
entry with both gates' evidence stated plainly, and return owner-gated so the
driver parks terminally. A gate-1-passing case files a CANONICAL-ASM GRANT
PATH entry (stays active); a gate-2-passing case includes the exhibited
precedent so the driver borderline-logs it ([[judge-sole-gate]]). Members of
this species so far: motion_SetMotion,
func_80045294, cpu_side_move_dir_4, func_80057CC8, plus the 2026-07-22 batch
(7) and 2026-07-27 batch (13) — see docs/grind/decisions.md.

## Related
- [[no-new-park-categories]] — no new cheat-tolerant categories; register-
  rotation/prologue-order are pure-C-reachable, not carve-outs
- [[no-park-permanently]] — owner-only exception grant
- [[hand-coded-asm-recognition]] / [[canonical-asm-retirement]] — the asm
  evidence bar this policy points at
- [[community-standard]] — the SOTN bar for mechanisms

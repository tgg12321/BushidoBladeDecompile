# func_800693CC hypotheses

## Open

- The original source probably used a ten-word render context plus six scalar
  locals whose declaration order accounts for `sp+0x40` and `sp+0x48`, but the
  saved-mask store must remain live across the renderer calls without volatile
  or an out-of-bounds aggregate access.
- A canonical aggregate model for the table beginning at `D_8009BC04`
  (`available`, `visible`, then eight `{ action, mode }` byte pairs) may remove
  the final tail address hoist and reduce expansion-time temporaries.
- Revisit with a correctly imported permuter workspace whose baseline agrees
  with `engine sandbox`; the prior workspace's baseline was not comparable.

## Rejected

- Fifteen-word anonymous array as a final model: correct offsets, but 0x78 frame.
- Ten-word array plus standalone input/mask: 0x68 frame, wrong input offset, and
  dead mask store eliminated.
- Reusing unused parameters as general scratch carriers: worse score and no
  frame improvement; also lacks an independent semantic justification.
- Register storage class, inline loop helpers, direct expressions, and shared
  loop temporaries: measured, no frame improvement.

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: rotated 2026-09-24, but ALREADY MATCHED off-main: branch `codex/func-800693cc`, commit 0cf578443 "Match: func_800693CC - COMPLETED-C" (2026-09-28): sandbox 0/307, func_80077894 0/28, verify-oracle SHA1 ok, `queue done` ok on the branch, fresh adversarial PASS. Never merged; branch is 1 ahead / ~456 behind main. It edited src/text1b.c, but on main the function is at `src/text1b_tu1c.c:3891-3895` and its caller func_80077894 at `src/text1b_tu2.c:73` and :1010-1015. Candidate + rejected variants are in that commit's memory/grind/func_800693CC/.
- CONSTRAINTS: two FAKE-annotated `available = &D_8009BC04` aliases with ablation evidence (pointer-alias-fake-exception). `s32 context[15]` has words 10/11/13 never written - a reviewer may test it against the oversized-locals carve-out in dead-vars-local-array.md; the commit's evidence (frame 0x60 vs 0x50 without it, live store to word 14) is the defence. [I, risk] D_8010278C / D_8010278E alias `D_80102788.unk_00[2]/[3]` (PadState, `include/code6cac.h:181`); `undefined_syms_auto.txt:683-684` says retire them with this function -> use struct members on main (one-handle rule) and check bytes.
- BLOCKER: none in codegen; integration only (TU split may shift things [I, low risk]).
- PLAN:
  1. `git show 0cf578443:memory/grind/func_800693CC/candidate.c`; port into `src/text1b_tu1c.c` replacing the stub + INCLUDE_ASM.
  2. Port the `func_80077894(s32 held, s32 pressed)` change; fix the prototype at `src/text1b_tu2.c:73`.
  3. D_8010278C/E references -> D_80102788 members.
  4. Sandbox both functions; `verify-oracle --rebuild`.
  5. Fresh layer-2 on main for both bodies; `queue unpark` (or named manual pop) then `queue done`.
  6. Bring the branch ledger files across.
- DEPENDS: same TU as func_8006BD28 - do this first, it's quick.
- ODDS/LANE: < 2 hours, high. Manual. Cheapest item in the whole queue.

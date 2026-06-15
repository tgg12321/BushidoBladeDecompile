# func_8001F938 (code6cac.c) — WIP, BLOCKED, branch-gen + RA cluster (13 rules)

## TL;DR (2026-06-14)
Triage only. HEAD matches via 13 regfix rules; honest sandbox distance 11
(107==107 insns). The cheat = a $3<->$5 reg-pair swap (idx 1/7/11/15) + a
folded `andi $3,$5,0xFFFF` insert + a 7-rule subst/delete/insert_label cluster
(idx 74-80) that rewrites the saturating-ternary branch generation and
synthesizes a `.LfuncF938_join` label. No edit attempted (complex, 27-commit
history; was "retired" 2026-05-14 leaving these rules — a stabilizer, not
COMPLETED-C).

## The gap
Two saturating ternaries:
- `raw_or_3 = (probe < 4) ? (u16)*((u16*)(arg0+0x270)) : 3; idx = ((raw_or_3<<16)>>15);`
- `sum_or_3 = (sum < 4) ? sum : 3; idx = sum_or_3 * 2;`
GCC emits a different compare/branch shape than target (target uses
`slt $2,$2,4; bne $2,$0,join; sll` for the select + sign-extend). Coupled with
a $3<->$5 register swap — likely a consequence of the branch shape (the
idx-pseudo lands in the other reg).

## Resume steps
1. Edit src/code6cac.c func_8001F938 directly (no candidate to paste).
2. Rewrite the `(probe<4)?(u16)*p:3` ternary to coax target's slt;bne;sll
   shape (explicit if/else, operand-order variants). Fix branch first, then
   re-check the $3/$5 swap.
3. Retire the 13 rules + full SHA1 gate. Verify EACH change via sandbox.

## Pointers
- `.claude/rules/switch-vs-ifchain-branch-sense.md`, `compare-operand-order-register.md`
- 27-commit history: `git log --all -- src/code6cac.c | grep F938` (and the
  2026-05-14 "retire ... un-pin stabilizer" commits that left these rules).

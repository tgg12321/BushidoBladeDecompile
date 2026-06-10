# func_8003AB44 — WIP notes (2026-06-10)

## TL;DR

8-way switch (`D_800A38AC` cases 0..7) in code6cac_c_ab.c. HEAD floor: **6**.
This session removed 3 score-inert cheat-asm scheduling barriers
(`__asm__ __volatile__("" ::: "memory")`) in cases 0 and 1, but did NOT lower
the floor. Candidate is a SOURCE CLEANUP, not floor progress.

## Resume instructions

1. Apply `candidate.c` to `src/code6cac_c_ab.c` (replace the existing
   `s32 func_8003AB44(void) { ... }` body).
2. Run `pwsh tools/eng.ps1 sandbox func_8003AB44 --disable all` → expect
   `score: 6, cheat_asm_stripped: 6` (HEAD has `cheat_asm_stripped: 9`).
3. Iterate from there on the remaining 6-distance gap (see "the gap" below).

## The gap (sandbox stripped, target_insns == build_insns == 93)

### Case 2 — branch polarity flip (4 diffs at idx 42-49)

**Target** wants:
```
beqz $v0, .L_end       # if func == 0, go to end (return 0)
addu $v0, $zero, $zero # delay: set return value
j    .L_done           # else jump to done block
addiu $a0, $zero, 3    # delay: setup for done's next jal
```

**Our build** naturally emits:
```
bnez $v0, .L_done      # if func != 0, go to done block
addiu $a0, $zero, 3    # delay: setup for done's jal
j    .L_end            # else go to end (return 0)
addu $v0, $zero, $zero # delay: set return value
```

These are semantically identical but with INVERTED branch sense and
correspondingly swapped delay-slot fills. GCC's reorg pass picks
`addiu $a0,3` as the BRANCH's delay slot fill (longer INSN_PRIORITY chain
to the done block's jal). Target picked `move $v0,$zero`.

Lever attempts that did NOT shift this polarity:
- Source-level test inversion (`==0 return; goto done` vs `!=0 goto done; return`)
- Wrapping in `{ s32 r; r = func(...); if (r == 0) ... }` (probably untested fully)
- Inlining the done block in case 2 (DID flip polarity but lost 1 insn — score 15 vs 6 base)

### Case 5/6 — register choice ($v1 in build, $v0 in target)

The `D_800A38AC++` translates to `lbu $rN, %gp_rel; addiu $rN, $rN, 1;
sb $rN, %gp_rel`. Our GCC picks $v1, target uses $v0. Regfix
`$2 <-> $3 @ 72-75` rewrites.

Likely cause: case 4 falls through to case 5/6. After `jal gpu_SetDispMask`
(void return), GCC conservatively treats $v0 as recently-returned, and the
case 5/6 jtbl entry label (referenced from the jump table) makes GCC
conservative about live-in registers at that point.

Lever attempts (no effect):
- Split case 4 from case 5/6 with duplicated increment (cross-jump merge
  consolidated them back, still picks $v1)

Not tried this session:
- Re-declare `gpu_SetDispMask` to return s32 (currently void in src)
- Move case 5/6's body BEFORE case 4 in source (might break fall-through
  semantics)

## Reusable observations

- The 3 `__asm__ __volatile__("" ::: "memory")` barriers in cases 0 and 1
  of HEAD's source are SCORE-INERT. Sandbox stripped them in both HEAD and
  the candidate (score 6 in both). Removing them is safe and reduces
  cheat-asm debt.
- This is a single-call-result test-and-dispatch within a switch case.
  The polarity is a delay-slot-fill priority issue, not a control-flow
  issue.

## Related rules

- [[switch-vs-ifchain-branch-sense]] — sibling pattern (rewrite as real
  switch retired regfix rules); applied here but didn't help because
  we already use a switch
- [[register-alloc-pure-c]] — pure-C levers for register allocation issues
- [[store-before-jal]] — relevant for understanding why GCC picks
  particular delay-slot fills
- [[inline-asm-policy]] — the rule classifying `__asm__("" ::: "memory")`
  as cheat-asm (the 3 barriers I removed are in this category)

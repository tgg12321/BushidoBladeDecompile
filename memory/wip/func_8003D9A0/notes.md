# func_8003D9A0 — WIP stub from stranded June-14 branch

## TL;DR

This WIP stub was created 2026-06-24 from the obsolete worker branch
`work/blkB2-0614` (commit `e1f215db`, 2026-06-14). On that branch the
function closed to COMPLETED-C via a loop-counter hoist; on `main` the
same source diff scores 19 due to compilation-context drift (per
[[stranded-branch-work-not-transplantable]] — this function is THE
experiment the memory was written from). Resume by applying the hint,
sandboxing on current main, and pursuing a derivative form if needed.

## The branch's technique (the hint)

```
src/code6cac_c2.c — func_8003D9A0
-    register s16 *s0 asm("s0") = a0;
-    register u32 *s1 asm("s1") = a2;
+    s16 *s0 = a0;
+    u32 *s1 = a2;
     s32 s4, s3;
     s32 s2;
-    asm("" : : "r"(s1));
     s4 = s0[0];
     s3 = s0[1];
+    s2 = a1 - 1;
     if (a1 != 0) {
-        s2 = a1 - 1;
         do {
             ...
```

**Rationale (from the branch commit message):** GCC sank `move s1,a2`
(s1 = a2, dead until the loop) into the delay slot of `beqz a1`,
leaving prologue_fix unable to pair it with its save. Hoisting
`s2 = a1 - 1;` ABOVE the early-exit guard makes cc1 fill the branch
delay slot with `addiu s2,a1,-1` (computed unconditionally), so
`move s1,a2` materializes in the prologue paired with `sw s1` —
matching the target. s2 is the genuine loop counter
(`while (--s2 != -1)`); the hoist is a normal pre-init, not a dead store.

**Branch result:** sandbox `--disable all` 2 -> 0; SHA1 == oracle;
0 rules, 0 cheat-asm; cheat-reviewer PASS.

## Why this is a hint and not transplantable

The standing experiment ([[stranded-branch-work-not-transplantable]])
applied the byte-identical source to `main` and measured **distance 19**
vs the branch's 2. Compiler + flags are frozen
([[no-compiler-divergence]]), so the gap is compilation-context drift
(headers, types, per-file state) accumulated over ~70+ commits the
branch is behind main. The technique may still apply, but via a
derivative form — additional reordering or hoisting that compensates
for whatever per-file state changed.

## Concrete next-session steps

1. Apply the branch diff to `src/code6cac_c2.c` on current main.
2. `& tools/wteng.ps1 main sandbox func_8003D9A0 --disable all` — record
   the current main-context score. Per the 2026-06-16 experiment it was
   19; if it's lower today, the drift may have partially closed.
3. Run `& tools/wteng.ps1 main diagnose func_8003D9A0` — classify the
   gap (matchable / control-flow / canonical / plateau) on current main.
4. If the gap is small (≤5), iterate from the hoist form with statement
   reordering between the `s4 = s0[0]; s3 = s0[1];` reads and the new
   `s2 = a1 - 1;`, and within the do-loop body.
5. If the gap is large (>10) and the diagnose output indicates a
   structural mismatch, the hint may not apply on current main; pursue
   pure-C from scratch with the branch's mechanism (delay-slot-fill via
   hoisted invariant) as the model.

## Cross-references

- Branch: `work/blkB2-0614` commit `e1f215db` (2026-06-14)
- [[stranded-branch-work-not-transplantable]] — the standing experiment
- [[hoist-call-arg-local-flips-jal-delay]]
- [[loop-note-fixes-delay-slot-steal]]

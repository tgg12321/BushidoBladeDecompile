# Hypothesis ledger — damage_DebugDisp

Floor 8 (candidate: for(;;) fence on CopyBlock loop). Verdict C, 79 insns.
The 8 diffs are TWO independent, well-localized problems. Raw objdump shows
9 positional diffs (4 + 5); sandbox masks one → score 8.

## Region A — inner accumulator loop: sum↔j register swap (build 0x124/0x128/0x12c/0x140)
BUILD  : j=$a0, sum=$a1  →  addiu a0,a0,1 ; addu a1,a1,v0 ; sltiu v0,a0,36 ; beq a1,v0
TARGET : sum=$a0, j=$a1  →  addiu a1,a1,1 ; addu a0,a0,v0 ; sltiu v0,a1,0x24 ; beq a0,v0
Scheduling is IDENTICAL (j++ already fills the lbu load-delay slot in both). Pure
register-allocation tie. greg dispositions: sum=pseudo 77→a1, j=pseudo 79→a0.
sum has the LOWER pseudo yet j wins the lower hard reg a0 ⇒ NOT a pseudo-order tie;
it's a local-alloc **priority** tiebreak (j, the counter, outranks sum for $a0).
sum is live across the loop-exit edge (post-loop `beq`), j is dead at exit — yet j
still wins. j is a real integer counter (target uses `sltiu ...,0x24`), so it CANNOT
be eliminated via a pointer-end compare (that would change the loop structure).

## Region B — 2nd-loop preheader: LICM-const block vs move block (build 0x1e8–0x1f8)
BUILD  order: move a0,t1 ; move a2,a0 ; lui t0,0x8000 ; lui a3,0x1f ; ori a3,0xffff
TARGET order: lui t0,0x8000 ; lui a3,0x1f ; ori a3,0xffff ; move a0,t1 ; move a2,a0
The two constants (0x80000000, 0x1FFFFF) are LICM-hoisted out of the k-loop into the
preheader; the two moves (ap=base, a2p=ap) are the pre-loop init. sched1 (basic-block
list scheduler) ties break to the lower-LUID insns; LICM inserts hoisted invariants at
the END of the preheader (higher LUID than the moves) → moves scheduled first. Target
had the constants at lower LUID / higher priority. Constant-holder named locals to bias
LUID were tried and REJECTED as cheat-by-spelling (magic_base/magic_max, score 4).

## Frontier (mechanism-grounded; for the drill session)
1. **Region A — flip the sum/j $a0 tie via priority, no dead code.** Probe: read
   local-alloc qty priority for pseudo 77(sum)/79(j) (.lreg); then try structural
   levers that lift sum's ref-priority or drop j's WITHOUT dead stores — e.g. hoist
   the check value `s32 chk = *(s32*)((u8*)chkptr+0x6C);` before the inner loop
   (changes sum's post-loop consumer to a local compare, may reprioritize sum), or
   narrow/relocate the sum/j init & compare. Verify each with `sandbox --disable all`.
2. **Region B — get LICM constants to schedule before the ap/a2p moves structurally.**
   Probe: change the range-check spelling / pointer-init placement so the moves' LUID
   rises above the hoisted constants OR the constants aren't hoisted into the same
   preheader block (e.g. compute a2p from ap inside the loop, or restructure the
   `(u32)(ptr-0x80000000)<=0x1FFFFF` test). NOT a named constant holder (cheat).
3. **Directed permuter seeded from floor-8.** PERM_* over statement order in the two
   regions + the range-check form; sweep the tie/schedule cross-product. Vet any
   closing form against the cheat catalog before proposing (dead-store/constant-holder
   forms are the known trap here).

## Ruled out (inherited — do NOT re-run)
- swap inner init `sum=0` first (V11): score 9 (rotated bp→a0, j→v1)
- `for(j=0;j<0x24;j++)` (V7): 9 ; `j` as s32 (V8): 9 (slti vs sltiu)
- swap inner body order (V10,V12): 5–10 ; sum at fn scope (V9): 8 (no change)
- combined decl+init (V2,V3): 8–13
- magic_base/magic_max const named locals (LUID bias): score 4 but CHEAT (rejected)

## [s1] The candidate for(;;) fence on the CopyBlock loop holds the floor at 8 (HEAD do-while = 9).
- mechanism: loop-exit-work-inside-loop-sched-fence materialises the trailing-word load-delay nop and pins k=0 after the sw; retires the two trailing-region regfix rules.
- probe: apply candidate.c to src; `sandbox damage_DebugDisp --disable all`
- result: score 8, verdict C, 79/79 insns, 8 rules dropped
- verdict: CONFIRMED

## [s1] Region A is a pure a0<->a1 register swap between accumulator sum and counter j, driven by a local-alloc priority tiebreak (not pseudo order, not scheduling).
- mechanism: Objdump vs target: identical scheduling (j++ fills lbu load-delay in both), only $a0/$a1 naming differs across addiu/addu/sltiu/beq. greg dispositions: sum=pseudo 77->a1, j=pseudo 79->a0 — sum has the lower pseudo yet j wins the lower hard reg $a0, so j (the counter) outranks sum for $a0 on local-alloc priority; sum is live past the loop, j dead at exit, yet j still wins.
- probe: objdump -drz sandbox .o (build_disasm.txt) diffed vs asm/funcs/damage_DebugDisp.s; cc1 -da greg dump register dispositions
- result: 4 positional diffs at 0x124/0x128/0x12c/0x140; confirmed pure allocation tie, no cheat needed
- verdict: CONFIRMED

## [s1] Region B is an independent scheduling diff: LICM-hoisted range-check constants vs pre-loop pointer moves are emitted in opposite order to target.
- mechanism: The two invariants (0x80000000, 0x1FFFFF) are LICM-hoisted into the k-loop preheader; sched1 ties break to lower-LUID insns and LICM appends hoisted insns after the ap/a2p moves, so build emits moves-first while target emits constants-first. Naming constant-holder locals to bias LUID was already rejected as cheat-by-spelling.
- probe: objdump build 0x1e8-0x1f8 vs target 0x800380F0-
- result: 5 positional diffs; region independent of Region A
- verdict: CONFIRMED

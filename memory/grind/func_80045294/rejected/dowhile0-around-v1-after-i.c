/* KILLED s10: do-while(0) around `v1 = a0<<4` after `i = a0`.
   Score: 2 -> 12 (target 83 / build 83, no insn-count change).

   Intent (merged synthesis of s6 pool-split + s7 cse.c BB-scope pass-level
   dead ends): put i=a0 before v1=a0<<4 in source (so sll's assignment LUID
   > move16's LUID -> target sched2 order sw;move16;sll), then wrap the
   v1 assignment in do-while(0) with the aim of forcing a real basic-block
   boundary between the two assignments so cse.c's BB-scoped value-numbering
   cannot substitute pseudo (i) into (ashift a0) at insn 17 (s7 root cause of
   every one-tree-late-assign RA rotation). The construct is sanctioned per
   .claude/rules/do-while-zero-exception.md (owner ruling 2026-07-06:
   do-while(0) is sanctioned for ANY codegen effect incl RA, with mandatory
   FAKE annotation).

   Result: score=12, build_insns=83 -- essentially the same RA rotation family
   as H1 / decl-init-decouple (score 11). The wrap did NOT create a cse.c BB
   boundary that survived. Two mechanism-plausible explanations, both consistent
   with the +12 measurement:
     (a) jump.c pass 1 folded the do-while(0) exit test (while(0) -> unconditional
         fall-through) BEFORE cse.c ran, collapsing the two would-be BBs into one.
         Then insn 15 (set 77 72) and insn 17 (ashift ...) are again in a single
         BB and cse.c substitutes 77 for 72 in the ashift as in every prior
         one-tree-late-assign near-hit.
     (b) cse.c does span across simple fall-through edges (cse_extended_basic_block
         name is not accidental), so even if the BB boundary survives jump.c, the
         value-equivalence class propagates across the fall-through into the wrap's
         body and the substitution fires anyway.
   Either way, do-while(0) is not a lever for defeating the cse.c substitution
   for THIS coupling.

   Implication: frontier #2 ("novel CFG-split rewrite that puts i=a0 and
   v1=a0<<4 in distinct BBs") loses do-while(0) as its natural realization.
   Any surviving realization would need a genuine control-flow join (semantic
   conditional branch that keeps insns 15 and 17 in unrelated CFG regions),
   which saTan0Init lacks (no semantic bounds check on a0/a1). Frontier #2 is
   effectively dead absent a permuter find of a shape that side-steps the fold.
*/
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 i = a0;
    s32 v1;
    s32 s4;
    s32 count;
    s32 s5;
    /* FAKE: do-while(0) attempts to create BB boundary defeating cse.c
       BB-scoped substitution of pseudo (i) into (ashift a0). Did not work. */
    do { v1 = a0 << 4; } while (0);
    s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    count = D_800A33AC;
    s5 = s4 + a1;
    /* rest unchanged */
}

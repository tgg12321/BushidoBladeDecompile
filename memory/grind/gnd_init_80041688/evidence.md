# Evidence bank — gnd_init_80041688

- [s1] sandbox --disable all: score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 (whole-file; nothing in gnd_init_80041688 itself).

- [s1] regfix.txt:477-479 are the 3 sole rules: pure subst on maspsx idx 68/69/70 swapping `$reg,offset($16)` operand of the three FALSE-color lbu insns. No cheat-asm, no pins, no barriers in the C source.

- [s1] asm/funcs/gnd_init_80041688.s .L80041798 target FALSE-color block: `lbu $v1,0x1A; lbu $a0,0x18; lbu $v0,0x19; sll a0<<16; sll v0<<8; or a0,a0,v0; or a0,v1,a0`. Register assignment b=v1,r=a0,g=v0.

- [s1] Cheat-free build FALSE-color block: same registers assigned (b=v1,r=a0,g=v0), same shifts, same OR-chain (`or a0,v1,a0` as final — b on LEFT of outer OR matches source `b | ((r<<16)|(g<<8))`). Only diff: lbu emission order is [r,g,b] instead of target's [b,r,g].

- [s1] Mechanism: sched1 backward-scheduling picks by INSN_PRIORITY = longest chain to end-of-block. r,g each have chain length 4 (lbu → sll → or_mid → or_final); b has chain length 2 (lbu → or_final). So r,g launch first. Target's original C must have made b's priority equal-or-higher (extra chain link on b) OR sched.c `true_dependence` saw an alias edge on the r,g MEMs it doesn't see for mine — a MEM_IN_STRUCT_P `/s` flag pair split (proven-spelling-class-reconstruction family).

- [s1] TRUE-color branch matches target: `func_8004881C(b,g,r)` arg-setup lbu's are ordered by argN (a0=b,a1=g,a2=r) — GCC's arg-passing naturally pins the emission order. No diff on that branch.

- [s1] or-tree-shape-shift (parenthesization reshuffle of the OR chain to give b a longer chain via `((b|r)|g)` etc.) is FORBIDDEN per the technique index — not a lever.

- [s2] s2 baseline (unchanged from s1): sandbox --disable all -> score=2, target_insns=82, build_insns=82, rules_dropped=3.

- [s2] Target FALSE branch (.L80041798): lbu v1,0x1A -> lbu a0,0x18 -> lbu v0,0x19 -> sll a0,a0,16 -> sll v0,v0,8 -> or a0,a0,v0 -> or a0,v1,a0. Registers b=v1, r=a0, g=v0.

- [s2] Build FALSE branch: lbu a0,0x18 -> lbu v0,0x19 -> lbu v1,0x1A -> sll a0,a0,16 -> sll v0,v0,8 -> or a0,a0,v0 -> or a0,v1,a0. IDENTICAL registers, IDENTICAL OR-tree shape (`or a0,v1,a0` final has b on LEFT). ONLY diff: the three lbu's emit in [r,g,b] instead of [b,r,g]. Diff = 3 lbu-swap-position insns => score=2 (masked weight).

- [s2] Target has TWO SEPARATE lbu blocks (TRUE arm @ 80041770-78 loading a0=0x1A, a1=0x19, a2=0x18 for `func_8004881C(b,g,r)` call; FALSE arm @ .L80041798). The TRUE-arm order is arg-passing-driven and matches naturally.

- [s2] sched1 chain-length priority: r,g each have 4-insn chain to final or; b has 2-insn chain. Priority-ordering places r,g ahead; b emits last regardless of declared/statement order.

- [s2] Four s2 structural axes measured dead at floor=2: MEM_IN_STRUCT_P (struct cast), MEM_ALIAS_SET (walking pointer), CFG restructure (ternary hoist), pseudo/decl order (block-local split). None move sched1's priority-driven emission.

- [s2] or-tree-shape-shift (repartner `(b|r)|g` to give b a longer chain) is FORBIDDEN per codegen-technique-index; not a lever.

- [s3] s3 baseline (unchanged from s2): sandbox --disable all -> score=2, target_insns=82, build_insns=82, 3 rules_dropped.

- [s3] s3 rg (block-local named intermediate): score=2. objdump FALSE @0x15cc-0x15e4: lbu a0,0x18 -> lbu v0,0x19 -> lbu v1,0x1A -> sll a0<<16 -> sll v0<<8 -> or a0,a0,v0 -> or a0,v1,a0. Byte-identical to baseline. `rg` folded pre-sched1.

- [s3] s3 cp (block-local): score=2. Base register in the three lbu's is $s0 (= player), not a distinct pseudo. Emission still [r,g,b]. cp -> player via GCC copy-prop before sched1 sees a distinct MEM.

- [s3] s3 cp (function-scope): score=15. Whole-function RA regression; cp's extended liveness costs more than any FALSE-branch gain. Function-scope liveness of a redundant pointer alias is inert for this reschedule AND destructive on total score.

- [s3] Cumulative structural axes measured dead across s1-s3: statement-reorder-b-first, struct-cast /s flip, walking-pointer bp arithmetic, ternary/hoist consolidation, block-local fb/fr/fg split, block-local cp copy, function-scope cp, block-local named-intermediate rg. Nine forms; all sched1 chain-length dominates.

- [s3] Structural conclusion: within block-local structural rearrangements of the FALSE arm, no axis moves sched1's INSN_PRIORITY-driven emission because r/g's chain-length-4 vs b's chain-length-2 is a HARD ordering — LUID/DECL/COPY-based tie-break only applies at equal priority. Every attempted axis either (a) folded before sched1 (rg, cp-block-local), (b) collapsed via combine (walking-pointer, struct-cast), or (c) net-regressed via broader RA cost (cp-function-scope, hoisted-loads).

- [s3] or-tree-shape-shift (repartner `(b|r)|g` to give b longer chain) FORBIDDEN per codegen-technique-index.

# Evidence bank — gnd_init_80041688

- [s1] sandbox --disable all: score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 (whole-file; nothing in gnd_init_80041688 itself).

- [s1] regfix.txt:477-479 are the 3 sole rules: pure subst on maspsx idx 68/69/70 swapping `$reg,offset($16)` operand of the three FALSE-color lbu insns. No cheat-asm, no pins, no barriers in the C source.

- [s1] asm/funcs/gnd_init_80041688.s .L80041798 target FALSE-color block: `lbu $v1,0x1A; lbu $a0,0x18; lbu $v0,0x19; sll a0<<16; sll v0<<8; or a0,a0,v0; or a0,v1,a0`. Register assignment b=v1,r=a0,g=v0.

- [s1] Cheat-free build FALSE-color block: same registers assigned (b=v1,r=a0,g=v0), same shifts, same OR-chain (`or a0,v1,a0` as final — b on LEFT of outer OR matches source `b | ((r<<16)|(g<<8))`). Only diff: lbu emission order is [r,g,b] instead of target's [b,r,g].

- [s1] Mechanism: sched1 backward-scheduling picks by INSN_PRIORITY = longest chain to end-of-block. r,g each have chain length 4 (lbu → sll → or_mid → or_final); b has chain length 2 (lbu → or_final). So r,g launch first. Target's original C must have made b's priority equal-or-higher (extra chain link on b) OR sched.c `true_dependence` saw an alias edge on the r,g MEMs it doesn't see for mine — a MEM_IN_STRUCT_P `/s` flag pair split (proven-spelling-class-reconstruction family).

- [s1] TRUE-color branch matches target: `func_8004881C(b,g,r)` arg-setup lbu's are ordered by argN (a0=b,a1=g,a2=r) — GCC's arg-passing naturally pins the emission order. No diff on that branch.

- [s1] or-tree-shape-shift (parenthesization reshuffle of the OR chain to give b a longer chain via `((b|r)|g)` etc.) is FORBIDDEN per the technique index — not a lever.

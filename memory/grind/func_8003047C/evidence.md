# Evidence bank — func_8003047C

- Audit diagnosis (regressions.md): Dead-goto label-pad confirmed: `goto loop_start;` immediately before `loop_start:` has no semantic effect (execution falls through anyway) and exists solely to coerce GCC's CFG/loop-analysis. Forbidden family per cheat-reviewer catalog. Additionally, `s32 neg1 = -1` is a suspicious opaque-constant variable not covered by the SOTN s32-one=1 sanction. Worker must redo with a clean loop structure (while/do-while/for, or goto-based without the dead leading jump) and verify that neg1 can be replaced by the literal -1.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged constructs located: src/code6cac_b.c:2343-2344 (`goto loop_start;` immediately before `loop_start:`) and src/code6cac_b.c:2333+2341 (`s32 neg1 = -1`).

- [s1] [fable-blitz 2026-07-07] asm/funcs/func_8003047C.s:6 materializes -1 ONCE in the preamble (addiu $a3,$zero,-1) and consumes it every iteration at line 22 (beq $v0,$a3). The loop is goto-based in C (no NOTE_INSN_LOOP -> no LICM) and cse cannot carry a constant across the loop label, so a literal -1 in the comparison would re-materialize per iteration -> neg1 is LOAD-BEARING-likely for constant placement.

- [s1] [fable-blitz 2026-07-07] asm/funcs/func_8003047C.s:8-32: entry falls through into .L80030494; bottom-tested bnez back-edge with addiu $a2,$a2,2 in the delay slot. The leading `goto loop_start;` emits a jump-to-immediately-following-label, which GCC 2.7.2 jump.c pass-1 deletes before cse/loop/sched run -> dead goto is INERT-likely.

- [s1] [fable-blitz 2026-07-07] Delay-slot semantics: addiu $a2,$a2,2 (asm line 32) executes on EVERY iteration incl. the last (delay slot of bnez), and a2 is dead after the loop -> a plain `do { ...; i++; a2 += 2; } while (i < 5);` is semantically equivalent to the committed goto form; a clean-loop candidate exists.

- [s1] [fable-blitz 2026-07-07] In-file precedent: COMPLETED siblings func_80030524 (src/code6cac_b.c:2368) and func_80030D04 (src/code6cac_b.c:2664) ship the identical `s32 neg = -1` spelling with do-while loops on main, un-annotated.

- [s1] [fable-blitz 2026-07-07] Judge bar (memory/grind/func_8003047C/state.json): clean loop structure (while/do-while/for or goto-based WITHOUT the dead leading jump) + verify neg1 replaceable by literal -1. No tension with the do-while(0) 2026-07-06 ruling on the GOTO itself (judge explicitly allows loop restructure); the tension is on neg1 (see ruling_question).

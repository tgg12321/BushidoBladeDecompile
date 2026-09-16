# SELF-VET — func_80056CB8 (s6, 2026-09-16)

CONSTRUCTS: `extern s32 D_800F6610;` (new TU-local extern declaration) + using
`D_800F6610` in place of `D_800F6608.w8` at the second ratan2 argument. No
other construct changed vs the s5-banked candidate (the shared-idx-local
experiment and the goto-tail experiment were both reverted this session and
are not part of the candidate).

## T1 semantic purpose
`D_800F6610` names the exact same storage as `D_800F6608.w8` (0x800F6608+8 =
0x800F6610, confirmed via undefined_syms_auto.txt). The construct changes
WHICH SYMBOL the read is spelled through, and that symbol choice has a
directly observable effect on the emitted bytes (two independent lui/lw pairs
vs one lui + two lw offsets) — confirmed by objdiff before/after. Not a no-op
spelling; it changes real codegen in the direction the target bytes require.

## T2 human-programmer test
Yes — a programmer who had two named globals in their original source
(`D_800F6608` for the camera-anchor struct and a second, distinct global
`D_800F6610`) would naturally read them as two separate C names. This is
exactly what the target's disassembly shows (asm/funcs/func_80056CB8.s:51-55
names D_800F6610 explicitly via its own %hi/%lo relocation) — this isn't an
inference from scheduling, it's a literal symbol name in the binary.

## T3 GCC-internals justification
Not needed as the primary justification: the change is justified by the
target's own relocation entries (an object-model / declaration fact), not by
an appeal to combine.c/global.c/etc. The GCC-level EXPLANATION for why the
bytes differ (CSE not sharing an %hi(D_800F6608) load across a call boundary
vs two independent %hi/%lo pairs) is incidental background, not the
justification for the construct.

## T4 permuter/search provenance
Not permuter-found. Found by direct evidence inspection: `tools/objdiff.py`
diffing the honest sandbox object against `build/src/text1b.o` (the
INCLUDE_ASM reference, i.e. target bytes) isolated the exact 2-line mismatch,
then reading the target's own asm/funcs/func_80056CB8.s confirmed the literal
symbol name.

## T5 family check
Not a member of any forbidden family: no dead code, no pins, no volatile
coercion, no alias rename (`D_800F6610` is the SYMBOL'S OWN name, not a
renamed alias of `D_800F6608`), no scheduling barrier. It's an ordinary
`extern` declaration of a real, separately-addressed global and an ordinary
read of it — plain C.

## T6 naming-announces-intent
`D_800F6610` is the auto-generated splat name for its own address (matches
the project's standard `D_<addr>` convention for anonymous data symbols) — no
pad/dummy/spill/tmp naming signal.

SANCTIONED-FAMILY-CLAIMS: none — this construct does not need a sanctioned
no-semantic-purpose family; it is ordinary compilable C with a truthful
semantic reading (Ruling 1, [[ordinary-c-judge-decidable]]: "a construct with
a real semantic reading is never a cheat merely because the agent chose it
after observing generated code").

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.

## Note on floor
This fix is SCORE-NEUTRAL (81 -> 81, build_insns 197 -> 197 both before and
after) — it does not move the honest floor. It is banked because it is a real
correctness fix (removes a wrong read the objdiff proved was wrong) with zero
downside, and because it cleans the remaining objdiff signal for future
sessions (see candidate.c's REMAINING RESIDUAL section for what's left: a
register-rotation cluster + a reorg.c delay-slot-duplication difference in
the code==4 tail). Given it is score-neutral, this session's outcome is
`progress`, not `candidate-ready` — floor is not 0.

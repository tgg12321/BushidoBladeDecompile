# SELF-VET — func_80017FA0

Diff under vet: `src/code6cac.c` only — `INCLUDE_ASM("asm/funcs", func_80017FA0);`
replaced by the body in `memory/grind/func_80017FA0/candidate.c`, plus the two
mechanical call-site/prototype fixes the body requires
(`extern void func_80017FA0(void);` -> `extern void func_80017FA0(s32 *);` and
`func_80017FA0();` -> `func_80017FA0(p0);`). No other file in the build pipeline
is touched. Measured THIS session: `sandbox func_80017FA0 --disable all` = 0 and
`build` SHA1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa (the oracle).

CONSTRUCTS: (1) goto-formed INNER loop (`inner:` label + `if (j < 2) goto inner;`)
in place of `do { ... } while (j < 2);`; (2) rotated outer-loop entry guard
`if (i < ptr[1])` against the live counter `i`; (3) `goto end;` early-exit to a
trailing empty label; (4) ordinary pointer/offset locals (`scr`, `ptr`, `p68`,
`ac_base`, `dp`, `sp_off`, `sp_inner`, `data_off`) and numeric absolute-address
stores `*(s32 *)(0x1F800064 + sp_inner)`. NO volatile anywhere, no inline asm, no
register pins, no dead locals, no local arrays, no aliases, no FAKE constructs,
zero regfix/asmfix rules.

## T1 semantic purpose
(1) The goto IS the inner loop's back edge: delete it and the body executes once
instead of twice — the function then writes 3 of the 6 words per group and is
simply wrong. It is live control flow that executes on every group, not a
semantically inert addition. (2) `if (i < ptr[1])` is the loop's real entry test
(`i` is 0 there, so it is the identical predicate to `ptr[1] > 0`, and it is the
same comparison the loop's own back-edge test performs); it guards the loop.
(3) `goto end;` is the null-pointer early-out; it is executed whenever a0[3]==0.
(4) Every local carries a value that is read and stored; every store lands in the
target's bytes. No construct in this diff is removable without changing what the
function computes, except (2)'s and (1)'s SPELLING — see T2/T3.

## T2 human-programmer
Yes on all four. A goto-formed counted loop is idiomatic late-90s C and is
already the shipped, byte-matched form of two COMPLETED-C functions in THIS VERY
FILE (`func_800206B0`, src/code6cac.c:2125 + :2143; `func_80021280`,
src/code6cac.c:2220 + :2224) — including the same "wrap a temp in a bare block
inside the loop body" shape. A reader does not ask "why is this here?" about a
loop's own back edge, its entry guard, or its null-check early-out. The numeric
scratchpad addresses are how PS1 code writes scratchpad RAM.

## T3 GCC-internals justification
Stated honestly: GCC internals explain WHICH of two semantically identical
spellings the original source used, not why any construct is present. The
construct's presence is explained entirely by program logic (it is the loop).
The measurement (from the cc1 `.loop` dump, tmp/grind/func_80017FA0/s5/vNV.loop,
not from a guess) is that the C front end emits NOTE_INSN_LOOP_BEG/END only for
for/while/do statements and loop.c analyses only note-delimited loops; with the
do-while spelling loop.c forms the three scratchpad stores' addresses as
DEST_ADDR givs of the biv `sp_inner`, combine_givs merges them and
strength_reduce hoists one biased base out of the loop (57 insns vs target 61).
This is the SAME axis, and the same NOTE_INSN_LOOP_BEG mechanism, that
`.claude/rules/loop-note-fixes-delay-slot-steal.md` documents as the ordinary-C
fix for a cheat-asm barrier — there the repair direction was goto -> while; here
the evidence (the target bytes) says the original was goto-formed. Choosing among
ordinary C control-flow structures (switch vs if-chain, while vs do-while vs
goto, mixed exit forms) is the substance of matching decompilation, not a
coercion: nothing inert is added, no type is falsified, no register is pinned,
nothing is hidden from the optimizer by an attribute.

## T4 permuter/search provenance
Not permuter output. The session ran the mandated pass-attribution dump first
(`cc1 -dL`), read the giv/combine/reduce trace in vNV.loop, then read
tools/gcc-2.7.2/loop.c (strength_reduce:3823, combine_givs:5494,
combine_givs_p:5457, express_from:5417) to establish that with a numeric address
on one biv, reduction is unconditional (benefit 6 - add_cost 2 > 0, threshold
huge) — i.e. the giv must never be FORMED. The loop-note property was then
derived from that reading and confirmed by one measurement. No detector-evasion
reasoning is involved; the construct is visible, live, and ordinary.

## T5 family check
No forbidden family matches, by shape or by analogy. Explicitly checked against
the catalog: the goto here is NOT "dead-goto label-pad" (the goto is the back
edge and always executes; nothing is padded) and NOT
"goto-end-with-ret-val accumulator + shared label" (no return value, no
accumulator, the label is the loop head, and this function's `goto end;` is the
plain mixed-exit form). No register-asm pin, no hardcoded-$N asm, no scheduling
barrier, no volatile of any kind (the s4 volatile-scratchpad construct is BANNED
for this function and is absent — verified by grep: the word `volatile` does not
occur in the body), no alias rename, no dead store, no constant holder, no local
array, no aggregate merge, no width-cast games, no linker-script reorder.

## T6 naming-announces-intent
No name in the diff announces coercion intent. Names are `scr`, `ptr`, `p68`,
`ac_base`, `dp`, `i`, `j`, `sp_off`, `sp_inner`, `data_off`, `temp`, and the
labels `inner` / `end` — all describe what the object or label IS. There is no
`pad`, `dummy`, `unused`, `spill`, `slack`, `_buf` or `tail`, and every named
object is both written and read.

SANCTIONED-FAMILY-CLAIMS: none.
  No construct in this diff is a coercion construct, so no sanctioned-family
  carve-out is claimed and no /* FAKE */ annotation is owed. For the record, the
  two spellings a reviewer is most likely to question are ordinary-C precedented
  in-tree and in SOTN master:
  - goto-formed counted loop: `src/code6cac.c:2143` (COMPLETED-C func_800206B0,
    same TU) and `src/code6cac.c:2224` (COMPLETED-C func_80021280);
    SOTN master `docs/reference/sotn-construct-index.md:1015`
    (`src/dra/5F60C.c:579` — `loop_check_equip_id_1:` with the goto back edge at
    line 582).
  - rotated loop-guard compare on the live counter: producer #1 of
    `.claude/rules/phantom-slot-frame-lever.md:37`, exhibit func_8003DBE4,
    in-tree instance `src/code6cac_c2.c:1325`. That rule is a diagnosis recipe
    and explicitly not a sanction, and the 2026-08-20 Judge examined this exact
    lever on the s4 candidate and ruled it fine with no annotation owed.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.

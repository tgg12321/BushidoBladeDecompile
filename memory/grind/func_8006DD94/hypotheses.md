# Hypothesis ledger — func_8006DD94

## s1 (recon, 2026-09-10) — floor 117 -> 8 ordinary-C, 0 with one unclassified construct

H1 — "the data model needs no declaration change; D_800A374C's existing `extern s32` plus
the %hi/%lo (sdata_exclude) shape is already correct for this function."
mechanism: maspsx --sdata-exclude keeps the symbol out of the $gp window.
probe: wrote the sibling-idiom body against that declaration and measured sandbox.
result: CONFIRMED — the function reaches score 0 with the declaration untouched.

H2 — "the stack descriptor this function builds is the 0x2C-byte S69E18/EnvA shape the
rest of src/text1b.c uses."
mechanism: frame = ALIGN8(vars)+ALIGN8(args)+ALIGN8(gp_regs) (mips.c compute_frame_size);
cc1 prints get_frame_size() as `vars=` in the .frame comment.
probe: compiled with S69E18 (0x2C) and with a 0x34 widening, read cc1's .frame line and
the sandbox score both times.
result: KILLED (instance). 0x2C gives vars=56 / rect at sp+0x48 / score 23; the target
needs vars=64 / rect at sp+0x50. The 0x34 layout gives vars=64 and score 8. The widening
must live in a function-local typedef — widening the shared S69E18 regressed COMPLETED-C
func_8006BB68 from 0 to 17.

H3 — "the target's 7th callee-saved register ($s5 = 0, passed as func_8006E480's second
argument) is reachable from a literal 0 or from a plain s32 local holding 0."
mechanism: loop.c move_movables hoists loop-invariant SETs whose destination is a pseudo;
a constant argument is expanded directly into hard register a1, and an s32 local holding 0
is constant-folded by cse (which runs before loop) back into that same form.
probe: four spellings compiled; cc1's .frame line and the a1 setup insn read out of cc1's
asm; sandbox run on the two that mattered.
result: KILLED (instance). literal 0 -> frame 112 / gp_regs 6 / score 8; `s32 semi = 0;`
before the loop -> identical (112/6, `move a1,zero`); the same declared inside the loop
body -> 112/6. Only a narrow-typed (`s16`, `u8`) or pointer-typed zero-holder reached
frame 120 / gp_regs 7, and `s32 *clut = NULL;` passed as `(s32)clut` measured score 0.

## OPEN — the only thing between this function and COMPLETED-C

Every construct except one is ordinary C and byte-exact (score 8 without it, 0 with it).
The construct is `s32 *clut; clut = NULL;`, read exactly once and cast to s32 as
func_8006E480's second argument. It has no observable effect over the literal `0`
(cheat-checklist T1), so it reads as the sanctioned "constant-holder / dead scalar local"
family (.claude/rules/named-local-fake-exception.md) — which mandates a /* FAKE */
annotation AND a demonstrably-spent modality ladder. Session 1 cannot claim the ladder,
and a Judge FAIL at FINAL CALL would lock the body permanently, so s1 filed a
ruling-request rather than a candidate-ready. The full measured table is in evidence.md.

Next sessions, in order of expected value:
1. Act on the ruling. If the constant-holder is granted, annotate and submit
   memory/grind/func_8006DD94/candidate.c verbatim — bytes are already proven at 0.
2. If it is refused, hunt an ordinary-C carrier for the same invariant zero. The
   mechanism is now known exactly (a pointer/narrow-mode pseudo surviving cse into
   loop.c), so the search is for a spelling where the pointer has a real semantic role —
   e.g. a `s32 *` that genuinely names the absent CLUT/second-texture operand of
   func_8006E480 (src/text1b.c:6172, `return (a0[0] & 0xFE1F) + (a0[1] << 7) + a1;`), or
   a re-typed prototype for that callee (the block-scope
   `extern s32 func_8006E480(s32, s32 *);` variant also measures 0 but collides with the
   file-scope prototype at src/text1b.c:5626).
3. Do NOT re-derive the body: the loop shape, the u8 colour triple, the s16 counter, the
   0x34 local EnvB typedef and the rect store order are all byte-confirmed.

## s2 (recon, 2026-09-10) — floor 8 ordinary-C -> 0 ordinary-C; the s1 open question is VOID

H4 — "s1's row is right: an `s32` local holding 0 is constant-folded by cse, so only a
narrow-mode or pointer-mode local can carry the loop-invariant zero into $s5."
mechanism claimed by s1: cse runs before loop.c and folds the SImode `(set pseudo 0)`,
leaving nothing for loop.c move_movables to hoist.
probe: recompiled `s32 semi; semi = 0;` and `s32 semi = 0;` on s1's OWN chassis
(v16/v17) and on the EnvA-named chassis (v14), sandbox each; then read the sliced
`.cse` and `.loop` RTL dumps of the winning form.
result: KILLED (instance). All three `s32` spellings measure sandbox 0, not 8. The `.cse`
dump shows `(insn 11 (set (reg/v:SI 77) (const_int 0)))` alive in the loop preheader
after cse; cse folds only the memory use. `.loop` shows the set already in the preheader,
so no move_movables hoist is involved. s1's mechanism story AND its measurement are both
wrong; the pointer-clothed `clut` construct it filed a ruling-request over was never
needed.

H5 — "the real requirement is only that the 0 reach the call through a NAMED LOCAL rather
than the literal, because a constant argument is expanded straight into hard register $a1
and no pseudo is ever live across the loop."
mechanism: expand emits the constant into the arg hard reg; with a local there is a
loop-spanning, call-crossing pseudo, and the allocator seats it in a call-saved register,
raising ALIGN8(gp_regs) 24 -> 32 and the frame 112 -> 120 (mips.c compute_frame_size).
probe: deleted the local entirely on the new chassis (v15_nolocal) and measured; measured
five different local TYPES; measured a cast literal `(s32)(s32 *)0` with no local.
result: CONFIRMED. No local -> 8. Cast literal, no local -> 8. Any local -> the frame is
right (s32 -> 0; u8/u16 -> 1; s8/s16 -> 2, the narrow ones paying one extension insn).

H6 — "the zero has a truthful semantic identity — it is the semi-transparency mode — so
the matching body needs no zero-holder, no pointer clothing and no FAKE annotation."
mechanism: not a codegen mechanism; a data-model reading. func_8007352C's decompiled body
(src/text1b.c:6710-6805) names the descriptor EnvA and hands offset 0x10 to
`SetSemiTrans` (:6790); func_8006E480 (:6106) builds a getTPage word whose abr
(semi-transparency) bits 5-6 come from its second argument.
probe: renamed the local `semi`, gave it BOTH consumers (`s.semi = semi;` and the
func_8006E480 argument), renamed the EnvB fields to EnvA's, measured sandbox and ran
verify-oracle.
result: CONFIRMED. sandbox 0 (117/117), verify-oracle build_sha1 ==
62efab4f73f992798c43e8c730aa43baa10bb4fa. Submitted as candidate-ready, ordinary C, with
no sanctioned-family claim (self_vet.md).

## OPEN — nothing. The function is bytes-proven in ordinary C.

The s1 "OPEN" block is VOID: it rested on H4, which is now killed by measurement. If the
Judge should nonetheless FAIL the named `semi` local, the next axis is NOT the pointer
form (strictly worse, already Judge-FAILed 2026-09-10 05:24) but the question of whether
the two-read shape or the one-read shape is preferred — both measure 0, and the two-read
shape is the one with a truthful reading for every read.

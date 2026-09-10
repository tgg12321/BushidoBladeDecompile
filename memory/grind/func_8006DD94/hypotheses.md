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

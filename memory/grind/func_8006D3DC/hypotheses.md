# Hypothesis ledger — func_8006D3DC

## s1 (2026-09-14, recon) — function MATCHED; all hypotheses resolved

H1 CONFIRMED — func_8006D3DC is the 6-entry analogue of its in-file twin func_8006DD94,
   and the twin's body transplanted with (6 iterations, q = *(arg0[1]+0x38) indexed q[i],
   no func_8006D808 tail, four colour arms) reaches distance 27 of 126 in one step.
   Mechanism: shared descriptor type + shared func_8007352C/SetDrawMode/AddPrim idiom.
   Probe: hand-written body A, sandbox --disable all = 27.

H2 CONFIRMED — the target's `addiu $s2,$zero,0x40` is a source-level named constant of
   the FIELD's type (u8), not a loop.c hoist of a literal. Mechanism: the literal form
   puts the constant in a QImode pseudo inside a conditional arm, which loop.c does not
   hoist (text1b.loop dump insn 170); a `u8` local is initialised in the preheader by
   construction and the three `sb` reads consume it with no truncation copy.
   Probe: A (literal) 27 -> B (`s32` holder) 22 -> C (`u8` holder) 10.

H3 KILLED — the `s32`-typed holder spelling `s32 dim = 0x40;` reaches distance 22 and
   leaves a `move v0,s5` QI-truncation copy ahead of the `sb` triple that the target does
   not have.
   kill_scope: instance. measured_on: HEAD chassis 2026-09-14, no FAKE constructs
   present, body B (full 126-insn body, all four colour arms spelled).

H4 CONFIRMED — the pre-loop `s.has_color = 1;` is part of the original descriptor-init
   block, not a coercion carrier: it emits target bytes 8006D420 + 8006D428, and the
   second-order sibling func_8006BB68 initialises the same descriptor slot (offset 0x28)
   in its own pre-loop init block. Probe: C (10) -> D (6), and the two added instructions
   are byte-exact matches at those addresses.

H5 CONFIRMED — the final 6-point residual was first-pass scheduler tie-break order among
   three mutually independent `sw $sN` / `init $sN` prologue pairs, and that order tracks
   the source order of the initialisers. Target order is semi(s5), i(s1), dim(s2).
   Probe: order dim,semi,i = 10; semi,dim,i = 4; semi,i,dim = 0.

No open frontier: sandbox --disable all = 0 and the full build SHA1 equals the oracle.

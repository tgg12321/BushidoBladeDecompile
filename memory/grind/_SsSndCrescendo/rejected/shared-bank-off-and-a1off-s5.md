# Rejected — caching BOTH `bank_off` and `a1_off` as separate locals (s5, permuter modality)

**Form:** extended the s4 `shared-bank-off` chassis (`rejected/shared-bank-off-s4.md`,
score 139/200/200) by ALSO hoisting the `(s16)a1 * 0xB0` product into its own
`s32 a1_off` local at top of function, and rewrote every clear site as
`*(s32 *)((u8 *)&_ss_score + bank_off + a1_off + 0x98) &= ~0x10;` (both
cached components reused, only the `+0x98` immediate added fresh).

**Measured:** `sandbox _SsSndCrescendo --disable all` — score **143**,
build_insns **184** (UNDER target_insns 200 this time — the opposite
direction from every earlier variant, which all landed >= target). Worse
than both the banked 130 form and the s4 139/200/200 form. Reverted.

**Why this was tried:** objdump-diffed the s4 chassis's own permuter
target.o vs base.o (tmp/perm_crescendo_s4/{target,base}.dis) instruction
by instruction. Target does NOT recompute `(a1<<16)>>16` sign-extend +
the ×0xB0 chain fresh at the top like a naive read would suggest is
"the same as our s4 form" — it materializes the bank pointer
`_ss_score + bank_off` into a dedicated register (s3 in target, held for
the WHOLE function) AND separately materializes `a1 * 0xB0` into another
dedicated register (s2), spilling both to the stack early
(`sw s3,52(sp)` / `sw s2,48(sp)`), then at the FIRST use recombines them
(`lw v1,0(s3); addu s0,v1,s2`) to form `base`. This looked like strong
evidence that C-level caching of BOTH sub-expressions (not just
`bank_off`) would match target's register reuse at the clear sites too.

**Why it's wrong / what the measurement disproves:** GCC's own CSE
apparently uses the `a1_off` local far MORE aggressively when it is a
named C variable than when it is register-materialized incidentally as
part of one fused expression — build_insns dropped from 213 (banked
130 form) through 200 (s4 form) to 184 here, i.e. GCC eliminated 16
*more* recomputations than target's own asm shows at the clear sites.
This means target's own asm, despite spilling `s2` (a1_off) to the
stack early, still does NOT reuse it at every clear site the way naming
an explicit `a1_off` local makes GCC do — the register materialization
of s2 in target's asm is a REGISTER-ALLOCATION artifact of computing
`base` once at the top (s0 = bank_ptr-deref + a1_off), not evidence
that a1_off itself is a reused named quantity at the LATER clear sites.
Those clear sites may still re-derive a1's ×0xB0 product fresh in
target (consistent with H3's already-CONFIRMED "clear sites re-derive
fresh" finding) — the visible s2 register at the TOP of target's asm is
just the ONE place base is built, not evidence of a general a1_off
caching idiom threaded through the whole function.

**Kill scope:** instance, this chassis, s5. Caching `bank_off` alone
(the s4 form, 139/200/200) remains the best insn-count-exact spelling
found; caching `a1_off` in ADDITION overshoots the fold in the wrong
direction. The banked 130-score form (inlined single expression, no
named `bank_off`/`a1_off` at all) remains the ledger's best sandbox
score despite not being insn-count-exact.

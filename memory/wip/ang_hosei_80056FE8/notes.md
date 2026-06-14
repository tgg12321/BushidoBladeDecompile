# ang_hosei_80056FE8 (text1b.c) — WIP, floor 10, BLOCKED (RA tie)

## TL;DR (2026-06-14)
40-insn angle-correction lookup. HEAD matched only via a forbidden
`register s32 partial asm("$5")` pin + `asm volatile("")` scheduling barrier.
Removed both → clean candidate, floor sandbox 10 / build_insns 42 (target 43).
Full build SHA1 mismatch (650213f4...) confirms a real register-allocation
gap, NOT a masking artifact. No pure-C lever closed it this session.

## The gap
Target: `base = a3*40` lives in **$a1**, `var_v0` in **$v0** throughout; then
`addu a1,a1,v0` computes `partial = base+var_v0` BEFORE the final
`lh v0,1034(v0)` (so the lh's load-delay slot is a `nop`).
Mine: `var_v0`'s test `lh a1,94(a0)` claims **$a1** first, so `base` is parked
in **$a3**; the partial-add then fills the lh delay slot → build is 1 insn
SHORT (42 vs 43). It's a `base`<->`var_v0` register swap (masked-invisible in
the sandbox score) plus the delay-slot-fill consequence.

## Resume steps
1. Paste candidate.c; confirm sandbox 10.
2. Push `var_v0` into $v0 and `base` into $a1 in pure C. The masked sandbox
   CANNOT see the a1/a3 choice (scores 10 either way) — verify ONLY via the
   FULL build SHA1.
3. Likely modality: decomp-permuter (tiny fn, pure RA/scheduling residual).

## Ruled out (do not re-derive)
- Dropping the `var_v0 =` test assignment: floor 14 (target reuses the value).
- Computing base late / inline: floor 27 (multiply moves past the load).
- Addition reordering (base+=, partial+=0x12C, group const): floor 15-16.

## Pointers
- `.claude/rules/register-alloc-pure-c.md` (Lever A; pins/barriers forbidden)
- Same masked-10 RA-coupling class as this batch's other backlog items.

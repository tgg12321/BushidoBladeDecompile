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

### 2026-06-22 — 10 more structural variants, all stay at masked 10 / build 42 insns
- No `partial` intermediate (inline `base+var_v0` in return): same alloc.
- `var_v0` declared FIRST (lower LUID): same alloc.
- Remove `a3` intermediate (inline def): same alloc.
- `var_v0 += base; return var_v0 + lookup + 0x12C;`: DIFFERENT alloc (base→$a2, var_v0→$v1), still 42.
- `s32 partial = base + var_v0; s32 lookup = ...; return partial+lookup+0x12C;`: same alloc as canonical candidate.
- Full accumulator (`var_v0 += base; var_v0 += lookup; var_v0 += 0x12C; return var_v0;`): DIFFERENT alloc (base→$a2, var_v0→$v1, lookup→$v0); still 42.
- Same-var split-init (`s32 partial = a3 * 40;` … `partial += var_v0;`): DIFFERENT alloc (partial→$a2, var_v0→$v1); still 42.
- `return (base + lookup + 0x12C) + var_v0;` (var_v0 as LAST addend): addiu 0x12C fills lh slot; still 42.
- `register s32 var_v0;` (storage-class hint, NOT asm-pin): same alloc; NOTE: cheat_asm_stripped counter increments → dropped.

**Pattern:** every variant produces the same TOTAL insn count (42) but with DIFFERENT register choices. Masked sandbox cannot distinguish — all score 10. The wall is the FULL coupling `var_v0→$v0 AND base→$a1 SIMULTANEOUSLY` — only then does the partial-add `addu $a1, $a1, $v0` become forced-before the `lw $v0, 0($a0)` (since the lw would clobber live var_v0 in $v0). When base is in $a2/$a3, the addu doesn't conflict with the lw's $v0 destination, so the scheduler is free to put it in the lh delay slot.

## Pointers
- `.claude/rules/register-alloc-pure-c.md` (Lever A; pins/barriers forbidden)
- Same masked-10 RA-coupling class as this batch's other backlog items.

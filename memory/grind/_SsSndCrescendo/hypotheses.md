# Hypotheses — _SsSndCrescendo

## Frontier (live, untested)

### H1 — outer `unk98` (0xA0) guard is a genuine BB2/4.1 addition over SOTN's cres.c
**Statement:** BB2's build wraps SOTN's unconditional unk42-sign logic in
`if (--score->unk98 >= 0) { <unk42 sign logic> } ` with the negative arm
skipping straight to the tail (clear 0x98 bit + final SpuVmGetSeqVol).
**Mechanism:** not a GCC codegen artifact — a real control-flow difference
between BB2's linked LIBSND build and sotn-decomp's reference build (see
hunt report: CRES was one of the modules "fixed after 4.0" for 4.1).
**Probe:** write C with this outer guard, using the offset mapping in
evidence.md (0xA0=unk98, 0x4C=unk42, 0x4A=unk40, 0x98=unk90 bit),
`sandbox _SsSndCrescendo --disable all`, read the delta.
**Status:** UNTESTED — next session's first move.

### H2 — call signature divergence: `func_80087770`'s 4th arg is a literal `1`, not SOTN's literal `0`
**Statement:** every `jal func_80087770` (the `SpuVmSetSeqVol` analog) in
the target passes `a3 = 1` (via `addiu a3,zero,1`), where cres.c's
`SpuVmSetSeqVol(..., 0)` always passes 0 as the 4th arg. This is either a
genuine 4.1 API change (extra flag) or `func_80087770` is NOT exactly
`SpuVmSetSeqVol` (hunt report already flags `func_80087770` as "WEAK" ID,
"VM_VOL/VM_SEQ region"). Need func_80087770's own signature/body (if
matched) or its callers elsewhere in main.c to pin down what the 4th arg
means before transcribing calls.
**Mechanism:** N/A — this is an object/API-identity question, not codegen.
**Probe:** `grep -n "func_80087770" src/main.c` for other call sites +
check if func_80087770 itself is COMPLETED-C (its body would show what the
4th param does).
**Status:** UNTESTED.

### H3 — `_SsVmGetSeqVol` (`SpuVmGetSeqVol` analog) writes through 2 output shorts at `sp+0x10`/`sp+0x12`, and the final call at the tail writes directly into the struct (`s0+0x5C`/`s0+0x5E`) instead of locals
**Statement:** SOTN's tail call is `SpuVmGetSeqVol(..., &score->unk78, &score->unk7A)` — writes straight into the struct fields, matching BB2 asm's final `_SsVmGetSeqVol(..., s0+0x5C, s0+0x5E)`. So 0x5C/0x5E = unk78/unk7A. This is CONFIRMED already by direct asm read (not a hypothesis — recorded here so the next session doesn't re-derive it): the two mid-function calls use `sp+0x10/0x12` stack temporaries (matching SOTN's `voll`/`volr` locals), the LAST call writes directly to the struct. No probe needed; fold into the H1 candidate write-up directly.
**Status:** CONFIRMED by static read, not yet compiled/measured.

## Not yet explored (queue for later sessions if H1 doesn't fully close it)

- Exact shape of the `((voll+1) <= (voll+unk40))` comparison vs asm's
  `slt`+`bnez` sequence at 0x842E8-0x8430C — need to check whether BB2's
  version keeps the same comparison or (given the outer-guard divergence)
  also restructures this inner compare.
- Whether `_ss_score[a0][a1].unk90 &= ~0x10` in the 3 places should be
  written as struct-field clears (via `score->unk90 &= ~0x10;` reusing the
  already-live `score`/`s0` pointer) or re-fetched from `_ss_score` fresh
  each time (asm re-loads `0(s3)` + recomputes `s2+v0` at the final tail
  clear at label `.L800844AC`, suggesting `score` pointer may NOT be live
  there — could be a second/different channel pointer; needs a careful
  read of which base each of the 3 clears uses. NOT yet checked — first
  clear uses `v0` computed at 0x843A0 from `s3`+`s2` fresh; the asm-labeled
  tail at 0x800844AC ALSO recomputes fresh from `s3` — worth checking if
  these two "clear 0x98" sites are actually the SAME physical channel or
  potentially a DIFFERENT one (e.g. next channel / a related channel) —
  this determines whether it's `score->unk90 &=~0x10` (simple) or requires
  separate addressing.

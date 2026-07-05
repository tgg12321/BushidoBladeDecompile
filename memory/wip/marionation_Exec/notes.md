# marionation_Exec — WIP (2026-07-05 session-7: register rotation REACHABILITY PROVEN)

## TL;DR (read first)
The honest baseline is still `candidate.c` = **masked 30** (m2c-tail, no cheats). This
session PROVED the target callee-saved allocation is reachable and pinned the exact
priority targets, but the clean (cheat-free, byte-matching) form is still open.

**Key correction to the model:** `masked` normalizes register names OUT, so the
masked-30 floor is **scheduling diffs, NOT the register rotation**. The rotation is a
RAW-only diff. So closing needs BOTH: (a) the register rotation (raw), and (b) ~4
independent scheduling residuals (the masked-30). Prior notes conflated these.

## The register mechanism (fully quantified, faithful debug cc1)
allocno pri ≈ FREQ×nrefs/livelen; FREQ = loop-exec weight of the ref sites
(status inner-poll=30000; arg1/i1494/i1496=20000; arg0/i1495/tbl=10000). All 8
s-pseudos mutually conflict → allocation is **sequential by priority** → pri order
== s0..s7 order. Honest baseline ledger (pri desc): arg1 952 > i1494 933 > i1496 666
> arg0 256 > i1495 202 > tbl 197. TARGET needs: i1494 > i1496 > arg1 > tbl > i1495 > arg0.
Three inversions: (i1496>arg1), (tbl>i1495), (i1495>arg0).
**Pre-reload greg ledger == final callee-saved allocation** (verified via reconcile.py) —
but VERIFY register claims against FINAL objdump; the ledger block-matcher can mislead.

## BREAKTHROUGH (diagnostic): param-staging reaches ALL 8 target regs in FINAL asm
`tmp/vM3.c` (saved: rejected/staging-diagnostic-all-callee-saved-correct.c):
candidate + `s32 aa; u8 *hold;` + `aa = a0; hold = a1;` placed LATE (after D_800F19C0)
+ tbl-def-AFTER-idx (T1) + use aa in the a0-test + hold in both copy arms.
→ FINAL objdump: i1494=s2, i1496=s3, arg1=s4, tbl=s5, i1495=s6, arg0=s7, status=s0,
saved=s1 — **all target-correct** (first time). masked stays 30 (masked ≠ registers).
Mechanism: staging LATE gives aa/hold short live ranges (low pri) → arg0→s7, arg1→s4.

## Why vM3 is NOT the answer (two blockers)
1. **Prologue placement.** Target saves params AT the prologue (`move s7,a0`@insn2,
   `move s4,a1`@insn4) — the ORIGINAL has a0/a1 DIRECTLY at s7/s4 (params low pri
   naturally). Staging emits those moves MID-preamble → bytes differ, insn count
   176 vs 179. Staging can't produce the prologue save (needs the param itself at
   s7, i.e. low pri WITHOUT a copy — the unsolved crux).
2. **Cheat-questionable.** `aa=a0`/`hold=a1` are purposeless param copies whose only
   effect is RA — same family a fresh reviewer FAILs. (`hold=a1` alone was called
   "honest ~m2c var_a1" in old notes, but a0-staging is new and must clear review.)

## THE OPEN CRUX
Find the NATURAL C structure that gives **arg0 lowest priority** (ll 78→>101, so it
saves at prologue to s7) and **arg1 3rd** (ll 84→~120), WITHOUT staging copies. arg0
is used at exactly ONE site (the `if(a0)` loop test) yet flow gives ll=78; extending
it honestly is the key.

## m2c RESULT (2026-07-05, now WORKING — junctioned main's tools/m2c into worktree)
Run: `wsl bash tmp/run_m2c.sh` (m2c -t mipsel-gcc-c --valid-syntax -f marionation_Exec).
Faithful structure revealed → tmp/vMC.c is the direct transcription. Findings:
- **arg0 is used DIRECTLY** (`if (arg0 != 0)`), NOT staged. => the session-7 `aa=a0`
  staging diagnostic is UNFAITHFUL (confirmed cheat, not the original). Drop it.
- arg1 gets TWO copies (var_a1 block1 / var_a1_2 block2); block1 checks arg1, block2
  checks the copy — candidate.c ALREADY matches this (dst/dst2). Not the lever.
- Outer loop = compound `||` timeout condition + `v0_2` flag + `v0=-1` default gated
  by `if(v0_2==0)`. Target holds all 3 idx regs (s2/s6/s3) => STORED pointers right,
  m2c's field-style is a decompiler artifact (confirms candidate).
- **Direct transcription vMC = masked 49 (WORSE than candidate 30)**, arg0→s5 arg1→s1.
  The candidate's goto structure beats raw m2c; the compound-`||` does NOT fix arg0.
=> m2c did not unlock the match. candidate.c (masked 30, goto structure, stored
  pointers) remains the best honest base. arg0→s7 still unreproduced by honest C.

## Scheduling residuals (the masked-30, independent of registers)
1. do_timeout pair-swap: `sll a0; addu v0,v0,tbl` order (idx 56/57).
2. saved-stage: target `lbu v0; andi s1,v0` vs ours `lbu s1; andi s1,s1` — try
   staging saved through v0 (`v0=*D_800A147C_2; saved=v0&3`, sanctioned reuse).
3. copy-loop a0/a1 (src/dst) allocation swap in block1.
4. prologue param-save order (tied to the crux above).

## DEAD ENDS confirmed this session (don't repeat)
- tail-restructure (shared epilogue): raw↓ but BREAKS staging (masked 51).
- do-while(a0==0) outer loop: refs DOUBLE (loop-depth weight), masks materialize
  as register-and, +2 insns (masked 48). Confirms old note.
- staging placed FIRST: aa/hold get long ranges → high pri → allocation reverts.
- branch-sense flip, faithful-preamble reorder: NO-OPS (GCC canonicalizes/reschedules).
- tbl load moved into loop / do_timeout: overshoots (freq doubles) or drops to t-reg.
- T1 needed for tbl/i1495 order, but T1 makes setup order (i1494-first) differ from
  target (tbl-first) — a tension staging doesn't resolve.

## Tooling (all in tmp/, worktree bb2-work-marion; regenerate from this note if lost)
- tmp/gccdbg/cc1 = instrumented debug cc1 (copied from main tmp/gccdbg; VERIFIED
  byte-identical codegen to canonical build/cc1 via verify_dbgcc1.py).
- tmp/mar.sh = WSL venv wrapper. Run: `wsl bash <abs>/tmp/mar.sh tmp/<tool>.py <args>`.
- tmp/probe.py <cand> = masked score + callee-saved mapping + allocno ledger.
- tmp/adiff.py <cand> = LCS-aligned objdump diff (ours vs target).
- tmp/genprobe.py = batch variant generator/prober. tmp/ledger3.py, reconcile.py.

## Next session
1. Apply candidate.c, confirm sandbox==30 (honest floor). vM3/staging = confirmed
   UNFAITHFUL cheat (m2c proves arg0 is direct); do NOT resume from it.
2. m2c is junctioned (tools/m2c) + tmp/run_m2c.sh works; the structure is fully
   extracted above — no need to re-run unless probing a variant.
3. The crux is narrow: arg0 (used once, direct) has flow ll=78 in EVERY structure
   tried (candidate, m2c, vMC) but target needs >101 (→s7). Next ideas UNTRIED:
   (a) directed permuter from candidate.c (clean base, insn-count-matched) —
   automated search for the livelen perturbation; vet finds for cheats.
   (b) instrument WHY flow gives arg0 ll=78 (BB2 flow.c reg_live_length walk) vs
   what target's structure would give — the gap is one specific CFG/live-set detail.
4. Knock down scheduling residuals independently (saved-stage via sanctioned v0 reuse).
Infra note: setup_worktree.ps1 should junction tools/m2c (main has it; worktrees don't).

# CDREAD triple candidate — puts + cb_read + cb_data (Closer session 8, 2026-07-10)

**PROVEN BIT-EXACT: 0/263 words differ vs disc/SLUS_006.63 over
0x80082000..0x8008241C** (tmp/closer/link_sim.py, zero masking, all relocs
resolved incl. cross-TU via build/bb2.map fallback). Candidate =
`cdread_triple.patch` (apply to src/system.c with `git apply`).

Closes queue item **tslTm2LoadImage_2** (= LIBC2 puts, dist 261) by replacing
its whole-region asmfix splice with three honest C functions:
- `tslTm2LoadImage_2` = **puts** (putchar loop + "<NULL>" fallback; self-decomp,
  20/20 insns exact on first compile)
- `static D_80082050` = **cb_read** (cdread.c v1.86: saved-result ptr
  D_800A1504, tsl-mode DMA-chain split, deferred advance)
- `static D_80082320` = **cb_data** (tsl data-DMA-complete callback) — its
  26-insn-deep "sunk prologue" reproduces NATURALLY from plain C through our
  cc1+prologue_fix+maspsx pipeline (the feared ASPSX-scheduling wall does not
  exist here).

## Load-bearing codegen findings (measured + cc1psx-confirmed)

1. **Volatile addressing forms in GCC 2.7.2** (THE key mechanism, reusable
   across the whole BIOS/CDREAD family):
   - plain global / de-volatile cast `*(s32 *)&X` → macro form `lw $r,X`
     (combine folds the address into the non-volatile mem);
   - volatile-DECL zero-offset access `X` → macro form `lw $r,X` with
     `#.set volatile` (pinned order, re-read semantics);
   - volatile CAST `*(volatile s32 *)&X` OR volatile STRUCT member
     `S.member` (sym+K, first access) → **la-form 3-insn**
     (`la $r,X; lw $r,0($r)`) — combine refuses to substitute the address
     into a volatile mem; cse only folds later same-base accesses.
   - cc1psx emits the IDENTICAL la-form from the struct spelling → Sony's
     source at the macro-form sites was per-member volatile globals, NOT the
     struct. The exe's two la-form sites (cb_read pos-compare + second
     tslmode read) are reproduced with `D_800A14D0.pos` / `D_800A14D0.tslmode`
     struct spellings (first-struct-access-per-BB la).
2. **Mixed volatility breaks target ordering**: plain reads commute across
   volatile accesses in sched — the pinned target order needs the member
   reads uniformly volatile (hence decl-level volatile, not per-site casts).
3. **saEft00Add byte-parity compensation** (in the patch): decl flip to
   volatile is compensated site-for-site — HEAD-plain sites become
   `*(s32 *)&X` de-volatile casts; HEAD `*(volatile s32 *)&X` cast sites keep
   their exact text (same emission on a volatile decl). Verified: cc1 output
   for func_800826CC / func_800827D0 / saEft00Add / saEft00Add_sub is
   IDENTICAL to HEAD modulo .L renumbering (tmp/closer/head_cmp.sh; no
   literal-.L rules exist for system.c — saEft00Add's are {lbl#N} regex).
4. **CdGetSector/CdGetSector2 wrappers** (func_80080620 /
   Vu0SetLightColMatrix_80080640) converted from void(void) register-
   passthrough accidents to honest (madr, size) forwarding — byte-neutral
   (session-2 CdSync/CdReady precedent).

## Why NOT claimed this session (two blockers, both operator-gated)

1. **Volatile grant needed** for the 12 per-member cdread symbols
   (proposal §3 in memory/closer/volatile-grant-proposals.md). Without it the
   sandbox strips the volatiles → gate can't read 0.
2. The asmfix splice `tslTm2LoadImage_2: replace_with_asmfile` must be
   stripped in the SAME commit the bodies land (my statics + the splice
   content would otherwise both emit ≈ +240 duplicate words). Same driver
   mechanics as func_8008AF9C (S_SCA session-6 precedent).

## Replay procedure (next session, after grant)

1. `git apply memory/closer/candidates/cdread_triple.patch`
2. Re-prove: `bash tools/wsl.sh 'bash tmp/closer/triple_prove.sh'` → expect
   `0 differing`. (If tmp/ was cleaned: the script + patched link_sim.py are
   reproduced trivially — link_sim needs the bb2.map fallback + splat-name
   fallback + region-scoped .text symbol resolution added 2026-07-10.)
3. Claim tslTm2LoadImage_2 in the outcome; driver strips the splice rule and
   rebuilds (sandbox self-heals to 0 like func_8008AF9C).

# Evidence bank — func_80062020

- s1 (recon): honest pure-C floor = 10 (sandbox --disable all), clean pure C
  (0 pins, 0 rules, 0 dead vars). LOOP REGION matches target 100%. Prior ledger
  "12" was pin-aided (register-asm pins NOT sandbox-stripped); naive pure-C = 20.
- s1 LEVER: indexed-source read `*(s32*)((u8*)arg0+ofs+K)` (fixed base + ofs index)
  makes GCC use ONE walking source giv (matches target); walking `a0[K]` -> 2 giv, floor 20.
- s1: all 10 residual diffs are the epilogue. Target uses raw index (12*count) TWICE
  (base pointer for cols b,c + %hi/%lo(1198)+index for col a). No uniform pure-C
  spelling reproduces this partial CSE; only the rejected dual-spelling does, and even
  that doesn't close (reg-alloc index->v0 not v1; the 4 pins were what forced v1).
- D_800F1198/119C/11A0 have NO reader in src (consumer is asm-only) -> no external
  object-model evidence for whether col a (flag word, bit0 tested by loop) is a
  genuinely separate object from cols b,c.

- WIP rejected_form: epilogue dual-spelling: p[0] written as *(s32*)((u8*)&D_800F1198+i12)=0 alongside p[1]/p[2] - codegen steer, FAILed retro-audit

- == imported from memory/wip notes.md ==
# func_80062020 (text1b.c) — rejected match, sound loop-fix preserved

## TL;DR
orch3a matched this (commit `dfb9e9ac`, branch work/orch3a) but the fresh
adversarial retro-audit (audit-orch3a, 2026-06-14) FAILed it: the epilogue
smuggled a **dual-spelling addressing-mode steer**. The LOOP-BODY fix is sound
pure C and retires the 4 `register asm("$N")` pins; only the 3-store EPILOGUE
needs a legitimate form. Resume from the loop-fix, rewrite the epilogue
CONSISTENTLY, re-measure. NOT reintegrated to main — func_80062020 is still
INCOMPLETE (pins present at HEAD).

## What's SOUND (keep)
Loop body — read source with the same byte offset `ofs` as the destination,
split-init the `i12` stride:
```c
ofs = 0;
do {
    t = *(s32 *)((u8 *)arg0 + ofs);       *(s32 *)((u8 *)&D_800F1198 + ofs) = t;
    t = *(s32 *)((u8 *)arg0 + ofs + 4);   i = i + 1;  *(s32 *)((u8 *)&D_800F119C + ofs) = t;
    t = *(s32 *)((u8 *)arg0 + ofs + 8);   *(s32 *)((u8 *)&D_800F11A0 + ofs) = t;
    ofs = ofs + 12;
    t = *(s32 *)((u8 *)arg0 + ofs);
} while ((t & 1) != 0);
```
Pure C, no pins, no rules. (`i12 = i+i; i12 = i12+i; i12 = i12<<2;` = i*12 is
sanctioned split-init accumulation.)

## What was REJECTED (do NOT reuse)
```c
p = (s32 *)((u8 *)&D_800F1198 + i12);
p[2] = 0;
p[1] = 0;
*(s32 *)((u8 *)&D_800F1198 + i12) = 0;     // <-- == p[0], re-spelled
```
The third store is `p[0] = 0` written as the full base-expression ONLY to force
GCC to emit a %hi/%lo split-address store (vs the full-address form used for
p[1]/p[2]). Same lvalue, two spellings, no semantic purpose -> codegen steer
(fails 6-test 1/2/3). Cheat-by-spelling, [[inline-asm-injection]]-adjacent.

## Resume
1. Apply the loop-body fix (sound).
2. Epilogue: try a CONSISTENT zero of the 3 words (`p[0]=p[1]=p[2]=0;`, or zero
   via the array base symbols in index order) and measure the honest distance.
3. If no consistent C reaches target's split/full addressing-mode mix, this is a
   genuine addressing-mode plateau — document it; do NOT re-introduce the steer.

Cheat reference (diff only): `git show dfb9e9ac` on branch work/orch3a.


- [s1] Honest pure-C floor = 10 (sandbox --disable all), clean form: 0 register-asm pins, 0 regfix/asmfix rules, 0 dead vars, no dual-spelling. build_insns 35 vs target 38.

- [s1] Loop region (func start .. 80062080) matches target 100% with the indexed-source lever; all 10 residual diffs are the epilogue (80062084..800620B4).

- [s1] Target epilogue: index 12*count in v1, base &D_800F1198 in v0, v0=index+base, sw zero 8(v0)/4(v0) for cols c,b, then separate lui at,%hi(1198);addu at,at,v1;sw zero,%lo(1198)(at) for col a (partial CSE, keeps index v1 live).

- [s1] col-0 (D_800F1198) is the list flag word (loop tests bit0); the epilogue zeros the terminator row's 3 cols.

- [s1] D_800F1198/119C/11A0 have no reader anywhere in src (consumer is asm-only) -> no external object-model evidence on whether col a is a separate object from cols b,c.

- [s1] Register placement (index->v1) and the col-a split are coupled: both stem from target double-using the raw index; the 4 pins in the abandoned src forced the v1 placement.

- [s1] Store order matters by 1 diff: reverse (p[2],p[1],p[0]) = 10; forward (p[0..2]) = 11 (target stores c,b,a).

## s2 findings (structural modality) — floor 10 -> 4

- [s2] **REGISTER-FLIP SOLVED (floor 10 -> 4).** REUSING the loop's `ofs` variable
  to hold the terminator index (`ofs = i*12` via split-init, instead of a fresh
  `i12`) biases RA to keep the index in v1 (target) instead of v0. `ofs` was the
  loop's byte-offset biv (v1); 12*count is also a byte offset -> semantically the
  same value, legitimate reuse. Result: the ENTIRE epilogue register allocation
  now matches target (index v1, base &1198 in v0, addu v0,v1,v0, cols b,c via
  4/8(v0)). Only col a's addressing remains. candidate.c updated to this form.
- [s2] With RA fixed, the residual score-4 is EXACTLY col a: mine `sw zero,0(v0)`
  (folds col a onto base pointer v0); target `lui at,%hi(1198); addu at,at,v1;
  sw zero,%lo(1198)(at)` (recomputes, keeps raw index v1 live) -> +2 insns +1 sw.
- [s2] Measured structural landscape at index-v1 (all pure C, 0 pins/rules):
    * single-object pointer p=&1198+ofs, store c,b,a (p[2],p[1],p[0]) = **4** (full CSE) <- clean floor
    * single-object pointer, store a,b,c (p[0..2])                 = 5
    * uniform 1198+displacement (ofs+8/+4/+0, no pointer var)      = 6 (full recompute, 3 lui)
    * three distinct symbols (11A0/119C/1198 + ofs)               = 6 (full recompute, 3 lui)
    * two-object: b,c via p=&119C+ofs (0/4), col a via 1198 symbol = **2** (col a MATCHES; only b,c anchor differs)
    * dual-spelling: b,c via p=&1198+ofs (p[2],p[1]), col a via *(&1198+ofs) = **0** (CHEAT, only closer)
- [s2] scan_hand_coded --single func_80062020 = tier LOW, score 0/8 (no S1/S2/S6
  strong signals). Per endgame-lock-disposition: canonical-asm REFUSED — the
  col-a partial-CSE is an ordinary GCC addressing/RA artifact, not hand-coded.
- [s2] **Distance-0 EXISTS but only via the same-lvalue dual-spelling cheat.** With
  the register flip fixed, the dual-spelling (rejected bank) now closes at 0
  (was 12 at s1). Every consistent/legitimate spelling gives full-CSE (4) or
  full-recompute (6); target's partial CSE (base pointer for b,c + separate
  %hi/%lo recompute for a) has NO non-steer pure-C form.
- [s2] The two-object flag(1198)/data(119C) split proves target's b,c "data view"
  is anchored at the FLAG's address (1198, disp 4/8), NOT at 119C -> b,c are the
  SAME interleaved struct-row {flag@1198,b@119C,c@11A0} as col a, so no genuine
  object separation reproduces the partial CSE. Frontier hyp #1 KILLED.

- [s2] Honest pure-C floor dropped 10 -> 4 this session via the `ofs`-reuse register lever (clean pure C: 0 pins, 0 rules, 0 dead vars). candidate.c updated.

- [s2] With RA fixed, the entire epilogue matches target through cols b,c; the sole residual (score 4) is col a: mine `sw zero,0(v0)` (folds onto base pointer) vs target `lui at,%hi(1198); addu at,at,v1; sw zero,%lo(1198)(at)` (separate recompute keeping raw index v1 live).

- [s2] Structural landscape at index-v1 (all pure C): single-object pointer c,b,a = 4 (full CSE, floor); pointer a,b,c = 5; uniform 1198+disp = 6 (full recompute); three-distinct-symbol = 6 (full recompute); two-object 119C-anchor = 2 (col a matches, wrong b,c anchor); dual-spelling = 0 (CHEAT, only closer).

- [s2] Distance-0 byte-match is PROVEN to exist but its only known spelling is the same-lvalue dual-spelling cheat (rejected bank). Every consistent legitimate spelling gives full-CSE (4) or full-recompute (6); target's PARTIAL CSE has no non-steer pure-C form.

- [s2] scan_hand_coded --single func_80062020 = tier LOW, score 0/8 -> col-a partial-CSE is an ordinary GCC addressing/RA artifact; canonical-asm refused per endgame-lock-disposition criterion 1.

- [s2] All three original frontier hypotheses resolved: #2 (register placement) CONFIRMED/solved -> floor 4; #1 (semantic object model) and #3 (combine fold) both KILLED with measurements.

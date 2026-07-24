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

# func_80089F3C (src/main.c) — WIP checkpoint 2026-08-06

**State: BELIEVED ALREADY COMPLETED-C. Blocked only on the build lock.**

Wiring: `asmfix.txt` `func_80089F3C: replace_with_asmfile "asm/funcs/func_80089F3C.s"`.
Recorded queue distance 20, verdict C. Body is a genuine draft (SpuSetReverbAttr-shaped).

## The finding — the recorded 20 is a measurement artifact, not a codegen gap

`engine score` compares OBJECT to OBJECT. Our object is compiled from C that accesses a
struct; the reference object's block came from `asm/funcs/func_80089F3C.s`, which names
splat's per-address symbols. Every one of the 20 differing instructions is that same
shape and nothing else:

```
ours   : lui at,0x0 ; sw s0,4(at)          HI16/LO16 reloc pair on D_800A2888, addend 4
target : lui at,%hi(D_800A288C) ; sw s0,%lo(D_800A288C)($at)     addend 0
```

The linker computes `AHL = (AHI<<16) + (short)ALO`, so `D_800A2888 + 4` and
`D_800A288C` resolve to the identical final word. The shipped executable carries no
relocations, so these cannot differ in the bytes the oracle checks.

## Proof that this accounts for ALL 20 (offset -> symbol, counts match exactly)

| our offset | address | target symbol | our count | target `%hi` count |
|---|---|---|---|---|
| +4  | 0x800A288C | D_800A288C | 8 | 8 |
| +8  | 0x800A2890 | D_800A2890 | 2 | 2 |
| +10 | 0x800A2892 | D_800A2892 | 2 | 2 |
| +12 | 0x800A2894 | D_800A2894 | 4 | 4 |
| +16 | 0x800A2898 | D_800A2898 | 4 | 4 |

Total 20 = the entire residual. All six symbols are absolutes at their named addresses
(verified in `undefined_syms_auto.txt`). `D_800A2884` (addend 0 on both sides) shows no
diff, which is the control.

Body carries ZERO detector-flagged cheats (`volatile_cheat_count = 0`); the volatile
casts it does have are on `D_800A2CDC + 0x1AA` (SPU hardware registers) and are
type-level MMIO, not coercion.

## Resume point

Nothing left to derive. With the build lock free, run:
`retire func_80089F3C` -> `verify-oracle` -> `queue done func_80089F3C`.
`retire` has a full-build SHA1 gate with auto-rollback, so it is self-verifying: if the
reloc-neutrality reading is wrong it rolls back and the residual is real after all.

## Tooling note

`inverse_compose.py classify` CANNOT read these four `replace_with_asmfile` functions:
`<stem>.tgt.s` carries their target as a `glabel`/`endlabel` block of `/* off addr bytes */`
disassembly text, which `goalmap.asm_body` skips entirely, and even after normalizing
(`tmp/norm_tgt.py`) the two sides use different conventions (`subu $sp,$sp,144` vs
`addiu $sp, $sp, -0x90`), so it reports a fictitious PRE-RA multiset difference.
Use `tools/pairdiff.py <stem> <func>` instead — it diffs the SCORED object pair with the
engine's own masking, which is what the score actually measures.

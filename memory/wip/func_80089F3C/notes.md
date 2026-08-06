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

## DECISIVE buildless proof — `tools/relocsim.py main func_80089F3C`

Added 2026-08-06. Simulates the link for this function: applies the MIPS REL addend rule
(`AHL = (AHI<<16) + (short)ALO`; HI16 field = `%high(S+AHL)`, LO16 field = `(S+AHL)&0xffff`)
plus R_MIPS_26, to our sandbox object, then compares raw words against the target's.

```
318 ours / 318 target insns, 73 relocs applied
raw word mismatches BEFORE relocation : 75
raw word mismatches AFTER  relocation : 2
  [287] ours 0c000000  target 0c022a4a     jal md_game_check_change_main_mode_katinuki
  [293] ours 0c000000  target 0c022423     jal spu_WriteReg
```

**316 of 318 words are byte-identical once the link is applied**, and the 2 that are not
are `jal`s to C-defined siblings whose final addresses the linker assigns (our object has
them as unrelocated 0 + R_MIPS_26). Every one of the 20 SCORED diffs resolves to equality.
This is proof independent of the build lock; `retire`'s SHA1 gate is then the confirmation.

## Both sandbox configurations (per the AllocRobRmd / saEft00Add evidence bar)

```
sandbox --disable all                  -> 20   (318/318, rules_dropped 1, cheat_asm_stripped 77)
sandbox --disable all --keep-cheat-asm -> 20   (318/318, rules_dropped 1, cheat_asm_stripped 0)
```

**Note the mechanism differs from saEft00Add** and the difference matters. saEft00Add's
control scored **0** unstripped, which is what proved its residual was SIBLING cheat-strip
shifting a `static` symbol's section offset (decisions.md 2026-08-04 00:47 / 83b0e069).
Ours scores 20 in BOTH configs — the residual is invariant to stripping because it is not
sibling-induced at all: it is the spelling difference between our C (named global +
addend) and the reference block, which `asmfix replace_with_asmfile` sources from
`asm/funcs/`, where splat named a symbol at each exact address. Same artifact FAMILY
(link-neutral reloc addends counted by .o-vs-.o scoring), third distinct mechanism.
AllocRobRmd (96a07856) is a fourth case: 0 in both configs, no residual to rule on.

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

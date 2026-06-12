---
name: halfword-index-srl-sra
paths: ["regfix.txt"]
# broad src/*.c glob removed 2026-06-11: surfaced via codegen-technique-index
description: "A `subst \"srl\" \"sra\"` regfix on a halfword (s16[]) array index where GCC made the index shift logical but target has arithmetic. Cause: m2c's shift-mask-shift reconstruction `arr[((var>>1)&0x1FFE)>>1]` lets GCC's combine fold the redundant >>1/*2 round-trip and canonicalize the sign-masked arithmetic shift to srl. Fix: index a byte pointer so the byte offset is written directly — `*(s16*)((u8*)&arr + ((var>>1)&0x1FFE))` — which keeps sra and retires the rule."
metadata:
  type: reference
---

# `subst "srl" "sra"` on a halfword array index — fix with a direct byte-offset cast

## Symptom

A function carries a single regfix rule of the form:

```
func_XXX: subst "srl" "sra" @ <idx>     # GCC made the index shift logical; target has arithmetic
```

In the target, a signed value is right-shifted then masked and used as a
**halfword (`s16`) array index**:

```mips
sra  $v0, $v1, 1        # var >> 1  (ARITHMETIC in target)
andi $v0, $v0, 0x1FFE   # byte offset into an s16[] (mask is even -> *2 already folded in)
lui  $at, %hi(Tbl)
addu $at, $at, $v0
lh   $vN, %lo(Tbl)($at)
```

Your build emits `srl` (logical) where the target has `sra`. The mask
(`& 0x1FFE`, `& 0xFFE`, any mask that clears the sign bit) makes the two shifts
produce identical bits, so GCC is free to pick either — and it picks the
logical form.

## Why m2c's reconstruction triggers it

m2c reconstructs the s16 index as a shift-mask-**shift** chain:

```c
((s16 *)&Tbl)[((var >> 1) & 0x1FFE) >> 1]   /* m2c form -> emits srl */
```

The trailing `>> 1` (halfword offset → element index) combines with the array
access's implicit `* 2` (element index → byte offset). GCC's `combine` pass
simplifies the whole `(((var >> 1) & 0x1FFE) >> 1) * 2` round-trip back to
`(var >> 1) & 0x1FFE` — and **during that simplification it canonicalizes the
surviving inner arithmetic shift to a logical one** (the sign bit is provably
masked away). Result: `srl`.

## The fix — compute the byte offset directly

Drop the redundant `>> 1` / `* 2` round-trip. Index a **byte pointer** so the
byte offset is written literally and GCC never runs the shift through that
simplification:

```c
var_v0 = *(s16 *)((u8 *)&Tbl + ((var_v1 >> 1) & 0x1FFE)) * 3;   /* keeps sra */
```

`(var_v1 >> 1) & 0x1FFE` is now the final byte offset added straight to the base
pointer (mirroring the target's `addu $at, $at, $v0`). With no outer
`>>1`/`*2` for combine to fold, the arithmetic shift survives as `sra`, the
`andi 0x1FFE` stays, and the `subst "srl" "sra"` regfix becomes unnecessary.

## Confirmed case — func_8001BAE4 (code6cac.c, 2026-05-26)

Queue top, verdict C, pure-C distance 1, one regfix `subst "srl" "sra" @ 29`.
Rewriting `((s16*)&Judge)[((var_v1 >> 1) & 0x1FFE) >> 1] * 3` →
`*(s16 *)((u8 *)&Judge + ((var_v1 >> 1) & 0x1FFE)) * 3` made GCC emit `sra`
naturally. `retire` dropped the rule; full SHA1 == oracle. 100% pure C.

NB: the function's two signed-`/2` and `/4` corrections (`if (x<0) x+=3;` branch
form, etc.) already matched and were left untouched — only the masked index
shift was the problem. Don't convert those corrections to `x / 4`: GCC may emit
the branchless `srl 30; addu; sra 2` form instead of the target's bgez+addiu
branch form.

## Related
- [[strength-reduce-defeat]] — the inverse (forcing a sign-split GCC folds away);
  different lever (asm `negu`), this one is pure-C structural
- [[lost-codegen-insert-cheat]] — other single-rule opcode/instruction cheats

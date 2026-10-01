---
name: halfword-index-srl-sra
paths: [".claude/rules/halfword-index-srl-sra.md"]
description: "Masked s16[] index shift emits srl where target has sra: m2c's `arr[((v>>1)&0x1FFE)>>1]` lets combine fold the >>1/*2 round-trip. Write the byte offset directly: `*(s16*)((u8*)&arr + ((v>>1)&0x1FFE))`."
metadata:
  type: reference
---

# `srl` vs `sra` on a halfword array index — write the byte offset directly

## Symptom

Target right-shifts a signed value, masks it, and uses it as a byte offset into an `s16[]`:

```mips
sra  $v0, $v1, 1        # ARITHMETIC in target
andi $v0, $v0, 0x1FFE   # even mask: already a byte offset
lui  $at, %hi(Tbl)
addu $at, $at, $v0
lh   $vN, %lo(Tbl)($at)
```

Your build emits `srl`. The sign-clearing mask makes both shifts bit-identical, so GCC may pick
either.

## Cause

m2c reconstructs `((s16 *)&Tbl)[((var >> 1) & 0x1FFE) >> 1]`. combine folds the trailing `>> 1`
with the array's implicit `* 2` and, while simplifying, canonicalizes the inner arithmetic shift to
logical.

## Fix

```c
x = *(s16 *)((u8 *)&Tbl + ((var >> 1) & 0x1FFE)) * 3;   /* keeps sra */
```

With no `>>1`/`*2` round-trip for combine to fold, `sra` and the `andi` survive, mirroring the
target's `addu $at,$at,$v0`. Example: func_8001BAE4 (code6cac.c), distance 1 → 0.

Leave already-matching signed-division corrections (`if (x<0) x+=3;` branch forms) alone — `x / 4`
may emit the branchless form instead.

Related: [[u16-global-lhu-lbu-low-byte]] · [[register-alloc-pure-c]]

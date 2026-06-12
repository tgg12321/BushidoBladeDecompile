---
name: u16-global-lhu-lbu-low-byte
paths: ["regfix.txt"]
# broad src/*.c glob removed 2026-06-11: surfaced via codegen-technique-index
description: "A single regfix `subst` downgrading a gp-rel u16 global read from `lhu` to `lbu` (e.g. `lhu $3,%gp_rel(D_800A3578)`→`lbu`). Cause: the function's entry/dispatch read only consumes the low byte but a sibling read needs the full halfword (word>>8 + write-back), so the global can't be retyped u8. Fix: leave it u16 and read the low byte at the dispatch site via `*(u8*)&G` — emits lbu with the wide reads intact."
metadata:
  type: reference
---

# `subst "lhu" "lbu"` on a u16 global dispatch read — read the low byte explicitly

## Symptom

A function carries a single regfix `subst` that downgrades the **opcode** of a
gp-relative global read from `lhu` to `lbu`:

```
func_XXX: subst "lhu\s+\$3,%gp_rel(D_800A3578)" "lbu	$3,%gp_rel(D_800A3578)" @ 1
```

The global is declared `u16` (so a plain read emits `lhu`), but the **entry /
dispatch** read of it only consumes the low byte — target reads it as `lbu`. The
variable can't simply be retyped `u8`, because a **sibling read** in the same
function genuinely needs the full halfword (e.g. a `(word >> 8)` high-byte check
plus a `word + 1` write-back that uses the high byte as a counter/flag).

## Cause

A `u16` lvalue read compiles to `lhu`. When the dispatch only compares the value
against small constants (state 0..N, fits in a byte), the target was written to
read just the low byte (`lbu`). The two reads are different widths in the same
function, so a blanket type change can't satisfy both.

## The fix — read the low half explicitly at the dispatch site

Leave the global `u16` (the wide reads still need it) and read the **low byte**
directly via a byte-pointer cast at the entry:

```c
extern u16 D_800A3578;
...
s32 state = *(u8 *)&D_800A3578;   /* lbu %gp_rel(D_800A3578) — low byte only */
...
/* sibling read elsewhere keeps the full halfword: */
u16 word = D_800A3578;            /* lhu — needed by (word >> 8) and word + 1 */
```

Little-endian: byte offset 0 is the low half, so `*(u8 *)&G` emits
`lbu $r,%gp_rel(G)` (GCC keeps gp-rel addressing for the small-data symbol even
through the address-of cast). Use `*(s8 *)&G` if the target read is `lb`
(signed).

## Confirmed case — func_8006EC0C (text1b.c, 2026-05-26)

Queue top, verdict C, pure-C distance 1, one regfix `subst "lhu" "lbu" @ 1` on
`D_800A3578`. The function's entry read `s32 state = D_800A3578;` emitted `lhu`;
target `lbu`. A later `u16 word = D_800A3578;` in the ramp-up branch needs the
full halfword (`if ((word >> 8) != 0)` + `D_800A3578 = word + 1;`), so retyping
the global to `u8` was not an option. Changing only the entry read to
`*(u8 *)&D_800A3578` → `sandbox --disable all` 1→0; `retire` dropped the rule;
SHA1 == oracle. 100% pure C.

## Related
- [[narrow-stack-param-subword-offset]] — same "read the half you want" lever for
  a sub-word **stack parameter** at the wrong offset
- [[halfword-index-srl-sra]] — sibling single-`subst` fork quirk closed by a
  direct byte-offset cast (index the bytes you want)

# saTan2KabutoWareMove (src/code6cac_b.c) — WIP checkpoint 2026-08-06

**Honest floor: 44 (unmoved this session — characterised only, not yet ground).**
Wiring: `asmfix.txt` `saTan2KabutoWareMove: replace_with_asmfile "asm/funcs/saTan2KabutoWareMove.s"`.

`volatile_cheat_count = 0` — the draft body is already detector-clean, so unlike
SetPacketData there is no free cheat-removal gradient to harvest here.

## Residual shape (ours 211 insns vs target 215)

**A. Prologue s-register rotation.** Target saves/uses `s2` where we use `s3` and vice
versa, and homes the second argument into a different register:
```
ours   : sw s3,36(sp) | move s3,a1   ... sw s2,32(sp)
target : sw s2,32(sp) | move s2,a1   ... sw s3,36(sp)
```
Same class as SetPacketData's cluster — a save-order/arg-home difference that then
propagates through the body.

**B. Constant lands in a different argument register.** `li a1,4` (ours) vs `li a2,4`
(target), and every downstream compare against it follows (`beq v0,a1` vs `beq v0,a2`).

**C. An explicit u16 widening we do not emit — the most actionable lead.**
```
ours   : lhu v1,106(s0) ...           beq v1,a1,@
target : lhu a1,106(s0) ... andi v1,a1,0xffff | beq v1,a2,@
```
Target keeps the raw `lhu` result in one register AND materialises a separately-masked
`andi ...,0xffff` copy for the compare; we fold the mask away because the `lhu` already
zero-extends. Target is +1 insn here, which is part of the 211-vs-215 count gap.
This is a **type/width** signal, not an allocation one: it says the value was held in
something wider than `u16` at the compare (e.g. read into an `s32`/`int` local, or compared
against a value whose type forces the promotion to be materialised). See
[[header-type-correction-from-use-sites]] and [[u16-global-lhu-lbu-low-byte]].

**D. A folded 0/1 diamond.** Target keeps `beq v1,v0,@ | nop` and a separate `li v0,1`
where we emit `bne v1,v0,@ | move v0,s6` — we reuse an already-live register for the
constant, target rematerialises it.

## Why C is already "correct" and still folds — the real blocker

The source ALREADY expresses target's structure:
```c
temp_a1 = *(u16 *)(arg0 + 0x6A);   /* raw  -> target keeps this in $a1 */
temp_v1 = temp_a1 & 0xFFFF;        /* mask -> target keeps this in $v1 */
...
if (temp_v1 != 4) ...              /* compares use the masked copy */
((u32)(temp_a1 - 0x19) >= 2U)      /* subtract uses the RAW copy    */
```
Target emits `lhu a1,106(s0)` + `andi v1,a1,0xffff` and keeps both live. We emit only the
`lhu` because `temp_a1` is `u16`, so combine proves the mask redundant via `nonzero_bits`
and folds it, collapsing the two values into one register.

**Measured negative (2026-08-06): widening `temp_a1` to `u32` does NOT defeat the fold —
score stayed 44.** The zero-extending `lhu` still tells combine the value fits in 16 bits,
so the declared type of the holder is irrelevant.

## Recommended next moves

1. The fold is basic-block-local (`reg_last_set_nonzero_bits`). The lever is therefore to
   put a **basic-block boundary between the load and the mask**, or to give `temp_a1` two
   reaching definitions so the nonzero-bits union is not provably 16-bit. Neither has been
   tried. Do NOT retry type widening — it is measured dead.
2. Re-measure after any success on (1); B (`li a1,4` vs `li a2,4`) and D (the folded 0/1
   diamond) may be downstream of A/C rather than independent.
3. Only then treat the prologue rotation, which is the known-hard save-order class
   ([[no-new-park-categories]]).

## Tooling

Use `tools/pairdiff.py code6cac_b saTan2KabutoWareMove`.
`inverse_compose.py classify` does NOT work on this function (`replace_with_asmfile`
target is unreadable to `goalmap.asm_body`) — see the func_80089F3C checkpoint.

---
name: narrow-stack-param-subword-offset
paths: [".claude/rules/narrow-stack-param-subword-offset.md"]
description: "A narrow (u16/s16) 5th+ stack parameter loaded from slot+2 where target reads slot+0: declare it as a 4-byte type and read the low half explicitly (`u16 lo = *(u16 *)&arg;`)."
metadata:
  type: reference
---

# Narrow stack parameter loaded at the wrong sub-word offset

## Symptom

A function takes a narrow (`u16`/`s16`) parameter beyond the 4 register args. Target loads it from the low half
of its 4-byte home slot (`lhu $s2, 0x440($sp)`); our build loads `slot+2` (`0x442`) — off by 2. (Measured
2026-05, before the `-mel` adoption; confirm the offset in the honest build first.)

## Cause

This cc1 places a HImode/QImode stack parameter at the padded (high) end of its word slot; only an SImode parm
stays at offset 0. No pin or scheduling lever moves it while the parameter stays narrow. Declaring it `s32`
alone fixes the offset but turns the load into `lw` and reshuffles registers (measured 1 → 4).

## Fix — 4-byte param + explicit low-half read

```c
void f(u8 *a0, s16 a1, s16 a2, s16 a3, s32 arg4) {  /* was u16 arg4 */
    u16 arg4_lo = *(u16 *)&arg4;                       /* lhu from slot+0 */
    ...
    rect[1] = arg4_lo;
}
```

Taking `&arg4` keeps the parameter in its SImode home at offset 0, and `*(u16 *)&arg4` is a plain halfword
load at the target offset. Callers pass a promoted int either way, so no ABI or call-site change. Use
`s16`/`*(s16 *)` for `lh`. Sub-word param reads are a frozen-list SOTN-accepted entry
([[no-new-park-categories]]). Example: efc_buki_draw_zanzou, 1 → 0.

## Related

[[halfword-index-srl-sra]] · [[register-alloc-pure-c]]

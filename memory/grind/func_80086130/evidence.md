# Evidence — func_80086130 (src/main.c)

## Session 1 (2026-08-17, modality: recon) — SOLVED, bytes proven

### Routing
`canonical func_80086130` → verdict **C**, `asm_insns: 0`, `total: 35`,
`distance: 26`, reason "pure-C distance 26 <= 50 — pure-C target". No canonical-asm
signal of any kind; this was always a pure-C function.

### Starting state
`src/main.c` carried a cheat body: `s32 func_80086130(s32 a0, s32 a1, s32 a2)`
with five `register T x asm("$N")` pins (`$2`, `$3`, `$6` declared `volatile int`,
`$5`, `$4`) and a statement-per-instruction transliteration of the target asm
(`v0 = (a1 << 16) >> 16; ra2 = v0 << 7; ra2 += v0; ...`), storing through
`*((s16 *)((u8 *)&D_80102A7A + ra1))`-style casts. Zero regfix/asmfix rules.
Honest sandbox distance 26 with 39 build instructions against 35 target
instructions. Six duplicate `extern` declarations sat above it.

### The decisive find: an already-matched sibling 280 lines up
`func_80086014` at `src/main.c:967` is COMPLETED-C (`sandbox --disable all`
→ score 0) and is the SAME FUNCTION SHAPE:

```c
s32 func_80086014(s16 idx, s16 x, s16 y)
{
    if ((u16)idx < 0x18) {
        D_80102A78[idx * 8 + 1] = y;
        D_80102A78[idx * 8] = x;
        D_800F65E0[idx] |= 3;
        return 0;
    }
    return -1;
}
```

Its asm (`asm/funcs/func_80086014.s`) and func_80086130's asm are
instruction-for-instruction the same skeleton — same `addiu sp,sp,-8` /
`addiu sp,sp,8` empty frame, same `andi`/`sltiu 0x18`/`bnez` guard with a
register copy in the delay slot, same `j` to a shared epilogue with
`addiu v0,zero,-1` in ITS delay slot, same `sll idx,16 / sra 16 / sll 4` byte-
offset computation, same `lui/addu at/sh` pair to the +2 then +0 halfwords of a
16-byte-strided table, same `lbu / addu v0,zero,zero / ori 3 / sb` flag update
with the `return 0` materialisation sitting in the `lbu` load-delay slot.
func_80086130 differs ONLY by scaling both stored values by 129 (emitted as
`sll r,v,7 ; addu r,r,v`).

So the target's C was the sibling's C plus the scaling. That reduced the whole
problem to "where do the two multiplies go", and made every structural axis
(guard shape, table spelling, flag update, exit form) already-answered.

### Semantics
A libsnd-style per-voice stereo volume setter. Voice index >= 24 → return -1.
Otherwise scale each channel by 129 (the 0..127 → 0..16383 percent-to-raw
conversion), store right to the +2 halfword and left to the +0 halfword of the
16-byte-strided voice-attribute table at `D_80102A78`, set the two low dirty
bits in `D_800F65E0[idx]`, return 0.

### Measured variant grid (12 hand-authored spellings, honest sandbox distance)
Sweep harness `tmp/grind/func_80086130/s1/sweep.py`, results
`tmp/grind/func_80086130/s1/sweep_results.txt`:

| variant | score | insns |
|---|---|---|
| `s16 vx = x*129; s16 vy = y*129;` intermediates | **1** | 35 |
| inline `y * 129` / `x * 129` at the store sites | 21 | 35 |
| `s32 vx` named, `y * 129` inlined | 21 | 35 |
| named `s32 i = idx * 8` index | 21 | 35 |
| `s32` params with `(s16)` casts at every use | 21 | 35 |
| `D_80102A7A[idx*8]` for the +2 store (inline mults) | 22 | 33 |
| `(y << 7) + y` / `(x << 7) + x` spelling | 23 | 32 |
| `s16 *p = &D_80102A78[idx*8]; p[1]=..; p[0]=..;` | 24 | 32 |
| flag update hoisted above the two stores | 24 | 35 |
| `s32` intermediates | 25 | 35 |
| `s32` intermediates, `vy` declared first | 25 | 35 |
| inverted early-return guard (`>= 0x18` → return -1 first) | 26 | 35 |

The starting cheat body scored 26 with 39 instructions, so even the plainest
sibling-analog spelling (21/35) beat it outright.

### Why `s16` and not `s32` for the intermediates — the actual mechanism
Both stores go to the same `s16` array, so GCC cannot reorder them; source order
is `[+1] = vy` then `[+0] = vx`, which the target confirms. The only free
variable was WHEN each multiply lands.

With `s32` intermediates (or with the multiplies inlined) sched1 leaves the
x-channel multiply half-formed across the address arithmetic — it emits
`sll v1,a0,0x7` before the `lbu` and keeps `addu v1,v1,a0` back to fill the
`lbu`'s load-delay slot. That consumes the delay slot, so the `return 0`
materialisation (`move v0,zero`) has nowhere late to go and gets hoisted to the
very top of the taken branch. Result: a ~13-instruction rotation against the
target and a score in the 21-25 band even though the instruction COUNT is
already exactly 35.

With `s16` intermediates both multiplies complete before the address arithmetic
begins, so the only instruction left available for the `lbu` delay slot is
`move v0,zero` — which is precisely where the target has it (and precisely where
the matched sibling has it too). Everything else falls into place: the sign-
extend scratch register is reused as `v0` for both channels, the offset lands in
`a1`, and all 35 instructions align.

### The last unit of distance was a scoring artifact, not a byte difference
At `s16` intermediates the normalized diff (`tmp/grind/func_80086130/s1/diffit.py`)
showed exactly one differing instruction out of 35:

```
*** 20  sh v1,0(at)   [%lo(D_80102A7A)]  |  sh v1,2(at)   [%lo(D_80102A78)+2]
```

`%hi(D_80102A78) == %hi(D_80102A7A) == 0x1010` and `0x2A78 + 2 == 0x2A7A`, so
both forms assemble to the same linked word at 0x80102A7A. `engine/score.py`
deliberately does NOT mask named-symbol HI16/LO16 addends (only section-relative
ones), so it counted the two spellings as different. The pre-existing reference
object had been built from the cheat body, which spelled that store through the
`D_80102A7A` extern. The matched sibling func_80086014 uses the same
`[idx * 8 + 1]` spelling this candidate does.

**Do not "fix" this by switching to `D_80102A7A[idx * 8]`.** Two distinct array
symbols are non-aliasing to GCC, which then merges the address computations: the
function drops to 32 instructions and the score jumps to 22. Measured
(`v08_a7a_extern` = 22/33 inline; the `s16`-intermediate + A7A combination = 22/32).

### Final verification (both run this session, with the edits in src/)
- `sandbox func_80086130 --disable all` → `"score": 0`, 35/35, `rules_dropped: 0`,
  no pins, no inline asm.
- `engine build` → SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle,
  **MATCH**. Run twice: once with the body final, once again after removing the
  six duplicate `extern` declarations the cheat body had left above the function.
- `regfix.txt` / `asmfix.txt` contain no `func_80086130` entry.

Zero rules, zero pins, zero inline asm, zero `volatile`, zero dead code →
COMPLETED-C by the standard's definition, pending the layer-1 cheat-reviewer and
the Judge.

### Reusable lesson for the queue
`D_80102A78` / `D_800F65E0` are a 16-byte-strided voice-attribute table plus a
per-voice dirty-flag byte array, touched by a family of small libsnd-style
setters clustered around 0x80085FD8-0x80086130 in src/main.c (`func_80085FD8`,
`func_80086014`, `func_80086080`, `func_80086130`, plus the `|= 8` and `|= 0x30`
sites at src/main.c:1081 and :1087). When one of that family is queued, diff its
asm against the already-matched members FIRST — the skeleton is shared and only
the value expression differs. The `s16`-vs-`s32` intermediate lever above is
likely to recur in that cluster wherever a scaled value is stored to an `s16`
table slot.

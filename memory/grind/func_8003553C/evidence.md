# Evidence bank — func_8003553C

## Session 1 (recon, 2026-07-30) — floor 17 -> 4

### What the function is
`func_8003553C` (src/code6cac_b2_pre.c, 43 target insns, canonical verdict **C**,
pure-C distance 17 at session start, 0 regfix/asmfix rules) builds one **POLY_G4**
(0x24 bytes) out of the bump allocator `D_800A38B4` and links it into the ordering
table at `D_800A374C + 0x401C`:

```
+0x04..0x06 rgb0 = (0,0,0x80)     +0x08/0x0A x0,y0 = (0,0)
+0x0C..0x0E rgb1 = (0,0,0x80)     +0x10/0x12 x1,y1 = (640,0)
+0x14..0x16 rgb2 = (0,0,0)        +0x18/0x1A x2,y2 = (0,240)
+0x1C..0x1E rgb3 = (0,0,0)        +0x20/0x22 x3,y3 = (640,240)
```
i.e. a full-screen (640x240) blue-to-black vertical gradient backdrop quad, then
`D_800A38B4 += 0x24`. Sibling idiom for the ot_Link call is already matched
elsewhere: `src/text1b.c:15445` — `ot_Link((u32 *)(D_800A374C + 0x44), (u32 *)p);`.

### The inherited body was a cheat-asm carrier
HEAD's version carried a hardcoded-`$N` `__asm__("addiu $3,$zero,640\naddiu
$2,$zero,240")` injection plus `register ... asm("v0"/"v1"/"a0"/"a1")` pins
(13 stripped cheat-asm insns; the sandbox strips them, so its honest score was
17 and its build was 45 insns). It has been fully replaced by pure C; no probe
this session used asm, pins, volatile, dead stores, or any coercion construct.

### Measured facts (all via `sandbox --disable all`, objdump-diffed each time)
1. **The whole residual is instruction ORDER inside one scheduling block.** Every
   form from A onward emits exactly 43 instructions with identical prologue,
   epilogue, `%hi/%lo` pairs, `move a1,s0` and delay-slot fill. Nothing is
   missing or extra — only positions differ. (Full table + side-by-side diff:
   `tmp/grind/func_8003553C/s1/forms_and_scores.md`.)
2. **CONFIRMED — the `lw` of `D_800A374C` is position-pinned by may-alias.**
   GCC 2.7.2 treats the load of the OT-base global as conflicting with the
   stores through `p` (p comes from an opaque global pointer), so sched1 can
   move it neither earlier nor later across them: its emitted position tracks
   its SOURCE position relative to the stores. Target emits it after the RGB
   byte block and before the last six coordinate stores, so the original source
   computed that argument there. Hoisting it into a local `ot` at exactly that
   point took the floor 14 -> 10; leaving it inline in the `ot_Link(...)` call
   forces it after every store.
3. **CONFIRMED — constant materialization order follows first *source* use, and
   the `li`s float to the top of the block.** Target opens with `li $v1,640`
   then `li $v0,240`, keeping 640 live in `$v1` across the entire body. That
   only happens when a 640-valued store precedes the 240-valued stores in the
   source. Putting the x1 (`+0x10 = 640`) store at the head of the body, ahead
   of the two 240 stores, took the floor 10 -> 4 and reproduced target's first
   five instructions and the whole RGB block exactly.
4. **KILLED — RGB-block-first source order** (the "natural" PsyQ
   `setRGB0..3` then `setXY4` order): score 16. It makes `li $v0,128` the first
   constant, which hoists `sb $v0,6` / `sb $v0,0xE` out of source order — target
   keeps the RGB block strictly in field order.
5. **KILLED — moving `D_800A38B4 = p + 0x24;` before the call** to get target's
   pre-jal `addiu $s0,$s0,0x24`: score stays 10/4 because the `sw` to the global
   moves before the jal too (target's `sw` is after), trading one diff for
   another. Staging the value in a separate local (`next = p + 0x24;` before the
   call, `D_800A38B4 = next;` after) is also neutral — GCC coalesces the local
   into `$s0` and still emits the `addiu` after the delay slot.

### Residual (score 4 = two misplaced instructions)
- **R1** `sh $v1,0x10($s0)` sits at block position 6 (immediately after
  `li $v0,128`) because that store is what materializes 640 early. Target emits
  the same store in the trailing group, between `sh zero,0x0A` and
  `sh zero,0x18`. Need: 640 created early in the RTL stream *without* an early
  store to +0x10.
- **R2** `addiu $s0,$s0,0x24` is emitted after the jal delay slot; target emits
  it immediately before the `jal`.

- [s1] func_8003553C builds one POLY_G4 (0x24 bytes) from the bump allocator D_800A38B4 — a full-screen 640x240 blue-to-black gradient backdrop quad (rgb0/rgb1 = 0,0,0x80; rgb2/rgb3 = black; corners (0,0) (640,0) (0,240) (640,240)) — links it at D_800A374C + 0x401C via ot_Link, then advances D_800A38B4 by 0x24.

- [s1] The inherited HEAD body was a cheat-asm carrier: a hardcoded-$N __asm__("addiu $3,$zero,640\naddiu $2,$zero,240") injection plus register asm("v0"/"v1"/"a0"/"a1") pins (13 cheat-asm insns stripped by the sandbox, honest score 17, 45 build insns). It is now fully pure C — no asm, no pins, no volatile, no dead stores, no coercion construct of any kind.

- [s1] Every pure-C form measured emits exactly 43 instructions, equal to target: the entire residual is instruction ORDER inside the single scheduling block between the initPolyG4 jal and the ot_Link jal. Prologue, epilogue, both %hi/%lo pairs, the move a1,s0 and the delay-slot fill are already identical.

- [s1] Residual R1 (the larger diff): our sh $v1,0x10($s0) lands at block position 6, immediately after li $v0,128, because that store is what materializes 640 early; target emits the same store in the trailing group between sh zero,0x0A and sh zero,0x18. What is needed is 640 created early in the RTL stream WITHOUT an early store to +0x10.

- [s1] Residual R2: addiu $s0,$s0,0x24 is emitted after the jal's delay slot; target emits it immediately before the jal. Both obvious source spellings were measured and are a wash (see the killed hypothesis).

- [s1] Matched sibling idiom for the call shape already exists in the tree at src/text1b.c:15445 — ot_Link((u32 *)(D_800A374C + 0x44), (u32 *)p); — the same OT-base-plus-constant argument form.

- [s1] Score ladder measured this session (sandbox --disable all): inherited cheat-asm carrier 17, RGB-block-first 16, coordinates-first 15/14, y2/y3-first + ot local 10, + x1 store leading 4.

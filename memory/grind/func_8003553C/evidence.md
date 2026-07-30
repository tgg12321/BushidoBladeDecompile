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

## Session 2 (structural, 2026-07-30) — floor 4 -> 2

NOTE for whoever reads this next: at the start of s2, src/code6cac_b2_pre.c still
held the ORIGINAL cheat-asm carrier (s1's candidate had never been applied to the
tree). The banked candidate was re-applied first and re-measured at 4 before any
probe. The tree now holds the score-2 form.

### R2 is SOLVED (the `addiu $s0,$s0,0x24` before the `jal`) — floor 4 -> 2
The lever is keeping the OLD primitive pointer and the ADVANCED pointer
simultaneously live across the call:
```c
    q = p;
    p += 0x24;
    ot_Link(ot, (u32 *)q);
    D_800A38B4 = p;
```
Both values are live at the call, so GCC materializes the argument copy
(`move $a1,$s0`) early, emits the in-place advance `addiu $s0,$s0,0x24` in the
pre-jal slot, and leaves the `sw` to the global after the call — target's exact
tail, all three instructions. s1's two killed spellings both had only ONE of the
two values live at the call, which is why they were a wash: `D_800A38B4 = p+0x24;`
before the call drags the `sw` forward too, and `next = p + 0x24;` staged in an
extra local is coalesced straight back into `$s0`. (V4, the second of those, was
re-measured this session on the score-4 base and is still 4.)

### The score-2 residual is exactly ONE instruction
`dump_W.txt` vs target: everything — prologue, both `%hi/%lo` pairs, the twelve
RGB byte stores in field order, `move a1,s0`, the addiu/jal/delay-slot/sw tail,
the epilogue — is instruction-for-instruction identical. The only diff is our
`sh $v1,0x10($s0)` at block position 6 (right after `li $v0,128`) where target
emits it in the post-load group between `sh zero,0x0A` and `sh zero,0x12`.

### Measured facts added this session
6. **KILLED — constant-holder locals cannot move the `li` earlier.** `s16 w = 640`
   (and `s16 h = 240`), the s32 variants, and the setXYWH-style `x + w` / `y + h`
   sum spelling ALL score exactly what the no-lever form scores: cse propagates the
   constant into the two use sites and deletes the standalone set, leaving no insn
   at the declaration point. There is therefore NO source spelling that materializes
   640 early without an early USE of 640.
   (`memory/grind/func_8003553C/rejected/constant-holder-local-640-folds-away.c`)
7. **KILLED — struct-typed member stores (a real POLY_G4 typedef).** Motivated by
   `tools/gcc-2.7.2/sched.c:834-881`, which carries the classic MEM_IN_STRUCT_P
   rule: an in-struct MEM with a varying address is treated as NOT conflicting with
   a MEM that is neither in-struct nor address-varying — which is exactly the lw of
   the scalar global D_800A374C. Had it applied, the pre-load/post-load barrier
   would have dissolved and the 0x10 store could sink freely. Measured: 12 / 10 / 10
   against a base of 2 — every struct spelling is strictly worse than its
   scalar-byte-offset counterpart.
   (`memory/grind/func_8003553C/rejected/struct-typed-polyg4-member-stores.c`)
8. **The statement-order axis inside the block is SATURATED at 2.** ~45 distinct
   orderings were generated and scored this session (full table in
   `tmp/grind/func_8003553C/s2/forms_and_scores.md` §C), sweeping the position of
   each 640 store, of the two 240 stores, of the `ot` load, and of the RGB block
   including splitting it into its four 3-store groups. The minimum is 2, and it is
   reached by exactly two forms — the ones that place exactly ONE 640-valued store
   at the head of the body (x1 leading, trailing group `8,A,12,18,20`; or the mirror
   with x3 leading and trailing group `8,A,10,12,18`). Every form with NO leading
   640 store bottoms out at 8; every form with TWO of them at 5.
9. **Scheduler model (from `dump_H.txt` vs `dump_W.txt`).** An `li` is not floated
   to the block head by priority — it is hoisted only a few slots above its FIRST
   USE, and right after an `li` is scheduled the list scheduler pulls that li's
   dependent stores forward. In the no-leading-640 form that is directly visible:
   `li $v0,128` is followed immediately by `sb $v0,6` / `sb $v0,0xE`, which breaks
   the RGB block out of field order and accounts for most of that form's score.
   Target keeps the RGB block in strict field order AND has `li $v1,640` at the very
   top with both of its uses in the post-load group — the combination the remaining
   single-instruction residual is made of.

- [s2] Floor 4 -> 2. The `addiu $s0,$s0,0x24` residual (R2) is CLOSED by keeping the old primitive pointer (`q = p;`) and the advanced pointer (`p += 0x24;`) simultaneously live across the ot_Link call, storing the global after the call. Only ONE misplaced instruction now remains in the whole function.

- [s2] src/code6cac_b2_pre.c had never received s1's candidate — it still held the cheat-asm carrier at s2 start. Always verify the tree's body against memory/grind/<func>/candidate.c before probing.

- [s2] Constant-holder locals are useless for constant-materialization ORDER in GCC 2.7.2: cse propagates the constant into every use and deletes the standalone set, so `s16 w = 640;` is bit-for-bit the same program as writing 640 at each use. Four spellings measured, all identical scores.

- [s2] MEM_IN_STRUCT_P (tools/gcc-2.7.2/sched.c:834-881) is NOT a usable lever for freeing poly-primitive stores from the may-alias barrier against a fixed-address global load: struct-typed member stores measured 12/10/10 against a scalar-cast base of 2.

- [s2] ~45 statement orderings measured; the block-ordering axis is saturated at 2. Sweep harness is reusable: tmp/grind/func_8003553C/s2/ordersweep.ps1 takes pipe-separated order specs, generates the body, splices it into src and scores it (~25 s per form).

- [s2] src/code6cac_b2_pre.c still held the ORIGINAL cheat-asm carrier at session start — session 1's candidate had never been applied to the tree. It was re-applied and re-measured at 4 before any probe; the tree now holds the score-2 form (zero rules, zero inline asm, zero pins, zero dead stores, zero volatile).

- [s2] Floor 4 -> 2 this session. Residual R2 (addiu before the jal) is CLOSED; the whole function is now ONE misplaced instruction from target.

- [s2] The single remaining diff (R1): our `sh $v1,0x10($s0)` is emitted at block position 6, immediately after `li $v0,128` — as late as the scheduler will sink it inside the pre-load store group — while target emits it in the post-load group between `sh zero,0x0A` and `sh zero,0x12`, i.e. in ascending field order. Everything else (prologue, both %hi/%lo pairs, all twelve RGB byte stores in field order, move a1,s0, the addiu/jal/delay-slot/sw tail, the epilogue) is instruction-for-instruction identical to target.

- [s2] Scheduler model derived from the two disassemblies (dump_H.txt vs dump_W.txt): an `li` is NOT floated to the block head on priority — it is hoisted only a few slots above its FIRST USE, and right after an `li` is scheduled the list scheduler pulls that li's dependent stores forward. In the no-leading-640 form this is directly visible: `li $v0,128` is followed immediately by `sb $v0,6` / `sb $v0,0xE`, which breaks the RGB block out of field order and accounts for most of that form's score. Target keeps the RGB block in strict field order AND has `li $v1,640` at the very top with BOTH of its uses in the post-load group — that combination is what the remaining instruction is made of.

- [s2] GCC 2.7.2 constant-holder locals are inert for constant-materialization ORDER: cse propagates the constant into every use and deletes the standalone set, so `s16 w = 640;` is the same program as writing 640 at each use. Four spellings measured, all identical scores. Do not re-derive this.

- [s2] MEM_IN_STRUCT_P (tools/gcc-2.7.2/sched.c:834-881) is not a usable lever for freeing poly-primitive stores from the may-alias barrier against a fixed-address global load — struct member stores measured 12/10/10 against a scalar-cast base of 2.

- [s2] Reusable harness for the next session: tmp/grind/func_8003553C/s2/ordersweep.ps1 takes pipe-separated statement-order specs (hex tokens = coordinate stores, RGB / RGBa..RGBd = the byte block, OT = the ot local), generates the body, splices it into src and scores it in ~25 s per form; sweep.ps1 scores every .c in a forms dir; dump.ps1 splices one form and objdumps the sandbox object.

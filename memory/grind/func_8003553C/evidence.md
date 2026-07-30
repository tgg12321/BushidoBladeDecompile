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

## Session 3 (structural, 2026-07-30) — floor stays 2; the residual's MECHANISM is now known

NOTE: as in s2, `src/code6cac_b2_pre.c` again held the ORIGINAL cheat-asm carrier
at session start (the driver reverts src between sessions). The banked candidate
was re-applied and re-measured at 2 before any probe, and src holds it again now.

### The single misplaced instruction is a REGISTER-ALLOCATION consequence, not an ordering one
Full RTL trace of the constant-holder form (`tmp/grind/func_8003553C/s3/rtl_B/`,
`cc1 -da`) — the 640 constant set is insn uid 17:

1. `base.i.combine` (pre-sched1): insn 17 is the FIRST insn after the initPolyG4
   call, i.e. RTL order already equals TARGET's emitted order, and it carries a
   `REG_EQUAL (const_int 640)` note. **cse does not delete it.** Session 2's
   explanation ("cse propagates the constant into both uses and deletes the
   standalone set") is factually wrong and is corrected here.
2. `base.i.sched` (post-sched1, still pseudos): sched1 has **sunk** insn 17 down
   to sit immediately before its first use (`sh 0x10`, uid 81).
3. `base.i.lreg` / `base.i.greg`: with a 2-insn live range, local-alloc gives
   pseudo 75 hard reg **$v0** — the same register the 240 and the 128 constants
   used and released earlier in the block. Target keeps 640 in **$v1**.
4. `base.i.sched2`: insn 17 now carries `REG_DEP_OUTPUT` against `li $v0,128`
   (uid 36) plus anti-deps on that constant's two uses (uids 38/49), so it can
   never be hoisted above `sb $v0,0xE`. It is emitted at block idx 7,
   mid-RGB-block — exactly what every "no leading 640 store" form scores 8 for.

**The causal chain is therefore: late-only uses ⇒ sched1 collapses the constant's
live range ⇒ local-alloc reuses $v0 ⇒ REG_DEP_OUTPUT pins the `li` mid-block.**

### DIAGNOSTIC (never committable) — a register pin isolates the constraint
`tmp/grind/func_8003553C/s3/forms3/I_pin_v1_DIAGNOSTIC.c` = the holder form with
target's statement order (both 640 stores in the post-load group) plus
`register s16 w asm("$3")`, compiled OUTSIDE the sandbox via
`tmp/grind/func_8003553C/s3/rtl.sh I` (the sandbox strips cheat-asm, so a pinned
form cannot be scored; `rtl_I/out.s` is read directly). Result: with 640 in a
register no earlier constant used, **every store lands in target's slot** — the
whole RGB block returns to strict field order AND `sh $3,16` sits in the
post-load group between `sh 0,10` and `sh 0,18`, which is precisely the
instruction our clean score-2 form misplaces. The ONLY residual left in the
pinned build is that `li 640` is emitted after `li 128` instead of ahead of
`li 240`. So: the register is the binding constraint on the STORE's position,
and a second, smaller constraint (the `li`'s own slot among the three constants)
sits behind it.

### Measured this session (all `sandbox --disable all`, table in tmp/.../s3/forms_and_scores.md)
1. **Constant holders re-measured on the s2 tail: 8, 8, 8** (`s16 w`, `s32 w`,
   `s16 w`+`s16 h`) — identical to the no-holder/no-leading-640 form (8). Inert
   in score, but NOT inert in RTL (see the trace above). Banked as
   `rejected/holder-const-set-sunk-by-sched1.c`.
2. **KILLED — the OT-base-load spelling axis** (session 2's frontier item 2).
   `ot = &((u32 *)D_800A374C)[0x1007];` → 2. OT base staged in an `s32` local
   with `+0x401C` applied after the coordinate stores → 2. Both identical to the
   base; the load's rtx shape does not move anything.
3. **KILLED — MEM_IN_STRUCT_P on the HImode stores.** New reading of
   `tools/gcc-2.7.2/sched.c:817-866`: the in-struct/varying-address exemption
   that would free a store from conflicting with the fixed-address `lw` of
   D_800A374C **excludes QImode**, so the twelve RGB byte stores can never
   escape it — but HImode coordinate stores CAN, and `p[i]`-style indexing does
   set MEM_IN_STRUCT_P (the RGB stores show as `mem/s:QI` in the dumps while
   `*(s16 *)(p + 8)` shows as plain `mem:HI`). Writing all eight coordinate
   stores as `sp[i]` (`s16 *sp = (s16 *)p;`) legitimately dissolves the barrier
   for them — and changes NOTHING: 2 (same as base) in the s2 order, 8/9/8 in
   the target order. GCC 2.7.2's list scheduler is movement-minimizing, so
   dependence freedom without a priority reason does not move an insn. Banked as
   `rejected/memstruct-hi-index-stores-barrier-free-but-inert.c`.
4. **KILLED — buying the second register by extending another constant's live
   range.** `p[0xE] = 0x80` moved into the post-load group (extend the 128
   pseudo) → 6, and the disassembly shows 640 still in $v0: sched1 re-collapses
   the 640 set next to its use, and the competing store's range ends immediately
   before that point, so $v0 is free again. `sh 0x22 = 240` moved into the
   post-load group (extend the 240 pseudo) → 11, with a holder → 11. Banked as
   `rejected/v0-liveness-extender-rgb1-blue-late.c`.

### Scheduler model, corrected and grounded in the source
`priority()` (sched.c:1425) walks LOG_LINKS, i.e. **predecessors**, so
INSN_PRIORITY is the longest dependence path from the block HEAD, and
`schedule_block` builds the block BACKWARD from the tail (`ready` is seeded by
walking insns in reverse and taking those with INSN_REF_COUNT == 0).
`rank_for_schedule` (sched.c:2399) compares (a) priority, (b) a three-way class
of the candidate's relation to the last-scheduled insn — where a dependence
whose `insn_cost` is 1 counts as "independent" — and (c) `INSN_LUID`, i.e.
source order, explicitly "so that we make the sort stable. This minimizes
instruction movement". Practical consequence for this function: with almost every
insn in the block tied at the same priority, **emitted order tracks SOURCE order**
and the only insns that actually move are ones whose register lifetimes make them
move. That is why 45 orderings bottom out at 2 and why the fix has to come from
the register side.

### Instrumentation gap (operator note, not agent-actionable)
`tools/gcc-2.7.2/sched.c` contains env-gated diagnostics (`BB2_PRIO_DEBUG` in
`priority()`, `BB2_RANK_DEBUG` in `rank_for_schedule()`), but the built binary
`tools/gcc-2.7.2/build/cc1` predates them — `grep -c RANKDBG` on the binary is 0
and the harness `tmp/grind/func_8003553C/s3/prio.sh` produces an empty log.
Getting exact per-insn priorities/rank decisions needs a cc1 rebuild, which is an
operator action (agents may not touch `tools/`).

- [s3] The whole residual of func_8003553C is a REGISTER choice, not a statement order: target holds 640 in $v1 across the block, our forms put it in $v0. RTL trace (tmp/grind/func_8003553C/s3/rtl_B/): cse KEEPS the constant set at the top of the RTL (with a REG_EQUAL note); sched1 SINKS it to sit adjacent to its first use; the resulting 2-insn live range makes local-alloc reuse $v0 (dead since the 128 constant's last use); that creates a REG_DEP_OUTPUT against `li $v0,128` which pins the `li 640` after `sb $v0,0xE` in sched2. Session 2's "cse deletes the holder's set" explanation is WRONG — the sink is sched1's, and that is what any future attempt must defeat.

- [s3] DIAGNOSTIC-ONLY proof (register pins are never committable, compiled outside the sandbox via tmp/grind/func_8003553C/s3/rtl.sh because the sandbox strips cheat-asm): with `register s16 w asm("$3")` and target's statement order (both 640 stores in the post-load group), EVERY store lands in target's slot — the RGB block is in strict field order and `sh $3,16` sits in the post-load group where target has it, i.e. the exact instruction our clean score-2 form misplaces. The only residual in the pinned build is `li 640` emitted after `li 128` instead of ahead of `li 240`.

- [s3] KILLED — the OT-base-load spelling axis (s2's frontier item 2): array-indexed slot (`&((u32 *)D_800A374C)[0x1007]`) and an s32-staged base read with the +0x401C applied later both score exactly 2, identical to the base.

- [s3] KILLED — MEM_IN_STRUCT_P as a lever. sched.c:817-866's in-struct/varying-address exemption EXCLUDES QImode, so the RGB byte stores can never escape the may-alias barrier against the `lw` of D_800A374C; HImode coordinate stores written as `sp[i]` DO get mem/s and are genuinely freed — and it changes nothing (2 / 8 / 9 / 8). GCC 2.7.2's list scheduler is movement-minimizing: freeing an insn to move gives it no reason to move.

- [s3] KILLED — extending another constant's live range to force 640 onto a second register. `p[0xE] = 0x80` in the post-load group scores 6 with 640 still in $v0 (sched1 re-collapses the 640 set next to its use, so the competing range ends just before it); `sh 0x22 = 240` in the post-load group scores 11 (11 with a holder too).

- [s3] Corrected scheduler model, read out of the source: `priority()` (sched.c:1425) walks LOG_LINKS = PREDECESSORS, so INSN_PRIORITY is the longest path from the block HEAD, and `schedule_block` builds the block BACKWARD from the tail. `rank_for_schedule` (sched.c:2399) breaks ties by class-relative-to-last-scheduled (a dependence of insn_cost 1 counts as independent) and then by INSN_LUID "to minimize instruction movement". With nearly every insn in this block tied on priority, emitted order tracks SOURCE order and only register lifetimes move things — which is why ~45 orderings bottom out at 2.

- [s3] Operator note: `tools/gcc-2.7.2/sched.c` has env-gated BB2_PRIO_DEBUG / BB2_RANK_DEBUG diagnostics but the built `tools/gcc-2.7.2/build/cc1` predates them (grep -c RANKDBG on the binary = 0), so tmp/grind/func_8003553C/s3/prio.sh logs nothing. Exact priority/rank tracing needs a cc1 rebuild — an operator action; agents may not touch tools/.

- [s3] Harness notes: from the PowerShell tool, sweep.ps1/splice.ps1 need BOTH `Set-Location` and `[Environment]::CurrentDirectory` set to the repo root (relative .NET paths), and `bash tools/wsl.sh` does NOT work there (`wsl` is not on that tool's Git-Bash PATH) — call `wsl.exe <cmd>` or `wsl.exe bash <script>` directly. `tmp/grind/func_8003553C/s3/rtl.sh <tag>` gives a full `cc1 -da` pass dump set outside the sandbox; `rtlsum.py` prints one line per RTL insn.

- [s2] Reusable harness for the next session: tmp/grind/func_8003553C/s2/ordersweep.ps1 takes pipe-separated statement-order specs (hex tokens = coordinate stores, RGB / RGBa..RGBd = the byte block, OT = the ot local), generates the body, splices it into src and scores it in ~25 s per form; sweep.ps1 scores every .c in a forms dir; dump.ps1 splices one form and objdumps the sandbox object.

- [s3] The residual is a register choice, not a statement order: target keeps 640 in $v1 across the whole block; every pure-C form we can write puts it in $v0.

- [s3] cse does NOT delete a constant-holder's set (it survives into base.i.combine at the top of the RTL with a REG_EQUAL note) — sched1 SINKS it to sit adjacent to its first use, local-alloc then reuses $v0, and sched2's REG_DEP_OUTPUT against li $v0,128 pins it mid-RGB-block. This corrects session 2's stated mechanism.

- [s3] DIAGNOSTIC-ONLY (never committable): with 640 pinned to $3 and target's statement order, all 43 insns except one land in target's slot — including the sh 0x10 store in the post-load group that our clean score-2 form misplaces. Residual in the pinned build: li 640 after li 128 instead of ahead of li 240.

- [s3] sched.c's in-struct/varying-address disambiguation exemption (lines 817-866) EXCLUDES QImode, so the twelve RGB byte stores can never escape the barrier against the lw of D_800A374C; HImode stores written as sp[i] do escape it and it changes nothing (2 vs base 2).

- [s3] GCC 2.7.2's list scheduler is movement-minimizing: priority() measures the path from the block HEAD, schedule_block runs backward from the tail, and ties break on INSN_LUID (source order). With nearly every insn in this block tied, emitted order tracks source order and only register lifetimes move things — which is why session 2's ~45 orderings bottom out at 2.

- [s3] The OT-base-load spelling axis is inert: array-indexed slot 2, s32-staged base read 2, identical to the base.

- [s3] Extending the 128 or 240 constant's live range past the 640 set does not buy a second register: 6 (640 still in $v0) and 11.

- [s3] Operator note: tools/gcc-2.7.2/sched.c carries env-gated BB2_PRIO_DEBUG / BB2_RANK_DEBUG diagnostics but the built tools/gcc-2.7.2/build/cc1 predates them (grep -c RANKDBG on the binary = 0), so exact priority/rank tracing needs a cc1 rebuild — agents may not touch tools/.

- [s3] src/code6cac_b2_pre.c again held the ORIGINAL cheat-asm carrier at session start (the driver reverts src between sessions); the banked candidate was re-applied, re-measured at 2, and is in place in src now.

- [s3] Harness gotchas for the next session: from the PowerShell tool set BOTH Set-Location and [Environment]::CurrentDirectory to the repo root (sweep.ps1/splice.ps1 use relative .NET paths), and `bash tools/wsl.sh` does not work there (no wsl on that tool's Git-Bash PATH) — call wsl.exe directly.

## Session 4 (permuter, 2026-07-30) — floor stays 2; the PERMUTER AXIS IS DEAD, with the reason measured

NOTE (third time): `src/code6cac_b2_pre.c` again held the ORIGINAL cheat-asm
carrier at session start. The banked candidate was re-applied and re-measured at
2 (`sandbox --disable all`, 43/43 insns) before any probe, and src holds it now.

### The clean single-function permuter workspace exists and is validated
`tmp/grind/func_8003553C/s4/mkws.sh <ws-dir> <base.c>` builds a
`difficult-is-not-impossible` section-3-compliant workspace in one command: it writes
`settings.toml` (func_name/gcc), a `compile.sh` that runs the REAL pipeline
(cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 | prologue_fix | maspsx 2.34 with
every project gate list | multu_pad) and extracts only `func_8003553C`'s region
before assembling, builds `target.o` from `prelude.inc` (minus `.set gp=64`) plus
`asm/funcs/func_8003553C.s` so the function sits at offset 0, and finally
validates by objdump-diffing base.o against target.o. Validation output for the
banked candidate: `base insns: 43  target: 43`, diff = the single known
`sh v1,16(s0)` displacement. Two gotchas cost real time and are recorded so the
next session does not repeat them: (a) `mktemp /tmp/fooXXXXXX.s` FAILS on this
WSL (suffix form unsupported — it prints a name on stderr and returns an empty
string), so temp templates must end in the X's; (b) the first run after the
heredoc rewrites `compile.sh` can fail spuriously — re-run it.
A standalone base.c (typedefs plus the four externs plus the function) reproduces the
full-TU codegen exactly; the TU context is NOT load-bearing for this function.

### `tmp/grind/func_8003553C/s4/score.sh` — the offline scorer (much faster than sandbox)
Compiles any standalone form through `wsA/compile.sh` and reports
`difflines` = objdump line-diff against target.o, plus the emitted insn count.
~1.5 s per form vs ~25 s for a sandbox splice+score. Correspondence with the
engine metric on measured points: banked candidate difflines 2 <-> sandbox 2;
chassis-B (target statement order) difflines 10 <-> sandbox 8. Monotone, so it is
a safe screening metric; confirm anything promising with `sandbox --disable all`.

### KILLED — decomp-permuter random search cannot navigate this residual
Two fresh-seed campaigns, both harvested and stopped in-session:
  * **chassis A** (`wsA`, label `chassisA-scalar-offsets`) — seeded from the
    banked score-2 candidate. Permuter base score **225**. ~1.5k iterations,
    stopped after an 8-minute no-improvement window at best **100**.
  * **chassis B** (`wsB`, label `chassisB-target-stmt-order`) — seeded from
    TARGET's own statement order (no leading 640 store; both 640 stores in the
    post-load group, ascending). Permuter base score **270**, best **50**.

**The decisive measurement:** every one of the ~85 output forms from both
campaigns was re-scored offline with `score.sh`. The distribution of
`difflines` (emitted insn count in parentheses) is
`1(44) x1 | 2(43) x6 | 3 x3 | 4(43) x14 | 5 x1 | 6(43) x13 | 8(43) x17 | 9-18 x30`.
**No form beats the incumbent's difflines 2 at 43 instructions.** The single
difflines-1 form has **44** instructions: it is chassis A's body with the
`*(s16 *)(p + 0x10) = 640;` store DUPLICATED into the post-load group, i.e. all
43 target instructions in target's exact order plus one extra `sh`. That is a
+1-instruction redundant dead store, not a match, and it shows the two roles
(materialize 640 early / store it late) genuinely need two different
instructions unless the register changes.

**Why the permuter cannot help here (the transferable finding):** its
weighted objective does NOT track the honest distance for this function. The
incumbent — one instruction displaced by 17 slots, sandbox 2 — scores **225**,
while chassis-B forms that are objectively much worse (difflines 10-12) score
**50-65**. The permuter's differ charges a long-range displacement as a large
pile of reorderings while rewarding register-name agreement, so the search
gradient points AWAY from the true optimum. Random mutation from the score-2
base is therefore not merely unproductive here, it is anti-correlated, and no
amount of extra sampling or reseeding changes that. Session 3's frontier item 2
("permuter from the score-2 base can find the register-lifetime shape hand
enumeration cannot express") is closed NEGATIVE.

### Sibling census (frontier item 3) — RUN, and it produced a real analogue
`tmp/grind/func_8003553C/s4/census.py` scans every `asm/funcs/*.s` belonging to a
function NOT in `engine/queue.json` (i.e. already matched) for
`addiu $vN, $zero, IMM` whose first consumer is >= 4 instructions later — the
exact "constant materialised into a register with only late uses, set NOT sunk"
shape this function needs. **164 hits.** The structurally closest is
`func_80072BC4` (`src/text1b.c:16641`), whose `arg0 >= 4` else-arm is another
POLY_G4 RGB block written through a register-held primitive pointer and opens
with THREE block-head constants:

```
.L80072C60:  addiu $v1,$zero,0x40 ; addiu $a0,$zero,0x80 ; addiu $v0,$zero,0x50
             sb $v0,0x14  sb $v0,0x15(0xA0)  sb $v1,0x16 ... sb $zero,4/5/6
             sb $v1,0xC   sb $zero,0xD  sb $a0,0xE
             sb $v0,0x1C  sb $v1,0x1D   sb $a0,0x1E
```

Its C (src/text1b.c:16669-16680) is plain ascending literal byte stores — no
holder, no cast trick, no reordering. The pattern across that block: constants
with **>= 2 uses spread across the block** (0x40 x3, 0x80 x2, 0x50 x2) get a
block-head `li` and their own hard register; single-use constants (0xA0, 0x10)
are materialised adjacent to their use. Our 640 has exactly 2 uses and still
sinks, so the discriminator is NOT the use count alone — the difference is that
in `func_80072BC4` all uses sit inside one uninterrupted store run, whereas our
two 640 uses sit AFTER the may-alias-pinned `lw` of D_800A374C and the
`move $a1,$s0`. That is a concrete, un-probed structural difference and it is
the strongest remaining lead.

### KILLED — the sibling's spelling transplant
`func_80072BC4` writes its fields as `*(u8 *)((s32)arg1 + N) = v` (an s32-cast
byte-offset store, which does NOT set MEM_IN_STRUCT_P) rather than our `p[N]`.
Transplanting that exact spelling onto chassis B (`C2_sibling_s32cast_spelling.c`,
all twenty stores respelled) gives difflines **10** — bit-identical to the
chassis-B control (`C3_chassisB_control.c`, difflines 10). The spelling is inert;
what makes the sibling's constants behave is its dataflow, not its syntax.
Also measured: `C1_rgb6_between_240s.c` (the permuter's best chassis-B
reordering with the pointer temps stripped — `p[6] = 0x80` hoisted between the
two 240 stores) = difflines 12, worse than the chassis-B control.

- [s4] The permuter axis for func_8003553C is DEAD and the reason is measured, not assumed: the permuter's weighted objective is anti-correlated with the honest distance on this function (incumbent = 1 displaced insn = permuter 225; chassis-B forms 5x worse objectively = permuter 50-65). ~85 output forms from two fresh-seed chassis, all re-scored offline; none beats difflines 2 at 43 insns.
- [s4] The one difflines-1 permuter form is 44 insns: the leading `*(s16*)(p+0x10) = 640;` store DUPLICATED into the post-load group. It reproduces all 43 target insns in target order plus one extra sh — direct proof that "materialise 640 early" and "store 640 late" need two separate instructions unless the 640 pseudo changes register.
- [s4] A clean single-function permuter workspace for this function is one command: `bash tmp/grind/func_8003553C/s4/mkws.sh <ws> <standalone-base.c>` (validated base 43 / target 43). A standalone base.c with just typedefs + 4 externs reproduces full-TU codegen exactly. WSL gotcha: `mktemp /tmp/xXXXXXX.s` (X's not last) silently returns empty.
- [s4] `tmp/grind/func_8003553C/s4/score.sh` scores any standalone form in ~1.5 s via objdump difflines against target.o (candidate 2 <-> sandbox 2; chassis B 10 <-> sandbox 8). Use it to screen forms, sandbox only to confirm.
- [s4] Sibling census (tmp/grind/func_8003553C/s4/census.py) found 164 matched functions with a block-head `addiu $vN,$zero,K` whose first use is >=4 insns later. Closest analogue: func_80072BC4's else-arm (src/text1b.c:16669-16680) — another POLY_G4 RGB block through a register pointer, three block-head constants, written as plain ascending literals. Its constants have 2-3 uses ALL INSIDE one uninterrupted store run; our 640's two uses sit after the may-alias-pinned OT lw and the `move $a1,$s0`. That interruption is the un-probed difference.
- [s4] KILLED — the sibling's `*(u8 *)((s32)p + N)` store spelling (no MEM_IN_STRUCT_P) is inert: difflines 10, bit-identical to the `p[N]` chassis-B control (10).

- [s4] Floor unchanged at 2 (sandbox --disable all, 43 build insns / 43 target insns). src/code6cac_b2_pre.c AGAIN held the original cheat-asm carrier at session start (third session running); the banked candidate was re-applied, re-measured at 2 before any probe, and is in place in src at session end.

- [s4] A validated, difficult-is-not-impossible-section-3-compliant single-function permuter workspace now exists as a ONE-COMMAND recipe: bash tmp/grind/func_8003553C/s4/mkws.sh <ws-dir> <standalone-base.c>. It writes settings.toml + a compile.sh running the real pipeline (cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 | prologue_fix | maspsx --aspsx-version=2.34 with every project gate list | multu_pad) with per-function region extraction, builds target.o from prelude.inc (minus '.set gp=64') plus asm/funcs/func_8003553C.s at offset 0, and self-validates by objdump diff. Validation for the banked candidate: base insns 43 / target 43, diff = the single known sh v1,16(s0) displacement.

- [s4] A standalone base.c (typedefs plus the four externs D_800A38B4 / D_800A374C / initPolyG4 / ot_Link plus the function) reproduces the full-TU codegen EXACTLY - the surrounding translation unit is not load-bearing for this function, so future probes can skip the ~25 s sandbox splice.

- [s4] tmp/grind/func_8003553C/s4/score.sh scores any standalone form in ~1.5 s as objdump difflines against target.o. Correspondence with the engine metric on measured points: banked candidate difflines 2 <-> sandbox 2; chassis B difflines 10 <-> sandbox 8. Monotone, so it is a safe screening metric (confirm anything promising with sandbox --disable all).

- [s4] WSL gotcha that cost real time: mktemp with a template whose X's are not last (e.g. /tmp/fooXXXXXX.s) FAILS on this machine - it emits a name on stderr and returns an EMPTY string. tools/mar_perm_compile.sh and tools/mar_perm_workspace.sh both carry this bug. Also, the first run of a script that has just heredoc-rewritten its own compile.sh can fail spuriously; re-run it.

- [s4] The single difflines-1 permuter form emits 44 instructions: the leading *(s16 *)(p + 0x10) = 640; store duplicated into the post-load group. It contains all 43 target instructions in target's exact order plus one extra sh - direct proof that 'materialise 640 early' and 'store 640 late' need two separate instructions unless the 640 pseudo changes hard register. It is a redundant dead store and is NOT proposed (banked as rejected/permuter-duplicate-640-store-44-insns.c).

- [s4] Session 3's frontier item 3 (sibling census) was EXECUTED this session. tmp/grind/func_8003553C/s4/census.py scans every asm/funcs/*.s whose function is NOT in engine/queue.json (i.e. already matched) for an addiu $vN,$zero,K whose first consumer is >= 4 instructions later: 164 hits.

- [s4] The closest structural analogue is func_80072BC4 (src/text1b.c:16641), whose arg0>=4 else-arm is another POLY_G4 colour block through a register-held primitive pointer and opens with THREE block-head constants (addiu $v1,0x40 / addiu $a0,0x80 / addiu $v0,0x50) whose first uses are several instructions later. Its C is plain ascending literal byte stores - no holder, no cast trick, no reordering. Pattern in that block: constants with >= 2 uses spread across the block (0x40 x3, 0x80 x2, 0x50 x2) get a block-head li and their own hard register; single-use constants (0xA0, 0x10) are materialised adjacent to their use.

- [s4] Our 640 also has exactly 2 uses and still sinks, so use-count alone is NOT the discriminator. The measured difference: in func_80072BC4 every use of a block-head constant lies inside ONE uninterrupted run of stores, whereas our two 640 uses sit AFTER the may-alias-pinned lw of D_800A374C and the move $a1,$s0. That interruption of the store run is the concrete, un-probed structural difference and the strongest remaining lead.

- [s4] Both campaigns were harvested with --stop before the session ended; python3 tools/permuter_campaign.py status reports '0 live campaign(s), 0 stale registry entr(ies)'. No background process was left running.

- [s4] No cheat construct was used or proposed this session: no regfix/asmfix edits, no register pins, no inline asm, no volatile, no dead stores, no holder locals in any banked form. The two permuter forms banked under rejected/ are recorded as rejected precisely because they are (a) a redundant dead store and (b) an artefact of the mis-specified objective.

## Session 5 (permuter, 2026-07-30) — floor stays 2; the ORDER/SPELLING/DECL space is exhaustively saturated, and the "early li + late stores" combination is proven unreachable in it

NOTE (fourth session running): `src/code6cac_b2_pre.c` again held the ORIGINAL
cheat-asm carrier at session start (the driver reverts src between sessions).
The banked candidate was re-applied and re-measured at **2** (`sandbox
--disable all`, 43 build insns / 43 target insns) before any probe, and src
holds it at session end.

### Why no decomp-permuter campaign was run (and what replaced it)
Session 4 did not merely fail to find an improvement with decomp-permuter — it
MEASURED the reason: the permuter's weighted objective is anti-correlated with
the honest distance on this function (incumbent = 1 displaced insn = permuter
225; forms 5x worse = 50-65). Re-running a campaign under that objective would
re-run banked evidence. Instead this session built the same instrument with the
objective FIXED:

* `tmp/grind/func_8003553C/s5/search_lib.py` — form generator + scorer. Emits a
  standalone TU (s4 proved the surrounding TU is not load-bearing), compiles it
  through `s4/wsA/compile.sh` (the real cc1 / prologue_fix / maspsx-2.34 /
  multu_pad pipeline), and returns the objdump instruction-sequence diff against
  `s4/wsA/target.o` plus a **constant-materialisation signature**
  `<idx li 640>/<idx li 240>/<idx li 128>@<register of 640>`.
  Control: the banked candidate scores difflines 2, sig `7/8/11@v1`; TARGET's own
  disassembly has the identical sig `7/8/11@v1`. (Note the sig equality — the
  banked body ALREADY reproduces target's constant order and register; the
  residual is only the store's slot. This corrects the s3/s4 framing that the
  primary objective is "get li 640 ahead of li 240".)
* `tmp/grind/func_8003553C/s5/hill.py` — exhaustive 1-token-move hill climb
  (160 neighbours/iteration, 8-way parallel, ~1 min/iteration) over the cross
  product of statement order x declaration order (6 perms of `u8 *p; u8 *q;
  u32 *ot;`) x coordinate-store spelling (`*(s16*)(p+N)` / `sp[N/2]` /
  `*(s16*)((s32)p+N)`) x RGB spelling (`p[N]` / `*(u8*)((s32)p+N)`) x OT-base
  spelling (`(u32*)(D_800A374C+0x401C)` / `&((u32*)D_800A374C)[0x1007]`).
* `tmp/grind/func_8003553C/s5/search.py` — 200 uniform-random full permutations
  as a control on the hill climb's basin (best 18; random ordering is hopeless,
  which is itself why s2's 45 hand orderings were the right instrument).

### The measurements (840 forms, all scored on the honest metric)
1. **Minimum over all 840 forms is 2 — the incumbent.** 50 forms tie at 2.
2. **Hill climbing from TARGET's own statement order converges onto the
   incumbent.** Start difflines 10, sig `13/7/10@v0`; after two iterations
   (320 neighbours) it lands on the exact banked order
   `10,1A,22,R0,R1,R2,R3,OT,08,0A,12,18,20` at difflines 2. The mirror chassis
   (x3 leading) starts and stays at 2. The basin has ONE attractor and the
   banked body is its global optimum — no local-search path out.
3. **DECISIVE NEGATIVE — the early `li` and the early store are inseparable.**
   428 of the 840 forms reach 640-at-block-index-7-in-$v1 (target's constant
   shape). EVERY one of them contains a 640-valued store BEFORE the OT load.
   Not one form in the space materialises 640 at the block head while both of
   its stores sit in the post-load group; every form whose 640 stores are all
   post-load puts 640 in **$v0** at index 11/13/15/17/23 (117+33+3+1+6 forms) and
   bottoms out at difflines 8. Session 4's frontier items 1 and 3 asked for
   precisely the missing combination; it does not exist under any ordering,
   declaration order, or store spelling. Best sigs by min difflines:
   `7/8/11@v1` -> 2 (n=358), `8/7/11@v1` -> 4, `11/7/10@v1` -> 4,
   `7/10/13@v0` -> 6, `13/7/10@v0` -> 10, `7/8/9@a1` -> 12.
4. **KILLED — the static-helper-parameter axis (new this session).** Motivation:
   a function parameter is defined at the CALL SITE, so a helper taking the
   640 as an argument would give the constant an early def with late uses —
   exactly the shape needed, and a shape a human would plausibly write
   (`setXY4`-style). Four partitionings measured (whole body / whole body with
   the q-p tail split / coordinates+link only / post-load group only):
   * `static void` is **NOT inlined** by GCC 2.7.2 at -O2 (no -finline-functions,
     `inline` keyword required): a real `jal` is emitted and the extracted region
     is 19-37 insns (difflines 32/30/17/16). Useful toolchain fact.
   * `static inline void` DOES inline (43 insns each) — and the parameter
     constant is propagated into its uses and then sunk exactly like a local
     holder: 640 ends in **$v0** at index 13/27/27/27, difflines 12/46/28/18.
   So parameter dataflow is no more resistant to sched1's sink than a holder
   local is. Banked as
   `rejected/inline-static-helper-param-const-still-sunk.c` and
   `rejected/noninline-static-helper-real-call-19-insns.c`.

### What this leaves for the next session
The residual is ONE instruction and the pure-ordering/spelling/declaration/
parameter space is now exhaustively measured dead (s2: 45 forms; s5: 840 forms +
8 helper chassis; s4: 85 permuter forms). Every remaining idea has to change the
*dependence graph or register pressure* of the post-call block itself, not the
order or the syntax of its statements. Concretely un-probed:
(a) the `initPolyG4` interface — if the original prototype RETURNED the
    primitive pointer (`u8 *initPolyG4(u8 *)`, PsyQ-style chaining), the
    post-call block's live-in set and the $v0 lifetime both change, which is the
    only lever measured to matter (the 640 gets $v0 today *because $v0 is free*);
    a returned value occupies $v0 at block entry with no extra instruction if the
    result is what feeds the stores. This needs the real prototype checked
    against every other caller of initPolyG4 in the tree before it is credible.
(b) whether any OTHER matched function in the tree emits a block-head
    `addiu $vN,$zero,K` whose ONLY uses are after an intervening may-alias-pinned
    load — i.e. re-run s4's census.py with the extra requirement that a
    non-store insn separates the constant from its first use. The 164 s4 hits
    were not filtered that way, and the single hit read (func_80072BC4) has an
    uninterrupted store run. If ZERO matched functions have the interrupted
    shape, that is evidence the original source did NOT have the OT load between
    the constant and its uses — i.e. lever L1's position is right but something
    else supplies the argument (e.g. the OT slot address held in a variable
    computed BEFORE initPolyG4, whose lw would then be pre-call and out of the
    way entirely). That is a structural hypothesis the s2 sweep never covered
    because it only moved the load WITHIN the block.

- [s5] The banked body already reproduces TARGET's constant-materialisation signature exactly (li 640 at block index 7 in $v1, li 240 at 8, li 128 at 11 — verified by disassembling target.o itself). The s3/s4 framing of the primary objective ("get li 640 emitted ahead of li 240") is therefore ALREADY SATISFIED; the sole residual is the slot of `sh v1,16(s0)`.
- [s5] 840 forms measured on the honest objdump metric across statement order x declaration order x 3 coordinate spellings x 2 RGB spellings x 2 OT spellings. Minimum 2 (50 ties). Hill climbing from TARGET's own statement order (difflines 10) converges in two iterations onto the exact banked order — single attractor, banked body is the global optimum of the space.
- [s5] DECISIVE: 428 forms reach target's constant shape (640 at index 7 in $v1) and ALL of them contain a pre-OT-load 640 store; every form with both 640 stores in the post-load group puts 640 in $v0 (index 11-23) and scores 8+. The early `li` is CAUSED BY the early store — the "early li + late stores" combination is unreachable by ordering/spelling/declaration order. Session 4's frontier items 1 and 3 are closed NEGATIVE.
- [s5] KILLED — static-helper parameter dataflow. `static void` is NOT inlined by GCC 2.7.2 at -O2 (real jal; 19-37 insn regions, difflines 32/30/17/16); `static inline` inlines (43 insns) but the parameter constant is propagated then sunk like a holder local: 640 in $v0 at index 13/27, difflines 12/46/28/18 over four partitionings.
- [s5] Toolchain fact worth reusing: this build does not inline plain `static` functions at -O2 — the `inline` keyword is required. So a helper-function hypothesis must be spelled `static inline` to be testable at all.
- [s5] Instruments for the next session: tmp/grind/func_8003553C/s5/search_lib.py (generator + honest scorer + constant-signature extractor), hill.py (exhaustive 1-token-move hill climb from a named chassis, 160 neighbours/iter, ~1 min/iter, 8-way parallel), scoreforms.py (score arbitrary hand-written standalone forms). All score via s4/wsA/compile.sh, so they must be launched through wsl.exe with the venv activated — the Bash tool's Git-Bash PATH has no mipsel toolchain.
- [s5] No permuter campaign was launched this session: s4 had already MEASURED decomp-permuter's objective to be anti-correlated here, so a campaign would have re-run banked evidence. tools/permuter_campaign.py status at session end shows every registry entry alive:false — nothing orphaned.
- [s5] No cheat construct was used or proposed: zero regfix/asmfix edits, zero pins, zero inline asm, zero volatile, zero dead stores, zero holder locals in any banked form. The two new rejected/ forms are ordinary pure-C helper-function spellings that simply do not reproduce target.

- [s5] Floor unchanged at 2 (sandbox --disable all, 43 build insns / 43 target insns). src/code6cac_b2_pre.c AGAIN held the original cheat-asm carrier at session start (fourth session running); the banked candidate was re-applied, re-measured at 2 before any probe, and is in place in src at session end.

- [s5] The banked body already reproduces TARGET's constant-materialisation signature exactly: li 640 at block index 7 in $v1, li 240 at 8, li 128 at 11 (verified by disassembling target.o itself with the same extractor). The s3/s4 framing of the primary objective as 'get li 640 ahead of li 240' is therefore already satisfied; the sole residual is the slot of sh v1,16(s0).

- [s5] 840 forms measured on the honest objdump metric this session across statement order x declaration order x 3 coordinate-store spellings x 2 RGB spellings x 2 OT-base spellings. Minimum difflines 2 (50 ties). Hill climbing from TARGET's own statement order (difflines 10) converges in two iterations onto the exact banked order - the basin has a single attractor and the banked body is its global optimum.

- [s5] DECISIVE NEGATIVE: 428 of the 840 forms reach target's constant shape (640 at index 7 in $v1) and ALL of them contain a 640-valued store before the OT load; every form whose two 640 stores both sit in the post-load group puts 640 in $v0 (index 11/13/15/17/23) and scores 8 or worse. The early li is caused BY the early store, so the 'early li + late stores' combination session 4's frontier asked for does not exist under any ordering, declaration order or store spelling. Frontier items 1 and 3 from s4 are closed NEGATIVE.

- [s5] KILLED - the static-helper-parameter axis. 'static void' is not inlined by GCC 2.7.2 at -O2 (real jal; 19/19/32/37-insn regions, difflines 32/30/17/16); 'static inline void' inlines (43 insns) but the parameter constant is propagated then sunk exactly like a holder local (640 in $v0 at index 13/27, difflines 12/46/28/18 over four partitionings).

- [s5] Toolchain fact worth reusing project-wide: this build does NOT inline plain 'static' functions at -O2 - the inline keyword is required. Any helper-function hypothesis must be spelled 'static inline' to be testable at all.

- [s5] No decomp-permuter campaign was launched: session 4 had already MEASURED the permuter's objective to be anti-correlated with the honest distance on this function, so a campaign would have re-run banked evidence. This session's substitute is the same search with the objective fixed. tools/permuter_campaign.py status at session end reports every registry entry alive:false - nothing was left running or orphaned.

- [s5] Reusable instruments for the next session: s5/search_lib.py (form generator + honest objdump scorer + constant-signature extractor, ~1.5 s/form), s5/hill.py (exhaustive 1-token-move hill climb from a named chassis, 160 neighbours/iteration, 8-way parallel), s5/scoreforms.py (score arbitrary hand-written standalone forms). They must be launched through wsl.exe with .venv activated - the Bash tool's Git-Bash PATH has no mipsel toolchain and no venv.

- [s5] No cheat construct was used or proposed: zero regfix/asmfix edits, zero register pins, zero inline asm, zero volatile, zero dead stores, zero holder locals in any banked form. The two new rejected/ forms are ordinary pure-C helper-function factorings that simply do not reproduce target.

## Session 6 (forensics, 2026-07-30) — floor stays 2; the residual's EXACT GCC decision is now named

NOTE (fifth session running): `src/code6cac_b2_pre.c` again held the ORIGINAL
cheat-asm carrier at session start (the driver reverts src between sessions).
The banked candidate was re-applied and re-measured at **2** (`sandbox
--disable all`, 43 build insns / 43 target insns; the reported
`cheat_asm_stripped: 4` comes from sibling functions in the same file, not from
this body) before any probe, and src holds it at session end. This is now a
standing fact, not a surprise — budget one turn for it at the top of every session.

### The instrument: cc1 -da already contains the scheduler's own decision log
`tools/gcc-2.7.2/build/cc1 ... -da` writes the list scheduler's full trace into
`base.i.sched`: `;; insn[N]: priority = P, ref_count = R` for every insn, then
one `;; ready list at T-n: <uid (prio)> ..., now <sorted>` line per scheduling
step, then the register-lifetime deltas. **No cc1 rebuild is needed** — session 3's
operator note about `BB2_PRIO_DEBUG`/`BB2_RANK_DEBUG` being absent from the built
binary is true but irrelevant: the stock `-da` trace answers the same questions.
Harness added this session: `tmp/grind/func_8003553C/s6/rtlc.sh <form.c> <tag>`
dumps every RTL pass for a STANDALONE form into `s6/rtl_<tag>/` and prints the
emitted asm (~2 s); `s6/rtl6.sh <tag>` does the same for the current src TU.

### CORRECTION 1 — sched1 does not reorder this block at all
`base.i.sched` is insn-for-insn identical to `base.i.combine` for BOTH the
incumbent (`rtl_INC/`) and the target-statement-order form (`rtl_T/`). Every
reordering visible in the emitted code — including the RGB block breaking out of
field order in the target-order forms — is **sched2's** (post-RA). Session 3's
framing of sched1 as the reordering agent is wrong for everything except one
specific insn class (below).

### CORRECTION 2 — cse does NOT delete a constant-holder's set; sched1 MOVES it
`rtl_HT/` (target statement order + `s16 w = 640;`) shows `(set (reg:HI 75)
(const_int 640))` alive at the TOP of the function RTL in `base.i.rtl`,
`base.i.jump`, `base.i.cse`, `base.i.loop`, `base.i.flow` and `base.i.combine`
(carrying REG_EQUAL) — and then sitting immediately before its first use in
`base.i.sched`. So session 2's "cse propagates and deletes the standalone set"
is wrong (session 3 already said so) AND session 3's "sched1 sinks it" is right
but was never explained. It is explained now.

### THE DECISION, in the compiler's own trace

    ;; ready list at T-11: 78 (2) 75 (2) 9 (7f000001), now 9 78 75

Insn 9 is the `li 640`. Every other insn in the block carries priority 2; insn 9
enters the ready list carrying **0x7f000001 = LAUNCH_PRIORITY** (`sched.c:187`)
and is therefore sorted to the front and scheduled immediately.
The chain that produces it:

* `schedule_block` builds the block **backward** — "The first insn scheduled
  becomes the new tail" (`sched.c:3813`), `head = insn` at the end. An insn
  becomes ready when all of its DEPENDENTS are already scheduled.
* Right before calling `schedule_insn`, `schedule_block` sets
  `INSN_PRIORITY (insn) = LAUNCH_PRIORITY` (`sched.c:3985`) with the comment
  "Give INSN high enough priority that at least one (maybe more) reg-killing
  insns can be launched ahead of all others."
* `schedule_insn` (`sched.c:2604-2631`) calls `adjust_priority (prev)` for each
  predecessor whose last dependent it just satisfied.
* `adjust_priority` (`sched.c:2534-2575`), n_deaths == 0 branch: if
  `birthing_insn_p (PATTERN (prev))` then `INSN_PRIORITY (prev) = max_priority`,
  which at that moment IS LAUNCH_PRIORITY. Comment: "Defer scheduling insns which
  kill registers ... Prefer scheduling insns which make registers live".
* `birthing_insn_p` (`sched.c:2496-2528`): `if (reload_completed == 1) return 0;`
  then for a `SET` of a `REG` that is live at this point,
  **`return (reg_n_sets[i] == 1);`**

So: **the instant the last remaining use of a single-set pseudo is scheduled,
sched1 promotes that pseudo's definition to LAUNCH_PRIORITY and emits it directly
before its first use.** This is unconditional for our 640 constant, whose pseudo
always has exactly one set. It is why holders, `static inline` helper parameters,
declaration order, store spelling and all 840 session-5 forms are inert: none of
them changes `reg_n_sets`.

### Why that one relocation decides the whole function

* **RA sees the sunk layout.** local-alloc allocates by density (short busy
  ranges first), so the 240 and 128 allocnos are placed before the 640 allocno and
  take `$v0` (the first free hard reg). The 640 allocno gets `$v1` **only if its
  live range overlaps one of them**. Measured greg dispositions:
  incumbent (leading 0x10 store) `72 in 16  73 in 5  75 in 3  76 in 2  78 in 2
  80 in 4` — 640 in `$v1`, target's register; target-order form `... 75 in 2
  77 in 2  80 in 2` — all three constants in `$v0`.
* **sched2 then places the `li`.** `birthing_insn_p` is disabled after reload, so
  the only thing left is `INSN_PRIORITY` = longest dependence path from the block
  head (`priority()` walks LOG_LINKS = predecessors, `sched.c:1425`). A `li` with
  no predecessors has priority 1 (the minimum) and is therefore chosen LAST in the
  backward build, i.e. emitted at the block HEAD — exactly where target has
  `addiu $v1,$zero,0x280` as block insn 0. When the 640 instead shares `$v0` with
  the other two constants, the shared hard register chains all three `li`s and
  their stores into one totally ordered dependence path and the `li 640` inherits
  that chain's depth, which is why every target-order form emits it immediately
  after `sb $v0,0xE`.

### The residual reduced to one boolean
`reg_n_sets[<640 pseudo>] == 1` (`sched.c:2516`). If that pseudo had two or more
surviving sets, birthing_insn_p returns 0, the definition stays at the block head
with both uses in the post-load group, its live range overlaps the 240/128 ranges,
local-alloc is forced onto `$v1`, and — per session 3's diagnostic-only `$3` pin,
which already proved the rest — every store lands in target's slot.

### Two "two sets" spellings measured, both KILLED
1. **Redundant duplicate set** (`w = 640;` written again before the second store):
   cse2 deletes it. `base.i.flow` carries a single 640 set; greg shows the usual
   three constant pseudos; the emitted code is bit-identical to the plain
   target-order form. Banked `rejected/redundant-second-set-640-folded-by-cse2.c`.
   (It is also a semantically-purposeless duplicate, so it would not have been
   proposable even if it had worked.)
2. **Variable reuse — one `s16 w` holding 240 and then 640.** This genuinely
   gives `reg_n_sets == 2` (greg drops to two constant pseudos: `75 in 2  76 in 2
   78 in 4`) and does defeat birthing_insn_p, but one C variable is one pseudo and
   therefore ONE hard register, while target holds 240 in `$v0` and 640 in `$v1`.
   Emitted code again bit-identical to the target-order form. Banked
   `rejected/shared-variable-240-640-one-hardreg.c`.

- [s6] The residual of func_8003553C is produced by ONE decision in tools/gcc-2.7.2/sched.c: adjust_priority (2534-2575) promotes a newly-ready insn to LAUNCH_PRIORITY 0x7f000001 (sched.c:187) when birthing_insn_p (2496-2528) is true, and schedule_block builds the block BACKWARD (3813), so the promoted def is emitted immediately before its first use. birthing_insn_p is true for a live SET of a REG iff reg_n_sets[REGNO] == 1. Our 640 pseudo always has exactly one set, so its definition is unconditionally pulled down to its first use.
- [s6] The stock `cc1 -da` dump ALREADY contains the list scheduler's full decision trace in base.i.sched (per-insn priority/ref_count, one ready-list line per scheduling step, plus register-lifetime deltas). No cc1 rebuild is needed and session 3's BB2_PRIO_DEBUG/BB2_RANK_DEBUG operator note is moot. The decisive line for this function is `;; ready list at T-11: 78 (2) 75 (2) 9 (7f000001), now 9 78 75`.
- [s6] CORRECTION to s3: sched1 does NOT reorder this block. base.i.sched is insn-for-insn identical to base.i.combine for both the incumbent and the target-statement-order form. All emitted reordering (including the RGB block leaving field order) is sched2's, post-RA. The ONE thing sched1 relocates is a single-set pseudo's definition, via the LAUNCH_PRIORITY/birthing-insn rule.
- [s6] CORRECTION to s2 (and confirmation of s3's counter-claim, now with the reason): cse does not delete a constant-holder's set. In rtl_HT the set lives at the top of the RTL through rtl/jump/cse/loop/flow/combine with a REG_EQUAL note and is moved down only by sched1.
- [s6] Register dispositions measured directly (greg): incumbent `75 in 3` (640 in $v1, target's register) vs target-statement-order form `80 in 2` (640 in $v0). local-alloc allocates by density, so the short-lived 240 and 128 allocnos take $v0 first and the 640 allocno gets $v1 only when its live range OVERLAPS theirs — which, after the unconditional sched1 sink, requires a 640-valued use in the pre-load group. That is the mechanism behind session 5's 428-form empirical law.
- [s6] sched2 places the `li` purely by INSN_PRIORITY = longest dependence path from the block head; a `li` with no predecessors has priority 1 and is emitted at the block HEAD (target's `addiu $v1,$zero,0x280` is block insn 0). When 640 shares $v0 with the 240 and 128 constants, the shared hard register chains all three `li`s and their stores into one ordered path and the 640 li inherits its depth — which is exactly why every target-order form emits it right after `sb $v0,0xE`.
- [s6] KILLED — redundant duplicate set (`w = 640;` written twice): cse2 deletes the second, reg_n_sets returns to 1, output bit-identical to the plain target-order form. rejected/redundant-second-set-640-folded-by-cse2.c
- [s6] KILLED — one variable reused for 240 then 640: this DOES give reg_n_sets == 2 (two constant pseudos instead of three in greg) and does defeat birthing_insn_p, but one C variable is one pseudo hence ONE hard register, and target needs 240 in $v0 and 640 in $v1. Output bit-identical to the target-order form. rejected/shared-variable-240-640-one-hardreg.c
- [s6] Harness: tmp/grind/func_8003553C/s6/rtlc.sh <form.c> <tag> dumps every RTL pass for a standalone form into s6/rtl_<tag>/ and prints the emitted asm (~2 s); s6/rtl6.sh <tag> does the same for the current src TU. Both must be launched via wsl.exe from the PowerShell tool.
- [s6] No cheat construct was used or proposed: zero regfix/asmfix edits, zero register pins, zero inline asm, zero volatile, zero dead stores. The two new rejected/ forms are ordinary pure-C spellings measured and disproven.

- [s6] Floor unchanged at 2 (sandbox --disable all, 43 build insns / 43 target insns). src/code6cac_b2_pre.c AGAIN held the original cheat-asm carrier at session start (fifth session running); the banked candidate was re-applied, re-measured at 2 before any probe, and is in place in src at session end. The sandbox's reported cheat_asm_stripped: 4 comes from sibling functions in the same file, not from this body.

- [s6] The stock cc1 -da dump ALREADY contains the list scheduler's full decision trace in base.i.sched: per-insn ';; insn[N]: priority = P, ref_count = R', one ';; ready list at T-n: <uid (prio)> ..., now <sorted>' line per scheduling step, and the register-lifetime deltas. No cc1 rebuild is needed, so session 3's operator note about BB2_PRIO_DEBUG/BB2_RANK_DEBUG being absent from the built binary is moot for this class of question.

- [s6] CORRECTION to session 3: sched1 does NOT reorder this block. base.i.sched is insn-for-insn identical to base.i.combine for both the incumbent and the target-statement-order form. Every emitted reordering - including the RGB block leaving strict field order in the target-order forms - is sched2's, post-RA. The ONE thing sched1 relocates is a single-set pseudo's definition, via the LAUNCH_PRIORITY / birthing-insn rule.

- [s6] CORRECTION to session 2 (confirming session 3's counter-claim, now with the reason): cse does not delete a constant-holder's set. In rtl_HT the (set (reg:HI 75) (const_int 640)) lives at the top of the function RTL through base.i.rtl / jump / cse / loop / flow / combine with a REG_EQUAL note, and only sched1 moves it down to its first use.

- [s6] The decisive trace line is ';; ready list at T-11: 78 (2) 75 (2) 9 (7f000001), now 9 78 75'. 0x7f000001 is LAUNCH_PRIORITY (sched.c:187); every other insn in the block carries priority 2. schedule_block sets INSN_PRIORITY = LAUNCH_PRIORITY on the insn it just scheduled (sched.c:3985) and schedule_insn -> adjust_priority (sched.c:2534-2575) propagates that value to any newly-ready predecessor for which birthing_insn_p is true.

- [s6] birthing_insn_p (sched.c:2496-2528) is 'if (reload_completed == 1) return 0;' then, for a live SET of a REG, 'return (reg_n_sets[i] == 1);'. So the sink is unconditional for any single-set pseudo, and it does not exist in sched2. That is why holders, static-inline helper parameters, declaration order, store spelling and all 840 session-5 forms are inert: none of them changes reg_n_sets.

- [s6] Register dispositions read directly out of base.i.greg: incumbent '72 in 16  73 in 5  75 in 3  76 in 2  78 in 2  80 in 4' (640 in $v1, target's register) versus target-statement-order form '72 in 16  73 in 5  75 in 2  77 in 2  79 in 4  80 in 2' (all three constants in $v0). local-alloc allocates by density, so the short-lived 240 and 128 allocnos take $v0 first and the 640 allocno gets $v1 only when its live range OVERLAPS theirs - which, after the unconditional sched1 sink, requires a 640-valued use in the pre-load group. That is the mechanism behind session 5's 428-form empirical law.

- [s6] sched2 places the li purely by INSN_PRIORITY = longest dependence path from the block head (priority() walks LOG_LINKS = predecessors, sched.c:1425). A li with no predecessors has priority 1, the minimum, and backward list scheduling therefore picks it LAST, emitting it at the block HEAD - exactly where target has addiu $v1,$zero,0x280 as block insn 0. When 640 shares $v0 with the 240 and 128 constants, the shared hard register chains all three lis and their stores into one totally ordered dependence path and the li 640 inherits that chain's depth, which is why every target-order form emits it immediately after sb $v0,0xE.

- [s6] The whole remaining residual is therefore ONE boolean: reg_n_sets[<the 640 pseudo>] == 1 at sched.c:2516. Two spellings of 'give it a second set' were measured and both fail - a redundant duplicate assignment is deleted by cse2 (reg_n_sets back to 1), and reusing one variable for 240 then 640 does give two sets but merges the two constants into one pseudo and therefore one hard register, while target needs $v0 and $v1.

- [s6] Harness added: tmp/grind/func_8003553C/s6/rtlc.sh <form.c> <tag> dumps every RTL pass for a standalone form into s6/rtl_<tag>/ and prints the emitted asm (~2 s per form); s6/rtl6.sh <tag> does the same for the current src TU. Both must be launched via wsl.exe from the PowerShell tool (the Bash tool's Git-Bash PATH has no mipsel toolchain).

- [s6] No cheat construct was used or proposed this session: zero regfix/asmfix edits, zero register pins, zero inline asm, zero volatile, zero dead stores, zero holder locals in any banked form. Nothing was left running in the background.

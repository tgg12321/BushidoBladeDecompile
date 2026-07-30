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

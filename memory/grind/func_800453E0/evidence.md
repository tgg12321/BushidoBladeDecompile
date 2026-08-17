# func_800453E0 — evidence ledger

## Status
**MATCHED in pure C, session 1 (2026-08-17, modality `recon`).**
`sandbox func_800453E0 --disable all` == **0**, with `rules_dropped: 7` and
`cheat_asm_stripped: 71` — i.e. the honest cheat-free distance is zero. 76/76
instructions identical to `asm/funcs/func_800453E0.s` register-for-register and
mnemonic-for-mnemonic (artifact: `tmp/grind/func_800453E0/s1/final_diff.txt`).
Matching body: `memory/grind/func_800453E0/candidate.c`, applied in place at
`src/text1a_c.c:1713-1759`.

**Integration note (NOT done by this session):** the 7 stale regfix.txt rules
for this function (regfix.txt:214-224 — two `subst` on addu operand order, one
10-instruction `reorder` of the copy loop, four `subst` register swaps in the
clear section) were written for the OLD C and must be deleted. A full build with
them still present will corrupt the function. This session may not touch
regfix.txt; retirement is the driver's/operator's step.

## What the function is (recovered semantics, not transcription)
`D_800EED10` is an array of 16-byte records; `D_800A33AC` is the live count.
The record layout, corroborated by the five sibling functions in
`src/text1a_c.c` that walk the same table with a 0x10 stride
(func_80045294 at ~1690, func_80045510 at 1760, func_800455AC, and the two
setters at ~1862/1875):

    struct { s16 id; s16 unk2; s32 unk4; s32 amt; void (*fn)(s16, s32); }
    /* offsets 0x0, 0x2, 0x4, 0x8, 0xC — splat names them D_800EED10,
       D_800EED14, D_800EED18, D_800EED1C; D_800EED00 is the record
       immediately BEFORE the table, i.e. &tbl[-1]. */

`func_800453E0(id)` **removes the entry whose `id` matches**: linear-search the
live entries; on a hit call `func_80045294(index + 1, -amt)` to undo that entry's
contribution, shift every following record down one slot, clear the freed tail
slot (`id = -1`, `fn = NULL`), decrement the count, return.

## Inherited state (session 0 / pre-grind)
Honest floor 26, `target_insns` 76 == `build_insns` 76 — so all 26 were
register/operand/scheduling/frame diffs, zero structural. The C was an asm
transcription: locals named `s0`/`s1`/`s2`/`s3`/`v0`/`v1`/`t0` after target's
registers, four goto labels (`L_search`/`L_copy`/`L_clear`/`L_not_found`), the
16-byte record copy hand-written as four named word temps, and a
`volatile s32 sp_pad;` frame-coercion cheat.

## Measured facts

1. **`volatile s32 sp_pad;` was a real, load-bearing cheat AND accounted for 12
   of the inherited 26 diffs.** cc1 on the inherited source printed
   `.frame $sp,48,$31 # vars= 8, regs= 5/0, args= 16, extra= 0` — target's frame
   exactly. But the sandbox's cheat-stripper removes the construct (unused
   volatile-typed scalar / frame coercion family), so the honest build got
   `vars= 0`, frame 40, and every one of the 5 register-save offsets plus both
   `addiu $sp` shifted by 8. **Instrument:** the `.frame` comment IS
   `get_frame_size()`; frame = ALIGN8(vars) + ALIGN8(args) + ALIGN8(gp_regs)
   (memory/project/phantom-frame-slots-gcc272.md). Script:
   `tmp/grind/func_800453E0/s1/frame.sh`.

2. **The correct frame (`vars= 8`, 0x30) falls out HONESTLY from the natural
   struct-array body** — no declaration reserves those 8 bytes. All five save
   offsets (0x18/0x1C/0x20/0x24/0x28) and both `addiu $sp` matched from the very
   first natural-C probe. Another instance of the ordinary GCC 2.7.2
   phantom-slot artifact: "target reserves more than it writes" did NOT imply a
   dead declaration here.

3. **A real 16-byte struct assignment is what produces target's copy loop.**
   Target's inner loop is 4×`lw` then 4×`sw` then `addiu $a3` / `lw` gp /
   `addiu $t0` / `slt` / `bnez` / `addiu $a2` in the delay slot. GCC expands a
   16-byte struct assign as one `movstrsi` block, and the scheduler keeps the
   loads and stores contiguous. The inherited four named word temps let the
   scheduler interleave `t0++` and `a2 += 16` into the store block and put
   `a3 += 16` in the delay slot instead of `a2 += 16` — exactly 10 diffs. One
   change, all 10 gone, and it has never regressed since.

4. **Binding a local `SubEntry *tbl` costs 7 instructions; a constant symbol
   base does not.** With `SubEntry *tbl = (SubEntry *)D_800EED10;` GCC
   strength-reduced to a walking pointer (`lh $v0,0($v1)`, `lw $a1,8($v1)`) and
   CSE'd one base register across the whole function, including the tail clear
   (`addu $v1,$v1,$s2` + two displaced stores instead of target's two
   `lui`/`addu` pairs). Build fell to 66 insns vs target's 76. Replacing the
   local with a constant-address macro (`#define SUBTBL ((SubEntry *)D_800EED10)`)
   restored the symbol-relative addressing and took 66 -> 69 insns, floor
   37 -> 21.

5. **The search loop and the tail clear must address the table through a
   source-level byte offset, while the copy loop uses the struct view.** Target
   re-materializes `%hi(SYM)` + an offset register for every access outside the
   copy loop. Writing those accesses as `*(s16 *)((u8 *)D_800EED10 + off)` /
   `*(s32 *)((u8 *)&D_800EED18 + off)` / `*(s16 *)((u8 *)D_800EED10 + (last << 4))`
   — the idiom used by every sibling in this file — reproduces it exactly, while
   the struct assignment stays in the copy loop. This HYBRID took the build back
   to 76/76 insns and floor 21 -> 18, with insns 13..61 then structurally
   identical to target.

6. **`i + 1` needs its own variable, separate from the shift-loop counter.**
   Target keeps `index + 1` in `$s1` (callee-save, live across the `jal`) and
   copies it into `$t0` (caller-save) for the loop: `addu $t0,$s1,$zero`. Reusing
   one variable for the call argument and the loop counter merges them into a
   single pseudo, which lands `i` in `$s1` and `j` in `$s0` — the mirror image of
   target's `$s0`/`$s1`. Introducing `s32 next = i + 1;` and starting the loop
   `for (j = next; ...)` gave two pseudos and the exact target assignment:
   floor **18 -> 6**, the single largest step of the session.

7. **The outer loop must re-read `D_800A33AC` BEFORE incrementing `i`.** With
   `for (...; i++, off += 0x10)` the increment is emitted first, leaving nothing
   to fill the `lw`'s load-delay slot — an unfillable maspsx `nop` and one extra
   instruction. Target's order is `lw` gp / `addiu $s0,$s0,1` / `slt` / `bnez`.
   Restructuring to a do-while whose body ends `count = D_800A33AC; i += 1;`
   with an `if (i >= count) return;` entry guard — the exact shape of the
   already-matching sibling **func_80045510** (src/text1a_c.c:1760-1763) — filled
   the slot. Same mechanism as
   `.claude/rules/loop-counter-fills-load-delay.md`, discovered on this
   function's own callee func_80045294. Floor **6 -> 3**.

8. **The byte offset must be a strength-reduction giv, not a second source
   induction variable.** With `off` advanced in the for-increment, `off = 0` was
   emitted in the pre-header BEFORE the LICM-hoisted base addresses; target emits
   `addu $v1,$zero,$zero` AFTER them. Writing `off = i << 4;` at the top of the
   loop body instead makes `off` a giv, so loop.c creates its zero-init in the
   pre-header after the movables. Fixed the ordering exactly. Floor **3 -> 1**.

9. **The last diff was a reloc addend, and the spelling that removes it is the
   one whose two base pointers derive from ONE symbol.** `SUBTBL[j-1] = SUBTBL[j]`
   emits `lui %hi(D_800EED10)` / `addiu` with a LO16 reloc against D_800EED10 and
   addend -16. The resolved immediate is identical to target's
   `%lo(D_800EED00)` (0xED10 - 0x10 = 0xED00) and `%hi` is 0x800F either way — so
   the FINAL LINKED BYTES were already correct, but `engine/score.py` does not
   mask data LO16 addends and counted it as 1 (the data analogue of
   memory/sandbox-lo16-text-addend-false-distance.md). Writing the loop against
   the D_800EED00-based view — `SUBSLOT[j] = SUBSLOT[j + 1]`, semantically
   identical since `SUBSLOT[n+1] == SUBTBL[n]` — makes both base pointers derive
   from that one symbol (`$s2 = %lo(D_800EED00)`, `$s3 = $s2 + 0x10`, matching
   target's roles) and emits the addend-0 reloc. Floor **1 -> 0**.

## Floor path (all measured this session, one attributable change per step)
| floor | form |
|---|---|
| 26 | inherited asm transcription + `volatile s32 sp_pad;` cheat |
| 37 | natural struct array via a local `SubEntry *tbl` (frame fixed honestly; walking pointer cost 10 insns) |
| 21 | same, but constant-symbol base macro instead of the local pointer |
| 18 | hybrid: byte-offset casts for search+clear, struct assignment for the copy loop (76/76 insns) |
| 6 | `next` split out from the shift-loop counter `j` |
| 3 | do-while + `count` re-read before `i += 1` (sibling func_80045510's shape) |
| 1 | `off = i << 4` as a giv instead of a second induction variable |
| **0** | shift-down loop written against `SUBSLOT` (D_800EED00 base) so both base pointers share one symbol |

## Cheat posture
The final body contains no inline asm, no register pins, no volatile, no dead
stores, no unused declarations, no frame coercion, and claims NO sanctioned
carve-out and no FAKE construct. It DELETES the inherited `sp_pad` cheat. Full
six-test walkthrough: `memory/grind/func_800453E0/self_vet.md`.

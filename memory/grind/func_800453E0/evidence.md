# func_800453E0 — evidence ledger

## Status
**MATCHED in pure C. Honest floor 0.**
Session 1 (2026-08-17, modality `recon`) closed 26 -> 0 but its submission was
bounced by the layer-1 cheat-reviewer on a CITATION defect, not a construct
defect: its H5 justified the outer-loop shape by citing
`.claude/rules/loop-counter-fills-load-delay.md`, a file that does not exist
(the technique appears only as a summary line in the index rule
`.claude/rules/codegen-technique-index.md`). That citation is BANNED for this
function.

Session 2 (2026-08-17, modality `recon`) did not argue the citation. It
reproduced distance 0, then ran a SUBTRACTION pass over session 1's constructs
and **removed the construct the citation was attached to** — plus one more —
after measuring both to be byte-neutral. Final state:

`sandbox func_800453E0 --disable all` == **0**, `rules_dropped: 7`,
`cheat_asm_stripped: 71`, `target_insns` 76 == `build_insns` 76, frame 0x30.
Matching body: `memory/grind/func_800453E0/candidate.c`, applied in place at
`src/text1a_c.c:1713-1755`. Disassembly + relocs:
`tmp/grind/func_800453E0/s2/final_func.txt`.

**Integration note (NOT done by any grind session):** the 7 stale regfix.txt
rules for this function (regfix.txt:214-224 — two `subst` on addu operand order,
one 10-instruction `reorder` of the copy loop, four `subst` register swaps in
the clear section) were written for the OLD C and must be deleted. A full build
with them still present will corrupt the function. Grind sessions may not touch
regfix.txt; retirement is the driver's/operator's step.

## What the function is (recovered semantics, not transcription)
`D_800EED10` is an array of 16-byte records; `D_800A33AC` is the live count.
The record layout, corroborated by the five sibling functions in
`src/text1a_c.c` that walk the same table with a 0x10 stride
(func_80045294 at 1666, func_80045510 at 1759, func_800455AC, and the two
setters near the end):

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
   first natural-C probe, and still do in the session-2 body
   (`addiu sp,sp,-48` at 0x275c of `s2/final_func.txt`). Another instance of the
   ordinary GCC 2.7.2 phantom-slot artifact: "target reserves more than it
   writes" did NOT imply a dead declaration here.

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
   local with a constant-address macro restored symbol-relative addressing and
   took 66 -> 69 insns, floor 37 -> 21.

5. **The search loop and the tail clear must address the table through a
   source-level byte offset, while the copy loop uses the struct view.** Target
   re-materializes `%hi(SYM)` + an offset register for every access outside the
   copy loop. Writing those accesses as `*(s16 *)((u8 *)D_800EED10 + off)` /
   `*(s32 *)((u8 *)&D_800EED18 + off)` / `*(s16 *)((u8 *)D_800EED10 + (last << 4))`
   — the idiom used by every sibling in this file — reproduces it exactly, while
   the struct assignment stays in the copy loop. This HYBRID took the build back
   to 76/76 insns and floor 21 -> 18 (session 1). **Session 2 re-measured it as
   a subtraction from the ZERO baseline:** rewriting ONLY the search loop's two
   accesses as `SUBSLOT[i + 1].id` / `SUBSLOT[i + 1].amt` took the build to 72
   insns and distance **11**. Banked as
   `rejected/struct-idiom-search-loop-costs-11.c`. This spelling is genuinely
   load-bearing.

6. **The table byte offset must be a strength-reduction giv, not a second source
   induction variable.** With `off` advanced in the for-increment, `off = 0` was
   emitted in the pre-header BEFORE the LICM-hoisted base addresses; target emits
   `addu $v1,$zero,$zero` AFTER them. Writing `off = i << 4;` at the top of the
   loop body instead makes `off` a giv, so loop.c creates its zero-init in the
   pre-header after the movables. Fixed the ordering exactly. Floor **3 -> 1**.
   Retained in the final body.

7. **The last diff was a reloc addend, and the spelling that removes it is the
   one whose two base pointers derive from ONE symbol.** `SUBTBL[j-1] = SUBTBL[j]`
   emits `lui %hi(D_800EED10)` / `addiu` with a LO16 reloc against D_800EED10 and
   addend -16. The resolved immediate is identical to target's
   `%lo(D_800EED00)` (0xED10 - 0x10 = 0xED00) and `%hi` is 0x800F either way — so
   the FINAL LINKED BYTES were already correct, but `engine/score.py` does not
   mask data LO16 addends and counted it as 1 (the data analogue of
   memory/sandbox-lo16-text-addend-false-distance.md). Writing the loop against
   the D_800EED00-based view — `SUBSLOT[j] = SUBSLOT[j + 1]`, semantically
   identical since `SUBSLOT[n+1]` is table entry `n` — makes both base pointers
   derive from that one symbol (`$s2 = %lo(D_800EED00)`, `$s3 = $s2 + 0x10`,
   matching target's roles) and emits the addend-0 reloc, visible in
   `s2/final_func.txt`. Floor **1 -> 0**.

8. **(SESSION 2, KILL) The outer loop does NOT need the do-while +
   `count = D_800A33AC;` re-read + entry guard.** Session 1 claimed that shape
   was required to fill the `lw`'s load-delay slot (its H5 — the claim carrying
   the banned citation). Measured from the zero baseline: replacing the entire
   scaffold with a plain `for (i = 0; i < D_800A33AC; i++)` and deleting `count`
   holds sandbox `--disable all` at **0**, 76/76 insns, and the two objects'
   disassemblies `diff` **clean** (`s2/forloop_func.txt` vs `s2/dowhile_func.txt`).
   A live control probe (`i + 1` -> `i + 2`, score 1) proves both builds were
   fresh. Once `off = i << 4` is a giv (fact 6), the loop condition's own re-read
   of the global supplies the delay-slot filler either way. The simpler `for`
   form is what is in `src/` and in `candidate.c`.

9. **(SESSION 2, KILL) `i + 1` does NOT need its own named local.** Session 1
   claimed a separate `s32 next = i + 1;` was required to keep the call argument
   and the shift-loop counter in two pseudos (its H4; true at the intermediate
   floor of 18, where sharing cost 12). Measured from the zero baseline:
   substituting `i + 1` inline at both use sites AND deleting the declaration
   holds distance at **0**, 76/76 insns, in both intermediate states. GCC creates
   the two pseudos from the two `i + 1` expressions on its own.

## Floor path
| floor | form |
|---|---|
| 26 | inherited asm transcription + `volatile s32 sp_pad;` cheat |
| 37 | natural struct array via a local `SubEntry *tbl` (frame fixed honestly; walking pointer cost 10 insns) |
| 21 | same, but constant-symbol base macro instead of the local pointer |
| 18 | hybrid: byte-offset casts for search+clear, struct assignment for the copy loop (76/76 insns) |
| 6 | `next` split out from the shift-loop counter (session 1; later shown path-only — fact 9) |
| 3 | do-while + `count` re-read before `i += 1` (session 1; later shown path-only — fact 8) |
| 1 | `off = i << 4` as a giv instead of a second induction variable |
| **0** | shift-down loop written against `SUBSLOT` (D_800EED00 base) so both base pointers share one symbol |
| **0** | *(session 2)* same, minus the do-while scaffold and minus `next` — byte-identical, four locals instead of six |

## Cheat posture
The final body contains no inline asm, no register pins, no volatile, no dead
stores, no unused declarations, no frame coercion, and claims NO sanctioned
carve-out and no FAKE construct. Every local (`i`, `j`, `last`, `off`) is read.
It DELETES the inherited `sp_pad` cheat. Session 2 additionally removed the two
constructs it measured to be byte-neutral, so nothing in the body survives that
a cheaper spelling could replace. Full six-test walkthrough:
`memory/grind/func_800453E0/self_vet.md`.

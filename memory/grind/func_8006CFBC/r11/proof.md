# func_8006CFBC — Ruling 11 proof for `temp` (2026-09-30, laneC)

Body under judgment: `memory/grind/func_8006CFBC/candidate.c` (sandbox 0, 218/218).
The retro-audit objection (reopen feeb74346): the 3-write local `value` had no
admitting ruling. The same local is kept, renamed `temp`, typed `s8 *` (so the two
casts `(s32)`/`(s8 *)value` go), annotated, and submitted under Ruling 11.

## Why not Rulings 5-10 (Ruling 11 applies only when none admits it)
- Ruling 5: the write is `temp = (s8 *)s.header + 0xC;` at all three sites; the
  record is picked by the member store before it, not by a constant subscript in
  the write (1(b) needs an integer-constant subscript selector) -> not Ruling 5.
- Ruling 5 extension: (B) needs the member store `s.header = ...` at the same
  nesting level as the write on every path; site 3 assigns `s.header` in the arms
  of an if/else chain. (D) needs every re-selection to pick a different element;
  site 2 (`table[12]`) runs for row 0 and row 1 of one outer iteration, and site 3
  can pick `table[17]` for both rows -> fails (B) and (D).
- Ruling 6: the writes are not in mutually exclusive regions (three loops in
  sequence) -> fails (A).
- Ruling 8: vmNoiseOn only. Ruling 10: no public original source.
- Ruling 9: (e) fails at site 1 -- on the path where neither row bit is set for a
  column, the `s.table` store of that column is overwritten by the next column's
  before anything reads `s` (no `func_8007352C(&s)` call on that path).

## (A) fresh local
Function-scope `s8 *temp;` (its writes are in three sibling top-level loops, so the
function body is the innermost enclosing scope). Not a parameter/global/static/
register; no `&temp`.

## Values (Ruling 11's definition: writes reaching a common read)
Three writes, each read only by the `s.table = temp;` right after it, so three values:
- V1 (0x8006D04C `addiu $v0,$v1,0xC`): `s.header + 0xC` for `table[column + 8]`.
- V2 (0x8006D164 `addiu $v0,$v1,0xC`): `s.header + 0xC` for `table[12]`.
- V3 (0x8006D23C `addiu $v0,$v1,0xC`): `s.header + 0xC` for `table[17|18|19]`
  (s.header reloaded at 0x8006D22C `lw $v1,0x18($sp)`).

## (B) every write live; no re-store on every path
(1) each write is read by the next statement (`s.table = temp;`).
(2) paths where temp holds a different value at the write:
- V1: first execution (outer 0, column 0): temp not yet written. Column c > 0:
  temp holds V1 for table[c + 7], the write stores it for table[c + 8].
- V2: outer 0, row 0 with count[0] == 0: temp holds V1 for table[11] (column 3).
- V3: row 0: temp holds V1 (table[11]) or V2 (table[12]); row 1 after row 0 took
  table[18] (result bit 0 clear, not 0x50005) and row 1 takes table[19] (bit 1 set):
  holds header table[18] + 0xC, stores table[19] + 0xC.

## (C) one-variable-per-value spelling
`spellings/split3.c`: cells1 / cells2 / cells3, each declared in the block of its
one write. Same statement list as the reuse body (declarations and identifiers only).
Each value is an `addiu` in the target (addresses above).

## (D)(1) dumps — commands
`r11/tools/dump.sh <cand> <tag>` splices the body over the INCLUDE_ASM line of
src/text1b_tu1c.c, preprocesses with the Makefile CPP flags and runs
`tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls
-fno-builtin -w -mel -msoft-float -dS -dl -dg -dR -dc`. `r11/tools/sdbg.sh <tag>` reruns
the instrumented cc1 with `BB2_ALLOC_DEBUG=1 BB2_SCHED_DEBUG=1` (its .s equals the dump
run's apart from the options comment). Reuse = tmp c1.c = candidate.c without comments;
split = spellings/split3.c. Full sched1 traces: `dumps/sched1_trace_reuse.txt`,
`dumps/sched1_trace_split.txt`; excerpts: `dumps/excerpts.txt`.

## (D)(2) mechanism
1. sched.c `adjust_priority` (tools/gcc-2.7.2/sched.c:2543, boost at :2584-2590) raises
   an insn that becomes ready to `max_priority` when `birthing_insn_p` (:2505) holds:
   it sets a live pseudo with `reg_n_sets == 1` (:2526).
2. Reuse: `temp` is one pseudo (reg 79) set three times -> `ADJPRI insn=71/301/435
   birth=0`. At site 1 (block 2, backward list scheduling) the table store 74 is picked,
   then the header store 66 (tie at pri 2, SELBEST), then the add 71: final order
   add 71, store 66, store 74 = the target's `addiu $v0,$v1,0xC; sw $v1,0x18($sp);
   sw $v0,0x1C($sp)` (0x8006D04C..54). Site 2 the same (301 before 296).
3. Split: each per-value local is set once (regs 85, 175, 202) -> `ADJPRI insn=72/304/440
   birth=1`, so the add is picked right after its consumer store and lands BELOW the
   header store (order 67, 72, 75).
4. local-alloc.c: with the add below the header store, the loaded header (reg 92/190/223)
   dies at the add, and `block_alloc` -> `combine_regs` (local-alloc.c:1784) ties the
   block-local destination to it: .greg dispositions 85/92, 175/190, 202/223 all in $3
   (`addiu $v1,$v1,12; sw $v1,28(sp)` against the target's `$v0`). The reuse pseudo
   79 is referenced in several blocks, so it has no local quantity: combine_regs refuses
   it (local-alloc.c:1836, `reg_qty[sreg] == -1`) and global.c seats it in $2 = $v0.
   Site 3 (V3) is this second half alone: the header is reloaded from s.header and dies
   at the add in both spellings, so only the tie differs (the split's one operand-only
   hunk).

## (D)(3) necessity (Q31: mechanism + banked search)
The property the target depends on: the add's destination pseudo is set more than once
(no sched1 birth boost at V1/V2) and is referenced in more than one basic block (no
local-alloc tie at V3). A one-variable-per-value spelling gives each value its own
variable; each of these values has exactly one write and is read in its own block
only, so its pseudo has `reg_n_sets == 1` and a local quantity, whatever its
declaration position, scope, type or statement order. Banked counting spellings, none
reaching the target (sandbox --disable all, 218 target insns):
| spelling | score |
|---|---|
| split3.c (block-local cells1-3) | 8 |
| split3fs.c (function-scope cells1-3) | 8 |
| split3_s32.c (s32 locals + casts) | 8 |
| split3_hdr.c (per-site header and cells locals) | 8 |
| split3_hdr_rev.c (same, table store first) | 14 |
| direct.c (no local: `s.table = (s8 *)s.header + 0xC;`) | 8 |
| nov2.c (`(s8 *)(s.header + 3)`) | 8 |
| vA.c / vB.c (both stores read `table[column + 8]`) | 17 / 16 |
| vC.c (chained `s.table = (s8 *)(s.header = ...) + 0xC`) | 8 |
| vD.c (`&s.header[3]`) | 8 |
| t1.c (`s.table = (s8 *)s.header; s.table += 0xC;`) | 8 |
| t2.c (per-value compound split `cells = hdr; cells += 0xC;`) | 9 |
No FAKE-construct spelling was measured or set aside (Q30).

## (D)(4) measured alternatives
- full one-variable-per-value: split3.c 8.
- ablation (each value split out alone, the rest shared): abl1.c (V1 alone) 3,
  abl2.c (V2 alone) 3, abl3.c (V3 alone) 2.
- structural respellings: the table above; t3.c (the reuse with the compound split) 7.
- permuter: see "(D)(4) permuter harvest" below.

## (E) name
`temp` -- Ruling 11 (E)(i) generic scratch word.

## (F) annotation
At the declaration in candidate.c: names the three values, cites Ruling 11 and this file.

## (D)(4) permuter harvest (2026-09-30)
Campaign `split3-per-value`: tmp/func_8006CFBC/perm_split (minimal TU, the build cc1 +
the Makefile maspsx/multu_pad/as recipe for src/text1b_tu1c.c: r11/tools/mkws.sh,
mkperm.py, perm_compile.sh; the reuse body compiles there byte-identical to the target,
0 differing objdump lines). Seeded from spellings/split3.c (r11/permuter/base.c), -j2,
stack diffs on. Base 420 (permuter score). 25,554 iterations in 1,326 s, stopped (no
novel find for the last ~16 min). No find reached 0. Finds: two 420s (formatting-only)
and one 397 (r11/permuter/find-397-alias.c): `s8 **new_var = &cells1; s.table =
*new_var;` -- an address-taken alias of the per-value local, a cheat-form (address
coercion), rejected per no-new-park-categories vetting.

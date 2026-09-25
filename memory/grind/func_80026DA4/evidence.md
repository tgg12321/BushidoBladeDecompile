# func_80026DA4 — evidence (manual session 2026-09-24)

Called by func_8002C61C each frame for D_80101F32 modes 0xF and 0x1C-0x21. Mode 0x1C sets +0x286 = 3 on record[D_800A3876] and 4 on the other; other modes run a 2-record drift loop (+0x28C, +0x134/+0x13C via
the Judge sin/cos table), a counter (D_800A389C) that after 43 counts gives the record with the
larger +0x28C +0x286 = 3 (the other 4),
and a distance/flag check. The tail (unless D_800A3910 is counting down)
places a point between the two records offset by a 6-row table picked from
the mode, then calls func_80032854.

## Result
COMPLETED-C: full build SHA1 == oracle 62efab4f73f992798c43e8c730aa43baa10bb4fa
(verify-oracle --rebuild --allow-dirty with the landing applied). Pre-landing
sandbox floor 10 was entirely the jump-table addend artifact: the sandbox kept
the old INCLUDE_RODATA block (table + string, 108 bytes) in front of GCC's own
table, so our `lw v0,%lo(.rodata+108)` compared against the target's named
`jtbl_8001042C` (0 source-level hunks, 1 operand-only).

## Floor trajectory (sandbox --disable all, HEAD dd272b4a1)
- 342: INCLUDE_ASM stub (no prior C anywhere in history; the April "Match"
  was an empty body + asmfix).
- 52: first transcription.
- 34: `if (D_800A3910 == 0) {switch...} else D_800A3910--;` (branch sense),
  flipped `>` compares, struct-array table access.
- 17: loop written `p = s0 + i * 0x44C` (unbiased giv + surviving counter;
  a walking `p += 0x44C` biv gets eliminated into a +0x13C-biased giv).
  Only residual: `la s0,D_80101EC8` hoisted above `jal func_8002BEA0`.
- 10 (= 0 real): func_80027A58 / func_80032854 called with NO prototype in
  scope (both are defined later in this file).

## Mechanism of the last hoist (why every variable-split failed)
sched1 (tools/gcc-2.7.2/sched.c) schedules backward. In the entry block the
`s0 = base` set has no in-block consumer, priority 1. The call gets the
adjust_priority birthing boost because it sets hard reg $v0 and
reg_n_sets[$v0] == 1 (only func_8002BEA0's value is used). The store to
D_800A3824 beats `s0 = base` on the equal-priority tie (potential_hazard
prefers the memory-unit insn), so `s0 = base` is picked last = placed first.
With implicit declarations, the two later calls become call_value insns that
also set $v0, reg_n_sets[$v0] > 1, the call loses its boost, and the LUID
tie-break puts `s0 = base` after the jal. Verified with BB2_SCHED_DEBUG.

Ruled out along the way (all worse, 29-93): splitting top/0x1C/middle/tail
pointers into separate variables (18-way role grid + 24 0x1C spellings +
6 entry forms). A non-call-crossing pointer can never get $s0 (global.c
seeds regs_used_so_far with every call-used reg), and CSE only produces the
target's `move v1,s0; addu s0,v0,s0; move s1,v1` temp when the selected record's own
variable holds the base. So the pointer is ONE variable, multi-set, and the
hoist had to be fixed from the call side. A `s16` return type with
`return --D_800A3910` also fixes the hoist but adds an sll/sra the target
lacks.

## Object model changes (landed with the function)
- include/code6cac.h: D_8008EB54 -> `Tbl8008EB54Entry D_8008EB54[6]`
  ({s16 unk0; s16 unk2;}), D_8008EB6C -> `u8 D_8008EB6C[6]` (the dossier's
  INDEXED-ACCESS signals; row count = the 6 switch kinds; 6*4 bytes ends
  exactly at 0x8008EB6C). Neither had any C user before.
- The named symbol single_dojo_yaburi_char_id_tbl (0x8008EB56) was
  D_8008EB54[0].unk2. It came from the Kengo global list
  (Kengo/kengo_globals_full.txt:952, PS2 address, no link to this storage)
  and the column is a y offset, not character ids. Retired per the
  aggregate-merge prong (c) after layer-2 FAIL #1: C extern deleted, rows
  removed from named_syms.txt / symbol_addrs.txt / undefined_syms_auto.txt,
  dlabel folded into D_8008EB54 (24 bytes) in asm/data/7D920.data.s, and
  asm/funcs/func_80026DA4.s now spells it %hi/%lo(D_8008EB54 + 0x2).
  The unbuilt monolith asm/6CAC.s still carries the old name (history).
- jtbl_8001042C is now emitted by GCC; the string D_80010478 that shared its
  .s file (still used by INCLUDE_ASM func_8002A458) moved to
  asm/rodata/D_80010478.s, included right after the function.
  asm/rodata/jtbl_8001042C.s is left on disk unreferenced, like
  jtbl_80010498.s.

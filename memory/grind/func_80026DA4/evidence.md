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

## 2026-09-30 — ff-c, retro-audit fix-forward (Q37 class B; tmp/audit-2026-09-29/review/batch_03.md)

Audit FAIL: `s0`/`s1` multi-written with no admitting ruling. The post-loop `s0 = base` re-stores
the value s0 holds on every path (Ruling 5 2(c) / Ruling 11 (B)(2) fail outright), so Ruling 11's
(D) package cannot admit it; route taken: owner Q51 (SOTN reuse citation) with Q53 paperwork.
CONCERN: func_80027A58 / func_80032854 called with no prototype in scope.

### Spellings (engine.cli sandbox --disable all --candidate; bodies in ff-c-2026-09-30/)

| spelling | score (insns 342 target) |
|---|---|
| v0 control (main verbatim) | 0 (342) |
| v1 no post-loop `s0 = base` | 61 (340) |
| v2 renamed s0->rec, s1->other (identifiers only) | 0 |
| v3 v2 + `extern void` prototypes of func_80027A58/func_80032854 | 7 (342) — the `la s0` hoist above `jal func_8002BEA0` |
| v3b v2 + `extern s32` prototypes (callee is defined void: a lying prototype; probe only) | 0 |
| v5 0x1C arm on its own `win`/`lose` locals, both re-sets kept | 16 |
| v5b same, pre-tail re-set dropped (then redundant on every path) | 18 (340) |
| v5c same, both re-sets dropped | 82 (335) |
| v6 one pointer pair per section (base / win,lose / rec,other / a,b) | 66 (336) |
| **v7 v2 + FAKE/SOTN annotations (proposed)** | **0 (342)** |

Plus the 2026-09-24 split grid above (18-way role grid, 24 0x1C spellings, 6 entry forms): 29-93.

### SOTN reuse citation (Q51; Q50 conditions 1-4), read at db41b28

- src/weapon/w_011.c:343-348, EntityWeaponShieldSpell, case 5 (config/splat.us.weapon.yaml:254
  `[0x51CDC, c, w_011]`; no INCLUDE_ASM / NON_MATCHING / #if / FAKE marker anywhere in the file):
  `prim = &g_PrimBuf[self->primIndex]; prim->drawMode |= DRAW_HIDE; self->ext.shield.unk90 = 0;
  prim = &g_PrimBuf[self->primIndex]; prim = prim->next;` — a record pointer re-assigned, on one
  straight path, the table element it already holds, then re-pointed at another element. Same
  variable also takes `&g_PrimBuf[...]` in other switch arms and walks `prim->next`.
- src/boss/rbo0/e_fake_sypha.c:807-820, EntityHolyLightning (config/splat.us.borbo0.yaml:99
  `[0x17804, c, e_fake_sypha]`; no guard or marker): `var_s1` walks a Point16 array in a loop,
  is set to `&self->ext.sypha.red`, written through, then re-set to the same address (813/816);
  `prim = self->ext.prim;` is likewise re-stored at 817/820.
Ours: `rec` = record 0 at entry, the D_800A3876 record in the 0x1C arm, re-set to record 0 after
the drift loop (redundant on that path, as w_011.c:346) and before the tail (redundant on the
else/0xF paths, a real change on the 0x1C path); `other` = rec's partner, re-derived at each re-set.
Scan used to find candidates: ff-c-2026-09-30/sotn_redundant_ptr.py (every hit read by hand).

### Why the re-sets are real (target bytes)
The target materializes `lui/addiu $s0, D_80101EC8` three times: asm/funcs/func_80026DA4.s:9-10
(entry), :133-134 (after the loop, reached only from the loop exit), :249-250 (at .L80027124, the
join of the 0x1C arm, the else arm and mode 0xF, before the tail). cse.c works per extended basic
block, so a C re-assignment after the loop / at the join emits a fresh lui/addiu; with no re-set
(v1) the base is not re-materialized and the allocation shifts (61).

### Implicit-int calls (CONCERN, unchanged)
With `extern void` prototypes the build is 7 off (`la s0` hoisted above `jal func_8002BEA0`; the
2026-09-24 BB2_SCHED_DEBUG mechanism above: without call_value sets of $v0, reg_n_sets[$v0] == 1 and
the first call gets sched.c adjust_priority's birthing boost). Both callees lie later in the same
original segment (0x80027A58, 0x80032854 > 0x80026DA4), so a call before any declaration is what a
single-file C89 original produces. Declaring them `s32` would also reach 0 but contradicts their
matched `void` definitions, so it is not used. Kept as is, with the existing comment.

### 2026-09-30 — layer-2 rev-26da4: FAIL (Q51 condition 2) -> REOPENED under owner Q37

Submitted: v7 (rec/other + FAKE/SOTN annotations; applied, rebuilt == oracle, sandbox 0, hash
8abdbed8e9abbd97), banked as rejected/q51-rev-26da4-2026-09-30.c. The landed 6be239081 body is banked
as rejected/retro-audit-2026-09-29.c.

Reviewer's finding (Q51 condition 2, "does the same thing when read"): the load-bearing reuse is
rec/other carrying the D_800A3876-SELECTED record in the 0x1C arm (the `rec = base + idx * 0x44C`
write) and record 0 everywhere else, then restored to record 0 at the three-path join before the tail.
That is a role change (selected record -> fixed record 0). Neither SOTN cursor does that: w_011.c's
`prim` re-stores the same element and then walks ->next; e_fake_sypha.c's `var_s1` re-stores the same
member address. The 0x1C-arm write has no citation, and the pre-tail tag (e_fake_sypha.c:816) does not
correspond to a join restore.

Implicit-int facts (for the next session): since the 2026-09-30 TU split, code6cac_b.c has NO
declaration of func_80027A58 / func_80032854 at all (they are defined `void` in code6cac_b_tu2.c);
`extern void` prototypes cost 7 (the `la s0` hoist). The SOTN cast precedent (st warp.c:10
`ImplicitGetDistanceToPlayerX`) is an s16 -> int return with the prototype in scope, not an
undeclared call to a void callee, so it does not cover this case.

Path to PASS (reviewer + author): either (a) a SOTN citation of a pointer re-pointed at an
index-selected table element in one arm and restored to a fixed element at a later join, plus an
admissible answer for the implicit-int calls; or (b) a Ruling 11 (A)-(H) package, which the
post-loop re-set blocks outright ((B)(2): it re-stores the held value on every path) unless a
spelling without that re-set is found; or (c) a different object model for the 0x44C-stride player
table (D_80101F32 / D_80101F08 / D_80102154 / D_801025A0 / D_8010214E / D_8010259A / D_8010237E are
per-word splat scalars inside records 0/1 of that table, i.e. split-scalars-hide-aggregate), which may
change the allocation the re-sets are compensating for.

Q37 fallback: body -> INCLUDE_ASM with INCLUDE_RODATA("asm/rodata", jtbl_8001042C) restored (the
landing had moved the jump table into GCC's own rodata); reopened.

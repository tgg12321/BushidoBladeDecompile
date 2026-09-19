# func_8002DAD0 — evidence log

## [s1] recon session — object model + structural derivation

**OBJECT MODEL:** `D_8008D118` — census row flags this as a byte LUT
("distance-squared magnitude/log2 lookup"), no header declaration. MATCHES:
the established codebase idiom is `extern u8 D_8008D118;` used as
`*(&D_8008D118 + idx)` / `*(((u8 *)&D_8008D118) + idx)` pointer arithmetic
(confirmed at src/code6cac.c:19,756,783 and, decisively, already present
in THIS TU at src/code6cac_b.c:278 and reused by 6+ sibling functions in
the same file, including the byte-identical shape used by func_8002E838
at line 1468/1490). No declaration-fix hypothesis needed — the extern
already exists in scope for func_8002DAD0's insertion point; candidate.c
uses it unchanged. This satisfies the mandatory DATA MODEL check for this
session.

**Cluster membership confirmed.** `.claude/rules/cop2-addressing-preamble-cluster.md`
lists func_8002DAD0 as an active member (LZCS site at old L125) of the
28-function `addu $t4,$aN,$zero`+cop2 cluster (owner ruling 2026-08-17,
widened 2026-09-01). The function actually carries FIVE such preamble
islands, not one: (1) a partial gte_SetRotMatrix-shaped preamble loading
only 3 words into ctrl regs 0/2/4 (R11,R22,R33 diagonal) from vecA,
feeding (2) a GTE `OP` (outer/cross-product, opcode 0x0C, raw word
0x4B70000C — NOT the MVMVA the rest of the function uses) fed by IR1-3
loaded directly (lwc2 $9/$10/$11) from vecB, storing the raw MAC1-3 cross
product to obj+0xC8/CC/D0 via gte_stlvnl; (3) the LZCS/LZCR leading-zero
block, byte-identical to func_8001A67C (src/code6cac.c) and func_8002E838
(this file, line ~1472); (4) a FULL 5-word gte_SetRotMatrix loading the
just-computed rotation matrix (built via RotMatrixY+RotMatrixX on an
identity, at obj+0xD8) into ctrl 0-4; (5)/(6) two gte_ldv0+MVMVA
(sf=1,mx=rotation,v=V0 — word 0x4A486012, same as func_800203B4 and
func_8002E838)+gte_stlvnl sequences that rotate vecA and vecB in place.
All six islands are verbatim reuses of PsyQ SDK macro bodies already
authorized/matched in func_800203B4 (src/code6cac.c) and, even more
directly, func_8002E838 (src/code6cac_b.c:1451-1552) — func_8002E838 is
almost exactly the SECOND HALF of this function's algorithm (single-vector
version: one ratan2, no cross-product/OP stage, no second gte_ldv0/MVMVA
pass). func_8002E838's matched C was used as the direct template for the
LZCS block, the identity-matrix stores, and the RotMatrixY/RotMatrixX call
shape in this function's candidate — all copied verbatim (same field
offsets 0xD8-0xE8, 0xF8, 0xFA, same table-lookup shape for `dist`).

**Structural derivation (algorithm).** `obj` is an unspecified struct (no
existing prototype for func_8002DAD0 anywhere in the tree — first
declaration). Fields used: 0x60/0x64/0x68 = three `s32*` position
pointers (p0=self/origin, p1, p2); 0xA8/0xAC/0xB0 = vecA = p1-p0 (s32[3]);
0xB8/0xBC/0xC0 = vecB = p2-p0 (s32[3]); 0xC8/0xCC/0xD0 = cross = vecA×vecB
(computed via GTE OP, then reused in-place as scratch: >>=6 on each axis,
then ratan2 inputs); 0xD8..0xE8 = a 3x3 rotation matrix (5 packed s32
words, later reinterpreted as the standard 20-byte MATRIX layout);
0xF8/0xFA = two s16 angle fields (0x800-angle1, 0x800-angle2). Verified by
finding func_8002E838 (same file) implementing the exact same
distance/log2/RotMatrixY+X/gte_SetRotMatrix+ldv0+MVMVA+stlvnl tail with
identical field offsets and byte-identical matched C — this function is
almost certainly the two-point ("aim/orient toward two targets") sibling
of that one-point function. kengo naming hint: `sa_tan0/saTan0KiWareMoveA`
(character "santan"/"sa_tan0" move-break motion).

**Measured floor drop this session: 204 → 44 → 25** (see hypotheses below
for each step). `sandbox --disable all --diff` confirms the residual is
ENTIRELY inside one straight-line block (target insns 88-119, the
dist_sq/log2 computation immediately after the first ratan2 call) — every
island, the two vector-diff blocks, the near-zero-cross bailout, the
identity-matrix stores, both RotMatrix calls, and both tail
gte_ldv0+MVMVA+stlvnl rotations are ALREADY byte-exact (confirmed by the
hunk list touching only offsets 0/3/44-56/70-119, nothing beyond insn 119
except the tail frame-size hunks 18-23 which were ALSO fixed this
session — see hypothesis 2). Remaining hunks are 5 source-level + 5
operand-only, all inside the dist_sq scratch-block reload pattern.

- [s1] OBJECT MODEL: D_8008D118 MATCHES the established declared shape -- extern u8 D_8008D118; already declared in this TU (src/code6cac_b.c:278) and used via *(&D_8008D118 + idx) / *(((u8*)&D_8008D118) + idx) pointer arithmetic by 6+ sibling functions in the same file, including the byte-identical shape in the matched func_8002E838. No declaration fix needed; candidate.c reuses the existing extern unchanged.

- [s1] func_8002DAD0 is a confirmed member of the cop2-addressing-preamble-cluster (28-function, owner ruling 2026-08-17 / widened 2026-09-01) but carries SIX preamble/op islands, not the one enumerated in the static table: a partial 3-word gte_SetRotMatrix-shaped preamble (ctrl regs 0/2/4 only) feeding a GTE OP (outer/cross-product, word 0x4B70000C) fed by IR1-3 loaded directly from vecB; the LZCS/LZCR block (byte-identical to func_8001A67C and func_8002E838); a full 5-word gte_SetRotMatrix; and two gte_ldv0+MVMVA(word 0x4A486012)+gte_stlvnl sequences that rotate vecA and vecB in place.

- [s1] func_8002E838 (src/code6cac_b.c:1451-1552, already COMPLETED-C in this same file) is structurally the single-vector half of this function's algorithm -- same field offsets (0xD8-0xE8 identity matrix, 0xF8/0xFA angle fields, D_8008D118 log2 table), same RotMatrixY-then-RotMatrixX call shape, same gte_SetRotMatrix/gte_ldv0/MVMVA/gte_stlvnl tail. Its matched C was used as the direct, byte-verified template for over half of this function's candidate body.

- [s1] sandbox --disable all --diff at floor 25 shows the residual is entirely contained in one straight-line block (target insns 88-119): after the first ratan2() call, obj+0xC8 is shifted right 6 and stored, then RELOADED from memory for the subsequent multiply in target (sra v1,v1,6; sw v1,200(s1); lw v1,200(s1); mult v1,v1) -- our candidate's structurally-identical-looking C (store then re-read the same fixed offset) does NOT reproduce this reload; GCC's cse/register-allocation keeps the value in a register instead. This is the open frontier item.

- [s1] No naming/prototype for func_8002DAD0 existed anywhere in the tree before this session (first declaration); parameter is treated as u8 *obj matching the sibling func_8002E838's own parameter style in the same file.

## [s2] dist_sq scratch-block: CSE invalidation mechanism found + spent, floor 25 -> 6

**Root cause of the H3 "phantom reload" (RESOLVED, see hypotheses.md H3):**
read `tools/gcc-2.7.2/cse.c` directly this session.
`note_mem_written` (cse.c:7539-7579): for a `SET (MEM (PLUS reg const)) ...`
store (any pointer-relative address, SImode, not QImode) sets
`writes_ptr->var = 1` unconditionally, and additionally sets
`writes_ptr->nonscalar = 1` (the `all` flag stays 0 for a plain `PLUS`
address — only BLKmode/scratch-address stores set `all`).
`invalidate_from_clobbers` then calls `invalidate_memory(w)` whenever
`w->var` is set — i.e. after ANY pointer-relative store.
`invalidate_memory` (cse.c:1701-1719) removes every hash-table entry `p`
with `p->in_memory` set when `all || (nonscalar && p->in_struct) ||
cse_rtx_addr_varies_p(p->exp)` — and `cse_rtx_addr_varies_p` is TRUE for
essentially any entry whose own expression is a pointer-relative MEM,
**regardless of whether its address has any relationship to the store
that triggered invalidation.** So: ANY second pointer-relative store
(even to a totally unrelated field) wipes the cache entry created by an
earlier pointer-relative store, forcing a genuine reload on the next read
of the EARLIER address. The insertion of the CURRENT store's own MEM=reg
equivalence happens AFTER invalidation, so it always survives until the
NEXT store (of anything, any address).

**Applied structure (candidate.c, current floor-6 body):**
```c
angle1 = ratan2(*(obj+0xC8), *(obj+0xD0));
*(obj+0xC8) = *(obj+0xC8) >> 6;      /* write 1: inserts C8 cache entry */
*(obj+0xCC) = *(obj+0xCC) >> 6;      /* write 2: nukes C8 entry (any addr) */
{
    s32 dz = *(obj+0xD0) >> 6;        /* plain register read, no memory write yet */
    dist_sq = *(obj+0xC8) * *(obj+0xC8)   /* C8 read: cache MISS -> genuine reload */
            + dz * dz;                     /* dz: ordinary register reuse, no memory op */
    *(obj+0xD0) = dz;                 /* deferred D0 store -- scheduler is free to push
                                          this arbitrarily late since alias analysis
                                          proves no real conflict with C8/CC */
}
```
Measured: `sandbox --disable all --diff` -> `target_insns=204,
build_insns=204`, 9 hunks, **0 source-level**, 5 operand-only (a0<->a1
register-seat rename on dist_sq, propagated through
sltiu/addu/bltz/move/srlv), 4 not-scored (masked branch-target-address
cascade, explicitly flagged "do NOT chase").

**Two orderings measured IDENTICAL at floor 6** (both produce byte-for-byte
the same object, same score):
1. CC's write immediately after C8's write, before the `{ }` block (the
   form kept in candidate.c — simpler to read).
2. CC's write moved INSIDE the `{ }` block, after `dz`'s read but before
   the `dist_sq=` statement.
Both satisfy the same invalidation-ordering requirement (a second
pointer-relative write occurs between C8's write and C8's read), hence
identical codegen. This rules out "CC's exact textual position relative
to dz" as a lever for the remaining a0-vs-a1 tie — the next session needs
a DIFFERENT change (see frontier item 1: find what stamps `preferences: 5`
onto dist_sq's pseudo in local-alloc.c, per the `.greg` dump numbers
72/75/78/129/130/140 captured this session at
tmp/grind/func_8002DAD0/dumps/code6cac_b.greg:11463-11500).

**Sub-hypotheses tried and rejected during the search for the H3 mechanism**
(kept for the record, not to be re-tried in the same form):
- Two-shift-then-separate-`dist_sq+=`-statements (write C8, `dist_sq=C8*C8`,
  write D0, `dist_sq+=D0*D0`, write CC): floor 10 (WORSE than the 25
  baseline was 25->10 not an improvement path) — confirmed no reload
  occurs when the invalidating write comes AFTER the read it would need to
  invalidate; this measurement is what pinned down that invalidation must
  happen BEFORE the read, not merely "somewhere in the block".
- All-three-writes-upfront-then-one-combined-sum (C8, D0, CC writes, then
  one `dist_sq=C8*C8+D0*D0` expression, using a real D0 write instead of a
  deferred temp): floor 10, 3 source-level hunks (D0's own reload
  reappeared, since a THIRD write (CC) after D0's own write invalidates
  D0's entry too, before D0 is read for its own square) — this is what
  motivated H4's insight that D0 must stay as a deferred-store TEMP (never
  written to memory before its own read) rather than a second immediate
  write.

- [s2] sandbox --disable all --diff at the new floor 6 shows 9 hunks: 0 source-level, 5 operand-only (a0<->a1 rename on dist_sq's final pseudo), 4 not-scored (masked branch-target-address cascade artifacts) -- the C is no longer 'saying something different' from target; the remaining gap is a pure register-allocation seat choice.

- [s2] tools/gcc-2.7.2/cse.c:1701-1719 (invalidate_memory) and cse.c:7539-7579 (note_mem_written) are the exact GCC-2.7.2 CSE mechanism that explains BOTH why target genuinely reloads obj+0xC8 after storing it AND why obj+0xD0 does not reload after its own store -- read directly this session, not inferred from dumps alone.

- [s2] .greg dump captured this session (tmp/grind/func_8002DAD0/dumps/code6cac_b.greg, function header at line 11463) shows 6 pseudos needing allocation (72 75 78 129 130 140); pseudo 75 (likely dist_sq) has preferences:5 and no conflict vs hard reg 4; pseudo 78 has preferences:2,5 and DOES conflict with hard reg 4 -- this is the concrete data the next session's register-seat lever needs to trace back to its source copy/call insn in local-alloc.c.

- [s2] Two independent textual orderings of the CC-write/dz-read/dist_sq-statement sequence produced byte-IDENTICAL objects at floor 6, ruling out 'CC's exact position relative to dz' as a further lever for the remaining tie.

- [s3] Chassis-check confirmed at dispatch: memory/grind/func_8002DAD0/candidate.c measures floor 6 (matches ledger's session-2 recorded floor exactly).

- [s3] sandbox --disable all --diff (this session, baseline and after every reverted experiment): 9 hunks, 0 source-level, 5 operand-only, 4 not-scored; all 5 operand-only hunks are the same a0<->a1 register-seat tie on dist_sq's final pseudo propagated through sltiu/addu/bltz/move/srlv (target[108,113,117,119,129]).

- [s3] .lreg dump (tmp/grind/func_8002DAD0/dumps/code6cac_b.lreg, function at line 14028): dist_sq is pseudo 75, defined once at insn 166 (dist_sq = pseudo129 + pseudo130, the c8sq and dzsq products, both still unresolved pseudos at scan time), used at insns 183 (sltiu<1024 compare), 193 (true-branch table-lookup address), 225 (false-branch shift amount), 895/914 (false-branch <0 compare and table-lookup address).

- [s3] .greg dump (tmp/grind/func_8002DAD0/dumps/code6cac_b.greg, function at line 11463): allocno 75 (dist_sq) shows '75 conflicts: 72 75 140 2 3 12 29' and '75 preferences: 5'; allocno 78 (dist, the ratan2 2nd-arg value) shows '78 conflicts: 72 78 4 29' and '78 preferences: 2 5'; allocnos 129 (c8sq) and 130 (dzsq) show conflicts but NO preferences line at all.

- [s3] Hand-traced tools/gcc-2.7.2/global.c's set_preference (line 1670): insn 183's SET (dest resolves via reg_renumber to hard reg 2/v0, src=reg75 via LTU's XEXP(0) unwrap) DOES call set_preference and would stamp a preference for hard reg 2 onto allocno 75 -- but reg 2 is already in allocno 75's conflict list (75 conflicts with 2), so prune_preferences (global.c:882) removes it; insn 225 traces identically to the same pruned hard-reg-2 outcome. Neither traced insn explains the surviving hard-reg-5 preference.

- [s3] Traced expand_preferences (global.c:828, the REG_DEAD-adjacency merge): requires an insn whose SET_DEST is a global allocno (75 or 78) AND a REG_NOTES REG_DEAD entry on the OTHER allocno (78 or 75) in the SAME insn, with the two allocnos non-conflicting. Checked every insn setting 78 (insns 195, 238) and the only insn setting 75 (insn 166) -- none pairs a death of one with a set of the other, so this merge path is also ruled out for the 75<->78 pair directly.

- [s3] Ran the pre-existing BB2_ALLOC_DEBUG env-gated diagnostic (tools/gcc-2.7.2/global.c:379/605, NOT modified this session -- read-only instrumentation already present in the tree) via tmp/grind/func_8002DAD0/s3/allocdbg.py: allocation order is pseudo72(ord0,hardreg17) -> pseudo75(ord1,hardreg5,nrefs6,livelen13,pri9230) -> pseudo78(ord2,hardreg5,nrefs3,livelen5,pri6000) -> pseudo140(ord3,hardreg3) -> pseudo130(ord4,hardreg65/lo-spilled) -> pseudo129(ord5,hardreg6). seed_used (call-used registers) includes hard regs 4,5,6,7 (a0-a3) as 'ever used' baseline, not as a live-conflict fact.

- [s3] src/code6cac_b.c was reverted to the committed `INCLUDE_ASM("asm/funcs", func_8002DAD0);` line before this session ended (git status --short confirms clean, only the pre-existing metrics/events.jsonl dirt remains) -- per asm-until-matched, C lands on main only at COMPLETED-C; the working candidate lives only in memory/grind/func_8002DAD0/candidate.c (unchanged from session 2, still the floor-6 best-known form).

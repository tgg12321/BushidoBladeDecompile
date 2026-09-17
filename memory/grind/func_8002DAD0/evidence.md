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

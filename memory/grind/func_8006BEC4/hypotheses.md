# Hypothesis ledger — func_8006BEC4

## s1 (2026-09-15, recon)

H1 — declaration fix (DATA MODEL flagged symbols). CONFIRMED-neutral: D_800A36AC and D_800A374C
header decls (`extern s32`) already match every use; the three undeclared D_800F1xxx symbols are
parity-indexed primitive buffers (strides 0x12C / 0x30 / 0x10*4 proven by symbol spacing) and are
declared TU-local as arrays. No header edit needed; score 0 reached with these declarations.

H2 — pointer locals must be block-scoped per use site (loop body vs tail). CONFIRMED (v1 38 ->
v2 10 together with H3): a single `p` spanning two blocks is REG_BLOCK_GLOBAL and lands in a2 with
an extra `move a0,a2`; block-scoped locals tie to a1/a0 in local-alloc.

H3 — `pos` split-init (`pos = *(Vec2s16 **)…; pos += arg0;`) puts the load into pos's own pseudo so
the add is in place and reorg fills the beq delay slot with it. CONFIRMED (part of v1->v2).

H4 — `sh` field stores must be struct-member (MEM_IN_STRUCT_P) for sched.c true_dependence to let
the D_800A374C load hoist above them. CONFIRMED (v3 8 -> v6 0; v2 10 -> v7 3).

H5 — the y0 residual is fold-const.c association direction; spelling the expression as
`D_800A3900 + 0x7C - i` (VAR+CON - ARG1 branch) yields `i - 0x7C` first. CONFIRMED (v7 3 -> v9 0).
Killed spellings (instance, this chassis): `D_800A3900 - (i - 0x7C)` (3 on struct chassis),
`D_800A3900 + (0x7C - i)` (same association). The `(s16)` cast (v3/v6) also works mechanically but
is the forbidden F2 redundant-width-cast family — rejected on policy, not needed.

Frontier: none — candidate.c scores 0; awaiting layer-1 / Judge.

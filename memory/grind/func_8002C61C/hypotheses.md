# Hypothesis ledger — func_8002C61C

## s1 (2026-09-06, recon) — floor 46 -> 2 (2 = score.py reloc artifact, bytes proven at object level)

H1 CONFIRMED — dispatch head `andi` is the fold-const adjacent-equality merge: spell the mode test
   as `mode == 0xF || mode == 0x1C || mode == 0x1D || ... || mode == 0x21` (u16 local `mode`).
   Micro-probe d4 byte-exact; d5 (`>= && <=` pairs) also exact. KILLED (instance) on this
   chassis, no FAKE: `switch` forms (sw: 58, sw3 separate case bodies: 62 — balanced `slt`
   tree, stmt.c balance_case_nodes); `(u32)(mode - c) < 2` retired spelling (SImode, no andi).

H2 CONFIRMED — loops 1/2 are index-based block copies over `Vec3i *` bases:
   `dst_a = (Vec3i *)&D_801020D8; dst_b = (Vec3i *)((u8 *)&D_801020D8 + 0x44C);
   src = (Vec3i *)0x1F800000; for (i = 0; i < 3; i++) { dst_a[i] = src[i]; dst_b[i] = src[i+3]; }`
   (loop 2: `&D_801020FC`, `i < 2`, `src[i+6]`, `src[i+8]`, src base stays 0x1F800000). loop.c
   giv inits give `la v0; addiu t0,v0,0x44C; move a3,v0`. Byte-exact in full_A/B/C.
   KILLED (instance): pointer-increment src with `src[0]`/`src[3]` (l1b: `ori 0x60` base in
   loop 2); local struct pointer `((Vec3i*)((u8*)dst+0x44C))[i]` (if_idx: dst IVs merged into one
   with +1100 offsets, body 80).

H3 CONFIRMED — loop 3 source: index access on a constant scratchpad record
   `((ProbeScr *)0x1F800078)[i].j[5..9]` (stride 0x108, Vec3i j[22]) or pointer biv from
   0x1F8000F0 with all-negative indices; both give `ori a1,a1,0xEC` and a single IV (last-recorded
   giv is the base). KILLED (instance): retired `a1 = 0x1F8000EC; a1[-14..0]` (biv used directly
   at a1[0] -> givs reduced into a second IV at 0xE0).

H4 CONFIRMED — loop 3 destination `$a2` is a loop.c-reduced giv from a named byte-offset local
   written once per iteration: `off = i * 0x44C;` then `*(s32 *)((u8 *)&D_80102054 + off) = ...`
   (or `(u8 *)&D_80101EC8 + off + 0x18C`, same bytes). Preheader order `move t1,0; lui/ori 0x5555;
   li a1; move a2,0` byte-exact (l3v1/l3v2, full_C = 2). KILLED (instance): `for (i=0, off=0; ...;
   i++, off += 0x44C)` biv form (full_A/B = 4: `move a2,zero` emitted before the hoisted constant);
   inline `i * 0x44C` per use (l3v3: 6 `la` IVs); every declaration-level array/struct-array/
   record-pointer/u8-array index spelling (l3a, l3b, l3v4, l3v5: `la`-based IV with reg+const
   offsets — see evidence OBJECT MODEL).

H5 OPEN (tooling, not C) — the remaining 2 units are `addiu a2,a2,%lo(D_1F80000C)` (splat
   mislabel of the literal loop increment 0xC at 8002C8B4/8002C914) scored against the literal
   12; unpaired LO16 cannot be resolved by score.py `_resolve_named_pair`. No C can print 0 here.
   Needs an out-of-scope byte-neutral fix (the two `.s` lines, or score.py). Sibling
   func_80029454.s carries the same artifact 5x.

## Frontier (next session)
1. Ruling on H4's spelling (byte-offset local + `(u8 *)&SYM + off` cast) — the brief bans the
   per-use pun for these symbols but every declaration-level alternative is measured
   byte-infeasible; sibling matched functions in this TU use the identical idiom (src/code6cac_b.c
   func_8002C0DC: `(u8 *)&D_80101EC8 + i * 0x44C`, `*(s16 *)(var_s0 + 0x40)`).
2. Tooling handoff for H5 (asm/funcs/func_8002C61C.s lines with `%lo(D_1F80000C)` -> `0xC`, or
   score.py unpaired-LO16 resolution); after it the candidate should print 0 unchanged.
3. Once 1+2 are cleared: header-side naming (PracticeMenuRec fields at +0x174/+0x18C/+0x210/
   +0x234, scratchpad record type) is cosmetic — codegen is fixed by the spellings above.

# exec_game — WIP checkpoint (current state 2026-08-05)

**Honest pure-C distance 121 → 70.** Candidate: `candidate.diff` (apply to `src/main.c`).
NOT applied to the tree — the 104 regfix rules no longer repair the new codegen, so the
full build would break. Verdict ASM-SUSPECT, frame-less both sides (`.frame $sp,0` — no
frame lever exists here). Instruments: `wsl bash tmp/csz/d.sh exec_game main`, then
`python tmp/csz/align.py exec_game`.

## Root cause: `MEM_IN_STRUCT_P` made our stores provably non-aliasing

`true_dependence` (`tools/gcc-2.7.2/sched.c:817`) disambiguates:

```c
&& ! (MEM_IN_STRUCT_P (mem) && rtx_addr_varies_p (mem) && GET_MODE (mem) != QImode
      && ! MEM_IN_STRUCT_P (x) && ! rtx_addr_varies_p (x))
```

A **struct-member store at a varying address** cannot alias a **non-struct load at a fixed
address**. Our `typedef struct Entry { s32 w0, w1; }` model set that flag on every store,
so `loop.c invariant_p` (line 2779) found `D_800A2D3C` loop-invariant and LICM hoisted the
count out of all five phases — precisely what the six `subst_multi` rules re-create.
Scalar `s32 *` indexing is an `INDIRECT_REF` of a scalar (`MEM_IN_STRUCT_P == 0`), the
store conflicts, the count stays in the loop. The struct was our invention, not evidence
([[splat-symbol-names-are-not-evidence]]): the list is an array of `s32` pairs, stride 8.

## The ladder (each step measured independently)

| # | change | score | insns |
|---|---|---|---|
| 0 | HEAD | 121 | 197 |
| 1 | `Entry *` → `s32 *`, `[0]`/`[1]`, stride 2 (whole function) | 116 | 201 |
| 2 | phase 1: walking `cur` → `base[i * 2]` / `base[i * 2 + 1]` indexing | 109 | 199 |
| 3 | phases 3 + 5: same index form for `outer` / `inner` / `p` | 101 | 192 |
| 4 | phase 3 outer loop: re-read `cnt = D_800A2D3C;` at the bottom, condition `cnt >= i` | 92 | **194** |
| 5 | phase 1: post-scan pointer gets its own variable `q` | 77 | 194 |
| 6 | per-phase named locals holding the bit constants, assigned BEFORE `base = …` | **70** | 195 |

Lever 1 is the root-cause fix. Levers 2-3 are the induction-variable fix: with a walking
pointer, `cur` is a biv and `cur[1]` becomes a *second* DEST_ADDR giv (`addiu $aN,$base,4`
plus a second `addiu $aN,$aN,8` per loop). Index form makes both accesses givs of `i`;
`combine_givs` merges them (constant difference) and one walking pointer with `0(reg)` /
`4(reg)` displacements falls out — target's shape. Lever 2 also produces target's
`move $t0,$t2` in the phase-3 inner preheader (LICM copying the now-invariant outer
element pointer). Lever 4 mirrors phase 4, which already matched. Lever 5 reproduces
target's two-register shape ($v1 scan pointer, $a1 recomputed) where we coalesced both;
RENAME 65 → 49. Lever 6 fixes the largest MOVED group: every phase's `lui/lw D_800A2D40`
pair sat before the loop's bit constants instead of after; MOVED 33 → 12 lines.

## Lever 6's spelling is FLAGGED, not assumed

The holders are LIVE — every value is a real operand in the emitted AND / store, so no
instruction is manufactured — and the tree already ships the idiom in
`gnd_land_hit_char_tsuba` (`c100 = 0x100; c1 = 1;`). **But it currently costs 11 distinct
per-phase locals** (`flagbit1`, `sentinel1`, `addrmask1`, `sentinel2`, …): sharing one
holder across phases lets CSE delete the later re-assignments and the gain collapses
(shared names 87 @ 195, distinct 70 @ 195). Eleven holders for what a programmer would
write as literals is exactly the shape [[named-local-fake-exception]] governs. **Layer-2
must rule before this can land.** The 92 state (levers 1-5) is the uncontroversial
fallback.

## Measured kills

- **Goto-form loop** (phase 1 `do/while` → `p1_top:` + `if (…) goto p1_top;`) — 148 @ 194.
  Dropping the LOOP notes stops the LICM hoist but also stops the *constant* hoisting the
  target HAS. Target's loop is a real loop; only the memory ref must be non-invariant.
- **Dropping the `base` local**, indexing `((s32 *)D_800A2D40)[…]` — 98 @ 205. Re-confirms
  session 1's round-1 kill (121→127): the `base` local is correct.
- **`p = (j) * 2 + base`** (source operand order, to mirror target's `addu $v1,$v0,$t0`
  where we emit `addu $a1,$t3,$v0`) — inert at 77. GCC canonicalizes the pointer PLUS
  chain, so the `subst "addu $3,$8,$2" "addu $3,$2,$8"` rules @19/@27 are **not**
  source-reachable.
- Scan loop as a real `while (p[0] == sentinel1) { … }` — 76 @ 198.
- Swapping `j++` / `p += 2` in the scan body (to steer reorg's `bne` delay-slot fill) — 71.
- Dropping phase 2's `v`/`cnt` split (target has the `move $a0,$v0` copy) — 71.
- Removing the dead `outer` / `inner` decls — inert; removed anyway.

## The residual 70 — all RA/scheduling, zero structure

Streams align 1:1: **60 RENAME + 12 MOVED, 0 structural**, 195 vs 194 insns. The MOVED set
is six ±1-slot scheduling pairs:

- Phase 1: our `sll $v0,$a2,3` fills the scan loop's `bne` delay slot; target uses
  `addiu $v1,$v1,8`. Two source orderings tried, both worse.
- Phase 2: our `cnt = v` copy takes the `bltz` delay slot; target puts `i = 0` there.
- Phase 3: `move $t0,$t2` one slot early. Phase 4: `addiu $t1,$t1,1` one slot off.
- Phase 5: our `lw $v1,4($a0)` one slot above target's, across the `sw D_800A2D3C`.

The 60 renames are a coherent permutation of the preheader registers (birth order in the
preheader block). **Next: `tools/ra_solver` on the preheader allocnos** — the structure is
settled and the streams align 1:1 — and `tools/sched_solver` on the six pairs.

## Reusable findings (rule-worthy; layer-2 + owner sign-off before registering)

1. *Struct-vs-scalar pointer typing is an ALIASING lever.* When a regfix cluster re-creates
   loads of a **global** that our build caches across a loop, and the loop stores through a
   **struct pointer**, the struct typing is what proves non-aliasing under
   `true_dependence`. Retyping to the scalar pointer the data actually is restores the
   reload. Sibling of [[store-const-reload-cse]] (there: *where* the read happens; here:
   what the store's *type* lets GCC prove).
2. *LICM can only place hoisted invariants AFTER the preheader's source statements*
   (`move_movables` → `emit_insn_before (…, loop_start)`). So whenever target's preheader
   shows a loop-invariant constant BEFORE a value the source assigns, that constant was
   source-level in the original, not hoisted. A cheap mechanical read of any preheader
   ordering diff.

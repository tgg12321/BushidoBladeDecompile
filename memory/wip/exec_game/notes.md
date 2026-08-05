# exec_game — WIP checkpoint (2026-08-05, session 2)

**State: honest pure-C distance 121 → 92.** Instruction counts now identical (194/194,
was 197/194). Candidate: `memory/wip/exec_game/candidate.diff` (apply to `src/main.c`).
Tree is at HEAD — the candidate is NOT applied, because the 104 regfix rules no longer
repair the new codegen and the full build would break.

Measured at HEAD `4b7623d0`. 104 rules, verdict ASM-SUSPECT, frame-less both sides
(`.frame $sp,0` — no frame lever exists here).

## Session 1's question, answered: it was LICM, and the trigger was our struct typing

Session 1 located the blocker (target re-reads `D_800A2D3C`/`D_800A2D40` at every loop
condition; we hoist) and left avenue #1 as "distinguish defeat-LICM from defeat-CSE".
It is LICM, and `true_dependence` (`tools/gcc-2.7.2/sched.c:817`) is why:

```c
&& ! (MEM_IN_STRUCT_P (mem) && rtx_addr_varies_p (mem) && GET_MODE (mem) != QImode
      && ! MEM_IN_STRUCT_P (x) && ! rtx_addr_varies_p (x))
```

A **struct-member store at a varying address** cannot alias a **non-struct load at a
fixed address**. Our body modelled the list as `typedef struct Entry { s32 w0, w1; }`, so
every `p->w0 = …` set `MEM_IN_STRUCT_P`, `loop.c invariant_p` (line 2779) found the count
loop-invariant, and LICM hoisted it — exactly what the six `subst_multi` rules re-create.
Scalar `s32 *` indexing is an `INDIRECT_REF` of a scalar (`MEM_IN_STRUCT_P == 0`), the
store conflicts, the count stays in the loop. The struct was our invention, not evidence
([[splat-symbol-names-are-not-evidence]]): the list is an array of `s32` pairs, stride 8.

## Levers that landed (each measured independently)

| # | change | score | insns |
|---|---|---|---|
| 0 | HEAD | 121 | 197 |
| 1 | `Entry *` → `s32 *`, `[0]`/`[1]`, stride 2 (whole function) | **116** | 201 |
| 2 | phase 1: walking `cur` → `base[i * 2]` / `base[i * 2 + 1]` indexing | **109** | 199 |
| 3 | phases 3 + 5: same index form for `outer` / `inner` / `p` | **101** | 192 |
| 4 | phase 3 outer loop: re-read `cnt = D_800A2D3C;` at the bottom, condition `cnt >= i` (mirrors phase 4, which already did this and already matched) | **92** | **194** |

Lever 1 is the root-cause fix. Levers 2-3 are the induction-variable fix (session 1's
open avenue #3): with a walking pointer, `cur` is a biv and `cur[1]` becomes a *second*
DEST_ADDR giv, so GCC emits `addiu $aN,$base,4` plus a second `addiu $aN,$aN,8` per loop.
Index form makes both accesses givs of `i`; `combine_givs` merges them (constant
difference) and one walking pointer with `0(reg)`/`4(reg)` displacements falls out —
target's shape. Lever 2 also produces target's `move $t0,$t2` in the phase-3 inner
preheader (LICM copying the now-invariant outer element pointer). Session 1's avenue #2
(the two lost `addu $t1,$t1,1`) resolved itself: both are present once the counts reload.

## Measured kills

- **Goto-form loop** (phase 1 `do/while` → `p1_top:` + `if (…) goto p1_top;`) — 148 @ 194.
  Dropping the LOOP notes does stop the LICM hoist, but it also stops the *constant*
  hoisting the target HAS (`lui $t4,0x8000` etc. in the preheader). Target's loop is a
  real loop; only the memory ref must be non-invariant. Wrong lever.
- **Dropping the `base` local**, indexing `((s32 *)D_800A2D40)[…]` so LICM hoists the load
  into the constant group — 98 @ 205. Re-confirms session 1's round-1 kill (121→127) on
  the new base: the `base` local is correct and the load-placement difference is
  scheduling, not hoisting.
- Removing the now-dead `outer` / `inner` decls: inert (92 either way); removed anyway.

## The residual 92 — all RA/scheduling, zero structure

Streams align 1:1. `python tmp/csz/align.py exec_game` (after `wsl bash tmp/csz/d.sh
exec_game main`) reports **65 RENAME + ~16 MOVED, 0 structural**. The moved set:

- The `lui`/`lw D_800A2D40` pair sits early in each of the 4 preheaders; target has it
  *after* the hoisted constants (4 × 2 insns). sched1 ordering, not hoisting (kill above).
- Phase 1: our `sll $v0,$a2,3` fills the scan loop's `bne` delay slot; target fills it with
  `addiu $v1,$v1,8` and keeps the scan pointer and the recomputed `p` in *two* registers
  ($v1 then $a1) where we coalesce both into $a1.
- Phases 3/4: `addiu $t1,$t1,1` one slot either side of the count reload.
- Phase 5: our `lw $v1,4($a0)` hoists above the `sw D_800A2D3C`.

The renames are a coherent permutation of the hoisted-constant registers (target
0x80000000→$t4, 0x2FFFFFFF→$t2, 0xFFFFFFF→$t3, base→$t0; ours $t3/$t2/$t0/$t5) — the
preheader birth order differs. **Next: `tools/ra_solver` on the preheader allocnos**, and
`tools/sched_solver` on the four `lw D_800A2D40` placements. The structure is settled.

## Reusable finding (rule-worthy; needs layer-2 + owner sign-off before registering)

*Struct-vs-scalar pointer typing is an ALIASING lever.* When a regfix cluster re-creates
loads of a **global** that our build caches across a loop, and the loop stores through a
**struct pointer**, the struct typing is what proves non-aliasing under
`true_dependence`. Retyping to the scalar pointer the data actually is restores the
reload. Sibling of [[store-const-reload-cse]] (there: *where* the read happens; here:
what the store's *type* lets GCC prove).

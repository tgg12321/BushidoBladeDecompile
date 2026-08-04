# func_8003D9A0 — WIP (current state 2026-08-04, RTL-forensics session)

Small loop function (60 insns): walks a `u32 *` packet list, per iteration
advances an s16 coordinate pair with wraparound at 0x200, calls
`initLoadImage(s1, s0, s4, s3)`, copies 24 bytes (`Copy24`) forward by 0x18,
steps s1 by 0x30.

## Where it stands — honest distance 4 (was 19)

`src/code6cac_c2.c:1219` now holds a **pin-free, barrier-free, cheat-free**
body. `sandbox --disable all` = **4**; full build SHA1 == oracle (verified).

The frame is now NATIVE: cc1 emits `.frame $sp,56 # vars= 8, regs= 7/0,
args= 16` with no coercion. The lever is the loop guard:

```c
s2 = a1 - 1;
if (s2 != -1) {           /* was: if (a1 != 0) { s2 = a1 - 1; */
    do { ... } while (--s2 != -1);
}
```

Everything else is unchanged. Emitted instruction count is identical (57 in
the isolated probe, 60/60 vs target in `fdiff`); the guard still compiles to
`beq $a1,$0` with `addiu $s2,$a1,-1` in the delay slot, exactly as target.

**Remaining 4-insn diff is prologue scheduling only:** target emits the
`sw s0,24(sp)` / `move s0,a0` pair FIRST, then `sw s1` / `move s1`, then
`sw s2`; we emit `sw s1` / `move s1` / `sw s2` / `sw s0` / `move s0`. Same 9
prologue insns, first 5 permuted. This is what `tools/prologue_config.json`'s
2 reorders encode.

## The mechanism (RTL forensics — the point of this session)

The phantom `vars=8` is an **unallocated pseudo given a stack slot by reload's
`alter_reg`**. Two distinct producers exist in this codebase:

1. **combine orphan-USE** (`tools/gcc-2.7.2/combine.c:10836-10841`,
   `distribute_notes`, REG_DEAD case). When a REG_DEAD note can't find a home
   and the backward scan from `i3` immediately hits a CODE_LABEL/JUMP_INSN,
   combine emits a bare `(use (reg N))`. That pseudo has refs but zero live
   length, global.c never allocates it, and `alter_reg` hands it 8 frame bytes
   at zero code cost. This is `tslLineG5Init`'s slot (pseudo 92 = the `sll 16`
   half of a HImode sign-extend).
2. **folded loop-guard compare** — the guard's comparison pseudo survives
   `reg_n_refs` accounting after jump/combine fold it into a bare branch.
   This is what the new `if (s2 != -1)` guard uses, and what
   `func_800400F8` (config.c:543) already documents in-tree.

Codebase census: **57 orphan pseudos across 31 TUs** (`tmp/orphan_census.py`),
in three RTL shapes — `ashift` (HImode sign-extend), `plus` (folded address),
`lt`/`ltu` (folded compare). The mechanism is ordinary, not exotic.

Also settled: **loop.c `check_dbra_loop` cannot have produced this loop** — the
inc→dec reversal requires `num_mem_sets <= 1` (loop.c:5747) and this body has
7 stores. The `bne $s2,$s5` / `$s5 = -1` downcounter is therefore ORIGINAL
source, not a compiler transform. Do not re-explore counting-up `for` forms.

## Measured negatives (do not re-run)

~90 spellings measured at `vars=0` / no improvement: `s16`-typed s4/s3 (always
costs an 8th callee-save), `s16` initLoadImage prototype, struct-pointer and
per-word forms of the Copy24 copy, `Rect`-struct field access, compound-assign
and no-temp forms of the y-advance, 8 loop-guard shapes × 5 wrap-test shapes ×
4 prologue shapes, counting-up `for`/`while` forms (vars=8 but wrong loop
tail), and all 20 dependence-legal permutations of the 5 pre-loop statements
(all score 4 — statement order is inert on the prologue schedule).

## Cheats left to retire on completion

- `tools/prologue_config.json` `func_8003D9A0` (2 reorders) — carries the
  residual 4 insns.
- `tools/frame_fix_funcs.txt` `func_8003D9A0 56` — now **provably dead**: cc1
  emits 56 natively and the full build still matches, so `apply_frame_fix` is
  a no-op. Delete it in the completion commit.
- Zero regfix/asmfix rules; zero cheat-asm in src (pins + barrier retired).

## Resume — next avenue

The only open item is the prologue save/move interleave. Statement order does
not move it, so attack the scheduler input: `sched.c` priority of the two
parameter-copy insns (`move s0,a0` has an in-block consumer via `lh s4,0(s0)`;
`move s1,a2` has none, yet ours schedules it first). Read the `.sched` dump
for `func_8003D9A0` in `tmp/rtl/base.c.sched` and compare against a sibling
whose prologue matches. Levers not yet tried: making `s1`'s first use even
later (H variant scored 6 — worse), and varying how `a2` reaches the loop.

## Instruments (tmp/, regenerate from this note if cleaned)

`frame_probe.sh` (fast per-func `.frame`), `fdiff.sh` (target-vs-honest diff),
`rtl9a0.sh` (cc1 -da dumps of the TU), `rtlscan.py` / `usehunt.py` (per-pass
RTL block search), `orphan_census.py` (codebase-wide orphan census),
`probe.py`..`probe8.py` (standalone micro-TU sweeps), `apply9a0.py` /
`sweep_order.py` / `sweep_perm.py` (real-TU variant scoring).

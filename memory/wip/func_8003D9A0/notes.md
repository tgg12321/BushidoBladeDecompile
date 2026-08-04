# func_8003D9A0 — WIP (current state 2026-08-04, session 2: MATCHED, honest 0)

Small loop function (60 insns): walks a `u32 *` packet list, per iteration
advances an s16 coordinate pair with wraparound at 0x200, calls
`initLoadImage(s1, a0, s4, s3)`, copies 24 bytes (`Copy24`) forward by 0x18,
steps the packet pointer by 0x30.

## Where it stands — honest distance 0

`src/code6cac_c2.c:1219` holds a **pure-C, cheat-free** body: zero regfix /
asmfix rules, zero register pins, zero `__asm__`, zero dead code.

- `sandbox --disable all` = **0** (60/60 insns; `fdiff` residue is only branch
  *target addresses* and one relocation offset, which differ because the
  sandbox object places the function at a different offset than the reference).
  The sandbox filters `prologue_config.json` / `frame_fix_funcs.txt` for this
  function (`engine/pipeline.py:74-80`), so the 0 is genuinely rule-free.
- Full build SHA1 == oracle, verified with the two config entries still
  present — they are now **inert**, not load-bearing.

Two levers, applied in this order, took it 19 → 4 → 0.

### Lever 1 (session 1) — guard respelling gives the frame natively

```c
s2 = a1 - 1;
if (s2 != -1) {           /* was: if (a1 != 0) { s2 = a1 - 1; */
```
cc1 then emits `.frame $sp,56 # vars= 8` with no coercion. The 8 bytes are a
**phantom slot**: an unallocated pseudo handed frame space by reload's
`alter_reg` at zero code cost. Producers in this codebase (57 instances over
31 TUs, census in `tmp/orphan_census.py`):
1. combine's orphan USE — `tools/gcc-2.7.2/combine.c:10836-10841`
   (`distribute_notes`, REG_DEAD case): when a death note finds no home and the
   backward scan from `i3` hits a CODE_LABEL/JUMP_INSN, combine emits a bare
   `(use (reg N))`; that pseudo has refs but zero live length, `global.c` never
   allocates it, `alter_reg` gives it 8 bytes. This is `tslLineG5Init`'s slot
   (pseudo 92, the `sll 16` half of a HImode sign-extend).
2. a folded loop-guard compare whose pseudo survives `reg_n_refs` accounting —
   what this function now uses. In-tree precedent: `func_800400F8`
   (`src/config.c:543`, Judge-ruled 2026-07-14).

### Lever 2 (session 2) — drop the `s0 = a0` param alias

The residual 4 was the prologue interleave: target emits `sw s0`/`move s0,a0`
FIRST, we emitted `sw s1`/`move s1,a2`/`sw s2` first. `sched2` verbose
(`tmp/rtl/base.c.sched2`) showed why — **all INSN_PRIORITYs in block 0 are 1**
(zero-cost dep chains), so order is decided purely by ready-list entry time,
and the backward scheduler emits earliest-entering first. `move s0,a0`
(insn 14) could not enter before `move s1,a2` (insn 17) because insn 14 feeds
the two `lh` loads that end the block, while insn 17 has no in-block consumer.
With that dependence present the target order is unreachable.

The fix is [[drop-param-alias-local]]: `s16 *s0 = a0;` created a SECOND pseudo
(param pseudo 72 → local pseudo 16) that reload coalesced into `move s0,a0`.
Deleting the alias and using `a0` directly gives the parameter one long live
range, RA promotes it to `$s0` itself, and the promotion-move schedules first.
Same rule, same distance-4 symptom, and the confirmed prior case is
`initLoadImage` — the function this one calls.

`a2` keeps its `s1` local because `s1` is *mutated* each iteration; `a0` is
never modified, so its alias was pure redundancy.

## Measured negatives (do not re-run)

Session 1 (~90 spellings, all `vars=0` or worse): `s16`-typed s4/s3 (always an
8th callee-save), `s16` initLoadImage prototype, struct-pointer / per-word /
`Rect`-struct forms of the Copy24 copy, compound-assign and no-temp y-advance,
8 guard × 5 wrap-test × 4 prologue grid, counting-up `for`/`while`.
**Settled:** loop.c `check_dbra_loop` cannot have produced this loop — the
inc→dec reversal needs `num_mem_sets <= 1` (loop.c:5747) and this body has 7
stores, so the `bne $s2,$s5` downcounter is original source.

Session 2, all still score 4 (the interleave is robust to everything except
the alias drop):
- all 20 dependence-legal permutations of the pre-loop statements;
- 14 further phantom-producing guard spellings (`s2 >= 0`, `s2 > -1`,
  `!(s2 < 0)`, `(u32)s2 != 0xFFFFFFFF`, `s2 != ~0`, `-1 != s2`, u32 counter,
  `--s2 >= 0` / `> -1` tails, `s2-- != 0`): **every** phantom-producing variant
  emits the identical wrong interleave, so the guard is not the lever;
- read-shape variants (`*s0`, `*(s0+1)`, byte-offset casts, walking pointer
  `*p++`, reads from `a0` with `s0` still declared, y-read before x-read):
  GCC copy-propagates the alias back, dependence unchanged;
- moving `s0` and/or `s1` inside the guard block (scores 6, 6, 10 — worse);
- `drop_s1` alone (keep the `s0` alias, drop `s1`) → still 4, confirming `s0`
  is the sole culprit.

## Handoff — completion gate (NOT run here, per instruction)

Remaining work is mechanical and belongs to the main session:
1. Delete `tools/prologue_config.json` `func_8003D9A0` (2 reorders) and the
   `tools/frame_fix_funcs.txt` `func_8003D9A0 56` line. Both are already
   inert — the build matches with them present while the honest output is
   byte-perfect without them.
2. `retire` + full build SHA1 verify, layer-2 `cheat-reviewer`, `queue done`.

## Transfer

Sibling `func_8003DBE4` (same file, line ~1294) still carries a
`s32 buf[2];` dead-array cheat for the SAME 8 phantom bytes. The phantom-slot
mechanism above is the honest replacement; try the folded-guard producer first.

## Instruments (tmp/, regenerate from this note if cleaned)

`frame_probe.sh` (fast per-func `.frame`), `fdiff.sh` (target-vs-honest diff),
`rtl9a0.sh` (cc1 -da dumps), `rtlscan.py` / `usehunt.py` (per-pass RTL search),
`orphan_census.py` + `prologue_census.py` (codebase-wide censuses),
`probe.py`..`probe9.py` (standalone micro-TU sweeps), `apply9a0.py` /
`sweep_order.py` / `sweep_perm.py` / `sweep_sched.py` / `sweep_reads.py` /
`sweep_alias.py` (real-TU variant scoring).

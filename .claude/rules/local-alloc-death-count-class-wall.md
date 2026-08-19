---
name: local-alloc-death-count-class-wall
description: "A pure $v0<->$v1 swap between a reused multi-load temp and a short constant is NOT a priority tie — local-alloc.c:472 gates on reg_n_deaths==1, so the multi-death temp is unconditionally punted to global-alloc and the single-death constant wins $v0 by ascending first-free. Reordering/decl-order/split cannot flip it; the only known flips are cheats."
paths: [".claude/rules/local-alloc-death-count-class-wall.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
metadata:
  type: reference
---

# The local-vs-global allocation CLASS wall (`local-alloc.c:472` death-count gate)

## Symptom

Your build and the target are byte-structurally identical — same instructions,
same order, same interleave — except **two registers are swapped**, typically
`$v0` <-> `$v1`. One of the two values is a variable **reused across several
loads/stores**; the other is a **short-lived constant or mask** built in place
(`lui`/`ori`, or a single `li`). Every reorder, declaration-order change and
block-local split you try either leaves the floor where it is or makes it worse.

This is **not** a `global.c` allocno-priority tie. Do not spend a session on
priority levers ([[register-alloc-pure-c]] Lever A/C) before running the
diagnosis below — four functions have each burned multiple sessions on that
misdiagnosis (their own s1/s2 ledger entries name `global.c` priority, and the
s3/s6 forensics sessions overturn it).

## Mechanism (read verbatim from `tools/gcc-2.7.2/`)

Local allocation runs **before** global allocation, and `local-alloc.c:472`
decides which pseudos it may touch at all:

```c
if (reg_basic_block[i] >= 0 && reg_n_deaths[i] == 1 && ...)
    reg_qty[i] = -2;   /* LOCAL — allocated now */
else
    reg_qty[i] = -1;   /* punted to GLOBAL alloc */
```

Two consequences, both **unconditional** (no tiebreak, no priority):

1. A variable that holds **N distinct loaded values** has `reg_n_deaths == N`.
   For N > 1 it is **always** deferred to `global_alloc`, no matter how the C is
   spelled. Three loads through one temp = three deaths, forever.
2. A constant/mask whose sets are RMW-chained into one contiguous quantity has
   `reg_n_deaths == 1` and lives in one basic block, so it is **always**
   local-allocated.

Then the register CHOICE is equally mechanical: `config/mips` defines **no
`REG_ALLOC_ORDER`** (`grep -n REG_ALLOC_ORDER tools/gcc-2.7.2/config/mips/*.h`
returns nothing; `regclass.c:112` only fills `reg_alloc_order` under that
`#ifdef`), so `find_free_reg` / `find_reg` scan hard registers in **ascending
regno** (`local-alloc.c:2249-2262`, `global.c:1057-1062` / `1203-1209`). The
lone local quantity therefore takes the lowest free caller-saved register —
**`$v0` (reg 2)** — and the punted multi-death web gets the leftover `$v1`.

A constant has **no copy source**, so it carries no copy-preference that could
override the scan. The outcome is fully determined by the death counts.

## The three exits, all measured dead (func_80061658 s3, func_800611A4 s6)

To reach the target's opposite assignment you must do one of:

- **(a) make the load-temp `reg_n_deaths == 1`** — impossible: 3 distinct loaded
  values are 3 deaths under every C spelling. A 3-word struct block copy
  (`*(struct{s32 a,b,c;}*)&G = *(struct{...}*)arg0;`) still lowers to lw/sw
  pairs that die 3x, *and* drops the mask interleave (scored 22 vs 43-insn base).
- **(b) make the constant `reg_n_deaths >= 2`** so it goes global, where the
  higher-ref-count load-temp outranks it for `$v0` — no byte-neutral pure C
  exists; the constant's single store is on one path, so a second use is an
  extra emitted instruction.
- **(c) introduce a `$v0`-blocking single-death LOCAL pseudo across the
  constant's live range** — this is a **load split**, and every split measured
  7-11 (worse): the split temp has no anti-dependence so the scheduler hoists
  it (distorting the interleave) and/or it steals `$v0` from the shared global
  web, scattering that web to `$a0`/`$a2`.

The one construct that DOES flip it — a **fresh invented staging local** that
converts one load out of the multi-death web into a single-death local pseudo
(func_800611A4 FORM A, sandbox 9 -> 2) — is a cheat: an invented carrier with no
semantic purpose, and [[staged-value-reused-variable]] prerequisite #2 requires
reusing an EXISTING variable. Existing dead locals were measured unusable as
carriers (wrong width truncates; a pointer local that crosses the call is
allocated as a call-crossing pseudo to `$a2`). **Do not re-propose it.**

## Diagnosis recipe (one dump, decisive)

```
cc1 ... -da            # produces <file>.i.lreg and <file>.i.greg
```

- `.lreg`: `Register <N> used K times across M insns in block B; dies in D places`
  — read `D` for both contended pseudos. `D > 1` on one of them **is** the wall.
- `.greg`: the `"N regs to allocate: ..."` list. A pseudo **absent** from that
  list was local-allocated; a pseudo **present** was punted. If the reused temp
  is the *only* entry ("1 regs to allocate"), you are looking at this class.

If the target's own bytes show the same shape with the registers reversed, the
target's source was **not** the obvious reused-temp C on this toolchain — the
cc1psx cross-check emits our wall too ([[cc1psx-calibration-only]]).

## Confirmed occurrences (all endgame-locked / OWNER-ACCEPTED INCOMPLETE)

| function | file | floor | ledger |
|---|---|---|---|
| `func_800611A4` | `src/text1b.c` | 6 | `memory/grind/func_800611A4/evidence.md` [s6] |
| `func_80061658` | `src/text1b.c` | 7 (9 pure-swap) | `memory/grind/func_80061658/evidence.md` [s3] |
| `func_80061710` | `src/text1b.c` | — | `memory/grind/func_80061710/evidence.md:84,95,216` |
| `func_80057CC8` | `src/text1b.c` | 3 | `memory/grind/func_80057CC8/evidence.md` [s21]-[s24] |

`func_80057CC8` [s24] adds a direct-instrumentation confirmation: `reg1=86`
appears in **zero** `QTYDBG` lines in its `cc1.err` local-alloc region, verifying
the code-read finding by measurement rather than inference.

Named siblings that share the shape and should be diagnosed here FIRST:
`func_800617C8`, `func_800618B4`, `func_8006133C` (from the func_80061658
frontier note — these are the "verify the winning form against a sibling"
targets, not yet measured).

## Related
- [[register-alloc-pure-c]] — the general pin-retirement levers; its "Confirmed
  limits" section is where this class belongs on the map. Read Step-0 first: the
  levers there assume a **priority** tie, which this class is not.
- [[register-alloc-deep-dive]] (`memory/project/`) — the `global.c:624`
  priority-tiebreaker wall, the *other* RA wall class. Distinguish them by the
  `.lreg` death count before choosing a lever set.
- [[staged-value-reused-variable]] — the sanctioned staging family, and exactly
  why the flip found here does not qualify for it.
- [[difficult-is-not-impossible]] — the matching C exists for the original
  source; what is proven dead is reaching it from *this* C shape on this
  toolchain, which is a disposition question, not a licence to cheat.

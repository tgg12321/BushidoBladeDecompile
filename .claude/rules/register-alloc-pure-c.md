---
name: register-alloc-pure-c
paths: ["src/*.c"]
description: "Retire a register pin in PURE C. gcc-2.7.2 prefers low regs; if target uses a lower reg than you, YOU are the anomaly (cc1 -da greg dump). Levers: block-local split, narrow type, loop precompute, dead assignment."
metadata:
  type: reference
---

# Retiring a register pin in pure C (no regfix, no inline asm)

When a function matches ONLY via `register T x asm("$N")` pins (or you are
tempted to add one), the pin is almost always papering over a **global
register-allocation tie that pure-C source structure can move.** Do NOT accept
the pin (tier-3 debt) or fall back to a regfix register-rename until you have
run the diagnosis and tried the levers below. This retired ALL 5 pins on
`saTan0Main` (main.c MIDI dispatcher, commit 6dba050) and the last pin on
`InitHiraRmd_80047FBC` (commit 10becc3) — both to 100% pure C (tier-4). Several
prior agents had given up on saTan0Main and left a pin.

## Step 0 — diagnose: is YOUR build or the target the anomaly?

MIPS gcc-2.7.2 defines **no `REG_ALLOC_ORDER`** (confirm:
`grep -r REG_ALLOC_ORDER tools/gcc-2.7.2/*/config/mips/mips.h` → empty), so the
allocator uses **default ascending register order** — lower-numbered hard regs
are PREFERRED:

| reg | num | reg | num |
|---|---|---|---|
| `$v0` | 2 | `$a3` | 7 |
| `$v1` | 3 | `$t0..$t9` | 8-15,24,25 |
| `$a0..$a2` | 4-6 | `$s0..$s7` | 16-23 |

**The key inference:** if the **target uses a LOWER-numbered register** than
your build for the same value (e.g. target `$v1`, you `$a2`), the target is
following the natural preference and **YOUR build is the anomaly** — something
is *blocking* the preferred register in your codegen. The pin is not
fundamental; find and clear the blocker. (If the target uses a HIGHER reg than
you, the situation is rarer and a pin may be closer to justified.)

See the actual allocation with an isolated `-da` (dump-all-RTL) compile:
```
cc1 <build-flags from compile.sh> -da /tmp/standalone.c -o /tmp/x.s
```
This writes `/tmp/standalone.c.greg` (post-global-alloc) and `.lreg`. The
`;; Register dispositions:` section maps `pseudo -> hardreg` (`77 in 6` =
pseudo 77 in `$a2`). Find your value's pseudo, see which reg it got and which
pseudo holds the preferred reg across its live range. (Build the standalone =
typedefs + the function's real `extern` decls + the body; copy the exact decls
from the .c so call arg-counts/pointer-ness match — they affect codegen.)

## Lever A — block-local variable split (shrink live range / conflicts)

**Symptom:** one variable (often a pointer or byte temp) is reused across many
sites — e.g. loaded in several `switch` cases. The global allocator assigns the
WHOLE pseudo ONE register and frequently reuses a soon-to-be-argument register
(`$a0..$a3`) because the temp dies just before the call's arg setup — instead of
the lower-numbered scratch the target uses.

**Fix:** read the value into a **fresh block-local variable in ONE (or a few) of
the sites** instead of the shared variable. Shrinking the shared pseudo's live
range / conflict graph frees the allocator to give it its preferred
(lower-numbered) register across the remaining sites.

```c
/* before: shared `u8 *cmd_ptr` reused in every case -> GCC parks it in $a2 */
case 0x90: cmd_ptr = *state; ...; handler(a0, a1, b, next); goto end;
/* after: ONE case reads into a block-local -> shared cmd_ptr now gets $v1 */
case 0x90: { u8 *cp = *state; ...; handler(a0, a1, b, next); goto end; }
```

**Reference:** `saTan0Main` (6dba050) — splitting ONE switch case's `cmd_ptr`
use into a block-local `cp` retired the last pin; the shared `cmd_ptr` then
allocated to its preferred `$v1` across the other cases. A `goto` out of the
block is fine in C.

## Lever B — narrow integer type (`char`/`short` vs `int`/`u32`)

The WIDTH of an integer variable changes GCC's value-width tracking AND its
register allocation. A narrow `char`/`short` (vs `u32`/`int`):
- emits byte/half masks (`andi …,0xFF`, sign-extends) the target has where a
  wide type elided them as redundant — **retiring `andi`-style regfix rules**;
- can yield the target's callee-save allocation for the variable naturally.

**Reference:** `saTan0Main` — `u32 b` → `char b` retired an `andi` regfix rule
+ 4 register pins in a single token. NB: the build uses `-funsigned-char`, so
`char` is *unsigned* — the lever is the narrow **width**, not signedness. The
vanilla decomp-permuter's type-mutation pass finds this automatically; run it
from a pin-free base and check whether a score drop came from a type change.

## Lever C — whole-function reallocation via a loop-local / precompute

The allocator is GLOBAL, so a change deep in a loop body can shift the WHOLE
function's allocation, including the prologue. Precomputing a call argument into
a loop-local can move a prologue copy GCC otherwise copy-prop-folds away.
**Reference:** `InitHiraRmd_80047FBC` prologue staging — see
[[dead-vars-local-array]].

## Lever D — dead self-assignment to break a value association

A dead `param = 0;` (DCE'd to ZERO emitted instructions, since the param is
unused afterward) can break GCC's `paramreg == value` association so a later
use binds to a callee-save instead of the still-live param register.
**Reference:** `InitHiraRmd_80047FBC` second pointer (`$s4` vs `$a0`) — see
[[dead-vars-local-array]].

## Order of attack

1. **Step 0 diagnosis.** Confirm the target uses the *preferred* (lower) reg →
   you're the anomaly, so it's pure-C-reachable.
2. **Run the vanilla (pin-free) permuter** from a pin-free base — it finds type
   mutations (Lever B) and some structural levers on its own.
3. **If it plateaus 1 register short**, dump `-da .greg`, find the blocker, and
   apply Lever A (block-local split) / C / D by hand.
4. **Verify with `dc.sh build` (full SHA1).** Isolated permuter scores can be
   UNFAITHFUL for functions with many relocations (saTan0Main's matching
   version scored 200 in isolation, while a non-matching variant scored 85) —
   always confirm a candidate in the full build, not just the permuter score.

## Confirmed limit — cpu_side_move_dir_4 (system.c, 2026-05-29)

`cpu_side_move_dir_4` plateaus at sandbox-distance **20** (4 regfix substs $3→$4 + a
15-element reorder rule) and **none of the pure-C levers above + permuter + cc1psx
cross-check closes it**. Pinned in full so a future resume doesn't re-derive it.

### Two COUPLED mechanisms (instrumented)

1. **GCC 2.7.2 global allocno-priority tiebreaker** (`global.c:624`,
   `allocno_compare`). arg0 (pseudo 72, `2 refs / 78 insn live`) and tbl_125c
   (pseudo 77, `3 refs / 152 insn live`) have priorities
   `pri = floor_log2(n_refs) * n_refs / live_length * 10000`:
   - arg0: `1 * 2 / 78 * 10000 = 256`
   - tbl_125c: `1 * 3 / 152 * 10000 = 197`

   arg0 wins on priority alone; even at a true tie the tiebreaker
   (`*v1 - *v2` = allocno number ascending = pseudo creation order) gives the
   param-save pseudo (created at RTL function entry) the win. **Result:** arg0
   gets `$s3`, tbl_125c gets `$s5` — target uses them flipped (tbl_125c in `$s3`,
   arg0 in `$s5`). The `register s32 *tbl_125c asm("s3")` pin is silently
   IGNORED by GCC because arg0 wins the same slot.

2. **GCC 2.7.2 list-scheduler priority for the `lw D_800F19C0` insn**
   (`sched.c:2385`, `rank_for_schedule`). At the `tslTm2LoadImage_2` call boundary,
   the ready queue contains `lw $a1, D_800F19C0` (insn 115, downstream depth ~1 —
   only feeds the call) and the table-arithmetic chain (`lbu → sll → addu → lw`,
   depth ~5). Both have only `REG_DEP_ANTI 86` (the call) — independent
   semantically. The scheduler picks INSN_PRIORITY-highest first; the longer
   table chain wins ⇒ `lw D_800F19C0` defers until just before the call. Target's
   asm has it interleaved EARLY (between the two `lbu`s) — different scheduler
   decision from the same RTL.

### Volatile-read lever PROOF (mechanism 1)

`*(volatile s32 *)tbl_125c;` before the debug_printf call bumps tbl_125c's
ref count 3 → 4 ⇒ `floor_log2(4)=2`, `pri = 2 * 4 / 152 * 10000 = 526` (beats
arg0's 256). Result: **tbl_125c lands in `$s3`** matching target; sandbox score
drops 20 → 16. **But** the volatile read emits an extra `lw $v0,0($s3)`
instruction (tier-3, non-matching). Proves the allocation IS C-reachable —
the lever just costs 1 insn that no non-side-effect-bearing C construct
emulates (`(void)tbl_125c`, `s32 *t = tbl_125c`, `(s32)tbl_125c & 0`,
sentinel pointer comparison — all DCE'd or CSE'd before priority calculation).

### cc1psx cross-check finding (per [[cc1psx-calibration-only]])

Built `tmp/csmd4_min.c` (137-line minimal standalone, hand-extern decls — DODGES
DOS 640K, see `saeft_psx_debug.sh` recipe). Ran BOTH compilers on the same C:

| | cc1psx (PsyQ) | decompals (ours) | TARGET asm |
|---|---|---|---|
| arg0 reg | `$s3` | `$s3` | `$s5` |
| arg1 reg | `$s4` | `$s4` | `$s6` |
| tbl_125c | `$s5` | `$s5` | `$s3` |
| `lw D_800F19C0` schedule | EARLY (interleaved) | LATE (after table arith) | EARLY (matches cc1psx) |
| arg5 stack-store delay slot fill | inline | filled into jal | nop |

**Both compilers produce the SAME register allocation from my C** — target's
allocation is unreachable by compiler choice alone. cc1psx matches target's
SCHEDULING (mechanism 2) but not target's ALLOCATION (mechanism 1). So this is
NOT a decompals-vs-PsyQ fork wall — it is a C-source structure that produces
target's `$s3 ↔ $s5` swap that I have not found.

### Partial lever found (mechanism 2 — works standalone, regresses in full)

```c
{
  s32 arg4, arg5, arg3;
  void *arg1;
  arg1 = D_800F19C0;
  arg3 = D_800A11DC[D_800A11D5];
  arg4 = tbl_125c[idx_1494[0]];
  arg5 = tbl_125c[idx_1494[1]];
  debug_printf(&D_800161C8, arg1, arg3, arg4, arg5);
}
```

In **standalone** (minimal .c), preloading BOTH arg1 + arg3 as locals extends
their downstream chains enough that decompals schedules `lw $5,D_800F19C0`
EARLY (matching target). In the **full sandbox**: score regresses 20 → 27
because arg3 preload also hoists the `D_800A11D5` byte read (target has it
LATE, idx 57-58) AND arg5's stack store gets pushed into `jal debug_printf`
delay slot (target has nop there). Function ends up 159 insns vs target's 160.

Preloading arg1 ALONE gets folded by CSE in the full context (back to score 20).
The arg3 preload was load-bearing for extending arg1's chain depth but
over-corrects the rest of the block.

### Permuter result (5000+ iters, clean offset-0 target)

`permuter/csmd4/` preserved. Base score 705 (= 21 reg-diff × 5 + 3 ins × 100 +
3 del × 100). Tested 4 base forms (arg4-only-inline, both-locals, both-bytes-
captured, +preload-globals); arg4-only-inline lowest at 705. Plateau over
5000+ iters; 5 tied-best mutations all type-mutation noise (`volatile unsigned
long`, etc.) that don't reduce the 3+3 ins/del structural cost. Permuter
exhausted at clean-target.

### Session-4 (2026-05-29) — executed every named lever, all negative

Per [[difficult-is-not-impossible]] § "Do NOT stop with documented unrun resume
avenues", session 4 took the prior session's "Resume here" list as a TODO and
empirically tested each candidate. All measured negative; the COUPLING gap
stands.

| Lever | Score | Δ insns vs baseline (160) | Verdict |
|---|---|---|---|
| **DImode chain mirroring matched siblings** (`unsigned long long new_var2; new_var=0xFF; new_var2=new_var; mask=new_var2;` at top, used as `temp & mask`) | 28 | +3 | FAIL — DImode pseudo constant-folded before priority calc; emits extra prologue insns |
| **DImode chain anchored at function top, body unchanged** | 28 | +3 | FAIL — same fold |
| **DImode chain feeding mask directly** (`temp = *idx_1494 & (s32)(new_var2 & 0xFFul)`) | 32 | +6 | FAIL — DImode promotion emits real `dsll/dsrl` codegen |
| **tbl_11dc local-var pattern** (extract `D_800A11DC` as a local pointer mirroring saEft01Init) | 32 | +3 | FAIL — extra pointer materialization, doesn't flip allocation |
| **tbl_125c lazy-init in do_timeout block** | 36 | −2 | FAIL — function shorter but worse alignment |
| **tbl_125c init at loop top (per-iter)** | 25 | −3 | FAIL — tbl_125c moves to `$s0` (still not target's `$s3`) |
| **Declaration-order swap** (tbl_125c declared first) | 20 | 0 | NO-OP — pseudo numbers for autos don't track decl order |
| **Removing register asm pin in source** | 20 | 0 | NO-OP — pin already stripped in sandbox |
| **Volatile-pointer-type tbl_125c decl** (`register volatile s32 *tbl_125c asm("s3")`) | 20 | 0 | NO-OP — volatile on POINTER, not deref data — no ref bump |
| **`*(volatile s32 *)tbl_125c;` before debug_printf** (the prior session's lever, re-verified) | 16 | +1 | TIER-3 (cheat) — confirms allocation IS C-reachable, but emits an extra `lw` |
| **arg1 preload via local** (`void *arg1_v = D_800F19C0;`) | 20 | 0 | CSE'd back |
| **arg1 preload + pin `register asm("a1")`** | 20 | 0 | pin stripped in sandbox, CSE eats the local |
| **Inline volatile cast on arg1** (`*(void* volatile*)&D_800F19C0`) | 22 | +2 | FAIL — emits extra addressing insns |
| **arg5 inlined into call** (no local var) | 20 | 0 | NO-OP |
| **All args inlined** (no arg4/arg5 locals) | 27 | 0 | FAIL — different scheduling |
| **arg4/arg5 evaluation order reversed** | 21 | 0 | FAIL — wrong sequence point |
| **tbl_125c assignment first** (before D_800F19B8 write) | 22 | 0 | FAIL |
| **idx_1495 init before idx_1494** | 22 | 0 | FAIL |
| **Extra ref to tbl_125c via stale `idx_1494` write** | 28 | 0 | FAIL — DCE'd or emits |

### Why no further pure-C lever is derivable

The greg (sandbox-stripped) shows:
```
;; 15 regs to allocate: 75 83 84 82 81 121 127 86 138 80 77 72 73 79 78
;; Register dispositions: 72 in 19  73 in 20  77 in 18  78 in 22  79 in 21
```
- arg0 = pseudo 72 → reg 19 ($s3)
- tbl_125c = pseudo 79 → reg 21 ($s5)  (target wants reg 19)
- The priority order ends `... 80 77 72 73 79 78` — tbl_125c (79) is LAST.

To flip the allocation, tbl_125c's priority must beat arg0's. The only known
mechanism that does this (volatile read at use site) emits a stray `lw` that no
non-side-effect C construct replicates. Every tested pure-C lever either:
- Doesn't change the priority sort at all (decls, declarations, harmless casts).
- Changes the sort but at the cost of extra instructions that exceed the
  saving — the couple compounds, doesn't cancel.

The same coupling defeats the DImode-chain hypothesis: the matched siblings'
chain works because `count = new_var2` is used as a real loop counter (3+ refs
through the loop body, value live across decrements). In cpu_side_move_dir_4
there is no loop counter; the chain has nowhere natural to live, so the DImode
pseudo either constant-folds away (no priority effect) or emits real DI arith.

The scheduling gap (mechanism 2) is structurally coupled to the allocation:
fixing one regresses the other (the standalone arg1+arg3 preload proof from
session 3 stands; the variants tested in session 4 either CSE-collapse arg1 or
hoist D_800A11D5 — neither helps in the full-function context).

### Status (session 4) — superseded by session 5

Session 4 stopped at floor 20 with the conclusion "cc1 instrumentation is
project-scope". **That was wrong.** Session 5 built a SEPARATE instrumented cc1
(`tmp/gccdbg/cc1`, canonical untouched, verified by SHA1) in 0 minutes (it
already existed from the saEft00Add session — `cross-jump-store-tail-merge.md`'s
`tmp/gccdbg`) and the instrumented dump produced the LEVER directly.

### Session-5 (2026-05-29) — instrumented cc1 unlocks the LEVER. Score 20 → 6.

Ran `tmp/gccdbg/cc1` (a separate build of GCC 2.7.2 with the `BB2_ALLOC_DEBUG`
env-var hook from `global.c:575` active — canonical `tools/gcc-2.7.2/build/cc1`
untouched). Got cpu_side_move_dir_4's full allocno table at priority-calc:

```
ord | pseudo | hardreg     | nrefs | livelen | pri  | identity
 10 |   77   | 18 ($s2)    |   7   |   154   | 909  | idx_1494
 11 |   72   | 19 ($s3)    |   2   |    78   | 256  | arg0   (target wants $s5)
 12 |   73   | 20 ($s4)    |   2   |    81   | 246  | arg1   (target wants $s6)
 13 |   79   | 21 ($s5)    |   3   |   152   | 197  | tbl_125c (target wants $s3)
 14 |   78   | 22 ($s6)    |   2   |   148   | 135  | idx_1495 (target wants $s4)
```

**The lever the dump exposed:** route `idx_1494` THROUGH `tbl_125c` via constant
pointer arithmetic. `idx_1494 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 -
(s32)D_800A125C));` is algebraically equivalent to `idx_1494 = &D_800A1494;`
(the delta is a link-time constant), so **GCC's combine pass folds it back to
the symbolic form (zero extra emitted bytes)** — but the RTL passes BEFORE
flow.c see tbl_125c referenced one more time, bumping its `reg_n_refs` from 3
to 4:

  pri = floor_log2(4)*4/152*10000 = **526** (beats arg0's 256).

ALLOCDBG with the chain confirms the new priority order:
```
 ord 9: pseudo 77 (idx_1494) → reg 17 ($s1), pri 1818  (was reg 18)
 ord 11: pseudo 79 (tbl_125c) → reg 19 ($s3), pri 657  ← matches target!
 ord 13: pseudo 72 (arg0) → reg 21 ($s5), pri 256  ← matches target!
 ord 14: pseudo 73 (arg1) → reg 22 ($s6), pri 246  ← matches target!
```

`tbl_125c → $s3`, `arg0 → $s5`, `arg1 → $s6`, `idx_1495 → $s4` — **three of
four register diffs flipped to target's allocation, zero extra emitted bytes**.
Sandbox score 20 → **16**.

**Second lever from the same probe:** the residual `idx_1494 → $s1` (target
$s2) came from idx_1494's livelen dropping from 154 to 77 (its definition
moved later in the chain), bumping its priority above `saved` (pseudo 80 at
$s2). Splitting the `temp = (*idx_1494) & new_var` read into a block-local copy
(`u8 *idx_local = idx_1494; temp = (*idx_local) & new_var;`) shifts the RTL's
last-use of idx_1494 in a way that flips it back to $s2 (Lever A from this
rule — block-local var split — empirically applies). Sandbox score **16 → 6**.

**Floor reached this session: sandbox-distance 6** (from baseline 20, 70%
reduction). The combined source change:

```c
extern u8 D_800A1494;
s32 cpu_side_move_dir_4(s32 a0, u8 *a1) {
    /* ... unchanged decls ... */
    register u8 *idx_1495 asm("s4");
    register s32 *tbl_125c asm("s3");
    /* ... */
    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    tbl_125c = D_800A125C;
    /* THE LEVER — bumps tbl_125c refs 3→4, flips priority, zero bytes emitted */
    idx_1494 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C));
    idx_1495 = idx_1494 + 1;
    /* ... rest unchanged ... */

    /* THE SECOND LEVER — block-local read flips idx_1494 from $s1 to $s2 */
    {
        u8 *idx_local = idx_1494;
        temp = (*idx_local) & new_var;
    }
    /* ... */
}
```

The remaining 6 diffs are the second coupled mechanism (`sched.c:2385`
list-scheduler priority keeping `lw $a1, D_800F19C0` deferred until just before
the call instead of interleaved with the table arithmetic) plus its cascade of
`+4`-byte branch offsets. **Every named lever for the *scheduling* gap in this
session — arg1 preload as local, arg1 preload with volatile cast, arg1
mid-block insertion, arg1+arg3 dual preload — was tested at this base and
scored 6/14/6/14**; the scheduling gap remains genuinely structural.

### Why this session reverts the source change (genuine partial-progress, not park)

The source change reaches score 6 but **breaks the existing 5 regfix rules** —
they were calibrated to maspsx indices in the BASELINE asm, and the chain
shifts indices. Committing in this state would: (a) break the oracle, (b) leave
the function in a worse state than baseline. The source change is reverted and
the technique documented above (fully reproducible in ~30 seconds from a clean
sandbox). The instrumented `tmp/gccdbg/cc1` is preserved for resume.

### Session-6 (2026-05-29) — instrumented sched.c probe EXECUTED.

Added `getenv("BB2_SCHED_DEBUG")` hooks to `tools/gcc-2.7.2/sched.c` at TWO
points: (1) right after the initial `SCHED_SORT` in `schedule_block` — dumps
the full ready queue with each insn's UID, kind (INSN/CALL/JUMP),
`INSN_PRIORITY`, `INSN_REF_COUNT`; (2) at the schedule-pick point (just before
`ready += 1; n_ready -= 1;`) — dumps for each clock tick which insn was
picked, the ready queue's full state with priority + LUID, and
`last_scheduled_insn`. Rebuilt `tmp/gccdbg/cc1` (sched.o + relink) with the new
hook live; canonical `tools/gcc-2.7.2/build/cc1` mtime/SHA1 untouched
(`045c9543d3…` May 18, 4263208 bytes vs the dbg cc1's `9f6c43d9b1…` May 29).

Ran `BB2_SCHED_DEBUG=1 tmp/gccdbg/cc1 ...` on the score-6 sandbox source (chain
+ idx_local levers from session 5 applied). The cpu_side_move_dir_4 do_timeout
block is block 3 of the function's sched run (RTL `block=3 n_insns=22`). The
instrumented dump exposed the priority gap directly.

**sched2's INSN_PRIORITY table for the do_timeout block:**

| insn | kind | pri | refcnt | RTL                          |
|------|------|-----|--------|------------------------------|
| 120  | INSN | **1** | 3 | `lw $5, D_800F19C0`          |
| 125  | INSN | **2** | 2 | `lbu $2, D_800A11D5`         |
| 129  | INSN | 3   | 2 | `sll $2, $2, 2`              |
| 139  | INSN | 4   | 1 | `lw $6, D_800A11DC($2)`      |
| 144  | CALL | 4   | 1 | `jal debug_printf`           |

The downstream RTL chain rooted at insn 120 (D_800F19C0 load):
```
120 (lw $5) → 121 (set a1, $5) → 144 (call) — depth 2
```
The chain rooted at insn 125 (D_800A11D5 load):
```
125 (lbu $2) → 129 (sll $2,$2,2) → 139 (lw a2, D_800A11DC) → 144 (call) — depth 4
```

`INSN_PRIORITY` ≈ chain depth. D_800A11D5's chain is 2 deeper than
D_800F19C0's, so it gets priority 2 vs 1. `rank_for_schedule` picks the higher-
priority insn FIRST (and the picked insn prepends to the chain LIFO, so the
highest-priority pick emits LAST). Net: D_800F19C0 emits AFTER D_800A11D5 in
the asm — matching cc1's raw output (`lbu $2, D_800A11D5` at line 61, `lw $5,
D_800F19C0` at line 62).

### The required lever: extend D_800F19C0's downstream depth from 2 → ≥4

For `lw $a1, D_800F19C0` to be picked AHEAD of `lbu D_800A11D5` (i.e., emit
EARLIER in the asm, matching target), its downstream chain must be longer than
D_800A11D5's (depth 4). To go from 2 → ≥4 requires inserting 2+ RTL insns that
consume `D_800F19C0`'s loaded value, survive combine, and feed into the call's
args/return.

### Every lever tested at the score-6 base — all negative

| Lever | Score | Δ insns | Verdict |
|---|---|---|---|
| `arg4 = tbl_125c[...] \| ((s32)D_800F19C0 & 0)` (folded chain) | 16 | 0 | FAIL — folded before sched1; no INSN_PRIORITY effect |
| Move `D_800F19C0 = …` init EARLIER (before the table-chain compute) | 18 | 0 | FAIL — STORE position doesn't affect the LOAD's priority; scheduling regression elsewhere |
| `arg1_v = (void*)((s32)D_800F19C0 + ((s32)tbl_125c - (s32)D_800A125C))` (mirror lever 1 for D_800F19C0) | 20 | +4 | FAIL — chain materialises; combine doesn't fold the inter-symbol delta here (tbl_125c is a callee-save pseudo, not a symbol_ref at combine time) |
| `dummy_chain = (s32)arg1_v; arg4 = … ^ (dummy_chain & 0);` | 16 | 0 | FAIL — CSE eats it after combine |
| `D_800F19C0 = (void*)((s32)tbl_125c + ((s32)&D_80016240 - (s32)D_800A125C));` (chain the STORE init) | 16 | 0 | NO-OP — the LOAD's chain is what affects priority, not the store's |
| `extern void *volatile D_800F19C0;` (volatile on the global) | 16 | 0 | NO-OP — volatile forces the LOAD but doesn't extend its downstream |

### Why lever 1's symmetry doesn't apply here

Lever 1 worked because `tbl_125c`'s pseudo (born from a symbol_ref load) was the
RTL pseudo whose REFERENCES we needed to bump for global.c. The delta-arithmetic
chain `idx_1494 = (u8*)tbl_125c + delta` added a USE of tbl_125c at combine
time (before global.c's count), then combine folded the address back to
symbol_ref — zero emitted bytes.

For D_800F19C0, the loaded value is consumed ONCE (as the call's arg1). There
is no "natural" RTL use site to chain through that survives combine. Every
attempt to chain D_800F19C0's value into arg4/arg5 computation either:
- CSE-eats the chain (`x & 0`, `x ^ 0`, `x - x` all fold).
- Materialises the chain as real instructions (regression).

The combine pass's symbolic-address folding (which makes lever 1 free) does
NOT apply to **loaded values** — those are pseudos with live ranges, not
symbol_refs.

### Status (session 6)
Mechanism FULLY characterised at the RTL-instruction level via instrumented
sched.c — INSN_PRIORITY 1 vs 2 from chain depth 2 vs 4. The C-source levers
that flip it WITHOUT emitting instructions don't exist in the explored search
space (every tested chain extension either folds or materialises). Sandbox-
distance floor remains **6** with chain + idx_local applied (per session 5).
The chain + idx_local source change is reverted to keep oracle green (the
chain shifts maspsx indices, breaking the existing 5 regfix rules — full match
needed for retire to drop them). The instrumented dbg cc1 + sched.c hooks are
preserved in `tmp/gccdbg/` (gitignored) for the next session's resume.

### Session-7 (2026-05-30) — avenues #2, #3, #4 EXECUTED. Genuine fork divergence confirmed.

**Avenue #2 — `debug_printf` varargs declaration: NO EFFECT.**
Changed `extern void debug_printf(void *, void *, s32, s32, s32);` to
`(void *, ...)` and to `()` (full varargs). Both produced identical sandbox
score 16, identical asm. `CALL_INSN_FUNCTION_USAGE` for fixed-mode arg passings
doesn't change when the declaration is varargs.

**Avenue #3 — instrument `priority()` (sched.c:1424). DECISIVE FINDING.**
Added `BB2_PRIO_DEBUG` hook that dumps per LOG_LINKS walk iteration: insn,
predecessor, REG_NOTE_KIND (0=true, 14=anti, 15=output), pred_priority, cost,
contributing priority, running max. Rebuilt `tmp/gccdbg/cc1` (now has all
THREE hooks: ALLOCDBG + SCHEDDBG + PRIODBG; canonical SHA1 `045c9543d3...` May
18 untouched; dbg SHA1 `a17fc6bbcb...` May 30).

PRIODBG dump for cpu_side_move_dir_4's debug_printf block (sched2 — the pass
that produces the final asm-emission order):

| insn | identity            | preds (kind=14 ANTI, 0 TRUE) | final_pri |
|------|---------------------|-------------------------------|-----------|
| 107  | `lw $5, D_800F19C0` | **ONLY** pred 89 (CALL, ANTI) | **1** |
| 110  | `lbu $v0, D_800A11D5` | pred 100 (ANTI), pred 102 (ANTI), pred 89 (ANTI) | **2** |

**The mechanism, exactly:** insn 107 (D_800F19C0 load) has ONLY ONE predecessor
in its LOG_LINKS — the CALL via REG_DEP_ANTI. No other RTL insn depends on it,
no other insn it depends on except the call. Priority = 1.

Insn 110 (D_800A11D5 load) writes to `$v0`, which is REUSED by the table
arithmetic chain (insns 98 → 100 → 102 read $v0). So 110 has ANTI-deps on 100
AND 102 (writing $v0 would clobber their reads). Those preds have priorities
2 and 2 → 110's pri = max(2, 2, 1) = 2.

**To raise insn 107's pri above 110's pri** requires additional predecessors
with priority ≥ 2. The only options:
- **TRUE dep on 107**: needs 107's address (`D_800F19C0`) to depend on an
  earlier computation. The symbol_ref is a leaf — no natural producer survives
  combine.
- **ANTI dep on 107**: needs earlier insns to READ $a1 (the register 107
  writes). Nothing in the block does (args are set into $a0/$v0/$v1/$a2/$a3
  by separate insns; $a1 is set ONLY by 107).
- **OUTPUT dep on 107**: needs earlier insns to WRITE $a1. None do.

Every C-source lever that would manufacture such a dep either folds to a no-op
(combine resolves it away before sched1) or materialises real instructions —
there is no GCC RTL pass that synthesizes a fake dependency between unrelated
leaf loads.

**Avenue #4 — cc1psx cross-check at lever-applied source.** Compiled
`tmp/csmd4_min_lever.c` (137-line minimal `.i` with chain + idx_local applied)
through cc1psx → prologue_fix → maspsx → as. Recipe in
`tmp/build_csmd4_psx_minimal.sh` (the full system.c does NOT cc1psx-compile —
its file-scope `__asm__("glabel …")` blocks use multi-line C-string
concatenation cc1psx rejects; the minimal `.i` is required).

**Session-7 mis-reported this avenue.** The original write-up compared the
region around `lw D_800F19C0` (lines 49-52 of the asm), saw cc1psx interleave
D_800F19C0 early, and concluded "cc1psx + lever source + maspsx + as produce
target bytes for this function" — WITHOUT measuring the full function diff.
**Session-7b corrects it.**

### Session-7b (2026-05-30) — actual byte-comparison: cc1psx ≠ target either

Byte-compared cc1psx's `cpu_side_move_dir_4` (160 insns) vs target
`build/src/system.o`'s `cpu_side_move_dir_4` (160 insns). Filtered `jal`
reloc-name noise (both objects emit `jal 0x0` + R_MIPS_26 to external symbols;
objdump's literal symbol name differs between standalone-min vs full-build but
the bytes are the same).

**Result: 23 real diffs, 160/160 same insn count, 137/160 masked-asm match.**

Diff categories (measurement, not inference):
1. **idx_1494 register: cc1psx puts it in `$s1`, target wants `$s2`.**
   Same diff our dbg cc1 has at this lever-applied source. cc1psx and
   decompals produce IDENTICAL idx_1494 allocation here.
2. **debug_printf-block scheduling: cc1psx's order differs from target**
   (idx 53-65 region). The "early `lw $a1, D_800F19C0`" the prior write-up
   pointed to is real, BUT cc1psx's surrounding insn order is NOT target's
   either — there's an additional shuffle the region-level inference missed.

cc1psx is NOT closer to target than decompals on this function. Both compilers
hit the SAME register-allocation gap (idx_1494 → wrong $s reg), AND cc1psx
introduces its own scheduling divergence on top of the lever's allocation
shift. The previously-claimed "PsyQ-vs-decompals fork divergence" was not real.

This is consistent with the project's compiler_parity record (0/282 cc1psx
wins) — re-affirmed by direct measurement here, not undercut.

### Status (session 7b — corrected)

cpu_side_move_dir_4's pure-C floor with the chain + idx_local levers applied is
sandbox-distance 16 (decompals/dbg cc1). cc1psx at the same source produces 23
diffs vs target — neither compiler matches.

The work continues on the C-source axis. The PRIODBG dump's specific finding —
insn 107 (D_800F19C0 load) has ONLY one predecessor (CALL via REG_DEP_ANTI),
no natural C use-site can extend its downstream chain without emitting bytes —
remains accurate FOR THAT INSN. But the bigger remaining gap is the same
idx_1494/saved $s1↔$s2 allocation issue, which afflicts BOTH compilers and
**is not a sched.c issue at all** — it's a global.c allocation tie the chain
lever inadvertently created (idx_1494's livelen dropped from 154 to 77 when
its def moved later via the chain, bumping its priority above saved's).

### ALLOCDBG re-run on chain-applied source (recap from session 5)

- idx_1494 (pseudo 77): refs 7, livelen 77, pri 1818 → $s1
- saved (pseudo 80): refs 2, livelen 21, pri 952 → $s2
- tbl_125c (pseudo 79): refs 5, livelen 152, pri 657 → $s3 ✓ (target)

To flip $s1/$s2 between idx_1494 and saved: saved needs pri > 1818, OR
idx_1494's pri must drop below 952. With idx_1494's 7 refs, dropping pri below
952 requires livelen > 73 (already 77, marginal) AND a few-insn livelen
extension would do it — **testable as a session-8 lever**, not tried yet
(session 5's claim that idx_local achieved this flip didn't re-measure;
session 7's re-measurement shows it doesn't).

### Session-8 (2026-05-30) — executed session-7b's named avenues; found a new lever.

**Avenue 1 — extend idx_1494's livelen past 77.** Tested two forms:
(a) `return temp + ((s32)idx_1494 & 0)` → folds to plain `return temp`, ALLOCDBG-
verified pseudo 77 unchanged (refs=7, livelen=77, pri=1818). DCE'd before flow.c.
(b) `*idx_1494 = (u8)temp;` before `return temp` (a real late store) → ALLOCDBG
shows pseudo 77 refs=8, livelen=89, **pri=2696** (BUMPED, wrong direction —
refs grew faster than livelen). The real-store version was tried only because
the fold version proved unmeasurable; with refs up, priority rose so the
saved↔idx_1494 tie didn't flip. Sandbox 16→17 with +1 insn.

Lever-A from this rule (block-local `u8 *local_idx = idx_1494;`) — CSE'd back to
pseudo 77 unchanged. No livelen extension that keeps refs constant survived.

**Avenue 2 — different chain expression that keeps idx_1494's livelen at 154.**
Tested the key insight: lever-1's chain `idx_1494 = (u8*)tbl_125c + delta_to_1494`
replaces idx_1494's def site, which drops its livelen from 154 to 77 (the
source of the saved↔idx_1494 tie). Three new formulations:
- (a) `idx_1495 = (u8*)tbl_125c + delta_to_1495` (chain via idx_1495 instead):
  ALLOCDBG verifies idx_1494 KEEPS livelen 154, ALL 5 reg slots match target —
  but emits +1 insn (D_800A1495 address materializes as `lui+addiu` because it
  isn't referenced elsewhere as a symbol that combine could fold). **Sandbox
  20→9 with +1 insn (build 161 vs target 160).** Not retire-able as-is.
- (b) `D_800F19C0 = (void*)((u8*)tbl_125c + delta_to_D_80016240)` (chain the
  STORE init of D_800F19C0 via tbl_125c): **THIS IS THE NEW BEST LEVER.**
  ALLOCDBG verifies:
    saved (pseudo 80): pri 952 → **$s1** ✓
    idx_1494 (pseudo 77): refs 7, livelen 154, pri 909 → **$s2** ✓ (NATURAL!)
    tbl_125c (pseudo 79): refs 5, livelen 152, pri 657 → **$s3** ✓
    arg0 (pseudo 72): pri 256 → $s4 (target $s5)
    arg1 (pseudo 73): pri 246 → $s5 (target $s6)
    idx_1495 (pseudo 78): pri 135 → $s6 (target $s4)
  5/6 reg slots correct; only idx_1495 wrong (rotated with arg0/arg1).
  D_800A125C and D_80016240 are sequential symbol_refs both reachable from
  combine's fold table, so the chain emits **ZERO extra instructions**.
  **Sandbox 20→15, build 160 (matches target count).**
- (c) Dual chain (a)+(b) combined: sandbox 17 with +1 insn (chains compound).
- (d) Original lever-1 + chain (b): sandbox 16 (lever-1 still drops idx_1494
  into $s1, undoing the win).

### New session-8 lever recipe (score 20 → 15, build matches target count)

```c
extern void *D_800F19C0;
...
s32 cpu_side_move_dir_4(s32 a0, u8 *a1) {
    /* ... unchanged decls ... */
    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    tbl_125c = D_800A125C;
    idx_1494 = &D_800A1494;                 /* NATURAL form, keeps livelen 154 */
    idx_1495 = idx_1494 + 1;
    D_800F19BC = 0;
    /* THE LEVER: bump tbl_125c's refs via the STORE-init chain; combine folds */
    D_800F19C0 = (void *)((u8 *)tbl_125c + ((s32)&D_80016240 - (s32)D_800A125C));
    /* ... rest unchanged ... */
}
```

The delta `(s32)&D_80016240 - (s32)D_800A125C` is a link-time constant.
Combine folds the entire expression back to the symbolic store
`D_800F19C0 = &D_80016240` (zero emitted bytes) but the RTL passes BEFORE
flow.c see tbl_125c referenced one more time — bumping its refs 3→5 (with
loop-depth weighting) and its priority 197→657 in global.c's allocno sort.

### Remaining 15 diffs (the un-closed residual)

All flow from one cause: idx_1495 in $s6 instead of $s4 (cascades to arg0/arg1
rotation and a +4-byte branch offset chain). To flip idx_1495's priority above
arg0's (256), idx_1495 needs pri > 256. Current refs=2, livelen=148, pri=135.
With refs=4 (added via 2 loop-depth-weighted uses): pri = 2*4/148*10000 = 540
WINS. But:
- `(void)idx_1495` near function end — DCE'd, no effect.
- `*idx_1495 = (u8)((s32)idx_1495 & 0)` — emits a real store (+1 insn, sandbox
  21).

The same structural problem session-7b identified for D_800F19C0's INSN_PRIORITY
applies here too: combine folds chains that don't emit bytes (no priority
effect), and chains that DO emit bytes don't pay off (priority gain < +1 insn
cost).

### Status (session 8)

**Sandbox-distance floor pushed from 16 to 15** with the new D_800F19C0-store-
via-tbl_125c chain (avenue-2 variant (b)). Build matches target count exactly
(160 insns). 5 of 6 callee-save regs match target's allocation through pure C
with zero emitted byte overhead. The remaining 15 diffs are the idx_1495/$s4
miss + the +4-byte branch-offset cascade.

Source change reverted (the chain shifts maspsx indices, breaking the existing
5 regfix rules); rule update fully documents the recipe for the next session.

### Session-9 (2026-05-30) — avenues A/B/C/D EXECUTED. Floor remains 15.

Per directive, executed all four named avenues from session 8 with ALLOCDBG
verification.

**Avenue A — chain idx_1495 via D_800F19A8** (`idx_1495 = (u8*)&D_800F19A8 +
(delta_to_D_800A1495)`). ALLOCDBG: ALL 6 callee-save slots match target's
allocation exactly (saved $s1, idx_1494 $s2, tbl_125c $s3, idx_1495 $s4, arg0
$s5, arg1 $s6). BUT sandbox **27, build 161** — the chain pulls an extra
spilled pseudo (ord=4 pseudo=90 hardreg=-1) growing the stack frame from 56 to
64 bytes AND emits +1 insn. Net regression despite the perfect allocation.

**Avenue B — chain idx_1495 via tbl_125c + delta_to_D_800A1495**. Sandbox
**17, build 161**. ALLOCDBG: saved/idx_1495/arg0/arg1 match target, BUT
tbl_125c and idx_1494 swapped ($s2 ↔ $s3). The chain bumped tbl_125c refs to
7 (pri 921) and idx_1494 stayed at 6 (pri 779), so tbl_125c wins $s2. Plus +1
insn for D_800A1495 address mat.

**Avenue C — trivial DCE'd-in-jump2 use**. Tried `if (idx_1495 ==
(u8*)0xFFFFFFFF) goto _dead_; _dead_:` — DCE'd before flow.c, ALLOCDBG
confirms idx_1495 still refs=2 unchanged. Tried `if (idx_1495 < idx_1494)
return -1;` (provably false but combine doesn't see it) — emits a real
comparison +3 insns, sandbox 26.

**Avenue D — different fold-targets**. Tried chaining the store through
`idx_1494` instead of `tbl_125c`: sandbox 28 (bumps idx_1494 refs to 9,
shifting allocation in the wrong direction). Other fold-target variants would
change semantics of the store.

### Bonus finding from variant exploration

Routing arg4/arg5 through idx_1495 (instead of idx_1494) DOES bump idx_1495's
refs — the cleanest such form is `arg5 = tbl_125c[*idx_1495];` which gets
idx_1495 to refs=3 (pri 202 — still below arg0's 256). The fully-routed form
`arg4 = tbl_125c[idx_1495[-1]]; arg5 = tbl_125c[idx_1495[0]];` gets idx_1495
to refs=4 (pri 540, beats arg0!) BUT tbl_125c and idx_1494 swap ($s2/$s3),
sandbox 17. The tradeoff is real: idx_1494 and idx_1495 compete for ref-count;
bumping one drops the other.

### The fundamental coupling (session-9 conclusion)

Three pseudos (idx_1494, tbl_125c, idx_1495) need specific allocations:
- idx_1494 → $s2 (target): needs pri > saved's 952 → refs ≥ 6 at livelen 154
- tbl_125c → $s3 (target): needs pri > arg0's 256, achieved with refs 5+ at
  livelen 152 (currently 657 via the chain lever)
- idx_1495 → $s4 (target): needs pri > arg0's 256 → refs ≥ 4 at livelen 148

ALL THREE simultaneously requires refs totaling 15+ across idx_1494 + idx_1495
(at +5 total weighted refs over baseline). The natural C provides 7+2=9 refs.
Every C-source restructure that adds a ref to one drops a ref from the other
(they're computed from the SAME indexing operations). Adding refs that
preserve both via separate use sites costs ≥ +1 emitted instruction.

The remaining 15 diffs at the score-15 base are fundamentally tied to this
3-way priority gap that no single C-source structure rebalances within the
combine-foldable lever class.

### Genuine resume avenues (session 9 untried)

1. **Permuter from the score-15 base** — directed at the idx_1495 priority
   slot. Untried in any session since session 7b's PRIODBG conclusion shifted
   the focus to specific dep edges. The combine-foldable chain levers have
   broadened the lever-class; permuter may find a combination not derivable
   by hand.
2. **Project-wide rodata reorder** placing D_800A1495 adjacent to a
   sometimes-referenced symbol so D_800A1495 itself becomes combine-foldable
   without +1 insn cost (same policy class as saEft00Add precedent).

**Not viable** (empirically disproven):
- Pure-C varargs declaration change (avenue #2 of session 7 — score unchanged).
- Adding C-source dependencies to extend insn 107's RTL chain (session 7's
  avenue #3 PRIODBG dump proves no natural use-site exists; every synthetic
  chain either folds or materialises).
- **Per-function cc1psx opt-in** (session 7b corrected): the byte comparison
  measures 23 diffs vs target. cc1psx is NOT the lever.
- Session 7b's "extend idx_1494 livelen" lever (avenue 1) — DCE'd or refs grow
  faster than livelen; both folded-and-real variants fail to flip $s1↔$s2.
- Session 7b's "different chain formulation" via D_800A1495 alone (avenue 2a):
  produces all 5 reg matches BUT +1 insn (sandbox 9, build 161).
- Session 9's avenue A (chain idx_1495 via D_800F19A8): ALL 6 regs match BUT
  +1 insn AND extra frame spill (sandbox 27, build 161, frame 56→64).
- Session 9's avenue B (chain idx_1495 via tbl_125c + D_800A1495 delta): +1
  insn AND tbl_125c/idx_1494 swap (sandbox 17, build 161).
- Session 9's avenue C (DCE'd-in-jump2 use): comparisons either DCE'd before
  flow.c (no ref bump) or emit real bytes (+3 insn cost).
- Session 9's avenue D (chain through idx_1494 instead of tbl_125c): sandbox
  28, idx_1494 priority shifts wrong direction.

## Confirmed limit — marionation_Exec (system.c, 2026-05-29)

`marionation_Exec` plateaus at sandbox-distance **56** with 44 rules (5 register-
rotation rotations on $s1–s4 + $s5–s7, 7 prologue rename substs, 7 epilogue rename
substs, 9 debug_printf scheduling substs, 1 15-element reorder, several
`.L128→.L999` label-dup tricks for delay-slot fills) and none of the pure-C levers
above + the chain-extender hypothesis from cpu_side_move_dir_4 closes it. Pinned in
full so the next attempt doesn't re-derive it.

### Same mechanism as cpu_side_move_dir_4, larger surface

cc1's NATURAL output (instrumented via `tmp/me_pre.sh`, pre-prologue_fix raw):
```
move $21, $4      ; arg0 → $s5 (low callee-save)
move $17, $5      ; arg1 → $s1 (lowest callee-save)
…then la $22, D_800A125C    ; tbl_125c → $s6
   la $19, D_800A1494       ; idx_1494 → $s3
   addu $23, $19, 1         ; idx_1495 → $s7
   addu $20, $19, 2         ; idx_1496 → $s4
```
Target wants: arg0→$s7, arg1→$s4, tbl→$s5, idx_1494→$s2, idx_1495→$s6, idx_1496→$s3.

The args win LOW callee-saves because **args are the first pseudos created at RTL
function entry** (pseudo creation order = allocno number ascending; in true priority
ties the lower allocno wins — `global.c:624`). arg0 and arg1 have very long live
ranges (live entire function, from entry to the `if (a0 != 0)` check at exit), but
*also* have the lowest allocno number, so they outrank tbl_125c / idx_149X on the
tiebreaker. Same mechanism as cpu_side_move_dir_4's `arg0 vs tbl_125c` tie.

### Chain-extender hypothesis (from the brief): already present, does not close it

The brief proposed: "adding a third loop-local pseudo with similar ref-count profile
(e.g. `s32 *idx_1496 = idx_1494 + 2;` even if unused) can raise the tbl/idx pseudo's
priority above arg pseudos." **`idx_1496` is ALREADY in `marionation_Exec`'s body**
(`src/system.c:503,515`) — both as a declaration AND as the base for the `*idx_1496`
/ `*(idx_1496 - 1)` reads in the check block. The allocation rotation persists
regardless. Adding a third extender doesn't flip the priority because args still
beat all idx vars on allocno number alone (long live range = denominator helps but
not enough vs creation-order tiebreaker).

### Volatile-bump lever REGRESSES here (cf. cpu_side_move_dir_4)

For cpu_side_move_dir_4, `*(volatile s32 *)tbl_125c;` before the debug_printf bumped
tbl_125c's refs 3→4 and dropped sandbox 20→16 (but cost 1 extra emitted `lw`). For
**marionation_Exec the same lever INCREASES sandbox 56→79**. The volatile read
emits its own `lw` *and* perturbs the surrounding scheduling in this longer function
(many more pseudos in flight at the debug_printf site than in the cpu_side_move_dir_4
isolated probe), producing extra mis-scheduled instructions that exceed the score
"saved" by any allocation shift. Confirms the volatile lever is *site-specific* —
proved C-reachable in cpu_side_move_dir_4's tighter context but not transferable.

### Why this is more entrenched than cpu_side_move_dir_4

| | cpu_side_move_dir_4 | marionation_Exec |
|---|---|---|
| score gap | 20 (4 substs + 1 reorder) | 56 (44 rules) |
| reg-rotation cycles | 1 (arg0↔tbl_125c) | 2 (3-cycle + 4-cycle) |
| coupled mechanisms | 2 (allocation + scheduling) | 3+ (alloc + sched + delay-slot label-dup) |
| chain-extender available | new (would be a fresh idx_1496) | ALREADY PRESENT, doesn't flip |
| volatile-bump result | works (cost 1 insn) | regresses (cost > saved) |
| target-replication | proven C-reachable per-mechanism | not yet shown reachable |

### Status
NOT pure-C-matchable in this session, by the same coupling that defeated
cpu_side_move_dir_4 — at larger scale and with the chain-extender lever **already
in place** (so the brief's proposed unlock is empirically tested + closed). The
44-rule cluster encodes two coupled register-rotation cycles + a scheduling reorder
+ delay-slot label-dup tricks; any single C-source restructure moves one piece and
regresses others. Resume avenues identical to cpu_side_move_dir_4's two listed
("arg1-preload + chain-extender that doesn't hoist", "global rodata reorder") plus
one new one specific to marionation_Exec:

3. **Arg-storage form** — the target captures `arg0 → $s7` and `arg1 → $s4` as
   explicit `addu $sN, $aN, $zero` paired *interleaved with the prologue stores* at
   idx 2 and 4. This pairing pattern is what `prologue_fix.py` rewrites cc1's
   natural output INTO. cc1's natural arg-save instead occurs at `move $21, $4`
   right after the first `sw $21`. **`tools/prologue_config.json:2735-2750` lists
   marionation_Exec with the target's interleaved arg-save sequence — but the
   args land in $s5/$s1 (cc1's choice), and prologue_fix only RENAMES indices to
   match target's offsets, not the allocation.** Investigate: if the C source can
   be restructured so cc1 NATURALLY allocates arg0→$s7 and arg1→$s4 (e.g. via a
   long-lived earlier use of $s5 that displaces arg0), the 7 prologue rename
   substs + the rotation rules might fall out together. The body's actual uses of
   args are minimal (arg0 only at exit, arg1 only at the byte-copy `dst = a1`),
   so changing arg liveness from "live whole function" to "transferred to a
   different callee-save mid-function" could shift the allocno priority enough.

DO NOT (per this session's testing): rerun the volatile-bump lever (regressed);
add tier-3 `__asm__` register pins (the existing `register asm("$6")` on `check`
is score-inert per sandbox); declare it impossible (matching C exists per
[[difficult-is-not-impossible]], just not within the explored levers).

### Session-10 (2026-05-30) — directed permuter from CLEAN single-function target executed; floor 1125 / 61 register-rotation diffs.

Per the standing directive ("Do NOT stop with documented unrun resume avenues — KEEP
GOING"), this session executed the **last** named-untried avenue from the prior
session's resume list: the directed permuter on a **clean single-function target**
(same pattern that paid off for func_8003DBE4 and saEft00Add). Prior session
disproved the session-8 lever-carryover hypothesis from cpu_side_move_dir_4 to
marionation_Exec instrumentally; this session was the remaining clean-target run.

**Setup (reproducible):**
- `permuter/marionation/`: workspace.
- `target.s` = `tools/decomp-permuter/prelude.inc` (with `.set gp=64` removed) +
  `asm/funcs/marionation_Exec.s`. `target.o` assembles cleanly with
  `marionation_Exec` at offset 0 (verified via `mipsel-linux-gnu-objdump -t`).
- `base.c`: preprocessed HEAD's `src/system.c` with the build CPP flags, then
  `tools/decomp-permuter/strip_other_fns.py` IN-PLACE to keep only `marionation_Exec`.
  Residual file-scope `__asm__(...)` blocks for unrelated canonical-asm functions
  removed via `tmp/clean_base_marionation.py` (pycparser can't parse the unescaped-
  newline string literals; the asm is irrelevant to marionation_Exec's body).
- `compile.sh`: cheat-free pipeline (cc1 build-flags | prologue_fix | maspsx | as).
- **Base score (clean target, no relocation noise): 1470** = 42×5 reg + 1×60 reorder + 5×100 ins + 7×100 del.
  Confirms the honest pure-C distance is dominated by the documented register-
  rotation + scheduling gap, NOT noise.

**Run:** `timeout 3600 python3 tools/decomp-permuter/permuter.py permuter/marionation -j8 --stop-on-zero`

**Iterations: 16,800+ over ~8 wallclock minutes** (8 parallel workers). Best-score
trajectory:
- iter 1-627: 1470 baseline.
- iter 627: **1135** — first improvement.
- iter ~1227: **1125** (best legitimate match candidate).
- iter ~3945: **1110** — but the candidate deletes the `if (a1 != 0)` guard
  (semantically broken; NOT a legitimate match — the deletion drops insn count by 4
  which spuriously lowers the score).
- iters 4000-16,800+: **plateau at 1110/1125**. ~12,800 iterations of plateau, with
  ~270 candidates saved. No further improvement.

**The lever the permuter found (saved at `output-1125-1/`, `output-1125-2/`):**
Insert a dead alias of `a1` via a fresh local `u8 *new_var2;`, declared in the
function's locals, and either:
- (variant 1) assign `new_var2 = a1;` between the first loop's `goto done;` and the
  `check2:` label (dead path), and use `dst = new_var2;` instead of `dst = a1;` in
  the second loop; OR
- (variant 2) assign `new_var2 = a1;` before the `success:` label, and use
  `if (new_var2 != 0)` instead of `if (a1 != 0)` in the first loop's guard.

Either form bumps the `a1`-derived pseudo's reference profile enough to flip ONE
register slot in the second loop's body. Score 1470 → 1125 (Δ=-345).

**What 1125 means structurally (decisive measurement, `tmp/diff_cand_target.py`):**
- cand 1125-1 insns: 179.
- target.o insns: **179** — same insn count.
- Total normalized diffs: **61** — all are pure register-rename and instruction-
  position differences. NO insertion/deletion structural diffs at the masked level.

The 61 residual diffs are exactly the documented register-rotation gap:
- arg0 lands in `$s4`/`$s5` (cand) vs `$s7` (target) — the 4-cycle rotation.
- arg1 lands in `$s5`/`$s7` (cand) vs `$s4` (target).
- tbl_125c, idx_1494/1495/1496 carry the 3-cycle rotation cascade.

Same insns, just different register assignments rippling through 60+ slots — the
canonical "lever-class plateau" signature.

**Combined with cpu_side_move_dir_4 (session 10 — `permuter/csmd4_v8/`, ~80 cands / 3h, plateau at 505):**
Two independent permuter runs from clean single-function targets, both with the
documented register-routing levers applied, both plateaued at scores whose
residuals are pure register-rotation + scheduling diffs at matched insn count.
**Strongest possible chain-level evidence** the lever-class hits a structural
ceiling both functions share — the `global.c:624` allocno-priority tiebreaker plus
the `sched.c:2385` list-scheduler priority gap (sessions 5-9), neither flippable by
pure-C source restructure without emitting bytes.

**Why this CONFIRMS — not extends — the prior status:**
Sessions 7b/8/9 ran 19 distinct manual levers + a 5000-iter permuter on
cpu_side_move_dir_4 from its score-15 base; this session ran a 16,800-iter
permuter on marionation_Exec from its score-1470 base (clean target). Both plateaus
deliver the same verdict: matched insn count + residual = pure register-rotation
cluster. No "miraculous lever" hiding in the manual search space the permuter would
have found.

**Status (session 10):** Confirmed limit holds. marionation_Exec stays carrying
its 44 rules. Best clean-target permuter score: **1125** (saved at
`permuter/marionation/output-1125-1/`). NOT a pure-C-matchable function within the
explored lever-class + permuter randomisation.

**Avenues now genuinely exhausted in the named-untried set:**
- ~~Directed permuter from clean single-function target~~ — DONE (this session).
- ~~Session-8 lever-carryover from cpu_side_move_dir_4~~ — DONE (prior session,
  instrumented disproof; 5 variants tested negative).
- ~~Arg-storage form C-restructure for natural arg0→$s7, arg1→$s4~~ — implicitly
  subsumed by the permuter sweep (any single C restructure that touches arg
  liveness would have been in the local mutation neighborhood; 16k iters found
  none).

The **only two genuinely remaining avenues** (carry-through to the next session)
are both non-local / policy-grade, identical to cpu_side_move_dir_4's resume set:
1. **`PERM_*`-directed permuter** targeting a specific allocno's ref profile (the
   randomization-only permuter cannot directly synthesize a non-CSE'd chain).
2. **Project-wide rodata reorder** (saEft00Add precedent class) — a user policy
   decision, not a single-function lever.

NOT viable, fully exhausted across sessions 5-10:
- Per-function cc1psx opt-in (session 7b: 23 diffs vs target; cc1psx hits the same
  allocation gap).
- C-source dependencies extending insn 107's RTL chain (session 7's PRIODBG).
- Volatile-bump lever on marionation_Exec (regresses; tested above).
- 19 manual levers on cpu_side_move_dir_4 (sessions 4-9).
- **Directed permuter from clean target on marionation_Exec** (this session).

**Artifacts preserved:**
- `permuter/marionation/` — full workspace (target.s/o, base.c, compile.sh, ~270
  saved candidates 1110..1690+).
- Canonical cc1 SHA1 `045c9543d39ab8109583b92137c7adde084f7a25` (2026-05-18) —
  **UNTOUCHED**.
- Instrumented `tmp/gccdbg/cc1` SHA1 `a17fc6bbcb21dbfb1d4bc9b69b9647fd17280a3b`
  (2026-05-29) — preserved with all 3 hooks (ALLOCDBG + SCHEDDBG + PRIODBG).

## Related
- [[register-asm-pins]] — pin reliability; this rule is the pure-C alternative
  to its "regfix the register name" fallback. **Read both when fighting a pin.**
- [[inline-move-aliasing]] — the asm escape valve (tier-3); prefer the pure-C
  levers here first.
- [[dead-vars-local-array]] — Levers C and D worked end-to-end there.
- [[minimize-regfix]] — every pin and rule is debt; these levers retire both.
- [[difficult-is-not-impossible]] — the matching C exists; coupling, not impossibility.
- [[cc1psx-calibration-only]] — the cross-check this section ran against (cc1psx
  matches target's scheduling, not target's allocation — confirms C-structure gap).

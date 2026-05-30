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

### Status (session 4)
NOT pure-C-matchable in the explored search space. Floor = sandbox-distance 20
(5 rules: 4 reg substs $3→$4 + 1 reorder). Cannot derive a further untried lever
from `tools/gcc-2.7.2/global.c` (priority formula known, dispositions read,
volatile lever is the only allocation-flipper), the `.greg` dump (priority list
end is empirically observed), siblings (saEft01Init, marionation_Exec, the
matched tslPolyF4Init — none share a unique structural feature that maps onto
this function's shape and flips allocation), or the permuter (5000+ iters
exhausted in session 3). State preserved in `permuter/csmd4/` and
`tmp/csmd4_min.c`. Genuine close needs a project-level lever: e.g. global rodata
reorder to displace `D_800A125C` so its `la` becomes part of a different RTL
pattern; OR cc1 instrumentation (BB2_ALLOC_DEBUG already in `global.c:575` but
the deployed cc1 binary predates it — rebuilding it is project-scope work).

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

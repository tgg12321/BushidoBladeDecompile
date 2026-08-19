# Stuck-monster forensics — the libcd twins (read-only audit, 2026-08-18)

Scope: the two functions the metrics flag as ~2,200 sandbox probes for zero movement.
Both were RENAMED on 2026-07-09 and both are the SAME Sony library function pair.

| metrics name | real identity | address | queue status today |
|---|---|---|---|
| `cpu_side_move_dir_4` | **`CD_sync`** (PsyQ libcd `bios.c` v1.86) | 0x80080DB0 | **PARKED** — "OWNER-ACCEPTED INCOMPLETE (ruling b, 2026-07-20)", 5 regfix rules retained |
| `marionation_Exec` | **`CD_ready`** (same object) | 0x80081030 | **ACTIVE**, distance 56, 42 regfix rules, `hand_coded_tier: LOW` |

Both live in `src/system.c` (CD_sync at line 376, CD_ready at line 487).
Ledgers: `memory/grind/CD_sync/` (103 sessions), `memory/grind/CD_ready/` (57 sessions),
plus `memory/wip/CD_sync/`, `memory/wip/CD_ready/`.

---

## 0. THE HEADLINE FINDING — the ledgers and the answer are in different files

**The grind ledgers are stale by a whole campaign, and the Grinder reads only the ledgers.**

`memory/grind/CD_ready/state.json` ends at **session 57, floor 4, modality `structural`**.
`memory/grind/CD_sync/state.json` ends at **session 103, floor 2, modality `permuter`**,
frontier = "every sanctioned pure-C axis is measured dead → file OWNER-ESCALATION".

Neither ledger records what happened next. On **2026-07-09 to 2026-07-13** a separate
"Closer" campaign (`docs/closer/`, `memory/closer/`) did the following, none of which is
in either ledger or either `frontier` entry:

1. **Proved the identity** by string xref in `disc/SLUS_006.63` (`memory/closer/libcd-identity.md`):
   the `$Id: bios.c,v 1.86 1997/03/28 makoto Exp $` rcsid at 0x80016274, `"CD_sync"` at
   0x80016240, `"CD_ready"` at 0x80016248. The Kengo-derived names were symbol-collision junk.
2. **Proved BB2 links Sony's object VERBATIM** (`memory/closer/phase2-closeout.md`, W3):
   PsyQ 4.0 `LIBCD.LIB` BIOS module = bios.c v1.86, all 1,526 words masked-identical to
   BB2 0x80080828–0x80082000; `CD_sync` 160/160, `CD_ready` 179/179. *Lightweight never
   compiled this code — Sony's build is the target.*
3. **Moved CD_ready off its 57-session floor**: floor 4 → **masked 2 at 179/179 insn parity**
   (candidate `marionation_p6_volatile1496.c`, then `marionation_vAT1_notailwrap.c` —
   masked 2, one-pair residual, two fewer FAKE devices). Both candidates were **deleted**
   from the tree by `043e4b80` and now exist only in git history.
4. **Proved the faithful Sony source reproduces the entire contested window** — see §2.

Consequence: if the Grinder reaches `CD_ready` today it resumes from an s57 frame that
predates the identity discovery, the masked-2 candidate, and the standalone proof. Any
future unpark of `CD_sync` has the same problem. **Before any further grinding on either
function, the Closer findings must be merged into `memory/grind/<func>/`.** This alone
plausibly explains a large share of the wasted probe budget: the two knowledge bases
never met.

---

## 1. CD_sync (ex-`cpu_side_move_dir_4`) — PARKED, and parked for a reason that is now answerable

### Status
**Parked, but explicitly "eligible for re-attempt."** Park reason:
> "cross-symbol idiom refused (no SOTN precedent); 5 rules retained to hold match; not sanctioned"

The refused idiom is still sitting in `src/system.c:394` today:

```c
idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1);
```

That is the "h5" form. **Closer Ruling 1 (`docs/closer/rulings.md`) declares it a fakematch:
the function CANNOT be completed with it**, regardless of what the residual does. So
CD_sync's real blocker is not the 2-instruction residual at all — it is that the chassis
the whole 103-session search was built on is disqualified.

### Residual characterization (floor 2)
From `memory/wip/CD_sync/notes.md`, ledger s103, and `phase2-closeout.md`:

- Residual = the pair `{sll4@54 ↔ addu5@55}` in the `printf` (do_timeout) block —
  target emits `addu $v0,$v0,$s5` **then** `sll $a0`; ours emits them swapped.
- Both instructions are `LAUNCH` (`adjust_priority` → `birthing_insn_p`, i.e. a SET to a
  live reg with `reg_n_sets == 1`), so `rank_for_schedule` falls through priority and
  class to the **LUID tiebreak** — statement order decides.
- Flipping the statement order fixes the ORDER but trades the register SEATS: in
  `local-alloc.c qty_compare`, `pri = floor_log2(refs)·refs·size/life` gives the t0-web
  qty and the arg5-value qty an **exact tie** (5.33 vs 5.33 in the wip measurement,
  1.33 vs 1.33 in the Closer w1 measurement), broken by **qty birth order** in t0's favour.
  Target needs the arg5 qty allocated first (→ `$v1`), t0 second (→ `$a0`).
- This is a **coupled fixed point**: statement order controls both the sched2 LUID tie and
  the qty live-ranges. Every hand decomposition fixes one half and breaks the other.
- `CD_ready` region-1 is the **identical** knot (confirmed by both notes files and by
  `phase2-closeout.md`: "the twins' residuals are THE SAME knot").

### What is closed (do not re-derive)
`memory/wip/CD_sync/notes.md` "Ruled out" section plus the g/h/k/f sweeps: G1–G4 hoists,
inline-arg forms, D1–D8 dual temp-free chains, a 10-spelling tie battery, a 192-combo
grid (floor 4), a2v hoists, the jalr-temp copy-pref (cse folds it), arg5-first k-frames,
reu-arg5, the h-sweep interleavings, dead stores (flow recounts). Closer added w3/w4/x1–x5/
y1–y5 and the honest respells P2/P3/P4 (38/28/28 — killed).

---

## 2. CD_ready (ex-`marionation_Exec`) — ACTIVE, and the destination has already been proven to exist

### Status
Active, queue distance 56 with 42 regfix rules — but that is the **committed** chassis,
which is far off (the rules include two full s-register permutation cycles,
`regfix.txt:1092-1097`, plus a prologue frame-offset rewrite). The banked best is
masked 2 at 179/179 and lives only in git history.

### Residual characterization (floor 2, post-Closer)
Two residuals existed at floor 4; **Closer closed one of them honestly and the other is
CD_sync's pair.**

- **Residual 2 (region-3 dbr steal) — CLOSED.** The `beqz a2; nop; sb; move a1,s4` window:
  reorg's `fill_simple_delay_slots` was stealing `move a1,s4` into check2's beqz slot.
  57 sessions failed on this; the Closer close was to make `Intr.c` (D_800A1496)
  **volatile — which is the ORIGINAL Sony semantics** (`Intr` is IRQ-mutated via
  `InterruptCallback(2, …)`). The volatile blocks the reorg fill and the nop is produced
  honestly. Banked as P6, then improved: the SOTN v1.77 source shows each check arm is
  self-contained (`Intr.c = 0; _memcpy(result, buf, 8); return c;` with the `result==0`
  guard INSIDE the inlined `_memcpy`) — there is no cross-arm `goto done` in the original.
  Spelling the arms that way flips **both** beqz destinations to target's 0x28c
  (cross-jump forward-redirects arm1's guard into arm2's identical `j; move v0,a2` tail).
  That also made the tail `do-while(0)` FAKE removable.
- **Residual 1 = CD_sync's `{sll/addu}` pair.** Same mechanism, same qty tie.

### The decisive experiment nobody has followed up
`memory/closer/phase2-closeout.md`, "THE DECISIVE FINDING":

> `tmp/closer/cdready_v1.c` (literal SOTN v1.77 shape: `static volatile CD_intr Intr`
> **member** reads, `while(1)`, static-inline `set_alarm`/`get_alarm`/`callback`/`_memcpy`),
> compiled STANDALONE with exact project flags — **the entire contested window matches:
> `lbu a0 / lbu v0 / lui+lw a1 / sll v0 / addu v0,v0,s5 / sll a0,a0 / lw v1,0(v0) / lbu com /
> addu a0,a0,s5 / sll / sw v1` — seats AND pair order, from plain faithful C with ZERO FAKE
> devices.**

The `while(1)` loop's `loop_depth` ref-weighting is what 155 sessions of `do-while(0)` FAKE
wraps were approximating by hand. **Sole defect of the standalone**: `loop.c move_movables`
hoists the one-use `&CD_comstr` movable into `$s8`, costing 2 prologue insns and an
`addu v0,v0,s8` where target uses the `$at` macro form.

The in-context port of that source into `src/system.c` scored **masked 70–75** — the
port was left unfinished, and the campaign was retired (`043e4b80`) with the port never
completed. **That, not the 2-instruction pair, is the actual unfinished work.**

---

## 3. Shared-assumption audit — what EVERY prior session assumed

| # | Assumption held by all 160 grind sessions | Untried inversion | Status of the inversion |
|---|---|---|---|
| A1 | The function is Lightweight game code, so its C is ours to invent | It is Sony's `bios.c` v1.86, linked **verbatim** as an object; the source is essentially known from `sotn-decomp psxsdk/libcd/bios.c` (v1.77, and v1.77→v1.86 changed only li-spelling, not C) | **Proven, never ported in-context** |
| A2 | `D_800A1494/1495/1496` are three independent `u8` splat scalars, reached via pointer locals (`idx_1494`, `idx_1495`) | They are **ONE 3-byte volatile struct** `static volatile CD_intr Intr {u8 sync; u8 ready; u8 c;}`. Declare the aggregate; drop the pointer locals entirely | **Never done in-tree** — see §4 lever 1 |
| A3 | The outer loop must be a `goto`-loop, because a real loop lets LICM promote an invariant to `$s8` | The original is `while(1)`; the `loop_depth` weighting it gives is what the FAKE `do-while(0)` wraps were emulating. The `$s8` promotion is a *separate*, separately-attackable defect | Standalone proves `while(1)` is correct |
| A4 | `volatile` on the Intr bytes would be a coercion cheat | It is **original semantics** (IRQ-mutated) and covered twice over: `legitimate-volatile-interrupt-touched` and closer **Ruling 4** (ground-truth-codegen volatile class, GRANTED for census-proven Sony library state) | Used once (P6) then the campaign ended |
| A5 | The residual is a scheduler/RA problem in the printf block | Under the 2026-08-18 `split-scalars-hide-aggregate` rule it is likely an **alias-analysis** problem manufactured by the pointer-local store `*idx_1494 = 2` | **Untested** — see §4 lever 1 |
| A6 | Our fork's `$fp` allocatability is a genuine cc1psx-vs-decompals fork divergence | **The target binary refutes the global form of this claim** — see §5 | **Refuted as stated; re-framed** |
| A7 | All measurements are current | Every Closer and grind number predates **`-mel`** (2026-08-04), which changed spill-slot layout, bitfield direction and lwl/lwr offsets tree-wide | Nothing re-measured post-`-mel` |
| A8 | The rule set at probe time is the rule set now | Rules landed after the last probe: `split-scalars-hide-aggregate` (2026-08-18), `header-type-correction-from-use-sites` (2026-07-13), `phantom-slot-frame-lever` (2026-08-04), `duplicated-statement-into-arms` scope clarification (2026-08-06), the text1a **-G8 per-file flag** precedent (2026-08-05) | All untried here |

---

## 4. Ranked untried inversions

### LEVER 1 — Declare the `CD_intr` aggregate (kills the pointer-local stores) — **CHEAP**
Applies to **both** functions. Current source uses pointer locals into adjacent splat
scalars with unmotivated address arithmetic — the exact signature in
`.claude/rules/split-scalars-hide-aggregate.md` (landed 2026-08-18, three days ago,
from the MoveImage `s6` finding):

- Tell 1: an address constant nobody can motivate → `((s32)&D_800A1494 - (s32)D_800A125C) + 1` (src/system.c:394).
- Tell 2: a size argument equal to the whole run → the `_memcpy(result, buf, **8**)` result buffers.
- Mechanism: `*idx_1494 = 2` is a MEM with a **varying** address and no `MEM_IN_STRUCT_P`.
  `canon_rtx` cannot resolve it, so it takes a **false true-dependence against every
  fixed-address global read in the block** — and the reads in the contested block are
  exactly `D_800A11D5` (CD_com), `tbl_125c` (CD_intstr) and `D_800A147C`. Those loads sink
  below the store. *That is what a "scheduler residual" in this block looks like.*
- The rule is explicit that adopting a struct on the READ side alone changes nothing —
  it is the STORE side that does the damage. Both functions store to `Intr`
  (`Intr.sync = 2` in CD_sync, `Intr.c = 0` / `Intr.ready = 2` in CD_ready), so both qualify.
- Bonus: this is also the **owner-sanctioned replacement for the refused cross-symbol
  idiom**. Ruling 1 said verbatim: *"if the symbols are genuinely one object: merge them
  into a single struct declaration, after which base+offset indexing is honest."* Phase 1
  W4 then answered YES for 1494/95/96 (and NO for 125C, which stays separate). **The
  sanctioned honest form was identified 2026-07-09 and never implemented.**
- Mechanics: `dlabel` at 0x800A1494 in `asm/data/*.s` + `symbol_addrs.txt` entry +
  `extern volatile CD_intr` declaration (volatile covered by Ruling 4).

### LEVER 2 — Finish the in-context faithful port — **CHEAP-TO-MODERATE**
The standalone already emits the entire contested window correctly with zero FAKE devices.
The port scored 70–75 because of **declaration/spelling shape**, and Phase 2 already named
the deltas to hunt: (a) Alarm accessed per-member (`D_800F19B8 = …; D_800F19BC = 0;
D_800F19C0 = name;`) rather than through struct members, because target's stores are
per-symbol `la`s; (b) `Intr` as the volatile struct extern (gives target's `addiu +1/+2`
derivation); (c) `while(1)`; (d) the comstr-hoist kill. Artifacts: `tmp/marion_faith2/`,
`tmp/closer/apply_faithful.py`, `tmp/closer/apply_structdecl.py` (may have been cleaned —
regenerate from `phase2-closeout.md` if so). Levers 1 and 2 are the same edit done properly:
do them together.

### LEVER 3 — Re-measure the banked best post-`-mel` — **CHEAP**
Every number in both ledgers and in `phase2-closeout.md` was taken **before** `-mel`
(2026-08-04). `-mel` moved every 4-byte reload spill slot and reversed bitfield allocation.
The CD_ready residual involves a prologue frame-offset rewrite (`regfix.txt:1111-1117`
rewrites seven `sw $sN, off($sp)` offsets), i.e. exactly the frame-layout class `-mel`
corrected. Re-scoring `marionation_vAT1_notailwrap.c` (recover via
`git show 043e4b80^:memory/closer/candidates/marionation_vAT1_notailwrap.c`) and the
h5/g3 CD_sync forms under today's toolchain is one sandbox run each and may have moved
for free.

### LEVER 4 — The `$fp` / `n_non_fixed_regs` question, correctly framed — **SPECULATIVE, one build to falsify**
See §5. Not a fork divergence. Possibly a per-object build-config difference, and the
experiment is byte-safe for `src/system.c`.

### LEVER 5 — Re-seed the grind ledgers from the Closer campaign — **CHEAP, do first**
Not a codegen lever but the highest expected value per token: merge
`memory/closer/libcd-identity.md`, `phase2-closeout.md`, Rulings 1/4, and the two recovered
candidates into `memory/grind/CD_ready/` and `memory/grind/CD_sync/`, and correct the s45
ledger entry that says "no external-repo analog exists" (`sotn-decomp` contains this exact
function). Otherwise the next session repeats sessions 1-57.

### LONG SHOT — anything in the pure-C statement-order space
`memory/wip/CD_ready/notes.md` records an **exhaustive** 140-ordering sweep of every
dependency-valid `do_timeout` interleaving: floor 4, zero hits, insns pinned at 178.
The pair provably cannot fall to statement order on the goto chassis. Do not re-run it.

---

## 5. The `$fp` fork-divergence claim does not survive contact with the binary

Phase 2 concluded that the standalone's sole defect (the `&CD_comstr` movable hoisted into
`$s8`) is caused by a fork divergence: *"SN keeps `$fp` non-allocatable and its
`n_non_fixed_regs` is 1 smaller → threshold 1 smaller → this exact movable fails the move
test; our decompals fork frees `$fp`."* `loop.c:532` confirms the mechanism:
`threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`.

**The premise is wrong as a global statement about the compilers.** I scanned all 1,434
`asm/funcs/*.s`:

- **130 target functions allocate `$fp` as an ordinary callee-saved GPR** — e.g.
  `addiu $fp, $zero, 0x1` (SpuSetReverbModeParam), `addu $fp, $v0, $zero` + `slt $v0, $fp, $v0`
  (func_80017A44), `mflo $fp` (func_8002F2D0), `sll $fp, $v0, 3` (func_8002CA8C).
  100 of those 130 also use `$s7`, i.e. `$fp` is the 9th callee-saved register the
  allocator reaches for. BB2's game code was compiled by the same SN `ccpsx`. **cc1psx
  demonstrably allocates `$fp`.**

But the sub-claim survives in a **different and more interesting form**:

- Across the **entire libcd BIOS.OBJ region** (0x80080828–0x80082000, 12 functions,
  1,526 words) there is **not one `$fp` reference**.
- Across the whole `src/system.c` address range (0x80080000–0x80083000, **53 functions**)
  there is **not one `$fp` reference** either.
- And `CD_ready` **saturates `$s0` through `$s7`** and still never touches `$fp` — the exact
  signature of an allocator whose order stops at `$s7`. (`CD_sync` reaches `$s6`, `CD_cw` `$s5`.)

So the honest reading is: **Sony's library build and Lightweight's game build had different
register configurations**, not that our fork differs from cc1psx. That reframing matters
three ways:

1. It **disqualifies the Ruling-2 / `fork-divergence-inline-asm` route** for these two
   functions, which the owner already declined on 2026-07-13 for scheduling/RA divergence
   without a crash. There is no cc1 crash here. Do not go down that path.
2. It makes the correct disposition a **build-configuration question**, which has a live
   precedent: `-mel` (2026-08-04, owner-elected configuration-fidelity correction) and the
   per-file `GP_FILES` `-G8` adoption for text1a (2026-08-05). `-ffixed-30` exists in this
   compiler (`toplev.c:3551` → `fix_register`, `regclass.c:475`), and `n_non_fixed_regs` is
   computed in `init_reg_sets_1` via `init_regs`, i.e. **after** option decoding — so the
   flag would lower the `move_movables` threshold exactly as Phase 2 described.
   `CONDITIONAL_REGISTER_USAGE` in `config/mips/mips.h:524` does not touch `$30`, so nothing
   would override it.
3. The experiment is **byte-safe to try**: since no function in `src/system.c`'s entire
   address range uses `$fp`, a file-scoped `-ffixed-30` cannot regress a single already-
   matching function in that file. The oracle is a complete, one-build verdict.

Caveats to state plainly: absence of `$fp` in a 53-function window is weak evidence on its
own (only 2 of the 53 even reach `$s7`), and this would be a per-file flag list entry that
needs owner election in the `-mel`/`-G8` mould. It is a speculative lever with a cheap,
unambiguous falsification — not a conclusion.

---

## 6. What I did NOT verify (read-only session)

No builds, no sandbox, no engine CLI were run. Everything above is static: git history,
ledgers, `docs/closer/`, `regfix.txt`, `src/system.c`, `asm/funcs/*.s`, and the cc1 sources
under `tools/gcc-2.7.2/`. The `-ffixed-30` mechanism is read from the compiler source, not
executed. All scoring claims are quoted from the banked ledgers, which were measured
pre-`-mel`.

## ADDENDUM — -ffixed-30 experiment RESULT (2026-08-18 boundary, owner-authorized)

MEASURED: a file-scoped `-ffixed-30` for src/system.c built to a GREEN oracle
(SHA1 == 62efab4f..., full make, then reverted — the flag is NOT currently
adopted). This proves the forensics §5 byte-safety claim: no function in
system.c's range uses $fp, so fixing it changes nothing in current code.
CONSEQUENCE for this function: the flag lowers loop.c move_movables'
threshold ((loop_has_call?1:2)*(1+n_non_fixed_regs)) exactly as the Sony
library build's register config would — when the faithful in-context port is
applied and the &CD_comstr hoist is the sole residual, request adoption via
an owner election recorded a la -mel/-G8 (Makefile: add `system` to a
FFIXED30_FILES list wired into cc_flags_for; mirror engine/buildconfig.py).
Do NOT hand-add the flag mid-session; it is a build-config election.

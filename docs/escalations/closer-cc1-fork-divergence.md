# ESCALATION — owner ruling needed: GCC 2.7.2 decompals-fork divergence blocking `_spu_FiDMA`

**Origin:** Closer Phase 3 wrap-up (2026-07-13). Banked candidate:
`memory/closer/candidates/spu_writebyio_splice.c`. Function:
`DispUpdateStatusMessage` (queue: active, ASM-SUSPECT, dist 204) — the splice
region `0x800889D4..0x80088D0C` = Sony `spu.c` `_spu_FwriteByIO` +
`_spu_FiDMA` + `_spu_Fr_`.

## The exact question

Our build compiler is the open-source `decompals/mips-gcc-2.7.2` fork. On the
Sony-source wait-loop class

```c
while (<volatile MMIO read expr>) { ...; if (++i > 0xF00) break; }
```

**our fork SEGFAULTS (cc1 SIGSEGV)** — for any `while`/`for` spelling with a
volatile read in the condition and an early exit in the body. The original
PsyQ `cc1psx` compiles the identical source without crashing and emits the
**exact target bytes** (including the dead `addu $3,$3,-1` that reorg's
increment-compensation clause produces).

`_spu_FiDMA` therefore cannot be closed from the faithful Sony source in our
toolchain. The closest crash-avoiding spelling (if + do-while + `goto`) lands
**2 words short**: it loses the loop notes, so `reorg.c mostly_true_jump`
returns likely=0 and the increment-compensation `addiu -1` never fires.
`_spu_FwriteByIO` is already **bit-exact 132/132** in the same region; only the
`_spu_FiDMA` sub-region is blocked.

**Should this be accepted as a fork-divergence class ("our fork cannot emit
what the original compiler did") — a Ruling-2-style disposition — or must the
function reach byte-exact through some not-yet-found pure-C spelling in our
fork?**

## Mechanism evidence (independently reproducible)

- **Crash exhibit:** `docs/escalations/spu3-fork-crash-evidence/` — 15 measured variants; the faithful
  `while (volatile & 0x30) { if (++i>0xF00) break; }` SIGSEGVs cc1.
- **cc1psx counter-exhibit:** `docs/escalations/spu3-fork-crash-evidence/00_cc1psx_output.s` — the ORIGINAL PsyQ
  compiler compiles that exact source and emits the target bytes verbatim,
  incl. the dead `addu $3,$3,-1`.
- **Why the workaround is 2 short:** goto/if-do-while spellings avoid the
  crash but drop loop notes → `mostly_true_jump` likely=0 → reorg.c:3615
  increment-compensation (the only producer of the `addiu -1`) never fires
  (DBRDBG-traced, `BB2_DBR_DEBUG=1`).
- **Reachability:** `_spu_FiDMA` is installed by address only
  (`g_snd_irq_data = 0x80088BA0`, the SPU DMA-complete IRQ callback);
  `_spu_Fr_` is unreferenced Sony dead code (S_SCA precedent).

This is the same *kind* of finding as the marionation fp-allocatability
fork-divergence noted in Phase 2 — "our fork cannot emit; the original
compiler can and did."

## The two named options for the owner

1. **Grant Ruling-2 (fork-divergence class).** Accept `_spu_FiDMA` as a
   documented fork-divergence: our decompals fork provably cannot compile the
   faithful Sony source (it crashes), the original `cc1psx` provably can and
   produces the target bytes. Disposition would follow the canonical-asm /
   authorized-exception path for the residual 2 words. **Constraint check:**
   this is NOT the `input-side compiler fix` that `no-compiler-divergence`
   forbids — it is a completion-class ruling, not a patch to the toolchain.

2. **Reject — require a pure-C close in our fork.** Keep the function ACTIVE
   in the queue for the Grinder; the standing search task becomes "find a
   loop-note-preserving spelling whose exit test does NOT target loop-end
   (blocking the `duplicate_loop_exit_test` that fires on the empty break
   arm) while keeping body order `[i++; limit; cond]`." A major search with
   no guarantee of existence.

## Related (lower-confidence, same family — NOT yet proven fork-class)

`func_80089F3C` (SpuSetReverbModeParam, queue: active, verdict C, dist 23) —
banked at `memory/closer/candidates/spusetreverbmodeparam_struct.c`. 20 of 23
masked words are self-heal struct-reloc addends; the **3 real words** are a
reload spill-slot ordering: our fork's reload runs TWO spill passes
("Spilling reg 8/64" ×2) leaking two transient slots before `sp58`'s; Sony's
compile leaked one → `sp58` lands at `sp+0x5C` vs target `sp+0x58`. This
*could* be another fork/reload divergence, but it is NOT proven fork-class —
a not-yet-found source shape may leak one slot. Next step is
instrumentation-side (patch cc1 `BB2_RELOAD_DEBUG` to identify the pass-1
stacked pseudo set), not an owner policy call. Listed here so the owner sees
the pattern; **no ruling requested on this one yet** — it stays a Grinder
reference item.

---

## Owner ruling — 2026-07-13 (Ruling-2 GRANTED, narrow form)

**Ruling**: Ruling-2 GRANTED as a narrow authorized-inline-asm sub-class,
codified in `.claude/rules/fork-divergence-inline-asm.md`. Not
whole-function canonical.

### What this ruling sanctions

A new sibling carve-out to `jtbl-rodata-split-infrastructure` and
`gte-wrapper-misroute-park`, defined by: **our decompals fork's cc1
SIGSEGVs on the exact C source cc1psx compiled to the target bytes,
and every crash-avoiding rewrite deterministically emits a
structurally-different shape.**

Every invocation runs a fresh **four-gate evidence check**:
1. Reproducible crash file (reviewer runs → confirms SIGSEGV).
2. cc1psx counter-exhibit compiling the same file cleanly.
3. cc1psx output byte-matches target in the affected region.
4. Probe grid (≥5 alternatives) each individually documented: crash /
   structurally-different-shape / target-match. No grid ≠ no gate.

Any gate missing → keeps grinding as an ordinary Grinder search item.

### What this ruling does NOT sanction

- Whole-function canonical-asm on a small residual (the disposition is
  region-scoped by construction).
- Register-allocation, scheduling, reload-count, or prologue-order
  divergences without a crash — those stay as C search work.
- Hardcoded-`$N` asm templates (still forbidden;
  [[inline-asm-injection]] unchanged).
- Toolchain patches (`no-compiler-divergence` unchanged; the fork bug
  is documented, not fixed).
- Retroactive unparking of any existing function without fresh four-gate
  evidence.

### Sharpened class description

Previously framed as "volatile-cond while". Independent probe grid
(2026-07-13) shows the actual crash class is:

> `while (LOOP_COND) { if (++LOCAL_COUNTER > CONST) break; }` at
> `-O1`/`-O2` in `tools/gcc-2.7.2/build/cc1`.

Volatile-ness of `LOOP_COND` is incidental (probe4/6/8 crash on plain
`u16 g`). The `++LOCAL_COUNTER > CONST` break inside a while's body is
load-bearing (probe7 without the pre-increment compiles fine). Grid
preserved under `docs/escalations/spu3-fork-crash-evidence/`.

### Disposition for `_spu_FiDMA` (first confirmed case)

- The four gates cleared as documented above.
- Authorized inline `__asm__` island covering only the pre-increment,
  delay-slot `+1` fill, and reorg-compensation `-1` (3 instructions).
- `%N` placeholders bound to `register T x asm("$N")` C-level pins;
  no hardcoded `$N` in the template.
- Outer wait-region control-flow stays pure C via the compiling
  `if (cond) do { ... } while (cond);` shape (probe3).
- `_spu_FwriteByIO` and `_spu_Fr_` remain pure-C matches around the
  asm island.
- Add to `inline_asm_canonical.txt` under a new "fork-divergence
  sub-class" header, citing this escalation and the rule doc.
- `retire` drops rules to zero; function reaches
  COMPLETED-INLINE-ASM-CANONICAL.

### Precedent scope for `func_80089F3C` (SpuSetReverbModeParam)

**Not pre-approved.** No cc1 crash on any tested source shape for that
function — the divergence is a reload-pass count difference. Fails
gate 1 today. Stays a Grinder search item until instrumentation or
source-shape work produces the four-gate pack, at which point the
ruling applies. Ruling-2 is NOT a general "our fork behaves
differently, ergo asm" wand.

### Follow-up steps

The paperwork (rule doc + this ruling record + codegen-index entry +
evidence dir) landed 2026-07-13. The actual `src/code6cac.c` edit +
`inline_asm_canonical.txt` append + `verify-oracle --rebuild` +
`retire` + layer-2 `cheat-reviewer` on the src edit are pending — the
Grinder was live on `func_8001C624` in the same `.c` file during this
session, so the src edit was deferred to avoid the conflict. Owner
decision needed: stop Grinder now to land the close, or defer to a
later session when Grinder is naturally idle.

---

## Attempted close 2026-07-13 — asm island lands wait loop, outer scheduling residual defers close

Attempted the Ruling-2 close in `src/main.c` (DispUpdateStatusMessage
sits in main.c, not code6cac.c — Grinder had been stopped as a precaution
but the file overlap was unnecessary). Wrote the candidate C body for
all 3 splice functions, removed the asmfix bridge, iterated on the asm
island form.

### What worked

The asm island for the wait loop closes byte-exact against target:
- Pre-`if` volatile RMW (0x80088BC8..0x80088BD4): pure-C from candidate,
  6 insns matching target
- 2nd volatile read + andi + beqz (0x80088BD8..0x80088BE4): moved
  INTO the asm block (fully-scoped island) to prevent GCC eliminating
  the `if (t)` branch when `t` was register-asm-pinned
- Wait loop proper (0x80088BE8..0x80088C10): 11 insns matching target
  byte-for-byte — including the delay-slot `+1` fill and reorg
  compensation `-1` that non-crashing pure-C forms provably cannot reach

### What did not close

Post-loop outer structure (0x80088C14..0x80088C5C) — 4 residual insns
in the `if (D_800A2D14 != 0) ... else bios_DeliverEvent(...);` region.
Target does two aggressive scheduler-level optimizations our fork
declined to reproduce:
1. Pre-loads `lui $a0, 0xF0000000` into the beqz's delay slot as a
   speculative constant hoist for the else branch. Our fork emits nop.
2. Fills `bios_DeliverEvent`'s jal delay slot with `addiu $a1, $zero, 0x20`.
   Our fork emits nop and places the load AFTER the jal.

Neither is a fork bug (cc1 compiles fine, just different scheduling).
Both are ordinary register-allocation / scheduling differences — exactly
the class Ruling-2 does NOT cover (per this rule's "does NOT sanction
fork-divergences without a crash"). Tried: rewriting `if/else` as
`if/return; fallthrough` shape (no change). Did not try the
constant-holder form or wider probe grid — those are ordinary Grinder
search work and belong there.

### Disposition

Reverted src/main.c and asmfix.txt to HEAD; oracle verified green
(SHA1 == 62efab4f...). Paperwork commits (rule doc, escalation, evidence
dir, index entry) remain landed and correct. The specific `_spu_FiDMA`
close is deferred until either:
- Owner widens Ruling-2 scope to include region-scoped inline asm for
  scheduling-class divergences (would need a fresh evidence gate)
- Or Grinder finds a C form for the outer scheduling residual
  (ordinary search work, no policy change needed)

DispUpdateStatusMessage remains `active` in the queue with the asmfix
bridge; when the Grinder reaches it, the fork-divergence rule provides
the wait-loop lever and the outer scheduling is what needs solving.

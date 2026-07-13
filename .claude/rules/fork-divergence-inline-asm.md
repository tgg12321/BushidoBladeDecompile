---
name: fork-divergence-inline-asm
paths: [".claude/rules/fork-divergence-inline-asm.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "OWNER RULING 2026-07-13 (Ruling-2) — narrow authorized-inline-asm sub-class for regions where our decompals GCC 2.7.2 fork provably CANNOT emit the target's bytes from the faithful C source that cc1psx did emit them from (cc1 SIGSEGVs on the intended source). Four-item evidence gate; disposition is region-scoped inline asm, NOT whole-function canonical. First confirmed case: _spu_FiDMA (DispUpdateStatusMessage splice)."
metadata:
  type: rule
---

# Fork-divergence inline-asm (Ruling-2, 2026-07-13)

**Owner ruling 2026-07-13** (first confirmed case: `_spu_FiDMA` in the
`DispUpdateStatusMessage` splice, `src/code6cac.c`; escalation:
`docs/escalations/closer-cc1-fork-divergence.md`). A narrow new
authorized-inline-asm sub-class sanctioning small `__asm__` islands in
otherwise pure-C functions when — and ONLY when — our decompals GCC
2.7.2 fork provably cannot emit the target's bytes from any legal C
source, because the C source the original PsyQ `cc1psx` did emit them
from crashes our cc1.

**This is not a whole-function canonical-asm class.** The surrounding
function stays pure C; only the region cc1 provably cannot reach gets
the asm island.

## Distinction from the two existing carve-outs

| Carve-out | Property that makes C impossible |
|---|---|
| **jtbl-rodata-split-infrastructure** | The GCC-emitted jtbl cannot live at the address the function references — a project-wide rodata reorder is required, no per-function C fix exists ([[jtbl-rodata-split-infrastructure]]). |
| **GTE leaf wrappers** | cop2 ops have no C analog — no C construct emits `avsz3`/`avsz4`/`mtc2`/`mfc2` ([[gte-wrapper-misroute-park]]). |
| **fork-divergence-inline-asm** (this rule) | Our fork's cc1 SIGSEGVs on the specific C source that cc1psx compiled to the target bytes. Every crash-avoiding rewrite deterministically emits a different loop structure. |

Both prior carve-outs have the property "no C input to the same compiler
produces target bytes." This rule has the same property, but the "same
compiler" clause requires a fork-divergence proof: our fork is not
cc1psx, and the difference is a bug in ours (a crash), not a design
choice.

## The four-item evidence gate (all mandatory)

Every function invoking this class must have all four on record BEFORE
authorization. The reviewer runs each check themselves; the worker's
summary is a starting point, not a verdict.

**Gate 1 — Reproducible crash.** A minimal C file that crashes
`tools/gcc-2.7.2/build/cc1` at production flags
(`-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w`).
The reviewer runs the file and confirms `Segmentation fault`. The file
lives under `docs/escalations/<slug>-evidence/` (committed, gitignore-safe)
and is preserved for the audit trail. Do NOT rely on `tmp/` paths for
gate evidence — `tmp/` is gitignored and job-cleanable.

**Gate 2 — cc1psx counter-exhibit.** The same C file compiled cleanly by
`tools/cc1psx_wrapper.sh` with equivalent flags. The reviewer runs it
and confirms exit 0. The emitted `.s` is preserved.

**Gate 3 — cc1psx output matches target bytes.** In the affected region,
the cc1psx `.s` matches the target `.s` (from `asm/funcs/<func>.s` or the
splice extent). "Matches" means the instruction sequence at the target
addresses is present in the cc1psx output at the equivalent labels;
exact byte-for-byte within the region.

**Gate 4 — Probe grid ruling out non-crashing alternatives.** A probe
grid of ≥5 C reformulations that (a) express the same runtime behavior,
(b) individually document their compile outcome (CRASH / different
structure / target-match). The grid must demonstrate: every alternative
either crashes cc1 the same way, OR compiles to a structurally different
form (different loop-note anchor, missing delay-slot fill, missing
compensation instructions) that provably cannot match target.
Just "our fork crashes on X" without the grid does NOT clear this gate —
the burden is to show the C-language degree of freedom that would
express the target's shape is the one that crashes.

Missing any gate → the case does not qualify → keeps grinding as an
ordinary Grinder search item.

## Disposition rules (region-scoped, not whole-function)

**The authorized asm island covers ONLY the instructions the probe grid
proved unreachable.** Not the surrounding calls, not the loop's outer
control-flow if a compiling-but-mismatching form also emits it, not
"conveniently the whole wait region." If the probe grid shows form P
compiles and produces N of M residual instructions target-matched,
the asm island is (M − N), no more.

**Placeholders + register-asm pins, never hardcoded `$N`.** The asm
block uses `%0`, `%1` operand syntax bound to `register T x asm("$N")`
C-level variables. This is the [[inline-move-aliasing]] discipline:
GCC sees the operation and tracks its operands; the pins steer register
choice. Hardcoded `$N` in the template ([[inline-asm-injection]]) is
still forbidden here — the disposition doesn't relax it.

**No regfix/asmfix rules retained on the function.** If the asm island
closes the residual, `retire` runs and drops rules; the function becomes
COMPLETED with zero rules. If rules can't be dropped, the disposition
did not actually close the function and Ruling-2 was mis-invoked.

**Completion class.** The function reaches COMPLETED-INLINE-ASM-CANONICAL
with a citation to this rule. Add to `inline_asm_canonical.txt` under a
new "fork-divergence sub-class" section header, with the required 1-2
line rationale citing (a) the crash class, (b) the escalation doc, (c)
the four-gate evidence file paths.

## Not a wand (scope limits)

- **Does NOT sanction fork-divergences without a crash.** If our fork
  compiles the faithful source but emits different bytes (register
  allocation, scheduling, reload-count, prologue-order differences),
  those are NOT this class. They're ordinary Grinder search items. The
  crash is the load-bearing evidence.
- **Does NOT sanction "our fork chooses different registers" as a
  divergence class.** Register-allocation walls are covered by
  [[register-alloc-pure-c]], [[difficult-is-not-impossible]], and stay
  as C search work.
- **Does NOT create a per-idiom sanction template.** Each function
  invocation runs the four gates fresh. The gates aren't precedents you
  can rely on; they're evidence you produce.
- **Does NOT unlock whole-function canonical-asm on a small residual.**
  The disposition is region-scoped by construction. If the probe grid
  says compiling forms produce 90% of the function correctly, only the
  remaining 10% gets the asm island.
- **Does NOT license unilateral toolchain patches.** `no-compiler-divergence`
  is unchanged — the fork bug is documented, not fixed. Fixing our fork
  would break the byte-identical property of every other function.
- **Does NOT unpark existing functions retroactively.** No function
  currently in `authorize`/`parked` gets re-routed under this rule; each
  needs the four-gate evidence produced fresh.

## First confirmed case — `_spu_FiDMA` (2026-07-13)

Sub-region of `DispUpdateStatusMessage` at 0x80088BA0..0x80088C5F.
Sibling `_spu_FwriteByIO` is already bit-exact 132/132; `_spu_FiDMA`'s
wait loop is 3 words short after every non-crashing C form.

**Gate 1** — `docs/escalations/spu3-fork-crash-evidence/00_faithful_sony_source.c` (the faithful Sony
`while ((*(volatile u16*)MMIO) & 0x30) { if (++i > 0xF00) break; }`)
SIGSEGVs cc1. Verified 2026-07-13.

**Gate 2** — same `a.c` compiled by `cc1psx_wrapper.sh`, exit 0. Output
preserved at `docs/escalations/spu3-fork-crash-evidence/00_cc1psx_output.s`.

**Gate 3** — `t_psx.s` loop region matches `asm/funcs/DispUpdateStatusMessage.s`
at `.L80088BEC` through `.L80088C10` (pre-increment + `sltiu` header +
volatile-condition body + `bnez` back-edge + delay-slot `addiu +1` +
reorg-compensation `addiu -1`). Byte-for-byte within the region.

**Gate 4** — 8-probe grid preserved at
`docs/escalations/spu3-fork-crash-evidence/` (00_faithful_sony_source.c +
probe1_for.c through probe8_o1.c; results table in
`PROBE_GRID_RESULTS.md` in the same dir). Crash class isolated to
`while (LOOP_COND) { if (++LOCAL_COUNTER > CONST) break; }` at O≥1;
non-volatile globals crash identically (volatile is incidental).
Compiling alternatives (`while(1) { !c break; ++i>K break; }`,
`if (c) do { ++i>K break; } while (c);`) both drop the loop-note anchor
that `mostly_true_jump` needs to declare the back-edge likely=1, which
in turn is the precondition for the delay-slot `+1` fill and reorg
increment-compensation `-1`. Deterministically 2-3 short depending on
form. Grid preserved for audit trail.

**Disposition** — a 3-instruction asm island covering the pre-increment,
delay-slot increment, and reorg-compensation. Outer control-flow of the
wait region stays pure C via the compiling `if (cond) do { ... } while
(cond);` shape (probe3). The remaining function is pure C.

Documented in `docs/escalations/closer-cc1-fork-divergence.md`.

## Related

- [[jtbl-rodata-split-infrastructure]] — sibling "no-C-form" carve-out
  for a project-wide rodata reorder problem.
- [[gte-wrapper-misroute-park]] — sibling "no-C-form" carve-out for
  cop2 opcodes.
- [[canonical-asm-authorization-recipe]] — the mechanical rules for
  writing `__asm__("glabel …")` blocks + verifying oracle. Applies here
  but with reduced scope (region-island, not whole function).
- [[hand-coded-asm-recognition]] — the OTHER path to inline asm
  (functions the original developer wrote in asm). This rule is
  different: the original developer wrote C; only our fork can't
  compile it.
- [[no-compiler-divergence]] — unchanged; this rule is a completion-class
  ruling, NOT a toolchain patch. Our fork's crash is documented, not
  fixed.
- [[no-new-park-categories]] — the "no new infrastructure carve-outs"
  policy. This rule is a *narrowly-scoped carve-out under the four-gate
  evidence bar*, not a general park category. It creates no auto-park
  mechanism; every invocation runs the gates fresh.
- [[inline-asm-injection]] — the forbidden hardcoded-`$N` template
  family. This rule does not relax it; asm islands under this class use
  `%N` placeholders and `register asm("$N")` pins.
- [[review-discipline-before-commit]] — layer-2 cheat-reviewer is
  mandatory for `auth:` commits invoking this rule.
- [[completion-standard]] — a function closed under this rule is
  COMPLETED-INLINE-ASM-CANONICAL, not COMPLETED-C.

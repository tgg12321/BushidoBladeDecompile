---
name: voice-control-playbook
paths: ["src/code6cac.c"]
description: "single_game_VoiceContorol (141 insns, code6cac.c) — the canonical case study for MINIMIZE-ASM-WHEN-BLOCKED. Pure C alone plateaus at ~18 diffs (cc1psx-confirmed toolchain floor); +3 §6.1 single-instruction barriers reaches 10 diffs; +5 specific non-wildcard regfix rules lands match. Documents the full attempt ladder so future agents don't re-derive the dead ends."
metadata:
  type: recipe
---

# single_game_VoiceContorol — the minimize-asm-when-blocked playbook

This function is the worked example for [[minimize-asm-when-blocked]] — the only function (as of 2026-05-18) that has cleared the §5.8 entry gate. Its arc is the canonical demonstration of: pure-C plateau → cc1psx-confirmed toolchain limit → 3 single-instruction §6.1 barriers + 5 specific non-wildcard regfix rules → byte match.

## Function shape

`single_game_VoiceContorol` lives in `src/code6cac.c`. 141 instructions, 564 bytes. Manages SPU voice list state with packets[2] and outputs to a shared D_80102790_p pointer. Two loops: an inner switch-driven update and a tail finalization loop.

The HEAD state before retirement matched via **32 wildcard `subst ".*"` rules + 4 asmfix `replace_first` rules** — bytes came from rule literals, not C codegen. Audit flagged this as the worst cheat category (force-overwrite).

## Why pure-C alone plateaus at ~18 diffs

Two GCC 2.7.2 optimizations conspire:

**LICM hoists the constant `1`.** Target emits `addiu $v0, $zero, 0x1; sh $v0, 0x4($a2)` inline. GCC's cc1 sees `1` as loop-invariant and hoists it: `li $t4, 1` in the preheader, then `sh $t4, 4($a2)` in a branch delay slot. Pure-C techniques to defeat (volatile, dependency tricks) either fail or regress.

**Strength reduction eliminates `i`.** Target keeps three induction variables: `$a2` (output ptr +2), `$t0` (i, +1), `$a3` (packet ptr +8). Loop compares `slti $v0, $t0, 0x2`. GCC creates 5 IVs and uses `slt $v0, $a2, $end_ptr` instead — it strength-reduces `i < 2` into a pointer comparison.

Combined: the archived LICM-unhoist recipe says explicitly *"LICM + GCC strength reduction (e.g. `&BASE + i*stride` becoming an induction var) — strength reduction is structural, not fixable by regfix."* This function is exactly that case.

## cc1psx calibration confirms it's a toolchain limit

The same source compiled through cc1psx (PsyQ GCC 2.7.2.SN.1 via dosemu) produces a DIFFERENT preamble order than target — not the decompals-vs-SN gap. cc1psx 2.7.2.SN.1 `[AL 1.1, MM 40]` with `-O2 -G0 -mips1` does NOT reach target's preamble; flag variations (`-O3`, `-fno-schedule-insns`, `-fno-schedule-insns2`, removing `-funsigned-char`) don't help either. **The gap is some combination of (a) different cc1psx patch level than the one Sony shipped to Lightweight in 1998, or (b) source structure no agent has found.** It is NOT closeable by switching our toolchain (cc1psx is calibration-only per [[cc1psx-calibration-only]]).

This calibration is what unblocks §5.8: cc1psx ALSO can't match, so pure-C alone isn't the answer.

Reproducing the calibration:

```bash
# Install custom dosemurc to disable KVM (one-time setup):
cat > ~/.dosemu/dosemurc << 'EOF'
$_cpu_vm = "emulated"
$_cpu_vm_dpmi = "emulated"
$_cpuemu = (2)
EOF

# Run cc1psx via existing wrapper:
bash tools/cc1psx_wrapper.sh <isolated_voice_source.c> > tmp/voice_psx.s
# Inspect for cc1psx's emitted asm and compare to target
```

## The landed playbook (commit `d33ea6b`, 2026-05-18)

**Three §6.1 single-instruction `__asm__ volatile` barriers + five specific (non-wildcard) regfix rules.** Total intervention count: 8 (within the [[minimize-asm-when-blocked]] ≤10 budget).

### The three barriers (each addresses one specific cc1 gap)

```c
/* 1. Defeat LICM's hoist of the constant 1. Without this barrier,
 *    cc1 hoists `1` to a callee-save register in the preheader.    */
register s32 voice asm("v0");
__asm__ volatile ("addiu %0, $zero, 1" : "=r"(voice));

/* 2. Force voice_mask right-shift via temp $v0. Without this,
 *    cc1 reuses $t1 in-place, cascading to bits register naming.   */
register s32 vm_shifted asm("v0");
register s32 voice_mask asm("t1");
__asm__ volatile ("srl %0, %1, 16" : "=r"(vm_shifted) : "r"(voice_mask));

/* 3. Force `i = 0` emission at tail-loop preamble start. Without
 *    this, cc1 emits `move $t0, $zero` at the end of preamble
 *    (closes a 2 + 4 cascade = 6 diffs).                            */
register s32 i asm("t0");
__asm__ volatile ("addu %0, $zero, $zero" : "=r"(i));
```

All three are single-instruction. None compute function logic. Each is justified in the commit message with a specific cc1-gap explanation. No multi-instruction asm. No multi-line `\n\t...` patterns.

### The five specific regfix rules

```
single_game_VoiceContorol: reorder 7,12,5,8,9,10,11,6 @ 5-12          # preamble scheduling
single_game_VoiceContorol: reorder 27,26 @ 26-27                       # inner addiu/lhu swap
single_game_VoiceContorol: subst "lbu\s+\$3,2\(\$7\)" "lbu\t$2,2($7)" @ 41   # b2 in $v0
single_game_VoiceContorol: subst "lbu\s+\$2,3\(\$7\)" "lbu\t$3,3($7)" @ 42   # b3 in $v1
single_game_VoiceContorol: subst "sll\s+\$3,\$3,8"   "sll\t$2,$2,8"   @ 43   # sll on $v0
```

Each rule closes ONE specific diff. NONE are wildcard `subst ".*"` (the cheat pattern). Each is auditor-acceptable.

### Required pin chain

```c
register s32 arg0_pinned asm("s0") = arg0;
register s16 *output    asm("a2");
register VoicePacket *packets asm("a3");
register s32 i          asm("t0");
register s32 voice_mask asm("t1");
register s32 bits       asm("v1");
register s32 voice      asm("v0");
register s32 vm_shifted asm("v0");
```

`bits = 0;` must be assigned explicitly on every path (including `else` branches and the `voice2 >= 8` path). Without explicit assignment on every path, cc1 thinks the init value flows through and can't reuse `$v1`. With explicit init, cc1 sees `bits`'s live range is split and puts it in `$v1` (matching target).

The asmfix sibling fix: `func_8001EFA0`'s branch-inversion rule referenced `.L349/.L345` (HEAD's compile). The new C body shifts cc1's file-wide label numbering, so the equivalent labels are `.L354/.L350` in the new build.

## What doesn't work — calibrated failure list

Future agents should not re-try these. Each was attempted with the documented result; saving the time of re-derivation.

### Source-structure variants (all regressions)

| Attempt | Result |
|---|---|
| `voice2` declaration reorder | 111 diffs (cc1 optimizes away the cast) |
| `u16 reload` intermediate variable | 117 diffs |
| `reload` as asm input (`"r"(reload)`) | Triggers code6cac.c structured-loop bug — symbols dropped from `.o` |
| Memory barrier between `lhu` and asm | Triggers code6cac.c bug |
| `register s32 bits asm("v1")` pin (alone) | 109 diffs (lifetime conflict with voice2 in inner block) |
| `register s32 const_four asm("t2") = 4` pin | 30 diffs regression |
| Removing all packet/output pins | 138 diffs |
| Reordering pin declarations | No change |
| Removing the `addiu 1` asm barrier | 131 diffs (LICM hoists `1`) |
| Interleaving output ptr setup between sp.packets writes | No change |
| Moving `i=0` before pointer setup | 50 diffs regression |
| Adding sdata_exclude entries | No change (function already uses lui+lw) |
| Permuter (6000 iter, 10 min) | Found only semantically broken variants — moved `packets =` initialization INSIDE switch case bodies, leaving packets uninitialized for first iteration |

### Barrier patterns that FAILED (don't re-add)

| Pattern | Outcome |
|---|---|
| `register s32 four_const asm("t2") = 4` + use in cases | Regresses to 20 diffs (cc1 generates extra moves to manage $t2) |
| `register s32 four_const asm("t2"); __asm__ volatile ("addiu ..." ...)` + use | **Triggers cc1 structured-loop bug** — voice and many other functions dropped from `.o` |
| `s32 reload = (s32)(u16)output[0]` separate variable | Triggers cc1 bug |
| `__asm__ volatile ("lhu %0, 0(%1)" : "=r"(voice2) : "r"(output))` | Closes addiu/lhu swap (2 diffs) but introduces 2 new diffs in sh-of-1 position (net 0 improvement) |
| `register s32 reload asm("v1")` | Conflicts with bits pin at $v1 (regression to 20 diffs) |
| `__asm__ volatile ("lbu %0, 2(%1)" : "=r"(b2_val) : "r"(packets))` inside case body | Massive regression (105 diffs, +1 instruction length) |
| `__asm__ volatile ("addiu %0, $zero, 1" ... : "0"(voice2))` with tied input | Forces decrement before asm — doesn't match target's decrement-after-asm pattern |

### Other non-paths

- **cc1psx as the build target** — refused per [[cc1psx-calibration-only]]. cc1psx is GCC 2.7.2.SN.1 (Sony fork); we are committed to decompals/mips-gcc-2.7.2 (open-source port). Switching is a project-level decision, not an agent fix.
- **CU-split to nudge cc1's RA** — refused per [[cu-split-evidence-required]]. No binary evidence supports voice being its own translation unit (no rodata gap, no .text gap, semantic grouping fits with file neighbors). Splitting purely for an RA nudge is the same moral category as regfix rules.
- **`-fno-strength-reduce` per-file in NO_SR_FILES** — affects entire code6cac.c (100+ functions); high regression risk; not attempted.
- **Bridge as `replace_with_asmfile`** — refused per [[bridge-is-not-decomp]]. 95% pure C + 8 narrow interventions beats 100% asm.
- **Authorize as canonical-asm** — refused per [[canonical-asm-retirement]]. `HAND_CODED` scan returns LOW (1/7, only S4 front-loads). No GCC-impossible signal. Not canonical asm; it's a normal C function that GCC's specific cost-model decisions can't match.

## The cc1 structured-loop bug (observed)

Triggered by several barrier+variable-declaration combinations involving `voice`. Symptom: `mipsel-linux-gnu-ld` reports `undefined reference to single_game_VoiceContorol` and many other code6cac.c functions. The `.o` file is missing those symbols. cc1 silently dropped large parts of the file's compilation output.

This appears to be the same family as the file-scope `__asm__` bug in code6cac.c (see [[canonical-asm-retirement]] §Known limitation). Inside-function `__asm__` blocks in specific arrangements can also trigger it.

Risk-free barrier patterns (the 3 used in the landed playbook): single-instruction asm with simple `"=r"` output and no input dependencies on local variables — or a single `"r"` input where the input is itself a register-pinned local.

## Pure-C journey — the diff trajectory

For reference / future calibration of similar functions:

| State | Diffs | Notes |
|---|---|---|
| HEAD (cheat-supported) | 0 | matches via 32 wildcard substs |
| Pure C, no rules, no barriers | 131 | LICM hoists `1`; everything cascades |
| Pure C + voice_mask pin + structural cleanups | 38 (v21) | 3 volatile asms (lhu/addiu 1/sh), voice→$v0 pin |
| Pure C + cc1psx-bits-pin insight | 20 (best v11-track) | length matches target, just 26 register-naming / preamble-order diffs |
| Pure C + bits explicit-init-all-paths + bits→$v1 pin | **18 (toolchain floor)** | confirmed by cc1psx calibration |
| 18-floor + 3 §6.1 barriers | **10** | the practical floor with §6.1 alone |
| 10-floor + 5 specific regfix rules | **0** | **the landed state (d33ea6b)** |

The plateau lessons: pure-C alone caps at ~18; adding barriers gains ~8 diffs; closing the remaining 10 requires specific (NOT wildcard) regfix.

## Saved artifacts (historical references — may not all still exist)

- `tmp/v21_voice_pin.diff` — v21 baseline (38 diffs)
- `tmp/code6cac.bak.20diffs.c` — best pure-C-only attempt
- `tmp/code6cac.bak.18diffs.c` — pure-C with cc1psx-pinning insight
- `tmp/code6cac.bak.10diffs.c` — pure-C + 3 barriers
- `tools/cc1psx_wrapper.sh` — cc1psx invocation (requires dosemurc setup above)

## See also

- [[minimize-asm-when-blocked]] — the principle this function tests
- [[community-standard]] — the SOTN bar voice now meets
- [[cc1psx-calibration-only]] — why cc1psx isn't a build path
- [[cu-split-evidence-required]] — why CU-split isn't an answer
- [[canonical-asm-retirement]] — why voice doesn't qualify (no GCC-impossible signal)
- [[bridge-is-not-decomp]] — why bridge wasn't the answer either

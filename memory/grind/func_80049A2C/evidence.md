# Evidence bank — func_80049A2C

## Baseline (session s1, 2026-07-20)
- Current src/text1b.c form: `sandbox --disable all` = **0** (score 0, 126/126 insns).
- Canonical verdict: **C** (pure-C distance 0).
- Target frame: `addiu $sp,-0x30` (48). Callee-saves s0,s1,s2,s3,ra at 0x18..0x28.
  Below 0x18: 24 bytes = 16 outgoing-arg area + 8 bytes locals padding.
  Frame equation (mips.c:compute_frame_size): 48 = ALIGN8(vars) + ALIGN8(args=16) + ALIGN8(gp_regs=20)
  → vars ∈ [1..8]. Target has 1–8 bytes of `get_frame_size()` locals.

## The four judge-flagged constructs — MEASURED contribution (isolated deletion, each with the other three still in place)
| Construct | Score after removing ONLY this | Load-bearing? |
|---|---|---|
| `s32 dummy[2];` + `(void) dummy;` | **12** | **YES** (frame slot for ALIGN8(vars)) |
| `char new_var4;` + `(void) new_var4;` | 0 | NO — pure noise |
| `if (a1_val) { }` | 0 | NO — pure noise (empty stmt DCE'd) |
| `(new_var = new_var3)` inline-assign + `int new_var;` | 0 | NO — pure noise |

**Combined:** removing all four noise-constructs together (keeping dummy) → 0.
Removing all four INCLUDING dummy → 12. Therefore dummy[2] is the SOLE
load-bearing cheat; three of the four judge-flagged items can just be deleted.

## Cross-reference
- phantom-frame-slots-gcc272 (memory/project/) — GCC 2.7.2 reserves locals bytes
  for LIVE locals it register-allocates away. The +8 target-frame slot is
  reachable in pure C via a live-then-DCE'd local (memory's minimal trigger:
  HImode local feeding HImode bitwise expression).
- dead-vars-local-array (.claude/rules/) — the 2026-07-01 SOTN carve-out for
  written-never-read arrays does NOT apply here: target has ZERO `sw ...,0x0-0x14($sp)`
  stores (only the 5 register saves), so no oracle-backed dead-store evidence.

## Session artifacts
- tmp/grind/func_80049A2C/s1/text1b.baseline.c — pre-session src snapshot.

- [s1] Baseline (on-disk src/text1b.c form): sandbox --disable all = 0, canonical verdict C (pure-C distance 0).

- [s1] Target func_80049A2C.s frame prologue: addiu $sp,-0x30; saves s0/s1/s2/s3/ra at 0x18/0x1C/0x20/0x24/0x28. Locals area = 0x00..0x17 = 24 bytes = 16 outgoing-arg + 8 unused padding.

- [s1] Frame equation (mips.c compute_frame_size, ALIGN8): 48 = ALIGN8(vars) + ALIGN8(16 args) + ALIGN8(20 gp_regs=5*4) → ALIGN8(vars)=8 → vars in [1..8].

- [s1] phantom-frame-slots-gcc272 (memory/project/) says GCC 2.7.2 reserves get_frame_size() for LIVE locals it RA's away; minimal trigger HImode bitwise. Provides a legitimate route to the +8 slot without any dead-declaration cheat.

- [s1] dead-vars-local-array 2026-07-01 SOTN carve-out does NOT apply: target has ZERO sw ...,0x0-0x14($sp) stores; the carve-out requires oracle-backed dead-store evidence.

- [s1] Noise-construct killings mean next session can safely drop the three cheats and focus its entire budget on the phantom-frame lever for dummy[2].

- [s2] [s2] Baseline confirmed: with s1's candidate applied to src/ (drops new_var4/empty-if/inline-assign, KEEPS dummy[2]), sandbox --disable all = 0, build_insns 126 == target_insns 126.

- [s2] [s2] With dummy[2] removed as well: sandbox --disable all = 12, build_insns 126. Same insns, purely sp-relative offset shifts (frame 0x28 vs target 0x30) — reproduces s1's measurement exactly.

- [s2] [s2] Target func_80049A2C.s contains SIX lh sign-extending loads: 0x0($s3) at 3A278 (bgez); 0x0($s3) at 3A2CC (a1_val); 0x0($v1) x3 (D_80099D3C reads); 0x12($s1) x3 (vehicle+0x12 lh reads). Precondition (1) of the phantom-slot mechanism (sign-extending narrow load) is SATISFIED here — the reason the mechanism doesn't fire is precondition (3): dead-injected HImode bitwise is DCE'd BEFORE combine sees the sign-ext-then-mask pattern.

- [s2] [s2] Recomputation of `a1_val = (*p_anim) * 2;` at the second obj-init block IS load-bearing (H3 KILLED). Removing it drops 3 target insns and shoots score to 34.

- [s2] [s2] Widening either a1_val or new_var2 to s32 does NOT trigger the phantom slot; new_var2 widening is strictly worse (H2 KILLED both axes).

- [s2] [s2] The tslLineG5Init witness form (`s16 v1 = load; s16 mask = load; if ((v1 & ~mask) & 1) {real stores;}`) works because its bitwise expression GATES real emitted stores — combine still proves the sign-ext redundant, but the outer if-body survives DCE so flow's reg_n_refs stays stale. In func_80049A2C every store is unconditional and the target sits at exactly 126 insns; there is no room to add a gated store WITHOUT diverging from target bytes.

- [s2] [s2] Consequence: the phantom-slot route via H1/H2 requires SEMANTIC CODE that emits SOMETHING but not more than the target. Every axis I probed either emits nothing (fully DCE'd, no phantom) or emits more than target (wrong bytes). This is a MEASURED negative for H1 and H2, not a lever-choice deficiency.

- [s3] [s3] cc1 -da greg dump of func_80049A2C on working baseline (src/text1b.c HEAD, dummy[2] in place, sandbox=0): 7 pseudos to allocate (89,123,133,141,94,73,81), ALL receive hard registers (Register dispositions all populated, none omitted). Only 'Spilling reg 7' event is a hard-reg $a3 spill around insn 135 (outgoing-arg passing to a call), NOT a pseudo→frame-slot reservation. No reload/alter_reg stale-ref reservation is emitted.

- [s3] [s3] cc1 -da greg dump on counterfactual (dummy[2] removed, otherwise identical body, sandbox=12): 7 pseudos, IDENTICAL register dispositions and IDENTICAL conflicts to the dummy-IN dump. Same $a3 spill for insn 135. Frame directive drops from vars=8 to vars=0.

- [s3] [s3] Consequence: the phantom-slot mechanism (phantom-frame-slots-gcc272) is provably NOT firing in func_80049A2C's shape. There is no live-code phantom-slot entry condition to replicate in a non-aggregate form.

- [s3] [s3] The target's +8 frame slot is therefore reachable ONLY via an aggregate declaration (array or struct). Combined with s2's H1/H2 (dead HImode-bitwise DCE'd before combine; scalar widening does not trigger slot) and the pre-existing rejected/scalar-dummy-no-frame.c (`s32 dummy;` scalar variants all sandbox=12, no frame slot — GCC reserves stack for AGGREGATES at declaration, not for scalars), the entire non-aggregate axis is closed.

- [s3] [s3] Two prior Judge FAIL rulings on this function (docs/grind/decisions.md 2026-07-19 23:46 and 2026-07-20 00:36) rejected: (a) the raw `s32 dummy[2]; (void) dummy;` shape as an [[inline-asm-policy]] catalog cheat with zero lever-exhaustion, and (b) the `s32 _pad[2]` FAKE-annotated OVERSIZED-LOCALS carve-out attempt as a fully-dead-pad first-family closure lacking the prerequisite engine `find_unused_local_arrays` allowlist wiring plus a `(void) _pad;` shim that syntactically defeats the detector.

- [s3] [s3] Owner endgame-lock-disposition rule filed 2026-07-20 (commit 717584ec, .claude/rules/endgame-lock-disposition.md) codifies the standing disposition for RA/scheduler-locked functions: canonical-asm ONLY with hand-coded evidence (scan_hand_coded STRONG signals); coercion families ONLY with SOTN precedent; absent both, INCOMPLETE-owner-accepted with cheat retained solely to hold the byte match. func_80049A2C fits the species (RA-locked +8-byte frame slot, exhausted phantom axis, no hand-coded signals per Judge's 2026-07-13 finding on the same asm-shape sibling func_80037540).

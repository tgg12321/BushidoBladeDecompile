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

- [s4] [s4] Baseline reconfirmed with current src (3 s1-noise dropped, dummy[2] kept): sandbox --disable all = 0, target_insns=126, build_insns=126, cheat_asm_stripped=397.

- [s4] [s4] H8 measurement: `struct { s32 a; s32 b; } dummy;` sandbox = 0 (same +8 frame slot as s32 dummy[2]; GCC 2.7.2 does not scalarize the two-field struct). Rejected form banked at memory/grind/func_80049A2C/rejected/struct-dummy-h8-same-frame-same-defect.c.

- [s4] [s4] scan_hand_coded.py --single func_80049A2C = HAND_CODED tier LOW score 0/8 (S1..S8 all negative). Canonical-asm-refusal certified: [[endgame-lock-disposition]] path (b) 'canonical-asm ONLY with hand-coded evidence' UNAVAILABLE.

- [s4] [s4] Permuter campaign infrastructure blocker: text1b.c INCLUDE_ASM_USE_MACRO_INC sibling func_8004A348 (~100-line embedded GTE asm block) trips (a) permuter's syntax parser — `_permuter_ignore_line __asm__(...)` prefix is ignored, and (b) the import.py-generated maspsx-based full-TU compile.sh pipeline — 'too many values to unpack (expected 2)'. A bespoke leaner workspace (tmp/grind/func_80049A2C/s4/build_perm_workspace.sh) hit the same maspsx exception. Setting up a running permuter workspace for func_80049A2C requires a project-tooling fix outside the grind-session contract.

- [s4] [s4] Cumulative aggregate-only conclusion is now exhausted across every sanctioned pure-C axis: phantom-firing H4 KILLED (s3 cc1 -da: mechanism does not fire), phantom-injection H1/H2/H3 KILLED (s2), scalar-widening KILLED (s2), scalar-dummy H6 KILLED (rejected/scalar-dummy-no-frame.c), struct-aggregate H8 KILLED (s4). The +8 frame slot is aggregate-only, and every aggregate spelling carries the same reviewer-visible 'no semantic purpose / fully-dead' defect that failed rulings 2026-07-19 23:46 + 2026-07-20 00:36.

- [s4] [s4] docs/grind/decisions.md grep(func_80049A2C) shows 4 hits (2 FAIL rulings + 2 headers, no OWNER-ESCALATION entry filed). Session-5 next action is filing that entry so 'owner-gated' disposition becomes valid per driver contract.

- [s4] [s4] End-of-session src state: restored to s1 candidate form (drop 3 noise, keep s32 dummy[2] + `(void) dummy;` sink). Sandbox = 0 reconfirmed post-restore.

- [s5] [s5] Baseline reconfirmed: sandbox --disable all = 0, target_insns=126, build_insns=126, cheat_asm_stripped=397, rules_dropped=0. Src/text1b.c is at the s1-recon candidate form (3 noise constructs dropped, s32 dummy[2] + (void) dummy sink at lines 1061 + 1123).

- [s5] [s5] Permuter import fresh attempt (my mandated modality) INDEPENDENTLY reproduced s4 blocker (1): tools/decomp-permuter/import.py warned 'Syntax error in base.c before: __asm__ at approximately line 1775' — the _permuter_ignore_line prefix is not honored by permuter's syntax parser on the canonical-asm sibling func_8004A348 (~100-line embedded GTE __asm__ block from INCLUDE_ASM_USE_MACRO_INC in text1b.c). Log: tmp/grind/func_80049A2C/s5/permuter-import-fresh.log.

- [s5] [s5] Permuter workspace compile.sh fresh direct-invocation INDEPENDENTLY reproduced s4 blocker (2): bash tools/wsl.sh 'cd nonmatchings/func_80049A2C && bash compile.sh base.c' died with 'MASPSX: An exception occurred: too many values to unpack (expected 2)' on the enlarged text1b.c TU. Log: tmp/grind/func_80049A2C/s5/permuter-compile-fresh.log. Both blockers are project-tooling fixes (touching tools/decomp-permuter/scripts + tools/maspsx) that the grind-session contract forbids.

- [s5] [s5] Cumulative permuter-modality closure: (a) even if the workspace could be built, s3's cc1 -da mechanism proof (7 pseudos all hard-reg allocated, delta between dummy-in and dummy-out greg passes = ZERO) proves the phantom-slot mechanism does not fire anywhere in this function's RTL pipeline; a permuter search would need to discover a mechanism the mechanism proof missed, which is extremely unlikely; (b) infrastructure fix is out of scope for a grind session per the standing contract; (c) [[permuter-fresh-seed-discipline]] campaign discipline requires a runnable workspace to even measure a fresh-seed basin — cannot apply here.

- [s5] [s5] OWNER-ESCALATION filed to docs/grind/decisions.md at line 954 (file grew from 951 → 975 lines). Header exactly matches the hirahira_w_frie / saTan0Init / cpu_side_move_dir_4 filed-by-grind template: '## 2026-07-20 — func_80049A2C (src/text1b.c) — **OWNER-ESCALATION** (filed by grind s5 per the 2026-07-19 23:46 + 2026-07-20 00:36 Judge FAILs and the standing 2026-07-20 endgame-lock-disposition rule; awaiting owner ruling — do not self-resolve)'. Grep verifies: exactly 1 occurrence of the OWNER-ESCALATION anchor for func_80049A2C. The escalation presents both options honestly, includes the full s1..s5 exhaustion ledger, plainly states no SOTN precedent for a fully-dead 8-byte pad, references .claude/rules/endgame-lock-disposition.md as governing policy, and notes recent precedent (saTan0Init 2026-07-20 → option (b); cpu_side_move_dir_4 2026-07-20 → option (b)).

- [s5] [s5] End-of-session src state: unchanged from session start (s1-recon candidate form; sandbox = 0, 126/126). No src edits attempted this session because the mandated modality (permuter) is infrastructure-blocked and every C-reachable axis was measured dead by s1..s4. candidate.c comment header updated to include the s5 note; no rejected form to bank (no new form probed on src).

- [s5] [s5] Contract satisfaction for 'owner-gated' verdict: (1) OWNER-ESCALATION for func_80049A2C exists at docs/grind/decisions.md:954 — CHECK; (2) every remaining sanctioned axis measured dead across the ledger (phantom-firing H4 KILL s3, phantom-injection H1/H2/H3 KILL s2, scalar-widening KILL s2, scalar-dummy H6 KILL rejected/, struct-aggregate H8 KILL s4, canonical-asm scan_hand_coded LOW 0/8 s4, permuter infrastructure-blocked s4 + INDEPENDENTLY-RECONFIRMED s5) — CHECK; (3) escalation_ref set below to the exact header line — CHECK.

## Session s6 (synthesis, 2026-08-19) — THE BANKED FLOOR OF 0 WAS AN ARTIFACT

- [s6] **CHASSIS RE-MEASURE.** src/text1b.c on main now carries
  `INCLUDE_ASM("asm/funcs", func_80049A2C);` (asm-until-matched migration,
  owner ruling 2026-08-19). Applying the s1..s5 candidate body verbatim to
  src and running `sandbox func_80049A2C --disable all` reproduces score 0,
  126/126 insns, cheat_asm_stripped=280 (was 397 pre-migration). So the
  "0" is reproducible on the new chassis — and s6 then established that the
  0 is not a byte match.

- [s6] **ROOT CAUSE OF THE FALSE 0 — a cheat-stripper spelling hole.** The
  s1 candidate declares `s32 dummy[2];    /* LOAD-BEARING: +8 byte frame
  slot; +12 score if removed */`. Deleting ONLY that trailing comment
  (byte-identical C otherwise, same declaration, same `(void) dummy;` sink,
  same position) flips the sandbox score from 0 to 12. Reproduced twice,
  alternating base/no-comment in one batch. Proof by inspection of the
  source the sandbox actually compiles
  (`tmp/sandbox/func_80049A2C/src/text1b.c`): with the comment, the
  `(void) dummy;` statement is blanked but the DECLARATION SURVIVES; without
  it, both are blanked. Mechanism named from the engine source (read-only):
  `engine/volatile_cheats.py:_ORPHAN_DECL_RE` is anchored `;[ \t]*$`, so a
  trailing comment on the declaration line makes the orphaned-declaration
  closure miss it, while `find_void_discard_unused_locals` still strips the
  `(void) dummy;`. The dead 8-byte local therefore survived into the
  "cheat-invisible" build and produced target's `addiu $sp,-0x30`.

- [s6] **THE HONEST FLOOR IS 12, NOT 0.** The clean body (no `dummy` in any
  spelling) measures `sandbox --disable all` = 12, target_insns 126,
  build_insns 126 — the entire residual is sp-relative offset shift from
  frame 0x28 vs target 0x30. This matches
  `memory/grind/func_80049A2C/migration_pin.json` ("floor": 12) exactly. The
  s1..s5 floor_history entries of 0 are hereby superseded; every conclusion
  in this ledger that was drawn from a *score* of 0 must be re-read as
  "scored 0 through the stripper hole", and every conclusion drawn from a
  *score of 12* on a strippable spelling is an artifact of the strip, not a
  codegen fact (see the matrix below).

- [s6] **TWO MORE SPELLING HOLES CONFIRMED (both give real frame 48 AND
  sandbox 0).** (a) `struct { s16 a; s16 b; } dummy;` with ZERO references —
  no detector covers a zero-reference struct-typed local
  (`find_unused_local_arrays` matches array declarators only;
  `find_orphaned_local_decls` requires >=1 reference inside a stripped span
  and its type alternation has no `struct`). This is the same construct as
  s4's H8, so **s4's "H8 sandbox = 0" is an artifact of the same class**.
  (b) `s32 dummy[2];` + `(void) dummy[0];` — `_VOID_DISCARD_NO_ADDR_RE`
  matches only `(void) name;`, not an indexed read, so nothing is stripped;
  additionally a dead read of an uninitialized local (UB). Banked as
  rejected/struct-zero-ref-detector-hole.c and
  rejected/void-discard-indexed-read-hole.c. Under checklist test T4 these
  pass detectors ONLY because the detectors miss the spelling — automatic
  FAIL, not candidates.

- [s6] **FRAME-SLOT MECHANISM LAW (10 real-build measurements; frames read
  from `make build/src/text1b.o` + objdump, which the stripper cannot touch;
  full matrix at tmp/grind/func_80049A2C/s6/matrix.md).** Baseline frame
  with no locals = 40; target = 48. The +8 appears iff the local aggregate is
  BLKmode, i.e. its (size, alignment) does NOT admit a scalar integer machine
  mode:
    * `char dummy[1]` (QImode), `s16 x[1]` (HImode), `s32 dummy[1]` (SImode)
      -> promoted to a pseudo, ZERO frame bytes -> frame 40.
    * `char dummy[2]`, `char dummy[3]`, `char dummy[4]`, `s32 dummy[2]`,
      `struct { s16 a; s16 b; }` -> BLKmode -> assign_stack_local ->
      get_frame_size() 2..8 -> ALIGN8 -> 8 -> frame 48 = target.
  **ALIGNMENT decides, not size**: `char dummy[4]` reaches 48 while
  `s32 dummy[1]` (also 4 bytes) does not. Frame equation corrected:
  48 = ALIGN8(vars=2..8) + 16 outgoing-args + 20 gp-saves.

- [s6] **`(void) dummy;` IS NOT LOAD-BEARING — s1's reading was inverted.**
  A zero-reference BLKmode local ALONE reaches frame 48 (`s32 dummy[2];`,
  `char dummy[2];`, `char dummy[3];`, `struct{s16;s16;}` all measured 48 with
  no reference of any kind). The reason s1 believed the sink was required is
  that removing the sink also removed the comment's protection in some
  variants and, for arrays with 2<=n<=64 and zero references,
  `find_unused_local_arrays` strips the declaration outright. So the Judge's
  constraint-2 note ("wire the `find_unused_local_arrays` allowlist so the
  pad can stand WITHOUT a `(void) _pad;` shim") is achievable at the CODEGEN
  level today — the shim was never needed — but it remains blocked at the
  POLICY level (the pad is still a wholly dead local) and at the ENGINE level
  (engine/ is outside the grind-session surface).

- [s6] **LIVE-AGGREGATE AXIS KILLED (new).** The only way an aggregate could
  have a semantic role and still hold the slot is if it is BLKmode *and* its
  live uses cost zero instructions. Measured counterexample: `s16
  a1_val_a[1];` genuinely carrying `a1_val` (written twice, read twice — a
  real semantic role, no dead declaration) gives real frame 40, sandbox 12,
  build_insns 126 — GCC promotes the HImode-able one-element array to a
  pseudo, so it reserves no frame bytes and emits no stack traffic. Push it
  to BLKmode (>= 2 elements or misaligned) and its live uses necessarily emit
  `sw`/`lw` at 0x00..0x14($sp); `asm/funcs/func_80049A2C.s` contains ZERO
  such accesses (only the five s0/s1/s2/s3/ra saves at 0x18..0x28) and is
  exactly 126 instructions with no slack. Banked as
  rejected/live-one-element-array-no-slot.c.

- [s6] **CLOSURE.** Combining s3's cc1 -da proof (no reload/alter_reg phantom
  slot fires anywhere in this function's RTL pipeline) with the s6 BLKmode
  law and the live-aggregate kill: target's +8 locals area is reachable ONLY
  through a wholly dead, memory-resident local declaration — i.e. the
  `unused-local-array frame coercion` forbidden family. There is no pure-C
  form in which every local has a semantic role and the frame is 0x30.
  Every 0 this function has ever scored was one of those constructs hidden
  from the stripper by a spelling hole.

- [s6] Target-asm re-verification (fresh grep of asm/funcs/func_80049A2C.s):
  126 instructions; sp-relative accesses are exactly `sw s1,0x1C` `sw ra,0x28`
  `sw s3,0x24` `sw s2,0x20` `sw s0,0x18` and the five matching `lw`s; three
  calls (func_80052C10 = InitFadePanel, func_8004153C, func_800417D0), all
  with <= 1 argument, so outgoing_args_size is pinned at the 16-byte
  REG_PARM_STACK_SPACE minimum and cannot absorb the +8.

- [s6] End-of-session src state: src/text1b.c restored EXACTLY to its
  session-start content (`INCLUDE_ASM("asm/funcs", func_80049A2C);`);
  `git status` shows no src dirt. candidate.c replaced with the honest clean
  body (floor 12, no `dummy` in any spelling) carrying the full s6 law in its
  header.

- [s6] CHASSIS: src/text1b.c on main carries INCLUDE_ASM("asm/funcs", func_80049A2C) after the 2026-08-19 asm-until-matched migration. Applying the s1-s5 candidate body reproduces sandbox 0 (126/126, cheat_asm_stripped 280), so the historical number is reproducible on the new chassis - and s6 proved it is not a byte match.

- [s6] THE FALSE 0: deleting only the trailing comment from `s32 dummy[2];    /* LOAD-BEARING ... */` (byte-identical C otherwise) flips sandbox --disable all from 0 to 12. Reproduced twice alternating in one batch. Post-strip source at tmp/sandbox/func_80049A2C/src/text1b.c shows `(void) dummy;` blanked but the declaration KEPT when the comment is present, and both blanked when it is absent.

- [s6] MECHANISM (read-only from engine source): engine/volatile_cheats.py:_ORPHAN_DECL_RE ends `;[ \t]*$`, so a trailing comment on a declaration line defeats the orphaned-declaration closure while find_void_discard_unused_locals still removes the `(void) dummy;` sink.

- [s6] HONEST FLOOR = 12: the clean body (no `dummy` in any spelling) measures sandbox --disable all = 12, target_insns 126, build_insns 126, real frame 0x28 vs target 0x30 - the whole residual is sp-relative offset shift. This equals memory/grind/func_80049A2C/migration_pin.json's independently recorded floor of 12. The s1-s5 floor_history entries of 0 are superseded.

- [s6] TWO MORE SPELLING HOLES (real frame 48, sandbox 0 each): `struct { s16 a; s16 b; } dummy;` with zero references (no detector has a struct/union arm), and `s32 dummy[2];` + `(void) dummy[0];` (_VOID_DISCARD_NO_ADDR_RE matches only `(void) name;`, not an indexed read; it is also a dead read of an uninitialized local). The first means session s4's H8 zero is an artifact of the same class.

- [s6] FRAME-SLOT LAW (10 real unstripped builds, tmp/grind/func_80049A2C/s6/matrix.md): frame is 40 with no locals and 48 with any BLKmode local. char[1] (QI), s16[1] (HI), s32[1] (SI) are promoted to pseudos and reserve NOTHING; char[2], char[3], char[4], s32[2], struct{s16;s16;} are BLKmode, go through assign_stack_local, and reserve 8 after ALIGN8. ALIGNMENT decides, not size (char[4] -> 48, s32[1] -> 40, both 4 bytes).

- [s6] CORRECTED FRAME EQUATION: 48 = ALIGN8(vars in 2..8) + 16 outgoing-args + 20 gp-saves. Session s1's `vars in [1..8]` reading is tightened by measurement: a 1-byte local yields frame 40 because it is QImode-promotable, not because 1 byte is too small.

- [s6] `(void) dummy;` IS NOT LOAD-BEARING - a zero-reference BLKmode local reaches frame 48 on its own (measured for s32[2], char[2], char[3], struct{s16;s16;}). Session s1's opposite reading was an artifact of the stripper. The Judge's constraint-2 note about standing the pad WITHOUT a shim is therefore already true at the codegen level; it remains blocked at the policy level (still a wholly dead local) and at the engine level (engine/ is outside the grind-session surface).

- [s6] LIVE-AGGREGATE KILL: `s16 a1_val_a[1];` carrying a1_val with every use real gives real frame 40, sandbox 12, 126 insns - register-allocated, zero stack traffic. Any BLKmode-sized live aggregate emits sw/lw at 0x00..0x14($sp), which target lacks entirely.

- [s6] TARGET RE-VERIFIED by fresh grep of asm/funcs/func_80049A2C.s: 126 instructions; the only sp-relative accesses are the five saves at 0x18/0x1C/0x20/0x24/0x28 and their matching loads; three calls (func_80052C10, func_8004153C, func_800417D0), all with <= 1 argument, so outgoing_args_size is pinned at the 16-byte REG_PARM_STACK_SPACE minimum and cannot absorb the +8.

- [s6] CLOSURE: combining session s3's cc1 -da proof (no reload/alter_reg phantom slot fires in this function's RTL pipeline) with the s6 BLKmode law and the live-aggregate kill, target's +8 locals area is reachable ONLY through a wholly dead memory-resident local declaration - the unused-local-array frame-coercion forbidden family. Every 0 this function has ever scored was such a construct hidden from the stripper by a spelling hole.

- [s6] DISPOSITION NOTE: no owner-gated claim is made this session. The pre-existing 2026-07-20 OWNER-ESCALATION rests on the now-disproved 'bytes proven, blocked only by policy' premise; a correction entry was filed at the tail of docs/grind/decisions.md (2026-08-19) rather than a terminal ruling, and the driver has not assigned escalation modality.

- [s6] SCOPE: src/text1b.c was restored EXACTLY to its session-start INCLUDE_ASM content; git status shows changes only in memory/grind/func_80049A2C/, docs/grind/decisions.md and the engine's own metrics/events.jsonl.

## [s7] Chassis re-measure: honest floor is 12, and the entire residual is the frame size.
- mechanism: `sandbox func_80049A2C --disable all` with the s6 candidate applied to src/text1b.c.
- probe: applied memory/grind/func_80049A2C/candidate.c verbatim at src/text1b.c:867 and ran the sandbox.
- result: score 12, target_insns 126, build_insns 126, rules_dropped 0. Target's five saves sit at 0x18/0x1C/0x20/0x24/0x28 with `addiu $sp,-0x30`; our build emits the identical five saves 8 bytes lower with `addiu $sp,-0x28`. The 12 differing instructions are exactly 1 prologue adjust + 5 saves + 5 restores + 1 epilogue adjust. Nothing else differs.
- verdict: CONFIRMED

## [s7] The +8 is NOT a cc1psx-vs-fork compiler divergence (kills the standing cross-ledger claim for this function).
- mechanism: memory/grind/func_80022F34 (brief-2026-08-18:38/44, evidence.md:77/137/296) records the belief that func_80049A2C and func_80037540 "must ADD a slot cc1psx reserves that our fork does not". That claim had never been measured on this function.
- probe: preprocessed the real src/text1b.c TU with the project cpp flags and fed the SAME .i to both compilers: tools/gcc-2.7.2/build/cc1 (project flags) and tools/cc1psx_wrapper.sh (the original PsyQ GCC 2.7.2.SN.1 via dosemu2, -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -w). Compared the emitted `.frame` directives.
- result: IDENTICAL frame decision from both compilers - `.frame $sp,40,$31 # vars= 0, regs= 5/0, args= 16, extra= 0`. cc1psx reserves nothing either. (Side observation: our fork hoists `sw $17,20($sp)` away from the other four saves exactly the way target does, while cc1psx emits all five saves contiguously - for this function the fork is the CLOSER compiler, not the further one.) Artifacts: tmp/grind/func_80049A2C/s7/fork.s, tmp/grind/func_80049A2C/s7/psx.s.
- verdict: KILLED (the compiler-divergence axis is dead; the +8 has to come from the C)

## [s7] Frame equation re-derived from the compiler source instead of from arithmetic guesses.
- mechanism: tools/gcc-2.7.2/config/mips/mips.c:4444 compute_frame_size. total = MIPS_STACK_ALIGN(get_frame_size()) + MIPS_STACK_ALIGN(current_function_outgoing_args_size) + extra + MIPS_STACK_ALIGN(gp_reg_size) + MIPS_STACK_ALIGN(fp_reg_size); gp_sp_offset = args + extra + vars + gp_reg_size - 4.
- probe: read the function end to end; cross-checked against target's save offsets (highest save 0x28, five saves) and against a measured 6-save variant (rejected/phantom-slot-hoist-rot-table-ptr.c).
- result: gp_reg_size is ROUNDED UP TO 8 before it enters the total, so 5 saves (20 B) and 6 saves (24 B) both contribute 24 - a 6th callee-save can never move this frame (measured: 6 saves, frame still 40). Therefore target's 0x30 forces vars + args = 24 exactly, i.e. either vars=8/args=16 or vars=0/args=24, and the two decompositions are byte-indistinguishable in the emitted code. args=24 still requires a call whose 5th argument word is STORED at 0x10($sp), and target has zero non-save sp traffic, so the live decomposition is vars=8.
- verdict: CONFIRMED

## [s7] s6's law "the +8 slot is reachable ONLY through a wholly dead, memory-resident local" is FALSE. 70 functions in this repo's own oracle-matching source carry a nonzero `vars=` with ZERO stack traffic, and none of them needs a dead local.
- mechanism: frame bytes also come from reload1.c:2404 `alter_reg`, which gives every pseudo with reg_renumber < 0 and reg_n_refs > 0 a stack slot sized MAX(inherent, reg_max_ref_width) and then rounded up to 8 by assign_stack_local's align == -1 path. A pseudo whose insns were all absorbed by combine keeps a stale reg_n_refs, is never allocated a hard register, and gets an 8-byte slot NO INSTRUCTION EVER TOUCHES.
- probe: compiled every src/*.c to .s with the project cc1 flags (tmp/grind/func_80049A2C/s7/sweep/), parsed all ~1200 `.frame` directives, and kept the functions whose body has no `($sp)` reference other than the callee-save stores/loads; then re-ran the instrumented cc1 with BB2_FRAME_DEBUG=1 to attribute each allocation.
- result: 70 such functions, 26 of them LOOPLESS. Every attributed one is `ctx=spill_new_p<N> mode=4 size=8 align=-1 alignment=8` - a phantom reload spill slot, not a local. Clean ordinary-C examples: `memset` (src/display.c:926, a six-line for-loop, frame 8 / vars 8, zero stack traffic) and - decisively - `func_800493E4` in OUR OWN FILE src/text1b.c (loopless, frame 0x20, vars=8, two saves, zero stack traffic, no dead local anywhere in its body). Since the tree SHA1-matches the oracle, all 70 are target-proven constructs.
- verdict: KILLED (s6's law) / CONFIRMED (the phantom-spill mechanism)

## [s7] Exact GCC pass and line that creates the phantom slot: combine.c distribute_notes' orphaned-REG_DEAD `(use)` insn.
- mechanism: when combine folds an insn away and the REG_DEAD note for its destination pseudo cannot be placed on any surviving insn, and the placement scan reaches a CODE_LABEL, combine.c:10836-10841 emits `(use (reg:SI N))` after that label to carry the note ("prevents problems with call-state tracking in caller-save.c"). The USE emits no code but keeps reg_n_refs[N] > 0. regclass then never sees a real constraint for N and reports it as "ST_REGS or none"; global_alloc lists it with an empty conflict set and does not allocate it; alter_reg hands it the 8-byte-rounded stack slot that nothing references.
- probe: minimal repro of `memset` under `cc1 -da` (tmp/grind/func_80049A2C/s7/mini/) - pseudo 79 is the loop-invariant `-1` that combine folded into the branch; lreg prints "Register 79 used 2 times across 2 insns in block 0; ST_REGS or none", greg lists "79 conflicts:" with no entry in Register dispositions, FRAMEDBG reports spill_new_p79. Then the same trace on func_800493E4 in the real text1b dumps (tmp/grind/func_80049A2C/s7/da/): pseudo 98 is an address-forming pointer whose HImode store combine absorbed, and it survives ONLY as `(insn 150 (use (reg:SI 98)))` sitting after code_label 83.
- result: mechanism named end to end and reproduced in two independent functions, one of them loopless and in this function's own TU.
- verdict: CONFIRMED

## [s7] Five ordinary-C reshapings of func_80049A2C aimed at orphaning a REG_DEAD across a label all leave vars=0.
- mechanism: the phantom slot needs a pseudo whose defining insn combine absorbs into a use in another block, with the death note stranded at a label. Each variant moves one definition across one of this function's two labels (the temp_v1 == 0xFF early return and the InitFadePanel branch).
- probe: five variants built through the real cpp + the instrumented cc1 on the full TU, read back through BB2_FRAME_DEBUG and the `.frame` directive: hoist the D_800EF980 base above the early return; hoist the D_80099D3C rotation-table pointer above the fade branch; pre-read `*p_anim` into a named s16 before the branch; form `obj = D_800A38B4` before the branch; invert the guard to `if (temp_v1 != 0xFF) { ... }`.
- result: all five report the single `ctx=round_frame frame_offset=0` record - zero frame allocations, frame 0x28. The rotation-table hoist additionally raised the callee-save count to 6 and STILL produced frame 40, independently confirming the gp_reg_rounded law above. Banked as memory/grind/func_80049A2C/rejected/phantom-slot-*.c.
- verdict: KILLED (these five spellings; the axis itself is wide open)

- [s7] Chassis re-measured this session: sandbox --disable all = 12, target_insns 126, build_insns 126, rules_dropped 0, with the s6 candidate applied at src/text1b.c:867. The 12 differing instructions are exactly 1 prologue adjust + 5 saves + 5 restores + 1 epilogue adjust; target `addiu $sp,-0x30` with saves at 0x18/0x1C/0x20/0x24/0x28 vs our `addiu $sp,-0x28` with the same five saves 8 bytes lower.

- [s7] cc1psx and the project fork produce the IDENTICAL frame decision for this body (vars=0, regs=5/0, args=16, frame 40). The cross-ledger claim in memory/grind/func_80022F34 that func_80049A2C 'must ADD a slot cc1psx reserves that our fork does not' is measured false and should not be repeated.

- [s7] MIPS frame bytes have two sources, not one: expand-time locals/temps (get_frame_size()) AND reload spill slots (reload1.c:2404 alter_reg). The latter rounds every slot up to 8 bytes via assign_stack_local's align == -1 path, so a single unallocated SImode pseudo costs exactly the 8 bytes this function is missing.

- [s7] 70 functions in this repo's oracle-matching source carry vars > 0 with zero non-save sp traffic; 26 are loopless. All attributed instances are phantom reload spill slots (FRAMEDBG ctx=spill_new_p<N>), not dead locals. Loopless examples worth reading next: func_80042F10, func_80086014, func_80086130, snd_CalcFade, snd_GetFadeCurve, disp_CalcFov, get_cs, get_ce, and - same file as the target - func_800493E4 and func_800493E4's neighbours in text1b.c.

- [s7] func_800493E4 (src/text1b.c) is the strongest precedent: loopless, ordinary C, no dead local, no register pin, frame 0x20 with vars=8 and zero stack traffic. Its carrier is pseudo 98, an address-forming pointer for an indexed HImode store; combine absorbed the pointer into the store and left `(insn 150 (use (reg:SI 98)))` after code_label 83.

- [s7] Named mechanism for the ledger: combine.c:10836-10841 distribute_notes emits a codegen-free `(use (reg))` after a CODE_LABEL when an orphaned REG_DEAD note cannot be placed; that USE keeps reg_n_refs > 0, regclass yields 'ST_REGS or none', global_alloc leaves reg_renumber = -1, and alter_reg reserves the phantom 8 bytes.

- [s7] A 6th callee-saved register does NOT change this frame (measured: 6 saves, frame still 40) because compute_frame_size rounds gp_reg_size to 8 before adding it. Any future session proposing to buy the slot with an extra saved register can skip the experiment.

- [s7] Five ordinary-C label-crossing reshapings of this body were measured and all left vars=0; they are banked in memory/grind/func_80049A2C/rejected/phantom-slot-*.c with the measurement in each header.

- [s7] src/text1b.c was restored to its committed INCLUDE_ASM state at the end of the session; no build-surface file was modified.

## [s8] Chassis re-measure (forensics modality): honest floor is 12 and unchanged.
- mechanism: `sandbox func_80049A2C --disable all` with the s7 candidate applied at src/text1b.c:867.
- probe: applied memory/grind/func_80049A2C/candidate.c verbatim and ran the sandbox; then re-ran it on the s8 cleaned body.
- result: score 12, target_insns 126, build_insns 126, rules_dropped 0 for both. The ledger floor of 12 is correct on this chassis; the driver's "measurement unavailable" is not a change.
- verdict: CONFIRMED

## [s8] The `int new_var3; new_var3 = 8;` constant holder in the candidate is pure noise and is now GONE.
- mechanism: it was used only as `*((s16 *) (obj + new_var3)) = 0;` twice, i.e. an opaque constant holder of the kind the 2026-07-20 Judge FAIL flagged as needing a FAKE carve-out.
- probe: three variants - P (second use literal 8), Q (first use literal 8), R (both uses literal 8, declaration and assignment deleted) - built through the real cpp + instrumented cc1 on the full TU, then R re-scored through the sandbox.
- result: all three report vars=0 and 107 cc1 insns, identical to the baseline. R additionally scores sandbox 12 with build_insns 126 - byte-for-byte the same output as the s6/s7 candidate. The constant holder never bought anything. candidate.c now omits it.
- verdict: KILLED (the construct is unnecessary; one of the four Judge-named constructs retired by measurement)

## [s8] The func_800493E4 precedent does NOT depend on its `do { } while (0);` FAKE.
- mechanism: func_800493E4 (same TU) carries a do-while(0) loop-note FAKE, so its phantom-slot precedent value was open to the objection that the slot is an artefact of that construct rather than of ordinary C.
- probe: deleted the do-while(0) and its comment from src/text1b.c and re-dumped the TU with BB2_FRAME_DEBUG=1.
- result: unchanged - `.frame $sp,32,$31 # vars= 8, regs= 2/0, args= 16` with `FRAMEDBG func=func_800493E4 ctx=spill_new_p98 mode=4 size=8`. The slot is produced by the ordinary C, not by the FAKE. s7's use of this function as an ordinary-C precedent stands.
- verdict: CONFIRMED

## [s8] THE TRIGGER LAW for the phantom slot, isolated in a nine-function minimal repro.
- mechanism: combine puts a global array's `symbol_ref` into a pseudo only when the symbol is accessed at two or more NON-CONSTANT indices; the second access then forms a single-use address add `P = idx + symbol_reg`, combine folds it back into the mem and deletes the def, and combine.c:10836 (distribute_notes) strands the REG_DEAD note on a codegen-free `(use (reg:SI P))`. reg_n_refs[P] stays > 0, regclass reports "ST_REGS or none", global_alloc leaves reg_renumber = -1, and reload1.c:2404 alter_reg reserves the 8-byte-rounded slot no insn references.
- probe: tmp/grind/func_80049A2C/s8/mini/m.c - nine one-line functions over `extern s16 G[]; extern u8 T[];`, compiled through the project cpp + the instrumented cc1, read back through `.frame` + FRAMEDBG.
- result: `G[a]=1; t=T[a*2]; if (t!=0xFF) G[t]=1;` gives vars=8. Necessary/sufficient conditions measured:
  * drop the first access entirely -> vars=0
  * make the first access a CONSTANT index `G[0]=1` (w2) -> vars=0
  * make the first access a READ `sink=G[a]` (w3) -> vars=8 (store vs read is irrelevant)
  * use the SAME index in both accesses (w4) -> vars=0 (CSE merges them)
  * delete the branch between the accesses (w5) -> vars=8 (no basic-block boundary is needed)
  * two later accesses `G[t]=1; G[t+1]=1;` (w6) -> vars=8
  * go through a pointer `p=&G[t]; *p=1;` (w7), or with `p[1]` too (w8) -> vars=8
  * two distinct index VARIABLES `G[a]=1; ... G[b]=1;` (w9) -> vars=8
  So the law is: TWO OR MORE ACCESSES TO ONE GLOBAL ARRAY AT TWO OR MORE DISTINCT VARIABLE INDEX EXPRESSIONS. Everything else (store vs load, branch vs straight line, pointer vs subscript) is free.
- verdict: CONFIRMED

## [s8] The trigger transplants into func_80049A2C and scales 1:1 with the number of single-use indexed accesses.
- mechanism: the function's rotation-table read `src = &D_80099D3C[(arg1 & 1) * 6]` followed by five `src++` walks is exactly one index expression; respelling the walks as subscripts creates N distinct index expressions.
- probe: variant D - all six rotation reads spelled `D_80099D3C[k], D_80099D3C[k+1], ... D_80099D3C[k+5]` with `s32 k = (arg1 & 1) * 6;`.
- result: `.frame $sp,80,$31 # vars= 40, regs= 6/0, args= 16` with FIVE FRAMEDBG records (spill_new_p118/p130/p142/p154/p159, 8 bytes each). Six accesses produced five orphans - one per address add beyond the base. The mechanism is fully controllable from C in this body.
- verdict: CONFIRMED

## [s8] TARGET'S FRAME REPRODUCED FROM ORDINARY C - variant F2 hits `.frame $sp,48 # vars= 8`.
- mechanism: keep the walking pointer (which is what reproduces target's `addiu $v1,$v1,0x2` sequence) but make the FIRST rotation read a subscript, so exactly one single-use address add exists to be orphaned.
- probe: variant F2 - `kidx = (arg1 & 1) * 6; *(s32*)(obj+0x4C) = (D_80099D3C[kidx] * *(s16*)(vehicle+0x12)) >> 12; src = &D_80099D3C[kidx + 1];` with the remaining four `src++` walks unchanged. (Variant F is the same idea with the index expression written out twice instead of named; identical result.)
- result: `.frame $sp,48,$31 # vars= 8, regs= 6/0, args= 16, extra= 0` - EXACTLY target's 0x30 frame - with `FRAMEDBG ctx=spill_new_p115 mode=4 size=8 align=-1 alignment=8`. No dead local, no pad, no `(void)` discard, no register pin, no inline asm. This is the first time in eight sessions that target's frame has been produced honestly. Banked as memory/grind/func_80049A2C/s8_variant_F2_frame48.c.
- verdict: CONFIRMED (s6's "reachable ONLY through a dead local" law is now refuted by construction, not just by precedent)

## [s8] F2 is not yet a match: the second index expression costs a SIXTH callee-saved register.
- mechanism: `regs= 6/0` instead of target's `5/0`. compute_frame_size rounds gp_reg_size to 8 before adding it, so 5 saves (20 B) and 6 saves (24 B) both contribute 24 and the frame TOTAL is still exactly 48 - the 6th save is invisible in the frame but visible in the instruction stream as an extra sw/lw pair.
- probe: `sandbox func_80049A2C --disable all` on F2 and on F.
- result: score 50, build_insns 129 vs target 126 (F: same 50/129). The residual is now +3 instructions and a register-allocation reshuffle, NOT a frame-size wall. The problem class changed from "unreachable" to "register pressure".
- verdict: CONFIRMED

## [s8] Ten other respellings measured dead (all vars=0), banked in rejected/.
- mechanism: each moves or re-forms one address computation in this body without creating a SECOND distinct variable index on the same symbol - which the s8 trigger law says is the necessary condition.
- probe: variants B, C, E, F5, H, I, J, K, N, O, P, Q built through the real cpp + instrumented cc1 on the full TU.
- result: B/C (array-index the D_800EF980 reads) vars=0, 107 insns - CSE merges the two same-index reads. E/F5 (index only the LAST rotation read) vars=0 - CSE rewrites it as `src[1]` because src is already live. H (named rotation index, same access form) vars=0. I (`p_anim = &D_800EF980[temp_v1]`) vars=0, codegen-identical. J (ot store reorder) vars=0. K/O (fold the D_80099CC8 base+index) vars=0. N (a SECOND rotation pointer `&D_80099D3C[k+3]`) vars=0 - both pointers are multi-use so neither address add is single-use. P/Q (single-use constant holder) vars=0.
- verdict: KILLED (these ten spellings)

- [s8] SCOPE: the only build-surface file touched was src/text1b.c, restored at end of session to its committed `INCLUDE_ASM("asm/funcs", func_80049A2C);` content. All other writes are under memory/grind/func_80049A2C/ and tmp/grind/func_80049A2C/s8/.

- [s8] [s8] Chassis re-measured: `sandbox func_80049A2C --disable all` = 12, target_insns 126, build_insns 126, rules_dropped 0, with the candidate applied at src/text1b.c:867. The ledger floor of 12 is correct on this chassis.

- [s8] [s8] TARGET'S FRAME IS NOW REPRODUCIBLE FROM ORDINARY C. Variant F2 - `kidx = (arg1 & 1) * 6; *(s32*)(obj+0x4C) = (D_80099D3C[kidx] * *(s16*)(vehicle+0x12)) >> 12; src = &D_80099D3C[kidx + 1];` with the remaining four `src++` walks unchanged - emits `.frame $sp,48,$31 # vars= 8, regs= 6/0, args= 16, extra= 0`, exactly target's 0x30 frame, with no dead local, no pad, no `(void)` discard, no register pin and no inline asm. Banked at memory/grind/func_80049A2C/s8_variant_F2_frame48.c.

- [s8] [s8] F2 is not yet a match: it reports `regs= 6/0` where target has `5/0`. sandbox = 50, build_insns 129 vs target 126. The frame TOTAL is insensitive to the sixth save (compute_frame_size rounds gp_reg_size to 8, so 5 and 6 saves both contribute 24), so the remaining delta is purely +3 instructions and a register-allocation reshuffle. The problem class changed from 'frame unreachable' to 'register pressure worth 3 instructions'.

- [s8] [s8] The trigger law, measured in a nine-function minimal repro: two or more accesses to ONE global array at two or more DISTINCT VARIABLE index expressions produce one phantom 8-byte slot per single-use address add beyond the first. Constant index in one of the accesses -> no slot. Same index in both -> no slot (CSE merges). Load vs store, branch vs straight line, subscript vs pointer are all irrelevant. Six subscripted accesses in the real function gave five slots.

- [s8] [s8] s7's belief that the carrier must cross a CODE_LABEL is measured false (minimal repro w5, no branch at all, still vars=8). That is why all five s7 label-crossing variants left vars=0: they moved definitions but never created a second index expression.

- [s8] [s8] The func_800493E4 precedent survives removal of its `do { } while (0);` FAKE (still `vars= 8`, `ctx=spill_new_p98`), so it remains a clean ordinary-C, oracle-proven precedent for this frame signature in this function's own TU.

- [s8] [s8] The candidate's `int new_var3; new_var3 = 8;` constant holder is completely codegen-neutral: replacing both `obj + new_var3` uses with the literal 8 and deleting the local keeps sandbox at 12 with build_insns 126. It has been removed from candidate.c - one of the four constructs named by the 2026-07-20 Judge FAIL is now retired by measurement rather than argument.

- [s8] [s8] RTL mechanism confirmed in the real dump: in func_800493E4 the carrier is pseudo 98 (`98 = 97 + 95`, where 95 holds the CSE'd `symbol_ref D_800EF980`); combine folds the symbol back into the HImode store, deletes insn 109, and emits `(insn 150 (use (reg:SI 98)))` carrying `REG_DEAD (reg:SI 98)` right after `jump_insn 99`. The relevant GCC source is combine.c:10829-10845 (the scan `for (tem = prev_nonnote_insn (i3); ...)` terminates on any non-INSN/CALL_INSN, i.e. a JUMP_INSN as well as a CODE_LABEL) plus reload1.c:2404 alter_reg and mips.c:4444 compute_frame_size.

- [s8] [s8] SCOPE: the only build-surface file touched was src/text1b.c, restored at end of session to its committed `INCLUDE_ASM("asm/funcs", func_80049A2C);` content (git status shows src/ clean). All other writes are under memory/grind/func_80049A2C/ and tmp/grind/func_80049A2C/s8/.

## [s9] Chassis re-measure (rederive modality): honest floor is 12, unchanged.
- mechanism: `sandbox func_80049A2C --disable all` with memory/grind/func_80049A2C/candidate.c applied to src/text1b.c.
- probe: applied the banked candidate verbatim and scored it.
- result: score 12, target_insns 126, build_insns 126, rules_dropped 0. The dispatch brief's "measurement unavailable" is not a chassis change; the ledger floor of 12 is correct on this chassis.
- verdict: CONFIRMED

## [s9] THE ORPHAN-VS-SCHEDULE EXCLUSION: the D_80099D3C fold that creates the phantom slot is the very same two-instruction shortening that destroys target's schedule.
- mechanism: attributed from cc1 `-da` dumps, not guessed. In `.combine` the matching baseline (BASE0) and the folding variant (F3) are IDENTICAL - the arg1 index chain (`andi` -> `sll` -> `addu` -> `sll` [-> `la` -> `addu`]) sits at the same position in both. The divergence is created by sched.c's FIRST, pre-RA scheduling pass. In the baseline the chain is SIX insns long (it still contains `la D_80099D3C` + `addu`) and is the longest path to the first rotation load, so sched1 hoists the whole chain to immediately after the `jal func_8004153C`. arg1's last use (the `andi`) therefore lands BEFORE the call-return copy - which is exactly target's order (80049A98 `andi $a0,$s1,0x1` ... 80049AD0 `addu $s1,$v0,$zero`) and is what lets $s1 carry arg1 and then be REUSED for the vehicle pointer: five callee-saved registers, `.mask 0x800f0000`. When combine folds the symbol_ref into the load (`lh D_80099D3C($x)`), the chain drops to FOUR insns, its INSN_PRIORITY falls below the call-return copy, sched1 stops hoisting it, the vehicle copy is emitted first, the arg1 and vehicle live ranges overlap, and global.c allocates a SIXTH callee-saved register ($s4).
- probe: tmp/grind/func_80049A2C/s9/da.sh built full `-da` dumps for BASE0 and F3; the ordered insn tags for the window around the D_80099D3C references were extracted from `.combine` and `.sched` and recorded verbatim in tmp/grind/func_80049A2C/s9/sched_attribution.txt.
- result: `.combine` order identical between the two; `.sched` order differs exactly as described. Every folding variant measured this session (F2/R3/F3/R5/R17/M4/M5) reports `regs= 6/0`; every non-folding variant reports `vars= 0`. In nine sessions no variant has ever reported `vars= 8` together with `regs= 5/0`.
- verdict: CONFIRMED (this is the mechanism behind s8's unexplained "sixth register", and it turns G1 from an open sweep into a closed contradiction)

## [s9] G1 KILLED: no ordering, naming or re-basing of the D_80099D3C fold recovers the five-register allocation.
- mechanism: G1's premise was that some spelling of "two distinct variable index expressions on D_80099D3C" exists whose second index does not force a sixth callee-saved register. Per the exclusion above, the sixth register is not caused by the second INDEX - it is caused by the FOLD, which is the necessary condition for the orphan. The chain length IS the instruction count, so a fold that removes two insns from the chain cannot be compensated without emitting those two insns back somewhere else.
- probe: seven new spellings measured through the real cpp + instrumented cc1 on the full TU (tmp/grind/func_80049A2C/s9/run.sh): R3 (unnamed index, fold on read 0), F3 (fold on reads 0 AND 1, pointer re-based at `D_80099D3C+4`), R5 (index computed before the call), R17 (index named immediately after the call), R18 (pointer set up BEFORE the folded read), R19 (pointer setup hoisted above the obj header stores), R16 (pointer for the three mults, subscripts for the three tail halfwords).
- result: R3 vars=8/regs=6 (108 insns; sandbox 50, build_insns 129); F3 vars=8/regs=6 (107 insns - the SAME count as the matching baseline, and still six saves); R5 vars=8/regs=6 (109); R17 vars=8/regs=6 (108); R18 vars=0/regs=6 (109); R19 vars=0/regs=6 (109); R16 vars=0/regs=5 (107). R18/R19 also establish an ordering law worth keeping: the folded access must come FIRST in source order, because once the walking pointer is live CSE rewrites any later subscript against it and the second symbol_ref use vanishes entirely.
- verdict: KILLED (frontier G1)

## [s9] G2 KILLED: D_800EF980 and D_80099CC8 cannot host the orphan - each is touched at exactly ONE index.
- mechanism: the s8 trigger law requires two or more accesses at two or more DISTINCT variable index expressions on the same symbol. This function reads D_800EF980 at the single index `temp_v1` (three times) and D_80099CC8 at the single address `arg0*2 + arg2` (once). Any respelling of a same-index access is merged by CSE into one address pseudo, which is then multi-use and therefore never folded, so combine never deletes a def and no REG_DEAD note is stranded. This is a property of the function's semantics, not of the spelling, so no further spelling search on these two symbols is warranted.
- probe: S1 (guard read spelled `D_800EF980[temp_v1]`, `p_anim` formed afterwards), S2 (the LAST anim read spelled as a subscript, guard via `p_anim`), S3 (the `(u8 *) D_800EF980` symbol materialisation hoisted ABOVE the `temp_v1 == 0xFF` early return so the def sits on the far side of a JUMP_INSN from its consumer). These join s8's B/C/I on the same symbol and K/O on D_80099CC8.
- result: all three report `.frame $sp,40 # vars= 0, regs= 5/0` at 107 insns - byte-identical to the matching baseline. S3 in particular refines the trigger law: crossing a JUMP_INSN is NECESSARY for the REG_DEAD note to strand but is useless without a fold, and a multi-use address pseudo is never folded.
- verdict: KILLED (frontier G2 - the "does the function semantically touch a second slot" question is moot; the blocker is CSE and use-count, not semantics)

## [s9] Fresh m2c re-derivation produces the SAME structural shape as the banked candidate, and yields two prototype claims - one free, one refuted.
- mechanism: rederive modality. `python3 tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax -f func_80049A2C asm/funcs/func_80049A2C.s`.
- probe: read the output, compared it statement-by-statement with the banked candidate, and turned its two liveness-derived prototype guesses into measured variants M1/M2/M3/M4/M5.
- result: m2c reproduces the candidate's control flow, its locals, and the walking-pointer rotation reads - it emits them as a deferred `temp_v1_N = temp_v1_(N-1) + 2` chain that matches target's placement of `addiu $v1,$v1,0x2` between the `mult` and the `mflo`. There is NO structurally different decomposition to be had from a fresh decompile; the banked candidate IS the natural decompilation. Two by-products: (a) m2c reads $a0 as live at the `jal func_80052C10` (80049A88), i.e. InitFadePanel may take the D_80099CC8 element pointer. Measured as M1: passing it costs ZERO instructions (107 insns, byte-neutral), so the prototype is plausible and free - but it does not change the frame. (b) m2c reads $a1 as live at the `jal func_800417D0`, suggesting a second argument. Measured as M2: +2 instructions (109), so func_800417D0 does NOT take `a1_val` as a second parameter. M4/M5 (m2c prototypes combined with the rotation fold) still report regs=6/0, so the extra liveness does not change sched1's decision.
- verdict: CONFIRMED (m2c re-derivation exhausted)

## [s9] The two-dimensional re-spelling of the rotation table is dead.
- mechanism: rederive - target's `sll $v1,$a0,1 / addu $v1,$v1,$a0 / sll $v1,$v1,2` index is exactly `k * sizeof(s16[6])`, so an `s16 [2][6]` declaration was a live hypothesis for the original type.
- probe: R2D - `s16 (*rot)[6] = (s16 (*)[6]) D_80099D3C;` with `rot[krow][0]` and `&rot[krow][1]`.
- result: `.frame $sp,40 # vars= 0, regs= 6/0`, 105 insns. GCC canonicalises the row arithmetic to the same flat address, so no second symbol_ref use survives - and the reshape costs the sixth register anyway.
- verdict: KILLED

## [s9] The in-TU orphan census: every OTHER spill_new producer in text1b.c is LOOP-shaped.
- mechanism: `grep ctx=spill_new` over the BB2_FRAME_DEBUG stderr of the whole text1b.c compilation lists every function in this TU that reaches reload1.c:2404 alter_reg on an unallocated pseudo.
- probe: tmp/grind/func_80049A2C/s9/v_BASE0/t.err.
- result: 20 records over six functions - func_8004876C (do-while pixel loop), func_800493E4 (the known two-index precedent, also loop-free but with a genuine second index), func_8004954C (a for-loop accumulator with NO global array anywhere in its body), func_80049584, func_80060E38 (9 slots), func_80074B18 (5), func_800770B8 (2). func_8004954C is the important datum: it proves the orphan does not require a global array at all - a loop back-edge alone supplies the JUMP_INSN that strands the note. func_80049A2C has no loop, which is why its only available carrier was the two-index global-array fold that this session has now closed on all three of its symbols.
- verdict: CONFIRMED

- [s9] SCOPE: the only build-surface file touched was src/text1b.c, restored at the end of the session to its committed `INCLUDE_ASM("asm/funcs", func_80049A2C);` content. All other writes are under memory/grind/func_80049A2C/ and tmp/grind/func_80049A2C/s9/.

- [s9] Chassis re-measured this session: sandbox func_80049A2C --disable all = 12, target_insns 126, build_insns 126, rules_dropped 0, with the banked candidate applied. The 12 differing instructions remain exactly the frame delta (1 prologue adjust + 5 saves + 5 restores + 1 epilogue adjust; our frame 0x28, target's 0x30).

- [s9] PASS ATTRIBUTION (cc1 -da, not a guess): BASE0 and the folding variant F3 are IDENTICAL in .combine - the arg1 index chain sits in the same position in both. The divergence is created by sched.c's FIRST (pre-RA) scheduling pass. Raw ordered insn tags for both dumps are recorded in tmp/grind/func_80049A2C/s9/sched_attribution.txt.

- [s9] THE EXCLUSION LAW: on D_80099D3C, producing the combine orphan and keeping target's instruction stream are mutually exclusive. The orphan requires combine to fold the symbol_ref into the load; that fold removes `la D_80099D3C` + `addu` from the arg1 index chain, shortening it from six insns to four; sched1 then stops hoisting the chain to just after the jal; arg1's last use lands after the call-return copy; the arg1 and vehicle live ranges overlap; global.c allocates a sixth callee-saved register. Target saves exactly five (.mask 0x800f0000) and REUSES $s1 for arg1 then vehicle. Chain length IS instruction count, so the shortening cannot be compensated without emitting the two instructions back.

- [s9] Seven new D_80099D3C spellings measured: R3 vars=8/regs=6/108 insns (sandbox 50, build_insns 129), F3 vars=8/regs=6/107 insns (same count as the matching baseline), R5 vars=8/regs=6/109, R17 vars=8/regs=6/108, R18 vars=0/regs=6/109, R19 vars=0/regs=6/109, R16 vars=0/regs=5/107. Across nine sessions no variant has ever produced vars=8 together with regs=5/0.

- [s9] ORDERING LAW (new, from R18/R19): the folded access must come FIRST in source order. If the walking pointer &D_80099D3C[kidx+1] is set up before the folded read, CSE rewrites the folded read against the live pointer and the second symbol_ref use - and therefore the orphan - disappears entirely.

- [s9] D_800EF980 is closed: the function reads it at exactly ONE index (temp_v1, three times). S1 (guard read subscripted), S2 (last read subscripted) and S3 (symbol materialisation hoisted above the temp_v1==0xFF early return, so the def and consumer straddle a JUMP_INSN) all report vars=0, regs=5/0, 107 insns - byte-identical to the matching baseline. CSE merges the same-index accesses into one multi-use address pseudo, which combine never folds.

- [s9] D_80099CC8 is closed for the same reason: a single access at a single address (arg0*2 + arg2), joined by s8's K/O measurements.

- [s9] TRIGGER LAW REFINED: crossing a JUMP_INSN is NECESSARY for the REG_DEAD note to strand, but is nowhere near sufficient - S3 crosses one and still yields vars=0. The fold (a single-use pseudo whose def combine deletes) is the operative condition.

- [s9] Fresh m2c re-derivation reproduces the banked candidate exactly: same control flow, same locals, and the same walking-pointer rotation reads emitted as a deferred temp_v1_N = temp_v1_(N-1) + 2 chain matching target's addiu $v1,$v1,0x2 placement between the mult and the mflo. The banked candidate IS the natural decompilation; no alternative decomposition exists at the source level.

- [s9] PROTOTYPE FINDINGS (free for whoever finally lands this function): passing the D_80099CC8 element pointer to InitFadePanel/func_80052C10 - which m2c infers from $a0 still being live at 80049A88 - costs ZERO instructions (M1: 107 insns, byte-neutral), so that prototype is plausible and free. Passing a1_val to func_800417D0 as a second argument costs +2 instructions (M2: 109), so func_800417D0 does NOT take it.

- [s9] The rotation table is NOT usefully spelled as s16 [2][6]: R2D gives vars=0, regs=6/0, 105 insns - GCC canonicalises row arithmetic to the same flat address.

- [s9] IN-TU ORPHAN CENSUS: grep ctx=spill_new over the BB2_FRAME_DEBUG stderr of the whole text1b.c compile gives 20 alter_reg records over six functions - func_8004876C, func_800493E4, func_8004954C, func_80049584, func_80060E38 (9 slots), func_80074B18 (5), func_800770B8 (2). Every one except func_800493E4 is LOOP-shaped, and func_8004954C reaches reload1.c alter_reg with NO global array in its body at all - a loop back-edge alone supplies the JUMP_INSN that strands the note. func_80049A2C has no loop, which is why its only array-based carrier was the one this session closed.

- [s9] SCOPE: the only build-surface file touched was src/text1b.c, restored at end of session to its committed INCLUDE_ASM("asm/funcs", func_80049A2C); content (git status src/ clean). All other writes are under memory/grind/func_80049A2C/ and tmp/grind/func_80049A2C/s9/.

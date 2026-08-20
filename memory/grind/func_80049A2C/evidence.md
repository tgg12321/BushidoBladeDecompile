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

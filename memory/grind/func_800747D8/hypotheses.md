# Hypothesis ledger — func_800747D8

## Session 1 (recon)

### H1 — declaration fix for D_8009BD20 / D_800A35D0 (DATA MODEL signals)
**Statement:** the candidate's existing declarations
(`u8 D_8009BD20[][2]`, `s16 D_800A35D0` scalar) already match the object
model evidenced by other functions in this TU; no declaration change is
needed to make further progress.
**Mechanism:** n/a — this is an object-model check, not a codegen lever.
**Probe:** cross-checked `D_8009BD20` indexing against
`asm/funcs/func_80074488.s:150-160` (row<<1 stride-2 pair access, matches
`[][2]`); checked every `D_800A35D0` reference across the TU (this
function + func_80075F80 + func_800768DC + two already-COMPLETED-C call
sites) — every one takes only its address, so scalar-vs-array is
address-identical codegen.
**Result:** MATCHES for D_8009BD20 (measured: score 6 achieved with this
declaration, no residual hunk touches this access); D_800A35D0 measured
moot (no possible codegen difference from a size change in this TU).
**Verdict:** CONFIRMED
**kill_scope:** n/a (not a kill)
**measured_on:** candidate.c applied verbatim (minus the D_800A35DC type
fix below) to src/text1b.c, `sandbox --disable all` → score 6/208/208.

### H2 — D_800A35DC must be `s8`, not `u8`
**Statement:** the candidate's `extern u8 D_800A35DC;` conflicts with the
type already committed for this symbol elsewhere in the same TU
(`extern s8 D_800A35DC;`, at the already-COMPLETED-C body around
src/text1b.c:8974/9120) and must be corrected to `s8`.
**Mechanism:** plain C declaration-conflict (compiler error), not a
codegen question — no ambiguity.
**Probe:** applied candidate.c verbatim; cc1 build failed
(`conflicting types for D_800A35DC`); grepped every declaration of the
symbol in the TU; all other sites use `s8`.
**Result:** fixed to `s8`; build succeeds, reproduces the banked floor 6
exactly (score 6, build_insns 208 == target_insns 208).
**Verdict:** CONFIRMED
**kill_scope:** n/a (not a kill; a correction, not a probe)
**measured_on:** src/text1b.c with candidate.c applied + this one-line fix,
`sandbox --disable all` → score 6/208/208.

### H3 — selection_sound block: two-named-locals if/else (`flag`/`sound` split)
**Statement:** splitting the `field64` test value and the call-arg result
value into two distinct C locals (matching the apparent v0-vs-a0 register
split visible in target's asm) closes or narrows hunks 17-19.
**Mechanism:** value-flow structure (test value vs. result value are
logically distinct quantities) — ordinary C, not a GCC-internals lever.
**Probe:** `s32 flag = field64; s32 sound; if (flag==0) sound=4; else
sound=0;` in place of baseline's `s32 sound=field64; if(sound==0)
sound+=4; else sound-=sound;`. `sandbox --disable all`.
**Result:** score 10, build_insns 205 (3 insns SHORT of target 208) — worse
on both axes than baseline.
**Verdict:** KILLED
**kill_scope:** instance
**measured_on:** this chassis (candidate.c + H2 fix), no FAKE constructs,
sandbox --disable all.
Banked: `rejected/selection_sound-two-var-split.c`.

### H4 — selection_sound block: default-then-override spelling
**Statement:** initializing the result to the delay-slot literal (4) and
overriding it in the branch (mirroring the asm's literal control-flow
order instead of H3's data-flow order) closes or narrows hunks 17-19.
**Mechanism:** same as H3 — control-flow-literal C structure.
**Probe:** `s32 flag = field64; s32 sound = 4; if (flag != 0) sound = 0;`.
**Result:** score 10, build_insns 205 — identical to H3.
**Verdict:** KILLED
**kill_scope:** instance
**measured_on:** this chassis, no FAKE constructs, sandbox --disable all.
Banked: `rejected/selection_sound-default-then-override.c`.

### H5 — selection_sound block: single-shot, no intermediate `flag` local
**Statement:** removing the extra `flag` local entirely and testing
`MENU_800747D8->field64` directly in the `if` condition closes or narrows
hunks 17-19.
**Mechanism:** same as H3/H4 — minimal-local-count C structure.
**Probe:** `s32 sound; if (MENU_800747D8->field64==0) sound=4; else
sound=0;`.
**Result:** score 10, build_insns 205 — identical to H3 and H4.
**Verdict:** KILLED
**kill_scope:** instance
**measured_on:** this chassis, no FAKE constructs, sandbox --disable all.
Banked: `rejected/selection_sound-single-shot-no-intermediate.c`.

## Live frontier (for next session)

1. **Hunk 9 — state-switch jtbl table-offset residual (operand-only).**
   Target `lw v0,0(at)` vs ours `lw v0,24(at)` at the `field3C`-driven
   5-case dispatch. Not yet attacked this session. Next step: read the
   `.combine`/`.jump2` dumps (`pwsh tools/grinder/dump.ps1 func_800747D8`)
   for this region — is GCC emitting its own jump table with a different
   entry count/base than target's `jtbl_80015A0C`? Compare against the
   sibling func_8006B578's rodata-split residual (jtbl_80015988) for
   whether this is the SAME class of issue (a separate-TU jump-table
   split needed) or a genuinely different case-ordering bug.
2. **Hunks 17-19 — selection_sound block (source-level + operand-only,
   3 KILLED spellings).** All 3 natural if/else respellings (H3/H4/H5)
   collapse to an identical wrong shape (score 10, build_insns 205)
   regardless of local-variable count or branch-literal placement, while
   baseline's `sound -= sound;` self-subtract reaches build_insns == 208
   (score 6, the smaller residual). Read `.combine`/`.cse` dumps for this
   block before hand-writing a 4th spelling — understand WHY the
   self-subtract avoids whatever fold collapses the natural forms by 3
   insns. Also flag `sound -= sound` itself for a cheat-smell review once
   the residual narrows further (see evidence.md session-1 entry) — it is
   not what a human would write from a spec, though it currently measures
   as the best-known honest form and was NOT changed this session.
3. **26 not-scored hunks** — masked branch/jump-target address artifacts.
   Correctly not chased (per the diff tool's own classification); no
   action needed unless a future full-build (`verify-oracle`) check
   reveals they matter after all (they shouldn't — same class as every
   other masked hunk in this project).

## [s1] The candidate's existing declarations (u8 D_8009BD20[][2], s16 D_800A35D0 scalar) already match the object model evidenced by other functions in this TU; no declaration change is needed.
- mechanism: n/a - object-model check, not a codegen lever
- probe: Cross-checked D_8009BD20 indexing against asm/funcs/func_80074488.s:150-160 (row<<1 stride-2 pair access matches [][2]); checked every D_800A35D0 reference across the TU (this function + func_80075F80 + func_800768DC + two already-COMPLETED-C call sites) - every one takes only the address.
- result: MATCHES for D_8009BD20 (score 6 achieved with this declaration, no residual hunk touches this access); D_800A35D0 is address-identical codegen regardless of scalar-vs-array size in this TU, so no measurable difference is possible here.
- verdict: CONFIRMED

## [s1] D_800A35DC must be declared s8 (not u8) to match the type already committed for this symbol elsewhere in the same TU (the already-COMPLETED-C body near src/text1b.c:8974/9120).
- mechanism: plain C declaration-conflict (compiler error), not a codegen question
- probe: Applied candidate.c verbatim; cc1 build failed with 'conflicting types for D_800A35DC'; grepped every declaration of the symbol in the TU.
- result: Fixed to s8; build succeeds and reproduces the banked floor 6 exactly (score 6, build_insns 208 == target_insns 208).
- verdict: CONFIRMED

## [s1] Splitting the selection_sound block's test value and result value into two named locals (flag = field64; sound = 4-or-0 by branch), matching the apparent v0-vs-a0 register split visible in target's asm, narrows hunks 17-19.
- mechanism: value-flow structure (test value vs result value as distinct C objects) - ordinary C
- probe: s32 flag = field64; s32 sound; if (flag==0) sound=4; else sound=0; in place of baseline's sound-=sound spelling. Measured with sandbox --disable all.
- result: score 10, build_insns 205 (3 insns short of target 208) - worse on both axes than the baseline.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis + H2 s8 fix, no FAKE constructs, sandbox --disable all

## [s1] Initializing the selection_sound result to the delay-slot literal (4) and overriding it in the branch (mirroring the asm's literal control-flow order) narrows hunks 17-19.
- mechanism: same as prior hypothesis - control-flow-literal C structure
- probe: s32 flag = field64; s32 sound = 4; if (flag != 0) sound = 0;
- result: score 10, build_insns 205 - identical to the two-var-split attempt.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis + H2 s8 fix, no FAKE constructs, sandbox --disable all

## [s1] Removing the intermediate flag local entirely and testing MENU_800747D8->field64 directly in the if condition narrows hunks 17-19.
- mechanism: same as prior hypotheses - minimal-local-count C structure
- probe: s32 sound; if (MENU_800747D8->field64==0) sound=4; else sound=0;
- result: score 10, build_insns 205 - identical to both prior attempts.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis + H2 s8 fix, no FAKE constructs, sandbox --disable all

## [s2] Reusing the SAME C variable across load, test, and result for the selection_sound block reaches build_insns == target_insns (208), matching the self-subtract baseline exactly, without the self-subtract cheat-smell.
- mechanism: n/a — ordinary C, ordinary variable reuse (single pseudo carries load+test+result); no GCC-internals coercion, no FAKE construct.
- probe: Replaced `s32 sound = field64; if (sound==0) sound+=4; else sound-=sound;` with `s32 sound = field64; if (sound==0) { sound=4; } else { sound=0; }` in candidate.c, applied to src/text1b.c, `sandbox func_800747D8 --disable all` -> score 6, build_insns 208 == target_insns 208. `--diff` shows the IDENTICAL 30-hunk classification (same 2 source-level + 2 operand-only + 26 not-scored hunks, byte-identical hunk contents) as the self-subtract baseline.
- result: CONFIRMED and banked as the new candidate.c. Strictly better than the s1 baseline: same honest floor, removes the self-subtract construct the s1 ledger flagged for future cheat-smell review (evidence.md s1: "a self-subtract instead of a plain `= 0` is not what a human would write from a spec"). That flagged concern is now resolved — no self-subtract in the candidate.
- verdict: CONFIRMED

## [s2] Introducing a FRESH, single-use, isolated test variable ("flag") separate from the result variable for the selection_sound block triggers a branchless arithmetic fold (`a0 = (flag==0)*4` via sltiu+sll) that eats 3 real instructions vs target's branching form, on this GCC 2.7.2 fork.
- mechanism: combine.c / constant-propagation recognizes the canonical "single-use-dead-after-test flag feeding a two-constant conditional-select into a separate result pseudo" shape and folds it to `sltiu`+`sll` branchless arithmetic instead of keeping cc1's normal branch+literal-set codegen. Confirmed via `--diff` on rejected/selection_sound-default-then-override.c this session: hunks 17-19 show our build's actual emitted insns are `lbu a0,0x64(v0); sltiu a0,a0,1; sll a0,a0,0x2` where target keeps `beqz v0,...` + delay-slot `li a0,4` + fallthrough `move a0,zero` (a real branch, no shift).
- probe: Re-applied rejected/selection_sound-default-then-override.c (s1's spelling, `s32 flag=field64; s32 sound=4; if(flag!=0) sound=0;`) to candidate.c's chassis, ran `sandbox func_800747D8 --disable all --diff` this session (tmp/grind/func_800747D8/s2/diff_defaultoverride.txt): score 10, build_insns 205 (3 short), hunks 17-19 show the exact branchless sltiu+sll fold. This explains (not just re-confirms) all 3 of s1's "natural if/else" rejections plus this session's own re-derivation — all 4 isolated-test-variable spellings collapse to this identical branchless shape regardless of which local holds which literal or which branch is written first.
- result: KILLED (explained). The isolated-test-variable family is dead for this block on this chassis; the same-variable-reuse spelling (see the CONFIRMED entry above) is the correct honest form and is now banked as candidate.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/selection_sound-default-then-override.c spliced onto candidate.c's chassis (s2), src/text1b.c, sandbox --disable all --diff; zero FAKE constructs anywhere in the candidate or the rejected form

## [s2] Hunk 9's jtbl_80015A0C table-offset residual (24 vs 0) is the same masked cross-TU-placement mechanism that sibling func_8006B578's jtbl_80015988 residual was (rotated 2026-09-16, its own s7/s12 evidence) — a GCC-synthesized-ADDR_VEC-vs-hand-transcribed-array placement conflict, not a pure src/text1b.c C-structure lever.
- mechanism: bb2.ld orders text1a_b_pre_rodata.o, text1b.o, text1a_b_mid_rodata.o. jtbl_80015A0C is CURRENTLY a hand-transcribed `const u32[6]` in src/text1a_b_mid_rodata.c:44-52 (not yet migrated to compiler-synthesized, unlike jtbl_80015988 which WAS migrated when func_8006B578 landed as C — see that file's own header comment). Once func_800747D8's real `switch(state)` is the committed C, GCC's own ADDR_VEC for it needs to land in build/src/text1b.o's own .rodata at exactly 0x80015A0C, which requires deleting the hand-transcribed jtbl_80015A0C array from text1a_b_mid_rodata.c in the SAME change — a precedented, already-executed-once (for jtbl_80015988) mechanical step, not a novel lever and not a wall.
- probe: Read src/text1a_b_mid_rodata.c (jtbl_80015A0C at lines 44-52, still hand-transcribed) and its header comment (documents doing this migration for jtbl_80015988 only so far). Read bb2.ld ordering (unchanged, not edited). Cross-referenced sibling func_8006B578's s7/s12 ledger entries, which independently derived and executed the identical migration for jtbl_80015988 with zero bb2.ld edits.
- result: Not attacked this session (out of the func_800747D8/src/text1b.c-only surface — the fix touches src/text1a_b_mid_rodata.c) and not measured — this is a read-only mechanism identification, not a probe with a negative result. Left on the frontier for the session that assembles the final submission: delete the jtbl_80015A0C array from text1a_b_mid_rodata.c in the same change that lands func_800747D8's C body, matching the func_8006B578 precedent exactly.
- verdict: (not scored this session — no measurement taken; see frontier)

## [s2] Reusing the SAME C variable across load, test, and result for the selection_sound block reaches build_insns == target_insns (208), matching the s1 self-subtract baseline exactly, without the self-subtract cheat-smell.
- mechanism: n/a - ordinary C, ordinary variable reuse; no GCC-internals coercion, no FAKE construct
- probe: Replaced `s32 sound = field64; if (sound==0) sound+=4; else sound-=sound;` with `s32 sound = field64; if (sound==0) { sound=4; } else { sound=0; }` in candidate.c, applied to src/text1b.c, sandbox --disable all
- result: score 6, build_insns 208 == target_insns 208; --diff shows the IDENTICAL 30-hunk classification (same 2 source-level + 2 operand-only + 26 not-scored, byte-identical hunk contents) as the self-subtract baseline
- verdict: CONFIRMED

## [s2] Introducing a fresh, single-use, isolated test variable ("flag") separate from the result variable for the selection_sound block triggers a branchless arithmetic fold (a0 = (flag==0)*4 via sltiu+sll) that eats 3 real instructions vs target's branching form, on this chassis.
- mechanism: combine.c / constant-propagation recognizes the canonical single-use-dead-after-test flag feeding a two-constant conditional-select into a separate result pseudo, and folds it to sltiu+sll branchless arithmetic instead of a branch+literal-set; confirmed via --diff on rejected/selection_sound-default-then-override.c, hunks 17-19
- probe: Re-applied rejected/selection_sound-default-then-override.c to candidate.c's chassis, ran sandbox --disable all --diff
- result: score 10, build_insns 205 (3 short); --diff shows the emitted insns are lbu a0,0x64(v0); sltiu a0,a0,1; sll a0,a0,0x2, where target keeps a real beqz branch + delay-slot li a0,4 + fallthrough move a0,zero. Explains (not just re-confirms) all 4 of the isolated-test-variable spellings banked in rejected/ (3 from s1, 1 new this session) as the same branchless-fold collapse.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/selection_sound-default-then-override.c spliced onto candidate.c's chassis (s2), src/text1b.c, sandbox --disable all --diff; zero FAKE constructs anywhere in the candidate or the rejected form

## [s2] Branch source order (sound == 0 written first vs sound != 0 written first) has no effect on the emitted hunks 17-19 for the selection_sound block on this chassis.
- mechanism: GCC normalizes branch polarity independent of source statement order for this shape
- probe: Swapped the if/else arm order in the new same-variable candidate spelling, applied to src/text1b.c, sandbox --disable all --diff
- result: score 6, build_insns 208; hunks 17-19 byte-identical to the non-swapped form (tmp/grind/func_800747D8/s2/diff_swapped.txt)
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis with if/else arms swapped, src/text1b.c, sandbox --disable all --diff; zero FAKE constructs

## Session 3 (permuter)

### H9 — decomp-permuter's `import.py` full-TU import of src/text1b.c is broken for this function; a hand-built minimal-context workspace (candidate.c body + own decls, compiled through the real pipeline) is the working substitute
**Statement:** `python3 tools/decomp-permuter/import.py src/text1b.c asm/funcs/func_800747D8.s` (with or without `--no-prune`) produces a `nonmatchings/func_800747D8/` workspace whose generated `compile.sh` crashes maspsx (`too many values to unpack`) on this TU, both via a pruning artifact (a `.comm` line corrupted by one of the TU's many `_permuter_ignore_line __asm__(...)` canonical-asm placeholder stubs) and via a `.file` directive whose realpath'd filename contains the repo's own space-containing directory name (`Bushido Blade 2 Decompile`), which breaks maspsx's naive `line.split()` `.file`-renumbering logic (`tools/maspsx/maspsx/__init__.py:1079`). The second bug is latent (the real permuter, via `tempfile.NamedTemporaryFile`, writes candidates outside the repo path and never hits it) but the first (the pruning/placeholder corruption) is real and specific to text1b.c's size/asm-block density.
**Mechanism:** import.py's static pruning pass mishandles one of the many whole-function canonical-asm placeholder replacements it performs when scrubbing a huge multi-thousand-line TU down to only what `func_800747D8` needs; the corrupted output collides with maspsx's `.comm` parser. Neither `tools/decomp-permuter/` nor `tools/maspsx/` may be modified from this session's surface (contract: never edit `tools/`) so the fix is procedural, not a tool patch.
**Probe:** Built a minimal, self-contained `base.c` (candidate.c's body plus its own typedefs/externs/prototypes, no `#include`) in `tmp/grind/func_800747D8/s3/perm_ws/`, reusing the `target.o`/`settings.toml` that `import.py`'s first (failed) run had already generated correctly (target.o comes straight from `asm/funcs/func_800747D8.s` via `as`, independent of the base.c bug). Verified this compiles cleanly through the exact real pipeline (cpp | cc1 | prologue_fix | maspsx | multu_pad | as) and disassembles to the expected function body. Launched a real campaign against it via `tools/permuter_campaign.py launch --func func_800747D8 --dir tmp/grind/func_800747D8/s3/perm_ws -j 8` (base_score 265 in the permuter's own unmasked unstripped metric — not comparable to the sandbox's masked score 6).
**Result:** CONFIRMED as a working substitute — base_score computed, campaign ran 21,884 iterations with 5 novel finds, cleanly stopped/harvested (0 orphaned processes). Practical for any future permuter session on this function or (with the same recipe) other text1b.c functions that hit the same import.py failure.
**Verdict:** CONFIRMED

### H10 — a permuter campaign on the minimal-workspace chassis (21,884 iterations, 8 jobs, 5 novel finds, best score 100 of base 265) finds no improvement on the hunks 17-19 selection_sound register-seat/branch-shape residual
**Statement:** none of the 5 novel finds (scores 100/120/130/170/220) improve on candidate.c's honest floor of 6; the best-scoring find (100) is a pure whitespace/parenthesization reformat with zero semantic or codegen difference from candidate.c's own same-variable-reuse spelling, and the others are either dead-store injections (worse, forbidden-family shaped) or a semantically-altered reordering (moves a call inside one branch only — not equivalent code).
**Mechanism:** the permuter's random-mutation passes (perm_add_self_assignment, perm_reorder_stmts, perm_sameline, etc.) explore the same narrow local-spelling space this ledger's s1/s2 sessions already exhausted by hand (isolated test var -> branchless fold per the s2 H-mechanism entry; dead stores are a different, forbidden axis) — no mutation pass in decomp-permuter's catalog expresses the specific v0-vs-a0 register-seat swap or the beqz-vs-bnez+move branch-polarity difference the target actually has, because both are downstream RA/scheduling decisions the permuter's source-level mutations don't directly control.
**Probe:** `tools/permuter_campaign.py launch/wait/harvest --stop` on `tmp/grind/func_800747D8/s3/perm_ws`, label `s3-permuter`, `-j 8`; 4 wait windows totalling 613.8s / 21,884 iterations; telemetry in `metrics/events.jsonl`; finds under `tmp/grind/func_800747D8/s3/perm_ws/output-*`.
**Result:** KILLED (explained, not just re-confirmed) — the local C-spelling search space around the selection_sound block is now doubly exhausted (hand-written s1/s2 forms + permuter's own randomized search), converging on the same conclusion: the same-variable-reuse spelling in candidate.c is the best honest form reachable by C-structure search alone. Separately hand-tested output-170-1's `do { ... } while (0);` wrap of the if/else (a SOTN-sanctioned family per `.claude/rules/do-while-zero-exception.md`) directly on src/text1b.c: **REGRESSED score 6 -> 7** (added a real source-level instruction the target doesn't have) — banked as `rejected/selection_sound-do-while-zero-wrap.c`.
**Verdict:** KILLED
**kill_scope:** instance
**measured_on:** candidate.c chassis (same-variable-reuse baseline unchanged) plus the do-while(0)-wrapped variant, both on src/text1b.c, `sandbox --disable all --diff`; zero FAKE constructs in either form; campaign workspace `tmp/grind/func_800747D8/s3/perm_ws`, 0 orphaned processes at session end.

## [s3] decomp-permuter's import.py full-TU import of src/text1b.c is broken for this function (maspsx crash on a pruning artifact); a hand-built minimal-context workspace (candidate.c body + own decls, compiled through the real pipeline, reusing import.py's correctly-generated target.o) is a working substitute.
- mechanism: import.py's static pruning pass corrupts a `.comm` line while scrubbing one of text1b.c's many whole-function canonical-asm placeholder stubs (`_permuter_ignore_line __asm__(...)`) during the huge-TU minimization pass; unrelated to func_800747D8's own C. A separate latent bug (maspsx's `.file` line handler breaking on a realpath'd filename containing the repo's own space-containing directory name) does not affect real permuter runs since decomp-permuter writes candidates via tempfile.NamedTemporaryFile outside the repo path.
- probe: Built tmp/grind/func_800747D8/s3/perm_ws/base.c as a minimal self-contained C file (candidate.c's function body plus its own typedefs/externs/prototypes, no #include), verified it compiles cleanly through the exact real pipeline (cpp | cc1 | prologue_fix | maspsx | multu_pad | as) and disassembles to the expected function body, then launched a real campaign against it.
- result: CONFIRMED as a working substitute: base_score computed (265, permuter's own unmasked metric), campaign ran 21,884 iterations with 5 novel finds, cleanly stopped/harvested with 0 orphaned processes.
- verdict: CONFIRMED

## [s3] A permuter campaign on the minimal-workspace chassis (21,884 iterations, 8 jobs, 5 novel finds) finds no C-structure improvement on the hunks 17-19 selection_sound register-seat/branch-shape residual, and the sanctioned do-while(0) wrap of the if/else (one of the campaign's finds) regresses the honest floor from 6 to 7 when hand-tested on the real chassis.
- mechanism: The permuter's mutation catalog (perm_add_self_assignment, perm_reorder_stmts, perm_sameline, do-while wraps, etc.) re-explores the same narrow local-spelling space this ledger's s1/s2 sessions already exhausted by hand (isolated test var -> branchless combine.c fold per the s2 mechanism entry); no mutation pass expresses the specific v0-vs-a0 register-seat swap or beqz-vs-bnez+move branch-polarity difference the target has, since both are downstream RA/scheduling decisions untouched by source-level C mutation of this already-minimal block. The do-while(0) wrap left a genuine extra branch/jump on this already-branching if/else instead of folding away.
- probe: tools/permuter_campaign.py launch/wait/harvest --stop on tmp/grind/func_800747D8/s3/perm_ws, label s3-permuter, -j 8; 4 wait windows totalling 613.8s; telemetry in metrics/events.jsonl; finds under tmp/grind/func_800747D8/s3/perm_ws/output-*. Then hand-applied output-170-1's do-while(0) wrap verbatim to src/text1b.c and measured with sandbox --disable all --diff.
- result: Best permuter find (score 100 of base 265) is a pure whitespace/parenthesization reformat with zero codegen difference from candidate.c. Other finds are either dead-store injections (worse, forbidden-family-shaped) or a semantically-altered call reordering. The do-while(0) wrap measured score 6 -> 7 (regression, one extra real instruction) -- banked as rejected/selection_sound-do-while-zero-wrap.c. Floor confirmed unchanged at 6 after reverting.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis (same-variable-reuse baseline, unchanged) plus the do-while(0)-wrapped variant, both applied to src/text1b.c; zero FAKE constructs in either form; sandbox --disable all --diff; campaign workspace tmp/grind/func_800747D8/s3/perm_ws, 0 orphaned processes at session end.

## s4 (enumerate modality, 2026-09-20)

- statement: Reordering the field67 block's (`case 1: if ((ret & 0xFF) != 0) { ... }`, currently byte-matching, zero diff vs target) four plain-assign statements (`menu = (S_800747D8 *)D_800A36A0;` x2, `work = (S_800747D8 *)D_800A36A0;`, `row = work->field67;`) into any of the 6 valid def-before-use orderings that hoist them ahead of the interleaved `menu->field67 += 1;` / `menu->field67 &= 1;` compound-assigns is worse than the current interleaved spelling, on this chassis.
- mechanism: hoisting the two identical `menu = (S_800747D8 *)D_800A36A0;` reloads together (no longer separated by an intervening compound-assign statement) lets GCC's local-CSE (cse.c) merge them into a single load, changing register/instruction count (208 -> 206 build_insns) even though the reload was already redundant value-wise; the interleaved placement in the current candidate.c defeats that merge and is required to keep the block at its current (already correct) byte count.
- probe: `tools/spelling_enum.py --candidate tmp/grind/func_800747D8/s4/enum_input.c --out tmp/grind/func_800747D8/s4/enum --no-swaps` (6 distinct spellings from 4 assigns, 0 named decls) then `tools/sweep_variants.py --func func_800747D8 --file text1b --variants tmp/grind/func_800747D8/s4/enum --json`.
- result: baseline 6; all 6 variants scored 24 (build_insns 206, worse in both score and instruction count). Zero improvement, all strictly worse. This block was NOT part of the open residual (it already matches target) — this was a control run validating the new enum+sweep tool pairing works correctly and confirming the current candidate.c's exact statement order in this block is load-bearing.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis with the field67 block's statement order permuted (6/6 valid orderings tested), src/text1b.c, sandbox --disable all; zero FAKE constructs.

- statement: The actual open residual (hunks 17-19, the `selection_sound` block: `s32 sound = MENU_800747D8->field64; if (sound == 0) { sound = 4; } else { sound = 0; } func_8005C650(sound, 0x7F, 0x7F);`) cannot be usefully swept by `tools/spelling_enum.py` because the tool's statement model has no concept of if/else nesting — it always sorts every decl/assign statement ahead of every "anchor" line (which is what an un-parsed `if (...) {` / `} else {` / `}` becomes), so any region containing per-arm assignments inside a conditional gets its assigns hoisted OUT of the branches, changing behavior (or, when the tool's inlining axis drops the region's only named local while leaving an assign to that now-undeclared name, producing invalid C).
- mechanism: `spelling_enum.py`'s `orderings()` (spelling_enum.py:128-159) unconditionally appends all collected `anchors` after all permuted decls/assigns — a structural limitation of the tool for control-flow-bearing regions, not a GCC codegen mechanism.
- probe: `tools/spelling_enum.py --candidate tmp/grind/func_800747D8/s4/enum_input.c --out tmp/grind/func_800747D8/s4/enum_ss --no-swaps` (region = the selection_sound block's decl + if/else + call) -> 4 variants; manually inspected all 4 (v0-v3) rather than blind-sweeping them, since v0/v1 hoist `sound = 4;`/`sound = 0;` out of the if/else (semantically wrong — the diamond becomes a no-op) and v2/v3 drop the `s32 sound = ...;` declaration while keeping the now-orphaned `sound = 4;`/`sound = 0;` assigns (invalid C — confirmed by an accidental partial-apply during `sweep_variants.py` producing `text1b.c:8475: 'sound' undeclared` when the tool's restore-on-finally hit an unrelated transient `OSError: [Errno 22] Invalid argument` mid-run on the WSL/NTFS 9p mount; manually reverted src/text1b.c to candidate.c's known-good text and re-verified floor 6 before continuing).
- probe continued: no sandbox measurement was taken on v0-v3 since manual inspection already showed 2/4 semantically wrong and 2/4 non-compiling — sweeping them would only reproduce the same non-informative result the tool already structurally guarantees for this region shape.
- result: no new evidence for or against any hunk-17-19 respelling. The s1 (3 kills), s2 (2 kills), s3 (permuter 21,884 iters + do-while(0) hand-test) evidence remains the complete search record for this block. The `enumerate` modality's tool is confirmed NOT applicable to if/else-diamond-shaped residuals without a region-shape restriction the tool does not currently support (a future improvement would need to treat each branch arm as its own flat sub-region, out of scope for a src/text1b.c-only session).
- verdict: KILLED
- kill_scope: instance
- measured_on: N/A (no compiling/semantically-valid variant existed to measure; the negative result is about tool applicability, not a measured chassis form) — recorded as an instance kill of "spelling_enum.py sweep of the selection_sound if/else region" specifically, not of any respelling within it.

## [s4] Reordering the field67 block's (switch(state) case 1, currently byte-matching, zero diff vs target) four plain-assign statements into any of the 6 valid def-before-use orderings that separate the two identical 'menu = (S_800747D8 *)D_800A36A0;' reloads from the intervening 'menu->field67 += 1;' / '&= 1;' compound-assigns is worse than the current interleaved spelling.
- mechanism: GCC's local CSE (cse.c) merges the two identical menu reloads into one once they are no longer separated by an intervening statement, changing build_insns from 208 to 206 and the sandbox score from 6 to 24 -- the interleaved placement in candidate.c defeats that merge and is required to keep this block at its current (already correct) instruction count.
- probe: tools/spelling_enum.py --candidate tmp/grind/func_800747D8/s4/enum_input.c --out tmp/grind/func_800747D8/s4/enum --no-swaps (6 spellings) then tools/sweep_variants.py --func func_800747D8 --file text1b --variants tmp/grind/func_800747D8/s4/enum --json
- result: baseline score 6; all 6 generated orderings scored 24 with build_insns 206 (strictly worse on both score and instruction count, zero improving variants).
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis with the field67 block's 4 plain-assign statements permuted through all 6 valid def-before-use orderings, src/text1b.c, sandbox --disable all, zero FAKE constructs

## [s4] tools/spelling_enum.py's flat statement model (orderings() unconditionally sorts every decl/assign ahead of every anchor line) cannot usefully enumerate the selection_sound if/else diamond (hunks 17-19's open residual) because the region's assigns live inside conditional branch arms -- hoisting them out changes program behavior or, when the tool's inlining axis also drops the block's only named local, produces C that fails to compile.
- mechanism: spelling_enum.py:128-159 orderings() structurally appends the collected anchor list after all permuted decl/assign statements regardless of their original position relative to control-flow anchors -- a tool limitation, not a GCC codegen mechanism.
- probe: tools/spelling_enum.py --candidate tmp/grind/func_800747D8/s4/enum_input.c --out tmp/grind/func_800747D8/s4/enum_ss --no-swaps on the selection_sound decl+if/else+call region -> 4 variants, manually inspected (not blind-swept).
- result: 2 of 4 variants (v0, v1) hoist 'sound = 4;'/'sound = 0;' out of the if/else, leaving an empty-armed diamond that changes behavior; 2 of 4 (v2, v3) drop the 's32 sound = ...;' declaration entirely while keeping the now-orphaned assigns, which is invalid C (confirmed via an accidental partial-apply during sweep_variants.py's restore step hitting a transient WSL/NTFS OSError mid-run: build failed with "text1b.c:8475: 'sound' undeclared"; src/text1b.c was manually reverted to candidate.c's known-good text and floor 6 was re-verified before continuing). No sandbox measurement of v0-v3 was taken since manual inspection already showed all 4 non-informative for this residual.
- verdict: CONFIRMED

## s5 (synthesis modality, 2026-09-20)

### H11 - the selection_sound call duplicated into both arms reproduces target's register seat
- statement: Writing the selection_sound block as two separate calls, one per arm (`if (MENU_800747D8->field64 == 0) { func_8005C650(4, 0x7F, 0x7F); } else { func_8005C650(0, 0x7F, 0x7F); }`), makes the field64 load land in $v0 with $a0 materialized separately in each arm - target's exact shape - but costs 3 surplus instructions because the two arms' identical argument-setup tails do not cross-jump-merge, measuring score 7 / build_insns 211 on this chassis.
- mechanism: a full argument setup plus call is five insns, so the arm after the conditional jump is not the single insn that GCC 2.7.2's jump.c store-flag transform requires (tools/gcc-2.7.2/jump.c:1066) - the branchless fold is blocked and the real branch with per-arm $a0 survives. The surplus comes from jump2 cross-jumping: tools/gcc-2.7.2/jump.c:2005 tries find_cross_jump against the code before the jump's own label with minimum=1 first, succeeds on the one shared `jal func_8005C650` the field67 path falls into, and therefore never reaches the sibling-jump pairing at tools/gcc-2.7.2/jump.c:2011-2021 that would merge the two arms' `li a1,127 / j / li a2,127` tails with each other.
- probe: four spellings swept with tools/sweep_variants.py (plain if/else, polarity-swapped, and both early-goto orderings) - tmp/grind/func_800747D8/s5/var/; the winning form's sandbox object disassembled with mipsel-linux-gnu-objdump and compared insn-by-insn against asm/funcs/func_800747D8.s:108-118.
- result: all four measure score 7, build_insns 211 (baseline 6 / 208). The register seat is CORRECT in all four - hunk 17 (`lbu a0` vs target `lbu v0`) disappears from `--diff` entirely and the scored residual becomes 3 source-level hunks that are purely the duplicated tail. Banked: rejected/selection_sound-duplicated-call-into-arms.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c floor-6 chassis, src/text1b.c, sandbox --disable all; zero FAKE constructs in any of the four spellings.

### H12 - every single-call two-constant select folds branchless (store-flag transform)
- statement: Every single-call spelling of the selection_sound block that selects between the constants 4 and 0 into a pseudo distinct from the loaded test value is folded branchless to `lbu / sltiu / sll` by GCC 2.7.2's jump.c store-flag transform and measures score 10 / build_insns 205, because the transform's admitting disjunct fires whenever either selected constant is zero and one of {4, 0} always is.
- mechanism: tools/gcc-2.7.2/jump.c:1166-1197, the block commented "That didn't work, try a store-flag insn", admitted by the disjunct at tools/gcc-2.7.2/jump.c:1190-1191 - `(reversep = 0, temp2 == const0_rtx) || (temp3 == const0_rtx && (reversep = can_reverse_comparison_p (temp4, insn)))`. Reversing the arms only flips which of the two disjuncts fires, so branch polarity and which arm holds which literal are both irrelevant.
- probe: twelve spellings measured identically at 10/205 - s1 H3/H4/H5 and the s2 mechanism form (already banked), plus nine NEW this session swept with tools/sweep_variants.py: `sound = 0; if (field64 == 0) sound = 4;`, a ternary in the call argument, a `switch (field64)` with case 0 / default arms, `sound = 4; if (field64 != 0) sound = 0;` with the field tested directly and no intermediate local, a `u8` (QImode) result variable, and reuse of the already-live locals `state` (three orderings) and `ret`. Variant sources: tmp/grind/func_800747D8/s5/var/, var2/, var3/.
- result: all twelve identical at score 10, build_insns 205 (3 short of target's 208). The QImode and switch spellings were specifically chosen to try to fail the transform's mode and control-flow-shape requirements; both fold anyway. Banked: rejected/selection_sound-distinct-pseudo-store-flag-fold-class.c.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/jump.c:1190
- measured_on: candidate.c floor-6 chassis, src/text1b.c, sandbox --disable all; zero FAKE constructs in any of the twelve spellings.

### H13 - declaration-shape levers on the surviving same-variable chassis are inert
- statement: On the floor-6 same-variable chassis, introducing a pointer local for the menu base (`S_800747D8 *m = MENU_800747D8; s32 sound = m->field64;`) or hoisting `s32 sound;` out of the inner block into the function-scope declaration list leaves the score and the instruction count unchanged at 6 / 208.
- mechanism: neither change alters the def-use chain that decides the hunk-17 seat - the loaded value still flows into the same pseudo that becomes the call argument, so local-alloc still seats it in $a0.
- probe: tmp/grind/func_800747D8/s5/var2/b10_pointer_local.c and b5_fnscope_decl.c, swept with tools/sweep_variants.py against the floor-6 baseline.
- result: both score 6, build_insns 208 - exact ties with the baseline, no improvement and no regression. Recorded as neutral ties rather than banked as rejected forms; a future session should not spend a measurement on either again.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c floor-6 chassis, src/text1b.c, sandbox --disable all; zero FAKE constructs.

## Live frontier (reset by s5 - supersedes the s1-s4 frontier)

1. **The 211 form's missed cross-jump merge is the shortest path to 208 with
   target's register seat.** The duplicated-call form is already
   byte-correct through the branch and the per-arm `$a0`; it is exactly 3
   insns over because the two arms' identical `li a1,127 / j <jal> /
   li a2,127` tails never merge. tools/gcc-2.7.2/jump.c carries a
   pre-existing, codegen-inert diagnostic knob for precisely this question
   (jump.c:66-89, `BB2_XJUMP_DEBUG=1`, prints
   `XJDBG: DO_CROSS_JUMP jump=N newjpos=N newlpos=N`). NEXT PROBE: compile
   the duplicated-call form with that env var set, capture the trace, and
   determine whether any source-level change to which block physically
   precedes the shared `jal func_8005C650` (e.g. the relative order of the
   selection_sound path and the field67 paths, or whether the field67
   paths' own calls are spelled so they do not present a minimum=1 match)
   flips jump.c:2005's shallow-merge preference and lets the sibling-jump
   pairing at jump.c:2011-2021 run instead.

2. **The only clause of the store-flag transform a C author can still fail
   is jump.c:1066** (the arm after the conditional jump must be exactly one
   insn followed by the join label; `next_nonnote_insn` does NOT skip a
   CODE_LABEL, so a label sitting between the conditional jump and the
   constant store also fails it). NEXT PROBE: enumerate C shapes that put a
   real, non-dead second insn or a genuinely-referenced label between the
   conditional jump and the `sound = 0` store - the most promising being a
   restructuring in which the zero arm is a shared join target for another
   real `func_8005C650(0, 0x7F, 0x7F)` site that already exists in this
   function (the `ret >> 16` case 1/case 2 arms, or the field67 case 1/2
   blocks). Do NOT add a statement that exists only to occupy the slot -
   that is a dead-store/pad cheat and would be a first reach of an
   unsanctioned family.

3. **Hunk 9's jtbl_80015A0C table-offset residual is unchanged and remains
   a final-submission integration step, not a C lever** (s2 established the
   mechanism and the func_8006B578/jtbl_80015988 precedent). NEXT PROBE: at
   final-submission assembly time, delete the hand-transcribed
   `jtbl_80015A0C` array from src/text1a_b_mid_rodata.c in the same change
   that lands func_800747D8's C body, then re-measure with
   `sandbox --disable all --diff`.

## [s5] Writing the selection_sound block as two separate calls, one per arm (if (MENU_800747D8->field64 == 0) { func_8005C650(4, 0x7F, 0x7F); } else { func_8005C650(0, 0x7F, 0x7F); }), makes the field64 load land in $v0 with $a0 materialized separately in each arm - target's exact shape - but measures score 7 / build_insns 211 on this chassis because the two arms' identical argument-setup tails do not cross-jump-merge with each other.
- mechanism: A full argument setup plus call is five insns, so the arm after the conditional jump is not the single insn that GCC 2.7.2's jump.c store-flag transform requires (tools/gcc-2.7.2/jump.c:1066); the branchless fold is blocked and the real branch with per-arm $a0 survives. The 3 surplus insns come from jump2 cross-jumping: tools/gcc-2.7.2/jump.c:2005 tries find_cross_jump against the code before the jump's own label with minimum=1 FIRST, succeeds on the single shared `jal func_8005C650` that the field67 path falls into, and therefore never reaches the sibling-jump pairing at tools/gcc-2.7.2/jump.c:2011-2021 that would merge the two arms' `li a1,127 / j / li a2,127` tails.
- probe: Four spellings swept with tools/sweep_variants.py (plain if/else, polarity-swapped, and both early-goto orderings) - tmp/grind/func_800747D8/s5/var/; the form's sandbox object disassembled with mipsel-linux-gnu-objdump and compared insn-by-insn against asm/funcs/func_800747D8.s:108-118.
- result: All four measure score 7, build_insns 211 (baseline 6 / 208). The register seat is CORRECT in all four: hunk 17 (ours `lbu a0,100(v0)` vs target `lbu v0,0x64(v0)`) disappears from --diff entirely, and the scored residual becomes the 3 duplicated tail instructions. Banked as memory/grind/func_800747D8/rejected/selection_sound-duplicated-call-into-arms.c with the full disassembly comparison in its header. This is the first spelling in five sessions to reproduce target's v0/a0 split.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c floor-6 chassis applied to src/text1b.c, sandbox --disable all; zero FAKE constructs in any of the four spellings

## [s5] Every single-call spelling of the selection_sound block that selects between the constants 4 and 0 into a pseudo distinct from the loaded test value is folded branchless to lbu/sltiu/sll and measures score 10 / build_insns 205, because GCC 2.7.2's jump.c store-flag transform is admitted whenever either selected constant is const0_rtx and one of {4, 0} always is.
- mechanism: tools/gcc-2.7.2/jump.c:1166-1197, the block commented "That didn't work, try a store-flag insn", admitted by the disjunct at tools/gcc-2.7.2/jump.c:1190-1191: `(reversep = 0, temp2 == const0_rtx) || (temp3 == const0_rtx && (reversep = can_reverse_comparison_p (temp4, insn)))`. Swapping the arms only flips which disjunct fires, so branch polarity and which arm holds which literal are both irrelevant.
- probe: Twelve spellings measured identically at 10/205: s1 H3/H4/H5 and the s2 mechanism form (re-measured this session), plus nine new ones swept with tools/sweep_variants.py - `sound = 0; if (field64 == 0) sound = 4;`, a ternary in the call argument, a `switch (field64)` with case 0 / default arms, `sound = 4; if (field64 != 0) sound = 0;` with the field tested directly and no intermediate local, a u8 (QImode) result variable, and reuse of the already-live locals `state` (three orderings) and `ret`. Variant sources: tmp/grind/func_800747D8/s5/var/, var2/, var3/.
- result: All twelve identical at score 10, build_insns 205 (3 short of target's 208). The QImode and switch spellings were chosen specifically to try to fail the transform's mode and control-flow-shape requirements; both fold anyway. Banked as memory/grind/func_800747D8/rejected/selection_sound-distinct-pseudo-store-flag-fold-class.c. The banked floor-6 baseline escapes the transform only because its result variable's prior value is the load (not a CONST_INT) and the arm immediately after the conditional jump assigns 4 (not zero) - which is exactly what forces the load into $a0 and keeps hunk 17 open.
- verdict: KILLED
- kill_scope: class
- measured_on: candidate.c floor-6 chassis applied to src/text1b.c, sandbox --disable all; zero FAKE constructs in any of the twelve spellings
- predicate_cite: tools/gcc-2.7.2/jump.c:1190

## [s5] On the floor-6 same-variable chassis, introducing a pointer local for the menu base (S_800747D8 *m = MENU_800747D8; s32 sound = m->field64;) or hoisting `s32 sound;` out of the inner block into the function-scope declaration list leaves both the score and the instruction count unchanged at 6 / 208.
- mechanism: Neither change alters the def-use chain that decides the hunk-17 seat - the loaded value still flows into the same pseudo that becomes the call argument, so local-alloc still seats it in $a0.
- probe: tmp/grind/func_800747D8/s5/var2/b10_pointer_local.c and b5_fnscope_decl.c, swept with tools/sweep_variants.py against the floor-6 baseline.
- result: Both score 6, build_insns 208 - exact ties with the baseline, no improvement and no regression. Recorded as neutral ties in the ledger rather than banked as rejected forms; a future session should not spend a measurement on either again.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c floor-6 chassis applied to src/text1b.c, sandbox --disable all; zero FAKE constructs

## [s5] The mandated kill re-audit finds every instance kill in this ledger still valid: the ledger has never contained a /* FAKE */ construct, so tools/fake_ablate.py has nothing to ablate, and direct re-measurement of the two closest-to-target kills on the current chassis reproduces their banked figures exactly.
- mechanism: n/a - this is a re-measurement audit, not a codegen lever.
- probe: Grepped candidate.c and all seven banked rejected forms for FAKE annotations (none present), then re-measured s1/H4 (default-then-override) as tmp/grind/func_800747D8/s5/var2/b2_default4_direct_test.c and tmp/grind/func_800747D8/s5/var/v3_zero_default_override.c (both polarities) and s1/H5 (single-shot-no-intermediate, covered by the same class sweep) on the live chassis.
- result: Chassis re-confirmed at the dispatch value (score 6, build_insns 208 == target_insns 208, same 30-hunk classification). Both re-measured kills reproduce score 10 / build_insns 205 exactly. No kill in this ledger was measured with a FAKE carrier occupying a target pseudo, so the func_8002EA24-s8 failure mode the re-audit guards against does not apply here.
- verdict: CONFIRMED

## Session 6 (solver)

### H14 - the duplicated-call form's polarity is a lever
**Statement:** writing the duplicated-call form with the inverted test
(`if (MENU_800747D8->field64 != 0) func_8005C650(0, ...); else
func_8005C650(4, ...);`) changes the emitted selection-sound block relative to
the banked `== 0` / 4-first spelling, because target's `beqz` is the opposite
sense of what the banked form emits.
**Mechanism:** `expand` emits `jumpifnot(cond)`, so the source arm order
decides which arm is the fallthrough and which is the branch target, and
reorg's `steal_delay_list_from_target` fills the delay slot from the target
thread - so the arm order should decide the delay-slot content.
**Probe:** variant A (`tmp/grind/func_800747D8/s6/var_A.c`) measured against
variant C (`var_C.c`, the banked spelling re-measured), both spliced onto
candidate.c's chassis on src/text1b.c, `sandbox --disable all`.
**Result:** variant A score 7 / build_insns 211; variant C score 7 /
build_insns 211. Identical figures. GCC canonicalises the branch sense before
the shape is decided, so the source arm order is inert here.
**Verdict:** KILLED
**kill_scope:** instance
**measured_on:** candidate.c floor-6 chassis applied to src/text1b.c with the
selection_sound block replaced by each spelling; zero FAKE constructs in
either form.
**Banked as:** rejected/selection_sound-dup-call-inverted-polarity.c

### H15 - the last unmeasured polarity of the single-call distinct-local regime
**Statement:** the single-call form with a fresh distinct local and the ZERO
arm written FIRST (`if (field64 != 0) sound = 0; else sound = 4;`) escapes the
205 fold, because its `sound = 4` sits in the branch-target arm rather than in
the fallthrough arm.
**Mechanism:** the store-flag gate at `tools/gcc-2.7.2/jump.c:1042-1066` keys
`temp` on `next_nonnote_insn (insn)`, the FALLTHROUGH arm's insn, so putting
the non-zero constant in the other arm should change which constant becomes
temp2.
**Probe:** variant B (`tmp/grind/func_800747D8/s6/var_B.c`) on the floor-6
chassis; `sandbox --disable all`, then objdump of the sandbox object, then a
full `-da` dump pair via `pwsh tools/grinder/dump.ps1 func_800747D8`.
**Result:** score 10, build_insns 205; objdump shows `sltiu a0,a0,1 /
sll a0,a0,2` at c7a4/c7ac - the 205 regime. The dumps explain why the polarity
cannot matter: `tools/gcc-2.7.2/jump.c:726-860` NORMALISES the two-arm form to
`x = b; if (c) x = a;` before the store-flag gate is ever consulted. In
`text1b.rtl` (function at 69695) the arms are insn 282 (`r140 = 0`) and insn
290 (`r140 = 4`); in `text1b.jump` (function at 66301) insn 290 is gone and a
NEW insn 622 (`r140 = 4`) sits before the conditional jump.
**Verdict:** KILLED
**kill_scope:** instance
**measured_on:** candidate.c floor-6 chassis applied to src/text1b.c; zero
FAKE constructs.
**Banked as:** rejected/selection_sound-distinct-local-zero-arm-first.c

### H16 - the 205 fold's admitting gate is temp3 == CONST_INT, not the const0 disjunct
**Statement:** every single-call spelling of this two-constant select whose
arms are one insn each and whose test does not reference the result variable
folds to the store-flag sequence, because `tools/gcc-2.7.2/jump.c:726-860`
first rewrites the two-arm form into `x = 4; if (c) x = 0;` and the rewritten
`x = 4` then makes `reg_set_last` return CONST_INT 4, which is the only
disjunct of the store-flag gate that MIPS/R3000 can satisfy.
**Mechanism:** `tools/gcc-2.7.2/jump.c:1178-1181` admits the transform on
`GET_CODE (temp3) == CONST_INT`, else on `(BRANCH_COST >= 2 && temp2 ==
const0_rtx) || BRANCH_COST >= 3`. `tools/gcc-2.7.2/config/mips/mips.h:2937`
defines BRANCH_COST as 2 only for PROCESSOR_R4000 / PROCESSOR_R6000 and 1
otherwise, so on our R3000 both fallbacks are dead and `temp3 == CONST_INT` is
the sole door. `temp3` comes from `reg_set_last (temp1, insn)` at
`tools/gcc-2.7.2/jump.c:1061`.
**Probe:** read the gate and mips.h; then confirmed the two-pass sequence in
the `-da` dumps for variant B (text1b.rtl insn 282/290 vs text1b.jump insn 622
plus the emitted `xor / ltu / neg / and 4` at insns 629-635); cross-checked
against the fourteen spellings this ledger has measured in the 205 bucket
(s1 H3/H4/H5, s2, s5 v3/v4/b1/b2/b11/c1/c2/c3/c4, s6 variant B) - all
identical at 10/205.
**Result:** CONFIRMED as the mechanism. Recorded as a CLASS kill of the
single-call two-constant select whose arms are one insn each: the gate is
structural, not spelling-dependent, so no further respelling inside that shape
is worth a measurement. The escape is NOT a different spelling of the select -
it is making reg_set_last unable to return a CONST_INT (H17).
**Verdict:** KILLED
**kill_scope:** class
**measured_on:** fourteen spellings across s1/s2/s5/s6, all on the floor-6
chassis applied to src/text1b.c, all with zero FAKE constructs; plus the
`-da` dump pair for variant B.
**predicate_cite:** tools/gcc-2.7.2/jump.c:1178

### H17 - reg_set_last's label-stop escapes the fold and recovers target's register seat
**Statement:** hoisting the default `sound = 4;` ABOVE the `selection_sound:`
CODE_LABEL makes the store-flag transform fail, and the resulting branchy form
emits target's register seat (`lbu v0,0x64(v0)`) and target's branch
(`beqz v0`) at target's instruction count.
**Mechanism:** `reg_set_last` (`tools/gcc-2.7.2/rtlanal.c:886-888`) scans
backwards from the conditional jump and "Stop[s] when we reach a label",
returning 0. With `sound = 4;` on the far side of the label, temp3 becomes the
REG itself rather than CONST_INT, and the gate at
`tools/gcc-2.7.2/jump.c:1178-1181` fails for BRANCH_COST 1 (H16). The block
then survives as a real branch, and with $a0 carrying the default the loaded
test byte is free to live in $v0.
**Probe:** variant F (`tmp/grind/func_800747D8/s6/var_F.c`): `s32 sound;` moved
to the function-scope declaration list, `sound = 4;` placed at the top of
`case 0:` (above the inner `switch` and above the `selection_sound:` label),
and the block reduced to `if (field64 != 0) sound = 0;` + the single call.
`sandbox --disable all` plus `--diff` (tmp/grind/func_800747D8/s6/diff_F.txt)
plus objdump of the sandbox object.
**Result:** score 8, build_insns 208 (== target_insns). CONFIRMED on the two
bytes that matter: hunk 17 now reads `lbu v0,0x64(v0)`, matching target
exactly, and the branch reads `beqz v0,<join>`, matching target exactly (the
floor-6 baseline emits `lbu a0,0x64(v0)` / `bnez a0,<join>`). The form
REGRESSES the score to 8 for a placement reason only: `sound = 4;` is now in a
different basic block from the branch, so reorg's backward
`fill_simple_delay_slots` cannot reach it - the branch's delay slot gets
`li a1,127` stolen from the target thread, the orphaned `li a0,4` lands in the
switch-dispatch delay slot at c734 (hunk 10, an insn target lacks), and $a0
pinned live across the field65 block pushes that block's `lbu`/`bne`/`addiu`
onto $a1 (hunks 13/14, 3 operand-only diffs the baseline lacks).
**Verdict:** CONFIRMED (as the mechanism and as the best-shape form); KILLED as
a floor improvement.
**kill_scope:** instance
**measured_on:** candidate.c floor-6 chassis applied to src/text1b.c with
`sound` hoisted to function scope; zero FAKE constructs.
**Banked as:** rejected/selection_sound-default-hoisted-above-label.c

### H18 - moving `goto confirm` into the arms gives the duplicated-call arms a private join
**Statement:** writing the duplicated-call form with the `goto confirm;` inside
each arm instead of after the if/else gives the two arms a private join label,
which lets the minimum=1 own-label `find_cross_jump` match the arms against
each other three insns deep instead of matching each separately against
case 2's call.
**Mechanism:** `tools/gcc-2.7.2/jump.c:2005` tries
`find_cross_jump (insn, JUMP_LABEL (insn), 1, ...)` first; the depth of that
match is decided by what physically precedes the jump's own label, so changing
the label the arms jump to changes the match depth.
**Probe:** variant E (`tmp/grind/func_800747D8/s6/var_E.c`) on the floor-6
chassis; `sandbox --disable all`.
**Result:** score 7, build_insns 211 - identical to both if/else spellings of
the duplicated-call form. GCC's jump-to-jump tensioning threads both arm jumps
straight through to `confirm` in the FIRST jump pass, long before jump2's
cross-jumping runs, so the arms never get a private join label from this
source change.
**Verdict:** KILLED
**kill_scope:** instance
**measured_on:** candidate.c floor-6 chassis applied to src/text1b.c; zero
FAKE constructs.
**Banked as:** rejected/selection_sound-dup-call-early-return-spelling.c

### H19 - the duplicated-call form's missed arm-to-arm merge, mechanism proved
**Statement:** the duplicated-call form's 3-insn surplus is the two arms'
`li a1,127 / li a2,127 / jal` tails failing to cross-jump-merge with each
other, and the reason is that the minimum=1 own-label attempt at
`tools/gcc-2.7.2/jump.c:2005` wins first with a one-insn match against case 2's
call and retargets both arm jumps to a label `do_cross_jump` created, after
which the sibling-jump pairing loop cannot see them.
**Mechanism:** `tools/gcc-2.7.2/jump.c:2011-2021` runs the sibling-jump loop
only when the own-label attempt returned `newjpos == 0`, and only when
`INSN_UID (JUMP_LABEL (insn)) < max_uid`; `do_cross_jump` creates its merge
label with a fresh uid above `max_uid` and (unlike the condjump path at
jump.c:1982-1989) never adds the retargeted simplejump to `jump_chain`.
**Probe:** the s5 frontier's named probe - the codegen-inert BB2_XJUMP_DEBUG
knob (`tools/gcc-2.7.2/jump.c:66-89`) run through the INSTRUMENTED cc1
(`tools/gcc-2.7.2/cc1`) on variant A, trace captured to
`tmp/grind/func_800747D8/s6/dumps/xjdbg.txt`, with insn uids resolved from
`tmp/grind/func_800747D8/s6/dumps/text1b.sched2` (function at 106432).
**Result:** CONFIRMED verbatim. Trace lines 1855-1870 show
`enter e1=288 e2=424 min=1 (own-label)` / `MATCH i1=286 i2=418 parallel` /
`result ... => WIN` / `DO_CROSS_JUMP jump=288 newjpos=286 newlpos=418`, and the
same for e1=304. jump_insn 288/304 are the two arms' `goto confirm`;
code_label 424 is `confirm`; call_insn 418 is case 2's call, which falls
through into `confirm` and is therefore the insn preceding that label. After
the retarget both jumps point at uid 660, and the trace contains ZERO
`chain-partner` entries for e1=288 or e1=304 in any `while (changed)`
iteration - while e1=369 (case 1's jump, still on the original label 424)
merges 8 insns deep in the same trace.
**Verdict:** CONFIRMED
**kill_scope:** n/a (not a kill)
**measured_on:** variant A applied to src/text1b.c, instrumented cc1 with
BB2_XJUMP_DEBUG=1; zero FAKE constructs.

### H20 - sibling sweep (dispatch-flagged UNSPENT siblings)
**Statement:** the two siblings the dispatch flagged as never mentioned by this
ledger (`main` in src/ings.c, `func_800692C0` in src/text1b.c) carry a spelling
this residual can transplant.
**Mechanism:** n/a - inheritance check, not a codegen lever.
**Probe:** func_800692C0 is this function's CALLEE and is already COMPLETED-C
on main; checked that candidate.c's call site matches the prototype main ships
(`func_800692C0((u32 *)&sp10, 0, (s16 *)(base + 0x40), &D_800A35D0)` - it
does, and the floor-6 build reproduces target's argument setup with no diff in
that region). `main`/ings.c shares no basic block with this function; it names
func_800747D8 only through the text1b/ings call graph.
**Result:** no transplantable block in either sibling. Recorded so a later
session does not re-sweep them.
**Verdict:** KILLED
**kill_scope:** instance
**measured_on:** candidate.c floor-6 chassis applied to src/text1b.c, compared
against the committed bodies of func_800692C0 and src/ings.c on main; zero FAKE
constructs.

## [s6] The single-call two-constant select folds to the store-flag sequence because tools/gcc-2.7.2/jump.c:726-860 first rewrites the two-arm form into `x = 4; if (c) x = 0;` and the rewritten `x = 4` then makes reg_set_last return CONST_INT 4, which is the only disjunct of the store-flag gate that an R3000 BRANCH_COST of 1 can satisfy; no respelling of the select inside that shape (one-insn arms, test not referencing the result variable) avoids it.
- mechanism: tools/gcc-2.7.2/jump.c:1178-1181 admits the transform on GET_CODE (temp3) == CONST_INT, else on (BRANCH_COST >= 2 && temp2 == const0_rtx) || BRANCH_COST >= 3. tools/gcc-2.7.2/config/mips/mips.h:2937 defines BRANCH_COST as 2 only for PROCESSOR_R4000 / PROCESSOR_R6000 and 1 otherwise, so on our R3000 both fallbacks are dead and temp3 == CONST_INT is the sole door. temp3 comes from reg_set_last (temp1, insn) at tools/gcc-2.7.2/jump.c:1061, and jump.c:726-860 guarantees a CONST_INT is sitting there.
- probe: Read the gate and mips.h; then confirmed the two-pass sequence in the -da dumps for variant B via `pwsh tools/grinder/dump.ps1 func_800747D8`: in tmp/grind/func_800747D8/dumps/text1b.rtl (function at line 69695) the arms are insn 282 (r140 = 0) and insn 290 (r140 = 4); in text1b.jump (function at 66301) insn 290 is GONE and a NEW insn 622 (r140 = 4) sits before the conditional jump, followed by the emitted xor / ltu / neg / and 4 at insns 629-635. Cross-checked against the fourteen spellings this ledger has measured in that bucket (s1 H3/H4/H5, s2, s5 v3/v4/b1/b2/b11/c1/c2/c3/c4, s6 variant B).
- result: All fourteen measure sandbox --disable all score 10, build_insns 205, emitting sltiu a0,a0,1 / sll a0,a0,2. This REFINES the s5 attribution, which credited jump.c:1190-1191 (either selected constant is const0_rtx): that clause is necessary but not sufficient, and naming the real gate is what identified the escape (reg_set_last must be unable to return a CONST_INT), which the next hypothesis then spells and measures.
- verdict: KILLED
- kill_scope: class
- measured_on: candidate.c floor-6 chassis applied to src/text1b.c across fourteen spellings spanning sessions 1, 2, 5 and 6; zero FAKE constructs in any of them; plus the -da dump pair for variant B.
- predicate_cite: tools/gcc-2.7.2/jump.c:1178

## [s6] Hoisting the default `sound = 4;` above the `selection_sound:` CODE_LABEL defeats the store-flag transform via reg_set_last's label-stop, and the surviving branchy form emits target's register seat `lbu v0,0x64(v0)` and target's branch `beqz v0,<join>` at target's instruction count; it does not lower the floor, because the assignment then sits in a different basic block from the branch.
- mechanism: reg_set_last (tools/gcc-2.7.2/rtlanal.c:867-916) scans backwards from the conditional jump and stops at a CODE_LABEL, returning 0, so temp3 becomes the REG rather than CONST_INT and the gate at tools/gcc-2.7.2/jump.c:1178-1181 fails for BRANCH_COST 1. The same label stops reorg's backward fill_simple_delay_slots, so the branch's delay slot is filled from the target thread instead.
- probe: Variant F (tmp/grind/func_800747D8/s6/var_F.c): `s32 sound;` moved to the function-scope declaration list, `sound = 4;` placed at the top of `case 0:` above both the inner switch and the selection_sound label, and the block reduced to `if (MENU_800747D8->field64 != 0) sound = 0;` plus the single call. Measured with sandbox --disable all, then --diff (tmp/grind/func_800747D8/s6/diff_F.txt), then objdump of tmp/sandbox/func_800747D8/text1b.o.
- result: score 8, build_insns 208 (== target_insns 208). CONFIRMED on the two bytes that matter: hunk 17 reads `lbu v0,0x64(v0)`, matching target exactly, and the branch reads `beqz v0,<join>`, matching target exactly - the floor-6 baseline emits `lbu a0,0x64(v0)` / `bnez a0,<join>`. This is the first form in six sessions to close either. The regression to 8 is placement only: the branch's delay slot gets `li a1,127` stolen from the target thread where target has `li a0,4`; the orphaned `li a0,4` lands in the switch-dispatch delay slot at c734 (an insn target does not have); and with $a0 pinned live across the field65 block that block's lbu/bne/addiu shift to $a1 (3 operand-only diffs the baseline does not have). Banked as rejected/selection_sound-default-hoisted-above-label.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c floor-6 chassis applied to src/text1b.c with `sound` hoisted to the function-scope declaration list; zero FAKE constructs.

## [s6] The duplicated-call form's 3-insn surplus is the two arms' `li a1,127 / li a2,127 / jal` tails failing to cross-jump-merge with each other, because the minimum=1 own-label find_cross_jump at tools/gcc-2.7.2/jump.c:2005 wins first with a one-insn match against case 2's call and retargets both arm jumps to a label do_cross_jump created, after which the sibling-jump pairing loop cannot see them.
- mechanism: tools/gcc-2.7.2/jump.c:2011-2021 runs the sibling-jump loop only when the own-label attempt returned newjpos == 0, and only when INSN_UID (JUMP_LABEL (insn)) < max_uid (tools/gcc-2.7.2/jump.c:2012); do_cross_jump creates its merge label with a fresh uid above max_uid and, unlike the condjump path at jump.c:1982-1989, never adds the retargeted simplejump to jump_chain.
- probe: Ran the s5 frontier's named probe: the codegen-inert BB2_XJUMP_DEBUG knob (tools/gcc-2.7.2/jump.c:66-89) through the INSTRUMENTED cc1 (tools/gcc-2.7.2/cc1) on variant A, capturing tmp/grind/func_800747D8/s6/dumps/xjdbg.txt, with insn uids resolved from tmp/grind/func_800747D8/s6/dumps/text1b.sched2 (function at line 106432).
- result: CONFIRMED verbatim. Trace lines 1855-1870: `enter e1=288 e2=424 min=1 (own-label)` / `MATCH i1=286 i2=418 parallel min->0` / `PAT-MISMATCH i1=284 set(reg<-127) vs i2=409 set` / `result e1=288 min=0 last1=286 => WIN` / `DO_CROSS_JUMP jump=288 newjpos=286 newlpos=418`, and the identical sequence for e1=304. jump_insn 288/304 are the two arms' `goto confirm`; code_label 424 is `confirm`; call_insn 418 is case 2's func_8005C650 call, which falls through into confirm and is therefore the insn preceding that label. After the retarget both jumps point at uid 660 and the trace contains ZERO `chain-partner` entries for e1=288 or e1=304 in any `while (changed)` iteration - while e1=369 (case 1's jump, still on the original label 424) merges 8 insns deep in the same trace.
- verdict: CONFIRMED

## [s6] Writing the duplicated-call form with the inverted test (`if (field64 != 0) func_8005C650(0, ...); else func_8005C650(4, ...);`) changes the emitted selection-sound block relative to the banked `== 0` / 4-first spelling.
- mechanism: expand emits jumpifnot(cond), so the source arm order decides which arm is the fallthrough and which is the branch target, and reorg's steal_delay_list_from_target fills the delay slot from the target thread - so the arm order should decide the delay-slot content and the branch sense.
- probe: Variant A (tmp/grind/func_800747D8/s6/var_A.c) measured against variant C (var_C.c, the banked spelling re-measured this session), both spliced onto candidate.c's chassis on src/text1b.c, sandbox --disable all; plus the full classed --diff for variant A (tmp/grind/func_800747D8/s6/diff_A_full.txt) and objdump of the sandbox object.
- result: Variant A score 7 / build_insns 211; variant C score 7 / build_insns 211 - the same figures. GCC canonicalises the branch sense before the shape is decided. This extends the session-2 polarity kill, which was measured on the same-variable floor-6 chassis, to the duplicated-call chassis. Banked as rejected/selection_sound-dup-call-inverted-polarity.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c floor-6 chassis applied to src/text1b.c with the selection_sound block replaced by each spelling; zero FAKE constructs in either form.

## [s6] Writing the duplicated-call form with the `goto confirm;` inside each arm instead of after the if/else gives the two arms a private join label, letting the minimum=1 own-label find_cross_jump match the arms against each other three insns deep instead of matching each separately against case 2's call.
- mechanism: tools/gcc-2.7.2/jump.c:2005 tries find_cross_jump (insn, JUMP_LABEL (insn), 1, ...) first, and the depth of that match is decided by what physically precedes the jump's own label, so changing the label the arms jump to should change the match depth.
- probe: Variant E (tmp/grind/func_800747D8/s6/var_E.c) on the floor-6 chassis; sandbox --disable all.
- result: score 7, build_insns 211 - identical to both if/else spellings of the duplicated-call form. GCC's jump-to-jump tensioning threads both arm jumps straight through to `confirm` in the first jump pass, long before jump2's cross-jumping runs, so the arms never get a private join label from this source change. Banked as rejected/selection_sound-dup-call-early-return-spelling.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c floor-6 chassis applied to src/text1b.c; zero FAKE constructs.

## [s6] The single-call form with a fresh distinct local and the ZERO arm written FIRST (`if (field64 != 0) sound = 0; else sound = 4;`) escapes the 205 fold, because its `sound = 4` sits in the branch-target arm rather than the fallthrough arm.
- mechanism: the store-flag gate at tools/gcc-2.7.2/jump.c:1042-1066 keys temp on next_nonnote_insn (insn), the fallthrough arm's insn, so putting the non-zero constant in the other arm should change which constant becomes temp2.
- probe: Variant B (tmp/grind/func_800747D8/s6/var_B.c) on the floor-6 chassis; sandbox --disable all, objdump of the sandbox object, and a full -da dump pair.
- result: score 10, build_insns 205; objdump shows sltiu a0,a0,1 / sll a0,a0,2 at c7a4/c7ac. The last unmeasured natural polarity of that regime now joins the other thirteen. Banked as rejected/selection_sound-distinct-local-zero-arm-first.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c floor-6 chassis applied to src/text1b.c; zero FAKE constructs.

## [s6] The two siblings the dispatch flagged as UNSPENT (`main` in src/ings.c and func_800692C0 in src/text1b.c) carry a spelling this residual can transplant.
- mechanism: n/a - inheritance check, not a codegen lever.
- probe: func_800692C0 is this function's callee and is already COMPLETED-C on main; checked candidate.c's call site against the prototype main ships - func_800692C0((u32 *)&sp10, 0, (s16 *)(base + 0x40), &D_800A35D0) - and the floor-6 build already reproduces target's argument setup for it with no diff in that region. Compared src/ings.c's matched body for any shared basic block with this function.
- result: No transplantable block in either sibling: func_800692C0 shares only a call boundary that already matches, and ings.c/main shares no block - it names func_800747D8 only through the text1b/ings call graph. Recorded so a later session does not re-sweep them.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c floor-6 chassis applied to src/text1b.c, compared against the committed bodies of func_800692C0 and src/ings.c on main; zero FAKE constructs.

## Session 7 (forensics)

### H18 - the 205 fold's ENTRY gate is the jump.c:754-860 normalisation, and blocking it alone is sufficient (the s6 "single tension" is chassis-specific, not general)
**Statement:** on the two-arm chassis (`if (c) sound = X; else sound = Y;` with
`sound` a pseudo distinct from the loaded test byte), the store-flag transform
at `tools/gcc-2.7.2/jump.c:1178-1181` is UNREACHABLE unless the normalisation
at `tools/gcc-2.7.2/jump.c:754-860` has already run, because the store-flag
block's own preconditions fail on the un-normalised two-arm shape: `temp =
next_nonnote_insn (insn)` is the fallthrough arm, and `reallabelprev == temp ||
(next_active_insn (temp) is a simplejump to JUMP_LABEL (insn))`
(`tools/gcc-2.7.2/jump.c:1065-1068`) is false, since the fallthrough arm is
followed by the arm's own `goto join` whose JUMP_LABEL is the JOIN, not the
else-label the conditional jump targets. Independently, `reg_set_last`
(`tools/gcc-2.7.2/rtlanal.c:886-888`) returns 0 for the un-normalised shape
because its backwards scan from the conditional jump reaches the
`selection_sound:` CODE_LABEL without finding any set of the result pseudo.
**Mechanism:** the two gates are in series, not in parallel. s5 attributed the
fold to `jump.c:1190-1191`; s6 corrected that to `jump.c:1178-1181`
(BRANCH_COST is 1 on the R3000, `tools/gcc-2.7.2/config/mips/mips.h:2937`, so
`temp3 == CONST_INT` is the only admitting disjunct). This session corrects the
FRAME: `temp3` can only BE a CONST_INT because `jump.c:754-860` moved the
branch-target arm's `sound = 4` in front of the conditional jump. Kill the
move and the store-flag gate never gets an input to fire on. Consequence: the
s6 frontier's "single stated tension" (`a0 = 4` must be simultaneously inside
the branch's basic block for reorg's backward fill AND invisible to
`reg_set_last`) is a property of the SINGLE-ARM `x = 4; if (c) x = 0;` chassis
only. On the two-arm chassis there is no tension at all, because target's
`li a0,4` is stolen from the branch's TARGET THREAD, not filled backwards -
the arm never needs to be in the branch's own block.
**Probe:** read `tools/gcc-2.7.2/jump.c:700-870` and `:1040-1200` end-to-end
against `tools/gcc-2.7.2/rtlanal.c:835-916` and
`tools/gcc-2.7.2/emit-rtl.c:1855-1894` (`next_active_insn` / `prev_active_insn`
skip CODE_LABELs and BARRIERs and stop only at INSN / CALL_INSN / JUMP_INSN),
then cross-checked against the s6 `-da` dump pair
(`tmp/grind/func_800747D8/dumps/text1b.rtl` insns 282/290 becoming
`text1b.jump` insn 622) which is the normalisation's observed output.
**Result:** CONFIRMED by construction from the compiler source plus the s6
dumps. This is a REFRAMING, not a new measurement: it says the whole
fourteen-spelling 205 bucket is ONE gate away from target, and names the gate.
**Verdict:** CONFIRMED

### H19 - PASS-INPUT ENUMERATION: the eleven C-visible preconditions of the jump.c:754-860 normalisation
**Statement:** every input shape that can make the normalisation fail, with the
predicate that rejects it. `insn` = the then-arm's terminating simplejump;
`temp` = the conditional jump; `temp3` = the FALLTHROUGH arm's insn; `temp2` =
the BRANCH-TARGET arm's first insn; `temp1` = the result pseudo X.

  B0 `tools/gcc-2.7.2/jump.c:754` - the then-arm must end in an unconditional
     jump (a plain `if` with no else never reaches this block at all).
  B1 `tools/gcc-2.7.2/jump.c:757-758` - `single_set (temp3)` must exist and its
     SET_DEST must be a REG: a fallthrough arm that STORES TO MEMORY kills it.
  B2 `tools/gcc-2.7.2/jump.c:775` - `prev_active_insn (temp3)` must BE the
     conditional jump: **a fallthrough arm of two or more insns kills it.**
  B3 `tools/gcc-2.7.2/jump.c:762-765` - `temp2` must be a `single_set` to the
     SAME pseudo.
  B4 `tools/gcc-2.7.2/jump.c:766-768` - `SET_SRC (temp2)` must be REG / SUBREG /
     CONSTANT_P: **a branch-target arm whose value comes from MEMORY kills it.**
  B5 `tools/gcc-2.7.2/jump.c:769-774` - `temp2` must carry no REG_NOTES other
     than a REG_EQUAL/EQUIV equal to its own source.
  B6 `tools/gcc-2.7.2/jump.c:784-789` - `reallabelprev == temp2` or
     `next_active_insn (temp2)` is a simplejump to the join: **a branch-target
     arm of two or more insns kills it.**
  B7 `tools/gcc-2.7.2/jump.c:778-781` - `prev_real_insn (JUMP_LABEL (temp)) ==
     insn` and the two `no_labels_between_p` checks around the else-label.
  B8 `tools/gcc-2.7.2/jump.c:794-812,831` - the `nuses` walk: `LABEL_NUSES` of
     the else-label must be fully accounted for by conditional jumps found
     walking BACKWARDS from `temp`; the walk breaks on any CALL_INSN
     (`tools/gcc-2.7.2/jump.c:812`) or any JUMP_INSN that is not a condjump to
     that label. A second, FORWARD entry to the else arm leaves `nuses != 0`
     and kills it.
  B9 `tools/gcc-2.7.2/jump.c:832` - `no_labels_between_p (p, insn)` where
     `p = PREV_INSN (temp)`: **any CODE_LABEL between the last conditional jump
     and the then-arm's terminating jump kills it** (this is what a
     TRUTH_ANDIF / TRUTH_ORIF drop-through label would supply).
  B10 `tools/gcc-2.7.2/jump.c:833` - `! reg_referenced_between_p (temp1, p,
     NEXT_INSN (temp3))`: **the condition may not mention the result pseudo.**
     This is the floor-6 baseline's escape, and it forces the loaded test byte
     and the result into ONE pseudo, which is exactly why the baseline emits
     `lbu a0,0x64(v0)` where target emits `lbu v0,0x64(v0)`.
  B11 `tools/gcc-2.7.2/jump.c:834-836` - `! reg_set_between_p (temp1, p, temp3)`
     and, when `SET_SRC (temp2)` is not a CONST_INT,
     `! reg_set_between_p (SET_SRC (temp4), p, temp2)`. Both regions are only
     two insns wide here (the condjump and the fallthrough arm), so neither is
     reachable from C without also tripping B2 or B10.

**Mechanism:** `tools/gcc-2.7.2/jump.c:754-860`, read end-to-end. B2/B6 are the
load-bearing pair for this function: every banked 205 spelling has one-insn
arms, and every spelling with multi-insn arms (the duplicated-call family)
escapes the fold and lands in the 7/211 bucket instead.
**Probe:** source read plus the fourteen banked 205 measurements (s1/s2/s5/s6)
and the three duplicated-call measurements (s5/s6) re-read against the list.
**Result:** CONFIRMED as an enumeration. The next session's search should be
over B4, B8 and B9 - the three predicates no banked spelling has yet tripped -
because B2/B6 (multi-insn arms) are already known to cost the cross-jump
problem of H20, and B10 is already known to cost the register seat (it IS the
floor-6 baseline).
**Verdict:** CONFIRMED

### H20 - the duplicated-call arms DO cross-jump-merge; what stops them in place is that the if/else join label is threaded away by the trailing `goto confirm;`
**Statement:** the two arms of the duplicated-call form merge three insns deep
(`jal` / `a2 = 127` / `a1 = 127`, mismatching only at `a0 = 4` vs `a0 = 0`) as
soon as the arm jump's OWN label is still the if/else join label, because the
join label's physical predecessor is then the sibling arm. In the in-place
spelling the join label is immediately followed by `goto confirm;`, so jump.c
threads both arm jumps past it to `confirm`, whose physical predecessor is
case 2's `func_8005C650` call - and THAT is why the minimum=1 own-label attempt
at `tools/gcc-2.7.2/jump.c:2005` only ever matches one insn.
**Mechanism:** `find_cross_jump` (`tools/gcc-2.7.2/jump.c:2403`) walks
`i2 = PREV_INSN (e2)` skipping only NOTEs and CODE_LABELs, so whatever insn
physically precedes the jump's label decides the match depth. Threading the
jump from the join to `confirm` swaps that predecessor from "the sibling arm"
to "case 2's call". This REPLACES the s6 statement of the same phenomenon,
which named the symptom (a 1-insn call-vs-call win) but not its cause, and
whose suggested next probe (reorder case 2 so its call is not the last insn
before `confirm`) attacks the wrong end of the chain.
**Probe:** variant A (`tmp/grind/func_800747D8/s7/var_A.c`, banked as
`rejected/selection_sound-dup-call-relocated-before-confirm.c`) moves the
`selection_sound:` block down so it is the last thing before `confirm:` and
therefore needs no trailing `goto confirm;`. Measured with
`sandbox --disable all`, then re-compiled through the instrumented cc1
(`tools/gcc-2.7.2/cc1`) with `BB2_XJUMP_DEBUG=1` via
`tmp/grind/func_800747D8/s7/xjdbg.sh`.
**Result:** score 32, build_insns 217. The merge is PROVED by
`tmp/grind/func_800747D8/s7/dumps/xjdbg.txt:1901-1907`:
`enter e1=406 e2=421 min=1 (own-label)` followed by three MATCH lines
(`i1=404/i2=418 parallel`, `i1=402/i2=416 set(reg<-127)`,
`i1=400/i2=414 set(reg<-127)`) and `PAT-MISMATCH i1=398 set(reg<-4) vs i2=412
set(reg<-0) lose=0`, ending `DO_CROSS_JUMP jump=406 newjpos=400 newlpos=414`.
The form is nonetheless REJECTED because the relocation itself costs +9 insns:
the single-call control
(`rejected/selection_sound-single-call-relocated-before-confirm.c`) moves
10/205 -> 29/214.
**Verdict:** CONFIRMED as the mechanism; KILLED as a floor improvement.
**kill_scope:** instance
**measured_on:** candidate.c floor-6 chassis applied to src/text1b.c with the
selection_sound block relocated to immediately before `confirm:`, in both the
duplicated-call and single-call spellings; zero FAKE constructs in either.

### H21 - target's own layout excludes the duplicated-call family on this chassis
**Statement:** target's selection path ends `addiu a1,zero,0x7F` /
`j .L80074A20` / `addiu a2,zero,0x7F` with the single `jal func_8005C650`
shared at `.L80074A20` (asm/funcs/func_800747D8.s:115-117 and :161-162). That
is ONE `a1`/`a2` pair for the whole selection block plus a 1-insn own-label
merge at the shared `jal` - i.e. target's if/else JOIN is itself followed by a
jump, which is exactly the configuration that threads arm jumps away (H20). A
duplicated call therefore cannot reach target's bytes on this chassis without
the relocation H20 already priced at +9.
**Mechanism:** H20's threading argument applied to target's own block order:
the selection block sits FIRST, before case 1's label
(asm/funcs/func_800747D8.s:107-118 precede `jlabel .L80074984` at :120), so
the block must end in a forward jump and its join can never be a
fallthrough-into-`confirm`.
**Probe:** read asm/funcs/func_800747D8.s:85-165 for the block order and the
merge points; cross-checked against the two relocation measurements above.
**Result:** CONFIRMED. Target's selection block is the SINGLE-call two-arm form
whose own `jal` was own-label-merged 1 insn deep into case 2's - the same
1-insn merge our duplicated-call form gets, but applied to a block that only
ever had one call. The remaining question is therefore entirely H19: which of
B4 / B8 / B9 the original C trips.
**Verdict:** CONFIRMED

### H22 - MECHANISM PROBE: an adjacent-byte `||` does not create a drop-through CODE_LABEL, so B9 is still untested
**Statement:** writing the condition as a TRUTH_ORIF over two ADJACENT byte
fields does not exercise predicate B9, because GCC merges the two byte tests
into a single halfword load before any second branch is emitted.
**Mechanism:** the two fields (`field64` at 0x64, `field65` at 0x65) are
adjacent, so the load/compare pair is rewritten as a single `lhu` of the
halfword at 0x64 tested against zero - one comparison, one conditional jump,
hence no TRUTH_ORIF drop-through label for `no_labels_between_p (p, insn)`
(`tools/gcc-2.7.2/jump.c:832`) to trip on.
**Probe:** variant D (`tmp/grind/func_800747D8/s7/var_D.c` - a MECHANISM PROBE
ONLY, deliberately NOT semantics-preserving, and therefore deliberately not
banked in rejected/): `if (field64 != 0 || field65 != 0) sound = 0; else
sound = 4;`. `sandbox --disable all --diff` ->
`tmp/grind/func_800747D8/s7/diff_D.txt`.
**Result:** score 10, build_insns 205 - the fold regime. The diff's hunks 18/19
read `lhu a0,100(v0)` / `sltiu a0,a0,1` / `sll a0,a0,0x2`, confirming both the
halfword merge and the store-flag output. B9 is therefore NOT disproved; it is
UNTESTED, and testing it needs a compound condition whose two operands cannot
be coalesced into one comparison.
**Verdict:** KILLED (as a B9 test vehicle - not as the B9 predicate)
**kill_scope:** instance
**measured_on:** candidate.c floor-6 chassis applied to src/text1b.c with the
selection_sound block replaced by the ORIF spelling; zero FAKE constructs.

### H23 - KILL RE-AUDIT (mandated): variant F still measures 8/208 and this ledger has no FAKE carrier to ablate
**Statement:** the s6 instance kill on variant F - the closest form to target,
and the only spelling that matches BOTH `lbu v0,0x64(v0)` and `beqz v0,<join>`
- stands unchanged on the current chassis, and its FAKE-ablation prong is
vacuous.
**Mechanism:** n/a - re-measurement.
**Probe:** `tmp/grind/func_800747D8/s6/var_F.c` re-applied to src/text1b.c and
re-measured with `sandbox --disable all`
(`tmp/grind/func_800747D8/s7/score_F_reaudit.txt`); then `python3
tools/fake_ablate.py --func func_800747D8 --file text1b --candidate
memory/grind/func_800747D8/candidate.c`.
**Result:** score 8, build_insns 208 - identical to s6, so the kill re-stands.
fake_ablate reports "no FAKE-annotated constructs found in
memory/grind/func_800747D8/candidate.c; nothing to ablate", so no banked kill
on this ledger was ever measured with a FAKE carrier occupying a target pseudo.
**Verdict:** CONFIRMED (the s6 kill re-stands)

## [s7] On the two-arm chassis the store-flag transform at tools/gcc-2.7.2/jump.c:1178-1181 cannot be reached until the normalisation at tools/gcc-2.7.2/jump.c:754-860 has already rewritten `if (c) x = a; else x = b;` into `x = b; if (c) x = a;`, so defeating the normalisation alone is sufficient to escape the 10/205 regime - no separate store-flag escape has to be engineered.
- mechanism: The store-flag block has its own precondition at tools/gcc-2.7.2/jump.c:1065-1068 (`reallabelprev == temp` or the next active insn after temp is a simplejump to JUMP_LABEL(insn)); on an un-normalised two-arm shape the fallthrough arm is followed by the arm's own `goto join`, whose JUMP_LABEL is the join and not the else-label the conditional jump targets, so that disjunction is false. Independently reg_set_last (tools/gcc-2.7.2/rtlanal.c:886-888) returns 0 there, its backward scan reaching the `selection_sound:` CODE_LABEL without finding any set of the result pseudo. The two gates are in series, not in parallel. This supersedes the s5 attribution (jump.c:1190-1191) and reframes the s6 one (jump.c:1178-1181): s6 named the gate the store-flag block sees, this names the gate that decides.
- probe: Read tools/gcc-2.7.2/jump.c:700-870 and :1040-1200 end to end against tools/gcc-2.7.2/rtlanal.c:835-916 and tools/gcc-2.7.2/emit-rtl.c:1855-1894 (next_active_insn / prev_active_insn skip CODE_LABELs and BARRIERs and stop only at INSN / CALL_INSN / JUMP_INSN), cross-checked against the s6 -da dump pair tmp/grind/func_800747D8/dumps/text1b.rtl (insns 282/290) vs text1b.jump (new insn 622), which is the normalisation's observed output.
- result: CONFIRMED by construction from the compiler source plus the banked s6 dumps. Corollary recorded in the ledger: the s6 'single stated tension' - a0=4 must be both inside the branch's basic block for reorg's backward fill AND invisible to reg_set_last - is a property of the single-arm `x = 4; if (c) x = 0;` chassis only. On the two-arm chassis there is no tension, because target's li a0,4 is stolen from the branch's TARGET thread and the arm never has to live in the branch's own block.
- verdict: CONFIRMED

## [s7] The jump.c:754-860 normalisation has eleven C-visible preconditions (banked as H19/B0-B11 with a file:line predicate each); of them B2 (tools/gcc-2.7.2/jump.c:775) and B6 (tools/gcc-2.7.2/jump.c:784-789) mean an arm of two or more insns defeats it, B10 (tools/gcc-2.7.2/jump.c:833) means a condition mentioning the result pseudo defeats it, and B4 (jump.c:766-768), B8 (jump.c:794-812,831) and B9 (jump.c:832) are the three predicates no banked spelling of this block has yet tripped.
- mechanism: tools/gcc-2.7.2/jump.c:754-860 read end to end. B2/B6 explain the whole measured partition of this ledger: every one-insn-arm spelling folds at 10/205, every multi-insn-arm (duplicated-call) spelling escapes at 7/211. B10 explains the floor-6 baseline: 'the condition mentions the result' is precisely what forces the loaded test byte and the result into one pseudo, which is why the baseline emits lbu a0,0x64(v0) where target emits lbu v0,0x64(v0).
- probe: Source read, then the fourteen banked 205 measurements (s1/s2/s5/s6) and the three duplicated-call measurements (s5/s6) re-read against the enumeration to check each lands in the predicted bucket.
- result: CONFIRMED as an enumeration; every banked measurement on this ledger is accounted for by B2/B6/B10. B4, B8 and B9 are the untried search space handed to the next session.
- verdict: CONFIRMED

## [s7] Relocating the selection_sound block so it sits immediately before `confirm:` (removing the trailing `goto confirm;`) lets the duplicated-call arms cross-jump-merge three insns deep, but the relocation costs more insns than the merge recovers on this chassis.
- mechanism: find_cross_jump (tools/gcc-2.7.2/jump.c:2403) walks i2 = PREV_INSN(e2) skipping only NOTEs and CODE_LABELs, so the insn physically preceding the jump's label decides the match depth. With the trailing `goto confirm;` present, jump.c threads both arm jumps past the if/else join to `confirm`, whose physical predecessor is case 2's func_8005C650 call - a 1-insn match. Without it the arm jump's own label is the join, whose predecessor is the sibling arm - a 3-insn match on jal / a2=127 / a1=127. This names the CAUSE behind the s6 symptom (the minimum=1 own-label win at tools/gcc-2.7.2/jump.c:2005) and retires s6's suggested probe of reordering case 2's call, which attacks the wrong end of the chain.
- probe: Variant A (tmp/grind/func_800747D8/s7/var_A.c, banked as rejected/selection_sound-dup-call-relocated-before-confirm.c) measured with sandbox --disable all, then recompiled through the instrumented cc1 tools/gcc-2.7.2/cc1 with BB2_XJUMP_DEBUG=1 via tmp/grind/func_800747D8/s7/xjdbg.sh. Relocation cost isolated with variant B (single-call control, banked as rejected/selection_sound-single-call-relocated-before-confirm.c).
- result: Variant A score 32 / build_insns 217; variant B (control) score 29 / build_insns 214, i.e. the relocation alone costs +9 insns over the in-place 10/205 spelling and the dup form's net +6 is that +9 minus the 3 insns the merge recovers. The merge itself is proved verbatim at tmp/grind/func_800747D8/s7/dumps/xjdbg.txt:1901-1907: `enter e1=406 e2=421 min=1 (own-label)` then three MATCH lines (i1=404/i2=418 parallel, i1=402/i2=416 set(reg<-127), i1=400/i2=414 set(reg<-127)) then `PAT-MISMATCH i1=398 set(reg<-4) vs i2=412 set(reg<-0)` then `DO_CROSS_JUMP jump=406 newjpos=400 newlpos=414`. Also recorded: target's own layout puts the selection block first, before case 1's label (asm/funcs/func_800747D8.s:107-118 precede jlabel .L80074984 at :120), and its path ends addiu a1,0x7F / j .L80074A20 / addiu a2,0x7F with a single shared jal at .L80074A20 (:115-117, :161-162) - one a1/a2 pair plus a 1-insn own-label merge - so target's block is the SINGLE-call two-arm form, not a duplicated call.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c floor-6 chassis applied to src/text1b.c with the selection_sound block relocated to immediately before `confirm:` in both the duplicated-call and the single-call spelling; zero FAKE constructs in either form or in the chassis.

## [s7] Spelling the condition as a TRUTH_ORIF over the two adjacent byte fields (field64 at 0x64, field65 at 0x65) does not exercise predicate B9, because GCC coalesces the two byte tests into one halfword comparison and emits no drop-through CODE_LABEL.
- mechanism: The fields are adjacent, so the pair of byte loads and compares is rewritten as a single lhu of the halfword at 0x64 tested against zero. One comparison means one conditional jump, so no TRUTH_ORIF drop-through label is emitted for no_labels_between_p (p, insn) at tools/gcc-2.7.2/jump.c:832 to trip on, and the normalisation proceeds exactly as for a simple condition.
- probe: Variant D (tmp/grind/func_800747D8/s7/var_D.c) - a MECHANISM PROBE ONLY, deliberately not semantics-preserving and therefore deliberately NOT banked in rejected/: `if (field64 != 0 || field65 != 0) sound = 0; else sound = 4;`. sandbox --disable all --diff -> tmp/grind/func_800747D8/s7/diff_D.txt.
- result: Score 10, build_insns 205 - the fold regime. Hunks 18/19 read lhu a0,100(v0) / sltiu a0,a0,1 / sll a0,a0,0x2, confirming both the halfword coalescing and the store-flag output. B9 is therefore not disproved, it is untested: testing it needs a compound condition whose two operands cannot be coalesced into a single comparison.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c floor-6 chassis applied to src/text1b.c with the selection_sound block replaced by the adjacent-field ORIF spelling; zero FAKE constructs.

## [s7] MANDATED KILL RE-AUDIT: the s6 instance kill on variant F (the closest banked form to target) re-measures identically on the current chassis, and this ledger carries no FAKE-annotated construct for fake_ablate to strip.
- mechanism: n/a - re-measurement of a banked instance kill under the current chassis and FAKE state.
- probe: tmp/grind/func_800747D8/s6/var_F.c re-applied to src/text1b.c and re-measured with sandbox --disable all (tmp/grind/func_800747D8/s7/score_F_reaudit.txt); then python3 tools/fake_ablate.py --func func_800747D8 --file text1b --candidate memory/grind/func_800747D8/candidate.c.
- result: Variant F: score 8, build_insns 208 - identical to s6, so that kill re-stands unchanged. fake_ablate reports 'no FAKE-annotated constructs found in memory/grind/func_800747D8/candidate.c; nothing to ablate', so the FAKE-ablation prong of the re-audit is vacuous here: no banked kill on this ledger was ever measured with a FAKE carrier occupying a target pseudo.
- verdict: CONFIRMED

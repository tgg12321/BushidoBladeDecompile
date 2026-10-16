# Evidence bank — func_8004A76C

## s1 (recon) — canonical gate verdict + OBJECT MODEL + prior audit

- `canonical func_8004A76C` returns **ASM-WHOLE**, reason "whole-function canonical-asm
  grant in inline_asm_canonical.txt". `sandbox func_8004A76C --disable all` this session:
  `{"score": 37, "no_c_body": true, "build_insns": 0}` — src/main/3AB48.c:30 is still a bare
  `INCLUDE_ASM("asm/funcs", func_8004A76C);` stub, so `--diff` prints "no diff: function is
  still whole-body INCLUDE_ASM — there is no C to compare" (no source-level/operand-only
  hunks exist to classify; there is no C body at all).
- `scan_hand_coded.py --single func_8004A76C`: **tier=STRONG, score=1/8**, driven by S7
  (callee-saved register used without save: unsaved `$s0`) — "GCC-impossible, decisive".
- `asm/funcs/func_8004A76C.s` (39 insns) is a 2-vertex-batch GTE perspective-transform loop:
  `lwc2`/lane loads into GTE data regs 0-5, `rtpt` (perspective-transform-triple), `mfc2`
  `$17`/`$18`/`$19` (GTE result SZ/flag-ish regs, flagged "handwritten instruction" by the
  assembler), `swc2` writes, and `sh` stores of the transformed `(x,y)` pair into the output
  array — classic PS1 3D pipeline code. GTE (cop2) ops have no C form at all (AGENTS.md,
  `inline-asm-policy`), so the body cannot be anything but hand/inline asm regardless of the
  S7 signal.
- **Already decisively audited.** `docs/audits/INLINE-ASM-AUDIT-2026-10-01.md` item A6 raised
  exactly the live question (whether a GCC file-scope global-register-variable
  `register T x asm("$16")` could produce the unsaved-$s0 shape from C) and closed it
  **"CLOSED, no change"**: (a) the only caller, `func_8004A4E0`, is itself whole-body hand
  asm (its own canonical-2026-09-29 audit: custom callee-saved ABI handing `$s0` live across
  `jal func_8004A76C`/`func_8004A808`/`func_8004A940`, which GCC cannot emit — `$s0` is
  advanced by this function at 0x8004A798 and read back by the caller after return at
  0x8004A5C0, something GCC assumes a callee-save cannot do); (b) the loop body is itself
  hand software-pipelined — `rtpt` for iteration N+1 is issued (0x8004A79C/0x8004A7D4) before
  iteration N's `sh` stores retire (0x8004A7A4-0x8004A7AC), and the `j` entry point
  (0x8004A794) jumps into the SECOND half of the unrolled loop to prime the pipeline — a GTE
  latency-hiding shape no GCC scheduler produces; (c) a global-register-variable is itself a
  register pin, independently banned by policy regardless of whether it would reproduce the
  bytes.
- OBJECT MODEL: no DATA MODEL section was dispatched for this function (no flagged globals
  in the brief). Independently confirmed by reading the body: the function touches only its
  four register-convention parameters ($a0 count, $s0 input vec ptr, $a1/$a2 output ptrs) and
  GTE coprocessor data/result registers — no global symbol, no `lw`/`sw` to any `D_8*`/`g_*`
  address anywhere in the 39-insn body. MATCHES n/a — no global object model applies.
- Sibling `func_8004A4E0` (23 sessions, floor flat at 163, rotated) is the caller of this
  function and shares the same ABI cluster; its ledger's s18/s18b Judge FAIL (2026-10-15
  16:50, docs/grind/decisions.md:1320) ruled that non-scanner GCC-impossible evidence cannot
  stand in for the `scan_hand_coded` STRONG bar for *that* function (func_8004A4E0 itself
  scans LOW 0/8, because it is the caller and saves everything). **func_8004A76C does not
  have that problem**: it scans STRONG 1/8 directly (S7), independent of A4E0's unresolved
  question, and additionally has the closed A6 audit specifically naming it.

- [s1] canonical func_8004A76C = ASM-WHOLE (whole-function canonical-asm grant in inline_asm_canonical.txt)

- [s1] sandbox func_8004A76C --disable all this session = {"score":37,"no_c_body":true,"build_insns":0}; --diff: 'no diff: function is still whole-body INCLUDE_ASM' (no hunks to classify, no C body at all)

- [s1] scan_hand_coded.py --single func_8004A76C = tier=STRONG score=1/8, driven by S7 (unsaved callee-saved $s0) — 'GCC-impossible, decisive'

- [s1] asm/funcs/func_8004A76C.s is a GTE (cop2) perspective-transform loop: lwc2 vertex loads, rtpt, mfc2 of GTE result regs ($17/$18/$19, flagged 'handwritten instruction' by the assembler), swc2/sh stores; GTE ops have no C form at all (AGENTS.md inline-asm-policy), independent of the S7 signal

- [s1] docs/audits/INLINE-ASM-AUDIT-2026-10-01.md item A6 already closed the exact open question this function's reopen asks ('decide afresh: hand-written asm vs compiled C'), with the global-register-variable alternative ruled out for the three reasons in H1

- [s1] OBJECT MODEL: no DATA MODEL section was dispatched for this function; independently confirmed by reading the 39-insn body that it touches only register-convention parameters and GTE coprocessor registers, no global symbol — MATCHES n/a, no global object model applies

- [s1] sibling func_8004A4E0 (the sole caller, 23 sessions, floor flat 163, rotated) scans LOW 0/8 and was Judge-FAILed on exactly the scan_hand_coded-STRONG bar (docs/grind/decisions.md:1320); func_8004A76C does not share that problem because it scans STRONG 1/8 directly and has its own closed A6 audit

## s2 (structural) — no C body to apply structural levers to; cluster scan completed

- Re-measured this session: `canonical func_8004A76C` still returns **ASM-WHOLE** (same
  reason, "whole-function canonical-asm grant in inline_asm_canonical.txt"); `sandbox
  func_8004A76C --disable all` still `{"score":37,"no_c_body":true,"build_insns":0}` —
  src/main/3AB48.c:30 is unchanged, still a bare `INCLUDE_ASM` stub. Nothing moved since s1.
- Structural-modality levers (block-local var splits, declaration order, type narrowing,
  statement re-association) operate on a C body under a sandbox-measured distance. This
  function has none — the canonical gate classifies the whole function as hand asm, so there
  is no `--diff` to classify into source-level/operand-only/not-scored hunks (confirmed again
  this session: `--diff` still prints "no diff: function is still whole-body INCLUDE_ASM").
  Every structural technique is inapplicable by construction, not merely unproductive.
- Completed the s1 frontier's cheap cluster-scan probe (item 2): ran
  `scan_hand_coded.py --single` on both remaining cluster siblings.
  - `func_8004A808`: **tier=STRONG, score=3/8** (S3 no-spills, S4 front-loads, S7 unsaved
    `$s0`) — "GCC-impossible, decisive" on S7 alone, same as func_8004A76C's own S7 hit.
  - `func_8004A940`: **tier=STRONG, score=3/8** (same S3/S4/S7 triplet; S7 fires on EIGHT
    unsaved callee-saves: `$s0`-`$s7`, the maximal shape).
  - All three jal-cluster siblings named in func_8004A4E0's custom-ABI evidence
    (func_8004A76C, func_8004A808, func_8004A940) now have an independently-measured
    scan_hand_coded STRONG verdict on file. Only the cluster's caller, func_8004A4E0 itself,
    scans LOW (0/8) — which is exactly the asymmetry its own s18/s18b Judge FAIL already
    turned on (docs/grind/decisions.md:1320): the caller saves everything it uses, the three
    callees don't, because the callees are the ones directly leaking unsaved `$sN` state back
    across the `jal` boundary.

- [s2] canonical func_8004A76C = ASM-WHOLE, unchanged since s1 (whole-function canonical-asm grant in inline_asm_canonical.txt)

- [s2] sandbox func_8004A76C --disable all this session = {"score":37,"no_c_body":true,"build_insns":0}; --diff reports no C body exists to diff

- [s2] scan_hand_coded.py --single func_8004A808 = tier=STRONG score=3/8 (S3+S4+S7, unsaved $s0)

- [s2] scan_hand_coded.py --single func_8004A940 = tier=STRONG score=3/8 (S3+S4+S7, unsaved $s0-$s7, all eight)

- [s2] All three func_8004A4E0-called cluster siblings (func_8004A76C, func_8004A808, func_8004A940) now carry independently-measured scan_hand_coded STRONG verdicts; only the caller func_8004A4E0 scans LOW

- [s3] sandbox func_8004A76C --disable all --diff this session: floor unchanged at 37, no_c_body=true, build_insns=0 (tmp/grind/func_8004A76C/s3/sandbox_diff.txt).

- [s3] asm/funcs/func_8004A76C.s (44 lines) is a 39-insn hand software-pipelined GTE perspective-transform loop using rtpt (cop2) with lwc2/swc2 vertex I/O and a pipelined jump-into-loop-body entry (.L8004A7B0) — consistent with s1's prior read of this file.

- [s3] tools/decomp-permuter/import.py src/main/3AB48.c func_8004A76C run directly (WSL venv) this session: 'Failed to find GLOBAL_ASM for function func_8004A76C' — confirms by direct tool attempt (not citation) that no permuter workspace can be built while this function has zero C bytes.

- [s3] This matches and reinforces s1 (recon, GTE loop / A6 audit closed) and s2 (structural modality inapplicable, no C body) — three independent modalities (recon, structural, permuter) have now each independently hit the same root fact: there is no C source for func_8004A76C to work on.

## s4 (enumerate) — fourth independent modality confirms same root fact; no new finding

- [s4] Re-measured this session (not cited from ledger text): `canonical func_8004A76C` →
  verdict ASM-WHOLE, reason "whole-function canonical-asm grant in inline_asm_canonical.txt".
  `sandbox func_8004A76C --disable all` → `{"score": 37, "no_c_body": true, "build_insns": 0}`.
  Floor unchanged at 37.
- [s4] The mandated SYSTEMATIC SPELLING SWEEP procedure (`spelling_enum.py` +
  `sweep_variants.py`) requires an existing C candidate with the diverging region written out
  between `/* ENUM-BEGIN */`/`/* ENUM-END */` markers inside a real function body. func_8004A76C
  has zero C statements anywhere (src/main/3AB48.c:30 is the bare `INCLUDE_ASM` stub) — there is
  no region to mark, no statement to enumerate, and inventing a synthetic C body to feed the
  tool would misrepresent the function (completion-bar item 3: honest code — the body is
  GTE cop2 + hand software-pipelined asm, not GCC-emittable C by any spelling, per s1 H1's
  class kill against docs/audits/INLINE-ASM-AUDIT-2026-10-01.md item A6).
- [s4] This is the FOURTH independent modality (recon s1, structural s2, permuter s3, enumerate
  s4) to hit the identical gate: ASM-WHOLE canonical grant + no_c_body=true. No modality that
  requires an existing C body can make progress here; the only remaining axis is the
  escalation-modality disposition filing (already queued as this ledger's live frontier since
  s1/s2).

- [s4] canonical func_8004A76C re-measured this session: verdict ASM-WHOLE, 'whole-function canonical-asm grant in inline_asm_canonical.txt' (unchanged from s1/s2).

- [s4] sandbox func_8004A76C --disable all re-measured this session: {"score": 37, "no_c_body": true, "build_insns": 0}; floor unchanged at 37.

- [s4] spelling_enum.py's required precondition (a C candidate body with an ENUM-BEGIN/END marked region) cannot be met: src/main/3AB48.c:30 is a bare INCLUDE_ASM stub with zero C statements.

- [s4] This is the fourth independent modality (recon s1, structural s2, permuter s3, enumerate s4) to hit the identical gate: ASM-WHOLE canonical grant + no_c_body=true. Each used a different tool/approach and each confirmed the same root fact rather than finding a new angle.

## s5 (synthesis) — first C bodies ever measured for this function; merged verdict

- [s5] KILL RE-AUDIT: all three instance kills (s2 structural, s3 permuter, s4 enumerate) were measured on the bare INCLUDE_ASM stub with NO FAKE carrier; no banked C form exists, so `tools/fake_ablate.py` has nothing to ablate (no FAKE construct ever present). Their measured_on still equals the current chassis (src/main/3AB48.c:30 stub, unchanged). None is void. The s1 class kill (docs/audits/INLINE-ASM-AUDIT-2026-10-01.md:93) remains the only foreclosure and rests on the wall (global register var = pin), not on a stale chassis.
- [s5] Synthesis probe A (tmp/grind/func_8004A76C/s5/honest_gte_loop.c): natural do/while loop, gte.h gte_ldv3/gte_rtpt/gte_stsxy3/gte_mfc2(17..19), sz stored as s16, vertex pointer as ordinary 4th arg. `sandbox --disable all --diff --candidate` = **33** (tmp/grind/func_8004A76C/s5/sandbox_honest.txt): 30 vs 37 insns, 3 hunks ALL source-level, 0 operand-only. Divergences: ldv3 via three pointers (addiu per vector) vs single-base offsets 0..20; not pipelined; $a3 vs $s0 base.
- [s5] Synthesis probe B (tmp/grind/func_8004A76C/s5/pipelined_goto.c): the target CFG written as C — `n--; load; rtpt; v+=3; goto mid; loop: rtpt; sz+=3; sz[-3..-1]=z; mid: stsxy3; z=mfc2 17..19; sxy+=3; if(n){n--; load; v+=3; goto loop;} sz[0..2]=z;` with PsyQ-style single-base gte_ldv3c/gte_stsxy3c defined locally. sandbox = **30** (tmp/grind/func_8004A76C/s5/sandbox_pipelined.txt), 18 vs 37 insns, 5 hunks all source-level. The GPR skeleton (sh -6/-4/-2(a2), mfc2 x3, tail sh 0/2/4(a2), branch-into-middle `j`) lines up with the target's; the gap is entirely the stripped GTE statements and the $s0 base.
- [s5] SCORER FINDING: in BOTH probes the sandbox output contains no `c2 0x280030` on the ours side — gte.h's `gte_rtpt()` (`.word 0x4A280030`) is stripped by --disable all, because engine/gtemacro.py keeps only PINNED header-exact units and DMPSX_WORDS (engine/gtemacro.py:273) does not list rtpt; probe B's multi-insn local ldv3c/stsxy3c were stripped too (0 lwc2/swc2 on ours side). So under the current pinned set the sandbox cannot score ANY C body of this function as containing its rtpt; distance 0 is unattainable by the scorer for every C spelling until rtpt/ldv3c/stsxy3c units are pinned (operator engine surface, owner ruling 2026-09-25 process).
- [s5] Neither 33 nor 30 is reported as a floor: both forms move the vertex pointer from the target's unsaved $s0 (written by hand-asm caller func_8004A4E0 at 0x8004A584/5A0/5C4 and read back after return at 0x8004A5C0, inline_asm_canonical.txt:199) to $a3. That signature is a false ABI claim (completion-bar honest code) and would not run against the real caller; the only C spelling that reads $s0 unsaved is a global register variable = pin (the wall). Honest floor stays 37 (stub).
- [s5] MERGED VERDICT: three independent blockers stack — (1) $s0 custom ABI, wall-bound (pin), s1 class kill; (2) sandbox strips unpinned GTE units (rtpt), scorer-bound; (3) the pipelining shape itself IS expressible as C (probe B skeleton aligns), so it is NOT a blocker — this corrects the A6 audit's reason (b), which is the weakest of its three reasons. Reason (a)+(c) alone carry the canonical grant.

- [s5] s5 honest natural GTE loop: sandbox 33, all hunks source-level (tmp/grind/func_8004A76C/s5/sandbox_honest.txt)

- [s5] s5 hand-pipelined goto form: sandbox 30, GPR skeleton aligns, gap = stripped GTE + $s0 base (tmp/grind/func_8004A76C/s5/sandbox_pipelined.txt)

- [s5] Neither score is a floor: both move the vertex pointer from unsaved $s0 (set/read by hand-asm caller func_8004A4E0, inline_asm_canonical.txt:199) to $a3, a false ABI; honest floor stays 37

- [s5] Sandbox strips gte.h gte_rtpt() (.word 0x4A280030 not in engine/gtemacro.py:273 DMPSX_WORDS) — no C body of this function can be scored with its rtpt under the current pinned set

- [s5] A6 reason (b) (pipelining has no C form) refuted; grant rests on reasons (a)+(c)

- [s6] Re-audit: rejected/pipelined-goto-wrong-abi-stripped-gte.c still sandboxes 30 (5 source-level hunks), tmp/grind/func_8004A76C/s6/sandbox_reaudit_pipelined.txt.
- [s6] inverse_compose classify (must run in WSL; the Windows-side run fails because objdump isn't found) = PRE-RA / no backend: the scorer strips the GTE asm, so the multiset differs (tmp/grind/func_8004A76C/s6/classify_pipelined.txt). The solver backends do not apply to the sandbox stream for this function.
- [s6] tools/grinder/dump.ps1 -Candidate fails for nested TU ids (looks for tmp/grind/<func>/dumps/main/3AB48.c); workaround: tmp/grind/func_8004A76C/s6/cc.sh runs cpp + instrumented cc1 -da directly.
- [s6] Raw cc1 of the s5 pipelined goto form (GTE kept): everything matches except (1) $a3 vs $s0 base, (2) mfc2 temps $2,$3,$8 vs $t0-$t2, (3) no nop-nop before the first rtpt, (4) dbr fills the jr delay slot with the last sh (target leaves a nop), (5) n-- / sxy+=3 delay-slot placement around beqz. tmp/grind/func_8004A76C/s6/probe_pipe.s, probe_pipe.i.greg.
- [s6] Class kill: when $v0/$v1/$a3 are free, $t0-$t2 is not what global.c:1057 first-fit picks (mips has no REG_ALLOC_ORDER). Target never touches $v0/$v1/$a3 → the seats corroborate hand-written asm. Floor stays 37 (stub).

- [s6] sandbox re-audit of the pipelined form = 30 (s6/sandbox_reaudit_pipelined.txt)

- [s6] classify = PRE-RA/no backend because the scorer strips GTE asm; it must run in WSL (objdump)

- [s6] tools/grinder/dump.ps1 -Candidate fails for nested TU ids; s6/cc.sh is the workaround

- [s6] raw cc1 with GTE kept: residual is $s0 base, $t0-$t2 seats, first-rtpt hazard nops, unfilled jr delay slot, beqz delay-slot decrement

- [s6] mips GCC 2.7.2 has no REG_ALLOC_ORDER; global.c:1057 first-fit cannot give $t0-$t2 while $v0/$v1/$a3 are free

## s7 (forensics): raw instrumented-cc1 pass attribution
- [s7] Re-audit: rejected/pipelined-goto-wrong-abi-stripped-gte.c still sandboxes 30 (5 source-level hunks) on the stub chassis (tmp/grind/func_8004A76C/s7/sandbox_reaudit_pipelined.txt); no FAKE to ablate.
- [s7] Closest raw-cc1 form is now rejected/psyq-rtpt-pair-reorg-slot-residual.c (tmp/grind/func_8004A76C/s7/p_f.s, 38 insns vs target 39): CFG, all GTE ops, entry nop pad, j-into-middle, sh offsets and the beqz slot content all match. Remaining: (1) base $a3 vs unsaved $s0 [s1 class kill]; (2) mfc2 temps $2/$3/$8 vs $t0-$t2 [s6 class kill global.c:1057]; (3) `addu a1,12` before the mfc2s instead of after; (4) the jr slot holds the last sh instead of nop.
- [s7] Pass attribution: (3) with the beqz slot, and (4), are reorg.c fill_simple_delay_slots decisions (backward scan reorg.c:2978-3022; stop_search_p reorg.c:719-744; mips.md:125 define_delay). Source order relative to the GTE macros is fixed by sched.c:1979 (volatile asm = full barrier), so the C input only chooses which insn precedes the branch, and whatever eligible ALU insn precedes it is moved into the slot. The target's `addiu a1; beqz a0; addiu a0,-1` and `sh; jr ra; nop` both contradict that scan: a fourth independent not-GCC signal (with the $s0 ABI, the $t0-$t2 seats and the `/* handwritten instruction */` mfc2 markers).
- [s7] Nop pad: PsyQ 4.0 INLINE_C.H:489 gte_rtpt (nop,nop) + :814 gte_rtpt_b (no nop) reproduce it; this residual is source-level and closed.
- [s7] PASS-INPUT ENUMERATION (dbr inputs): sxy-increment position {after the mfc2s (p_base), before them (p_b/p_f), in the loop arm (p_c)}; counter test {`n != 0` then `n--` (p_base), `n-- != 0` (p_e: extra move copy)}; store qualifier {plain, volatile sz (p_d: leaves the jr slot empty; forbidden volatile coercion)}. label_census on p_b: CFG and predecessor multiset identical to the target ([1,1,2]); only $s0 refs (16 vs 0) and insn count differ (tmp/grind/func_8004A76C/s7/label_census_p_b.txt). nrefs_census/loop_movables do not apply to the stub TU (no C body for func_8004A76C in src; nrefs_census died in cpp on the Windows side, tmp/grind/func_8004A76C/s7/nrefs_census_stub.txt).
- [s7] BB2_DBR_DEBUG produced no DBRDBG lines from tools/gcc-2.7.2/build/cc1 (empty *.dbrdbg files); future dbr forensics should use tools/gcc-2.7.2/cc1 if it carries the instrumentation.

- [s7] Closest raw-cc1 form (rejected/psyq-rtpt-pair-reorg-slot-residual.c, tmp/grind/func_8004A76C/s7/p_f.s, 38 vs 39 insns) matches the target's CFG, every GTE op, entry nop pad, j-into-middle, sh offsets and beqz slot content; four residuals remain: $a3 vs unsaved $s0 base (s1 class kill), mfc2 temps $2/$3/$8 vs $t0-$t2 (s6 class kill global.c:1057), addu a1 before vs after the mfc2s with the slot constraint (s7 class kill reorg.c:2990), jr slot filled vs nop (reachable only with forbidden volatile).

- [s7] sched.c:1979: volatile asm (all gte.h macros) is a full scheduling barrier, so the C source order of non-asm insns relative to GTE ops is preserved to dbr.

- [s7] label_census on p_b: CFG and predecessor multiset identical to the target ([1,1,2]); only $s0 refs (16 vs 0) and insn count differ.

- [s7] BB2_DBR_DEBUG gives no DBRDBG output from tools/gcc-2.7.2/build/cc1; .dbr dumps were used instead.

- [s7] nrefs_census / loop_movables do not apply: no C body for func_8004A76C exists in the stub TU.

## s8 (rederive)
- [s8] Sibling check (mandatory before own probes): func_8004A4E0's candidate.c (written 2026-10-15 17:03, unspent per dispatch) is comments-only over its own INCLUDE_ASM stub — zero C statements, nothing to transplant. puts (libc2, COMPLETED-C) shares no code shape with a GTE loop. No transplant source exists for either named sibling.
- [s8] Fresh m2c decompile this session (`python3 tools/m2c/m2c.py -t mips-ido-c -f func_8004A76C asm/funcs/func_8004A76C.s`, tmp/grind/func_8004A76C/s8/m2c_out.c/err.txt): m2c emits `void func_8004A76C(s32 arg0, void *arg2)` — only TWO formal parameters (mapped from $a0, $a1). It never assigns $s0 (the vertex-pointer base the asm actually reads/increments 16x) a parameter slot at all; every `lwc2 $N, ($s0)` / `rtpt` / `mfc2` comes back `M2C_ERROR(/* unknown instruction */)` and $s0 appears only inside those opaque error stubs, never as a declared variable. This is an INDEPENDENT tool (m2c's own register-to-parameter heuristic, not our hand analysis) landing on the same fact as the s1 ABI class kill: the function's real third input lives in a register C function signatures cannot place there (only $a0-$a3 bind to formal params under the standard MIPS o32-derived convention GCC emits). GTE unsupported in m2c as expected (consistent with AGENTS.md inline-asm policy — GTE has no C form).
- [s8] Fresh decomp.me corpus similarity search this session over the existing 461-scratch cache (`python3 tools/decomp_me_scrape.py search --asm-file asm/funcs/func_8004A76C.s --corpus tmp/decomp_me_corpus --top 10`, tmp/grind/func_8004A76C/s8/decomp_me_search.txt): best hit similarity 0.011 (RecordButtonsToDevkit, an unrelated devkit I/O function) — no GTE-shingled scratch in the corpus resembles this function's asm at all, a null result on the SAME corpus the sibling's s12 already found null against its own target. Confirms (independently, against a different target asm) that this 461-entry gcc2.7.2-psx corpus contains nothing GTE-pipelined to mine.
- [s8] docs/reference/sotn-construct-index.md has zero hits for `rtpt` or `gte_rtpt` — no SOTN-master precedent exists for representing a hand-pipelined GTE rtpt loop as a C-visible construct (checked per the family-selection table's mandate to grep the index before any precedent claim).
- [s8] Kengo corpus (candidate transplant source named in the rederive modality description) was permanently deleted 2026-10-07 (owner: "a rabbit hole") per memory/reference/external-references.md; not reintroducible, not checked further.
- [s8] sandbox --disable all re-measured this session: {"score": 37, "no_c_body": true} — floor unchanged, stub chassis confirmed current.

- [s8] sandbox --disable all re-measured this session: {"score": 37, "no_c_body": true} -- floor unchanged, stub chassis confirmed current.

- [s8] docs/reference/sotn-construct-index.md has zero hits for 'rtpt' or 'gte_rtpt' -- no SOTN-master precedent exists for representing a hand-pipelined GTE rtpt loop as a C-visible construct.

- [s8] Kengo corpus (the rederive modality's named transplant source) was permanently deleted 2026-10-07 (owner: 'a rabbit hole') per memory/reference/external-references.md; not reintroducible.

- [s8] All four independent not-GCC signals from prior sessions stand unchanged: $s0 caller-owned ABI (s1), $t0-$t2 non-first-fit mfc2 seats (s6, global.c:1057), reorg.c-contradicting delay-slot content (s7, reorg.c:2990), and handwritten mfc2 markers in the asm itself.

## s9 (forensics)
- [s9] Re-audit: rejected/psyq-rtpt-pair-reorg-slot-residual.c = sandbox 32 on the current stub chassis (tmp/grind/func_8004A76C/s9/sandbox_psyq_pair.txt), 5 source-level hunks; fake_ablate finds no FAKE construct. Stub = 37 (s9/sandbox_stub.txt).
- [s9] Correction to s7's attribution of the jr slot: in this cc1 the epilogue is RTL (mips_expand_epilogue, tools/gcc-2.7.2/config/mips/mips.c:5336 emits return_internal at :5369), so the jr $ra slot is filled by the ordinary fill_simple_delay_slots backward scan (reorg.c:2969), NOT the DELAY_SLOTS_FOR_EPILOGUE block (reorg.c:3257) and not the textual function_epilogue `j $31` path. s7 p_f.i.dbr confirms: `(sequence [(jump_insn 105 ... return_internal) (insn 95 sh ...)])`.
- [s9] PASS-INPUT ENUMERATION for the jr slot (reorg.c:2969 scan + stop_search_p reorg.c:719-745): the scan stops on CODE_LABEL (labels_p=1), JUMP_INSN, BARRIER, SEQUENCE, or any asm insn; it skips NOTEs. Plain sh to a reg+offset address is always eligible (dslot attr mips.md:79-82: only branch/jump/call/load/xfer/hilo/fcmp are dslot=yes). So the empty slot needs one of: (1) a live label between the last store and return_internal, (2) an asm insn last (adds bytes or is an empty-asm barrier -- wall), (3) a volatile/conflicting last store (s7 p_d -- volatile on an ordinary RAM buffer, outside the catalog). (1) measured this session: a trailing `return;` (q_a) is a jump-to-next and is deleted with its label; an in-loop return (q_c) keeps the label and empties the slot but costs a bne-inline exit arm + `j .L1` + slot.
- [s9] PsyQ inline headers (8 cached copies, tmp/gteplan/) contain no macro that moves GTE regs 17-19 into GPRs and none that clobbers $2/$3; the $t0-$t2 mfc2 seats have no header-exact source (complements s6's global.c first-fit class kill).
- [s9] Artifacts: s9/q_a.{c,s,i.*}, s9/q_c.{c,s,i.*} (full -da dumps incl. .jump2/.dbr), s9/cc.sh.

- [s9] The jr ra slot in this cc1 is filled by fill_simple_delay_slots' backward scan from the RTL return_internal (mips.c:5336/5369 -> reorg.c:2969), not by the DELAY_SLOTS_FOR_EPILOGUE block (reorg.c:3257); s7 p_f.i.dbr shows (sequence [return_internal, sh]).

- [s9] Pass inputs that leave that slot empty: a live CODE_LABEL, jump, barrier or asm insn as the last insn before return_internal (stop_search_p reorg.c:719-745), or a volatile/conflicting final store; plain reg+offset sh is always eligible (mips.md:79-82). In-loop return realizes the label input at the cost of an extra jump and inverted exit layout; asm-last is wall; volatile sz is outside the volatile catalog (s7).

- [s9] No PsyQ inline header copy moves GTE 17-19 into GPRs or clobbers $2/$3.

- [s9] Closest banked form re-measured: sandbox 32 (s9/sandbox_psyq_pair.txt); stub 37 (s9/sandbox_stub.txt).

## s10 (forensics)
- [s10] Original PsyQ cc1psx (tools/cc1psx.exe via tools/cc1psx_wrapper.sh) reproduces the port cc1 output byte-for-byte (label spelling aside) on the two closest banked forms, s7 p_f and s9 q_c, and on six new seat-input variants (tmp/grind/func_8004A76C/s10/port_vs_psx.txt). The s6 seat kill (global.c:1057) and s7/s9 delay-slot kills (reorg.c:2969/732) therefore hold for the real compiler, not only for the decompals port.
- [s10] mfc2 seat input census (s10/gen.py, v1..v6): s16/u16 temps -> $2,$2,$2; z[3] -> $3,$2,$2; reversed decl / fused 3-output asm / s32 sz -> $2,$3,$8. No measured input yields the target $8,$9,$10.
- [s10] label_census on p_f.psx.s (s10/label_census_p_f_psx.txt): CFG and predecessor multiset [1,1,2] identical to target; residual is $s0 refs 16 vs 0 and 36 vs 39 insns (the 2 nops + jr nop are assembler-filled in the build under .set reorder).
- [s10] candidate.c unchanged (INCLUDE_ASM stub, ASM-WHOLE canonical GTE grant path); floor 37.

- [s10] Original cc1psx output equals port cc1 output (label spelling aside) on p_f, q_c and v1..v6 (tmp/grind/func_8004A76C/s10/port_vs_psx.txt): the s6 seat and s7/s9 delay-slot kills hold for the real PsyQ compiler.

- [s10] mfc2 seat census: narrow temps collapse to $2; array gives $3,$2,$2; other shapes $2,$3,$8; target $8,$9,$10 unreached by any measured input.

- [s10] label_census on p_f.psx.s: CFG/predecessor multiset [1,1,2] matches target; residual is $s0 refs 16 vs 0 and assembler-filled nops.

- [s10] candidate.c unchanged (INCLUDE_ASM stub on the ASM-WHOLE canonical GTE grant path); floor 37.

## s11 (rederive) — second independent m2c run + mandated kill re-audit, both re-confirm s8

- Fresh `python3 tools/m2c/m2c.py asm/funcs/func_8004A76C.s` (tmp/grind/func_8004A76C/s11/m2c_out.c,
  m2c_err.txt): identical shape to s8's cached run — `void func_8004A76C(s32 arg0, void *arg2)`,
  TWO formal parameters, zero slot for `$s0` (the vertex-pointer base the asm actually loops on).
  m2c treats every `lwc2`/`rtpt`/`mfc2 $17-19`/`swc2` as `M2C_ERROR(/* unknown instruction */)` —
  cop2/GTE opcodes are outside m2c's MIPS-I model, so the loop body cannot be lowered to C by this
  tool at all, independent of the ABI question. Re-confirms s8's finding with a fresh run (no
  m2c/toolchain version change since s8).
- Sibling re-check (ledger-mandated, func_8004A4E0's candidate.c re-read in full this session):
  unchanged since s8 — still comments-only over its own INCLUDE_ASM stub (ASM-WHOLE class kill,
  jtbl cross-function-target finding), its own s11/s12 rederive entries record the SAME null
  decomp.me/m2c result for ITS body. No transplantable C block exists in either direction; the two
  functions share only the custom callee-saved ABI fact, already banked at this ledger's s1.
- Mandated kill re-audit (floor flat since s5): `fake_ablate.py --candidate
  rejected/psyq-rtpt-pair-reorg-slot-residual.c` reports **zero FAKE constructs** — the kill was
  never propped by a FAKE carrier. Re-measured `sandbox --disable all --candidate
  rejected/psyq-rtpt-pair-reorg-slot-residual.c --diff` on the current chassis: 5/5 hunks now
  classed **source-level** (engine/gtemacro.py strips the unpinned `gte_rtpt`/`lwc2`/`swc2`/`mfc2`
  block entirely since it isn't a PINNED unit, so the diff shows the whole GTE core missing, not a
  register-seat residual) — confirms the s5/s7/s10 frontier note verbatim: the sandbox cannot score
  a GTE body until engine/gtemacro.py pins `gte_rtpt`/`gte_rtpt_b`/`gte_ldv3c`/`gte_stsxy3c`, which
  is an operator-only engine change outside this session's surface.

- [s11] canonical func_8004A76C returns ASM-WHOLE citing an existing whole-function grant already recorded in inline_asm_canonical.txt (not pending).

- [s11] Fresh m2c run reconfirms s8's two-parameter signature with no $s0 slot and M2C_ERROR on every cop2/GTE opcode — m2c cannot lower this body at all, independent of the ABI question.

- [s11] Sibling func_8004A4E0's candidate.c re-read in full this session: unchanged since s8, comments-only, its own s11/s12 independently hit the same null rederive result for its own body — no transplantable block in either direction.

- [s11] fake_ablate.py confirms the closest rejected form carries zero FAKE constructs, so its prior kill was never FAKE-propped.

- [s11] Re-measuring that form on the current chassis still shows 5/5 hunks source-level because engine/gtemacro.py still strips the unpinned gte_rtpt/lwc2/swc2/mfc2 block wholesale — the sandbox cannot score any GTE-bearing C body for this function until that engine-side pin lands, which is outside this session's surface.
- [s12] rederive re-confirmation: fresh m2c invocation on asm/funcs/func_8004A76C.s produces
  the same shape as s8/s11 (void func_8004A76C(s32 arg0, void *arg2), no $s0 param, all GTE ops
  M2C_ERROR). Sibling func_8004A4E0 candidate.c re-checked at its current HEAD (floor 163, 23
  sessions since its s1) -- confirmed still zero-C INCLUDE_ASM, no transplant material. chassis
  measurement this session: sandbox --disable all on the unedited stub = 37 (matches ledger
  floor); --candidate memory/grind/func_8004A76C/candidate.c is unscorable (engine strips the
  whole unpinned GTE block before linking, same gap banked s5/s7/s10).

- [s12] chassis measurement this session: sandbox func_8004A76C --disable all on the unedited INCLUDE_ASM stub scores 37 (matches ledger floor, cheat_asm_stripped=71).

- [s12] sandbox --disable all --candidate memory/grind/func_8004A76C/candidate.c is unscorable this session ('func_8004A76C not found' after cheat-asm strip) -- consistent with s5/s6/s7/s10's banked finding that engine/gtemacro.py strips the whole unpinned gte_rtpt/lwc2/swc2/mfc2 block before the GTE body can be scored.

- [s12] Third independent m2c invocation (fresh process, not s8/s11's cache) reproduces the identical lowering failure: 10 M2C_ERROR sites for the 6 lwc2 + rtpt + 3 mfc2 + 3 swc2 GTE instructions, and emits no $s0 parameter -- m2c's generic ABI model cannot represent the custom callee-saved $s0 handle shared with func_8004A4E0/func_8004A808/func_8004A940.

- [s12] Sibling func_8004A4E0 (func_8004A76C's own caller/cluster-mate) carries zero C across 23 of its own grind sessions and an owner-ruling (Q40) ASM-WHOLE grant -- there is no sibling C to inherit, confirming s8's null transplant finding is still current.

- [s13] Structural-modality re-audit: the closest rejected form (rejected/psyq-rtpt-pair-reorg-slot-residual.c) re-measured on the current chassis holds -- fake_ablate finds zero FAKE constructs, and --diff shows the same 5/5 source-level hunks (all the stripped lwc2/c2-rtpt/swc2/mfc2 GTE opcodes) as s9/s11. The nine prior modalities (recon/structural x1/permuter/enumerate/synthesis/solver/forensics x2/rederive x3) plus this session's structural re-pass now agree: the only scored residual is the unpinned engine/gtemacro.py macro strip, an operator-only engine gap, not a C-shape question.
- [s13] s10's six-variant z-temp sweep (s16/u16/reversed-decl/array/fused-asm/narrow-sz) already exhausts the one place a structural var-split/decl-order/narrowing lever could move this chassis's register seats; none reached the target's $t0-$t2. No new structural probe was found worth measuring separately this session.

- [s13] sandbox func_8004A76C --disable all on the INCLUDE_ASM stub still scores 37 (no_c_body=true) this session.

- [s13] fake_ablate.py on rejected/psyq-rtpt-pair-reorg-slot-residual.c reports zero FAKE-annotated constructs, confirming the kill was never FAKE-propped (re-confirms s9/s11).

- [s13] sandbox --diff on that rejected form: target 37 insns / ours 16 insns / 5 hunks, 5 source-level / 0 operand-only / 0 not-scored -- all five hunks are the stripped lwc2/c2-rtpt/swc2/mfc2 GTE opcode block.

- [s13] This is the 9th modality pass (recon/structural/permuter/enumerate/synthesis/solver/forensics x2/rederive x3, now structural x2) to independently land on the same conclusion: a custom non-GCC $s0-callee-saved ABI shared with func_8004A4E0/func_8004A808/func_8004A940, plus an unpinned GTE macro block the sandbox cannot score until engine/gtemacro.py gains PINNED units for gte_rtpt/gte_rtpt_b/gte_ldv3c/gte_stsxy3c.

- [s13] Sibling func_8004A4E0's candidate.c (23 sessions, floor 163) remains pure INCLUDE_ASM comments -- no C to transplant for this function's GTE body either.

- [s14] structural modality (mandated): mandated kill re-audit re-run on the closest
  rejected form (rejected/psyq-rtpt-pair-reorg-slot-residual.c) — fake_ablate finds zero
  FAKE constructs (nothing to ablate), confirming s13's reading stands unchanged.
  sandbox --diff on the current candidate.c (INCLUDE_ASM stub) cannot even produce a diff:
  the cheat-disabled build drops func_8004A76C entirely once the unpinned GTE macro block
  is stripped ("func_8004A76C not found in tmp/sandbox/func_8004A76C/main/3AB48.o"). This
  reconfirms the scored residual is wholly the engine/gtemacro.py PINNED-units gap, not a
  C-shape question, so no structural lever (var split, decl order, narrowing,
  re-association) has anything live to attack on this chassis — same conclusion as s13,
  now re-measured this session per the mandated re-audit.

- [s14] fake_ablate on rejected/psyq-rtpt-pair-reorg-slot-residual.c reports zero FAKE-annotated constructs (nothing to ablate) — same as s13.

- [s14] sandbox --disable all --candidate memory/grind/func_8004A76C/candidate.c --diff fails with 'func_8004A76C not found in tmp/sandbox/func_8004A76C/main/3AB48.o' — the GTE-macro-stripped stub cannot be scored at all on the current chassis, confirming the engine/gtemacro.py PINNED-units gap (frontier item 2) is still the live blocker, not a C-form question.

- [s14] The stub (no_c_body) alone scores 37 (target_insns=37, build_insns=0) via tools/wteng.ps1 main sandbox func_8004A76C --disable all this session.

- [s14] Function is an ASM-WHOLE canonical GTE grant per .claude/rules/inline-asm-policy.md and completion-bar.md item 2 (GTE/cop2 has no C form) — 13+ prior sessions across solver/forensics/rederive/structural modalities all independently reconfirm this; candidate.c header (s1/s5/s7/s8) and the ledger's live frontier already state this is the correct final disposition, with escalation modality (not mandated this session) as the next step to file the decisions.md entry.

- [s15] rejected/psyq-rtpt-pair-reorg-slot-residual.c re-measured --diff on the current chassis: target 37 insns / ours 16 insns, 5 hunks, 5/5 source-level, 0 operand-only, 0 not-scored. Hunks 2 and 4 are the complete lwc2-sextuple+c2-rtpt+addiu macro expansions, wholly absent from "ours" (not a register-seat question — the instructions are not emitted at all). Hunks 3/5 are the surviving mfc2/sh/branch skeleton with register seats $v0/$v1/$a1 + bnez vs target's $t0-$t2 + beqz.

- [s15] ENUMERATION (mandated modality): tools/spelling_enum.py on the `mid:` block (gte_stsxy3c call, sxy+=3, 3x gte_mfc2 read, if(n!=0){n--;gte_ldv3c;v+=3;goto loop}) of the rejected form — the one region with any statement-order freedom in the surviving skeleton. 8 anchors, 3 assignments, 0 named locals -> 6 distinct spellings; re-run with the commutative-swap axis enabled produces the identical 6 (no swappable commutative pairs here). All 6 variants (tmp/grind/func_8004A76C/s15/enum/v0..v5.c), measured individually via `sandbox func_8004A76C --disable all --candidate <variant>`, score exactly 32 — zero hit, zero movement from the un-enumerated form's score. ENUMERATION: 6 spellings, best 32, 6 at the floor.

- [s15] Interpretation: the enumerate sweep is exhaustive over the skeleton region's spelling space (by the tool's construction) and finds nothing, consistent with s7/s9/s11/s13/s14's independent finding that the scored residual is dominated by engine/gtemacro.py's wholesale strip of unpinned GTE macro blocks (hunks 1/2/4), not a statement-order/declaration/type question in the surviving C. No enumerate-axis frontier remains on this region. The stub stays; the live frontier (GTE macro pinning, escalation disposition) is unchanged from s14.

- [s15] rejected/psyq-rtpt-pair-reorg-slot-residual.c re-measured --diff this session: target 37 insns / ours 16 insns, 5 hunks, 5/5 source-level. Hunks 2 and 4 are the complete lwc2-sextuple+c2-rtpt+addiu macro expansions, wholly ABSENT from ours (not a register-seat question -- the opcodes are not emitted at all, confirming engine/gtemacro.py strips them wholesale). Hunks 3/5 are the surviving mfc2/sh/branch skeleton ($v0/$v1/$a1+bnez ours vs $t0-$t2+beqz target).

- [s15] tools/spelling_enum.py on the `mid:` block (gte_stsxy3c; sxy+=3; 3x gte_mfc2; if(n!=0){n--;gte_ldv3c;v+=3;goto loop}) -- the only statement-order freedom in the surviving (non-macro) skeleton -- enumerates exactly 6 spellings with or without the commutative-swap axis (no swappable pairs present).

- [s15] All 6 enumerated variants (tmp/grind/func_8004A76C/s15/enum/v0.c..v5.c) measured individually via sandbox --candidate score exactly 32, matching the un-enumerated rejected form and s10's prior six hand-authored structural variants (s16/u16/reversed-decl/array/fused-asm/narrow-sz) -- three independent sweep methods (s10 hand, s13 re-audit, s15 mechanical enumerator) now agree: this skeleton region has zero live spelling lever.

- [s15] candidate.c and hypotheses.md updated with the s15 enumerate entry and histogram; evidence.md updated with the --diff re-measurement and enumeration results.

- [s15] No change to the function's disposition: ASM-WHOLE GTE grant (custom callee-saved $s0 ABI, scan_hand_coded STRONG 1/8 S7, cc1psx-confirmed real-compiler residual, 9 prior sessions across forensics/rederive/structural/synthesis/solver all independently reconfirming) remains correct; this session's enumerate-modality sweep adds a 5th independent confirmation method and closes the one remaining untried systematic-search axis on the surviving skeleton.

## s16 (enumerate) — spelling_enum search space now FULLY exhausted, not just the mid: block
- Widened spelling_enum beyond s15's `mid:` block to all three other blocks (prologue,
  `loop:`, epilogue). Both come back "1 distinct spelling" (0 named locals, 0 assignments) --
  the tool's model has nothing to permute there (no scalar-local assignment statements),
  so the systematic spelling sweep required by the `enumerate` modality now covers the
  WHOLE function body: three blocks with a degenerate size-1 space (nothing to enumerate)
  plus s15's one block with a real size-6 space (already swept, zero hit). This closes the
  enumerate modality for this function: no further spelling axis exists anywhere in it.
  Reinforces (does not supersede) s1's scan=STRONG/1-8 + s9/s10/s13/s14's engine-pin-gap
  finding: the ASM-WHOLE canonical GTE grant remains the correct disposition.

- [s16] sandbox func_8004A76C --disable all this session: score 37, target_insns 37, build_insns 0, no_c_body true -- current chassis is still the INCLUDE_ASM stub; floor unchanged at 37.

- [s16] The prior (discarded) session's s16 hypothesis in memory/grind/func_8004A76C/hypotheses.md claimed kill_scope class with a predicate_cite that was a tool description, not a file:line/commit -- corrected this session to kill_scope instance with no predicate_cite, matching the driver's stated rule that an empty search result is an instance kill.

- [s16] s15's class kill (hypotheses.md, mid: block, 6 spellings swept, predicate_cite engine/gtemacro.py:82 + engine/gtemacro.py:472) is untouched by this correction -- it already carried a real file:line citation and is not the entry that was discarded.

- [s16] The spelling-enumeration search space for this function's surviving (non-macro) skeleton is now fully mapped across all four blocks: three are degenerate (size 1, nothing to permute) and one (mid:, the three gte_mfc2 scalar reads) has a real size-6 space already swept at s15 with zero hit.

- [s16] No change to the function's disposition: ASM-WHOLE canonical GTE grant (custom callee-saved $s0 ABI not GCC-producible, scan_hand_coded tier STRONG 1/8 S7, cc1psx-confirmed real-compiler residual via s10, engine/gtemacro.py PINNED-units gap blocking any score on hunks 1/2/4) stands, reconfirmed across 10+ sessions and 5 independent methods (hand-authored s10, structural re-audit s13/s14, mechanical enumerator s15/s16).

## s17 (enumerate, repeat dispatch) — mandated kill re-audit; no new spelling-enum frontier exists to sweep

- [s17] `sandbox func_8004A76C --disable all` this session on the plain INCLUDE_ASM stub: `{"score": 37, "target_insns": 37, "build_insns": 0, "no_c_body": true}` -- floor unchanged.

- [s17] Mandated kill re-audit (floor flat 9 sessions: s9-s17) on the closest banked form, rejected/psyq-rtpt-pair-reorg-slot-residual.c: `tools/fake_ablate.py --func func_8004A76C --file main/3AB48 --candidate <that file>` reports "no FAKE-annotated constructs found; nothing to ablate" (the kill was never FAKE-propped -- 5th consecutive confirmation of this, after s7/s9/s11/s13/s14).

- [s17] `sandbox func_8004A76C --disable all --diff --candidate rejected/psyq-rtpt-pair-reorg-slot-residual.c` fresh this session (tmp/grind/func_8004A76C/s17/diff_reaudit.txt): score 32, target_insns 37, build_insns 16, 5/5 hunks source-level, 0 operand-only, 0 not-scored. Hunks 1/2/4 are the unpinned gte_rtpt/gte_ldv3c/gte_stsxy3c macro-strip (engine/gtemacro.py:82/:472), unchanged from every prior re-audit. Hunk 3's mfc2 seats land at $2(v0)/$3(v1)/$a1 in this cc1 invocation (vs the $t0-$t2 wording quoted in the older s6/s10 entries) -- same global.c:1057 first-fit mechanism producing a different concrete allocno name; the score masks seat identity either way, so this is not a new residual, just a restated instance of the already-banked seat-mismatch class.

- [s17] Re-read s15/s16 in full: the spelling_enum search space over the surviving (non-macro) skeleton is still fully mapped -- one real region (`mid:`, size 6, swept zero-hit at s15, class-killed predicate_cite engine/gtemacro.py:82+472) and three degenerate size-1 regions (prologue/loop/epilogue, s16). No widening of the ENUM markup beyond s16's four-block coverage is available: every remaining statement in the rejected form is either a macro call, a pointer/array store, a decrement, or a goto/label -- all fixed-position anchors in spelling_enum.py's model, not candidates for a fifth region.

- [s17] No change to the function's disposition: ASM-WHOLE canonical GTE grant stands, now reconfirmed across 11 sessions (s1/s5-s17) and 5 independent methods; this session's contribution is the mandated re-audit holding clean (zero FAKE, zero drift) rather than a new search axis, since none exists under `enumerate` beyond what s15/s16 already closed.

- [s17] sandbox func_8004A76C --disable all on the plain INCLUDE_ASM stub this session: score 37, target_insns 37, build_insns 0, no_c_body true -- floor unchanged.

- [s17] fake_ablate.py on the closest rejected form reports 'no FAKE-annotated constructs found; nothing to ablate' -- 5th consecutive confirmation (after s7/s9/s11/s13/s14).

- [s17] sandbox --diff --candidate on that form this session: score 32, 5/5 hunks source-level, 0 operand-only, 0 not-scored; hunks 1/2/4 the unpinned GTE macro strip, hunk 3 the mfc2-seat mismatch ($2/$3/$a1 this invocation).

- [s17] spelling_enum search space (s15 mid: block size 6 zero-hit, s16 prologue/loop/epilogue degenerate size-1) remains fully mapped; no fifth region or wider ENUM markup is available on this chassis.

- [s17] canonical func_8004A76C gate already returns ASM-WHOLE (inline_asm_canonical.txt grant, confirmed s1); disposition is operator-side completion, not a C-search question.

## s18 (synthesis)
- [s18] Kill re-audit: fake_ablate.py finds zero FAKE constructs in rejected/psyq-rtpt-pair-reorg-slot-residual.c and rejected/pipelined-goto-wrong-abi-stripped-gte.c (tmp/grind/func_8004A76C/s18/ablate_*.txt); sandbox --disable all --diff re-measures 32 and 30, both 5 source-level / 0 operand-only / 0 not-scored (tmp/grind/func_8004A76C/s18/sandbox_*.txt); INCLUDE_ASM stub 37 (sandbox_stub.txt). No drift since s5/s7.
- [s18] Global-register-variable diagnostic (wall-banned pin, measured only to separate the ABI residual from the others): the s7 p_f body with `register SVECTOR *v asm("$16")` at file scope and a 3-arg signature compiles (port cc1 -da and original cc1psx, byte-identical: tmp/grind/func_8004A76C/s18/port_vs_psx.txt empty) to the target's exact CFG, GTE op placement, entry nop pad, $16 base with no prologue save, j-into-middle with `addu $16,24` in the slot, beqz slot `addu $4,-1` -- and three residuals: (a) mfc2 seats $2/$3/$7 (g_s0.i.greg "75 in 2 76 in 3 77 in 7") vs target $8/$9/$10, i.e. freeing $a3 hands it to the third temp, still first-fit (global.c:1057); (b) `addu $5,12` before the mfc2s (target after, reorg.c:2990 shape); (c) jr slot holds `sh $7,4($6)` (target nop, reorg.c:2969 backward scan). So the $s0 ABI is NOT the only GCC-unreachable feature: even with it granted by the forbidden pin, the seats and both delay-slot facts remain. tmp/grind/func_8004A76C/s18/g_s0.s, g_s0.psx.s.
- [s18] Synthesis of s1-s17: four independent not-GCC signals stand on measured predicates -- unsaved $s0 input/output ABI (s1, caller func_8004A4E0 hands $s0 both ways), $t0-$t2 seats with $2/$3/$7 free (s6, global.c:1057; s18 re-confirms with $a3 freed), the addiu-a1/beqz/a0-decrement slot combination (s7, reorg.c:2990), and the empty jr slot after an eligible sh (s9, reorg.c:2969) -- plus splat's `/* handwritten instruction */` markers on the three mfc2. Every C-side axis (structural s2/s13/s14, permuter s3, enumerate s4/s15/s16/s17, synthesis s5, solver s6, forensics s7/s9/s10, rederive s8/s11/s12) is measured. Merged best form = rejected/psyq-rtpt-pair-reorg-slot-residual.c (sandbox 32; raw cc1 38 vs 39 insns).

- [s18] fake_ablate: no FAKE constructs in rejected/psyq-rtpt-pair-reorg-slot-residual.c or rejected/pipelined-goto-wrong-abi-stripped-gte.c; sandbox 32 and 30, 5/5 source-level each; stub 37.

- [s18] Even with the $s0 ABI granted via a wall-banned global register variable, the target's $t0-$t2 mfc2 seats, the addiu-a1-after-mfc2 beqz slot combination and the empty jr slot are not emitted by port cc1 or cc1psx; the three non-ABI not-GCC signals are independent of the ABI signal.

- [s18] Synthesis of s1-s18: four independent measured not-GCC signals (s1 ABI, s6/s18 seats global.c:1057, s7 beqz slot reorg.c:2990, s9 jr slot reorg.c:2969) plus splat /* handwritten instruction */ mfc2 markers; every modality except escalation has run; the canonical entry inline_asm_canonical.txt:25 stands.

- [s19] (solver) Kill re-audit holds: psyq-rtpt-pair-reorg-slot-residual.c scores 32, 5/5 source-level, zero FAKE (tmp/grind/func_8004A76C/s19/sandbox_reaudit.txt).
- [s19] inverse_compose classify on the sandbox objects reports PRE-RA, but only because the sandbox strips the unpinned GTE units (16 vs 37 insns). On the raw cc1 stream (s19/a3_raw_cc1.s) the multiset matches target except the jr-slot nop: residual = RA seats + reorg slots.
- [s19] RA model of the $a3 chassis (s19/a3.model.json) is exact (simulate 7/7). inverse.py global depth 3: NEGATIVE for the full target disposition (v->$s0, z->$t0-$t2) and for the ABI-granted goal (v->$a3, z->$t0-$t2). $t0-$t2/$s0 never appear as hard regs pre-RA, so their preference atoms are foreclosed; z1-z3 conflict with no hard reg except $sp, so first-fit lands on $v0/$v1/$t0. The sched solver does not apply: the addiu-a1 position and both delay slots are reorg.c products (s7/s9 class kills), downstream of sched_solver's model.

- [s19] Re-audit: psyq-rtpt-pair-reorg-slot-residual.c = 32, 5/5 source-level, zero FAKE (s19/sandbox_reaudit.txt, s19/ablate_reaudit.txt).

- [s19] ra_solver global model of the $a3 chassis is exact: simulate 7/7 dispositions (s19/simulate.txt).

- [s19] inverse.py depth 3 NEGATIVE for the full target disposition and the ABI-granted subset (s19/inverse_full.txt, s19/inverse_a3.txt).

- [s19] classify reports PRE-RA on the sandbox objects only because of the unpinned-GTE strip. The raw cc1 multiset matches target except the jr-slot nop, a reorg product.

- [s19] The sched solver does not apply: the addiu-a1 placement and both delay slots come from reorg.c (s7/s9 class kills), downstream of the sched model.

- [s19] The function is already listed at inline_asm_canonical.txt:25 (custom $s0 ABI).

- [s20] (forensics) Re-audit: psyq-rtpt-pair-reorg-slot-residual.c = 32, 5/5 source-level, zero FAKE (s20/sandbox_reaudit.txt, s20/ablate_reaudit.txt).
- [s20] Dumps of the closest form (s20/p.i.lreg, p.i.greg): z1-z3 (pseudos 76-78) are live at the start of blocks 1 and 3 -> global pseudos; local-alloc does not decide them. greg conflicts for 76-78 = pseudos + $sp only, no preferences; dispositions 76->$2 77->$3 78->$8. The s19 frontier "local-alloc suggestion pass" is closed by dump read.
- [s20] label_census (s20/label_census.txt): CFG identical to target (3 labels, preds [1,2,1] in order). Diffs are only $s0 refs 16 vs 0 (ABI) and the jr-slot fill. Residual = ABI + mfc2 seats + reorg slots, as s18/s19.

- [s20] Re-audit: closest form scores 32, 5/5 source-level, zero FAKE (tmp/grind/func_8004A76C/s20/sandbox_reaudit.txt).

- [s20] z1-z3 (pseudos 76-78) are global pseudos: live at start of BB1 and BB3; conflicts only pseudos + $sp; seats $2/$3/$8 by global first-fit (s20/p.i.lreg, s20/p.i.greg).

- [s20] label_census: CFG identical to target; residual is $s0 ABI + mfc2 seats + reorg delay slots, consistent with s18/s19.

- [s20] canonical gate reports whole-function canonical-asm grant (inline_asm_canonical.txt:25).

- [s20] tools/grinder/dump.ps1 fails for subdirectory TUs (FileNotFoundError tmp/grind/func_8004A76C/dumps/main/3AB48.c); dumps produced via s20/cc.sh instead.
- [s21] Re-audit holds: closest form 32, 5/5 source-level, zero FAKE (s21/sandbox_reaudit.txt, ablate_reaudit.txt); canonical gate = whole-function grant inline_asm_canonical.txt:25 (s21/canonical.txt).
- [s21] nrefs_census on the spliced closest form (s21/nrefs_census.txt): allocno order 74,75,72,73,76,77,78; dispositions z1->$2 z2->$3 z3->$8. Allocation order cannot reach $8-$10: z conflicts are pseudos+$sp only, so first-fit takes $2 at any rank. Target leaves $2,$3,$7 all free yet uses $8-$10 — not a first-fit disposition (fifth restatement of the s6 seat signal, now with order eliminated).
- [s21] loop_movables over s20 dumps: no non-phony loops (goto-built loop has no LOOP_BEG), loop.c inert (s21/loop_movables.txt). tools/grinder/dump.ps1 on the spliced TU failed ("dumps/main/3AB48.rtl: No such file or directory", s21/dump_log.txt) — s20's standalone -da dumps of the same form were used instead.

- [s21] canonical func_8004A76C reports the whole-function canonical-asm grant (inline_asm_canonical.txt:25).

- [s21] Closest form re-measures 32 with 5/5 source-level hunks (GTE macro strip) and zero FAKE constructs.

- [s21] nrefs_census allocno order 74(a2) 75(a3) 72(a0) 73(a1) 76($2) 77($3) 78($8); priority order cannot change z1-z3's first-fit seats because they conflict with no hard reg but $sp.

- [s21] loop_movables: no non-phony loops in the goto-built form; loop.c makes no decision.

- [s21] tools/grinder/dump.ps1 on the spliced TU failed with 'dumps/main/3AB48.rtl: No such file or directory'; s20's standalone -da dumps of the same form were used instead.

- [s22] decomp.me corpus directory tmp/decomp_me_corpus held exactly 461 cached gcc2.7.2-psx scratches both at s8 and at s22 -- the 316->461 growth seen in sibling func_8004A4E0's s12 predates this function's own s8, so no new scratches existed to find this session.

- [s22] tmp/grind/func_8004A76C/s22/decomp_me_search_461.txt is byte-identical to s8's decomp_me_search.txt: best similarity 0.011 against RecordButtonsToDevkit, no GTE-shaped candidate in the top 10.

- [s22] m2c's output (s12/m2c_out.c) emits M2C_ERROR for every lwc2/rtpt/swc2/mfc2 cop2 instruction; tools/m2c/m2c_macros.h's 'valid syntax' unknown-instruction macros exist for arbitrary unhandled instructions in general but GTE opcodes have no C semantic to lower to under any m2c invocation -- this is the AGENTS.md inline-asm-policy fact (GTE has no C form), not a routable tool limitation.

- [s22] Sibling func_8004A4E0/candidate.c re-read in full: unchanged since 2026-10-15 17:03 (comments-only, ASM-WHOLE, floor 163), nothing transplantable onto this chassis -- consistent with this ledger's own prior s18 read.

- [s22] sandbox --disable all this session: {"score": 37, "scorable": true, "target_insns": 37, "build_insns": 0, "no_c_body": true, "cheat_asm_stripped": 71} -- floor unchanged from s14-s21.

- [s22] CC1PSX SELF-DISPROOF (driver): check did not produce a scorable result (cc1psx produced no scorable object: 'func_8004A76C not found in tmp/cc1psx/func_8004A76C/psx.o'); rotation proceeds on the source-side assumption.

- [s23] (escalation) scan_hand_coded --single: tier=STRONG 2/12 (S7 unsaved $s0, S11 live-in $s0) — gate (a) PASSES (s23/scan.txt). canonical gate: whole-function grant inline_asm_canonical.txt:25 (s23/canonical.txt). Stub sandbox 37 (s23/sandbox_stub.txt).
- [s23] Kill re-audit: psyq-rtpt-pair-reorg-slot-residual.c = 32, 5/5 source-level, zero FAKE (s23/sandbox_reaudit.txt, s23/ablate_reaudit.txt) — unchanged on HEAD 07d619ccf.
- [s23] Disposition filed: docs/grind/decisions.md "## 2026-10-15 — func_8004A76C — CANONICAL-ASM GRANT PATH". Remaining step is operator integration (layer-2 review + queue done as COMPLETED-INLINE-ASM-CANONICAL).

- [s23] scan_hand_coded --single func_8004A76C: tier=STRONG 2/12 (S7, S11 on $s0) (tmp/grind/func_8004A76C/s23/scan.txt)

- [s23] canonical func_8004A76C: whole-function canonical-asm grant in inline_asm_canonical.txt (row 25)

- [s23] sandbox --disable all on INCLUDE_ASM stub: score 37 (s23/sandbox_stub.txt)

- [s23] closest pure-C form re-measures 32, 5/5 source-level, zero FAKE (s23/sandbox_reaudit.txt, s23/ablate_reaudit.txt)

- [s23] driver cc1psx_check: ok=false, function not in psx.o (stub candidate, vacuous)

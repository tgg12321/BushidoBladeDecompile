# Evidence bank — _exeque

## [s1] recon — floor 187 (raw INCLUDE_ASM) -> 15

OBJECT MODEL: the DATA MODEL section flagged `_que_plus_0x4` / `_que_plus_0x8`
as SPLIT-AGGREGATE (no header decl) with a comment in include/gpu.h:70-80
saying the merge should land "when _exeque leaves INCLUDE_ASM". MATCHES: the
struct is already declared (`extern volatile GpuQueueItem _que[64];`,
include/gpu.h:81-88, fields func@0/arg@4/count@8/data@0xC) and already
consumed via `_que[idx].field` in the sibling `_addque2` (src/display.c:891-930,
not INCLUDE_ASM — already a completed pure-C consumer of the SAME struct).
Writing `_exeque` against `_que[D_8009BF7C].func/.arg/.count` (never touching
`_que_plus_0x4`/`_que_plus_0x8` as bare externs) is therefore hypothesis #1 and
IS the correct object model — measured: canonical distance dropped
187 -> 24 the moment the function was written in C at all (verdict flips
INCLUDE_ASM -> "C", pure-C distance 24/187), and the instruction-level diff
(tmp/grind/_exeque/s1/diagdiff.py) shows the struct-based `_que[idx].field`
sequences for the CALL SITE (`_que[D_8009BF7C].func(...)`) match target's
per-field base-address recompute pattern byte-for-byte in that region. The
struct declaration itself is NOT the source of the remaining residual (see
below) — no further data-model work is indicated for this function.

Baseline: `canonical _exeque` before any edit reported verdict C, pure-C
distance 187 (== raw INCLUDE_ASM asm-insn count; nothing had been attempted).
Target asm/funcs/_exeque.s is 187 real instructions (33 of them plain `nop`,
mostly load-delay slots since the target is essentially unscheduled /
strictly source-ordered around the volatile `_que`/`D_8009BF7C`/`D_8009BF78`
accesses).

### Full manual decode of asm/funcs/_exeque.s (verified against the C written this session)

```
s32 _exeque(void) {
    if (*D_8009BF54 & 0x01000000) return 1;          /* early exit, masked already */
    mask = SetIntrMask(0);
    D_8009BF84 = mask;
    if (D_8009BF78 != D_8009BF7C && !(*D_8009BF54 & 0x01000000)) {
        do {
            if (((D_8009BF7C+1)&0x3F) == D_8009BF78 && D_8009BE80 == 0)
                DMACallback(2, 0);
            while (!(*D_8009BF48 & 0x04000000)) {}
            _que[D_8009BF7C].func(_que[D_8009BF7C].arg, _que[D_8009BF7C].count);
            D_8009BF68[0] = (s32)_que[D_8009BF7C].func;
            D_8009BF6C  = (s32)_que[D_8009BF7C].arg;
            D_8009BF70  = _que[D_8009BF7C].count;
            D_8009BF7C  = (D_8009BF7C+1)&0x3F;
        } while (D_8009BF78 != D_8009BF7C && !(*D_8009BF54 & 0x01000000));
    }
    SetIntrMask(D_8009BF84);
    if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000)
        && D_8009BE7C != 0 && D_8009BE80 != 0) {
        D_8009BE7C = 0;
        ((s32(*)(void))D_8009BE80)();
    }
    return (D_8009BF78 - D_8009BF7C) & 0x3F;
}
```
This decode is measured CORRECT at the control-flow level: with it written
verbatim, sandbox distance is 15/187 (built 186 insns vs target 187), and the
remaining diff (below) is pure instruction-scheduling/ordering, not a missing
or wrong branch/condition.

### Bug found + fixed this session: `D_8009BF84` was never declared

`D_8009BF84` (the saved interrupt mask that _exeque writes at entry and reads
before the unlock `SetIntrMask(D_8009BF84)` call) had **no extern declaration
anywhere in src/display.c** before this session — a genuine bug, not a style
choice, that predates this session's edits (nothing in the file referenced it
before _exeque was written). GCC 2.7.2 accepted the undeclared identifier
silently (implicit extern) but this produced observably wrong codegen for the
`SetIntrMask(D_8009BF84)` call (the built object loaded `a0 = 0` into the
call instead of the global's value — confirmed via
`objdump -dr --disassemble=_exeque` showing `move a0,zero` in the delay slot
of the `jal SetIntrMask` reached via `R_MIPS_26 SetIntrMask` relocation,
tmp/grind/_exeque/s1/mine.txt before the fix). Adding
`extern s32 D_8009BF84;` (src/display.c, alongside D_8009BE7C/D_8009BE80)
dropped the sandbox score 24 -> 15 with build_insns 183 -> 186 (closer to
target's 187). Measured via `sandbox _exeque --disable all` both before and
after the one-line addition.

### Remaining residual (floor 15) — pure scheduling, NOT object-model or missing-logic

Using `engine.score.normalized_insns(..., mask=False)` on `build/src/display.o`
(target reference, byte-correct build) vs `tmp/sandbox/_exeque/display.o`
(our cheat-disabled build) — script banked at
tmp/grind/_exeque/s1/diagdiff.py — every remaining diff block after masking
address-only branch-target noise is confined to ONE region: the post-call
"debug record" triple-write

```c
D_8009BF68[0] = (s32)_que[D_8009BF7C].func;
D_8009BF6C    = (s32)_que[D_8009BF7C].arg;
D_8009BF70    = _que[D_8009BF7C].count;
```

Target computes+stores each field STRICTLY IN SOURCE ORDER (load `.arg` ->
immediately `sw` into `D_8009BF6C` -> THEN reload `D_8009BF7C` fresh -> load
`.count` -> immediately `sw` into `D_8009BF70`). Our build's list scheduler
instead loads `.arg` into `$a1` and `.count` into `$a0` WITHOUT storing them
yet, interleaves the `D_8009BF7C = (D_8009BF7C+1)&0x3F` increment and the
loop-continuation reload of `D_8009BF78`/`D_8009BF7C`, and only THEN emits
both `sw $a1,...` / `sw $a0,...` back to back just before the loop-exit
`beq`. Net effect: same instructions, same values, DIFFERENT SCHEDULE — this
is the class of diff [[difficult-is-not-impossible]] and
[[no-compiler-divergence]] describe as "the C source structure, not the
compiler, decides the schedule."

RULED OUT as an object-model / volatile-declaration fix: checked
`volatile_extern_allowlist.txt:75` — the D_8009BF68 grant's own justification
TEXT explicitly says "the SAME printf folds the non-volatile adjacent
siblings D_8009BF6C and D_8009BF70" as the evidence D_8009BF68 needs its own
volatile and they do NOT. So marking D_8009BF6C/D_8009BF70 volatile would
contradict banked project evidence (get_alarm's own closing grant) and is not
the fix — the residual is a pure scheduling/statement-structure question,
not a declaration gap. Did not test it (no need — the census evidence already
rules it out; see OBJECT MODEL note in [[unannotated-fake-inflates-honest-floor]]-adjacent
caution: don't spend a measurement disproving something the ledger already
answers).

- [s1] canonical _exeque baseline (before any edit): verdict C, pure-C distance 187 (raw INCLUDE_ASM insn count).

- [s1] After writing the full struct-based C body: canonical distance 24, sandbox score 24 (build_insns 183 vs target 187).

- [s1] After adding the missing `extern s32 D_8009BF84;` declaration: sandbox score 15 (build_insns 186 vs target 187).

- [s1] engine.score.normalized_insns(mask=False) diff between build/src/display.o (target reference) and tmp/sandbox/_exeque/display.o (our disabled build) shows every remaining real (non-branch-target-masking) diff is confined to the post-call debug-record triple-write (D_8009BF68[0]=func; D_8009BF6C=arg; D_8009BF70=count) - target stores each field immediately after computing it, our build's scheduler groups both non-func stores at the end of the loop body.

- [s1] _addque2 (src/display.c:892-930), the sibling function writing the SAME GpuQueueItem struct, is already pure C and not INCLUDE_ASM - corroborates the struct object model is correct project-wide, not just hypothesized.

- [s1] volatile_extern_allowlist.txt:75 (D_8009BF68 grant) explicitly documents D_8009BF6C and D_8009BF70 as the non-volatile siblings in the same debug-record triple, ruling out a volatile fix for the remaining residual.

- [s2] sandbox --disable all on the applied candidate.c (H5a+H5b, no volatile) measures score 12, build_insns 185, target_insns 187 -- confirmed floor drop from the s1-banked 15.

- [s2] tmp/grind/_exeque/s1/diagdiff.py (existing s1 tool) does a normalized-instruction SequenceMatcher diff between build/src/display.o and tmp/sandbox/_exeque/display.o for _exeque; used repeatedly this session to localize exactly which instructions differ after each edit.

- [s2] tmp/grind/_exeque/s2/fulldump.py (new this session) dumps the full objdump -d listing for _exeque from both build/src/display.o (target) and tmp/sandbox/_exeque/display.o (mine) so the two can be read side-by-side without SequenceMatcher's opcode-level truncation -- this is what revealed the pointer-reuse mechanism for D_8009BE7C.

- [s2] _exeque is installed as an asynchronous DMA-interrupt callback at src/display.c:915/927 (`DMACallback(2, _exeque);`), confirmed by reading the surrounding _addque2 source -- this is the IRQ-writer citation for the H6 ruling-request candidate.

- [s2] The remaining floor-12 residual is exactly two sites: (a) the post-call triple-store block (D_8009BF68[0]/D_8009BF6C/D_8009BF70), unchanged in shape from the s1-banked residual and confirmed immune to pure statement/declaration reordering this session (H4a, H4b); (b) a single jalr delay-slot fill difference in the final-callback block, closable only via an unsanctioned volatile spelling (H6).

- [s3] sandbox _exeque --disable all confirms floor 12/187 unchanged from the s2-banked chassis (build_insns 185, cheat_asm_stripped 143, rules_dropped 0) both before and after this session's edits.

- [s3] src/display.c reverted to its committed INCLUDE_ASM("asm/funcs", _exeque); state before finishing this session (git status/diff on src/display.c show zero delta from HEAD) -- the working candidate lives only in memory/grind/_exeque/candidate.c per asm-until-matched.

- [s3] Fresh pwsh tools/grinder/dump.ps1 _exeque dumps were taken this session (tmp/grind/_exeque/dumps/*) on the floor-12 chassis, superseding s1's stale-chassis dumps referenced in the prior frontier.

- [s3] The three s3 triple-store spellings (mask-reuse, fresh two-locals, direct assignment) plus s2's H4a/H4b statement reorderings are five total independently-measured C-level respellings of this specific block, all byte-identical in the triple-store region -- this is now a well-evidenced (though still instance-scoped per this session's kill_scope discipline) wall for pure statement/variable respelling WITHIN that block.

- [s3] target asm (asm/funcs/_exeque.s lines corresponding to addresses 8007D860-8007D8E4) shows a strict load-store-load-store-load-store interleave per field (recompute -> load .arg -> store D_8009BF6C -> recompute -> load .count -> store D_8009BF70 -> recompute -> increment+store D_8009BF7C) that our build's list scheduler does not reproduce regardless of tested C-level spelling; the deferred-store shape is a scheduling decision, not an addressing/object-model gap (the object model itself, established in s1 H1, remains correct).

## [s4] PERMUTER — floor 12 -> 2 via TWO do-while(0) wraps (SOTN-sanctioned family, both FAKE-annotated)

Built a clean single-function permuter workspace (`tmp/grind/_exeque/s4/build_perm_workspace.sh`,
`tmp/grind/_exeque/s4/perm_ws/`) using the real project pipeline (cpp with the
Makefile's CPP_FLAGS/CPP_DEFS, `tools/gcc-2.7.2/build/cc1` with the mandatory
`-mel -msoft-float`, `prologue_fix.py`, `maspsx.py` with the real
`MASPSX_FLAGS`, `multu_pad.py`), extracting just `_exeque`'s region from the
full-TU compile (awk on `.globl\t_exeque` ... `.end\t_exeque`) so the permuter
score reflects only this function, matched against `asm/funcs/_exeque.s`
assembled with `tools/decomp-permuter/prelude.inc` (r3000, no gp=64). This
supersedes the s3 frontier item "directed permuter run" — it was run this
session, not merely proposed.

Two consecutive campaigns (`tools/permuter_campaign.py launch/harvest`, `-j4
--stop-on-zero`, `--stack-diffs` default ON), each re-seeded from the
previous campaign's best find (chassis discipline: base.c updated to the
current src/display.c state before each relaunch):

1. **Campaign 1** (base permuter score 980, sandbox floor 12/187): within
   ~4700 iterations (waited via `permuter_campaign.py wait`, two windows)
   found `output-615-1` — wrapping the triple-store block
   (`D_8009BF68[0]=...; D_8009BF6C=...; D_8009BF70=...;`) in a single
   `do { ... } while (0);`. Applied to `src/display.c` and measured via
   `sandbox _exeque --disable all`: **floor 12 -> 7** (build_insns 185 ->
   186). All other novel finds in this campaign (`output-715/765/915-*`)
   were `extern volatile int/short/char D_8009BF6C/70` coercions on
   non-IRQ, non-MMIO game-state globals — REJECTED per
   [[legitimate-volatile-interrupt-touched]] (these globals are plain
   debug-record scalars, no IRQ writer, no qualifying use-site shape) and
   [[inline-asm-policy]]'s volatile-coercion catalog entry. `output-905-1`
   (moving `SetIntrMask(D_8009BF84)` inside the loop) was REJECTED as not
   even semantically equivalent — it changes runtime behavior (re-arms the
   interrupt mask every loop iteration instead of once after the loop),
   which the permuter's mutation search does not itself verify.
   `output-837-1` (a `new_var` staged intermediate) scored worse (837) than
   the do-while wrap and was not pursued.
2. **Campaign 2** (base permuter score 615, sandbox floor 7/187, chassis =
   campaign 1's do-while form applied to src): within ~2900 iterations
   found `output-200-1` — a SECOND, NESTED `do { ... } while (0);` wrapping
   just the `D_8009BF68[0]=...; D_8009BF6C=...;` pair (leaving
   `D_8009BF70=...;` and the `D_8009BF7C` increment outside, i.e. splitting
   the single-level wrap from campaign 1 into two nested levels around a
   narrower sub-block). Applied to `src/display.c` and measured: **floor
   7 -> 2** (build_insns 186, unchanged insn count — pure reschedule).
   This is a NESTED wrap, which [[do-while-zero-exception]] requires a
   "single-level-insufficient" justification for: campaign 1's single-level
   wrap around the WHOLE triple-store block measurably left 5 more
   instructions unmatched (floor 7) than nesting a second level around
   just the first two stores (floor 2) — direct A/B measurement on the
   SAME surrounding chassis is the required justification.
3. **Campaign 3** (base permuter score 200, sandbox floor 2/187, chassis =
   campaign 2's nested-do-while form): launched, targeting the single
   remaining residual (identified via direct objdump diff of
   `tmp/sandbox/_exeque/display.o` vs a freshly-rebuilt `target.o`): the
   final IRQ-callback block's `jalr $v0` — target keeps
   `sw $zero,0($v1)` (`D_8009BE7C = 0;`) BEFORE the `jalr`, with an
   explicit unfilled `nop` in the delay slot; our build's `reorg.c`
   delay-slot filler moves the store INTO the jalr's delay slot instead
   (`jalr $v0` / `sw $zero,0($v1)` swapped vs target). This is the EXACT
   same residual s2 already identified and KILLED as only closable via an
   unsanctioned `volatile s32 *p` respelling (H6, rejected — the
   guard-and-clear use-site shape is not one of
   [[legitimate-volatile-interrupt-touched]]'s three catalogued shapes).
   Campaign 3's outcome is recorded in the frontier / kill ledger below.

**Both do-while(0) wraps are FAKE-annotated in candidate.c** per
[[do-while-zero-exception]]'s mandatory-annotation prerequisite; the
nested wrap's justification (single-level insufficient, per direct
A/B floor measurement 7 vs 2) is recorded in the annotation comment.

- [s4] Built a clean single-function permuter workspace (tmp/grind/_exeque/s4/perm_ws/, script tmp/grind/_exeque/s4/build_perm_workspace.sh) using the real project pipeline (cpp with Makefile CPP_FLAGS/CPP_DEFS, tools/gcc-2.7.2/build/cc1 with the mandatory -mel -msoft-float, prologue_fix.py, maspsx.py with the real MASPSX_FLAGS, multu_pad.py), extracting only _exeque's region from the full-TU compile so the permuter score reflects one function, matched against asm/funcs/_exeque.s assembled standalone via tools/decomp-permuter/prelude.inc.

- [s4] Three sequential campaigns, each re-seeded from the previous best chassis (base.c updated to the improved src/display.c before relaunch): campaign 1 (base 980 -> found 615, sandbox floor 12->7), campaign 2 (base 615 -> found 200, sandbox floor 7->2), campaign 3 (base 200, 9096 iterations, 0 novel finds, sandbox floor unchanged at 2).

- [s4] Direct objdump -dr diff of tmp/sandbox/_exeque/display.o against a freshly rebuilt target.o after each floor drop confirmed exactly which region closed each time: campaign 1 closed the entire triple-store region; campaign 2 closed it further with no visible remaining diff in that region; the floor-2 residual is entirely the final-callback jalr $v0 delay-slot fill (target: sw $zero,0($v1) then jalr with unfilled nop delay; ours: jalr with the store in its delay slot) -- the exact same site s2's H6 already identified and could only close via an unsanctioned volatile pointer.

- [s4] All src/display.c edits were reverted to the committed INCLUDE_ASM("asm/funcs", _exeque); state before this session ended, per asm-until-matched -- this is a progress outcome (floor 2, not 0), so no C lands on main.

- [s4] memory/grind/_exeque/candidate.c, evidence.md, hypotheses.md, and self_vet.md were all updated this session with the full campaign trace, the two applied FAKE-annotated do-while(0) constructs, and the two rejected permuter proposals (volatile coercion, behavior-changing statement move).

## [s5] permuter session
- Applied s4's candidate.c to src/display.c (fixed the two forward-declaration
  mismatches: `void _exeque();` -> `extern s32 _exeque(void);` at both call
  sites' TU-scope declarations, added the missing `extern s32 D_8009BF84;`).
  Confirmed via `sandbox _exeque --disable all`: score 2/187, build_insns 186,
  matching the chassis-check number in this session's brief.
- Two hand-derived structural variants on the final-callback block (do-while(0)
  wrap; cb-local hoist) both measured negative -- see hypotheses.md [s5].
  Neither improved nor is a viable direction.
- Re-launched the jalr-delay-slot directed permuter campaign from a fresh copy
  of s4's campaign-3 workspace (tmp/grind/_exeque/s5/perm_ws/, base score 200,
  target.o/base.o carried over unchanged since the residual chassis is
  identical). One ~547s blocking wait call, 15356 iterations, 0 novel finds.
  harvest --stop cleanly killed 5 procs. Combined with s4's 9096 iterations on
  the identical search space, cumulative >=20k -- CHASSIS RULE now applies:
  this exact chassis+residual is not a valid permuter re-seed target for a
  future session without a structural change to the surrounding C first.
- src/display.c left in place with s4's candidate.c body (floor 2/187, no
  regression) at session end -- matches [[asm-until-matched]]: the committed
  representation for an INCOMPLETE function is INCLUDE_ASM, so this working
  tree state is NOT committed; it is the ledger's job (candidate.c) to carry
  the frontier for the next session, which is unchanged from s4's file.
- Artifacts: tmp/grind/_exeque/s5/perm_ws/campaign.log,
  tmp/grind/_exeque/s5/perm_ws/campaign_meta.json.

- [s5] Applied s4's banked candidate.c to src/display.c (fixed two stale forward declarations -- `void _exeque();` -> `extern s32 _exeque(void);` at both the ADDQUE2 block and the _sync block -- and added the missing `extern s32 D_8009BF84;`); confirmed via sandbox _exeque --disable all: score 2/187, build_insns 186, matching the driver's dispatch-time chassis check.

- [s5] Two hand-derived structural variants of the final-callback block (do-while(0) wrap; cb-local hoist) both measured negative and were reverted -- neither is a viable direction for the residual.

- [s5] A second fresh-seed permuter campaign on the jalr-delay-slot residual ran 15356 iterations with zero novel finds; cumulative with s4's prior campaign on the identical chassis+residual now exceeds the ledger's 20k-iteration CHASSIS RULE threshold for re-seeding.

- [s5] src/display.c is left in the working tree with s4's candidate.c body applied (floor 2/187) at session end -- per [[asm-until-matched]] this is NOT committed; candidate.c in the ledger carries the frontier for the next session unchanged in substance from s4, with this session's negative results appended to its header.

- [s6] Re-applied memory/grind/_exeque/candidate.c to src/display.c verbatim (fixing the two forward-decl and one missing-extern issues the header documents) and confirmed sandbox _exeque --disable all == 2/187, matching the ledger's last recorded floor exactly — chassis unchanged since s4.

- [s6] Ran pwsh tools/grinder/dump.ps1 _exeque and read tmp/grind/_exeque/dumps/display.dbr (the reorg.c delay-branch-reorg pass dump, the correct dump for a delay-slot-fill question, distinct from .sched/.sched2 which are the two list-scheduler passes that run BEFORE reorg) around the D_8009BE7C/D_8009BE80 final-callback block. Confirmed the exact mechanism the ledger already named: RTL insn 311 (`(set (mem:SI (reg/v:SI 3 v1)) (const_int 0))`, the `*p = 0;` store with its address pre-materialized in hard reg v1 by insn 298) is folded by reorg.c's fill_simple_delay_slots into a (sequence [call_insn 316 ... insn 311]) -- the D_8009BE80 jalr call_insn followed by the store as its delay-slot fill, because no resource conflict is detected between the store (using hard reg v1, not clobbered/used by the call) and the call_insn.

- [s6] All four fresh structural respellings tried this session (final-block guard operand order, outer-guard operand order, SetIntrMask call relocation/duplication, outer-loop C spelling) either regressed the score or left it unchanged -- none moved the jalr-delay-slot fill. This adds to s4/s5's exhausted permuter search (~24k iterations on this exact residual) and s2/s4/s5's hand-derived variants (do-while wrap: no effect; cb-local hoist: worse; volatile: closes it but not submittable per H6's scope analysis).

- [s7] At session start, src/display.c carried `INCLUDE_ASM("asm/funcs", _exeque);` (NOT s6's candidate applied, despite s5/s6 evidence notes saying it would be left applied) — the CHASSIS CHECK block's "measurement unavailable" at dispatch matches this: whatever reverted it, the dispatch-time honest floor could not be measured because there was no C body in place. This session re-applied memory/grind/_exeque/candidate.c to src/display.c (including the two forward-decl fixups and the missing D_8009BF84 extern) and reconfirmed sandbox _exeque --disable all == 2/187, matching the ledger's last recorded floor exactly — chassis genuinely unchanged since s4, only the committed-vs-working-tree state drifted between sessions. Reverted src/display.c to INCLUDE_ASM before ending this session (asm-until-matched: no draft C left on main).

- [s7] Ran the SYSTEMATIC SPELLING SWEEP protocol (enumerate modality) on the final-callback block's named-intermediate space (`p`, plus two newly-introduced candidate intermediates `flag`/`cb` for the guard read and callback pointer) — see hypotheses.md [s7] for full detail. 19 spellings enumerated and swept against the real engine sandbox; best result is 2/187, achieved only by the two spellings that collapse back to the existing chassis (both new intermediates inlined away). Every spelling that names `flag` or `cb` as a real local scores 5-27, strictly worse. Class-killed (instance scope) this specific 3-local enumeration space as a source of improvement.

- [s7] Process note banked for future sweep_variants.py users on this function: the tool splices only the function span, so it inherits whatever forward declarations/externs are CURRENTLY on disk in src/display.c — running it against the bare INCLUDE_ASM state (missing D_8009BF84 extern, stale `void _exeque();` decl) silently produces WRONG scores (baseline read as 11 instead of 2) with no error. Always apply candidate.c's fixups to src/display.c first and confirm the swept baseline reproduces the ledger floor before trusting any variant score.

- [s7] src/display.c was at INCLUDE_ASM (clean, no drift) at this session's start; re-applied candidate.c's body + forward-decl fixups and reconfirmed sandbox score 2/187 (build_insns 186 vs target_insns 187) before doing any new work.

- [s7] The previous session (also labeled enumerate modality, same wall-clock day) already ran the systematic spelling-enumeration protocol on the final-callback block and banked a correctly-scoped instance kill in memory/grind/_exeque/hypotheses.md [s7] and memory/grind/_exeque/candidate.c's header comment — the driver discarded that session's OUTCOME JSON only, for a wording defect (the STATEMENT field said 'every spelling' — class language — while kill_scope was 'instance'), not for a defect in the underlying measurement or the ledger files, which were already correctly worded and remain unchanged this session.

- [s7] No new probes were run this session beyond the chassis re-confirmation; this session's sole contribution is the corrected outcome JSON plus the re-verification measurement above.

- [s7] The frontier is unchanged from s6/s7: the jalr-delay-slot residual (1 site) is not closable by any respelling of the final block's own local-variable declarations/ordering; the ruling-request on the volatile use-site-shape question (H6) remains the highest-value single lever, and the reorg.c resource-set-targeted register-forcing probe (s6 frontier item 2) remains untried.

- [s8, synthesis] Re-read the full ledger (evidence.md, hypotheses.md, rejected/ bank of 8 forms, candidate.c). Chassis check at dispatch reported "measurement unavailable" (src was at committed INCLUDE_ASM, as expected between sessions per asm-until-matched — this is normal, not drift). Applied candidate.c verbatim to src/display.c and reconfirmed sandbox _exeque --disable all == 2/187 (186 build insns vs 187 target), matching the ledger's last recorded floor exactly.

- [s8] Ran the driver-mandated KILL RE-AUDIT with `tools/fake_ablate.py` (the tool named in the brief for exactly this purpose) on the current chassis: keep-both-FAKE-wraps=2/187, drop-nested-only=7/187, drop-outer-only=7/187, drop-both=12/187. Both wraps are independently load-bearing; neither is an inert carrier occupying a pseudo a live lever needs (the func_8002EA24 s8 false-kill pattern the driver's instructions warn about). This re-confirms s4's original A/B measurement via the dedicated tool rather than by hand, closing out the mandatory re-audit for this flat-floor window.

- [s8] Synthesis: merged the s4-s7 evidence into two remaining untried STRUCTURAL axes (spelling-level respellings of the final-callback block are now exhaustively closed by s6's 4 structural probes + s7's 19-way enumeration). Axis 1 (policy): the H6 volatile ruling-request, unchanged since s2. Axis 2 (mechanism): reorg.c's `mark_set_resources` for a CALL_INSN (reorg.c:573-604, `include_delayed_effects=1`) marks every `call_used_reg` ($v0/$v1/$a0-$a3/$t0-$t9) as SET by the D_8009BE80 jalr; a delay-slot candidate insn (the `*p=0` store) is excluded from the fill only if it needs a resource the call itself needs live across the jump (e.g. the callee-address register). s7's 19-way enumeration already falsified the most obvious spelling of this axis — naming the CALLBACK POINTER (`cb`) as a fresh local, in every inline/keep/order/swap combination, only ever regressed (5-27, never better). The un-falsified remainder of axis 2 is restructuring how D_8009BE7C's ADDRESS is computed (e.g. field/struct-access instead of `&D_8009BE7C`) — s7 varied whether p/flag/cb are named, never how p's own address expression is spelled. No code was written or measured for axis 2 this session (synthesis modality, budget spent on ledger re-audit); it is the frontier's #2 item for the next session.

- [s8] Reverted src/display.c to its committed INCLUDE_ASM state before ending the session (`git checkout -- src/display.c`); only memory/grind/_exeque/{candidate.c,hypotheses.md,evidence.md} carry this session's edits.

- [s8] Re-read the full ledger (evidence.md 283->291 lines, hypotheses.md 500->530+ lines, candidate.c, all 8 rejected/ forms) before any measurement, per the synthesis-modality mandate.

- [s8] Chassis check at dispatch showed 'measurement unavailable' only because src/display.c was at its normal committed INCLUDE_ASM state between sessions (asm-until-matched) -- not chassis drift; re-applying candidate.c reproduced floor 2 exactly.

- [s8] Spelling-level respellings of the final-callback block are now exhaustively closed: s6 ran 4 fresh structural probes (guard && order x2, SetIntrMask relocation, outer-loop while-vs-if+do-while), s7 ran a 19-way systematic enumeration of {p,flag,cb} inline/keep x order x commutative-swap -- every spelling that names a NEW local for the guard read or callback pointer scores worse (5-27); only the exact existing chassis reaches 2.

- [s8] The kill re-audit via tools/fake_ablate.py (the tool the brief names for exactly this purpose) mechanically confirms both FAKE do-while(0) wraps are load-bearing on the current chassis, ruling out a func_8002EA24-style false kill for this flat-floor window.

- [s8] Two structural (not spelling) axes remain genuinely untried: (1) the H6 legitimate-volatile-interrupt-touched ruling-request, unchanged since s2; (2) forcing D_8009BE7C's ADDRESS COMPUTATION (not the callback pointer, which s7 already falsified as a fresh local in all 19 combinations) into a call_used_reg that reorg.c's mark_set_resources(CALL_INSN, include_delayed_effects=1) (reorg.c:573-604) marks as needed by the D_8009BE80 jalr, which would structurally exclude the *p=0 store from the delay slot fill.

- [s8] src/display.c was reverted to its committed INCLUDE_ASM state before session end (git checkout -- src/display.c); no draft C or cheats left on main. Only memory/grind/_exeque/{candidate.c,hypotheses.md,evidence.md} carry this session's edits.

- [s9] Chassis re-confirmed: applying memory/grind/_exeque/candidate.c verbatim to src/display.c (it had drifted back to INCLUDE_ASM at session start) reproduces sandbox _exeque --disable all == 2/187 (186 build insns vs 187 target), identical to s4-s8.

- [s9] The classify tool requires object-level mode for this function: the text-stream classifier's own guard (docs/grind/inverse-compose-2026-08-06.md) refuses display.c because it is zero-rule; ran with --target-object build/src/display.o --ours-object tmp/sandbox/_exeque/display.o instead, argument order is `classify <stem> <func>` (display then _exeque, not the reverse — confirmed by a first failed attempt that looked up function 'display').

- [s9] sched_solver's own README documents that reorg.c's delay-slot filling runs after sched2 and is explicitly NOT modelled by the toolkit ('the mapper cancels reorg rather than modelling it') — this session's empty-search result is the first time that documented limitation has been directly demonstrated on _exeque's specific residual rather than inferred from the instrumented-cc1 dump read at s6.

- [s9] Both ra_solver (classify: no RA component) and sched_solver (perturb: no SCHED1/2 component, zero candidate blocks) are now formally exhausted for this residual by their own tools, not just by prior dump-reading — the residual is unreachable via the solver route specifically, though this says nothing about whether a C-level register-pressure lever (s8 frontier axis 2, a structural-modality probe) can still change reorg.c's own decision.

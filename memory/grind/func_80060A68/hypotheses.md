# Hypothesis ledger — func_80060A68

## Frontier after s1 (floor 2, candidate.c banked)

The residual is ONE adjacent swap of two independent, equal-priority (both 3) loads:
stage load `lw $5,16($3)` (insn 25) must move from slot 11 to slot 12, past copy2's
address load `lw $4,12($3)` (insn 35). Everything else — all 64 other instructions,
every register, the frame — matches.

### H1 (primary): change insn 35's readiness/hazard path so it schedules at backward
cycle T-47 instead of T-45.
Mechanism: 35 currently jumps ahead via the stall-filler launch ("launching 35 before
32 with no stalls") triggered by copy1-store's stall window; 25 is plain-class and
places only when alone-ready. Honest levers to measure: respell copy1 so its
store/data-load chain has no stall at that cycle (e.g. split copy1's data load into a
named temp; or reorder copy1/copy2 data-vs-addr chains); respell copy2's address read
(named local reused for copy2 only — but beware: target reloads 12($3) three times,
so a SHARED local is wrong; a per-statement fresh temp keeps the reload).
Probe: sandbox-measure each respelling; check the .sched trace for whether 35 still
launches before 32.

### H2: permuter campaign from the floor-2 base.
The base is an excellent seed (score 2, pure ordering residual). Directed PERM on the
copy-block statement forms + temp declaration order. Run via tools/permuter_campaign.py
with the single-function target per [[difficult-is-not-impossible]] §3 (build
target.o from asm/funcs/func_80060A68.s so the function sits at offset 0).
Closing forms are PROPOSALS — vet against the checklist before banking.

### H3 (only if H1/H2 measure dead): re-read [[sched-rank-class-tie-wall]] and read
rank_for_schedule (tools/gcc-2.7.2/sched.c:2399-2456) against insns 25/35's actual
dependence classes to determine whether this tie is structurally closed for pure C.
If the wall rule's closure argument applies verbatim, the ledger must say so with the
sched.c line-level reasoning before anyone claims exhaustion — the residual is 2, and
2026-05-12's "plateau" claim on this function already died once this session.

## Killed this session (do not re-derive)
- K1: caching the dispatch index in a shared local (old body) — CSE holds the
  D_800F10D0 address in a register; 39-level wall. Killed by v2 (fresh re-read).
- K2: fresh single-set pointer local (`p10`) in ANY source position — load lands
  slot 5 (before the sw $0 fence) or slot 23/26 (after); never slot 12. Killed by
  v3/v4/v5/v6 grid.
- K3: `u16` typed dispatch-index local — emits a redundant `andi` target lacks.
  Killed by v2→v3 (s32 fixes it).
- K4: gp-store `D_800A3478 =` written between the two u16 copy statements with
  inline reads — store-order fence pins read2's load below the gp store; target
  needs it above. Killed by v8; the split-read (temp2) is the required shape.
- K5: staging the 0x10 pointer through `idx` — schedules early but allocates $6,
  cascading renames (score 7). The stage variable must be temp_a1 (target's
  `lhu $5,4($5)` self-overwrite). Killed by v10.
- K6: source-position of the temp_a1 stage statement (before copy1 / after copy1 /
  after copy2) — all three produce IDENTICAL emit. Killed by v11-v13.

## [s1] The 2026-05-12 'RA/scheduling plateau' was actually CSE holding the D_800F10D0 address across the body; a fresh re-read of *(u16*)outer (sibling func_80060B70's idiom) collapses it
- mechanism: cse.c address reuse vs target's per-use lui/addu/%lo rematerialization
- probe: v2 rewrite: drop $3 pin + _frame_pad, inline first idx read, fresh second read; sandbox
- result: 39 -> 10; frame 0x20 returned naturally without the pad
- verdict: CONFIRMED

## [s1] Target's store order (sh 0x18 < sw gp D_800A3478 < sh 0x1A) with read2's load ABOVE the gp store forces a split-read temp (read before gp store, store after), same shape as temp_a1/D_800A347C
- mechanism: sched.c write_dependence: gp-symbol store vs base-reg memory refs cannot be disambiguated, so stores emit in source order and later-in-source loads cannot cross them
- probe: v8 (pair between stores, inline read) vs v9 (split-read temp2); sandbox + disasm
- result: v8=10/67; v9=2/65 with the whole middle block byte-matching
- verdict: CONFIRMED

## [s1] The early 0x10($3) pointer load at target slot 12 requires a multi-set stage variable, and the stage variable is temp_a1 itself (target's lhu $5,4($5) self-overwrite)
- mechanism: sched.c load-late launch priority for reg_n_sets==1 destinations ([[staged-value-reused-variable]], sanctioned 2026-07-03); fresh single-set local measured at slot 5 or slot 23/26 in every position, never slot 12
- probe: position grid v3-v6 (single-set) then v10 (stage via idx) then v11 (stage via temp_a1); sandbox + disasm
- result: single-set: 8/10/6 never slot 12; via idx: 7 (wrong reg $6); via temp_a1: 2 with ALL registers matching
- verdict: CONFIRMED

## [s1] Source position of the stage statement moves the contested load past copy2's address load
- mechanism: LUID/source-order tie-break in rank_for_schedule
- probe: three positions (before copy1, after copy1, after copy2), sandbox + disasm each
- result: identical emit all three; both insns priority 3 (equal) in the .sched dump; copy2's load jumps via stall-filler launch, stage load places only when alone-ready
- verdict: KILLED

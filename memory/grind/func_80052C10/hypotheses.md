# Hypothesis ledger — func_80052C10

## s1 frontier (recon)

1. **Register-seat recovery ($v0 -> $t9).** Mechanism: global register allocator
   (global.c) allocno priority / preference order — find what in a plausible
   fuller original body (more locals/calls before this store) would push the
   address pseudo past $v0..$t8 to $t9, without a pin or dead store. Probe:
   dump `.greg`/`.lreg` for the minimal body (`cc1 -da`, `tools/gcc-2.7.2/cc1`)
   and compare against a synthetic body with extra live scratch values: does
   adding genuine (non-dead) prior computation shift the allocation? If no
   natural-C body of any shape reaches $t9, that's evidence for the hand-written
   side despite the LOW scan_hand_coded tier.
2. **Trailing double-nop.** Mechanism unknown — not inter-object padding (next
   function starts immediately after, no gap), not GTE/cop2 latency (no cop2 op
   here). Probe: check sibling single-store scratchpad-poke functions elsewhere
   in the codebase (grep other `lui+ori+sw+jr+nop+nop` 6-insn leaf shapes) for
   a matched C analog; check `-da` `.sched`/reorg dumps for why a second nop
   would ever follow `jr $ra` in a leaf void function (delay-slot filling never
   fills past the one slot — a second nop has to come from source, e.g. two
   statements where the second is fully optimized away, which is itself a
   likely FAKE/dead-store situation and needs scrutiny under
   [[dead-store-fake-exception]] if pursued).
3. **Re-weigh the canonical-asm grant itself.** The grant's stated mechanism
   (combine.c always folds the offset) is measurably false for this address;
   scan_hand_coded is LOW/0. If (1) and (2) cannot be closed without a cheat
   construct after a real attempt, the right move is a `ruling-request` citing
   this evidence (not a unilateral revert of the 2026-06-07 grant) — the
   reopen_reason explicitly asks the Grinder to decide afresh, but a grant
   revocation is itself owner-relevant.

## s1 — kill + refined frontier (this session)

**KILLED (instance):** "A single-statement body `*(T *)0x1F800400 = 0;` (volatile
or plain) with no other live state reaches the target's `$t9` register seat."
Measured: `.greg` dump (`tmp/grind/func_80052C10/s1/3AB48.greg.txt`) shows the
volatile form compiles to exactly one pseudo (reg 72, zero conflicts), which
global.c allocates to `$v0` (lowest free scratch reg) every time, per
[[register-alloc-pure-c]]'s stated allocation order. This is an INSTANCE kill
on the single-statement chassis (both `vu32` and plain `s32` pointer forms
tried, no FAKE construct present) — it says nothing about whether SOME larger
body with real competing live state could reach `$t9`; that remains open
(frontier item 1 below).

Revised frontier for session 2:
1. **Register-seat recovery via genuine multi-value pressure.** To reach `$t9`
   needs ~13 other simultaneously-live scratch pseudos in-function — no
   caller or sibling evidence (checked this session: every call site is a
   true parameterless `void(void)` sink) suggests the original body had any
   such content. Next probe: construct a synthetic body with that much real
   pressure purely to test the allocator's behavior (diagnostic only, never
   landed) and confirm/deny $t9 is reachable at all before concluding "no
   natural single-store body reaches it" more broadly than this session's
   single-statement instance.
2. **Trailing double-nop** — unchanged from s1, still unexplained, still
   flagged source-level (no register lever reaches it).
3. **Re-weigh the canonical-asm grant.** New data this session CUTS BOTH WAYS:
   the grant's "combine.c always folds" is right for the non-volatile reading
   (score-4, folded-displacement form) and wrong only for the volatile
   reading; AND the immediately-next sibling function
   (`func_80052C28.s`) carries an independent, already-annotated
   `/* handwritten instruction */` marker, which is affirmative evidence FOR
   hand-written asm in this cluster, cutting against reopening the grant
   on scan_hand_coded's LOW/0 tier alone. Recommend NOT escalating a
   ruling-request yet — gather the item-1 probe first; if $t9 proves
   unreachable even under heavy synthetic pressure (a class-level finding,
   citable against [[register-alloc-pure-c]]'s ordering predicate), that is
   the point to ask.

## s2 (structural) — frontier item 1 resolved: $t9 IS reachable, but only at prohibitive cost

**CONFIRMED:** "$t9 is reachable by genuine (non-pinned, non-dead) C register
pressure for the scratchpad-address pseudo in func_80052C10's chassis."
- mechanism: global.c's register allocator has no `REG_ALLOC_ORDER` override
  for MIPS (confirmed: `grep REG_ALLOC_ORDER tools/gcc-2.7.2/config/mips/*` —
  no hits), so `global.c:1079`'s `#ifndef REG_ALLOC_ORDER` default path applies:
  ascending hard-register-number preference, i.e. $v0(2),$v1(3),$a0-$a3(4-7),
  $t0-$t7(8-15),$s0-$s7(16-23), THEN $t8(24),$t9(25). Reaching $t9 needs 22
  other hard regs already claimed by simultaneously-live pseudos — not ~13 as
  the s1 frontier estimated (that number omitted $s0-$s7, which sit BEFORE
  $t8/$t9 in the default order despite being callee-saved).
- probe: synthetic diagnostic chassis (never landed, tmp/grind/func_80052C10/s2/gen2.py):
  `target` pointer declared first (so its live range spans the whole body),
  N other `vu32 *pK = (vu32*)0x1F80....` pointers declared after it, all N
  dereferenced-and-stored BEFORE `*target = 0;` (forces target's live range to
  overlap all N). Swept N = 5,9,13,15,20,22,23 via
  `tools/grinder/dump.ps1 func_80052C10 -Candidate <file>` + `.greg` dispositions.
  Dumps saved: tmp/grind/func_80052C10/s2/{p5,p9,p13,q13,q15,q20,q22,q23}.greg.
- result: reg72 (the `target` pseudo, pseudo-id of the first-declared, longest-lived
  local) tracked across N: N=13→hard reg 16 ($s0); N=15→18 ($s2); N=20→23
  ($s7, last of the s-bank); N=22→**25 ($t9)**, confirmed via `sandbox
  --candidate tmp/grind/func_80052C10/s2/q22.c --diff`: score 106, 110 build
  insns vs target's 6, with a `addiu $sp,$sp,-32` prologue (saved in
  tmp/grind/func_80052C10/s2/q22_sandbox_diff.txt) — the s0-s7 saves this
  pressure level requires. N=23→30 (an even higher reg, past t9, since by then
  t9 itself is also claimed by an earlier-order competitor).
- verdict: CONFIRMED (not a kill — this is the positive mechanism finding the
  s1 frontier asked for).

**Practical corollary (separate hypothesis, instance-scope):** "A body whose
total instruction count stays close to the target's 6 insns can reach $t9 via
this genuine-pressure mechanism."
- measured_on: the gen2.py pointer-pressure chassis, N=5..23, no FAKE construct,
  no register pin; swept exhaustively over the only mechanism that reaches $t9
  (ascending default order forces $s0-$s7 saturation first).
- result: KILLED on this chassis — every N that reaches $t9 (N>=22) necessarily
  also claims all of $s0-$s7, which under MIPS/PsyQ calling convention forces a
  non-trivial save/restore prologue+epilogue (measured: 110 insns at N=22,
  `addiu $sp,$sp,-32` + corresponding restores, vs the target's 0-insn
  prologue). No N in the swept range reaches $t9 without that overhead — the
  mechanism that reaches $t9 and the mechanism that keeps the body near 6 insns
  are mutually exclusive on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: gen2.py pointer-pressure chassis (declare-target-first,
  dereference-others-before-target shape), N=5,9,13,15,20,22,23, no FAKE
  construct, no register pin, measured via `.greg` dispositions +
  `sandbox --candidate --diff`.

This sharpens s1's open register-seat-anomaly gap: the target's $t9 choice is
mechanically *possible* in GCC 2.7.2 but demands the kind of register pressure
(22+ live scratch values, s0-s7 saved) this function's actual asm shows no
sign of (no save/restore insns at all in the 6-insn target). That is evidence
*against* a small natural-C original reaching $t9 here, and so *for* the
canonical-asm (hand-written) reading already supported by the sibling
func_80052C28 marker (s1 evidence) — not conclusive (only this one chassis
shape was swept; a differently-shaped body might reach pressure without using
$s0-$s7, e.g. one with 22 live values that are all call-clobbered temporaries
with no calls — untested), but it meaningfully narrows the frontier.

## [s1] A single-statement body `*(T *)0x1F800400 = 0;` (volatile vu32 or plain s32 pointer form) reaches the target's $t9 register seat for the scratchpad address pseudo.
- mechanism: global.c register allocation order: lowest-numbered free scratch register is preferred absent competing live pseudos.
- probe: .greg dump for func_80052C10 (tmp/grind/func_80052C10/s1/3AB48.greg.txt) on the vu32 candidate body; confirmed only one pseudo (reg 72, zero conflicts) exists, allocated to $v0.
- result: Both the vu32 and plain s32 single-statement forms allocate the address pseudo to $v0 (lowest free scratch reg), never $t9. Reaching $t9 would need ~13 other simultaneously-live scratch pseudos occupying $v0..$t8 first, which no caller or sibling evidence suggests ever existed in this body.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main/3AB48.c func_80052C10, single-statement chassis `*(vu32 *)0x1F800400 = 0;` and `*(s32 *)0x1F800400 = 0;`, no FAKE construct present, no register pin used

## [s2] $t9 is reachable by genuine (non-pinned, non-dead) C register pressure for the scratchpad-address pseudo in func_80052C10's chassis.
- mechanism: global.c's register allocator has no REG_ALLOC_ORDER override for MIPS (grep confirmed), so the #ifndef REG_ALLOC_ORDER default path (tools/gcc-2.7.2/global.c:1079) applies: ascending hard-register preference $v0,$v1,$a0-$a3,$t0-$t7,$s0-$s7,$t8,$t9 -- 22 registers must be claimed by other simultaneously-live pseudos before $t9 is chosen.
- probe: gen2.py pointer-pressure chassis (target pointer declared first so its live range spans the body, N other vu32* pointers declared after and dereferenced before target's own use to force overlap); swept N=5,9,13,15,20,22,23 via tools/grinder/dump.ps1 -Candidate + .greg dispositions.
- result: CONFIRMED: at N=22 other live pointers, the target pseudo (reg72) is allocated hard reg 25 ($t9), matching the target asm's register choice. Verified via sandbox --candidate q22.c --diff (score 106, 110 build insns, addiu $sp,$sp,-32 prologue for the s0-s7 saves this pressure level requires).
- verdict: CONFIRMED

## [s2] A body whose total instruction count stays close to the target's 6 insns can reach $t9 via this genuine-pressure mechanism.
- mechanism: n/a -- this is the practical corollary, not a new codegen mechanism: the same ascending-order allocator that reaches $t9 at N>=22 also forces $s0-$s7 into use, which the PsyQ/MIPS calling convention requires saving/restoring across the function body.
- probe: Same gen2.py sweep, N=5..23, reading build-insn counts from sandbox --candidate --diff at each reachability threshold.
- result: KILLED on this chassis: every N that reaches $t9 (N>=22) also claims all of $s0-$s7 and incurs a non-trivial save/restore prologue+epilogue (110 insns at N=22 vs the target's 6, with addiu $sp,$sp,-32 -- target has zero prologue insns). No swept N reaches $t9 without that overhead; the mechanism that reaches $t9 and the mechanism that keeps a body near 6 insns are mutually exclusive on this chassis shape.
- verdict: KILLED
- kill_scope: instance
- measured_on: gen2.py pointer-pressure chassis (declare-target-first, dereference-others-before-target shape), N=5,9,13,15,20,22,23, no FAKE construct, no register pin, measured via .greg dispositions + sandbox --candidate --diff

## [s3] Unguided decomp-permuter random mutation, seeded from the banked floor-4 chassis, finds a byte-closer (or any novel) form for func_80052C10.
- mechanism: decomp-permuter's default randomization-pass weight set (perm_var, perm_refer_to_var, perm_cast_simple, perm_reorder_stmts, perm_ins_block, perm_split_assignment, etc.) applied directly to the AST of the candidate body, scored against asm/funcs/func_80052C10.s via a hand-built workspace (tools/decomp-permuter/nonmatchings/func_80052C10{,_ptr}/).
- probe: Two campaigns, -j4, default weights, no --hint (no existing hint slug fits this function's shape — register-asm-pins/shared-end-label/loop-rotation/opaque-one/loop-counter-fills-load-delay all assume richer bodies than a single store). Chassis A: `*(vu32*)0x1F800400 = 0;` (the banked candidate.c, 38,674 iterations, 833.3s). Chassis B: `vu32 *p = (vu32*)0x1F800400; *p = 0;` (confirmed byte-identical to chassis A via sandbox --candidate --diff before permuting; 46,787 iterations, 1096.8s).
- result: Zero novel outputs on both chassis (finds_total=0 in both harvests). Many mutation attempts on chassis A raised internal AssertionErrors ("dereferencing non-pointer" in perm_split_assignment etc.) because a single-expression body gives the mutator almost no AST surface; chassis B (adding a named pointer local) ran cleanly but still found nothing novel. This independently confirms, via unguided search rather than hand analysis, that neither the $t9 register-seat hunk nor the trailing-nop source-level hunk is reachable by any of decomp-permuter's default single-step mutations from either byte-equivalent starting chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: tools/decomp-permuter/nonmatchings/func_80052C10/ and .../func_80052C10_ptr/ workspaces, both chassis confirmed sandbox score 4 (no FAKE construct, no register pin), decomp-permuter default weights, -j4, combined 85,461 iterations, 0 novel finds

## [s3] Sibling ledger `puts.c` (src/main/psxsdk/libc2/puts.c, COMPLETED-C, closed ledger) names a construct transplantable onto func_80052C10's chassis.
- mechanism: n/a — inheritance check per SIBLING LEDGERS directive.
- probe: Read src/main/psxsdk/libc2/puts.c in full (small, COMPLETED-C body: NULL-check + putchar loop).
- result: puts() shares no structural element with func_80052C10 (no scratchpad MMIO access, no single-store body, no double-nop tail, no register-pressure pattern) — it is listed as a sibling only because an earlier session mentioned it in passing, not because it shares code shape. Nothing to transplant.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main/psxsdk/libc2/puts.c (read in full, no code transplanted — chassis has no shared constructs to test)

## [s3] Unguided decomp-permuter random mutation, seeded from the banked floor-4 single-statement chassis (`*(vu32*)0x1F800400 = 0;`), finds a byte-closer or any novel form for func_80052C10.
- mechanism: decomp-permuter's default randomization-pass weight set applied to the body's AST, scored against asm/funcs/func_80052C10.s via a hand-built workspace (tools/decomp-permuter/nonmatchings/func_80052C10/).
- probe: Campaign launched -j4, default weights, no --hint (no catalog hint slug matches this body's shape); waited in two ~9-min windows (833.3s total), harvested+stopped at 38,674 iterations.
- result: finds_total=0, finds_new=0 across the whole campaign. Many mutation attempts raised internal AssertionErrors (single-expression body gives mutators almost no AST surface) but the campaign kept running and logged real iterations regardless.
- verdict: KILLED
- kill_scope: instance
- measured_on: tools/decomp-permuter/nonmatchings/func_80052C10/ workspace, chassis `*(vu32*)0x1F800400 = 0;` (sandbox score 4 confirmed), decomp-permuter default weights, -j4, 38,674 iterations, no FAKE construct, no register pin

## [s3] A structurally richer but byte-equivalent chassis (named local pointer: `vu32 *p = (vu32*)0x1F800400; *p = 0;`) gives decomp-permuter enough AST surface to find a byte-closer or any novel form.
- mechanism: Same mutation set as above, but with a named local variable added as a mutation target (perm_var, perm_refer_to_var, perm_reorder_decls, etc. all gain a foothold they lacked on the single-expression chassis).
- probe: Confirmed via sandbox --candidate --diff that this chassis is byte-identical to the banked candidate (score 4, same hunks, same $v0 seat) before permuting it, so any novel find would still be an honest lead. Campaign launched -j4, default weights; waited in two ~9-min windows (1096.8s total), harvested+stopped at 46,787 iterations.
- result: finds_total=0, finds_new=0. No AssertionError storm this time (richer AST tolerates the mutators), but still nothing novel.
- verdict: KILLED
- kill_scope: instance
- measured_on: tools/decomp-permuter/nonmatchings/func_80052C10_ptr/ workspace, chassis `vu32 *p = (vu32*)0x1F800400; *p = 0;` (sandbox score 4 confirmed byte-identical to candidate.c), decomp-permuter default weights, -j4, 46,787 iterations, no FAKE construct, no register pin

## [s3] Sibling ledger puts.c (src/main/psxsdk/libc2/puts.c, COMPLETED-C, closed) names a construct transplantable onto func_80052C10's chassis.
- mechanism: n/a -- inheritance check per SIBLING LEDGERS directive.
- probe: Read puts.c in full (NULL-check + putchar loop, ~20 lines).
- result: No shared structural element with func_80052C10 (single MMIO scratchpad store, no loop, no string handling); the sibling mention in this ledger was incidental, not a missed transplant.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main/psxsdk/libc2/puts.c (read only, no transplant attempted -- no shared construct exists)

## [s4] A systematic spelling sweep of the single-store statement's declaration/address/value/cast axes (no semantic change) finds a byte-closer form or a novel chassis for func_80052C10.
- mechanism: tools/spelling_enum.py's decl/inline/order axes plus hand-widened address-representation, cast-form, volatile-spelling, and trailing-statement axes -- exhaustive over the syntactic spelling space of a one-statement body, as distinct from decomp-permuter's random AST mutation (s3).
- probe: spelling_enum.py on a 2-decl region produced 2 variants (its DECL_RE failed to parse `vu32 *p = ...;` as a decl because there is no whitespace between `*` and `p`, so `p` fell through to ANCHOR -- a tool limitation, not a search gap, since the pointer-named/inlined axis was independently covered by hand). Hand-wrote 14 more variants (v2-v14) covering: decl order (p-first vs val-first), pointer split decl+assign, address held in a named `u32 addr` local vs inline literal, `(u32)0` / `0u` value casts, extra parens around the `(vu32 *)` cast, `volatile u32 *` vs `vu32 *` spelling, lowercase hex literal, `vu32 *const p`, and an explicit trailing `return;`. Swept all 16 with tools/sweep_variants.py --json (tmp/grind/func_80052C10/s4/sweep_output.json).
- result: Every variant: score 4, build_insns 5, identical two-hunk diff (operand-only $t9 vs $v0 register seat; source-level missing trailing `nop`) to the banked candidate. Zero-hit sweep.
- verdict: KILLED
- kill_scope: instance
- measured_on: 16 syntactic spellings of the single-statement MMIO-store body (declaration/order/address/cast/value/trailing-statement axes), no FAKE construct, no register pin, measured via tools/sweep_variants.py this session

## [s4] A systematic spelling sweep of the single-store statement's declaration/address/value/cast axes (no semantic change) finds a byte-closer form or a novel chassis for func_80052C10.
- mechanism: tools/spelling_enum.py's decl/inline/order axes plus hand-widened address-representation, cast-form, volatile-spelling, and trailing-statement axes -- exhaustive over the syntactic spelling space of a one-statement body, distinct from decomp-permuter's random AST mutation (s3).
- probe: spelling_enum.py on a 2-decl region (vu32 *p = ...; u32 val = 0; *p = val;) produced 2 variants (its DECL_RE failed to parse the pointer decl due to a whitespace quirk, so only the val named/inlined axis came out of the tool). Hand-wrote 14 more variants covering decl order, split decl+assign, named vs inline address, value casts (u32)0/0u, extra parens, volatile vs vu32 spelling, lowercase hex, const pointer, and trailing explicit return. Swept all 16 with tools/sweep_variants.py --json.
- result: Every one of the 16 variants scored identically: score 4, build_insns 5, same two-hunk diff (operand-only $t9/$v0 register seat, source-level missing trailing nop) as the banked candidate. Zero novel or byte-closer forms.
- verdict: KILLED
- kill_scope: instance
- measured_on: 16 syntactic spellings of the single-statement MMIO-store body (declaration/order/address/cast/value/trailing-statement axes), no FAKE construct, no register pin, tools/sweep_variants.py this session

## s5 (synthesis) — merged picture and frontier reset

**CONFIRMED:** "The target's trailing `jr $ra; nop; nop` tail is a hand-assembled signature in this
binary." In a census of all 1437 functions, 11 have the tail, and the other 10 are Sony
asmpsx-built modules (evidence.md s5). That settles the s1-s4 frontier item "trailing double-nop
unexplained": no C lever can produce the word inside the function span. Its representation is the
function's own .s file (`.claude/rules/rodata-object-alignment.md:63`).

**KILLED (instance):** "Block-local short-lived pressure (N volatile loads live across the store, a
single basic block) puts the 0x1F800400 address pseudo in $t9 without a frame." Measured at N=14/22/24:
the address went to $s0/$a1/$v1, with a stack frame and spills at each N (scores 116/212/238).
Rejected form: rejected/blocklocal-pressure-22-no-t9.c.

**Synthesis.** Four independent findings point the same way. (1) The $t9 seat needs about 22
competing live values in global.c, which forces an $s0-$s7 frame (s2), and block-local pressure
spills instead of reaching $t9 (s5). (2) The double-nop tail occurs only in asmpsx modules (s5).
(3) The function sits inside a contiguous run of hand-asm neighbours, including trapping `sub`/`addi`
in func_80052C28 (s1, s5). (4) 85k permuter iterations and a 16-spelling sweep found nothing (s3, s4).
Against that, scan_hand_coded is LOW/0, but it has no trailing-nop or register-seat detector. The
2026-06-07 grant's stated reason ("combine.c always folds") is wrong for the volatile reading (s1).
The canonical-asm reading still stands, but on different grounds. The next step is a ruling on the
re-audit, not another C probe.

Frontier for s6:
1. Ruling on the re-audit: keep the canonical-asm row and rewrite its reason to the s5 evidence, or
   name a C shape still to try.
2. If C work continues: the only untried idea is a body whose original surrounding C (inlined into
   a caller?) explains both the $t9 seat and the extra word. No caller evidence supports it (s1).
3. Teach scan_hand_coded a "jr;nop;nop tail" signal. That is tools work, outside the Grinder surface,
   and is noted for the operator.

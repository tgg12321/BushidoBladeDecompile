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

## [s5b] The trailing nop is an inter-object pad from an object boundary at 0x80052C24 with the next object 8-aligned at 0x80052C28 (Judge-directed shape, decisions.md:1380).
- mechanism: link-time alignment of func_80052C28's object would insert one nop word after func_80052C10's 5-insn C body.
- probe: (1) census of object .text start phases in build/bb2.map (tu_boundary_census.py); (2) scratch link of a C-compiled func_80052C10 object + stub next object under the repo SUBALIGN(2) model, GNU default, and explicit ALIGN(8) (link.sh).
- result: 102/225 objects (60+ verbatim PsyQ modules) start at 4 mod 8 with no pad, so the original link did not 8-align object starts. Scratch link: SUBALIGN(2) -> next object at 0x80052C24 (no pad); default -> 0x80052C30 (3 words); ALIGN(8) -> 0x80052C30. No model gives the single word.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c single-store body `*(vu32 *)0x1F800400 = 0;` as its own object, no FAKE construct, scratch link in tmp/grind/func_80052C10/s5/ (sub2.ld / natural.ld / align8.ld), plus bb2.map object-start census

## [s5b] The asmpsx module 16-byte pad excludes func_80052C10 because 0x80052C28 is not 16-aligned (the ruling's objection to evidence (a)).
- mechanism: the ruling read the pad as absolute 16-byte alignment.
- probe: absolute end addresses of the ten Sony asmpsx jr;nop;nop functions (tu_boundary_census.txt).
- result: nine end at 12 mod 16 and one at 0 mod 16; the pad is relative to each module's start. A module starting at gte_SetMatrixRotTransIRVec 0x80052A88 (or math_LerpSVector 0x80052788) ends exactly at 0x80052C28 after padding. The objection does not hold.
- verdict: KILLED
- kill_scope: instance
- measured_on: build/bb2.map symbol addresses + asm/funcs/*.s insn counts for the 11 jr;nop;nop functions; no C candidate involved

## s5b frontier reset
1. The C side is unchanged: the $t9 seat still needs a frame-free chassis no session has found (s2 global pressure and s5 block-local pressure both add frames). Untried: a body with many values live across the store that GCC keeps in call-clobbered regs only, e.g. args a0-a3 + returned v0/v1 live through, which the s1 caller evidence argues against (void(void) sink).
2. Tools (operator, outside the Grinder surface): a scan_hand_coded signal for "module-relative 16-pad word after jr's delay slot" would turn the s5/s5b evidence into a gate-class signal; the ruling says a new signal class needs owner approval.
3. Pin the module start: test which of 0x80052788 / 0x80052A88 begins the hand-asm module (e.g. whether math_LerpSVector/math_LerpMatrix3x3 carry compiled-C signatures); a boundary there is real TU-boundary evidence under rodata-object-alignment.md "New TU boundaries".

## [s5] The trailing nop is an inter-object pad from an object boundary at 0x80052C24 with func_80052C28's object 8-aligned at 0x80052C28 (the Judge's named shape, decisions.md:1380).
- mechanism: link-time alignment of the next object would put one nop word after func_80052C10's 5-insn compiled body.
- probe: Census of object .text start phases in build/bb2.map (tu_boundary_census.py), and a scratch link of a C-compiled func_80052C10 object plus a stub next object under the repo SUBALIGN(2) model, GNU ld default, and an explicit ALIGN(8) (link.sh).
- result: 102 of 225 objects (60+ LIBSCAN-verbatim PsyQ modules: libc2, libgte, libsnd, libspu, libapi) start at 4 mod 8 with no pad, so the original PSYLINK did not 8-align object starts. Scratch link: SUBALIGN(2) puts the next object at 0x80052C24 (no pad); GNU default at 0x80052C30 (3 words, from as's 2**4 .text alignment); ALIGN(8) also 0x80052C30. None gives the one word. Getting it needs a per-object 8-alignment no other object shows, which is the speculative-boundary case rodata-object-alignment.md:48-49 bans.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c single-store body `*(vu32 *)0x1F800400 = 0;` compiled as its own object, no FAKE construct, scratch links tmp/grind/func_80052C10/s5/{sub2,natural,align8}.ld, plus bb2.map object-start census

## [s5] The asmpsx module 16-byte pad cannot explain func_80052C10's trailing word because 0x80052C28 is not 16-aligned (the ruling's objection to the tail-census evidence).
- mechanism: the ruling read Sony's module pad as absolute 16-byte alignment.
- probe: Absolute end addresses of the ten Sony asmpsx jr;nop;nop functions, and the module sizes from candidate starts in the 3AB48 run.
- result: Nine of the ten Sony modules end at 12 mod 16 (e.g. mtx_03.o 0x8007EB4C..0x8007EC5C = 17x16), so the pad is module-relative. A module starting at math_LerpSVector 0x80052788 (74x16 bytes) or gte_SetMatrixRotTransIRVec 0x80052A88 (26x16) ends exactly at 0x80052C28. math_LerpSVector carries its own STRONG S9 scan signal, and func_80052C28 (STRONG S9) would start the next object. The objection does not hold.
- verdict: KILLED
- kill_scope: instance
- measured_on: build/bb2.map symbol addresses and asm/funcs/*.s instruction counts for the 11 jr;nop;nop functions plus scan_hand_coded --single over the 3AB48 run; no C candidate involved

## [s5] Kill re-audit: no banked instance kill of func_80052C10 was measured with a FAKE carrier, so none is void on the current chassis.
- mechanism: fake_ablate on the banked candidate plus a fresh baseline diff.
- probe: tools/fake_ablate.py on candidate.c (nothing to ablate) and sandbox --disable all --diff on candidate.c this session.
- result: Score 4, the same two hunks as s1-s4: operand-only $t9 vs $v0, source-level missing trailing nop. Every banked kill stands as measured.
- verdict: CONFIRMED

## [s6] Kill re-audit (solver): the banked candidate re-measures 4 with no FAKE carrier.
- mechanism: mandated re-audit before new probes (flat floor, instance kills exist).
- probe: sandbox --disable all --diff on candidate.c; tools/fake_ablate.py on candidate.c.
- result: score 4, same two hunks (operand-only $t9 vs $v0; source-level trailing nop). fake_ablate: no FAKE constructs, nothing to ablate. No banked kill was measured under a FAKE carrier.
- verdict: CONFIRMED
- artifacts: tmp/grind/func_80052C10/s6/sandbox_baseline.txt, fake_ablate.txt

## [s6] inverse_compose classify routes the residual to SCHED, but that routing does not fit this function.
- mechanism: classify (object path, --target-object build/src/main/3AB48.o --ours-object tmp/sandbox/...) masks registers; the only multiset difference left is one `nop`, which it reads as a maspsx load-delay nop.
- probe: classify run in WSL (Windows-side python cannot find objdump, so run it from WSL).
- result: "FIRST DIVERGENCE: SCHED". The function has no load, so the nop cannot be a load-delay nop. It sits after the jr delay slot, which is the s5 module-pad word, not a scheduler output. inverse_sched has nothing to solve, so the SCHED axis is not a real lever here. The $t9 seat is hidden by the classifier's register mask.
- verdict: CONFIRMED (classifier mis-route recorded; no scheduler search run)
- artifacts: tmp/grind/func_80052C10/s6/classify.txt, classify.sh

## [s6] CLASS KILL: a frame-free GCC 2.7.2 body under the build flags does not seat the address pseudo in $t9 (closes s5b frontier item 1).
- mechanism: there are two ways to reach hard reg 25. (a) Ascending scan: mips.h defines no REG_ALLOC_ORDER, so local-alloc find_free_reg (local-alloc.c:2248-2254), global find_reg (global.c:1058-1064) and reload's order_regs_for_reload (reload1.c:3769-3777) all scan ascending. $16-$23 ($s0-$s7) come before $24/$25, and any of them in use makes compute_frame_size save it (mips.c:4479-4481 MUST_SAVE_REGISTER). (b) A copy preference toward hard reg 25: the only producers of $25 in the MIPS port are the `%^` call templates and .cpload, all gated on TARGET_ABICALLS or TARGET_LONG_CALLS (mips.md:6165, :6182, :6219, :6235; mips.c:4874). The Makefile:42 CC_FLAGS pass -mno-abicalls and no -mlong-calls, so the RTL never mentions reg 25.
- probe: the analytic check above, plus an empirical ladder (ladder.py): N values from a volatile extern array kept live across the store, in a one-block form and in a branchy form, N=8..25, run through the build cc1 with the exact CC_FLAGS.
- result: the one-block address seat is exactly $(N+2): $10..$15 with no frame for N=8..13, then $16 with a frame at N=14, climbing to $25 at N=23, still framed. The branchy form keeps the address in $2. Every seat from $16 up comes with a prologue, and the target has none. This generalises s2 (global pressure, 22 pseudos) and s5 (block-local pressure).
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:2248
- measured_on: ladder chassis in tmp/grind/func_80052C10/s6/ladder.py (no FAKE), build cc1 tools/gcc-2.7.2/build/cc1 with Makefile:42 flags; candidate.c body for the baseline
- artifacts: tmp/grind/func_80052C10/s6/ladder.py, ladder.txt, ladder_N{14,15,22}_{local,global}.s; rejected/ascending-pressure-ladder-t9-needs-frame.c

## [s6] Padding cannot pin the module start in the 3AB48 hand-asm run (s5b frontier item 3, partial).
- probe: tails of gte_SumSquares3, math_LerpSVector, gte_SetMatrixRotTransIR, gte_SetRotMatrixClearTrans, gte_ReadFarColor, and the map addresses 0x80052754..0x80052C10.
- result: every function between them ends `jr; <delay>` and the next symbol follows at once. There is no pad word anywhere from 0x80052754 to 0x80052C10, so padding cannot place a boundary at 0x80052788 or 0x80052A88. Only the start-relative 16-pad arithmetic from s5 bears on it.
- verdict: CONFIRMED

## s6 synthesis / frontier
The register hunk on the C side is now closed at class level. Under -mno-abicalls GCC 2.7.2 seats the address in $t9 only with $s0-$s7 in use, and that brings a prologue the 6-word target does not have. The trailing nop has no compiled-C producer either (s5). Both residual hunks point to hand-written asm with a `li $t9` macro expansion. The remaining work is outside C: a scan_hand_coded signal, which needs owner approval per the ruling, or an escalation-modality disposition. The C ladder still has rederive and forensics modalities. The only C reading left is a different flag set (e.g. abicalls) for this TU, and no evidence in the binary supports one: no .cpload, no gp setup.

## [s6] Kill re-audit: the banked single-store candidate still scores 4 with no FAKE carrier, so no banked instance kill was measured under a FAKE construct.
- mechanism: Mandated re-audit (flat floor, instance kills exist).
- probe: sandbox --disable all --diff on candidate.c; tools/fake_ablate.py on candidate.c.
- result: Score 4 with the same two hunks (operand-only $t9 vs $v0, source-level trailing nop). fake_ablate found nothing to ablate.
- verdict: CONFIRMED

## [s6] inverse_compose classify (object path) routes the residual to SCHED, but that verdict is a mis-route for this function.
- mechanism: classify masks registers and reads the single surplus nop as a maspsx load-delay nop.
- probe: classify main/3AB48 func_80052C10 --target-object build/src/main/3AB48.o --ours-object tmp/sandbox/func_80052C10/main/3AB48.o, run from WSL.
- result: FIRST DIVERGENCE: SCHED. The function has no load. The surplus nop follows jr's delay slot, which is the s5 module-pad word, so no scheduler search applies. The $t9 seat is hidden by the register mask.
- verdict: CONFIRMED

## [s6] A frame-free GCC 2.7.2 body compiled with the Makefile CC_FLAGS cannot seat the scratchpad-address pseudo in $t9 (hard reg 25).
- mechanism: mips.h defines no REG_ALLOC_ORDER, so local-alloc find_free_reg (local-alloc.c:2248), global find_reg (global.c:1058) and reload order_regs_for_reload (reload1.c:3769) all scan ascending. That puts $s0-$s7 ahead of $t8/$t9, and using any of them forces a prologue save (mips.c:4481 MUST_SAVE_REGISTER). The other route, a hard-reg-25 copy preference, needs RTL that mentions $25, and those templates are all gated on TARGET_ABICALLS or TARGET_LONG_CALLS (mips.md:6165, :6182, :6219, :6235; mips.c:4874). The build passes -mno-abicalls and no -mlong-calls (Makefile:42).
- probe: The analytic predicate above, plus an empirical ladder (tmp/grind/func_80052C10/s6/ladder.py). N=8..25 values from a volatile extern array are kept live across the store, in one-block and branchy forms, and compiled with the build cc1 and the exact CC_FLAGS.
- result: In the one-block form the address seat is exactly $(N+2): $10-$15 with no frame for N=8..13, $16 with a frame at N=14, and $25 at N=23, still framed. In the branchy form the address stays in $2. No measured form reaches $t9 without a prologue, and the target has none.
- verdict: KILLED
- kill_scope: class
- measured_on: ladder chassis tmp/grind/func_80052C10/s6/ladder.py (no FAKE construct), tools/gcc-2.7.2/build/cc1 with the Makefile:42 flags; baseline candidate.c single-store body (no FAKE)
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:2248

## [s6] Inter-function padding in the 3AB48 hand-asm run does not pin the module start at 0x80052788 or 0x80052A88.
- mechanism: s5b frontier item 3: look for a pad word that closes an object before either candidate start.
- probe: Function tails and map addresses for gte_SumSquares3 through gte_ReadFarColor (0x80052754..0x80052C10).
- result: Every function ends jr plus its delay slot, with the next symbol right after. There is no pad word in the range, so padding gives no boundary evidence and only the s5 start-relative 16-pad arithmetic bears on the module start.
- verdict: CONFIRMED

## [s7 forensics] Kill re-audit: the banked single-store candidate still scores 4 and has no FAKE construct.
- mechanism: Required re-audit (floor flat, instance kills exist).
- probe: sandbox --disable all --diff --candidate memory/grind/func_80052C10/candidate.c (tmp/grind/func_80052C10/s7/sandbox_candidate_diff.txt).
- result: Score 4 with the same two hunks: operand-only (lui/ori/sw seat $t9 vs our $v0) and source-level (the trailing nop). The body has no FAKE construct to ablate.
- verdict: CONFIRMED

## [s7 forensics] A hand-written reorder-mode source reproduces the whole 6-word target byte for byte: `li $25,0x1f800400; sw $0,0($25); j $31; nop`.
- mechanism: In reorder mode the assembler fills the `j $31` delay slot with its own nop; maspsx emulates ASPSX 2.34 and marks it `# DEBUG: branch/jump`. The author's explicit `nop` (a habit from noreorder style) then follows as a second word. `li` with a constant that has non-zero low bits expands to lui+ori on the author's chosen register $25. One mechanism explains BOTH residual hunks: the $t9 seat (author's choice) and the trailing nop (explicit nop after an assembler-filled slot). It needs no module-relative 16-pad, so it also clears the Judge's objection that 0x80052C28 is not 16-aligned (decisions.md:1382).
- probe: tmp/grind/func_80052C10/s7/handasm_reorder.s through `maspsx.py --aspsx-version=2.34`, then `mipsel-linux-gnu-as` with the Makefile AS_FLAGS, then objdump (handasm_test.sh -> handasm_reorder.txt).
- result: 3c191f80 37390400 af200000 03e00008 00000000 00000000, identical to the target words 801F193C 00043937 000020AF 0800E003 00000000 00000000 (LE).
- verdict: CONFIRMED

## [s7 forensics] The 3AB48 TU (or this function) was compiled with -mabicalls, giving $25 a copy preference.
- probe: whole-binary census of asm/funcs/*.s (tmp/grind/func_80052C10/s7/t9_census.sh -> t9_census.txt).
- result: No function anywhere has `jalr $t9`. The only function that writes $gp is __SN_ENTRY_POINT. With no .cpload or PIC call shape in the binary, the abicalls reading has no support.
- verdict: KILLED
- kill_scope: instance
- measured_on: asm/funcs/*.s census of the original binary's split asm (no C chassis, no FAKE)

## [s7 forensics] Frame-free $t9 constant seats occur only in hand-asm modules. The one compiled-C lui/ori $t9 seat (func_80060E38) is framed under heavy pressure, as the s6 class kill predicts.
- probe: t9_census.txt (10 functions with `lui $t9`), plus each one's context and source status.
- result: RotMatrix* are Sony libgte asm. gte_MulMatrix0ClearTrans (`lui $t9,0xFFFF`, in the 3AB48 run, canonical) and func_800520B8 (`lui/addiu $t9` + `jr $t9` with handwritten `add`/`sub`) are hand asm. func_8003E6D8 (299 insns) and func_80060E38 (src/main/51268.c:216, `addiu $sp,-0x70`, $fp in use, about 30 live scratchpad pointers) are compiled C, and both are framed under pressure.
- verdict: CONFIRMED

## [s7 forensics] Neighbourhood idiom census (s5b frontier item 3): func_80052C10 sits inside a contiguous run of 13 canonical hand-asm routines.
- probe: tmp/grind/func_80052C10/s7/run_census.py -> run_census.txt (0x80052600..0x80052D80).
- result: Every function from math_SquareRoot0 (0x800526A0) to func_80052C4C is in inline_asm_canonical.txt, frame-free, and uses $t registers. All but func_80052C10 carry handwritten-instruction (trapping add/sub) markers. The first compiled C after the run is func_80052D00 (framed, $s0-$s4). func_80052C10 is the only member with no trapping op, simply because it has no arithmetic. The module-head question (0x80052788 vs 0x80052A88) is moot now that the reorder-mode mechanism explains the pad word without module arithmetic.
- verdict: CONFIRMED

## s7 synthesis / frontier
Forensics found a single, simple producer for the exact target bytes: a reorder-mode hand-asm body with an explicit trailing nop. scan_hand_coded still reports LOW: S12 deliberately exempts a nop with no following instruction as "tail padding" (tools/scan_hand_coded.py:735-737), and S12 is informational anyway. The C side keeps both residuals with no compiled producer (s5: no second nop after jr's delay slot; s6: no frame-free $t9). The remaining disposition is outside the grind surface: an owner-approved signal class "explicit nop after an assembler-filled jr delay slot (jr;nop;nop at a function end not explained by alignment)" would give it a gate-class hit. That needs owner approval per the 2026-10-16 ruling, so it is not a grind lever.

## [s7] Kill re-audit: the banked single-store candidate.c still scores 4 with no FAKE construct present
- mechanism: Required re-audit while the floor is flat
- probe: sandbox --disable all --diff --candidate candidate.c
- result: Score 4: one operand-only hunk ($t9 vs $v0 lui/ori/sw seat) and one source-level hunk (trailing nop). Nothing to ablate.
- verdict: CONFIRMED

## [s7] A reorder-mode hand-written source `li $25,0x1f800400; sw $0,0($25); j $31; nop` assembles to exactly the 6 target words
- mechanism: In reorder mode the assembler (ASPSX, emulated by maspsx 2.34) fills the j $31 delay slot with a nop. The author's explicit nop becomes the 6th word, and li expands to lui+ori on the author's register $25. This explains both hunks without module padding.
- probe: tmp/grind/func_80052C10/s7/handasm_test.sh: maspsx --aspsx-version=2.34, then mipsel-linux-gnu-as with Makefile AS_FLAGS, then objdump
- result: 3c191f80 37390400 af200000 03e00008 00000000 00000000 == target words (LE 801F193C 00043937 000020AF 0800E003 0 0)
- verdict: CONFIRMED

## [s7] The 3AB48 TU was compiled with -mabicalls, giving the pseudo a hard-reg-25 copy preference
- mechanism: abicalls call templates move call targets into $25
- probe: census of asm/funcs/*.s for jalr $t9 and $gp writes (s7/t9_census.sh)
- result: Zero jalr $t9 in the binary. Only __SN_ENTRY_POINT writes $gp. No PIC fingerprint anywhere.
- verdict: KILLED
- kill_scope: instance
- measured_on: original-binary asm/funcs/*.s census; no C chassis, no FAKE construct

## [s7] The only compiled-C lui/ori $t9 seat in the binary (func_80060E38) is framed under heavy pressure, consistent with the s6 class kill
- mechanism: Ascending allocation reaches $25 only after $s0-$s7, which forces a frame
- probe: t9_census.txt plus source status of the 10 lui-$t9 functions
- result: func_80060E38 (src/main/51268.c:216) has addiu $sp,-0x70 and uses $fp. func_8003E6D8 is 299 insns and framed. The others are Sony libgte or canonical hand asm.
- verdict: CONFIRMED

## [s7] func_80052C10 sits inside a contiguous run of 13 canonical frame-free hand-asm routines (0x800526A0..0x80052C4C)
- mechanism: Neighbourhood idiom census (s5b frontier item 3)
- probe: s7/run_census.py
- result: Every neighbour is in inline_asm_canonical.txt with handwritten markers. The first compiled C (func_80052D00) is framed with $s regs. The module-head question is moot under the reorder-nop mechanism.
- verdict: CONFIRMED

## [s8, rederive] Fresh m2c (mips_to_c) decompile of asm/funcs/func_80052C10.s suggests no new chassis
- mechanism: independent structural re-derivation from the raw asm, bypassing the engine's own candidate history
- probe: `python3 tools/m2c/m2c.py -t mips-ido-c asm/funcs/func_80052C10.s` (freshly invoked this session; tmp/grind/func_80052C10/s8/m2c_out.c)
- result: m2c emits `void func_80052C10(void) { *(s32 *)0x1F800400 = 0; }` -- structurally IDENTICAL to the already-banked candidate.c (single volatile-style store, no second statement, no trailing-nop suggestion, no register-seat hint of any kind). m2c's MIPS model has no awareness of register-seat preference or trailing nop/delay-slot semantics beyond basic disasm, so it cannot propose anything the engine hasn't already measured.
- verdict: KILLED
- kill_scope: instance
- measured_on: m2c mips-ido-c target against the unmodified target asm; no FAKE construct (pure re-derivation, no chassis applied to src/)

## [s8, rederive] decomp.me corpus (gcc2.7.2-psx bucket, 461 scratches, tmp/decomp_me_corpus) has no citable near-match for this function's asm shingles
- mechanism: cross-corpus similarity search for a human-submitted decompilation of the identical or near-identical instruction sequence
- probe: `python3 tools/decomp_me_scrape.py search --asm-file asm/funcs/func_80052C10.s --corpus tmp/decomp_me_corpus --top 10` (tmp/grind/func_80052C10/s8/decomp_me_search.txt)
- result: single hit, similarity 0.011 (SpuSetKey, decomp.me/scratch/Bbxp2) -- an order of magnitude below any citable threshold (c.f. func_80052B7C s12's 0.022-0.026 non-citable range). No corpus precedent for a 6-insn lui/ori/sw/jr/nop/nop MMIO-zero-store body with a $t9 seat.
- verdict: KILLED
- kill_scope: instance
- measured_on: decomp_me_scrape.py search against tmp/decomp_me_corpus/gcc2.7.2-psx bucket; no chassis change, no FAKE construct

## [s8, rederive] Sibling transplant check: func_80052B7C (unspent sibling, floor 19, 28 sessions) shares the TU but not the mechanism -- nothing to transplant
- mechanism: cross-ledger inheritance check mandated by the sibling-ledgers block
- probe: read memory/grind/func_80052B7C/candidate.c (s1-s28) in full
- result: func_80052B7C's residual is GTE (cop2) macro-internal instruction batching and delay-slot placement inside fixed __asm__ volatile strings (gte_SetRotMatrix / gte_stlvl) -- a completely different class of problem (macro-opaque asm_operands RTL, no free C-level variables) from func_80052C10's register-seat + trailing-nop residual on a single scalar store. func_80052B7C's own candidate.c explicitly confirms (s22) it already checked siblings and found nothing transplantable in the other direction. Re-confirms s3's prior finding that no shared construct exists between these two functions.
- verdict: KILLED
- kill_scope: instance
- measured_on: read-only comparison of memory/grind/func_80052B7C/candidate.c against func_80052C10's residual; no transplant attempted (no shared construct), no FAKE construct

## [s8, rederive] Sibling transplant check: func_80060E38 (COMPLETED-C, matched) reaches a lui/ori-$t9 seat for a scratchpad address, but only inside a large framed function, corroborating s6's solver class kill from the actual matched binary (not just simulation)
- mechanism: real matched-C evidence for the pressure-ladder-needs-a-frame finding
- probe: grep "t9" asm/funcs/func_80060E38.s (tmp/grind/func_80052C10/s8/sibling_t9_check.txt) -> src/main/51268.c:228 `D_800A34A8 = &SPAD51268->unk68;` inside func_80060E38 (addiu $sp,$sp,-0x70, uses $s0-$s7/$fp per s7's prior census)
- result: the $t9 seat there computes the ADDRESS of a scratchpad struct member and stores that POINTER into a global (`sw $t9, %gp_rel(D_800A34A8)($gp)`), not a direct zero-store to the scratchpad -- a different statement shape from func_80052C10's `*(vu32*)ADDR = 0`. It only reaches $t9 because func_80060E38 is a ~360-insn function with dozens of live pseudos and a full $s0-$s7+$fp save/restore frame at that point in the instruction stream -- exactly the "N>=22 live scratch pseudos, needs a frame" condition s6's solver predicted (local-alloc.c:2249 ascending free-reg scan). No frame-free analogue exists in the matched binary to transplant onto func_80052C10's 1-statement, 0-frame body.
- verdict: CONFIRMED

## [s8, rederive] Sibling transplant check: func_800520B8 uses $t9 for `jr $t9` jump-table dispatch (explicit "handwritten instruction" comment at both sites), not for a data store -- reconfirms s7's abicalls/PIC-register-convention reading, no new store-address lever
- mechanism: $t9/$25 is the MIPS o32/n32 convention register for an indirect call/jump target, independent of any lui/ori-into-store-address idiom
- probe: grep "t9" asm/funcs/func_800520B8.s (tmp/grind/func_80052C10/s8/sibling_t9_check.txt)
- result: both lui/addiu/add/jr-$t9 sequences in func_800520B8 are hand-asm-marked ("/* handwritten instruction */") jump-table computed-gotos, structurally unrelated to a store-address computation. Nothing to transplant; this is the same $t9-as-branch-target idiom already covered by s7's zero-jalr-$t9 census, not a new store-seat mechanism.
- verdict: KILLED
- kill_scope: instance
- measured_on: read-only asm census of asm/funcs/func_800520B8.s; no chassis change, no FAKE construct

## [s8] A fresh m2c (mips_to_c) decompile of asm/funcs/func_80052C10.s produces a structurally identical body to the banked candidate (single volatile-style store, no second statement, no register-seat or trailing-nop hint).
- mechanism: independent re-derivation from raw asm, bypassing engine candidate history
- probe: python3 tools/m2c/m2c.py -t mips-ido-c asm/funcs/func_80052C10.s
- result: Output: void func_80052C10(void) { *(s32 *)0x1F800400 = 0; } -- byte-for-byte the same shape as candidate.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: m2c mips-ido-c target vs unmodified target asm; no chassis applied, no FAKE construct

## [s8] The decomp.me corpus (gcc2.7.2-psx bucket, 461 scratches) has no citable near-match for this function's instruction shingles.
- mechanism: cross-corpus similarity search for a human decompilation of the same or near-identical asm
- probe: python3 tools/decomp_me_scrape.py search --asm-file asm/funcs/func_80052C10.s --corpus tmp/decomp_me_corpus --top 10
- result: Single hit, similarity 0.011 (SpuSetKey) -- an order of magnitude below citable (c.f. func_80052B7C s12's 0.022-0.026 non-citable range).
- verdict: KILLED
- kill_scope: instance
- measured_on: decomp_me_scrape.py search against tmp/decomp_me_corpus/gcc2.7.2-psx; no chassis change, no FAKE construct

## [s8] Sibling func_80052B7C (unspent, 28 sessions, floor 19) shares the 3AB48 TU but shares no transplantable construct with func_80052C10.
- mechanism: cross-ledger inheritance check (sibling-ledgers block)
- probe: read memory/grind/func_80052B7C/candidate.c s1-s28 in full
- result: func_80052B7C's residual is GTE macro-internal instruction batching/delay-slot placement inside fixed __asm__ volatile strings -- a different problem class (macro-opaque asm_operands RTL, zero free C-level variables) from func_80052C10's register-seat + trailing-nop residual on a plain scalar store.
- verdict: KILLED
- kill_scope: instance
- measured_on: read-only comparison, no transplant attempted (no shared construct), no FAKE construct

## [s8] func_80060E38 (COMPLETED-C, matched) reaches a lui/ori-$t9 seat for a scratchpad address, corroborating s6's solver class kill (t9 reachable only under heavy register pressure with a frame) from real matched-C evidence rather than simulation alone.
- mechanism: local-alloc.c:2249 ascending free-register scan reaches $25/$t9 only after $s0-$s7 are occupied, which requires a save/restore frame
- probe: grep t9 asm/funcs/func_80060E38.s -> src/main/51268.c:228 D_800A34A8 = &SPAD51268->unk68; inside a ~360-insn, addiu $sp,-0x70 framed function
- result: The t9 seat there stores a computed POINTER into a global, not a direct zero-store, and only arises inside a function with dozens of live pseudos and a full $s0-$s7+$fp frame. No frame-free analogue exists in the matched binary.
- verdict: CONFIRMED

## [s8] func_800520B8's lui/addiu/add/jr-$t9 sequences are hand-asm-marked jump-table dispatch (the ABI call-target convention for $t9/$25), not a store-address computation -- no new store-seat lever, reconfirms s7's abicalls census.
- mechanism: $t9/$25 is the MIPS indirect call/jump target convention register, independent of any store-address lui/ori idiom
- probe: grep t9 asm/funcs/func_800520B8.s
- result: Both t9 sequences carry explicit "/* handwritten instruction */" markers and feed jr $t9 computed-goto dispatch; structurally unrelated to func_80052C10's residual.
- verdict: KILLED
- kill_scope: instance
- measured_on: read-only asm census, no chassis change, no FAKE construct

## [s9] The literal address 0x1F800400 in func_80052C10's store corresponds to a field inside an existing declared struct (Unk1F800000Rec / its Unk1F8000B8Union tail) that should be accessed via that struct instead of a raw cast, per the split-scalars-hide-aggregate / header-type-correction levers.
- mechanism: aggregate-merge-family / header-type-correction-from-use-sites: a mistyped or under-modeled global accessed through a raw offset cast instead of its real declared type
- probe: Read include/game.h:1017-1077 (Unk1F8000B8Union, Unk1F800000Rec) and compute sizeof(Unk1F800000Rec) = 0xB8 + sizeof(Unk1F8000B8Union.raw[0x400-0xB8]) = 0x400; compare against 0x1F800400 - 0x1F800000 = 0x400. Cross-check with grep over named_syms.txt/symbol_addrs.txt/undefined_syms_auto.txt/src/main/*.c for '1F800400' and '0x1F8004'.
- result: sizeof(Unk1F800000Rec) is exactly 0x400, so the struct spans 0x1F800000..0x1F8003FF and 0x1F800400 is the first byte PAST its end, not a member inside it. Zero census hits and zero other source references to 0x1F800400 anywhere in the project -- func_80052C10 is the only code that touches this address, so no sibling evidence suggests a different intended shape either. The bare-literal `*(vu32 *)0x1F800400 = 0;` is already the correct, simplest declaration; no struct exists to redeclare through.
- verdict: KILLED
- kill_scope: instance
- measured_on: include/game.h:1017-1077 struct definitions (static read) + grep census over named_syms.txt/symbol_addrs.txt/undefined_syms_auto.txt/src/main/*.c for '1F800400'/'0x1F8004' (zero hits beyond this function); candidate.c unchanged; sandbox re-confirms floor 4, no chassis change, no FAKE construct

## [s10] Kill re-audit: the framed pressure-22 instance kill re-measured on the current tree still scores far above floor, and contains no FAKE carrier.
- mechanism: kill re-audit mandate (floor flat, instance kills exist)
- probe: sandbox --disable all --candidate rejected/blocklocal-pressure-22-no-t9.c; fake_ablate.py on it and on candidate.c
- result: 212 (214 build insns vs 6); fake_ablate: nothing to ablate on either form. candidate.c re-measures 4 (1 operand-only $t9 seat, 1 source-level trailing nop). Kill stands.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/blocklocal-pressure-22-no-t9.c on current main tree, no FAKE construct

## [s10] Escalation gates: scan_hand_coded LOW 0/12 (gate a FAIL); SOTN-index census for $t9 seat / trailing nop = 0 (gate b FAIL); cc1psx self-disproof 4/4 closer=false. Rotation record filed in docs/grind/decisions.md (2026-10-16 OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): ROTATED).
- verdict: CONFIRMED

## [s10] The framed pressure-22 form (rejected/blocklocal-pressure-22-no-t9.c) might score at or below floor 4 when re-measured on the current tree with FAKE constructs ablated.
- mechanism: kill re-audit mandate: instance kills are chassis-relative
- probe: sandbox func_80052C10 --disable all --candidate rejected/blocklocal-pressure-22-no-t9.c; fake_ablate.py on that form and on candidate.c
- result: 212 (214 build insns vs 6 target); no FAKE construct to ablate in either form; candidate.c re-measures 4. Kill stands.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/blocklocal-pressure-22-no-t9.c on current main tree, no FAKE construct

## [s10] Endgame gates for func_80052C10: canonical-asm gate needs a STRONG scan tier; coercion gate needs a cited SOTN precedent.
- mechanism: judge-sole-gate AND-gates
- probe: scan_hand_coded.py --single func_80052C10; grep sotn-construct-index.md for t9/$25/trailing nop/scratchpad
- result: scan LOW 0/12, no gate-class signal; precedent census 0 relevant hits (one unrelated scratchpad throwaway local). Both gates fail; cc1psx_check ours 4 / psx 4 closer=false.
- verdict: CONFIRMED

# Hypothesis ledger — InitHiraRmd_80047FBC

## Frontier (session s1 → s2)

### KEY OBSERVATION (correction from initial reading)
Sandbox at offset 0x1b4 ALREADY emits `sw v0, 16(sp)` in the jal delay slot — GCC
produces the discarded-return spill by itself (no explicit `buf[k] = call();` needed
in the C). The 32-byte `buf[8]` reserves the frame; GCC schedules the discarded v0
into the free slot in the delay slot. **Target and sandbox agree on the dead store.**
The only diff is target-insn-#18 `addu $s0, $s4, $v0` vs sandbox `addu s0, a0, v0`.

Consequence: making `buf[k] = call()` explicit is unlikely to move the residual — it
mostly re-legitimizes the frame reservation under the [[dead-vars-local-array]]
2026-07-01 carve-out (WRITTEN-never-read requirement), but does not attack the
copy-prop that keeps $a0 == arg0 at insn #18.

### H1 — force $s4 retention by making `base` visibly diverge from `arg0`
**Statement:** GCC copy-props `base == arg0` since both trace to $a0 with no
intervening redefinition. Instead of trying to break the association via a dead
self-assign to arg0 (proven KILLED, see H3), give `base` a real (byte-identical but
optimizer-opaque) reason to occupy a callee-save. Candidates:
  (a) `u32 *base; base = (u32 *)arg0; if (base != (u32*)0) { }` — trivially-true branch
      forcing a data-dependent live range on `base`.
  (b) reorder: compute `p = base + shifted_a1` in one statement, then reload `p` from
      `base + word_offset`; hope RA schedules `base` into $s4.
  (c) accumulate arg1's shift into `base` first via `base = arg0; base += shifted_a1;
      base -= shifted_a1;` — variant of split-init-accumulation (see
      [[split-init-accumulation-sanctioned]]).

**Probe (s2):** apply (a) first (least invasive); measure sandbox distance and dump
disasm. If distance stays at 1, iterate through (b) then (c).

### H2 — permuter directed pass on `base`/`arg0` binding
**Statement:** the residual is a single register-choice diff at a specific insn. This
is exactly the shape decomp-permuter's directed PERM_* macros are designed to sweep
(see [[permuter-directives]]). Auto-search may find a legal C form that the manual
levers in H1 miss.

**Probe (s2 or s3):** build a permuter workspace, seed with the current form minus
`arg0 = 0;` (which is a proven-dead lever we don't want the permuter re-discovering),
and run a directed campaign varying declaration order + copy chain.

### H2 — force $s4 retention via base-through-non-$a0 CSE
**Statement:** GCC copy-props `base == arg0` because both variables trace to $a0's
initial value and nothing between defines them differently. If `base` is initialized
from a *derived* value (e.g. `base = (u32*)arg0; base = (u32*)((s32)base + 0); base -= 0;`)
GCC may still fold — but if the assignment path passes through a callee-save
register naturally, the copy stays.

**Mechanism:** currently `p = (u32 *)arg0; base = p;` — `base` becomes an alias of $a0
after copy-prop. Alternative: assign `base` FIRST, then `p`, and make later uses of
`p` diverge from `arg0`. This might make GCC materialize `base` into a callee-save
because it's live across the loop's callees.

**Probe (s2):** rewrite as `u32 *base = (u32 *)arg0; u32 *p = base;` — declaration
order swap. Also try `u32 *p = (u32*)arg0; u32 *base = p;` vs `u32 *base = (u32*)arg0;
u32 *p = (u32*)((s32)base + shifted_a1);` (skip the intermediate `p = arg0` step).

### H3 — the current `arg0 = 0;` trick provably fails; not a viable rescue lever
**Statement:** the L108 `arg0 = 0;` in current src does NOT force $s4 usage at
insn #18 (sandbox still emits `addu s0, a0, v0`). Documenting this as a KILLED
mechanism so s2+ do not re-try the "dead self-assign breaks copy-prop" theory in this
function's shape.

**Verdict:** KILLED by s1 measurement (sandbox score=1 with the trick in place).

## Sibling context
- The whole 8004xxxx cluster (AddTbpOfst_80047EE8, InitHiraRmd_800480C0, func_800481E8)
  is cheat-carrying INCOMPLETE — do NOT copy their forms as templates.
- Target dead-store `sw $v0, 0x10($sp)` is the strongest signal in the target asm; H1
  is the primary lever for s2.

## [s1] L108 `arg0 = 0;` breaks GCC's $a0==arg0 value-association at the second `base` use, causing insn #18 to bind to $s4
- mechanism: dead self-assign-to-param as a copy-prop severing trick (Lever D variant; forbidden without /* FAKE */ + lever-exhaustion)
- probe: current src already has the construct; run sandbox --disable all and read insn #18
- result: sandbox score=1; insn #18 is `addu s0, a0, v0` (still $a0). Mechanism does not fire in this function's shape.
- verdict: KILLED

## [s1] the dead `sw $v0, 0x10($sp)` in target's jal delay slot requires an explicit `buf[k] = efc_buki_draw_zanzou(...);` capture to be legitimized under the dead-vars-local-array 2026-07-01 carve-out
- mechanism: written-never-read local array with target dead-stores matched by explicit stores in the C source
- probe: objdump sandbox .o at jal delay slot
- result: sandbox ALREADY emits `sw v0, 16(sp)` at offset 0x1b4 with no explicit buf[k]= in the C — GCC schedules the discarded return into the free frame slot on its own. The store is not the residual; the copy-prop is.
- verdict: KILLED

## [s2] H1a: assigning base FIRST (u32 *base = (u32*)arg0) then deriving p from base fixes insn #18 to $s4
- mechanism: removing the intermediate `p = arg0; base = p` chain makes base the primary reference so GCC materializes it directly into $s4
- probe: s2 sandbox --disable all: base = (u32*)arg0; p = base + shifted; p = base + word
- result: score 5, build 64 insns. Insn 0x120 IS now addu s0,s4,v0 (matches target), but prologue emits `move s4,a0` directly (single move) instead of target's staged `move s0,a0; move s4,s0` — save order flips (s4 saved before s0) and one insn is eliminated. Net: fixes one diff, introduces multiple new diffs.
- verdict: KILLED

## [s2] H1b: parallel init (p = arg0; base = arg0) produces a different codegen than the chained (p = arg0; base = p) form
- mechanism: distinct initializers might give GCC different value-numbering / copy-prop analysis
- probe: s2 sandbox --disable all: p = (u32*)arg0; base = (u32*)arg0; p = base + shifted; p = base + word
- result: score 1, byte-identical to the chained baseline. GCC folds parallel and chained inits to identical output — copy-prop collapses base==arg0 either way. Same single residual at insn #18.
- verdict: KILLED

## [s2] H1c: declaring base in an inner scope (after p is derived) shortens its lifetime and improves allocation
- mechanism: narrower live-range of base might let GCC pick a different register or avoid the copy-prop tie
- probe: s2 sandbox --disable all: outer p = arg0; p += shifted; then inner { u32 *base = (u32*)arg0; ... loop ... }
- result: score 8, build 63 insns. Inner-scope base drops the s4 setup entirely and shifts the loop-body allocation, introducing multiple new diffs. Strictly worse.
- verdict: KILLED

## [s2] H1d: declaring base as s32 base_addr (integer) instead of u32 *base (pointer) gives GCC a different aliasing/allocation profile
- mechanism: s32 vs pointer type might affect pointer-aliasing analysis, address-mode selection, or RA cost model
- probe: s2 sandbox --disable all: p = (u32*)arg0; base_addr = arg0 (as s32); p = (u32*)(base_addr + shifted); p = (u32*)(base_addr + word)
- result: score 1, byte-identical to H1b. GCC treats s32 and u32* the same after copy-prop; the address arithmetic collapses to identical MIPS. Same single residual, BUT reaches score=1 WITHOUT the arg0=0 cheat (previously believed load-bearing).
- verdict: KILLED (bytes) / CONFIRMED as inert-cheat-eliminator (progress: reduces committed candidate from 2 cheats to 1)

## [s2] H3 confirmation: the `arg0 = 0;` dead self-assignment in the s1-committed candidate is INERT for insn #18
- mechanism: prior belief (per s1) was that this KILLED the copy-prop severance mechanism; H1d now confirms score is identical WITHOUT it, so it is providing zero codegen effect
- probe: s2: removed arg0 = 0; from src, re-ran sandbox with H1d form
- result: score 1 (unchanged from with the cheat). The construct provides zero benefit. Committed candidate carries it purely as a failed hypothesis remnant.
- verdict: CONFIRMED (arg0=0 is inert here — safe to drop from candidate)

## [s3] removing the unused `s32 buf[8]` local shrinks the frame and preserves the sw v0,0x10(sp) delay-slot store (GCC allocates the slot itself)
- mechanism: test whether buf[8] is decorative frame padding or genuinely load-bearing per the s2 frontier probe
- probe: s3 (structural): edit src to remove `s32 buf[8];` + `(void)buf;`; sandbox --disable all; disasm
- result: score 1→15, target_insns 65 vs build_insns 65. Frame shrinks 0x50→0x30. `sw v0,0x10(sp)` still emerges (GCC-allocated) but many stack offsets shift by 0x20 causing 15 differing instructions across the function. buf[8] IS load-bearing for the frame reservation — it is NOT decorative.
- verdict: KILLED

## [s3] splitting `count = *p++` into `word_offset = ((*p)>>2)<<2; count = *(base_addr+word_offset); p = (base_addr+word_offset+4)` lifts base_addr's reg_n_refs to 3 pre-loop, tilting GCC's local-alloc tiebreaker toward $s4 at insn #18
- mechanism: add explicit legitimate base_addr uses to raise its allocation priority per [[duplicated-statement-into-arms]] / [[difficult-is-not-impossible]] Lever D-style RA steering
- probe: s3 (structural): apply the split-recompute form; sandbox --disable all
- result: score 3, 65/65 insns. Insn #18 becomes `addu a0,a0,v0` (a0 still holds the base value); the two explicit `base_addr + word_offset` uses fold via CSE into a single computation folded through $a0 — the split does not increase distinct references seen by local-alloc AFTER CSE. New downstream diff at addr-materialization for subsequent lw. Copy-prop still wins the tie.
- verdict: KILLED

## [s3] swapping the initialization order to `base_addr = arg0; p = (u32*)arg0;` changes LUID/reg_n_deaths analysis and may retain base_addr in $s4 at insn #18
- mechanism: declaration-order structural probe (part of the s3 structural modality catalog: `.claude/rules/codegen-technique-index.md`)
- probe: s3 (structural): swap init order; sandbox --disable all
- result: score 1 unchanged, byte-identical to the p-first form. Copy-prop analysis identical regardless of statement order. LUID reshuffle does not budge insn #18. (Retained in candidate as no-cost stylistic preference.)
- verdict: KILLED

## [s3] hoisting `new_var = base_addr + ((word>>2)<<2)` earlier in the loop body (right after the first `p += 4;`) shortens base_addr's post-hoist idle live range and might change RA weighting at insn #18
- mechanism: loop-body statement reorder — within-scope structural axis
- probe: s3 (structural): move new_var compute above the a1v load; sandbox
- result: score 1 unchanged. GCC scheduler recovers the same placement regardless of source position within the load block. Insn #18 diff unaffected.
- verdict: KILLED

## [s3] deferring `new_var = base_addr + ((word>>2)<<2)` to the very end of the loop body (immediately before the call) frees base_addr's live range across the reads and may relocate the residual
- mechanism: opposite-direction loop-body reorder
- probe: s3 (structural): move new_var compute to after v0v load; sandbox
- result: score 13. Late placement changes scheduling of the multi-lhu block AND the call-arg materialization; 12 new diffs across the load and pre-call region. Mid-loop position (between a2v and a3v reads) is load-bearing for match. Not a winning direction.
- verdict: KILLED

## [s4] A fresh-seed PERM_* directed campaign over the s3 candidate can find a byte-closing form that lifts base_addr into $s4 at insn #18 via a legitimate C structural mutation
- mechanism: auto-search sampling over declaration order / int-type / constant-hoisting / value-aliasing per [[permuter-directives]]; the residual is a single-insn RA tiebreaker exactly the shape permuter is designed for
- probe: s4 (permuter modality): built permuter workspace (tmp/grind/InitHiraRmd_80047FBC/s4/perm_ws/) with pre-preprocessed base.c, per-func compile.sh extracting the InitHiraRmd_80047FBC region, target.o at offset 0 (base_score=5 = one reg-diff x 5). Launched detached campaign s4_chassis1 -j 6 --stop-on-zero; polled in-turn; harvest+stop before session end.
- result: converged in 9.6s (217 iterations); single output-0-1 at score 0. The closing form stacks THREE no-semantic-purpose constructs: (1) inherited buf[8] unused array, (2) `int new_var3 = 16;` used only as the shift count in `arg1 << new_var3` (constant-holder scalar), (3) `s32 new_var2; ... new_var2 = sx_arg2;` used in 2/4 call-arg summands (same-value local alias of live sx_arg2). Applied to src/text1b.c: sandbox=0, 65/65 insns confirmed. Reverted to s3 form after cheat vetting. Bytes proven; classification determined by inspection.
- verdict: KILLED

## [s5] A structurally different chassis that drops base_addr entirely and uses arg0 directly in both offset computations (p init + new_var), per frontier probe 1, produces a lower/different baseline whose permuter basin contains a non-cheat score-0 form.
- mechanism: The s1-s4 chassis all keep parallel derivations (p and base_addr both from arg0). Chassis C uses arg0 as the single anchor across the call; GCC must preserve arg0 in a callee-save to survive efc_buki_draw_zanzou, potentially reaching the target's $s4=arg0 shape without a separate base_addr local. Permuter mutations over this chassis explore a different search space than the s4 chassis and may find a byte-closing form outside the cheat family the s4 campaign converged on.
- probe: Applied chassis C to src/text1b.c (see tmp/grind/InitHiraRmd_80047FBC/s5/chassis_c_arg0_direct.c and rejected/s5_chassis_c_arg0_direct_no_base.c). Sandbox --disable all baseline. Built permuter workspace (perm_ws/) mirroring s4 with new base.c; launched campaign s5_chassis_c_arg0_direct (fresh seed, -j 6, --stop-on-zero, --stack-diffs default). Polled in-turn ~15 min; harvest --stop.
- result: Chassis C sandbox=5, build_insns=64/target 65. Disasm diff (65/65 lines after normalization): chassis C DOES reach the target-matching `addu $s0,$s4,$v0` at insn 0x120 (the s1-s4 residual) — but LOSES the target's staged prologue (`move s0,a0; move s4,s0; addu s0,s0,a1`; 3 insns), collapsing to `move s4,a0; addu s0,s4,a1` (2 insns) with save-order flip. Net worse. Permuter campaign: 2747 iterations, base_score=170 (permuter metric penalizes each byte diff × 5 for reg + stack-diff coverage), best iter score ~170 (baseline), 0 finds. No score-0 basin located.
- verdict: KILLED

## [s5] The residual `addu $s0,$s4,$v0` vs `addu $s0,$a0,$v0` and the target's staged prologue are TWO INDEPENDENT constraints (copy-prop tiebreaker at insn #18 AND explicit `move s0,a0` before `move s4,s0`) that the C source must satisfy simultaneously.
- mechanism: s1-s4 chassis (with base_addr local) satisfy the prologue staging (s0=arg0, s4=s0 via base_addr's callee-save preference) but LOSE the copy-prop tie at insn #18 (score 1). Chassis C (no base_addr) satisfies the copy-prop tie at insn #18 (via arg0-live-across-call forcing s4 pin) but LOSES the prologue staging (no need for the extra `move s0,a0`). Anti-correlated axes: neither chassis is closer than the other by structural rewrite alone.
- probe: Comparison of chassis C disasm vs s3-candidate disasm vs target asm.
- result: Chassis C's insn 0x120 IS `addu s0,s4,v0` (matches target), but its prologue has 7 insns (vs target's 8) and register-choice diffs on the staging insns. s3 candidate's prologue matches target exactly (8 insns) but its insn #18 is `addu s0,a0,v0`. Two axes cannot be closed independently by chassis choice.
- verdict: CONFIRMED

## [s6] cse2 (second CSE pass) rewrites operand (reg 79) into canonical class member (reg 72) at insn 36, causing greg to emit `(reg 16 s0) = (plus (reg 4 a0) (reg 2 v0))` instead of target's `(plus (reg 20 s4) (reg 2 v0))`.
- mechanism: cse.c canon_reg/find_qty_regno places regs 72 (arg0), 78 (p), 79 (base_addr) in one equivalence class rooted at reg 72; cse2 walks insn 36 and substitutes the class root. Confirmed by RTL pass-dumps: reg 79 is preserved through rtl/cse/loop and rewritten to reg 72 ONLY at cse2, then unchanged through combine/lreg/greg. Named pass: `cse2` invoked via toplev.c `rerun_cse_after_loop`.
- probe: Re-verified prior s6 dumps in tmp/grind/InitHiraRmd_80047FBC/s6/: insn36_evolution.txt matches text1b.i.rtl..text1b.i.greg per-pass grep; sandbox --disable all baseline this turn score=1 (65/65) confirms current committed src still hits the same residual; probe_arg0zero/ dumps show cse2 fold suppressed when arg0=0 is present, greg then emits target-shape `(reg 16 s0) = (plus (reg 20 s4) (reg 2 v0))`, raw cc1 asm `addu $16,$20,$2` and post-maspsx final.s line 134 both target-matching.
- result: cse2 canonical-reg substitution at insn 36 is the named pass and decision producing the s1-s5 residual. arg0=0 defeats it at compile time; the sandbox stripper (engine/volatile_cheats.py:791 find_dead_param_assigns) removes un-annotated arg0=0 before scoring, so all s1-s5 sandbox measurements of this lever were of the stripped form — the KILLED verdict was about the stripper, not the compiler. FAKE-annotated instances bypass the stripper (line 815 `_stmt_fake_annotated` guard) and IS visible to cc1.
- verdict: CONFIRMED

## [s6] The FAKE-annotated `arg0 = 0; /* FAKE: defeats cse2... */` form banked in rejected/s6_arg0zero_faked.c qualifies under the [[dead-store-fake-exception]] 2026-07-01 carve-out.
- mechanism: All four rule prerequisites are met: (1) lever-exhaustion documented across s1-s5 (~14 measured-KILLED hypotheses); (2) GCC pass named (cse2 canonical-reg substitution at insn 36, {reg 72, 78, 79} equivalence class); (3) `/* FAKE */` annotation present verbatim on the statement; (4) the exact spelling `arg0 = 0;` is enumerated in dead-store-fake-exception.md line 34 as the archetype.
- probe: Judge ruling filed 2026-07-20 03:22 in docs/grind/decisions.md (committed as 94ba752b).
- result: Judge PASS — legitimacy of the isolated arg0=0 FAKE lever confirmed. Scope explicitly does NOT clear a FINAL CALL: composite candidate still carries `s32 buf[8]; (void)buf;` (independent blocker per s3 evidence: load-bearing for 0x50 frame, fails dead-vars-local-array WRITTEN test).
- verdict: CONFIRMED

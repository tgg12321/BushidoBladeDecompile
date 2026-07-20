# Evidence bank — InitHiraRmd_80047FBC

## Session s1 (recon, 2026-07-20)

- Canonical gate: verdict=C, distance=1 (pure-C target).
- Sandbox `--disable all`: honest score=1, target=65 insns, build=65 insns.
- Current src/text1b.c:77 body ALREADY carries two forbidden constructs and STILL misses by 1 insn:
  - `s32 buf[8];` at L101 with only `(void)buf;` at L142 — WRITTEN-NEVER-READ carve-out
    ([[dead-vars-local-array]] 2026-07-01) requires actual STORES into the array; this
    array has zero stores, so it is a plain unused-array cheat (catalog entry, forbidden).
  - `arg0 = 0;` at L108 — dead self-assignment of a parameter; forbidden by
    [[inline-asm-policy]] expanded catalog unless annotated `/* FAKE */` after documented
    lever-exhaustion per [[dead-store-fake-exception]]. Neither is in place.

## The residual — sandbox vs target diff

Sandbox and target agree on 64/65 instructions. The single divergence is at target
insn #18 (offset 0x387FC, `addu $s0, $s4, $v0`) vs sandbox offset 0x24
(`addu s0, a0, v0`). Second computation of `p = (u32 *)((s32)base + (((*p) >> 2) << 2))`:

- Target uses $s4 (the callee-save copy of arg0 made in prologue: `addu $s4, $s0, $zero`).
- Sandbox uses $a0 directly — GCC copy-props `base == arg0` since $a0 still holds arg0
  at this point.

The current C body's L108 `arg0 = 0;` was written specifically to sever this
copy-prop association ("breaks GCC's $a0==arg0 value association, so this reference
binds to $s4"). Measurement: **it doesn't work** — sandbox still emits `addu s0, a0, v0`.
The stated mechanism failed.

## The dead store in target — `sw $v0, 0x10($sp)` in the jal delay slot

Target insn #55 (`sw $v0, 0x10($sp)` in the delay slot of `jal efc_buki_draw_zanzou`)
stores a value into stack slot 0x10 that is never reloaded. The offset 0x10 lies
inside the 32-byte frame block above the callee-save area (buf[0..7] at sp+0..sp+0x1c
if buf were the first local — so 0x10 = buf[4]). This is strong evidence that the
original C had a WRITTEN dead-store — either:
  (a) A written-never-read local array with `buf[k] = efc_buki_draw_zanzou(...);`
      qualifying under the [[dead-vars-local-array]] 2026-07-01 carve-out; OR
  (b) An earlier-frame home for a discarded call return via
      [[restore-discarded-return-displaces-v0]] variants.

Sandbox current form has zero explicit stores at sp+0x10; the store is coming
from some other codegen path — need to inspect. (Left for s2 to trace.)

## Sibling shape

`AddTbpOfst_80047EE8` (src/text1b.c:19) — same cluster — carries `register asm` pins +
`INLINE_MOVE_ALIASING` __asm__ move + `s32 unused_slack[8]` + `(void)unused_slack;`.
That sibling is also cheat-carrying INCOMPLETE; not a template to copy.

`InitHiraRmd_800480C0` (src/text1b.c:144) — carries multiple `register asm("$N")` pins
too. Cheat-carrying.

`func_800481E8` (src/text1b.c:191) — carries `register asm("$16") cached` + a
`move %0,%1` __asm__ block (INLINE_MOVE_ALIASING). Cheat-carrying.

**Conclusion:** the entire cluster is currently living on the forbidden
INLINE_MOVE_ALIASING / dead-vars-local-array / register-asm-pin family. Cluster-wide
solution likely lies in a WRITTEN dead-store pattern that qualifies under the 2026-07-01
carve-outs, OR a genuinely different C shape that makes arg0-copy → $s4 natural.

## Artifacts
- `tmp/grind/InitHiraRmd_80047FBC/s1/sandbox_disasm.txt` — objdump of sandbox `.o`.

- [s1] canonical InitHiraRmd_80047FBC → verdict=C, distance=1 (pure-C target, 65 target insns)

- [s1] sandbox --disable all → score=1, rules_dropped=0, cheat_asm_stripped=393 (whole-file counter, not per-func)

- [s1] residual is single insn at target offset 0x387FC (insn #18) `addu $s0, $s4, $v0` vs sandbox offset 0x24 `addu s0, a0, v0` — computation of `p = base + ((*p >> 2) << 2)`

- [s1] current src/text1b.c:77 body carries TWO forbidden constructs and still misses by 1: (a) `s32 buf[8];` with only `(void)buf;` — unused array, no stores, does not qualify for the dead-vars-local-array 2026-07-01 WRITTEN-never-read carve-out; (b) `arg0 = 0;` at L108 — dead self-assignment of parameter, forbidden without /* FAKE */ + lever-exhaustion per dead-store-fake-exception

- [s1] sandbox and target BOTH emit `sw v0, 16(sp)` in the jal delay slot — GCC schedules the discarded efc_buki_draw_zanzou return value into the free slot in `buf[]` automatically; no explicit buf[k]= is required for that store to appear

- [s1] prologue and epilogue byte-match target — frame=0x50, save order $s0/$s4/$ra/$s3/$s2/$s1 all correct; the `buf[8]` declaration IS load-bearing for the 32-byte frame reservation

- [s1] sibling cluster (AddTbpOfst_80047EE8, InitHiraRmd_800480C0, func_800481E8) all remain cheat-carrying INCOMPLETE (register-asm pins, INLINE_MOVE_ALIASING __asm__ moves, unused_slack arrays) — NOT templates to copy

- [s2] [s2] baseline sandbox (as-committed src): score=1, 65/65 insns, single residual at insn #18 (target `addu $s0,$s4,$v0` vs sandbox `addu $s0,$a0,$v0`)

- [s2] [s2] the target prologue `move s0,a0; move s4,s0; addu s0,s0,a1` (staged copy through $s0) is produced BY BOTH chained-form (`p=arg0; base=p`) AND parallel-form (`p=arg0; base=arg0`) — GCC's copy-prop coalesces them

- [s2] [s2] the target prologue is NOT produced by declaration reorder that puts base first — H1a form emits `move s4,a0` directly (1 move, not staged) and flips save order

- [s2] [s2] H1d proves `arg0 = 0;` in the s1 candidate is inert — removing it leaves score=1 unchanged, so the committed candidate can be simplified to eliminate one of two cheats

- [s2] [s2] tried inner-scope base (H1c) as an alternative liveness lever; strictly worse (score 8, -2 insns) — narrower base lifetime is not the answer

- [s2] [s2] declaration type (s32 vs u32*) for `base` is codegen-neutral after copy-prop — H1b and H1d produce byte-identical .o

- [s2] [s2] the residual is a genuine RA tie-breaker: at insn #18 GCC picks $a0 (which still equals base after copy-prop) over $s4 (the callee-save copy). None of the 4 statement-level structural probes attempted budged this choice while preserving the target prologue.

- [s2] [s2] frontier: the split-init-accumulation form `base = arg0; base += shifted; base -= shifted;` proposed in s1 hypotheses is a cheat-by-any-spelling per [[no-new-park-categories]] — no-op arithmetic with the sole purpose of steering RA. NOT probed. Should be marked KILLED-BY-POLICY in future ledger updates.

- [s3] [s3] baseline sandbox with s2 candidate applied (H1d form, no arg0=0): score=1, 65/65 insns, single residual at insn #18

- [s3] [s3] `s32 buf[8]` local is load-bearing for FRAME reservation — removing it: score 1→15, frame 0x50→0x30, ~15 stack offsets shift; NOT decorative. Remains a forbidden un-written array unless legitimized under [[dead-vars-local-array]] 2026-07-01 carve-out (which requires actual STORES matching target dead stores; target's `sw v0,0x10(sp)` in the jal delay slot IS GCC-allocated on its own and does not by itself satisfy the written-not-read requirement).

- [s3] [s3] split-recompute form (`count = *(base+off); p = base+off+4`) does NOT lift base_addr's ref count as seen by local-alloc — CSE folds the two base+off computes into one before reg-alloc, so priority tiebreaker is unchanged; introduces new $a0-anchored diff.

- [s3] [s3] within-scope declaration-order and loop-body statement reorderings do not budge the insn #18 copy-prop tie (measured across 3 orderings, all score 1 or score 13).

- [s3] [s3] the residual is a robust GCC-internal tiebreaker between $a0 (arg-register-alias-via-copy-prop) and $s4 (callee-save-holding-base_addr) — five distinct structural probes have failed to invert it. This is exactly the shape the s2 frontier flagged as a permuter-directed problem rather than a manual-lever problem.

- [s3] [s3] rejected forms banked: rejected/p1_word_offset_split_recompute.c (score 3), rejected/p5_new_var_deferred_to_end.c (score 13).

- [s4] [s4] baseline sandbox (s3 candidate applied): score=1, 65/65 insns, single residual at insn #18

- [s4] [s4] permuter workspace built: pre-preprocessed base.c (491k), compile.sh mirrors sandbox pipeline (cpp -> cc1 -> prologue_fix -> maspsx -> multu_pad -> extract InitHiraRmd_80047FBC region -> as), target.o assembled from asm/funcs/InitHiraRmd_80047FBC.s at offset 0; baseline diff was the exact single residual (addu s0,a0,v0 vs addu s0,s4,v0), matching sandbox metric

- [s4] [s4] campaign s4_chassis1 (jobs=6, --stop-on-zero): base_score=5 (permuter metric = one reg-diff x 5); converged to score=0 at iter 217 / 9.6s elapsed; harvest+stop completed with reason logged. Metrics in metrics/events.jsonl.

- [s4] [s4] permuter closing form APPLIED to src/text1b.c: sandbox --disable all score=0 confirmed (65/65 insns, cheat_asm_stripped=392, rules_dropped=0). Then REVERTED to s3 candidate after cheat vetting; sandbox=1 restored.

- [s4] [s4] closing form's THREE constructs each fail the vetting checklist ([[no-new-park-categories]]): (1) buf[8] unused - not the WRITTEN carve-out (zero source stores; target's sw v0,0x10(sp) is GCC-allocated from discarded call return, not source-level); (2) `int new_var3 = 16;` used only in `arg1 << new_var3` - natural spelling is the immediate 16; no semantic purpose; potentially reviewable under [[named-local-fake-exception]] constant-holder carve-out but only individually with FAKE + exhaustion + review; (3) `new_var2 = sx_arg2;` mid-loop - same-value local alias of a live var, not covered by [[staged-value-reused-variable]] (which requires the target var to be currently-dead), not covered by [[duplicated-statement-into-arms]] (which requires cross-arm duplication).

- [s4] [s4] stacking three no-semantic-purpose codegen-steering constructs in a 65-insn function exceeds any single SOTN 2026-07-01 sanctioned-family precedent; vetting checklist answers all four smell-test questions in the cheat direction (no semantic purpose, no human would write it, justification references RA internals, 'necessary only because permuter said so')

- [s4] [s4] hypothesis killed: directed permuter's search space over the base_addr/p init chain + type variants + PERM_VAR/PERM_LINESWAP mutations. The mutations available to permuter (constant hoisting, value aliasing, declaration reorder, type substitution) all resolve this specific RA tiebreaker only through cheat family constructs. A permuter re-seed with different chassis is unlikely to yield a non-cheat closing form given the residual's shape (single-insn copy-prop vs callee-save tiebreaker).

- [s4] [s4] artifacts: campaign converged in 217 iters (~292s wall) with fresh-seed budget well under the 20-30 min per-basin cap; harvest recorded 1 find and stopped before session end (fresh-seed discipline satisfied)

- [s5] [s5] baseline sandbox with s3 candidate applied: score=1, 65/65 insns (unchanged from s3/s4 baseline)

- [s5] [s5] chassis C (drop base_addr; arg0 used directly in both `p = arg0+shift` init and `new_var = arg0+wshift` loop compute; buf[8] retained for frame): sandbox --disable all score=5, build_insns=64, target_insns=65

- [s5] [s5] chassis C disasm normalized to 65/65 lines. Confirmed structural diffs vs target: (1) target `sw s0,56(sp)` + `move s0,a0` (2 insns) missing in chassis C; (2) chassis C `move s4,a0` vs target `move s4,s0`; (3) chassis C `addu s0,s4,a1` vs target `addu s0,s0,a1`. Chassis C DOES have the target-matching `addu s0,s4,v0` at 0x120 (the s1-s4 residual insn), so the copy-prop tie IS resolved by dropping base_addr — but at the cost of collapsing the prologue staging

- [s5] [s5] permuter workspace tmp/grind/InitHiraRmd_80047FBC/s5/perm_ws/ (base.c 491k preprocessed from chassis C src, compile.sh mirrors sandbox pipeline, target.o copied from s4)

- [s5] [s5] campaign s5_chassis_c_arg0_direct (jobs=6, --stop-on-zero, --stack-diffs default): base_score=170 (permuter metric); 2747 iterations over ~15 min elapsed; 0 finds; harvest+stop completed with reason logged. Fresh-seed discipline satisfied (single basin, no novel find within cap → harvest)

- [s5] [s5] cluster-wide implication: the residual is not chassis-choice-solvable in either direction. The prologue-staging axis (satisfied by base_addr in a callee-save) and the copy-prop-tiebreaker axis (satisfied by dropping base_addr) are anti-correlated with the current cluster-mate chassis catalog. Any solution must satisfy BOTH — e.g. a 3-variable chain (`tmp = arg0; base = tmp; p = tmp + shift`) forced to survive CSE, OR a construct that raises arg0's reg_n_refs across the loop without eliminating base_addr. Both are outside the auto-permuter mutation space (which permutes existing shape rather than adding a variable) and outside the s1-s4 lever set already killed

- [s6] sandbox --disable all baseline this turn: score=1, 65/65 insns, cheat_asm_stripped=393 (unchanged from s3-s5 baseline; confirms current committed src still hits the same single residual at insn #18).

- [s6] s6 dump inventory: 14 per-pass RTL dumps + greg + allocdbg for s3-candidate form (text1b.i.{rtl,cse,loop,cse2,combine,flow,jump,jump2,lreg,greg,sched,sched2,dbr}); parallel 14-pass dump tree under probe_arg0zero/ with arg0=0 injected; func.greg (363 lines isolated section) + allocdbg.stderr (1034 ALLOCDBG lines) + insn36_evolution.txt.

- [s6] Per insn36_evolution.txt: insn 36 stays `(set (reg/v:SI 78) (plus:SI (reg/v:SI 79) (reg:SI 86)))` through rtl/cse/loop; at cse2 the RHS becomes `(plus:SI (reg/v:SI 72) (reg:SI 86))` — the reg-79→reg-72 rewrite; combine/lreg preserve it; greg maps reg 78→$s0, reg 72→$a0, reg 2→$v0 yielding `addu $s0, $a0, $v0` (sandbox residual).

- [s6] probe_arg0zero raw cc1 output (text1b.s:141) and post-maspsx final.s:134 both emit `addu $16,$20,$2` = `addu $s0, $s4, $v0` — target-matching bytes.

- [s6] engine/volatile_cheats.py:791 find_dead_param_assigns strips un-annotated `arg0 = 0;` before scoring; line 815 `_stmt_fake_annotated` bypasses the strip when the statement carries a `/* FAKE */` marker (verified paths cited in FINDINGS.md).

- [s6] Judge ruling 2026-07-20 03:22 in docs/grind/decisions.md line 981 (commit 94ba752b) — PASS on isolated arg0=0 FAKE lever qualification under dead-store-fake-exception; explicitly scoped, NOT a final commit gate; buf[8] independently unresolved.

- [s6] Cluster impact: sibling functions AddTbpOfst_80047EE8, InitHiraRmd_800480C0, func_800481E8 share the same shape (parallel derivations of p and base from arg0) and would benefit from the same mechanism if closed.

- [s7] [s7] baseline sandbox --disable all this session: score=1, 65/65 insns, rules_dropped=0 (unchanged from s3-s6 baseline; current committed src carries `s32 buf[8]; (void)buf;` + `arg0 = 0;` (un-annotated); measured via existing s6 dumps + engine gradient consistency).

- [s7] [s7] cc1 .frame comment on current committed src: `.frame $sp,80,$31 # vars= 32, regs= 6/0, args= 24, extra= 0` for InitHiraRmd_80047FBC. Frame equation: ALIGN8(vars=32)+ALIGN8(args=20→24)+ALIGN8(gp_regs=24)=80=0x50 ✓ matches target.

- [s7] [s7] Target's ONLY sp-relative stores: sw v0 at 0x10 (jal delay slot) + sw s0/s1/s2/s3/s4/ra at 0x38/0x3C/0x40/0x44/0x48/0x4C. Zero stores in the sp+0x18..0x37 (32-byte) locals region. Verified by grep of asm/funcs/InitHiraRmd_80047FBC.s.

- [s7] [s7] args=24 = ALIGN8(5*4) proves the outgoing-arg area is sp+0..sp+0x17 (24 bytes for a 5-arg call). sw v0,0x10(sp) in the delay slot IS the 5th outgoing arg — NOT a dead store into locals. This corrects the s1 evidence claim (recorded in ledger evidence.md as the primary motivator for the buf[k]=call() capture avenue).

- [s7] [s7] efc_buki_draw_zanzou is declared `void efc_buki_draw_zanzou(s32,s32,s32,s32)` at include/m2c_context.h:647 — 4-arg void. The candidate calls it with 5 s32 args; GCC 2.7.2 with -w accepts the arity mismatch and places arg #5 at the o32 5th-arg slot sp+0x10.

- [s7] [s7] vars= gradient across 8 variants (baseline + v1/v3/v4/v6/v8/v9/v11): only buf[8]/buf[8]+read (baseline/v9) and address-of'd scalar arrays (v11) reach vars=32 with sp-stores=7. All are structurally the same cheat (frame-reservation via a declaration with no semantic purpose).

- [s7] [s7] v3_bufwritten (buf[0]=arg1) achieves vars=32 but adds sp-stores=8 (new sw at sp+0x18 for the buf[0] write) — verified by count in variant .s. Target has 7 sp-stores — the extra write is a byte diff. WRITTEN carve-out approach (Judge constraint b option i) is provably byte-changing.

- [s7] [s7] Live-locals shapes that GCC WON'T reg-alloc away (v6 8 s32 cascade, v8 4 u64 cross-mul, v4 8 s16 bitwise ladder — the phantom-frame-slots-gcc272 documented trigger) all yielded vars=0 in this function's shape — the trigger does not fire when the s16 values feed HImode→SImode widening for call args (as this function does).

- [s7] [s7] Named GCC pass producing the s1-s6 residual (re-confirmed): cse2 canonical-reg substitution at insn 36 folds reg 79 (base_addr, → $s4) into reg 72 (arg0, → $a0), yielding sandbox `addu $s0,$a0,$v0` vs target `addu $s0,$s4,$v0`. See tmp/grind/InitHiraRmd_80047FBC/s6/insn36_evolution.txt.

- [s7] [s7] No OWNER-ESCALATION entry for InitHiraRmd_80047FBC exists in docs/grind/decisions.md — only the 2026-07-20 03:22 Judge PASS for the isolated arg0=0 FAKE lever (line 981). owner-gated outcome invalid this session.

- [s7] [s7] artifacts saved: tmp/grind/InitHiraRmd_80047FBC/s7/{baseline,v1_nobuf,v3_bufwritten,v4_himode_bitwise,v6_cascade_s32,v8_u64_locals,v9_bufread,v11_addrof_8scalars}.{c,i,s}; FINDINGS.md; gen_variants.py; gen_variants2.py; probe_frame.sh; probe_all.sh; probe_all2.sh. Rejected forms banked to memory/grind/InitHiraRmd_80047FBC/rejected/{s7_bufwritten_written_carveout_fails.c, s7_addrof_8scalars_no_purpose.c}.

- [s8] [s8] baseline sandbox --disable all: score=1, 65/65 insns (unchanged from s3-s7 baseline; current committed src carries `s32 buf[8]; (void)buf;` + `arg0 = 0;` un-annotated)

- [s8] [s8] Kengo/disc/SLUS_200.21 is ELF32 MIPS-III (PS2); disassembled InitHiraRmd @ 0x1077f8 (276 bytes/69 insns), HiraRmdAddTbpOfst @ 0x1073d8, PutHiraRmd @ 0x10adf8; ALL structurally unrelated to BB2's cluster (different signatures, different callees, different control flow)

- [s8] [s8] Kengo InitHiraRmd is a 3-arg struct-header initializer (packet setup, calls GetAllocPacketSize + InitPartsVertColData); BB2's InitHiraRmd_80047FBC is a 4-arg table walker calling efc_buki_draw_zanzou (weapon afterimage). The name is a splat auto-name coincidence, not a Kengo rename — cluster is likely something like DrawWeaponTrailFromTable per callee semantics

- [s8] [s8] Fresh m2c decompile (tools/m2c/m2c.py -t mips-ido-c) produces chassis-C-equivalent shape (no base_addr, arg0 used directly); identical to s5's rejected/s5_chassis_c_arg0_direct_no_base.c which was KILLED at score=5 (loses staged prologue). m2c re-confirms the s5-documented anti-correlation between copy-prop tiebreaker and prologue staging

- [s8] [s8] s16*-walker variant (`s16 *hp = (s16*)((s32)p+4); a1v=*hp++;...`) applied to src: score=31 (30 new diffs); GCC assigns hp to a different register than $s0 and the whole hword-load block cadence diverges. Confirms u32*+byte-cast walker is load-bearing for target register cadence

- [s8] [s8] const-qualified base initializer (`const u32 *const base = (const u32*)arg0;`) applied to src: score=1 byte-identical; GCC 2.7.2's cse2 discards const at RTL level, {reg 72, reg 78, reg 79} equivalence class forms identically

- [s8] [s8] rederive-modality avenues catalog now complete: Kengo transplant KILLED (false cognate), m2c fresh KILLED (reproduces s5 chassis C), decomp.me corpus not applicable (no PSX BB2 project), structural variants KILLED across 16+ probes s2-s8. All four canonical rederive lanes exhausted for this function

- [s8] [s8] no OWNER-ESCALATION entry for InitHiraRmd_80047FBC exists in docs/grind/decisions.md — only the 2026-07-20 03:22 Judge PASS at line 981 (scoped to isolated arg0=0 FAKE lever qualification, explicitly NOT a final-commit gate). owner-gated outcome invalid this session

- [s8] [s8] artifacts saved: tmp/grind/InitHiraRmd_80047FBC/s8/{FINDINGS.md, v1_s16_walker.c}; rejected forms banked: memory/grind/InitHiraRmd_80047FBC/rejected/{s8_s16_hword_walker.c, s8_const_qualified_base.c}

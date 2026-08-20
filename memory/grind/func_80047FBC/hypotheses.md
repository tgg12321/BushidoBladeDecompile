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

## [s7] The target's `sw $v0, 0x10($sp)` in the jal delay slot (target insn #55) is a discarded-return dead store that requires an explicit `buf[k] = efc_buki_draw_zanzou(...)` capture to legitimize buf[8] under the [[dead-vars-local-array]] WRITTEN carve-out (per Judge constraint b, option i, and s1 evidence).
- mechanism: s1 assumed the store came from the discarded call return being spilled into a local frame slot. Forensic check: cc1 reports args=24 (ALIGN8(5*4)) for the current candidate, indicating the outgoing-arg region is sp+0..sp+0x17 (24 bytes = 5-arg call). Offset 0x10 lies INSIDE that outgoing-arg region — it is the outgoing 5th arg slot, not a local. The candidate calls efc_buki_draw_zanzou with 5 args; GCC places arg #5 at sp+0x10 in the delay slot. Header (include/m2c_context.h:647) declares it as 4-arg `void efc_buki_draw_zanzou(s32,s32,s32,s32)` but GCC 2.7.2 with -w accepts the 5-arg call and places arg #5 at sp+0x10.
- probe: Grep asm/funcs/InitHiraRmd_80047FBC.s for all sp-relative stores. Result: 7 sw's — 0x10 (arg #5) + 0x38/0x3C/0x40/0x44/0x48/0x4C (6 callee-saves). Zero stores in the sp+0x18..sp+0x37 locals region (32 bytes = vars=32). Also: efc_buki_draw_zanzou is declared `void`, so `buf[k] = efc_buki_draw_zanzou(...)` is a type error — the s1 frontier probe's proposal is unbuildable as written.
- result: Target has ZERO sw's into the vars=32 locals region (sp+0x18..0x37). The WRITTEN carve-out requires target bytes to contain the dead stores — none exist here. Any explicit source-level `buf[k] = X;` would introduce a NEW sw at sp+0x18+ that target does not have, causing a byte diff. Verified by v3_bufwritten variant: `buf[0] = arg1;` yields vars=32 correctly but sp-stores=8 (adds sw v0,24(sp)) vs target's 7.
- verdict: KILLED

## [s7] A live-locals C shape (per [[phantom-frame-slots-gcc272]] documented trigger — HImode locals feeding HImode bitwise expressions) can produce vars=32 without an unused array, satisfying Judge constraint b option ii (structural change preserving 0x50 frame without an unused array).
- mechanism: GCC 2.7.2's get_frame_size() reserves bytes for locals it later register-allocates away. If we write live-purpose C that yields vars=32, no unused-array cheat is required. Documented trigger: two HImode (s16) locals feeding HImode bitwise expression `(a & ~b) & 1`.
- probe: Compiled 6 candidate live-locals shapes with instrumented cc1 (tmp/gccdbg/cc1) and read the `.frame` comment: v4_himode_bitwise (8 s16 + bitwise ladder), v6_cascade_s32 (8 s32 real cascade), v8_u64_locals (4 u64), v9_bufread (buf[8] + dead read), v11_addrof_8scalars (8 addr-of'd s32). All variants preprocessed via project cpp flags then compiled with -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin.
- result: vars= gradient: v4=0, v6=0, v8=0, v9=32, v11=32. v6/v8 (real live locals): GCC register-allocates all 8 away, vars=0. v4 (HImode bitwise trigger): does NOT fire in this function's shape (the s16 locals participate in call-arg widening which erases the trigger). v9 (buf[8] + dead read): vars=32 but the read is DCE'd, structurally the same cheat as baseline. v11 (8 addr-of'd scalars): vars=32 with sp-stores=7 matching target, but 8 unused named scalars with 8 unused pointer aliases have no semantic purpose — fails cheat-by-any-spelling test (naming pattern signals coercion intent).
- verdict: KILLED

## [s7] The isolated arg0=0 /* FAKE */ lever (Judge-PASS 2026-07-20) applied to src/text1b.c will produce sandbox --disable all score=0 (per frontier probe 1).
- mechanism: s6 raw-pipeline probe already showed target-matching bytes from cc1+maspsx with the FAKE-annotated form; engine/volatile_cheats.py:815 _stmt_fake_annotated bypasses the stripper.
- probe: Not executed this session (mandate: forensics, not structural). Session focused on the buf[8] resolution question (Judge constraint b) which is the composite gate.
- result: Deferred to a structural-modality session; forensics established that Judge constraint b cannot be satisfied by either sanctioned path, so applying arg0=0 alone is insufficient for candidate-ready.
- verdict: CONFIRMED

## [s8] Kengo (PS2 successor) InitHiraRmd / HiraRmdAddTbpOfst / PutHiraRmd are source-level relatives of BB2's cluster and their source-shape can be transplanted to close BB2's cluster.
- mechanism: Kengo's Marionation engine reuses SquareSoft Lightweight code; the cluster's shared name prefix suggests a common ancestor whose C shape reproduces target codegen when compiled through our fork.
- probe: Disassembled Kengo InitHiraRmd (0x1077f8, 276 bytes, 69 insns) + HiraRmdAddTbpOfst (0x1073d8) + PutHiraRmd (0x10adf8) via mips-linux-gnu-objdump. Compared signatures, callees, and control-flow shapes.
- result: Kengo InitHiraRmd is a 3-arg struct-header initializer that populates packet fields via sll-4 (16-byte-aligned VU addresses), calls GetAllocPacketSize and InitPartsVertColData. BB2's InitHiraRmd_80047FBC is a 4-arg (s32,s32,s16,s16) weapon-afterimage table walker calling efc_buki_draw_zanzou (5-arg). NO callee-signature / control-flow / sub-region overlap. The name is a splat auto-name collision, not a rename.
- verdict: KILLED

## [s8] A fresh m2c decompile of the target asm surfaces a structurally-different C shape that our fork's cc1 can compile to target bytes.
- mechanism: m2c reconstructs the original C-level structure from the asm; if the s1-s7 candidate is a poor structural reconstruction, m2c's output may reveal a cleaner shape.
- probe: python3 tools/m2c/m2c.py -t mips-ido-c --context include/m2c_context.h -f InitHiraRmd_80047FBC asm/funcs/InitHiraRmd_80047FBC.s.
- result: m2c emits chassis-C-equivalent shape (no base_addr local; arg0 used directly as `arg0 + ((word>>2)*4)` in both pre-loop and call-arg positions). Structurally identical to s5 rejected/s5_chassis_c_arg0_direct_no_base.c which was measured score=5 and KILLED (loses target's staged prologue). m2c re-derives the same anti-correlated pair the s5 finding documented; no new shape.
- verdict: KILLED

## [s8] Restructuring the inner hword-load block to walk via `s16 *hp` post-increment (m2c-suggested for the hword reads) instead of `u32 *p` with byte-cast increments produces a shape closer to target.
- mechanism: Native-type walker for the hword reads might let GCC choose a register cadence that matches target's $s0-anchored increments.
- probe: Applied `s16 *hp = (s16*)((s32)p+4); a1v=*hp++; ...; p=(u32*)hp;` to src/text1b.c (s8/v1_s16_walker.c). Sandbox --disable all.
- result: score=31 (30 new diffs). GCC allocates hp to a different register than $s0, so the entire load-address emission cadence diverges from target across the hword block. The u32*+byte-cast walker is load-bearing for target's $s0 cadence.
- verdict: KILLED

## [s8] Declaring base with const qualifiers (`const u32 *const base = (const u32*)arg0`) steers cse2's canon_reg substitution to keep base separate from arg0's equivalence class.
- mechanism: const-qualified copies might not participate in cse2's canonicalization if the pass treats them as distinct value expressions.
- probe: Applied `const u32 *const base = (const u32*)arg0;` to src/text1b.c. Sandbox --disable all.
- result: score=1, byte-identical to baseline. GCC 2.7.2 discards const at the RTL level; cse2 forms the same {reg 72, reg 78, reg 79} equivalence class and folds insn 36 identically. const is inert as an RA/CSE lever in this compiler.
- verdict: KILLED

## [s9] Declaring `base` BEFORE `p` (pure declaration-order swap, unchanged init chain `p=arg0; base=p;`) shifts cse2's canon_reg equivalence-class root selection so insn #36's canonicalization substitutes reg 72 -> reg 79 ($s4) instead of the current reg 79 -> reg 72 ($a0), closing insn #18 residual.
- mechanism: cse2's canon_reg picks a canonical representative from each equivalence class; earlier-declared pseudos may have lower LUIDs and become the root.
- probe: Applied decl-order swap (base declared before p, init lines unchanged) to src/text1b.c; ran sandbox --disable all.
- result: score=1 unchanged (65/65 insns, single residual still `addu $s0,$a0,$v0` at insn #18). GCC 2.7.2 assigns pseudo-regnos by first-USE LUID not declaration LUID; `p = (u32*)arg0` uses p first so reg 78 -> p and reg 79 -> base regardless of decl order. cse2 class root selection unchanged.
- verdict: KILLED

## [s9] The cluster (AddTbpOfst_80047EE8 + InitHiraRmd_80047FBC + InitHiraRmd_800480C0) shares a single source-level idiom (macro / inline helper / struct-typed arg) that produces both the staged prologue AND cse2-fold-invariance uniformly across all members.
- mechanism: Cluster peers show parallel target shapes (staged move s0,a0; move sN,s0 prologue + vars=32 frame + 5-arg efc_buki_draw_zanzou call) suggesting a shared original source construct.
- probe: Read all three peers' asm/funcs/*.s. Tabulated callee-save mapping: 80047EE8 base=$s2 (3 gp saves), 80047FBC base=$s4 (5 gp saves), 800480C0 base=$s2 (7 gp saves). Diffed prologue idioms.
- result: The staged double-copy `move s0,a0; move sN,s0` idiom IS shared, and the current src ALREADY expresses it via `p=arg0; base=p;`. BUT base's callee-save mapping varies with function-specific pressure ($s2 in 80047EE8/800480C0, $s4 in 80047FBC because sx_arg2/sx_arg3 crowd earlier callee-saves). Sibling 800480C0 uses `register asm("$18") saved_arg0;` pin to force the mapping — forbidden per inline-asm-policy expanded catalog. No shared pure-C idiom produces both the staging AND correct cse2 canonicalization across all cluster members without an explicit register pin.
- verdict: KILLED

## [s10] The composite closing candidate (arg0=0 FAKE + s32 buf[8] unwritten) can reach COMPLETED-C via a sanctioned pure-C lever within the current SOTN-family carve-outs.
- mechanism: arg0=0 FAKE lever is Judge-PASSed for isolated qualification (2026-07-20 03:22, decisions.md line 981) and s6 forensics named cse2 canon_reg substitution at insn 36 as the compile-time defeat mechanism; buf[8] is the load-bearing 0x50-frame carrier per s3 measurement (removing it: score 1->15).
- probe: Cross-referenced the 2026-07-01 SOTN family-research catalog ([[dead-vars-local-array]] carve-out prereqs, [[dead-store-fake-exception]] archetype list, [[named-local-fake-exception]], [[pointer-alias-fake-exception]]) against s7 evidence (target has ZERO sp-relative stores in sp+0x18..sp+0x37 locals region, sw v0,0x10(sp) lies in args=24 outgoing-arg region for the K&R 5-arg efc_buki_draw_zanzou call, not in locals; efc is void-typed at include/m2c_context.h:647 so buf[k]=efc(...) is unbuildable; buf[k]=arg1 adds a byte-diff sw at sp+0x18).
- result: No sanctioned lever exists. WRITTEN carve-out requires target to CONTAIN corresponding dead stores — this target does not. All 6 phantom-frame variants (v4/v6/v8/v9/v11 + baseline) reg-alloc away in this function's shape, per s7. The composite candidate cannot be legitimized without either (a) a new SOTN-precedent-backed exception carving out unwritten-array-as-frame-carrier — no such precedent found — or (b) canonical-asm authorization — refused by [[endgame-lock-disposition]] absent STRONG scan_hand_coded signals which are absent per s9.
- verdict: KILLED

## [s10] InitHiraRmd_80047FBC meets the [[endgame-lock-disposition]] species criteria for OWNER-ESCALATION filing.
- mechanism: Species = (1) byte-matches on main ONLY via cheat; (2) small number of insns short in honest pure-C; (3) sanctioned pure-C levers exhausted with mechanism named at GCC-pass level.
- probe: Confirmed against s1-s9 ledger: (1) current committed src carries `s32 buf[8]; (void)buf;` + un-annotated `arg0 = 0;` both stripped by sandbox; oracle green only via these; (2) exactly 1 residual insn (single-diff at #18); (3) 18 measured hypotheses across 5 modalities (recon/structural/permuter/forensics/rederive) + 13 banked rejected forms + cse2 canon_reg + get_frame_size named at pass level. Owner AND-gates: (a) scan_hand_coded LOW (no S1 multu pacing / S2 empty branch / S6 BIOS jumptable — ordinary GCC-scheduled table walker); (b) no SOTN precedent for unwritten local-array frame carrier (2026-07-01 dead-vars-local-array requires WRITTEN and matching target stores, both absent).
- result: All three species criteria satisfied. Both AND-gates fail (asm refused for lack of hand-coded signals; coercion family refused for lack of SOTN precedent). Precedent for owner disposition: last four escalations of same species (motion_SetMotion 2026-07-18, func_80057CC8 / saTan0Init / cpu_side_move_dir_4 2026-07-19-20, func_80049A2C 2026-07-20) all ruled option (b) INCOMPLETE-owner-accepted.
- verdict: CONFIRMED

## [s10] Rederive modality is provably exhausted across all 4 canonical lanes for InitHiraRmd_80047FBC; further rederive sessions produce nothing new.
- mechanism: The catalog is (a) sibling cluster transplant / cluster-shared-idiom, (b) upstream successor (Kengo PS2) transplant, (c) fresh m2c decompile, (d) declaration-order / cse2-canon-root reshuffling.
- probe: s8 Kengo InitHiraRmd disassembled at 0x1077f8 — false cognate (3-arg struct-header initializer, no callee/CF overlap). s8 fresh m2c produces chassis C (score=5, loses staged prologue). s9 decl-order swap (base first) — KILLED, GCC assigns pseudo-regnos by first-USE LUID not decl LUID. s9 cluster-shared-idiom conjecture — KILLED, cluster peers rely on forbidden register-asm pin.
- result: All 4 lanes measured KILLED. No lane has a remaining unmeasured probe.
- verdict: CONFIRMED

## [s11] Applying rejected/s6_arg0zero_faked.c verbatim to src/text1b.c reaches sandbox --disable all score=0 via the FAKE-bypass path (engine/volatile_cheats.py:815 _stmt_fake_annotated), confirming the s6 forensics RTL derivation end-to-end through the sandbox integration and discharging Judge constraint (a).
- mechanism: The s6 form declares base_addr as s32 (not u32* base), uses base_addr in the two later addu sites, and marks arg0=0 with a /* FAKE */ annotation. The volatile_cheats detector's _stmt_fake_annotated bypass (line 815) recognises the annotation and skips find_dead_param_assigns's stripper, letting cc1 see the assignment. cc1's cse2 canonical-register substitution then does NOT fold {reg 72 arg0, reg 78 p, reg 79 base_addr} at insn 36 (because arg0 is live-across-the-store), so greg emits insn #18 as (reg 16 s0) = (plus (reg 20 s4) (reg 2 v0)) matching target's `addu $s0, $s4, $v0` exactly. Prior sessions s1-s10 measured 1 because the un-annotated form was stripped before cc1 saw it — the CONFIRMED probe closes the audit trail from RTL claim to sandbox integration.
- probe: Overwrote src/text1b.c InitHiraRmd_80047FBC body with rejected/s6_arg0zero_faked.c (edits: replace `u32 *base` decl with `s32 base_addr`, replace `base = p` with `base_addr = arg0`, add /* FAKE */ annotation to `arg0 = 0`, rewrite the two `(s32)base` sites to `base_addr`). Ran `& tools/wteng.ps1 main sandbox InitHiraRmd_80047FBC --disable all`. Log: tmp/grind/InitHiraRmd_80047FBC/s11/sandbox_s6form.log. Snapshot of applied form: tmp/grind/InitHiraRmd_80047FBC/s11/text1b_s6applied.c. Composite banked to memory/grind/InitHiraRmd_80047FBC/composite_candidate.c. src/text1b.c reverted (git diff --stat src/text1b.c == empty).
- result: Sandbox returned {"score": 0, "target_insns": 65, "build_insns": 65, "scorable": true, "cheat_asm_stripped": 392}. Score dropped from ledger-baseline 1 to 0 with the FAKE annotation active. Confirms the s6 RTL derivation and closes the sandbox-integration audit gap flagged by the 2026-07-20 03:22 Judge PASS (`Ruling scope: legitimacy of a SINGLE arg0 = 0 /* FAKE */ ... — not a final commit gate`).
- verdict: CONFIRMED

## [s12] An OWNER-ESCALATION entry for InitHiraRmd_80047FBC can be filed under the 2026-07-20 endgame-lock-disposition policy with the full s1-s11 dossier and the two mutually-exclusive owner options (sanction unwritten-buf[8] family / refuse and accept INCOMPLETE-owner-accepted).
- mechanism: Species criteria per .claude/rules/endgame-lock-disposition.md: (1) main byte-matches only via composite cheat (buf[8]+arg0=0), (2) sandbox honest floor=1 in pure C with all sanctioned levers dead across 5 modalities x 11 sessions, (3) both AND-gates fail: (a) scan_hand_coded NEGATIVE (no STRONG signals; ordinary table walker), (b) no SOTN precedent for unwritten 32-byte local as phantom frame carrier. Judge PASS at line 981 discharges the isolated arg0=0 FAKE lever half; the buf[8] half is provably unresolvable within sanctioned families per s7 grep (zero target stores in sp+0x18..sp+0x37) and the 2026-07-20 00:36 OVERSIZED-LOCALS refusal.
- probe: Cite s1-s11 evidence bank verbatim in the escalation entry; enumerate all 13 banked rejected forms; enumerate all 4 rederive lanes exhausted (Kengo transplant / m2c fresh / decl-order LUID / cluster-shared idiom); enumerate all 5 modalities exercised end-to-end; state the two owner options honestly with no self-resolution; append to docs/grind/decisions.md at the end.
- result: Entry filed at docs/grind/decisions.md:985 following the func_80049A2C 2026-07-20 / motion_SetMotion 2026-07-18 / saTan0Init 2026-07-19 template. Grep confirms exactly one OWNER-ESCALATION entry for InitHiraRmd_80047FBC now exists.
- verdict: CONFIRMED

## [s13] The 2026-08-18 general "phantom-frame-slot volatile pad local" family covers this function's 32-byte unwritten frame carrier, converting the 2026-07-22 endgame-lock refusal into a bytes-proven candidate.
- mechanism: The 2026-07-22 owner ruling refused func_80047FBC (then InitHiraRmd_80047FBC) because AND-gate (b) failed - no SOTN-master precedent for an UNWRITTEN local acting as a frame carrier. The 2026-08-17 func_8001E404 re-scope, extended 2026-08-18 into a general family, found the opposite in SOTN master: `volatile u32 pad[4]; // FAKE` (src/st/sel/stream.c:80) and `volatile u32 pad; // !FAKE:` (src/st/sel/2C048.c:564) are declared first, never written, never read, in fully matched PSX code. The family's constraints are array form, first-decl position, `// !FAKE` annotation, an engine allowlist row, plus frame forensics proving the slot is untouched in target and honest producers measured inert first. s7 already banked the frame forensics for this function (zero sw in sp+0x18..sp+0x37) and s1-s12 banked the exhaustion.
- probe: Applied an unwritten `volatile u32` pad array of 8 words as the first declaration of the s11 composite body (replacing `s32 buf[8];` + `(void)buf;`), function renamed func_80047FBC, into src/text1b.c in place of the INCLUDE_ASM line. Ran `sandbox func_80047FBC --disable all` and `--disable none`. Then ran a three-point attribution ladder (clean / FAKE-store only / pad only).
- result: pad + FAKE store = score 0 (65/65, 0 rules dropped); pad only = 1; FAKE store only = 14; clean = 15. The pad closes 14 of the 15 residual instructions; the s6 FAKE dead-param-assign closes the last one. Logs in tmp/grind/func_80047FBC/s13/.
- verdict: CONFIRMED

## [s13] The candidate cannot be returned as candidate-ready because the sanctioned pad is stripped by the sandbox until engine/volatile_cheats.py carries a per-function allowlist row - a surface a grind session may not touch.
- mechanism: `engine/volatile_cheats.py::_is_sanctioned_pad` only exempts (function, name, element-count) triples listed in `_SANCTIONED_UNWRITTEN_PADS`, which today holds func_8001E404, func_8001E6E4 and func_8003CF84. Everything else is stripped by `find_unused_local_arrays` before scoring, and cheat-asm stripping is unconditional in the sandbox (the `--disable` flag governs regfix/asmfix rules only). The 2026-08-18 ruling itself mandates the row ("the sanctioned pads are allowlisted in the engine's volatile-cheat detector so honest floors read true"), so adding it is a prescribed integration step, not a workaround.
- probe: Re-spelled the pad as `pre_pad` with an annotation that does not repeat the identifier token (so the detector genuinely sees it) and re-measured: `--disable all` = 14 and `--disable none` = 14, identical, confirming the strip is unconditional. Also verified the earlier `pad`-named spelling returned `func_volatile_cheat_count == 0` - an accidental under-detection caused by the identifier appearing inside its own comment.
- result: Bytes proven at 0 with the pad honoured; honest floor reads 14 with it stripped. Filed as an INTEGRATION HANDOFF entry in docs/grind/decisions.md (2026-08-20) with the exact operator steps, and returned owner-gated.
- verdict: CONFIRMED
## [s14] The s13 bytes-proven result reproduces on the 2026-08-20 dispatch chassis, and the ONLY remaining blocker is an owner-class extension of engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS.
- mechanism: The sandbox strips cheat-asm unconditionally (`--disable` governs regfix/asmfix rules only), and `_is_sanctioned_pad` exempts an unwritten volatile pad ONLY for (function, name, element-count) triples listed in `_SANCTIONED_UNWRITTEN_PADS`. Without a row for func_80047FBC, `find_unused_local_arrays` deletes the `volatile u32 pre_pad[8];` declaration before cc1 sees it, the frame drops 0x50 -> 0x48 and 14 instructions diverge. With the pad honoured, GCC 2.7.2's get_frame_size reserves the 32 locals bytes at sp+0x18..sp+0x37 that target reserves and never touches, and the body matches at 65/65.
- probe: Re-applied both s13 bodies to src/text1b.c (bodyE = `pre_pad`, detector-visible; bodyB = `pad`, accidentally detector-invisible, identical C otherwise) and ran `sandbox func_80047FBC --disable all` on each; diffed the two bodies to confirm the only difference is the declaration's identifier and comment; re-ran `scan_hand_coded --single`; re-verified the three precedent file:line citations resolve.
- result: bodyE = 14 (cheat_asm_stripped 280), bodyB = 0 with 65/65 target/build insns and 0 rules dropped (cheat_asm_stripped 279). diff = 1 line. scan_hand_coded = LOW 1/8. All three citations resolve at the cited lines. Filed docs/grind/decisions.md:7530 as `**OWNER-ESCALATION**` (superseding s13's mis-titled INTEGRATION HANDOFF span) and returned owner-gated citing it.
- verdict: CONFIRMED

## [s13] The s13 bytes-proven form reproduces on the 2026-08-20 dispatch chassis: the candidate C compiles to 65/65 instructions identical to asm/funcs/func_80047FBC.s when its sanctioned volatile pad is honoured rather than stripped.
- mechanism: The sandbox strips cheat-asm unconditionally (--disable governs regfix/asmfix rules only) and engine/volatile_cheats.py::_is_sanctioned_pad exempts an unwritten volatile pad only for (function, name, element-count) triples in _SANCTIONED_UNWRITTEN_PADS (today: func_8001E404, func_8001E6E4, func_8003CF84). With no row for func_80047FBC, find_unused_local_arrays deletes `volatile u32 pre_pad[8];` before cc1 sees it, the frame drops 0x50 -> 0x48 and 14 insns diverge. With the pad honoured, GCC 2.7.2 get_frame_size reserves the 32 locals bytes at sp+0x18..sp+0x37 that target reserves and never touches, and the last insn (#18 addu $s0,$s4,$v0) is closed by the already-Judge-PASSed `arg0 = 0; /* FAKE */` cse2 lever.
- probe: Applied both s13 bodies to src/text1b.c line 20 in place of the INCLUDE_ASM line and ran `sandbox func_80047FBC --disable all` on each: bodyE (`pre_pad`, detector-visible) and bodyB (`pad`, accidentally detector-invisible via the comment-token bug, identical C otherwise). Diffed the two bodies. Re-ran scan_hand_coded --single. Re-verified the three precedent file:line citations resolve.
- result: bodyE = score 14, target_insns 65, build_insns 65, rules_dropped 0, cheat_asm_stripped 280 (tmp/grind/func_80047FBC/s14/sandbox_bodyE.log). bodyB = score 0, 65/65, rules_dropped 0, cheat_asm_stripped 279 (tmp/grind/func_80047FBC/s14/sandbox_bodyB.log). diff of the two bodies is exactly ONE line, the declaration, differing only in identifier and comment text - both codegen-neutral - so the 0 is honest evidence of byte-identity for the candidate and the 280-vs-279 strip delta is exactly the deleted declaration.
- verdict: CONFIRMED

## [s13] Endgame-lock AND-gate (a) (canonical-asm) still FAILS and gate (b) (in-hand SOTN-master precedent for the closing construct) now PASSES, so the correct disposition is an OWNER-ESCALATION with a passing gate, not the 2026-07-27 standing-ruling refusal.
- mechanism: Gate (a) needs STRONG scan_hand_coded signals (S1/S2/S6). Gate (b) needs an actually-exhibited SOTN-master precedent for the closing construct; the 2026-08-18 owner ruling turned the per-function pad carve-outs into a general family resting on two matched-PSX SOTN exhibits, which is exactly the shape this function's 32-byte untouched frame slot needs.
- probe: `python3 tools/scan_hand_coded.py --single func_80047FBC`; then read .claude/rules/no-new-park-categories.md:385-402, docs/reference/sotn-construct-index.md:99-105 and engine/volatile_cheats.py:740-755 to confirm every citation resolves and to check the family's form constraints and prerequisites against the candidate.
- result: Gate (a): tier=LOW score=1/8, only S4 (4 loads in an 8-insn window @ insn 25) - FAILS. Gate (b): .claude/rules/no-new-park-categories.md:390 carries the general-family ruling verbatim ('Supersedes the per-function 2026-08-17/18 leading/trailing-pad carve-outs with a general family'); docs/reference/sotn-construct-index.md:101 = src/st/sel/2C048.c:564 `volatile u32 pad; // !FAKE:`; :103 = src/st/sel/stream.c:80 `volatile u32 pad[4]; // FAKE`; both untagged (PSX) - PASSES. Candidate satisfies all four form constraints (array form, first-decl, // !FAKE annotation, no (void) shim) and both prerequisites (s7 frame forensics; honest producers measured inert across 13 sessions / 5 modalities / 14 rejected forms / ~3k permuter iters).
- verdict: CONFIRMED

## [s13] The remaining blocker is owner-class by the engine's own text, not merely out-of-session-surface, so it cannot be self-served by the driver, a Judge, or an integration-handoff remedy.
- mechanism: engine/volatile_cheats.py:740-753 scopes _SANCTIONED_UNWRITTEN_PADS to 'EXACTLY these (function, name, count) triples' and states 'Any extension requires a fresh owner ruling'. The 2026-08-18 general-family ruling simultaneously mandates that a sanctioned pad carry such a row so honest floors read true. Adding `"func_80047FBC": frozenset({("pre_pad", 8)})` is therefore the prescribed integration step AND an owner decision.
- probe: Read the allowlist and its docstring; confirmed the three existing rows and the extension clause; confirmed no row exists for func_80047FBC; confirmed src/text1b.c ships INCLUDE_ASM with zero regfix/asmfix rules and zero cheat-asm, so nothing on main holds a byte-match that could be retired instead.
- result: Filed `## 2026-08-20 - func_80047FBC (src/text1b.c) - **OWNER-ESCALATION** ...` at docs/grind/decisions.md:7530, superseding s13's mis-titled INTEGRATION HANDOFF span, stating both gates' evidence, the exact allowlist row requested, the operator steps, and the fallback disposition if the owner declines (revert to the 2026-07-22 REFUSED / OWNER-ACCEPTED INCOMPLETE ruling, honest floor 14).
- verdict: CONFIRMED

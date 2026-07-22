# Evidence bank — AddTbpOfst_80047EE8

- WIP rejected_form: {'form': 'committed src: unused_slack[8] + $16/$18 pins + move asm barrier', 'score': '0 (matches)', 'reason': 'ALL THREE are catalogued cheats (dead-vars-local-array FORBIDDEN 2026-05-31; register pins; inline-move-aliasing asm injection). Not COMPLETED-C.'}

- WIP rejected_form: {'form': 'clean pure-C body (candidate.c)', 'score': 15, 'reason': 'HONEST floor — 0 cheats, loop byte-identical, but 32-byte frame gap + prologue cascade remain. Cannot be lowered by pure-C (no legit way to make unused frame).'}

- == imported from memory/wip notes.md ==
# AddTbpOfst_80047EE8 — WIP (blocked-lane triage 2026-06-14)

## TL;DR — POLICY-GRADE, NOT a pure-C grind
Sprite-effect dispatch (text1b.c:0x80047ee8). The clean pure-C body
(candidate.c, cheat_count 0) has the LOOP byte-identical to target; ALL ~15
sandbox diffs are the FRAME SIZE: target reserves 0x48 (72 bytes), clean C needs
only 0x28 (40). The 0x20 (32-byte) gap is UNUSED frame slack no semantics fill.
The committed src fabricates it with FORBIDDEN cheats. **This needs a project
policy decision — flagged for orchestrator/user.**

## The committed cheats (why it's INCOMPLETE)
1. `s32 unused_slack[8]; (void)unused_slack;` — dead-vars-local-array, FORBIDDEN
   2026-05-31 (engine/volatile_cheats.find_unused_local_arrays). Fabricates the 32 bytes.
2. register pins on $16/$18.
3. INLINE_MOVE_ALIASING `__asm__("move %0,%1")` barrier — preserves a redundant
   `move s2,s0`.
With all three -> SHA1 matches. Without -> sandbox 15 (frame + prologue + 1 move).

## The path forward (per the park, confirmed this pass)
1. PROJECT-WIDE POLICY: either (a) re-sanction a NARROW frame-slack idiom for the
   verified UNUSED-frame case (needs SOTN-grounded research like the volatile
   carve-out), OR (b) canonical-asm-authorize this cluster. Sibling
   InitHiraRmd_80047FBC is IDENTICAL (memory/wip/InitHiraRmd_80047FBC).
2. ROOT-CAUSE the 72-byte frame: did the original Marionation source have a local
   array/struct AddTbpOfst writes to (a stack temp) that our reconstruction
   dropped? Check callers, .data layout, Kengo (PS2 reuse). If a GENUINE used
   buffer exists, declaring it (used) legitimately produces the frame -> clean
   close. THIS is the only single-function avenue; everything else is policy.

## Confirmed this pass
- Clean pure-C body: cheat_count 0, sandbox 15 (target 53 / build 52 insns).
- Loop body byte-identical; the gap is entirely frame-size + prologue cascade
  + the 1 redundant move the asm barrier preserves.

## Pointers
- `.claude/rules/register-alloc-pure-c.md` (dead-vars Lever D FORBIDDEN section)
- `.claude/rules/inline-asm-injection.md` (the move-barrier cheat)
- `memory/wip/InitHiraRmd_80047FBC` (same-cluster sibling, same policy question)


- [s1] Canonical gate: verdict=C, distance 16 (pure-C target). Sandbox --disable all baseline on the wip-imported clean form re-measured 16 (ledger had 15; minor drift).

- [s1] Floor 10 achieved this session: memory/grind/AddTbpOfst_80047EE8/candidate.c; full-stream diff (tmp/grind/AddTbpOfst_80047EE8/s1/diff_p6.out) shows 53/53 insns with ONLY the 10 frame insns differing: addiu sp,sp,-0x28 vs -0x48 and the 8 save/restore offsets shifted by 32 bytes.

- [s1] The 32-byte gap is an unused vars region: target saves regs at 0x38-0x44 and its only other sp store is the 5th outgoing arg at 0x10(sp) (outgoing-args region) — zero stores in 0x18-0x37, so the written-array carve-out is byte-diverging here, same as sibling s7 proof.

- [s1] Unused s32 buf[8] probe: sandbox strips it (cheat_asm_stripped 381->382), score unchanged at 16 — score-inert, cannot even measure the frame component via the cheat form.

- [s1] No near-clone leads in tmp/duplicates.txt for this function; the only siblings are the InitHiraRmd cluster (all cheat-carrying INCOMPLETE, not templates). Sibling InitHiraRmd_80047FBC has an OWNER-ESCALATION filed 2026-07-20 in docs/grind/decisions.md for the IDENTICAL 32-byte unwritten-frame residual, awaiting ruling.

- [s1] The shipped candidate carries arg0=0 /* FAKE */ (dead-store-fake-exception family): mechanism named (cse2 canonical-reg substitution), but the per-function pure-C lever-exhaustion prerequisite is NOT yet discharged — sibling killed the pure alternatives (const-qualified base, decl-order, split-init, s2-s9) on its body; they must be measured (or transfer-argued with probes) on this body before the construct can ship.

- [s1] src/text1b.c reverted to HEAD after measurement (git status clean except metrics + memory/grind); the committed 3-cheat form still holds the oracle match on main.

- [s2] [s2] Baseline re-confirmed: s1 candidate (single-walker + first precompute + FAKE arg0=0) measures sandbox --disable all = 10, 53/53 insns

- [s2] [s2] FAKE removal control = 11: the FAKE arg0=0 store is worth exactly 1 insn (addu s0,a0,v0 vs addu s0,s2,v0), nothing else moves

- [s2] [s2] Exhaustion sweep: const decl-init / decl-order swap / split-init reversal / u32 retype / two-statement rebind ALL measure 11 (53/53) on THIS body — the sibling s8/s9 kills transfer with direct measurement; mask offset spelling measures 14 (52 insns, loses target's srl/sll pair)

- [s2] [s2] The dead-store-fake-exception prerequisite (documented per-function lever-exhaustion + named mechanism cse2 canonical-register substitution) is now DISCHARGED for arg0=0 /* FAKE */ on AddTbpOfst_80047EE8; candidate.c header updated; 6 rejected forms banked in memory/grind/AddTbpOfst_80047EE8/rejected/

- [s2] [s2] docs/grind/decisions.md checked: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (filed 2026-07-20, identical 32-byte unwritten-frame species) still AWAITING RULING; precedent block shows same-species escalations ruled option (b) INCOMPLETE-owner-accepted (motion_SetMotion, saTan0Init, cpu_side_move_dir_4, func_80057CC8)

- [s2] [s2] src/text1b.c reverted to HEAD after measurements (git status clean except metrics + memory/grind); committed 3-cheat form still holds the oracle on main

- [s3] [structural] cc1 .frame instrument built (tmp/grind/AddTbpOfst_80047EE8/s3/framedump.sh): floor-10 form = `.frame $sp,40 # vars=0, regs=4/0, args=24`; TARGET = `.frame $sp,72` vars=32 phantom (ZERO stores in vars region 0x18-0x37). Gap = ALIGN8(vars): need vars in [25,32] with SAME 5 sp-stores + byte-identical 53-insn stream.

- [s3] [structural] PHANTOM-FRAME GRID MEASURED DEAD (9 variants, tmp/grind/AddTbpOfst_80047EE8/s3/frame_grid.md + sweep.py). NO stream-preserving structural variant moves vars off 0 (frame stays 40): fn-scope hoist, u64 word, struct record (adds saved reg -> regs=5 wrong), staged temps, himode-pair-folded — all vars=0. Only a WRITTEN s32 rec[6] reaches vars=24, at the cost of +6 diverging sp-stores the target lacks (forbidden dead-vars-local-array; no carve-out, same as sibling s7).

- [s3] [structural] DECISIVE CONTROL v08_live_guard: the EXACT phantom-frame-slots-gcc272 trigger (two s16 locals feeding `(hv & ~hm) & 1` guarding a REAL global store, the tslLineG5Init mechanism that reserves vars=8) reserves vars=0 in THIS body — it only emits the guard insns, no phantom slot. The reservation is a reload spill-slot artifact of tslLineG5Init's cross-call live HImode temps; this loop consumes every value immediately at the efc call so reload never reserves an unused slot. The frontier F2 mechanism claim ("function-specific phantom-frame trigger, s16->SImode widening + u32 word local") is FALSIFIED: those shapes do not trigger reservation here.

- [s3] [structural] Sandbox re-confirmed floor=10 (`& tools/wteng.ps1 main sandbox AddTbpOfst_80047EE8 --disable all` -> score 10, 53/53 insns) with the candidate.c form applied. src reverted to HEAD after measurement.

- [s3] docs/grind/decisions.md: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (filed 2026-07-20) STILL AWAITING RULING; NO escalation filed for AddTbpOfst_80047EE8 itself yet. Both F1 (FAKE arg0 lever-exhaustion, s2) and F2 (phantom-frame structural axis, s3) now discharged; canonical-asm signals negative (same ordinary table-walker as sibling s10). Function is at the endgame-lock exhaustion state; remaining action = mirroring owner-escalation once the sibling family ruling lands.

- [s3] cc1 .frame instrument: floor-10 form = `.frame $sp,40 # vars=0, regs=4/0, args=24`; TARGET = `.frame $sp,72` vars=32 phantom (zero stores in vars region 0x18-0x37). Gap = ALIGN8(vars): need vars in [25,32] with the same 5 sp-stores and byte-identical 53-insn stream.

- [s3] 9-variant phantom-frame grid: NO stream-preserving structural variant moves vars off 0 (frame stays 40, distance 10). fn-scope hoist / u64 word / staged temps / folded himode-pair all vars=0; struct record adds a saved reg (regs=5, wrong); u64 pair adds 3 saved regs (regs=7, diverges); only a written s32 rec[6] reserves vars (24) but adds 6 diverging sp-stores the target does not contain.

- [s3] DECISIVE: v08_live_guard reproduces the exact phantom-frame-slots-gcc272 trigger (two s16 locals feeding (a&~b)&1 guarding a real global store — the tslLineG5Init mechanism that yields vars=8) and gets vars=0 here. The reservation is a reload spill-slot artifact of tslLineG5Init's cross-call live HImode temps; this loop consumes every value immediately at the efc_buki_draw_zanzou call, so reload never reserves an unused slot. Frontier F2's mechanism claim is falsified.

- [s3] Sandbox re-confirmed floor=10 (53/53 insns) with candidate.c applied; src reverted to HEAD after measurement (committed cheat form still holds the oracle on main).

- [s3] docs/grind/decisions.md: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (filed 2026-07-20) STILL AWAITING RULING; no escalation filed for AddTbpOfst_80047EE8 itself yet. Precedent block: 4 same-species escalations (motion_SetMotion, saTan0Init, cpu_side_move_dir_4, func_80057CC8) all ruled option (b) INCOMPLETE-owner-accepted.

- [s3] Exhaustion state: F1 (FAKE arg0 pure-spelling exhaustion, s2) + F2 (phantom-frame structural axis, s3) both discharged with measurements; canonical-asm signals negative (ordinary table walker, same as sibling s10). No sanctioned pure-C lever remains.

- [s4] [permuter] Whole-function permuter campaign on the floor-10 clean chassis (tmp/grind/AddTbpOfst_80047EE8/s4/ws, --stack-diffs so frame offsets score; base_score 266, 9451 iters, fresh seed): 3 novel finds, best 202, NEVER 0. Descent 266->242->202 came from ONE lever only — a volatile dead-frame local (`volatile long pad;` widened to `volatile unsigned long long pad;`) reserving phantom frame bytes. That is a FORBIDDEN volatile-coercion / dead-vars frame cheat (inline-asm-policy expanded catalog). It reserves only ~8 bytes so it plateaus at 202; the next novel was a WORSE 207 at 444s. The permuter found NO legitimate pure-C frame-growth form.
- [s4] [permuter] Independent corroboration of s3: the 32-byte phantom frame is not reachable by any non-cheat C shape in this body. Structural sweep (s3, 9-variant grid + v08 decisive control) and random permuter search (s4, ~9500 iters) BOTH conclude the only frame-growth levers are forbidden dead/volatile-local cheats. Frontier F2's permuter avenue is now measured DEAD in the permuter modality.
- [s4] Artifacts: tmp/grind/AddTbpOfst_80047EE8/s4/build_ws.sh (workspace builder), tmp/grind/AddTbpOfst_80047EE8/s4/ws/ (base.c/target.o/compile.sh/settings.toml + output-{242,202,207}-1). Rejected form banked: memory/grind/AddTbpOfst_80047EE8/rejected/permuter-volatile-pad-frame-coercion.c. Campaign stopped clean (7 procs killed, no orphans).
- [s4] Floor unchanged at 10; candidate.c form re-confirmed sandbox --disable all = 10 (53/53) this session before the campaign. src/text1b.c reverted to HEAD after (git clean except metrics + memory/grind). docs/grind/decisions.md unchanged since s3: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (2026-07-20) still AWAITING RULING; no escalation filed for AddTbpOfst_80047EE8 itself.

- [s4] Permuter campaign base_score 266 (--stack-diffs, frame offsets scored); best-new 202 over 9451 iters; all 3 finds non-zero and volatile-pad cheat-forms.

- [s4] The permuter's sole frame-growth lever is inserting a volatile dead local (volatile long / unsigned long long pad) to reserve phantom frame bytes — the forbidden volatile-coercion/dead-vars cheat family (inline-asm-policy expanded catalog 2026-05-31).

- [s4] 202 plateau reserves only ~8 bytes; 32 needed. Next novel was a WORSE 207 — the basin yielded its improvement early and then only sideways/worse variants (fresh-seed 'basin yields early or not at all' signal).

- [s4] Independent corroboration of s3: structural sweep (9-variant grid + v08 decisive control) and random permuter search (~9500 iters) BOTH conclude the 32-byte phantom frame is unreachable by any non-cheat C shape in this body.

- [s4] Floor unchanged at 10; candidate.c re-confirmed sandbox --disable all = 10 (53/53) this session; src/text1b.c reverted to HEAD (git clean except metrics + memory/grind). Committed 3-cheat form still holds the oracle on main.

- [s4] docs/grind/decisions.md unchanged since s3: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (2026-07-20) still AWAITING RULING; no escalation filed for AddTbpOfst_80047EE8 itself yet.

- [s5] [permuter] Fresh-seed campaign on a STRUCTURALLY DIFFERENT chassis — the no-FAKE floor-11 form (distinct RA basin from s4's floor-10 FAKE chassis; sandbox --disable all = 11, 53/53). base_score 5200 (vs s4's 266 — the no-FAKE form scrambles register allocation, giving a genuinely different, noisier basin). 17057 iters, -j6, --stack-diffs.
- [s5] [permuter] Campaign descended 5200 -> 207 and PLATEAUED FLAT at 207 from iter ~12753 through 17057 (driver trajectory tmp/grind/AddTbpOfst_80047EE8/s5/ws + drive_window.py output: best held at 207 across thousands of iters, zero novel improvement), then harvested + stopped clean (pid_alive False, NO orphan permuter procs). Never reached 0.
- [s5] [permuter] The 207 best form's ONLY mutation vs the clean chassis is `volatile unsigned int pad;` — an `unsigned int` variant of the SAME forbidden volatile dead-frame-local cheat s4 found (`volatile long`/`unsigned long long pad`). Reserves ~8 phantom bytes (not the needed 32), so it plateaus and cannot close. Banked: memory/grind/AddTbpOfst_80047EE8/rejected/permuter-volatile-uint-pad-nofake-basin.c.
- [s5] [permuter] Independent corroboration across a SECOND structurally-distinct basin: s3 (structural 9-variant grid), s4 (permuter on floor-10 chassis, ~9500 iters), and now s5 (permuter on floor-11 no-FAKE chassis, ~17000 iters) — ~26500 combined permuter iters over two chassis basins — ALL conclude the 32-byte phantom frame is reachable ONLY via forbidden dead/volatile-local cheats, never a legit pure-C frame-growth form. Permuter modality is now measured DEAD on this function across BOTH chassis basins.
- [s5] Floor unchanged at 10; candidate.c (floor-10 FAKE form) re-confirmed as best. Baseline of the s5 no-FAKE chassis measured sandbox --disable all = 11 this session before the campaign. src/text1b.c reverted to HEAD after (git clean except metrics + new rejected form). docs/grind/decisions.md unchanged since s4: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (2026-07-20) still AWAITING RULING; no escalation filed for AddTbpOfst_80047EE8 itself yet.
- [s5] Artifacts: tmp/grind/AddTbpOfst_80047EE8/s5/build_ws.sh (workspace builder), tmp/grind/AddTbpOfst_80047EE8/s5/drive_window.py (wall-clock window driver), tmp/grind/AddTbpOfst_80047EE8/s5/ws/ (base.c/target.o/compile.sh/settings.toml + output-207-1 the best/cheat form).

- [s5] s5 permuter campaign ran on a genuinely distinct basin: the no-FAKE floor-11 chassis (sandbox --disable all = 11, 53/53) rather than s4's floor-10 FAKE chassis. permuter base_score 5200 (vs s4's 266) confirms the no-FAKE form scrambles whole-function register allocation into a different, noisier search landscape.

- [s5] Campaign descended 5200 -> 207 fast, then plateaued FLAT at 207 from iter ~12753 to 17057 (driver trajectory in drive_window.py output), never 0. Harvested + stopped clean; pid_alive False; NO orphan permuter procs.

- [s5] The 207 best form's ONLY mutation vs the clean chassis is `volatile unsigned int pad;` — an unsigned-int variant of the SAME forbidden volatile dead-frame-local cheat s4 found (volatile long / unsigned long long pad). Reserves ~8 phantom bytes (not the needed 32), so it plateaus and cannot close. Banked at rejected/permuter-volatile-uint-pad-nofake-basin.c.

- [s5] Triple corroboration that the 32-byte phantom frame is unreachable by non-cheat C: s3 (structural 9-variant grid + v08 decisive control), s4 (permuter on floor-10 chassis ~9500 iters), s5 (permuter on floor-11 no-FAKE chassis ~17000 iters). ~26500 combined permuter iters across two structurally-distinct basins BOTH conclude the only frame-growth levers are forbidden dead/volatile-local cheats.

- [s5] Floor unchanged at 10; candidate.c (floor-10 FAKE form) remains best. src/text1b.c reverted to HEAD after measurements (git clean except metrics + new rejected form). docs/grind/decisions.md unchanged since s4: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (2026-07-20) still AWAITING RULING; no escalation filed for AddTbpOfst_80047EE8 itself yet.

- [s6] [forensics] cc1 -da full-pass RTL dump of the floor-10 candidate (tmp/grind/AddTbpOfst_80047EE8/s6/dumps/, tu.i.{rtl,greg,...}). .frame = `$sp,40 vars=0 regs=4/0 args=24`. get_frame_size()=0 at BOTH .rtl (post-expand) AND .greg (post-reload): ZERO frame/stack-var refs in the fn at either pass. .greg register dispositions: all 22 pseudos (74-99) land in hard regs; "Hard regs used: 2 4 5 6 7 16 17 18 29 31" — NO frame pointer, NO spill slots. Route-b (reload spill) is provably inactive; route-a (function.c source-DECL slot) emits nothing because the body declares no stack local.
- [s6] [forensics] TARGET frame layout decoded from asm/funcs/AddTbpOfst_80047EE8.s: `.frame $sp,72`. args 0x00-0x17 (24, holds 5th efc arg $v0@0x10), vars 0x18-0x37 (32, phantom), regs 0x38-0x47 (16: s0@0x38 s1@0x3C s2@0x40 ra@0x44). The ONLY sp stores are the 4 reg-saves (0x38-0x44) + the outgoing 5th-arg $v0@0x10; the ONLY sp loads are the 4 reg-restores. ZERO sw/lw in the vars region 0x18-0x37.
- [s6] [forensics] DECISIVE: a reload spill slot ALWAYS emits sw(spill)+lw(reload) into its slot. Target's vars region has ZERO stores/loads, so the 32 bytes CANNOT be a reload/register-pressure artifact. This FALSIFIES the phantom-frame-slots-gcc272 / tslLineG5Init framing that s3/s4/s5 chased (that mechanism is a reload spill-slot reservation). The residual is NOT a register-allocation or scheduling phenomenon.
- [s6] [forensics] POSITIVE CONTROL (tmp/grind/AddTbpOfst_80047EE8/s6/control/ctl.{c,s,c.rtl}): a source-level declared local array `int buf[8]` reproduces target's frame shape EXACTLY. ctlA (buf[8] whose address escapes to sink()) = `.frame $sp,56 vars=32` with ZERO stores into the region; ctlB (declared-but-unused buf[8]) = `vars=32` with ZERO body stores. GCC 2.7.2 function.c allocates the array slot via assign_stack_local at RTL-expand from the DECL and NEVER reclaims frame_offset after DCE — so a dead/unwritten local aggregate reserves the vars region with no emitted stores, byte-shape-identical to target.
- [s6] [forensics] NAMED MECHANISM: the exact GCC pass/decision producing the 32-byte divergence is function.c stack-frame layout (assign_stack_local, invoked at RTL-EXPAND from a source-level local-aggregate DECL), NOT global.c RA, NOT reload spilling, NOT sched/reorg. Target's original C declared a >=32-byte (8-word) local aggregate that is dead in the emitted output. The ONLY pure-C form reproducing it is declaring such a local = the forbidden dead-vars-local-array; the 2026-07-01 WRITTEN carve-out is INAPPLICABLE (target has zero stores in the region). This is upstream of RA, so no RA/scheduling lever (the entire s3/s4/s5 search space) can ever reach it. Endgame-lock confirmed with a precisely named mechanism.
- [s6] Floor unchanged at 10; candidate.c re-confirmed sandbox --disable all = 10 (53/53) this session before dumping. src/text1b.c reverted to HEAD after (git clean except metrics). docs/grind/decisions.md: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (2026-07-20) still AWAITING RULING; no escalation filed for AddTbpOfst_80047EE8 itself yet. Note: the 2026-07-20 19:32 "AddTbpOfst — PASS" ruling in decisions.md is a DIFFERENT function (unsuffixed AddTbpOfst, D_801027F1/F6 table walker), NOT AddTbpOfst_80047EE8.
- [s6] Artifacts: tmp/grind/AddTbpOfst_80047EE8/s6/dump.sh, s6/control.sh, s6/dumps/tu.i.{rtl,greg,lreg,combine,sched,jump2,dbr,...} (full pass pipeline), s6/dumps/tu.s, s6/control/ctl.{c,s,c.rtl} (positive control).

- [s6] cc1 -da candidate dump: .frame $sp,40 vars=0 regs=4/0 args=24; get_frame_size()=0 at both .rtl (post-expand) and .greg (post-reload); ZERO frame/stack-var refs in the fn at either pass.

- [s6] .greg register dispositions: all 22 pseudos land in hard regs; 'Hard regs used: 2 4 5 6 7 16 17 18 29 31' — no frame pointer, no spill slots. Route-b (reload spill) provably inactive.

- [s6] Target frame $sp,72 decoded: args 0x00-0x17 (24, 5th efc arg $v0@0x10), vars 0x18-0x37 (32 phantom), regs 0x38-0x47 (s0/s1/s2/ra). ONLY sp stores = 4 reg-saves + $v0@0x10 outgoing arg; ONLY sp loads = 4 reg-restores. ZERO sw/lw in the vars region.

- [s6] A reload spill slot always emits sw(spill)+lw(reload); target's zero region accesses PROVE the 32 bytes are NOT a reload/RA/scheduling artifact — falsifying the phantom-frame-slots-gcc272/tslLineG5Init framing s3/s4/s5 chased.

- [s6] Positive control ctlA/ctlB: GCC 2.7.2 function.c allocates a declared local-array slot via assign_stack_local at RTL-EXPAND from the DECL and never reclaims frame_offset after DCE; a dead/unwritten local aggregate reserves vars=32 with no emitted stores = exact target shape.

- [s6] NAMED MECHANISM: the divergence pass is function.c stack-frame layout (assign_stack_local at RTL-EXPAND from a source-level local-aggregate DECL), upstream of global.c RA and reload. Target's original C declared a >=32-byte (8-word) dead local aggregate. Only pure-C reproduction = forbidden dead-vars-local-array; 2026-07-01 WRITTEN carve-out inapplicable (target has zero region stores).

- [s6] Floor unchanged at 10 (candidate.c re-confirmed sandbox --disable all = 10, 53/53 this session before dumping). src/text1b.c reverted to HEAD after measurement.

- [s6] docs/grind/decisions.md: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (2026-07-20) still AWAITING RULING; no escalation filed for AddTbpOfst_80047EE8 itself. The 2026-07-20 19:32 'AddTbpOfst PASS' entry is a DIFFERENT function (unsuffixed table walker), not AddTbpOfst_80047EE8.

- [s7] [forensics] CROSS-CLUSTER INVARIANT (decoded directly from committed asm/funcs, no toolchain needed): all 4 cluster members reserve EXACTLY 32 phantom bytes in the vars region 0x18-0x37 with ZERO sw/lw into it, despite different callee-save counts and total frames — AddTbpOfst_80047EE8 (frame 0x48/72, 4 saved regs s0/s1/s2/ra @0x38-0x44), InitHiraRmd_80047FBC (0x50/80, 6 regs s0-s4/ra @0x38-0x4C), InitHiraRmd_800480C0 (0x58/88, 8 regs s0-s6/ra @0x38-0x54), func_800481E8 (0x48/72, 4 regs). In every case args=0x00-0x17 (24, holds the 5th efc arg $v0@0x10), vars=0x18-0x37 (32 phantom), reg-saves start at 0x38. The 32-byte phantom is INVARIANT across the cluster regardless of register pressure — evidence of a SHARED source-level dead local aggregate (~32 bytes) in the original Marionation source of all four, not a per-function RA accident.

- [s7] [forensics] SIZE-PIN cc1 probe (tmp/grind/AddTbpOfst_80047EE8/s7/probe/agg.{c,s}, decompals gcc-2.7.2 cc1 -O2 -G0 -mips1): source-aggregate-size -> reserved vars: int[6]=24B ->vars=24, int[7]=28B ->vars=32, int[8]=32B ->vars=32, int[9]=36B ->vars=40 (8-byte frame alignment). Target's vars=32 therefore pins the original dead local aggregate at 25-32 bytes = a 7- or 8-word int array (or any 25-32B aggregate rounding to 32). a8_dead (declared-unused int[8]) = `.frame $sp,32 vars=32` with ZERO sp stores into the region = byte-shape-identical to target. Independently re-confirms s6's named mechanism (function.c assign_stack_local at RTL-expand from a source DECL) via the deterministic size->frame mapping.

- [s7] [forensics] ARRAY-vs-STRUCT distinction: a same-size (32-byte) local STRUCT (MATRIX-shaped: short[3][3]+long[3]) reserved vars=0 in BOTH the addressed (mtx_used, &m passed to sink) and unused (mtx_dead) forms — GCC 2.7.2 scalarized/eliminated the struct, whereas the int[8] array robustly reserves vars=32 whether addressed or dead. So the ONLY source construct reproducing target's zero-store 32-byte phantom is a dead local ARRAY = exactly the forbidden dead-vars-local-array. The 2026-07-01 WRITTEN carve-out stays inapplicable (target has zero stores in the region); no non-array legit substitute exists. Airtight re-confirmation of endgame-lock.

- [s7] Floor UNCHANGED at 10 (inherited; not re-measured this session — forensics modality names the mechanism, does not lower the floor; the floor-10 candidate.c form is unanimously re-confirmed across s1-s6). src/text1b.c left at HEAD (committed cheat form still holds the oracle on main); git clean except metrics + memory/grind + s7 scratch. docs/grind/decisions.md unchanged since s6: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (2026-07-20) still AWAITING RULING; NO escalation entry names AddTbpOfst_80047EE8 itself yet, so owner-gated is not available — this remains progress-recorded exhaustion. Artifacts: tmp/grind/AddTbpOfst_80047EE8/s7/probe.sh, s7/probe/agg.{c,s}, s7/probe/agg.c.{rtl,greg,...}.

- [s7] Cross-cluster invariant (decoded from committed asm, no toolchain): all 4 sibling functions reserve EXACTLY 32 phantom bytes in vars region 0x18-0x37 with ZERO sw/lw into it, despite 4/6/8/4 callee-saved regs and 72/80/88/72 total frames. In each, args=0x00-0x17 (24, 5th efc arg $v0@0x10), vars=0x18-0x37 (32 phantom), reg-saves from 0x38.

- [s7] cc1 size-pin probe: source-aggregate-size -> reserved-vars is deterministic (int[6]=24, int[7]/[8]=32, int[9]=40 at 8-byte frame alignment). Target vars=32 => original dead local aggregate is 25-32 bytes = a 7- or 8-word int array. a8_dead (declared-unused int[8]) = .frame vars=32 with ZERO sp stores = byte-shape-identical to target.

- [s7] Array-vs-struct: a same-size 32-byte struct local reserved vars=0 in both addressed and dead forms (GCC scalarized/eliminated it); only the array form robustly reserves the zero-store 32-byte slot. So the ONLY pure-C construct reproducing target is a dead local ARRAY = the forbidden dead-vars-local-array; no legit struct substitute; 2026-07-01 WRITTEN carve-out inapplicable (target has zero stores in the region).

- [s7] Independently re-confirms s6's named mechanism (function.c assign_stack_local at RTL-EXPAND from a source-level local-aggregate DECL, upstream of global.c RA and reload) via the deterministic size->frame mapping AND the cross-cluster pressure-invariance, strengthening it from a single-function claim to a cluster-wide species claim.

- [s7] Floor unchanged at 10 (inherited; not re-measured this session — forensics names the mechanism, does not lower the floor; floor-10 candidate.c unanimously re-confirmed s1-s6). src/text1b.c left at HEAD (committed cheat form still holds the oracle). docs/grind/decisions.md unchanged: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (2026-07-20) still AWAITING RULING; NO escalation entry names AddTbpOfst_80047EE8 itself, so owner-gated is unavailable — progress-recorded exhaustion.

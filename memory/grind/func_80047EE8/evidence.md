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

- [s8] [rederive] Fresh m2c decompile of asm/funcs/AddTbpOfst_80047EE8.s (tools/m2c/m2c.py --valid-syntax; tmp/grind/AddTbpOfst_80047EE8/s8/m2c_fresh.c) reconstructs the body with arg0 kept LIVE as the base throughout (`efc_buki_draw_zanzou(arg0 + ((temp_a0>>2)*4), ...)`) and declares ZERO local aggregate / NO frame array (vars=0). An independent decompiler working from the target bytes produces a body with no local-array correlate — the target's 32-byte phantom vars region (0x18-0x37, zero sw/lw) has NO semantic content. Independently re-confirms the s6/s7 function.c-source-DECL mechanism from a fully separate tool.
- [s8] [rederive] The m2c natural shape (arg0-live base, single void*-walker) transcribed to project idioms and measured: `& tools/wteng.ps1 main sandbox AddTbpOfst_80047EE8 --disable all` = score 35, build_insns 52 (target 53). WORSE than the floor-10 candidate and one insn short — arg0-live copy-props base==arg0 so the second pointer binds from $a0 not target's $s2, corroborating s2's finding that the FAKE arg0=0 store is the only construct reaching the $s2 binding on this body. Banked: rejected/m2c-arg0live-natural-rederive.c; applied form tmp/grind/AddTbpOfst_80047EE8/s8/m2c_rederive_applied.c.
- [s8] [rederive] Kengo transplant lane: kengo_matches.csv maps AddTbpOfst_80047EE8 -> Kengo `AddTbpOfst` in src/amami/am_rmd.c (49 insns vs our 53, diff -4, name-unique, combined_score 0.13). BUT the repo holds only the Kengo disc BIN (Kengo/Kengo - Master of Bushido (USA).bin) — NO Kengo source tree exists (src/amami absent), and Kengo is PS2 (different toolchain). Not transplantable; the CSV row is a disasm-matching heuristic, not source. (Mirrors sibling InitHiraRmd s8 finding that the Kengo name is not a usable transplant.)
- [s8] [rederive] decomp.me corpus lane (tools/decomp_me_scrape.py search --asm-file asm/funcs/AddTbpOfst_80047EE8.s --top 8; tmp/grind/AddTbpOfst_80047EE8/s8/decompme_search.txt): the closest community scratches are gcc2.7.2-cdk table-walkers at max similarity 0.138 — pure shingle noise (any GCC 2.7.2 loop shares these), no structural sibling, no scratch reproducing the phantom-frame idiom. No external community solution exists for this shape.
- [s8] Floor unchanged at 10: candidate.c re-measured sandbox --disable all = 10 (53/53) this session. src/text1b.c reverted to HEAD after all measurements (git clean except metrics + memory/grind + tmp scratch); committed 3-cheat form still holds the oracle on main. docs/grind/decisions.md: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (2026-07-20) STILL AWAITING RULING; NO escalation entry names AddTbpOfst_80047EE8 itself yet, so owner-gated remains unavailable — this stays progress-recorded exhaustion. Rederive modality now exhausted across all three lanes (m2c / Kengo / decomp.me).

- [s8] Fresh m2c (tools/m2c/m2c.py --valid-syntax) reconstructs the body with arg0 kept LIVE as the base throughout and declares ZERO local aggregate (vars=0): an independent decompiler confirms the target's 32-byte phantom vars region (0x18-0x37, zero sw/lw) has NO semantic correlate - re-confirming the s6/s7 function.c source-DECL mechanism from a fully separate tool.

- [s8] The m2c natural shape transcribed to project idioms measured sandbox --disable all = 35, build_insns 52 vs target 53 - worse than the floor-10 candidate and one insn short; arg0-live binds the second pointer from $a0 not $s2, corroborating s2 that the FAKE arg0=0 store is the only construct reaching the $s2 binding on this body. Banked rejected/m2c-arg0live-natural-rederive.c.

- [s8] Kengo transplant lane dead: kengo_matches.csv maps AddTbpOfst_80047EE8 -> Kengo AddTbpOfst am_rmd.c:49 (name-unique, combined_score 0.13) but the repo holds only Kengo/*.bin (no src/amami tree, PS2 toolchain); the CSV row is a disasm-matching heuristic, not usable source.

- [s8] decomp.me corpus lane dead: closest community scratches are gcc2.7.2-cdk table-walkers at max similarity 0.138 (shingle noise), no structural sibling, no scratch reproducing the zero-store phantom-frame idiom.

- [s8] Floor unchanged at 10: candidate.c re-measured sandbox --disable all = 10 (53/53) this session; src/text1b.c reverted to HEAD after all measurements (git clean except metrics + memory/grind + tmp).

- [s8] docs/grind/decisions.md: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (filed 2026-07-20) STILL AWAITING RULING; NO escalation entry names AddTbpOfst_80047EE8 itself, so owner-gated is unavailable - this remains progress-recorded exhaustion.

- [s8] All five modalities (recon s1 / structural s2-s3 / permuter two-basin s4-s5 / forensics s6-s7 / rederive s8) now exhausted with measured negatives; the 32-byte residual is upstream of every sanctioned pure-C axis (function.c assign_stack_local from a dead source-level local-array DECL, cluster-invariant, array-only, no struct substitute, WRITTEN carve-out inapplicable).

- [s9] [rederive] NEW LANE (sibling-committed-chassis transplant, not run in s8's m2c/Kengo/decomp.me): transplanted InitHiraRmd_80047FBC's committed `u32 *base` pointer-carry chassis (arg0=0 FAKE break; loop-body offset compute placed between the a2v and a3v reads) onto AddTbpOfst_80047EE8 — a structurally-DIFFERENT pure-C shape from BOTH the floor-10 `s32 saved` candidate AND s8's arg0-live m2c form. Measured `& tools/wteng.ps1 main sandbox AddTbpOfst_80047EE8 --disable all` = score 10, 53/53 insns.
- [s9] [rederive] Full objdump diff (tmp/grind/AddTbpOfst_80047EE8/s9/build.norm vs target.norm via diff.sh): the emitted body is BYTE-IDENTICAL to target EXCEPT the 10 frame-offset insns — `addiu sp,-0x28` (build) vs `-0x48` (target) + the 8 save/restore offsets shifted by the 32-byte phantom vars region. IDENTICAL residual to the floor-10 candidate; the pointer-carry form is CO-OPTIMAL, not an improvement.
- [s9] [rederive] STRENGTHENS the endgame-lock: a THIRD structurally-distinct legit pure-C chassis (pointer-carry `u32 *base`) converges on the EXACT same frame-only residual. Not only does no structurally-different shape ESCAPE the 32-byte phantom frame (s8), multiple distinct reaching-forms hit the SAME invariant residual — the residual is chassis-INDEPENDENT, confirming it is the s6/s7-named function.c source-DECL phantom (dead >=32-byte local array), upstream of every C-structure lever. s8's arg0-live=35 was one insn short only because it lost the $s2 binding; the two forms that DO reach the target instruction set (s32 saved + u32 base) both land on the identical 32-byte gap.
- [s9] Floor unchanged at 10. candidate.c (floor-10 s32 saved form) retained as best; the co-optimal pointer-carry form banked at rejected/sibling-chassis-pointer-carry-floor10.c. src/text1b.c reverted to HEAD after measurement (git clean except metrics); committed 3-cheat form still holds the oracle on main. docs/grind/decisions.md: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (2026-07-20) STILL AWAITING RULING (confirmed still open, lines 985/1114); NO escalation entry names AddTbpOfst_80047EE8 itself, so owner-gated remains unavailable — this stays progress-recorded exhaustion. Artifacts: tmp/grind/AddTbpOfst_80047EE8/s9/diff.sh, s9/build.dis, s9/build.norm, s9/target.norm.

- [s9] s9 sibling-committed-chassis transplant (u32 *base pointer-carry) measured sandbox --disable all = score 10, 53/53 insns — a structurally-different pure-C shape from BOTH the floor-10 s32-saved candidate AND s8's arg0-live m2c form (which scored 35).

- [s9] Full objdump diff (tmp/grind/AddTbpOfst_80047EE8/s9/build.norm vs target.norm via diff.sh): emitted body BYTE-IDENTICAL to target except the 10 frame-offset insns (addiu sp -0x28 vs -0x48 + 8 shifted save/restore offsets) — the identical 32-byte phantom-frame residual as the floor-10 candidate.

- [s9] Endgame-lock STRENGTHENED: three structurally-distinct legit pure-C chassis (s32 saved floor-10, u32 *base pointer-carry floor-10, m2c arg0-live scored 35 one insn short on the $s2 binding) — the two that reach the target instruction set BOTH land on the exact same 32-byte gap, proving the residual is chassis-invariant and is the s6/s7-named function.c source-DECL phantom (dead >=32-byte local array), upstream of all C-structure levers.

- [s9] Floor unchanged at 10; candidate.c (s32-saved floor-10 form) retained as best; co-optimal pointer-carry form banked at rejected/sibling-chassis-pointer-carry-floor10.c. src/text1b.c reverted to HEAD after measurement (git clean except metrics); committed 3-cheat form still holds the oracle on main.

- [s9] docs/grind/decisions.md confirmed still open: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (filed 2026-07-20) STILL AWAITING RULING (lines 985/1114); NO escalation entry names AddTbpOfst_80047EE8 itself — owner-gated therefore remains unavailable; this stays progress-recorded exhaustion, not self-resolved.

- [s10] [synthesis] SYNTHESIS — full modality ladder MERGED and both endgame-lock-disposition AND-gates directly measured on THIS function (mirrors sibling InitHiraRmd_80047FBC s10, decisions.md line 1008). Ladder complete: recon s1 (floor 15->10, single-walker + live first-arg precompute + FAKE arg0=0), structural s2 (F1 FAKE arg0 lever-exhaustion discharged, 6 pure spellings dead) + s3 (F2 phantom-frame 9-variant grid dead, v08 tslLineG5Init decisive control vars=0), permuter two-basin s4 (floor-10 chassis, 9451 iters, only forbidden volatile-pad) + s5 (floor-11 no-FAKE chassis, 17057 iters, plateau 207, only forbidden volatile-pad), forensics s6 (NAMED function.c assign_stack_local at RTL-expand from source-level dead local aggregate; reload-spill theory FALSIFIED — zero region sw/lw) + s7 (cluster-invariant 32B across 4 members; cc1 size-pin: vars=32 => dead int[7-8]; array-only, struct scalarizes to vars=0), rederive s8 (m2c arg0-live=35 / Kengo no-source / decomp.me noise) + s9 (sibling u32*base chassis transplant = co-optimal floor 10, chassis-invariant residual).

- [s10] [synthesis] GATE (a) canonical-asm — DIRECTLY MEASURED negative this session (s1-s9 only transfer-argued it from the sibling): `python3 tools/scan_hand_coded.py --single AddTbpOfst_80047EE8` => tier=LOW score=1/8 "no strong hand-coded indicators"; only S4 (4 loads in 8-insn window @insn19) fires — the ordinary load-batch of a table walker; S1 multu-pacing/S2 empty-branch/S6 BIOS-jumptable (the STRONG signals per endgame-lock-disposition) all ABSENT. `& tools/wteng.ps1 main canonical AddTbpOfst_80047EE8` => verdict=C, asm_insns=0, distance=8, "pure-C distance 8 <= 50 — pure-C target". Ordinary GCC-scheduled table walker, not hand-written asm.

- [s10] [synthesis] GATE (b) coercion-family — no SOTN/community precedent for an unwritten >=32-byte local-array phantom-frame carrier (established s6 positive-control + s7 cluster-invariant size/type pin + array-only proof; identical survey to the sibling escalation decisions.md lines 993/1013). The ONLY construct reproducing target's zero-store 32-byte vars region is the forbidden dead-vars-local-array (dead s32 buf[7-8]); 2026-07-01 WRITTEN carve-out inapplicable (target has ZERO sw in 0x18-0x37); no struct substitute (GCC 2.7.2 scalarizes a same-size struct to vars=0). BOTH AND-gates fail => endgame-lock species CONFIRMED for AddTbpOfst_80047EE8 by direct measurement.

- [s10] [synthesis] MERGED ATTACK / DISPOSITION: no un-run sanctioned pure-C axis remains and no un-run modality remains (all 6 ladder rungs exercised). The 32-byte residual is upstream of every C-structure/RA/scheduling lever (function.c assign_stack_local from a dead source-level local ARRAY DECL, cluster-invariant across all 4 members). Terminal artifact = the mirroring OWNER-ESCALATION for AddTbpOfst_80047EE8; drafted verbatim-ready at memory/grind/AddTbpOfst_80047EE8/escalation_draft.md for an escalation-modality session to file into docs/grind/decisions.md (adapted frame numbers: frame 0x48/72, 4 saved regs s0/s1/s2/ra @0x38-0x44, vars 0x18-0x37=32 phantom, args 0x00-0x17 holding 5th efc arg $v0@0x10). Per the parallel-filing precedent (func_80049A2C, gnd_init_80041688, InitHiraRmd_80047FBC all filed while awaiting ruling), it need NOT wait for the sibling ruling to land.

- [s10] Floor unchanged at 10 (synthesis names/merges, does not re-measure; floor-10 candidate.c unanimously re-confirmed s1-s9). src/text1b.c left at HEAD (no edit this session; committed cheat form still holds the oracle on main; git clean except metrics + memory/grind). docs/grind/decisions.md: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (2026-07-20) STILL AWAITING RULING (line 985); NO escalation entry names AddTbpOfst_80047EE8 itself yet, so owner-gated remains unavailable to THIS session — progress-recorded exhaustion, escalation drafted and handed to the next (escalation) session. Artifacts: tmp/grind/AddTbpOfst_80047EE8/s10/gates.log.

- [s10] GATE (a) canonical-asm measured NEGATIVE directly on this function: scan_hand_coded tier=LOW score=1/8, only weak S4 (4 loads in 8-insn window @insn19) fires; STRONG S1 multu-pacing / S2 empty-branch / S6 BIOS-jumptable all absent. Canonical verdict=C, asm_insns=0, distance=8 ('pure-C target'). Ordinary GCC-scheduled table walker, not hand-written asm.

- [s10] GATE (b) coercion-family: no SOTN/community precedent for an unwritten >=32-byte local-array phantom-frame carrier. The only construct reproducing target's zero-store 32-byte vars region (0x18-0x37) is the forbidden dead-vars-local-array (dead s32 buf[7-8] per s7 cc1 size-pin); WRITTEN carve-out inapplicable (target has ZERO sw/lw in the vars region); no struct substitute (GCC 2.7.2 scalarizes a same-size struct to vars=0).

- [s10] Full modality ladder MERGED (all 6 rungs, measured negatives): recon s1 (floor 15->10), structural s2 (F1 FAKE-arg0 lever-exhaustion, 6 spellings dead) + s3 (F2 phantom-frame 9-variant grid dead, v08 decisive control vars=0), permuter s4+s5 (two chassis basins, ~26500 iters, only forbidden volatile-pad, never 0), forensics s6 (NAMED function.c assign_stack_local at RTL-expand from a dead source-level local-array DECL; reload-spill theory FALSIFIED) + s7 (cluster-invariant 32B across 4 members; array-only), rederive s8 (m2c=35 / Kengo no-source / decomp.me noise) + s9 (sibling u32*base chassis transplant co-optimal floor 10, chassis-invariant residual).

- [s10] The 32-byte residual is upstream of every C-structure/RA/scheduling lever and chassis-invariant across all three reaching C shapes (s32 saved, u32*base, and cluster). Both AND-gates fail -> endgame-lock species CONFIRMED. No un-run sanctioned pure-C axis and no un-run modality remain.

- [s10] docs/grind/decisions.md re-checked this session: sibling InitHiraRmd_80047FBC OWNER-ESCALATION (line 985, filed 2026-07-20) STILL AWAITING RULING; NO escalation entry names AddTbpOfst_80047EE8 itself, so owner-gated is unavailable to this session. Mirroring escalation drafted verbatim-ready at memory/grind/AddTbpOfst_80047EE8/escalation_draft.md.

- [s10] Floor unchanged at 10 (synthesis merges/names, does not re-measure the floor; floor-10 candidate.c unanimously re-confirmed s1-s9). src/text1b.c left at HEAD (no edit this session); git clean except metrics + memory/grind + tmp scratch.

- [s11] [structural] ESCALATION-DISPOSITION. Assigned structural axis is measured dead (s2 6-spelling sweep + s3 9-variant phantom-frame grid + v08 decisive control); the s6/s7-named mechanism (function.c assign_stack_local at RTL-expand from a source-level dead local ARRAY DECL) is provably upstream of every C-structure/RA/scheduling lever, so NO un-run structural lever exists. Per the prime directive I did not re-run the dead axes. Terminal disposition action taken instead: filed the mirroring OWNER-ESCALATION for AddTbpOfst_80047EE8 into docs/grind/decisions.md (2026-07-21, filed by grind s11), adapting the s10 verbatim draft with frame numbers 0x48/72, 4 saved regs s0/s1/s2/ra @0x38-0x44, vars 0x18-0x37=32 phantom.

- [s11] Handoff verified independently (verify-opus-handoff-claims): applied candidate.c to src/text1b.c and re-measured `& tools/wteng.ps1 main sandbox AddTbpOfst_80047EE8 --disable all` = score 10, 53/53 insns (clean pure-C floor CONFIRMED this session). The current HEAD committed-cheat form (register pins $16/$18 + s32 unused_slack[8] + INLINE_MOVE_ALIASING), measured with cheat-asm stripped, = score 8, build_insns 51 (2 insns SHORT of target 53) — this 8 is a stripped-cheat artifact, NOT a legitimate reachable pure-C form; the honest clean floor is 10. src/text1b.c reverted to HEAD via git checkout after measurement (git clean except metrics/events.jsonl).

- [s11] docs/grind/decisions.md pre-file state confirmed: NO entry named AddTbpOfst_80047EE8 existed (owner-gated was therefore unavailable until this session filed it). Sibling InitHiraRmd_80047FBC OWNER-ESCALATION (line 985, filed 2026-07-20) still AWAITING RULING; parallel-filing precedent confirmed active (func_80049A2C, gnd_init_80041688, func_80033550 all filed while awaiting the family ruling). Escalation now filed => owner-gated disposition valid; the driver parks the function and the queue advances until the owner rules.

- [s11] Artifact: tmp/grind/AddTbpOfst_80047EE8/s11/measurements.md (both sandbox JSONs + revert confirmation).

- [s11] s11 re-verified the clean pure-C floor: candidate.c applied to src/text1b.c measures sandbox --disable all = score 10, 53/53 insns (matches s1-s10 unanimous floor). Residual = exactly the 10 frame-offset insns (addiu sp -0x28 vs target -0x48 + 8 save/restore offsets shifted by the 32-byte phantom vars region).

- [s11] The HEAD committed-cheat form (register pins $16/$18 + s32 unused_slack[8] + INLINE_MOVE_ALIASING), cheat-asm stripped, measures score 8 / build_insns 51 — 2 insns SHORT of target 53. This 8 is a stripped-cheat artifact (stripping INLINE_MOVE_ALIASING drops a real instruction), NOT a legitimate reachable pure-C form; the honest clean floor is 10.

- [s11] Assigned structural axis is measured dead: s2 6-spelling init-chain sweep (all 11 except FAKE=10) + s3 9-variant phantom-frame grid + v08 decisive tslLineG5Init control (all vars=0). The s6/s7-named mechanism (function.c assign_stack_local at RTL-expand from a source-level dead local ARRAY DECL) is upstream of every C-structure/RA/scheduling lever, so no un-run structural lever exists; dead axes were not re-run per the prime directive.

- [s11] Both endgame-lock-disposition AND-gates were measured dead directly in s10: (a) canonical-asm NEGATIVE (scan_hand_coded tier=LOW 1/8, only weak S4 fires; canonical verdict=C distance=8; ordinary GCC-scheduled table walker); (b) coercion-family NEGATIVE (no SOTN precedent for an unwritten >=32-byte phantom-frame array carrier; WRITTEN carve-out inapplicable — target has ZERO sw/lw in vars region 0x18-0x37; a same-size struct scalarizes to vars=0, so ARRAY-only).

- [s11] docs/grind/decisions.md pre-file state: NO entry named AddTbpOfst_80047EE8 existed (owner-gated was unavailable until this session filed it). Filed the mirroring OWNER-ESCALATION at decisions.md line 1191 (2026-07-21, grind s11), adapting the s10 verbatim draft (frame 0x48/72; 4 saved regs s0/s1/s2/ra @0x38-0x44; vars 0x18-0x37=32 phantom; args 0x00-0x17 holding 5th efc arg $v0@0x10).

- [s11] Sibling InitHiraRmd_80047FBC OWNER-ESCALATION (decisions.md line 985, filed 2026-07-20) still AWAITING RULING; parallel-filing precedent active (func_80049A2C, gnd_init_80041688, func_80033550 all filed while awaiting the family ruling) — this filing need not wait for the sibling ruling.

- [s11] src/text1b.c reverted to HEAD via git checkout after measurement; git status --short = only ' M metrics/events.jsonl' (normal engine capture). The committed 3-cheat form still holds the oracle on main.

## s12 (2026-08-20, escalation modality) - BYTES PROVEN; integration handoff

[s12] CHASSIS RE-BASELINE. On the post-migration tree src/text1b.c:19 is
`INCLUDE_ASM("asm/funcs", func_80047EE8);` - the pre-migration on-main body (2 register-asm pins
$16/$18, one INLINE_MOVE_ALIASING `__asm__ volatile("move %0, %1")`, `s32 unused_slack[8]` +
`(void)unused_slack;`) is GONE from main. The ledger's floor 10 re-measures unchanged on the
banked candidate chassis.

[s12] THE 2026-07-22 REFUSAL'S GATE (b) IS SUPERSEDED. That ruling (decisions.md:1298) refused
this function because no SOTN-master precedent existed for an unwritten local acting as a phantom
frame carrier. The owner's 2026-08-18 ruling establishes exactly that construct as a GENERAL
family - `.claude/rules/no-new-park-categories.md:390` "Phantom-frame-slot volatile pad local ...
Supersedes the per-function 2026-08-17/18 leading/trailing-pad carve-outs with a general family."
SOTN-master exhibits: `volatile u32 pad; // !FAKE:` src/st/sel/2C048.c:564
(docs/reference/sotn-construct-index.md:101) and `volatile u32 pad[4]; // FAKE` src/st/sel/stream.c:80
(index:103). In-repo landed applications: src/code6cac.c:1491, src/code6cac.c:1570,
src/code6cac_c2.c:856. The unpark brief (memory/grind/func_80047EE8/brief-2026-08-18.md) named
this function among the 11 consequent unparks.

[s12] THE COMPOSITE HAD NEVER BEEN COMPILED. Every volatile pad ever measured ON THIS FUNCTION was
an 8-byte SCALAR found by permuter (s4/s5, rejected/permuter-volatile-*.c) - the wrong size, and
correctly rejected. The 32-byte ARRAY form that the size-pin probe [s7] identifies (int[7] or
int[8] -> vars=32) had only ever been compiled OUTSIDE the function as the s6 positive control
(ctlB, non-volatile). s12 compiled the sanctioned spelling in situ for the first time:
`volatile u32 pre_pad[8];` as the first declaration of the banked floor-10 chassis.

[s12] RESULT - FULL DRIVER BUILD SHA1 == ORACLE. With the composite body applied in place of the
INCLUDE_ASM line, `& tools/wteng.ps1 main build` produced
`sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa` == `want ... MATCH`
(tmp/grind/func_80047EE8/s12/build_oracle.log). The built object's prologue is
`addiu sp,sp,-72` - target's frame exactly, vs the chassis's former -0x28
(tmp/grind/func_80047EE8/s12/built_func.txt, 53 instructions). The volatile array DOES reach
vars=32 (Judge-risk flag 2 of the unpark brief - "an inference, not a measurement" - is now a
MEASUREMENT, and it is positive). The 10-instruction frame-offset residual the ledger carried
since s1 is CLOSED. func_80047EE8 is byte-matched by pure C plus two annotated,
sanctioned-family FAKE constructs.

[s12] WHY THE SANDBOX STILL PRINTS 10. `sandbox func_80047EE8 --disable all` with the composite in
place: score 10, 53/53 insns, rules_dropped 0, cheat_asm_stripped 279
(tmp/grind/func_80047EE8/s12/sandbox_composite.log). The sandbox strips cheat-asm
UNCONDITIONALLY, and a sanctioned unwritten pad is only exempted for functions listed in
`engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS` (today func_8001E404, func_8001E6E4,
func_8003CF84 - verified by reading engine/volatile_cheats.py:746-772). func_80047EE8 has no row,
so the honest floor reads pad-stripped. The 2026-08-18 ruling itself mandates that row; adding it
is the prescribed integration step, but engine/ is outside a grind session's allowed surface.
Identical posture to the sibling func_80047FBC handoff (decisions.md 2026-08-20 entry).

[s12] SRC STATE AT SESSION END. src/text1b.c was reverted to
`INCLUDE_ASM("asm/funcs", func_80047EE8);` (asm-until-matched, 2026-08-19): committing the
composite before the allowlist row exists would put a body on main that the engine still scores as
a cheat carrier and that `queue done` would refuse. The proven body is banked verbatim at
memory/grind/func_80047EE8/candidate.c with a full header; self-vet at
memory/grind/func_80047EE8/self_vet.md.

[s12-rerun] BYTE PROOF INDEPENDENTLY REPRODUCED. The first s12 filing was DISCARDED by the driver
(decisions.md:8761) purely on a heading-token technicality - its entry was titled INTEGRATION
HANDOFF and carried no OWNER-ESCALATION token, so the validator did not see an escalation naming
this function. The re-run session re-derived the proof from scratch rather than inheriting it:
applied the candidate body over src/text1b.c:19 via tmp/grind/func_80047EE8/apply.py, ran
`wteng main build` -> sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH
(tmp/grind/func_80047EE8/s12/build_oracle_s12b.log); re-ran `sandbox --disable all` on the same
tree -> score 10, target_insns 53, build_insns 53, cheat_asm_stripped 279
(tmp/grind/func_80047EE8/s12/sandbox_composite_s12b.log); measured 0 regfix/asmfix rows naming
func_80047EE8; re-read .claude/rules/no-new-park-categories.md:390-402,
docs/reference/sotn-construct-index.md:101/:103 and engine/volatile_cheats.py:746-772 and confirmed
every citation in the self-vet resolves in the live tree. The 2026-08-18 family text itself says
verbatim that the engine allowlist "requires it and a per-function row" - i.e. the missing row is
prescribed by the ruling, not a workaround. src/text1b.c reverted to
INCLUDE_ASM("asm/funcs", func_80047EE8) at exit. Re-filed as decisions.md:8765
(**OWNER-ESCALATION**, bytes proven, gate (b) passes).

- [s12] Re-verified THIS session (not inherited): with memory/grind/func_80047EE8/candidate.c applied over src/text1b.c:19, `& tools/wteng.ps1 main build` produced sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH — the function is byte-matched by that C in a real build (tmp/grind/func_80047EE8/s12/build_oracle_s12b.log).

- [s12] On the identical tree, `sandbox func_80047EE8 --disable all` prints score 10 with target_insns 53 and build_insns 53 — the residual is entirely the sandbox stripping the sanctioned unwritten pad, not a byte gap (tmp/grind/func_80047EE8/s12/sandbox_composite_s12b.log).

- [s12] Measured, not asserted: 0 rows in regfix.txt + asmfix.txt name func_80047EE8; the function is not in inline_asm_canonical.txt; the candidate body contains no register-asm pin, no __asm__ of any kind, no alias rename and no (void) discard shim. The pre-migration on-main form carried 2 register-asm pins, 1 INLINE_MOVE_ALIASING __asm__ block, an unqualified s32 unused_slack[8] and a (void)unused_slack; shim — the cheat inventory strictly decreases.

- [s12] engine/volatile_cheats.py:746-772 read directly: _SANCTIONED_UNWRITTEN_PADS holds func_8001E404, func_8001E6E4 and func_8003CF84 only, and _is_sanctioned_pad gates on exact function + exact name + exact element count + volatile. engine/volatile_cheats.py:745 reserves allowlist extension to a fresh owner ruling.

- [s12] .claude/rules/no-new-park-categories.md:390-402 (owner ruling 2026-08-18, 'Phantom-frame-slot volatile pad local') states verbatim that it 'Supersedes the per-function 2026-08-17/18 leading/trailing-pad carve-outs with a general family' and that applications use the ARRAY form because 'the engine allowlist engine/volatile_cheats.py _SANCTIONED_UNWRITTEN_PADS requires it and a per-function row'. Every FORM CONSTRAINT is met by the candidate (array form, first-decl position, volatile, /* FAKE: */ annotation, no (void)pad shim).

- [s12] AND-gate (a) FAILS: scan_hand_coded tier LOW (1/8) at s10 — this is ordinary compiled C. AND-gate (b) PASSES with cited precedent (docs/reference/sotn-construct-index.md:101 and :103 SOTN-master PSX exhibits; in-repo landed applications src/code6cac.c:1491, src/code6cac.c:1570, src/code6cac_c2.c:856). Because a gate genuinely passes, this is a live OWNER-ESCALATION, not the 2026-07-27 terminal refusal.

- [s12] The 2026-07-22 REFUSED / OWNER-ACCEPTED INCOMPLETE disposition (decisions.md:1298) is SUPERSEDED — its sole ground (no SOTN precedent for the closing construct) is falsified. Future sessions must not quote floor 10, the endgame-lock species, or that refusal as live for this function.

- [s12] src/text1b.c was reverted to INCLUDE_ASM("asm/funcs", func_80047EE8); at session exit (asm-until-matched, 2026-08-19); the proven body lives only in memory/grind/func_80047EE8/candidate.c with self-vet at memory/grind/func_80047EE8/self_vet.md.

- [s12] Process lesson banked in hypotheses.md: an escalation entry for this function MUST carry the literal token OWNER-ESCALATION (or CANONICAL-ASM GRANT PATH) in its '## ' heading, or the driver discards the session regardless of merit — the identical discard/re-file cycle happened to the sibling func_80047FBC (decisions.md:7526 then :7530).

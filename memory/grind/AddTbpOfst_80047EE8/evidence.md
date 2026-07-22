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

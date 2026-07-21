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

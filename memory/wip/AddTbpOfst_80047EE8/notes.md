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

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

## [s1] A single-statement body `*(T *)0x1F800400 = 0;` (volatile vu32 or plain s32 pointer form) reaches the target's $t9 register seat for the scratchpad address pseudo.
- mechanism: global.c register allocation order: lowest-numbered free scratch register is preferred absent competing live pseudos.
- probe: .greg dump for func_80052C10 (tmp/grind/func_80052C10/s1/3AB48.greg.txt) on the vu32 candidate body; confirmed only one pseudo (reg 72, zero conflicts) exists, allocated to $v0.
- result: Both the vu32 and plain s32 single-statement forms allocate the address pseudo to $v0 (lowest free scratch reg), never $t9. Reaching $t9 would need ~13 other simultaneously-live scratch pseudos occupying $v0..$t8 first, which no caller or sibling evidence suggests ever existed in this body.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/main/3AB48.c func_80052C10, single-statement chassis `*(vu32 *)0x1F800400 = 0;` and `*(s32 *)0x1F800400 = 0;`, no FAKE construct present, no register pin used

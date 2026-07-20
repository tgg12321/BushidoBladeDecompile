# Hypothesis ledger — mario_getMarioVoiceData_8005BE84

## Frontier from s1 recon (2026-07-20)

### H1 — Force live-across-call shifted value (block-scoped s32 doubled)
**Statement:** Declaring `s32 doubled = arg0 << 1;` in the OUTER block (before
the if) and USING it inside the if AFTER the last call chain (as the source
of the +1) may force GCC to keep the shift result live across the call
sequence — matching target's s0=arg0<<1 preservation.

**Mechanism:** RA won't coalesce `doubled` into s0-temp if `doubled` has a
use that spans callee-save boundaries. The shifted value crosses func_80085F98,
_EE4, _E4C, _EE4, md_game_check — all callee-preserving — so `doubled` must
live in a callee-save reg (s0 natural). Then `temp_a0 = doubled + 1;` at
the read site emits `addiu <fresh>, s0, 1` — matching target.

**Probe:** Write the variant, sandbox --disable all, check whether delay
slot flips to sll AND +1 lands in a0.

**Result:** Not yet run. WIP notes cite "shift arg0 in place + temp_a0 late"
(variants e/i/j) at 15-17 — but those may have done in-place `arg_save <<= 1`
INSIDE the if (making shift+read same block, breaking the base-first addu
order). H1 keeps the shift-compute OUTSIDE the if (before the base compute
is unchanged), differing from the rejected variants.

### H2 — Shift-between-lh-and-bltz (hoist shift to just before bltz)
**Statement:** Insert a computation whose result feeds the bltz delay slot as
sll rather than addi — e.g., separate the lh from the bltz with an
intermediate that forces reorg to defer the shift.

**Mechanism:** reorg.c's fill_slots_from_thread walks candidates backward
from the branch. If the sequence just before bltz has ONLY an sll (no addi
candidate), it picks sll. Restructure so addi is NOT a candidate at bltz
fill time (e.g., addi lives in the fallthrough block only).

**Probe:** Try `s16 v = *p; if ((arg0 <<= 1, v) >= 0) { ...; temp = arg0+1; ... }`
or comma-expr variants. Check whether GCC parses/schedules distinctly from
the rejected e/i/j forms.

### H3 — Union/cast tricks to break coalesce without add of live use
**Statement:** Introduce a decoy use of the un-shifted arg0 (e.g., store it
to a debug sink, pass as unused arg) so RA keeps arg0-as-s0 across the call,
and the shift lands in a different reg. Then find a way to make the shift
happen in the delay slot.

**Mechanism:** Making arg0 live-out from the if would fight the shift-into-s0
target. Actually the target USES s0-as-shifted, so decoy-live arg0 would
displace target's allocation. **Verdict a priori:** likely wrong direction —
target's s0 IS the shifted value, not arg0. Probably KILL without probing;
listed here so the next session doesn't waste a hypothesis on it.

## Constraints (BINDING)
- No register asm() pins ($16, $3, or any). Cheat.
- No `__asm__` (hardcoded-$N or otherwise). Cheat.
- No `volatile` coercion on D_8009AD1C. Cheat.
- No dead stores / `/* FAKE */` unless documented lever-exhaustion + layer-2.
- No modification of function ordering / signatures / callee decls.

## [s1] Baseline floor from WIP candidate.c reproduces at 4 under s1 conditions (git HEAD 38921159).
- mechanism: canonical=C d=4; sandbox --disable all score=4 (46 build / 46 target, scorable=true, rules_dropped=0).
- probe: Applied candidate.c to src/text1b.c; ran wteng canonical + sandbox --disable all.
- result: canonical distance=4; sandbox score=4.
- verdict: CONFIRMED

## [s1] The 4 residual diffs are all CONTROL-FLOW class; substantively they are (a) `sll s0,s0,1` hoisted early (should be in bltz delay slot) and (b) `addiu +1` folded into s0 (should be a late addiu into fresh a0).
- mechanism: objdump comparison: build has `sll s0,s0,1` at pre-lh position and `addiu s0,s0,1` in bltz delay slot; target has empty pre-lh and `sll s0,s0,1` in delay slot with `addiu a0,s0,1` LATE after md_game_check. Both shifts are for `(arg0<<1)+1` computation.
- probe: wsl objdump -d on tmp/sandbox/.../text1b.o vs asm/funcs/mario_getMarioVoiceData_8005BE84.s.
- result: Diff localized to one 4-insn window (bltz+delay ± addi/sll pair).
- verdict: CONFIRMED

## [s1] No sibling function in the codebase shares this shape (mario_getMarioVoiceData_8001B3C0/8001C820/80048AD0 are structurally unrelated; find_duplicates --threshold 0.65 returns nothing for 8005BE84).
- mechanism: Opcode-class similarity over 1419 asm functions; length-band filter.
- probe: python3 tools/find_duplicates.py --threshold 0.65 --limit 2000, then grep 8005BE84.
- result: No hits.
- verdict: CONFIRMED

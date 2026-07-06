# func_8003DBE4 — WIP (fleet-bw1, 2026-07-06)

## TL;DR — POLICY-GRADE unused-frame gap, NOT a pure-C grind

This function was previously COMPLETED-C (commit `9e722985`, 2026-05-28) using
`s32 buf[2];` (unreferenced dead array) to fabricate the target's extra 8 bytes
of stack frame. The 2026-05-31 `dead-vars-local-array` ban made that construct
FORBIDDEN, which re-routed the function back into the active queue at honest
sandbox-distance 21 — even though main's actual build STILL byte-matches the
oracle today (the array still works mechanically; it's now a policy violation,
not a compile failure).

**Proven this session:** the entire 21-instruction sandbox distance is 100%
attributable to the 8-byte frame gap (target frame 0x40, clean-C frame 0x38).
Every other instruction in the function is byte-identical. Confirmed via:
- Manual `objdump -d --disassemble=func_8003DBE4` diff of target
  (`build/src/code6cac_c2.o`, pristine oracle-matched reference) vs a
  freshly-rebuilt clean candidate — only the stack offsets and
  branch/jump targets differ, shifted uniformly by 8/0x20 bytes; every opcode
  otherwise matches 1:1.
- `objdump` shows **zero** `sw`/`lw` instructions anywhere in the target
  touching stack offsets 0x10-0x17 (the missing 8 bytes) — only the
  callee-saved register spill area (0x18-0x38) and the incoming 5th-argument
  stack read are ever accessed.
- `m2c --valid-syntax -f func_8003DBE4 asm/funcs/func_8003DBE4.s` independently
  reconstructs the function with **zero** stack-resident locals — every value
  lives in a register in m2c's own decompilation.
- Removing `buf[2]` from the committed src and re-running
  `sandbox --disable all` produces the exact same score (21) as leaving it in
  — proving the array plays no role in the honest cheat-invisible measurement
  and this floor is genuine, not sandbox-blind to the cheat.

**Conclusion: no real local variable can explain the 8 bytes** (a genuine
spill/stack-resident local would leave a load/store instruction at that
offset in the target, and none exists). The only C constructs that produce an
unused, unaccessed frame region are "declared but never referenced" locals —
exactly what `dead-vars-local-array.md` forbids for the unwritten/unread case.
The narrow 2026-07-01 carve-out (written-never-read arrays) does not apply:
this target has no dead stores anywhere in the gap to justify it.

## Identical precedent already escalated

This is the SAME shape as `AddTbpOfst_80047EE8` / `InitHiraRmd_80047FBC`
(`memory/wip/AddTbpOfst_80047EE8`, commit `3d7e222d`, 2026-06-14 — a 32-byte
unused-frame gap, escalated as a PROJECT-WIDE POLICY decision, not
closable by single-function grind). `func_8003DBE4` is a second, independent
data point for that same evidence request (8-byte gap here vs 32-byte there),
reached completely independently via a different function.

## Levers tried / ruled out this session

- Register-alloc levers (A/B/C from `register-alloc-pure-c.md`): N/A — there
  is no register-allocation discrepancy anywhere in this function; every
  opcode besides the frame-shift cascade already matches.
- Dead-scalar / constant-holder (`named-local-fake-exception.md`): explicitly
  excluded — "anything whose mechanism is 'reserve frame bytes' stays
  forbidden pending its own evidence request."
- Written-never-read array carve-out (`dead-vars-local-array.md` 2026-07-01):
  does not apply — target has zero dead stores in the gap region.
- Searched for a genuine dropped local (extra call needing more outgoing-arg
  stack space, extra saved register, alignment quirk): none found; the
  target's callee-saved register set (s0-s7, ra) is identical to the clean
  build's, and no call in the function needs more than the standard 16-byte
  outgoing-argument area.

## Recommendation

Group with the `AddTbpOfst_80047EE8` / `InitHiraRmd_80047FBC` cluster for
whatever policy resolution the owner reaches on the unused-frame-gap evidence
request. Until then this stays INCOMPLETE (queue-active); do not re-add
`s32 buf[2]` as a "fix" — it's the same forbidden construct that got this
function re-routed to the queue in the first place.

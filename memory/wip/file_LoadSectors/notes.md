# file_LoadSectors — BLOCKED (8-byte frame-size gap is the ONLY diff)

## TL;DR
The function BODY matches the target byte-for-byte (insns 14-41 identical). The
ONLY 14 diffs are a uniform **8-byte stack-frame-size gap**: target frame = 48
(`addiu sp,sp,-48`, saves at sp+24..44), ours = 40 (saves at sp+16..36). Every
diff is just the +8 offset shift on the prologue/epilogue save/restore.

## Key finding: HEAD's `_pad[2]` is a VESTIGIAL no-op cheat
HEAD declares `s32 _pad[2]` (a frame-padding cheat by name + unused). It is
DCE'd by GCC -- raising it to `_pad[4]` does NOT change the frame (still 40), so
the array contributes nothing. The natural frame (16-byte arg-save area + 6
saved regs s0-s4/ra = 24, => 40) is 8 bytes short of target's 48.

## Why blocked (not closable by a worker)
To add 8 bytes of frame you need a stack local that survives DCE -- i.e. one
that is USED, address-taken, or volatile. But:
- the body ALREADY matches perfectly, so any USED local would change the body
  (regression);
- an unused / address-taken / volatile 8-byte array is a forbidden
  frame-coercion cheat (dead-vars-local-array, [[register-alloc-pure-c]] Lever D).
The function's logic (open/seek/read-loop/close) genuinely needs NO stack local.
So the target's extra 8 bytes is either an original unused local of unknown
content, or a **cc1-vs-cc1psx frame-sizing divergence** (our open-source cc1
computes a frame 8 bytes smaller than PsyQ cc1psx for this exact call pattern).

## Avenues for next session
- **cc1psx calibration**: compile this function with PsyQ cc1psx (tools/cc1psx);
  if cc1psx produces the 48-byte frame from the SAME clean source, this is a
  confirmed compiler-frame divergence -> escalate (not pure-C-closable), do NOT
  add a `_pad`.
- sibling ings.c loaders may reveal a real original local (then it's a named,
  used local, not a cheat).

## Floor
- HEAD distance 14, carrying a vestigial DCE'd `_pad[2]` cheat. Body matches;
  only the 8-byte frame size differs. No non-cheat pure-C closes the frame gap.

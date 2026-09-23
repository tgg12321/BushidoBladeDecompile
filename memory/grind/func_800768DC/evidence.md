# func_800768DC — evidence (manual session 2026-09-23)

Score trail (sandbox --disable all, 294 target insns):
- v1 raw-offset pointers, one `st` local reused: 132
- v2 sibling-style `(arg3 * 2) + (s32)D_800A36A0` block locals + `row = arg3*10 + base`: 104 (frame vars=32 vs target 16)
- v3 struct typedef `SelWork_800768DC` with per-player [2] arrays, single `w` local: 46 — frame vars=16 exact
- v7 struct + macro accessor, `w` only where the global is held across stores: 8
- x1 = v7 with the sorted-insert compare spelled `slots[i] > pick` (was `pick < slots[i]`): **0**
- y5 = no pointer locals at all, every access via the macro: **0** (landed form)

Full build with y5 spliced: SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (2026-09-23).

Mechanisms measured:
- Phantom frame bytes: each s16 local loaded straight from s16 memory into an HImode
  pseudo that then feeds a narrowed (shorten_compare) HImode loop compare cost 8 bytes
  (`use (mem:SI sp+N)` reload insns in .greg). The struct array-ref form produces
  exactly the target's 16.
- A single pointer local gets ONE hard reg for the whole function; the target uses a
  different reg per region (v0/a1/v1/a0), i.e. fresh expressions of the global, not a
  long-lived local.
- Per-loop separate s16 vars (a/b/c/d) scored 122..157 vs the shared `i`/`j` at 104.

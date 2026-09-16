# Hypotheses — func_8006A564

## H1 (session 1) — srl vs sra on the packed-color halving ops
**Statement:** casting the halved color value through `(u32)` before `>>1`
makes GCC emit `srl` instead of `sra`, matching target at both halving
sites.
**Mechanism:** GCC's shift-right codegen picks `sra`/`srl` based on the
shifted operand's signedness at the RTL level; `s32 v0` defaults to
arithmetic shift, `(u32)v0` forces logical.
**Probe:** objdump `tmp/sandbox/func_8006A564/text1b.o` before/after the
cast; grep for `sra`/`srl` at the two halving sites.
**Result:** CONFIRMED — both sites now emit `srl s0,s0,0x1` byte-identical
to `asm/funcs/func_8006A564.s:101` and `:148-149`. Sandbox masked score
unchanged at 137 (this diff was not the dominant residual — see H2).

## H2 (session 1, frontier — NOT yet probed) — register-allocation mapping
**Statement:** the +5 instruction / 137-point residual is entirely a
register-allocation mapping difference: our build's callee-save set is
{s0,s1,s2,s3,s4,s5} mapped {arg1,scratch,arg1... } vs target's
{s0,s1,s2,s3,s4} mapped {scratch,arg1,arg0,arg2,flag-local}. Fixing the
source structure (declaration order of `tile`/`obj2`/`v0`/`s4`, or
parameter-liveness shape) so GCC's global allocator assigns the same 5
registers (dropping our extra 6th) should close most or all of the
137-point gap.
**Mechanism (untested):** GCC 2.7.2's `global.c` allocno-priority ordering
is sensitive to first-use order and live-range length of each pseudo;
right now `tile` is reused/reassigned many times (long live range,
crosses all 3 tile-draw blocks + the final section) which may be pushing
it to a scratch register with a longer conflict list, forcing one extra
value to spill to a callee-save slot instead of reusing a temp already
freed by DCE.
**Next probe:** `pwsh tools/grinder/dump.ps1 func_8006A564` then read the
`.greg`/`.lreg` dump to see which pseudo lands in the extra register (s5
equivalent) and why it doesn't get folded into an already-freed hard reg;
try (a) declaring `s4`-equivalent local FIRST (before `tile`/`obj2`/`v0`)
to bias LUID, (b) splitting `tile`'s live range at the `obj2 = ...` /
`tile = *(obj2+0x1C)` reassignment into a distinctly-named local (the
asm literally reuses $s0 for two logically different pointers — a fresh
C name at that reassignment point may look "more natural" to the
allocator AND is arguably the more truthful spelling: it's a different
object). Do NOT try register-asm pins or scheduling barriers — plain
declaration-order / naming levers only, per the register-alloc-pure-c
technique family.

## Frontier for next session
1. **H2 above** — read the `.greg` dump, try declaration-order + the
   `tile`-reassignment split-name lever, re-measure via sandbox.
2. If H2's easy levers don't close it, run the permuter on
   `asm/funcs/func_8006A564.s` (clean single-function target per
   `difficult-is-not-impossible.md` §3) directed at the register-mapping
   diff specifically (`--stop-on-zero`).
3. Re-check the SetDrawMode/AddPrim tail (`s4` in this C == the value fed
   as `func_8006E480`'s 2nd arg) — confirm its liveness/placement matches
   target's `$s4` exactly once H2's register remap is applied; it may
   already be correct and just riding along with the s5-vs-s4 shift.

## [s1] Casting the halved packed-color value through (u32) before >>1 makes GCC emit srl instead of sra, matching target at both color-halving sites (block 3 and the final color-select block).
- mechanism: GCC 2.7.2 picks arithmetic vs logical right-shift codegen from the shifted operand's C signedness; s32 defaults to sra, (u32) forces srl.
- probe: objdump tmp/sandbox/func_8006A564/text1b.o before/after the cast; diff against asm/funcs/func_8006A564.s:101,105,148,149.
- result: Both halving sites now emit `srl s0,s0,0x1` byte-identical to target. Sandbox masked score unchanged at 137 (this was a real but non-dominant instruction-level fix; the register-allocation gap in H2 dominates the residual).
- verdict: CONFIRMED

# func_8007636C hypotheses

## 2026-09-24 manual session (closed 184 -> 0)

Codex s1 (2026-09-24): m2c scalars KILLED (248); descriptor aggregate
CONFIRMED (184).

- CONFIRMED: descriptor = S_80074488 (shared with func_80074488); work area =
  the existing SelWork_800768DC view (+ u16 f34 at 0x34). 151.
- CONFIRMED: `q = s.sp18 + 0x24` captured BEFORE the highlight test and reused
  for both sp1C stores; highlight offset `s.sp18 + 12 + arg3 * 12`
  (`(arg3 + 1) * 12` 109, `(arg3 * 12 + 12)` 65, `+= 12 + arg3*12` 17 — only
  `s.sp18 = s.sp18 + 12 + arg3 * 12` hoists arg3*12+12 as one invariant).
- CONFIRMED: values carried in s.sp18 itself (a separate `p` local seats p in
  one register for all sites; the target uses a different register per site).
  21 -> 17.
- CONFIRMED: `s.sp1C += *(u8 *)(s.sp18 + 2) * 16` (x8 in pass 3) directly after
  the first sp1C store. 54 -> 24.
- CONFIRMED: second pass selector arms ordered f7E-first
  (`f3C != i || f14 >= 4`). 136 -> 63 (with the q hoist).

### Last 3 points: the pass-1 loop guard (`move t0,zero; slt; beqz`)

Mechanism (instrumented cc1 RTL dumps, tmp/e636/dump/): cse turns the rotated
guard's `(s16)i` into a pseudo = 0 and keeps it (slt_si needs a register
operand); with a literal 0 nothing else is equivalent, so COMBINE later folds
`slt` + `beqz` into `beqz x` (x = u8 + 3 is known non-negative). The target
kept the `slt` with a rematerialized zero register, i.e. cse substituted a
DIFFERENT, long-lived SImode register known to be 0.

Measured-negative (all 3/348 unless noted):
- literal 0 at both func_8006E480 calls (the natural form): 3.
- pass-1 guard spellings: `i = 0` before `s.sp40 = 0`, before/after the rsin
  call, `for (; i < ...)`, `while (i < ...) { ...; i++; }`, color folded into
  the chain assignment (g1-g4): 3 each.
- zero variable used at ONE site only (arg3 test, either root read, the
  s.sp40 store, the last loop's init): 3 each (single use -> cse propagates the
  constant and the variable disappears).
- the variable must be live past pass 1 (uses only before/inside pass 1 = 3).
- permuter (tmp/perm_636, 4121 iters): only score-0 find was an unmotivated
  `new_var = 0` threaded through 7 unrelated sites — REJECTED (cheat-form).

Adopted: `s32 mode = 0;` passed as func_8006E480's second argument at both
call sites (set once, live past pass 1) -> 0/348. FAKE-annotated
constant-holder per named-local-fake-exception; evidence that the original
had such a holder: the case-2 sibling func_800759D0 keeps this argument's
zero in $fp across its three func_8006E480 calls (asm lines 20/56/334/356).

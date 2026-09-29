# D_800A35C8 / D_800A35CA timer stores — receipts for the pointer alias (2026-09-29, manual s2)

The target's cancel block stores both timers through two direct gp_rel stores:
`li v0,0xF; sh v0,%gp_rel(D_800A35C8); li v0,0x14; sh v0,%gp_rel(D_800A35CA)`
(asm/funcs/func_800720FC.s:651-654). These are inside the player loop, which
contains a call (func_8005C650).

## Single declaration kept

src/text1b.c keeps its single declaration `extern s16 D_800A35C8[];`, which
func_8006F100 indexes as `D_800A35C8[i]`. The landing reaches the target
through a local pointer, `timer = D_800A35C8; timer[0] = 0xF; timer[1] = 0x14;`.
That is a C-level pointer alias under pointer-alias-fake-exception.md: FAKE
annotation, named mechanism, exhaustion below, layer-2. No TU split and no
per-file declaration are needed. The split and Q21 route of the first submission
is withdrawn; see ../split/tu_boundary.md, "Superseded".

## Mechanism (cc1 dumps: tmp/func_800720FC/a3d/*.cse/.loop; excerpts below)

- **Array element spellings.** Every array-element store with a constant index
  gets its address from explow.c memory_address. That function force_reg's a
  constant address while cse is still expected (explow.c:398-399), reached via
  change_address for an ARRAY_REF or COMPONENT_REF (emit-rtl.c:1315).
  - The second address, `(const (plus D_800A35C8 2))`, is then rewritten by
    cse.c use_related_value (cse.c:1781, called at cse.c:6531-6535) as
    `(plus base 2)`. This happens because -G0 prices the symbol at 2 insns
    (mips.h CONST_COSTS, SYMBOL_REF_FLAG unset) against 1 for reg+offset.
  - The base pseudo is now used twice. loop.c's large-loop single-usage rule,
    which would put a once-used address back into its MEM (loop.c:721-765),
    therefore does not apply.
  - move_movables hoists the base (loop.c:1623-1632: threshold 29 x savings x
    life 4 >= 42 insns). The stores become `sh v0,0(sN)` / `2(sN)` off a
    callee-saved base register set before the loop.
- **Pointer spelling.** Through the local, `timer[1]` is `*(timer + 1)`. cse
  folds `timer` to the constant and leaves `(mem (const (plus sym 2)))` as a
  constant address, which cse never re-registers (the constant-address early
  return in find_best_addr).
  - The `timer` pseudo then has one use left, `timer[0]`. loop.c's single-usage
    rule substitutes the symbol into that MEM and deletes the set.
  - Result: two direct stores and no hoisted base. The a3d .cse excerpt shows
    insn 1618 `(set (mem/s:HI (const:SI (plus:SI (symbol_ref "D_800A35C8")
    (const_int 2)))) ...)` beside insn 1613 `(set (mem:HI (reg/v:SI 532)) ...)`.

## Exhaustion — every spelling measured (timers/out/*.json + cc1 / cc1psx listings)

Scores are engine distance for func_800720FC. The body is timers/landing_body_scalars.c
with only the timer lines changed. The TU is the landing file as tusplit.py builds it,
with D_800A3578 retyped s16. "la" means a base register was hoisted before the loop.

| spelling | declaration | cc1 | cc1psx |
|---|---|---|---|
| `D_800A35C8[0] = 0xF; D_800A35C8[1] = 0x14;` | `s16 []` | 4, la | la |
| `D_800A35C8[1] = 0x14; D_800A35C8[0] = 0xF;` | `s16 []` | 6, la | la |
| `*D_800A35C8 = 0xF; *(D_800A35C8 + 1) = 0x14;` | `s16 []` | 4, la | la |
| `[0]`, `[1]` | `s16 [2]` (sized) | 4, la | la |
| `.p1 = 0xF; .p2 = 0x14;` | `struct {s16 p1, p2;}` | 4, la | la |
| `.p2 = 0x14; .p1 = 0xF;` | struct | 6, la | la |
| `timer = D_800A35C8; timer[0]; timer[1]` | `s16 []` | 1*, direct | direct |
| `D_800A35C8 = 0xF; D_800A35CA = 0x14;` | two scalars (second handle) | 0, direct | direct |

\* The 1 is a scorer artifact. Our maspsx emits `%gp_rel(D_800A35C8+2)` where the
target names `%gp_rel(D_800A35CA)`: the same address. engine/score.py resolves named
HI16/LO16 pairs but not GPREL16. Spliced into src/text1b.c with the s16 retype, the
full build gives SHA1 == oracle, and sandbox scores 0 against the rebuilt reference
(2026-09-29).

Earlier in-tree measurements agree (evidence.md s2):
- store-order swaps: 6-7;
- the second-scalar probe probes/alias_D_800A35CA_sandbox0.c: 0, but it adds a
  second handle for the bytes and is rejected;
- cc1psx on the array form hoists (tmp/cc1psx/func_800720FC).

The two compilers agree on every spelling. func_8006F100 under the other
declarations was measured too:
- two scalars, pointer selector: 36;
- struct, pointer selector: 35;
- committed array body: 0 (timers/out/pre_*.json).

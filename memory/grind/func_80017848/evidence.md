# Evidence bank — func_80017848

## s1 (recon, 2026-08-18)
- Baseline re-confirmed: canonical = C (distance 16 / 127 insns); sandbox --disable all = 16,
  125/127 build insns, frame 0x40 both sides. The inherited ledger below is accurate as-is.
- Target preheader detail not previously recorded: the a3 copy is `addu a3,a0,zero` — a copy
  of the SLOTS POINTER (not the shift). My build's `move a3,a1` copies the shift instead.
  The loop base is then `addu a0,a1,a3` and the count reload goes through it
  (`lw v0,0x1C(a0)` loop 1 / `lw v0,0x20(a0)` loop 2). Guard count load in target is
  `lw v0,0x1C(v0)` — dest clobbers the address reg, so the count value cannot survive to
  the loop (structurally forced reload, matches the inherited root-cause analysis).
- GCC 2.7.2 loop.c read (tools/gcc-2.7.2/loop.c:2760ff): plain MEM loads ARE hoistable in a
  store-free loop (no RTX_UNCHANGING_P requirement; only volatile / unknown_address_altered /
  store-dependence reject), BUT the bound load sits after the conditional `beq` exit, i.e.
  conditionally executed (maybe_never), so loop.c will not move it in EITHER build. Therefore
  my build's count-caching (`move a1,v0` + `slt v0,v1,a1`) comes from a cse-family fold over
  the rotated-while's duplicated exit test, NOT from LICM — which pass exactly is hypothesis
  H2's probe. jump.c duplicate_loop_exit_test confirmed present at jump.c:2163.
- No sibling/duplicate lead: tmp/duplicates_leads.txt has no entry for this function.
- Artifact: tmp/grind/func_80017848/s1/build_disasm_floor16.txt (current 125-insn build body).
- Frontier for next session: hypotheses.md s1 section (H1 guarded do-while with distinct
  guard/bound lvalues + slots-copy local; H2 -da pass forensics; H3 coupled single-base
  prediction).

- == imported from memory/wip notes.md ==
# func_80017848 (src/ings.c) — blob strike

Whole-body asmfix blob masks this function; the oracle is unaffected until the blob retires,
so improved-but-incomplete C is safe to leave. Do NOT edit asmfix.txt.

## Current state: sandbox `--disable all` = **16** (was 84). 125/127 insns. Zero rules, zero cheat-asm.
Everything outside the two scan loops is byte-exact: prologue/epilogue (frame 0x40), the two
top guards, the `math_Distance3D` call, the four link-record stores, and both count-append blocks.

## Score ladder
| state | score | frame |
|---|---|---|
| inherited (register-asm pins, cached slot pointers) | 84 | 0x30 |
| drop pins; inline `*(u8**)(ctx+0xC)` everywhere (the call kills the memory CSE) | 63 | 0x30 |
| guarded do-while + integer-offset addressing + cached `link` pointer | 56 | 0x30 |
| tail: cache the record pointer across the count store | 52 | 0x30 |
| `slots` local inside the loop guard body | 46 | 0x30 |
| **`while` instead of `do-while`** — the rotated-while guard is the phantom-slot producer | 33 | **0x40** |
| drop the redundant explicit `if` guard (it emitted a second `blez`) | 28 | 0x40 |
| `links` inline in the loop + index expression written **i-first** | **16** | 0x40 |

## Levers that moved it (mechanism)
- **Never cache `*(u8**)(ctx+0xC)` across the `math_Distance3D` call** — the call clobbers memory so
  GCC reloads it; only the pure `slot<<6` shift survives (target keeps it in s0/s1).
- **`ptr + int` is not symmetric.** C's `pointer_int_sum` always emits the pointer operand first, so a
  target `addu v0,v0,v1` (shift-first) can only be spelled as integer arithmetic:
  `*(s32 *)((slot_a << 6) + (s32)base + FIELD)`. The `math_Distance3D` args and the `link` record
  address are ptr-first in target, so those stay pointer arithmetic.
- **A store kills memory CSE**, so the four post-call stores need one cached `link` pointer and each
  count-append block needs its own cached record pointer, or `lw 0xC(s2)` is re-emitted after every `sw`.
- **`while` vs `do-while` decides the frame.** The 16 bytes of phantom locals (`vars= 16`,
  frame 0x40 vs 0x30) come *only* from the rotated-`while` guard — producer #1 in
  [[phantom-slot-frame-lever]]. No named-local spelling produced them (swept 5 candidates, all
  `vars= 0`); every `do-while` spelling gives frame 0x30. This was the single biggest lever.
- **Index expression must be i-first**: `*(u8 *)(i + (slot_a << 6) + (s32)slots + 0x24)`. Any
  base-first or pointer-arithmetic spelling costs ~13 points (it lets CSE collapse the loop's base
  into the top guard's `slots` register and drops the `lw a0,0xC(s2)` reload).

## Remaining residual (16 pts, ~8 differing insns per scan loop, both loops identical)
Target hoists `(slot_a<<6) + slots` into ONE preheader register (`move a3,a0; addu a0,a1,a3`) used by
both the body index (`addu v0,a0,v1`) and the bound load, and **re-loads the count every iteration**
(`lw v0,0x1C(a0); slt v0,v1,v0`). Mine keeps `off` and `slots` in separate registers, adds both inside
the loop (one extra `addu`), and caches the count in `a1` (`move a1,v0` in the preheader), saving the
two bound-reload insns. Root cause: in target the guard's count load is `lw v0,0x1C(v0)` — dest ==
address reg, so the count dies at the `blez` and the loop must reload; in mine the loop's bound is
CSE-equal to the guard's, so GCC keeps the value live instead. Every spelling that gives the loop its
own base register also makes the guard's count survive, so the two effects have not been separable.

## Killed levers (measured, all with the current surrounding code)
- Register-asm pins — anti-cheat, and score-inert (the sandbox strips them).
- Explicit `if (n > 0)` guard around the `while` — emits a redundant second `blez` (33 vs 28).
- Any `do-while` shape — loses the 16 phantom frame bytes (37–42).
- `rec` pointer local before the loop (19), assigned inside the loop body (21), or with the bound
  read through it (19/23).
- `links` as a local before the loop (28) or inside the loop body (24) — inline is best (16).
- Base-first / pointer / slots-first index spellings (29 each); `for` instead of `while` (16, tie).
- Named locals for the guard counts, the slot offsets, the link index, separate `i`/`j`, or named
  top-guard fields — none change `vars` (all stayed `vars= 0` pre-`while`).

## Instruments (kept in tmp/, regenerate if lost)
- `tmp/frameprobe.sh` — cpp | cc1 on src/ings.c, prints the `.frame`/`vars=` line. Separates
  "wrong frame" from "wrong codegen" far faster than the sandbox.
- `tmp/dis17848.sh` + `tmp/norm17848.py` — normalized target-vs-build instruction diff.
- `tmp/score_sweep.py` + `tmp/run_sweep.sh <variants.py>` — scores a dict of body variants
  (score + frame), restoring src/ings.c afterwards.


- [s1] sandbox --disable all = 16 this session (125/127 insns, frame 0x40 both sides); canonical verdict C distance 16

- [s1] target a3 copy is the SLOTS pointer (addu a3,a0,zero), not the shift; loop base addu a0,a1,a3 feeds both the single index addu v0,a0,v1 and the per-iteration count reload lw 0x1C/0x20(a0)

- [s1] target guard count load is lw v0,0x1C(v0) (dest==addr) so the count dies at the blez — the reload is structurally forced in target

- [s1] GCC 2.7.2 loop.c hoists plain MEM loads in store-free loops, but NOT conditionally-executed ones (bound load sits after the beq exit), so my build's count cache is a cse-family fold over the duplicated exit test, not LICM

- [s1] residual is 8 pts per scan loop, loops identical in shape; everything outside the loops is byte-exact per inherited ledger, re-confirmed by 125-insn disasm

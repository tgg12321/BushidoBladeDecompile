# camera_set_zoom — WIP checkpoint (2026-08-05, session 1)

**State: honest pure-C distance 48 → 3.** Instruction counts now identical (219/219).
Candidate source: `memory/wip/camera_set_zoom/candidate.diff` (apply to `src/code6cac.c`).
Tree is at HEAD — the candidate is NOT applied, because at distance 3 the 47 regfix
rules no longer repair the new codegen and the full build would break.

Canonical verdict: **C**. Zero cheat-asm. 47 regfix rules (39 subst, 5 `$A<->$B` swaps,
2 delete, 1 insert) all still keyed to the function.

## Levers that landed (each measured independently)

| # | change | score |
|---|---|---|
| 0 | HEAD | 48 |
| 1 | **split the call-crossing finals from the clamp temps** — new `dx_final`/`dz_final`; both arms assign them; the two `+=` at the tail read them | **19** |
| 2 | first `tmp` block: `tmp >>= 3; dst[0] += tmp;` instead of `dst[0] += tmp >> 3;` | 19→23→(with 3)→14 |
| 3 | second `tmp` block gets its **own variable** `tmp2` (keeps the `>> 3` inline form) | **14** |
| 4 | `dx_delta` split-init: `dx_delta = *(s32*)(arg0+0xB8); dx_delta -= *(s32*)(arg0+0xC8);` | **12** |
| 5 | else-arm gets its **own** `dx_shift` instead of reusing `dx_delta` | **8** |
| 6 | **`int camera_set_zoom` → `void`** (bare `return;` only) | **6** |
| 7 | hoist the `arg0+4` read into a local `id` before the `D_800A3769` store | **3** |

Lever 1 is the big one and confirms the catalog law *conflict edges = variable identity*:
target keeps the pre-clamp value in caller-saved regs (`$a0`, `$v0`) and only the
post-clamp finals in `$s3`/`$s2`; we reused one variable so it crossed the call and had
to be callee-saved, which cascaded into the whole `$s1`/`$s2`/`$s3` permutation that the
five `$17<->$18` swap rules were repairing.

Lever 6 is a **new, reusable finding**: an `int` return type on a function whose bodies
only do bare `return;` keeps `$v0` live-out, and GCC's dbr then refuses to fill branch
delay slots with `li $v0,K`. Two stray `nop`s disappeared and the instruction count
became exact the moment the type became `void`.

## Measured kills

- Naming the owner pointer (`u8 *owner = *(u8**)arg0;`) — no RTL change, score stayed 8.
- `s32 mode; mode = 2; *(...) = mode;` — constant-propagated away, score stayed 3.
- Applying `tmp >>= 3` to **both** tmp blocks — 19 → 23. Target genuinely uses different
  forms for the two blocks (block 1 single-register, block 2 the hoisted `sra v1,t0,0x3`
  in the branch delay slot).

## The residual 3 — fully quantified

```
T: lw v0,0(s0)      O: lw v1,0(s0)
T: li v1,2          O: li v0,2
T: sh v1,646(v0)    O: sh v0,646(v1)
```
Source: `*((s16 *) ((*((u8 **) arg0)) + 0x286)) = 2;`

Phase-5 `local_alloc` reproduces our side exactly (camera_set_zoom scores
**order 36/36 blocks, assign 51/52 qtys**). The block is **block 41**:

| ord | qty | first_reg | birth | death | refs | got | pri |
|---|---|---|---|---|---|---|---|
| 0 | 1 (the constant `2`) | 187 | 6 | 8 | 2 | `$v0` | 10000 |
| 1 | 0 (the owner pointer) | 186 | 4 | 8 | 2 | `$v1` | 5000 |

`qty_compare` priority is `floor_log2(refs)*refs*size/(death-birth)*10000`. Both have
refs=2, so the **shorter-lived constant outranks the pointer** and takes `$v0` first;
the pointer then overlaps it and falls to `$v1`.

**Spec for the fix:** the pointer must be allocated first. Two sufficient conditions:
1. the pointer's def is emitted **after** the constant's (const born 4 / pointer born 6
   ⇒ pointer pri 10000 vs const 5000), or
2. the pointer's refs ≥ 4 at span 4 (pri 20000).

Condition 1 is the natural one — it means the original C emitted the stored value into a
register **before** computing the destination address. Every C spelling tried so far for
that gets constant-folded, since GCC's `expand_assignment` always evaluates a MEM
destination's address first. Next avenues: a stored value GCC cannot fold to a literal
but which still compiles to a single `li` (i.e. born in this block, not an earlier one);
or a `qty_size`-based route — the hook does not yet print `qty_size`, which is the known
Phase-5 gap.

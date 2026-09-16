# SELF-VET — func_8006A564

Diff under review: `src/text1b.c`, the single `INCLUDE_ASM("asm/funcs",
func_8006A564);` line replaced by the pure-C body (plus the `extern s32
D_800A34F8;` declaration it needs). No other file is touched. Measured this
session: `sandbox func_8006A564 --disable all` = score 0, target_insns 199 ==
build_insns 199; full-tree `verify-oracle` re-linked to
`62efab4f73f992798c43e8c730aa43baa10bb4fa` == the oracle.

CONSTRUCTS: (1) named local intermediates `v0` / `v1` in the final block, each
written and read, each holding a value the target computes; (2) `v1` assigned
in two disjoint live ranges (once per record-fill group) inside one merged
block scope; (3) same-variable compound-assignment split `v1 = *(s32 *)(arg1 +
0); v1 += 0xC;` and `*(s32 *)(arg1 + 0x1C) += 0xF;`; (4) block-scope `{ s32
v0; ... }` declarations in blocks 1-3 (inherited unchanged from the s8
candidate); (5) plain pointer/offset loads and stores, `u32` casts for the
logical right shifts, and an `s32` cast on the `func_8007352C` argument.
NO inline asm, NO register variables, NO `volatile`, NO scheduling barriers,
NO dead stores, NO self-assigns, NO pads, NO unused locals, NO arrays, NO
`goto`, NO `do {} while (0)`, NO `/* FAKE */` annotations, NO alias renames,
NO casts whose only purpose is width coercion.

## T1 semantic purpose
Every construct changes the function's observable output.
- `v1` (both assignments): the value it holds is stored to `*(s32 *)(arg1 + 4)`
  in both groups. Delete the variable and the stores lose their operand. In
  cluster B it materializes as the target's own second register — the target
  bytes are literally `addiu v1,v0,12; sw v0,0(s1); sw v1,4(s1)`, i.e. the
  target itself keeps base and base+0xC in two registers. It is not
  byte-neutral filler; it is one of the two values the target computes.
- `v0`: read three times in cluster B (`*(arg1+0) = v0`, `v1 = v0 + 0xC`).
- The compound splits compute the stored values `base+0xC` and `+0xF`.
- The block-scope `v0` declarations in blocks 1-3 each carry the palette byte
  that is stored into `tile[4]/[5]/[6]`.
There is no construct in this diff whose removal leaves behavior identical.

## T2 human-programmer
Yes. The final block fills the same 3-field record twice in a row from two
adjacent source fields (`tile+0x28`, `tile+0x2C`). A human writes
`base = rec->a; limit = base + 12; out->p = base; out->q = limit;` and, for the
second fill immediately after, reuses the same pair of locals rather than
declaring a second pair. That is exactly what the matched sibling
`func_8006A1A0` in this same file does (`p1 = ptr[3]; tbl = p1 + 0xC;
s.sp18 = p1; s.sp1C = tbl;`). Nothing in the body invites a reader to ask
"why is this here?" — every statement stores a field the function must store.

## T3 GCC-internals justification
Honest answer, stated both ways. The SEARCH that found this spelling was
GCC-internal: session 9's forensic work read `birthing_insn_p`
(`tools/gcc-2.7.2/sched.c:2505-2536`, predicate `reg_n_sets[i] == 1`) and
`adjust_priority` (`sched.c:2584`) and predicted that giving the `+0xC` pseudo
a second real assignment in the function would remove the `LAUNCH_PRIORITY`
boost (`sched.c:187`, installed `sched.c:4049`) that was pulling the add past
the store in the backward scheduler. That prediction was then confirmed in the
dumps (`tmp/grind/func_8006A564/s9/f.sched` vs `.../s8/f.sched`).
But the CONSTRUCT does not need that story to be explained: "hold the record's
base and base+0xC in two locals and reuse the pair for the second fill" is a
complete program-logic account of the C, and it is the account a reader gets
from the code alone. The GCC internals explain how it was FOUND, not why it is
there. No construct in the diff exists only to steer a pass; the two values are
both stored, and both are in the target's bytes.

## T4 permuter/search provenance
Not permuter output. The form was derived by reading `sched.c` and then
spelled by hand as five hypothesis-driven variants (`tmp/grind/func_8006A564/
s9/genf.py`, f01..f05); three of them reached 0 and the adopted one (f01) is
the smallest diff from the previously banked body. No detector was dodged and
nothing about the spelling depends on a detector not catching it — it is the
same C a person would write for two consecutive record fills.

## T5 family check
Checked against the full forbidden-family catalog: no register-asm pins, no
hardcoded-`$N` asm, no scheduling barriers, no INLINE_MOVE_ALIASING, no
volatile in any spelling, no unused local array or frame coercion, no
dead-param-assign, no dead-conditional-store, no empty-body `if`, no `if (1)`
wrapper, no dead-goto label pad, no DImode chain, no goto-end accumulator, no
param-local-alias declaration trick, no `s32 one = 1;` opaque variable, no
lowercase `asm(...)`, no build-time asm rewriting, no `asm("sym")` rename, no
redundant width casts, no linker/rodata reordering. By analogy too: the closest
forbidden shapes are dead stores and constant holders, and neither applies —
`v1` is read immediately after each of its two assignments, and neither value
is a constant.

## T6 naming-announces-intent
`v0` and `v1` are the existing convention in this body (and across this decomp,
from the m2c-derived register naming); they are also, coincidentally, the exact
MIPS registers the target assigns. Neither is named `pad`, `dummy`, `unused`,
`spill`, `tmp_pad`, `slack` or anything else that announces coercion, and
neither is used only as a discard, an `&`-target, or a declaration — both are
read by real stores.

SANCTIONED-FAMILY-CLAIMS: none — this diff is ordinary C. No construct in it
requires a carve-out: there is no dead store, no constant holder, no pointer
alias to a global, no written-never-read array, no `do {} while (0)`, no
volatile, and no `/* FAKE */` construct. The only judgement call is that one
local (`v1`) is assigned twice in one scope; both assignments produce a value
that is stored to memory in the target's own bytes, so it is a named
intermediate used twice, not a borrowed carrier for a codegen effect. The
in-repo matched precedent for the identical emitted shape from the identical
source shape is `func_8006A1A0` in this same file (COMPLETED-C, on main).

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff.

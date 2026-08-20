# SELF-VET — func_80057CC8

CONSTRUCTS: none (no matching-hack construct of any family is present in the diff)

The diff replaces `INCLUDE_ASM("asm/funcs", func_80057CC8);` at src/text1b.c:1524 with an
ordinary C body. Every declaration in that body is written once and read at least once; there
are no dead stores, no self-assigns, no constant holders, no unused/written-never-read arrays,
no register pins, no `__asm__` of any kind, no `volatile`, no alias renames, no
`do { } while (0)` wraps, no `(void)` discards, no `goto`. No `/* FAKE */` construct exists
because no construct in the diff requires one.

Full construct inventory (every local, so nothing is hidden by summary):
  - `prev_idx`, `next_idx` (unsigned short) — the two neighbour vertex indices; both computed
    and both read.
  - `table` (s16 *) — the vertex table loaded from `*(s16 **)(arg0 + 4)`; read for cx/cy and
    for the prev-neighbour coordinate pair.
  - `nt` (s16 *) — the vertex table re-read before the second neighbour lookup; read twice.
  - `pi`, `ni` (s32) — the sign-extended prev/next indices; each read twice (once for the x
    component, once for the y component of the same coordinate pair).
  - `cx`, `cy` (u16) — the current vertex's coordinates; each read three times.
  - `ang_prev`, `ang_next`, `ang_mid` (s32) — the three angles; all read.
  - `base`, `half` (s32) — named intermediates of the wrap-around midpoint computation; both
    read on the branch that computes them.
  - `scale` (s32) — the radius scale; read twice.
  - `tmp` (s32, block scope) — the raw `arg1 + 1` before the wrap test; read twice.

## T1 semantic purpose
Yes, for every construct. Each local named above carries a value that is consumed by a later
computation or by one of the two output stores `*arg2` / `*arg3`. Deleting any one of them
changes what the function computes, not merely which registers GCC picks. There is no
construct whose removal would leave behavior byte-identical-in-semantics — the test that
catches purposeless coercion does not fire anywhere in this body.

## T2 human-programmer
Yes. Given the specification ("given a polygon vertex table, a vertex index, and two output
pointers, compute the angle to the previous neighbour and to the next neighbour, take the
angular midpoint with correct wrap-around, and emit the point at `arg0[2] * 40` units along
that midpoint direction from the current vertex"), the natural C is exactly this: read the
table, sign-extend the two neighbour indices, index the table as coordinate PAIRS
(`table[i * 2]`, `table[i * 2 + 1]`), call `ratan2` on the two deltas, mask to 12 bits, take
the midpoint, and scale by the sine table. Nothing in the body would make a reader ask "why is
this here?". In particular the s29 form is STRICTLY MORE natural than the previous 28-session
floor-3 chassis it replaces: that chassis carried a hand-rolled pointer local built from a
cast-heavy compound address expression
(`p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);`) plus a dead `s16 new_var`
carrier inside the `*arg3` expression. Both are GONE here, replaced by plain array indexing.

## T3 GCC-internals justification
No. The mechanism narrative in the ledger (pseudo 86's two SETs forcing global-alloc and a v1
copy-preference) is the EXPLANATION OF WHY THE OLD FORM MISSED, recorded as forensics; it is
not the justification for anything in this diff. The justification for indexing a coordinate
table as `table[i * 2]` / `table[i * 2 + 1]` is that the data is an array of x/y pairs — a
program-logic explanation that stands entirely on its own with no reference to GCC's
allocator, scheduler, DCE, RTL emission order, `reg_n_refs`, allocno priority, or any other
internal. No construct here is named after or shaped by a GCC pass.

## T4 permuter/search provenance
No. No permuter run was used this session. The form was derived by reading the ledger's own
mechanism finding (the shared `p` local is written twice, so it is one multi-SET allocno) and
asking the ordinary programming question "does this function actually need a pointer local at
all, or is it indexing an array of pairs?" — the answer being that it is indexing an array of
pairs. The six prior permuter campaigns (s4, s5, s13, s14, s22, s23) converged only on
alias-holder cheat forms and every one of those was rejected in-session and banked; none of
them proposed this form, and nothing here depends on a detector failing to recognise a
spelling.

## T5 family check
Not applicable — there is no construct to place in a family. This is not a respelling of any
banned form. Specifically, against the constraints binding this function:
  - It is NOT the closed split-`p` family (Judge 2026-07-19 17:09): there is no pointer local
    split into two source-level pointer locals. `nt` is the table BASE re-read (the same value
    `table` holds, re-loaded because the target re-loads it), not a second offset pointer; the
    old `p` offset pointer does not exist in any spelling.
  - It is NOT the compound-address duplication refused by the owner on 2026-07-20: no address
    expression is written more than once. `table[pi * 2]` and `table[pi * 2 + 1]` are two
    DIFFERENT element reads of one array, which is ordinary indexing, not duplication.
  - It is NOT dead-store, constant-holder, pointer-alias-to-global, duplicated-statement,
    dead-array, do-while(0), variable-reuse, named-intermediate-as-lever, sub-word read,
    mixed-exit, or volatile — none of those shapes appear.

## T6 naming-announces-intent
No. Names are `prev_idx`, `next_idx`, `table`, `nt`, `pi`, `ni`, `cx`, `cy`, `ang_prev`,
`ang_next`, `ang_mid`, `base`, `half`, `scale`, `tmp` — all semantic. There is no `pad`,
`_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack`, or `_frame_pad`. `nt`,
`pi`, `ni`, and `tmp` are short but each denotes a real quantity (next table, prev index, next
index, the pre-wrap candidate index) and each is genuinely read.

SANCTIONED-FAMILY-CLAIMS: none — the diff claims no sanctioned-family exception because it
contains no construct that needs one.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.

## Verification actually run this session (not claimed, measured)
  - `sandbox func_80057CC8 --disable all` = **score 0**, target_insns 111, build_insns 111,
    rules_dropped 0. The sandbox scores with regfix/asmfix DISABLED and cheat-asm stripped, so
    this is the honest cheat-free distance.
  - `build` = `sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle, **MATCH** (run twice:
    once on the pre-final `v7` spelling and once on the final comment-stripped body).
  - Zero regfix/asmfix rules exist for this function (the 7 legacy rules were retired by the
    2026-08-19 asm-until-matched migration; `rules_dropped: 0` in the sandbox output confirms
    there is nothing to drop).
  - Zero inline asm of any kind in the function body.

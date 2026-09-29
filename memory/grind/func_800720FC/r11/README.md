# Ruling 11 package — func_800720FC's shared `cells` (2026-09-29, manual s2)

Body: memory/grind/func_800720FC/landing_body.c. `cells` is a function-scope
`s32`. It receives six values, and no read can see more than one of them:

| value | write (landing_body.c) | reads | meaning |
|---|---|---|---|
| V1 | `cells = s.header + 0x18;` (first `if` block) | `s.cells = cells;` and the loop's `s.cells = cells + (mode * 6 + i) * 8;` | cell table after the two headers of `sheets[4]` |
| V2 | `cells = s.header + 0xC;` (hdr0) | `s.cells = cells;` | cell table after hdr0's header |
| V3 | same (hdr4) | same | after hdr4's header |
| V4 | same (hdr8) | same | after hdr8's header |
| V5 | same (grid inner loop, per cell) | same | after the cell header |
| V6 | same (hdr2C) | same | after hdr2C's header |

## (A) Fresh local, not a borrow
A plain local, declared once at function scope, the innermost scope enclosing
all six writes (V1 is in the `if` block, V5 in the grid loop, the rest at
function level). It is never a parameter, global, static or register variable,
and `&cells` never appears.

## (B) Every write is live, no re-store
Each write is read before the next write:
- V1 by the if-arm and the 6-iteration loop;
- V2 to V6 by the immediately following `s.cells = cells;`.

No write stores a value the variable already holds on every feasible path.
- **V2 vs V1 (+0x18 vs +0xC):** they are different sheets. On the path where the
  first block does not run, `cells` has no earlier value.
- **V3, V4, V6 vs the value before:** each follows a new `s.header` load from a
  different field (hdr4 / hdr8 / hdr2C). Take the path where hdr0 != hdr4 (data:
  distinct sheet pointers). There the variable holds hdr0 + 0xC at V3's write
  and V3 stores hdr4 + 0xC. V4 and V6 are the same case against the preceding
  sheet.
- **V5:** each iteration's header is a different cell sheet
  (arg1[3 + i*2 + j] or hdr30). On the path through the second iteration,
  `cells` holds the previous cell's table.

## (C) Same statements; every value real
- (1) The one-variable-per-value spelling is r11/variants.py `split_all`
  (tmp/func_800720FC/r11/split_all.c). It has `cells1` in the if-block, `cells5`
  in the inner loop body, and `cells2/3/4/6` at function scope.
- (2) It differs from the landing only in declarations and identifiers.
- (3) Every write is `s.header + K`, an `addiu` present in the target at each
  site (asm/funcs/func_800720FC.s lines 78, 177, 368, 510, 577, 594).

## (D) Allocator proof
**(1) Dumps.** Banked in r11/dumps/, minimal-TU builds from r11/mkws.sh. Command:

    cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dl -dg -df

- **reuse.c.lreg** says "Register 77 used 19 times across 74 insns; dies in 5
  places; crosses 2 calls". So pseudo 77 (`cells`) is not local-allocated.
  reuse.c.greg dispositions give "77 in 18", which is $s2. Every write site is
  `(set (reg/v:SI 77) (plus ... (const_int 12|24)))` (insns 174, 418, 900, 1237,
  1410, ...), and the target has `addiu s2,v0,{24,12}` at all six sites.
- **split.c.lreg** says V2/V3/V4/V6 (pseudos 80/79/78/77) are each "used 2 times
  across 2 insns in block N". Its local-alloc section gives ";; Register 77 in 2.",
  "78 in 2", "79 in 3", "80 in 2", so they sit in v0/v1. V5 (pseudo 477) is "used
  6 times across 2 insns in block 53", greg "477 in 2". V1 (pseudo 126) crosses 2
  calls, greg "126 in 16", which is $s0.

**(2) Mechanism.**
- local-alloc.c allocates only pseudos whose whole life lies in one basic block.
  local-alloc.c:192 documents reg_qty; a pseudo not local to the block keeps
  reg_qty < 0 and is left to global.c (see the check at local-alloc.c:1824-1827). Within a block
  it takes the first free register in REG_ALLOC_ORDER, and call-clobbered $2/$3
  come first.
- global.c allocates a pseudo that crosses calls only to call-saved registers
  (find_reg with the pseudo's call-crossing conflicts: reg_n_calls_crossed,
  global.c).
- In the reuse spelling, the one pseudo spans the V1 block, where it is live
  across the func_8007352C calls of the 6-cell loop, and all later sites. So it
  is a single global, call-crossing allocno. global.c seats it in $s2 for its
  whole life, and every write site's `+0xC` lands in $s2.

**(3) Necessity: mechanism plus search (Q31).**
- **The property.** The target's `addiu s2,...` at the V2..V6 sites needs those
  values in a call-saved register, although nothing at those sites crosses a
  call. With one variable per value, each of V2..V6 is written and read inside
  one basic block with no call between. That puts it in local-alloc's domain,
  whose first-fit choice is a call-clobbered register ($2/$3 in split.c.lreg).
- **What follows.** No per-value spelling can give these block-local values
  call-saved registers without adding a statement that keeps them live across a
  call, which (C)(2) forbids. The V1-only ablation shows the reverse coupling:
  splitting V1 alone leaves `cells` without a call-crossing life, and global.c
  re-seats it (120).
- **The search.** Every banked counting spelling misses (table below).

**(4) Measured alternatives** (tmp/func_800720FC/rtu.sh: the landing TU with the
body substituted, engine score; the landing itself scores 1, the GPREL16 scorer
artifact, see ../timers/README.md; oracle-verified 0):

| spelling | score / insns |
|---|---|
| split_all (one variable per value) | 124 / 687 |
| only_V1 split, rest shared | 120 / 687 |
| only_V2 split | 4 / 690 |
| only_V3 split | 4 / 690 |
| only_V4 split | 4 / 690 |
| only_V5 split | 120 / 687 |
| only_V6 split | 4 / 690 |
| struct_rw (no local; reads use `s.header + 0xC` / `sheets[4] + 0x18` directly) | 131 / 690 |
| permuter campaign from split_all | see "Permuter" below |

## (E) Name
`cells`, form (ii). Every value is the same kind of quantity: the address of the
8-byte cell table that follows the header(s) of the sprite sheet being drawn. The
descriptor field it feeds is `s.cells`, and func_8007352C / func_80073728 walk it
as 8-byte cells. The name is true of every write.

## (F) Annotation
The declaration comment in landing_body.c says the variable holds several
values of one kind, names the kind (the two forms), and cites Ruling 11 and this
directory.

## (G) Review
Layer-2 on the manual path.

## (H) Everything else
Everything else in the body is judged on its own. In particular, the timer
pointer alias is judged under pointer-alias-fake-exception (../timers/README.md)
and the dead 0x100 scale stores under dead-store-fake-exception.

## Permuter
Campaign r11-split-all ran 2026-09-29 09:10-09:41 UTC, 30.5 min, -j 2.
- **Setup:** workspace from r11/mkws.sh on tmp/func_800720FC/r11/split_all.c,
  permuter base score 1845, harvested and stopped through
  tools/permuter_campaign.py, with telemetry in metrics/events.jsonl.
- **Result:** 453 outputs, each re-scored with the engine metric
  (r11/permuter_engine_scores.txt). Best is 22 (output-863-1), then 24, 26,
  27, 28. None reaches the target.
- **What the finds reuse:** the best finds re-introduce a variable that spans
  blocks.
  - 863-1, 1168-1 and 1177-1 add a function-scope `new_var = s.header` in the
    grid loop and read it again for V6. This changes the program: V6 would use
    the last grid header instead of hdr2C.
  - 768-1 (38) writes V5 into `cells2`, making cells2 live across the grid's
    calls.
  - 1250-1 adds a `Desc720FC *new_var = &s` alias.

  All of these give a value a multi-block, call-crossing life, which is the
  mechanism (D)(2) names. No permuter output with one variable per value comes
  near the target.

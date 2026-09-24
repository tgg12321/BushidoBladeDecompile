# func_80074E08 matching ledger

## s1 — natural reconstruction

- Canonical gate: C (`hand_coded_tier: LOW`).
- Baseline: whole-body `INCLUDE_ASM`, 281 target instructions, no C body.
- Reconstruction: emit one tile, build four linked animation records from the
  table at `arg0[0] + 0x18`, then append paired draw-area and draw-offset GPU
  packets. Reuse the pre-existing `S_80074488` record layout; use natural
  `u16 rect[4]` and `u16 offset[2]` stack objects.
- Match-motivated constructs: none.
- Measurement: score 78, 282/281 instructions. Main mismatch: repeated reloads
  of `arg0[5]`, missing signed-16 loop-counter extensions, and four pointer
  setup sequences computing the table pointer from the stack copy instead of
  the loaded record-header value.

## s2 — preserve semantic source values

- Bind `arg0[5]` to a local primitive cursor, make the two-iteration counter
  `s16`, and use scoped named values for each table entry so both record fields
  are derived from the same load.
- Match-motivated constructs: none; all locals hold real consumed values.
- Measurement: score 44, exactly 281 instructions. The primitive cursor and
  signed loop counter closed the instruction-count gap. Remaining source-level
  differences: GCC hoisted the explicitly named `x_offset`, pointer-next
  computations still occurred after the first stack store, and the later
  rectangle x coordinate occupied a saved register.

## s3 — expose repeated arithmetic and complete pointer pairs

- Spell the two real `arg1 * 0xF0` uses directly so ordinary CSE can retain the
  value at its first-use position, rather than explicitly introducing a
  hoistable local. Give each table load its own fresh value/next pair, both
  computed before the record stores. Separate the later rectangle x coordinate
  from that expression.
- Match-motivated constructs: none; the arithmetic is the direct computation
  at each semantic use, and every named intermediate is once-written and
  consumed.
- Measurement: score 21, 281/281 instructions. The arithmetic and register
  seats now match. Residual: four `table = header + 0xC` results use `$v0`
  instead of target `$v1`, plus the in-loop table adjustment is scheduled
  differently.

## s4 — truthful pointer model and statement-order close

- Use the pre-existing shared `EnvA` descriptor model. Its `header` and
  `table` members express the two pointer roles used by the descriptor helper
  family; only byte-addressing sites require casts.
- Reuse `table` only for its single role, feeding `s.table`, at four repeated
  record sites. This is the Ruling 5 extension shape detailed below.
- Move the independent `pad20` and `pad24` initializations after the
  texture-derived `s.table` adjustment. Both orders have identical C semantics;
  this order reproduces the target schedule.
- Measurements: pointer model with per-site `table` locals 15; reused `table`
  before the statement-order change 9; reused `table` plus the natural
  statement-order change **0/281**. `--diff` reports zero source-level and zero operand-only
  hunks (six branch-target relocation artifacts are explicitly not scored).

## Full-cluster ablations

- Plain per-write form (`memory/grind/func_80074E08/carrier-free.c`), with a fresh
  block-local `table` at each site and the otherwise final body: **12/281**.
- Put `pad20`/`pad24` before the `s.table` adjustment: **9/281**.
- The first matching form used fresh `texture` and `u_offset` intermediates
  around the byte lookup. Removing that entire cluster and spelling the
  expression plainly as `s.table += *((u8 *)s.header + 2) * 8`: **0/281**.
  The plain form is the landing form.
- Four single-level `do { ... } while (0)` wrappers around the per-write
  `table` sites: 25/281, rejected. Direct duplicate record reads: 40/289,
  rejected. Function-scoped one-local-per-write values: 69/282, rejected.

## Ruling 5 extension — reused `table`

- Every write and consumer is textually identical:
  `table = (s8 *)s.header + 0xC;` then `s.table = table;`. `table` has no other
  reader or role.
- Each `s.header` immediately selects one element of the same stable `records`
  array (`[3]`, `[2]`, `[0]`, `[1]`), and no statement touches `s` between
  that member store and the `table` write. `records` is loaded once and never
  modified.
- Each write/consumer pair is in its own sibling block. `s.header` is also read
  by the descriptor callees and by the real byte-at-`+2` lookup, so it is not a
  staging field. Each new selection differs from the previous one.
- The variable is declared once at the innermost scope enclosing all sites,
  names its sole consumer role, carries a real target computation, and no
  statement was added for reuse. No FAKE annotation is required by Ruling 5.
- One-local-per-write receipt:
  `memory/grind/func_80074E08/carrier-free.c`, 12/281.
- Allocation receipt: in the reuse-form `text1b.rtl`, `table` is pseudo 78; the
  `text1b.greg` allocation table assigns pseudo 78 to hard register 3 (`$v1`),
  matching target. In the per-write dump, the four `+ 0xC` pseudos are 115,
  120, 144 and 151; local allocation assigns each to hard register 2 (`$v0`),
  exactly accounting for the 12-point residual. Dumps are under
  `tmp/grind/func_80074E08/dumps/` and
  `tmp/func_80074E08/alloc_carrier_free/`.
- Permuter receipt: campaign `carrier-free-12-envA`, from the shared-EnvA plain
  per-write seed, ran 35,979 iterations for 1,164.7 seconds and found no zero;
  its best new native score was 633. A stopped continuation added 816
  iterations/28.2 seconds without improving the 820 base. The fresh EnvA-seed
  search therefore produced no carrier-free alternative.

## Oracle

- `verify-oracle --rebuild --allow-dirty`: SHA1
  `62efab4f73f992798c43e8c730aa43baa10bb4fa`, matching the oracle; freshness
  true and golden fixtures green.

## Fresh adversarial review

- PASS on the final `EnvA`/`records`/`table` body after independent inspection
  of the honest 0/281 sandbox, durable 12/281 carrier-free ablation, GCC
  allocation receipt, harvested permuter campaign, and source text.
- The reviewer found no asm, register pins, volatile coercion, dead stores,
  false aliases, prototype lies, fabricated object layout, or unsanctioned
  match-only constructs.

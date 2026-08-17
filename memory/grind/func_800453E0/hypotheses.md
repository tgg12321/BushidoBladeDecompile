# func_800453E0 — hypotheses

Status: **matched in pure C, honest floor 0.** Session 2 (2026-08-17) reproduced
session 1's distance 0 and then SIMPLIFIED the body, killing two of session 1's
own "this construct is required" claims by measurement. The live frontier is
empty.

## CONFIRMED (still load-bearing in the final body)

- **H1 — the inherited `volatile s32 sp_pad;` was a frame-coercion cheat whose
  removal by the sandbox's cheat-stripper accounted for 12 of the 26 inherited
  diffs.** Mechanism: `get_frame_size()` counts the unreferenced volatile
  scalar's slot (`vars= 8`), so with it cc1 emits target's 0x30 frame; stripped,
  `vars= 0`, frame 0x28, and all 5 register-save offsets plus both `addiu $sp`
  shift by 8. Probe: the `.frame` comment via
  `tmp/grind/func_800453E0/s1/frame.sh`, compared against the sandbox `.o`
  disassembly. CONFIRMED — and dissolved: the natural body reaches `vars= 8`
  with nothing declared to reserve it (`addiu sp,sp,-48` in
  `tmp/grind/func_800453E0/s2/final_func.txt`).

- **H2 — target's copy loop is a real 16-byte struct assignment, not four
  hand-written word temps.** Mechanism: GCC expands the struct assign via
  `movstrsi` as one block, so the 4 loads and 4 stores stay contiguous and the
  induction updates schedule after them; separate named temps let the scheduler
  interleave the counter increment and one pointer advance into the store block.
  Probe: `SubEntry` + `SUBSLOT[j] = SUBSLOT[j + 1]`. CONFIRMED — all 10
  copy-loop scheduling diffs closed in one step and never regressed.

- **H3 — target's addressing is symbol-relative (`%hi(SYM)` + offset register)
  outside the copy loop, so the base must NOT be bound to a local pointer or a
  struct index.** Mechanism: a local `SubEntry *tbl` makes the base an unknown
  value, so strength reduction builds a walking pointer and CSE shares one base
  register function-wide. Probes: local pointer (66 insns, floor 37) vs
  constant-address macro (69 insns, floor 21) vs macro + byte-offset casts for
  search/clear (76 insns, floor 18) — all session 1; RE-CONFIRMED session 2 from
  the ZERO baseline, where rewriting only the search loop's two accesses as
  `SUBSLOT[i + 1].id` / `.amt` took 0 -> 11 and 76 -> 72 insns
  (`rejected/struct-idiom-search-loop-costs-11.c`). CONFIRMED.

- **H6 — the table byte offset is a strength-reduction giv, not a source
  induction variable.** Mechanism: a source variable advanced in the loop
  increment gets its zero-init emitted at its source position, ahead of the
  LICM-hoisted base addresses; a giv's init is created by loop.c into the
  pre-header AFTER the movables, which is target's order. Probe: `off += 0x10`
  in the for-increment vs `off = i << 4;` at the top of the body. CONFIRMED
  (session 1, floor 3 -> 1). Retained in the final body, which writes
  `off = i << 4;` as the first statement of the loop.

- **H7 — the final residual was a reloc-addend artifact, and both of the copy
  loop's base pointers derive from one symbol in target.** Mechanism:
  `SUBTBL[j-1] = SUBTBL[j]` emits a LO16 reloc against D_800EED10 with addend
  -16, whose resolved immediate already equals `%lo(D_800EED00)` (`%hi` is
  0x800F either way) — the linked bytes were correct but `engine/score.py` does
  not mask data LO16 addends. Writing the loop against the D_800EED00-based view
  (`SUBSLOT[j] = SUBSLOT[j + 1]`) puts `$s2 = %lo(D_800EED00)` and
  `$s3 = $s2 + 0x10`, matching target's register roles, and emits the addend-0
  reloc — visible in `tmp/grind/func_800453E0/s2/final_func.txt`
  (`277c R_MIPS_HI16 D_800EED00` / `2780 R_MIPS_LO16 D_800EED00` /
  `addiu s3,s2,16`). CONFIRMED (session 1, floor 1 -> 0).

## KILLED (session 2 — these were session 1 claims, disproven by measurement)

- **H5 (session 1) — "the outer loop must be a do-while that re-reads
  `D_800A33AC` into `count` at the bottom of the body, before incrementing `i`,
  or the `lw`'s load-delay slot goes unfilled and maspsx inserts a nop."**
  KILLED. Probe: from the distance-0 do-while body, replace the whole
  `i = 0; count = D_800A33AC; if (i >= count) return; do { ... count =
  D_800A33AC; i += 1; } while (i < count);` scaffold with a plain
  `for (i = 0; i < D_800A33AC; i++) { ... }` and delete `count`. Result:
  sandbox `--disable all` == **0**, 76/76 insns, and the two objects'
  disassemblies are **byte-identical** (`diff
  tmp/grind/func_800453E0/s2/forloop_func.txt
  tmp/grind/func_800453E0/s2/dowhile_func.txt` — clean). A live control probe
  (`i + 1` -> `i + 2`) moved the score to 1, proving the builds were fresh and
  not a stale artifact. The delay slot is filled either way once `off = i << 4`
  is a giv (H6): GCC's own loop-condition re-read of the global supplies the
  independent instruction. NOTE: this hypothesis was the subject of the
  layer-1 FAIL — session 1 justified it by citing
  `.claude/rules/loop-counter-fills-load-delay.md`, which does not exist as a
  file (the technique is only summarized in the index rule
  `.claude/rules/codegen-technique-index.md`). That citation is BANNED for this
  function and is not used anywhere in the session-2 ledger or self-vet; more
  importantly the construct it was cited to justify is now GONE from the body,
  so the question is moot.

- **H4 (session 1) — "`i + 1` needs its own variable (`s32 next`), separate from
  the shift-loop counter, or the two coalesce into a single pseudo and the
  `$s0`/`$s1` assignment mirror-images target's."** KILLED as a NECESSITY claim.
  Probe: from the distance-0 body, substitute `i + 1` inline at both use sites
  (the `func_80045294` argument and the inner loop's initializer) and delete the
  `s32 next;` declaration. Result: sandbox `--disable all` == **0**, 76/76
  insns, both before and after removing the declaration. H4 was a true
  observation about an INTERMEDIATE form (at floor 18 the shared variable did
  cost 12), but it is not a property of the final body — GCC creates the two
  pseudos from the two `i + 1` expressions on its own.

## Frontier
Empty — the function is matched at honest distance 0 in a body with no cheat,
no carve-out, no FAKE construct, and no construct that a measured simplification
could remove. The only outstanding work is INTEGRATION, which a grind session is
not permitted to perform: delete the 7 stale regfix.txt rules at
regfix.txt:214-224 (written for the OLD C; a full build with them in place will
corrupt this function), then verify the oracle SHA1 and mark the queue item done.

## Method note worth carrying to other functions
Session 1 reached 0 through seven measured steps and then stopped, banking every
step's construct as "required". Session 2 re-ran each of those claims AS A
SUBTRACTION FROM THE ZERO BASELINE and found two of them were artifacts of the
path, not of the destination. Doing that subtraction pass before submitting is
cheap (one sandbox call per construct), shrinks the reviewable surface, and — as
here — can delete the exact construct a reviewer objected to instead of arguing
for it.

## Notes worth carrying to other functions in this file
- `src/text1a_c.c` has FIVE more functions walking this same 16-byte record
  table (func_80045294 at ~1666, func_80045510 at 1759, func_800455AC, and the
  two setters near the end). The `SubEntry` typedef + the `SUBSLOT` view are now
  in the file at line ~1720 and available to them; if any is still queued, the
  struct-assignment and byte-offset findings above transfer directly.
- The general lesson: an asm transcription with register-named locals and goto
  labels can sit at a low-but-immovable distance while the FORM is wrong. Here
  recovering the semantics first (a record-array entry removal) and then
  choosing among semantically-identical spellings closed 26 -> 0, with no cheat
  and no carve-out.

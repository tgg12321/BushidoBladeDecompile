# func_800453E0 — hypotheses

All hypotheses raised this session were CONFIRMED and are now closed; the
function reached honest sandbox distance 0. The live frontier is empty.

## CONFIRMED (session 1, 2026-08-17, modality `recon`)

- **H1 — the inherited `volatile s32 sp_pad;` was a frame-coercion cheat whose
  removal by the sandbox's cheat-stripper accounted for 12 of the 26 diffs.**
  Mechanism: `get_frame_size()` counts the unreferenced volatile scalar's slot
  (`vars= 8`), so with it cc1 emits target's 0x30 frame; stripped, `vars= 0`,
  frame 0x28, and all 5 save offsets plus both `addiu $sp` shift by 8.
  Probe: read the `.frame` comment via `tmp/grind/func_800453E0/s1/frame.sh`,
  compared against the sandbox `.o` disassembly. CONFIRMED — and dissolved: the
  natural body reaches `vars= 8` with nothing declared to reserve it.

- **H2 — target's copy loop is a real 16-byte struct assignment, not four
  hand-written word temps.** Mechanism: GCC expands the struct assign via
  `movstrsi` as one block, so the 4 loads and 4 stores stay contiguous and the
  induction updates schedule after them; separate named temps let the scheduler
  interleave `t0++`/`a2 += 16` into the store block. Probe: introduce
  `SubEntry` + `SUBSLOT[j] = SUBSLOT[j + 1]`. CONFIRMED — all 10 copy-loop
  scheduling diffs closed in one step and never regressed.

- **H3 — target's addressing is symbol-relative (`%hi(SYM)` + offset register)
  outside the copy loop, so the base must NOT be bound to a local pointer.**
  Mechanism: a local `SubEntry *tbl` makes the base an unknown value, so
  strength reduction builds a walking pointer and CSE shares one base register
  function-wide (costing 7 instructions, incl. collapsing the tail clear's two
  `lui`/`addu` pairs). Probe: local pointer (66 insns, floor 37) vs
  constant-address macro (69 insns, floor 21) vs macro + byte-offset casts for
  search/clear (76 insns, floor 18). CONFIRMED.

- **H4 — `i + 1` and the shift-loop counter are two distinct pseudos in target.**
  Mechanism: target holds `i + 1` in callee-save `$s1` across the `jal`, then
  `addu $t0,$s1,$zero` copies it into a caller-save for the call-free inner loop;
  one shared variable coalesces them and mirror-images the `$s0`/`$s1`
  assignment. Probe: add `s32 next = i + 1;`, start the loop `for (j = next; ...)`.
  CONFIRMED — floor 18 -> 6, the largest single step.

- **H5 — the outer loop re-reads `D_800A33AC` before incrementing `i`, which is
  what fills the load-delay slot.** Mechanism: same as
  `.claude/rules/loop-counter-fills-load-delay.md` (found on this function's own
  callee func_80045294) — with the increment emitted first there is no
  independent instruction to fill the `lw`'s delay slot and maspsx inserts a
  `nop`. Probe: adopt the already-matching sibling func_80045510's shape
  (`i = 0; count = D_800A33AC; if (i >= count) return;` + do-while ending
  `count = D_800A33AC; i += 1;`). CONFIRMED — floor 6 -> 3.

- **H6 — the table byte offset is a strength-reduction giv, not a source
  induction variable.** Mechanism: a source variable advanced in the loop
  increment gets its zero-init emitted at its source position, ahead of the
  LICM-hoisted base addresses; a giv's init is created by loop.c into the
  pre-header AFTER the movables, which is target's order. Probe: replace
  `off += 0x10` in the loop increment with `off = i << 4;` at the top of the
  body. CONFIRMED — floor 3 -> 1, pre-header ordering exact.

- **H7 — the final residual was a reloc-addend artifact, and both of the copy
  loop's base pointers derive from one symbol in target.** Mechanism:
  `SUBTBL[j-1]` emits a LO16 reloc against D_800EED10 with addend -16, whose
  resolved immediate already equals target's `%lo(D_800EED00)` (`%hi` is 0x800F
  either way) — the linked bytes were correct but `engine/score.py` does not mask
  data LO16 addends. Writing the loop against the D_800EED00-based view
  (`SUBSLOT[j] = SUBSLOT[j + 1]`) puts `$s2 = %lo(D_800EED00)` and
  `$s3 = $s2 + 0x10`, matching target's register roles, and emits the addend-0
  reloc. Probe: both spellings, compared instruction-by-instruction and by
  `objdump -r`. CONFIRMED — floor 1 -> 0.

## KILLED
None. No hypothesis raised this session was disproven.

## Frontier
Empty — the function is matched. The only outstanding work is INTEGRATION, which
this session is not permitted to perform: delete the 7 stale regfix.txt rules at
regfix.txt:214-224 (they were written for the old C and will corrupt the function
if a full build runs with them in place), then verify the oracle SHA1 and mark the
queue item done.

## Notes worth carrying to other functions in this file
- `src/text1a_c.c` has FIVE more functions walking this same 16-byte record table
  (func_80045294 at ~1690, func_80045510 at 1760, func_800455AC at ~1812, and the
  setters at ~1862/1875). The `SubEntry` typedef + `SUBTBL`/`SUBSLOT` views are
  now in the file at line 1713 and available to them; if any is still queued, the
  struct-assignment and byte-offset findings above transfer directly.
- The general lesson: an asm transcription with register-named locals and goto
  labels can sit at a low-but-immovable distance while the FORM is wrong. Here
  recovering the semantics first (a record-array entry removal) and then choosing
  among semantically-identical spellings closed 26 -> 0 in seven measured steps,
  with no cheat and no carve-out.

---
name: aggregate-merge-family
paths: [".claude/rules/aggregate-merge-family.md"]
description: "Frozen-list family: merging per-word splat D_ scalars into one aggregate — prongs (a)-(e), compiler-necessity evidence (a1)-(a4′), forced-in bytes (Q13/Q14), and the narrow per-function F4/item-4 exceptions (Q63, Q73, Q96-Q99)."
metadata:
  type: rules
  tier: hygiene
---

# Per-word splat symbol → aggregate merge (owner ruling 2026-08-17)

> **Tier (owner ruling Q91):** merging per-word splat symbols into the real aggregate, declared in the shared header and used as the one handle, is blocking (completion-bar item 4); retiring the old rows in `named_syms.txt` / `undefined_syms_auto.txt` is hygiene.

Frozen-list entry of [[no-new-park-categories]] (SOTN `Vram`-style merges). Two or more splat
`D_<addr>` scalars may become one aggregate when ALL hold:
- **(a)** The object model is established by evidence independent of and predating the
  byte-chasing (cross-TU stride indexing, base+offset addressing in the original, a committed
  naming-census schema), OR by compiler necessity (a1)-(a4) below.
- **(b)** The declaration reflects that shape (records where evidence shows records; a flat
  array only for a flat array); a record stride encoded as a magic index fails.
- **(c)** Complete: every merged symbol leaves C and the splat config (one C handle per
  location). A row may stay in `undefined_syms_auto.txt`/`named_syms.txt` while an
  `INCLUDE_ASM` sibling's `.s` references it, if no C names it and it is suffixed
  `/* alias of <base>+N; retire with <sibling> */`.
- **(d)** Canonical declaration in the shared header; never TU-local, never a per-use pointer
  pun.
- **(e)** Byte-neutral for every other consumer, `verify-oracle --rebuild`, layer-2.

Second views of the same bytes (per-file declarations Q21, union word views Q33/Q46/Q57, the
local-array cast store Q36): [[aggregate-declaration-views]].

## Compiler-necessity evidence for (a) (2026-09-26)

- **(a1)** Banked dumps (`.cse`, `.loop`, `.lreg`, `.greg` and/or `BB2_*_DEBUG`, with command
  lines) for split and merged spellings name the decision (pass + `tools/gcc-2.7.2` location)
  that needs ONE object, so no separate-object spelling can produce it; best split floor + a
  structural respelling recorded. Split spellings relying on refused/banned constructs are set
  aside; an admissible sanctioned-family split spelling that reaches the target defeats (a1).
- **(a2)** cc1psx (calibration only) on the same preprocessed TU: split does NOT, merged DOES
  produce the target's address-forming instructions (opcode, register, offset); outputs banked.
- **(a3)** Minimal span: lowest to highest label the function references; every label inside
  merged, none outside.
- **(a4)** One record layout across the span; array length = span / record size.
- **(a4′) Mixed-field struct (Q7)**, in place of (a3)/(a4), with (a1)/(a2) against the separate
  labels: (1) span = exactly the bytes the function accesses (incl. reachable indexed/walk
  ranges); (2) members only for used bytes, width from the accesses, ONE declared signedness
  under which every access is ordinary C (value casts ok, puns not), arrays only for indexed/
  walked bytes; (3) unaccessed gaps only as compiler padding or one offset-named filler
  (`u8 unkNN[k];`); (4) the ledger lists each member's offset, width, type and every accessing
  instruction; (5) prongs (b)-(e) apply.

**Forced-in bytes (Q13).** A byte inside the span the function never touches may be a named
member only if another function's ORIGINAL bytes access exactly that offset/width (all
accessors agree). Its signedness equals an accessor's revealed signedness: `lb`/`lh` vs
`lbu`/`lhu`, or for words/store-only an ordered compare (`slt`/`slti` vs `sltu`/`sltiu`), a
non-divide `sra` vs `srl`, or `div` vs `divu`. Equality tests, bounds checks, divide-expansion
shifts and `mult`/`multu` are NOT evidence. Otherwise it stays a filler. **Q14:** when no
instruction anywhere reveals signedness (full no-reveal listing incl. sign branches and
sub-word reads) AND signed/unsigned builds of every C accessor are byte-identical (banked), the
member keeps main's current type (or the SDK type assigned to it), named in the ledger. Names
follow the naming-evidence rules (offset names always ok).

## Narrow per-function exceptions (Q63, Q73, Q96-Q99); nothing else may cite them

Interim labels for the named functions only (owner direction 2026-10-03): each is retired, not
extended, once a one-object spelling matches.

- **D_800A37D2 / D_800A37D3 (Q63):** only after the one-array and two-member-struct whole-tree
  forms are banked failing (scratch SHA1 + differing words). Then two adjacent `u8` scalars,
  indexed via the first's address (`p = &D_800A37D2; p[t != 0]++`,
  `(&D_800A37D2)[D_800A3748]`), each indexed use and the `p` assignment `/* FAKE */`-annotated,
  `p` meeting [[pointer-alias-fake-exception]]. For func_8001C8DC and func_8003CF84 only.
- **D_80101EC8 per-word reads (Q73):** `D_80101FA0`, `D_80101FA8`, `D_801023EC`,
  `D_801023F4`, `D_80101FBC`, `D_80101FC4`, `D_80102408`, `D_80102410`, read by own name only in
  func_8002BC68 / func_8002BEA0, only after the three single-object spellings (direct struct
  field, typed base pointer, per-record pointer) are banked missing for the shared-base reason
  (`.cse` `use_related_value` and `.greg`/final `.s` addressing), with Q31 mechanism + search.
  Each per-word declaration is `/* FAKE */`-annotated; every other access goes through
  `D_80101EC8`.
- **D_800A37E8 / EA / EC (Q96):** func_80027AD8 and func_8002AB08 only. Three `s16` scalars;
  the vector's address handed to callees is `&D_800A37E8`, `/* FAKE */`-annotated at the
  assignment. Basis: the `s16[3]` and `{x,y,z}` forms banked at 2 (cse rewrites the dot's [0]
  address to `vec`'s register; cc1psx identical),
  memory/grind/judge-decl-cleanup/followups/func_80027AD8.vec-investigation.md.
- **D_800A376A / D_800A376B (Q97):** func_8002738C only: the per-player byte indexed as
  `*(&D_800A376A + a0)`, `/* FAKE */`-annotated. Basis: the `u8 D_800A376A[2]` form only
  (source in followups/func_8002738C.c12-merge-variant.diff; func_8003B2C8 12 / func_8003B328 16
  as reported by the cheat-sweep lane, scores not banked). No struct form was measured: open debt
  (docs/grind/handoff-2026-09-30.md); a one-object form that matches retires Q97.
- **D_800906A4 / D_8009060C (Q98, item 4):** declared `s16 D_800906A4[39][2]`,
  `s32 D_8009060C[38]` (StatusUpBuf retires). func_8003DA8C only: the pair's [1] read (used twice) as
  a byte offset off the declared array and its [0] through a row pointer, both
  `/* FAKE */`-annotated (element reads schedule above the scalar stores: 8 / 9).
  followups/func_8003DA8C.stagetables.*.
- **CdlREAD D_800A14D0 (Q99, item 4):** one `extern volatile CdlREAD D_800A14D0` in libcd.h;
  cb_read only keeps one block-scope `extern volatile s32` line for cnt, size and tslmode,
  `/* FAKE */`-annotated (none 2; the measured partial sets cnt, cnt+size, cnt+tslmode,
  size+tslmode 2-3; singletons size / tslmode not measured). followups/cb_read.cdlread.*.

Records: docs/grind/decisions.md OWNER RULING entries of 2026-08-17, 2026-09-03, 2026-09-26
(Q7, Q13, Q14) and 2026-09-30 (Q63, Q73) and 2026-10-03 (Q96-Q99).

Related: [[aggregate-declaration-views]] · [[split-scalars-hide-aggregate]] ·
[[header-type-correction-from-use-sites]]

---
name: aggregate-merge-family
paths: [".claude/rules/aggregate-merge-family.md"]
description: "Frozen-list family: merging per-word splat D_ scalars into one aggregate — prongs (a)-(e), compiler-necessity evidence (a1)-(a4′), forced-in bytes (Q13/Q14), and the narrow per-function F4/item-4 exceptions (Q63, Q73, Q96-Q99, Q125)."
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

## Narrow per-function exceptions (Q63, Q73, Q96-Q99, Q125); nothing else may cite them

Interim labels for the named functions only (owner 2026-10-03), retired, not extended, once a
one-object spelling matches.

- **D_800A37D2 / D_800A37D3 (Q63):** only after the one-array and two-member-struct whole-tree
  forms are failing (SHA1 + diff words). Then two adjacent `u8` scalars,
  indexed via the first's address (`p = &D_800A37D2; p[t != 0]++`,
  `(&D_800A37D2)[g_fight_followup_fighter_index]`), each indexed use and `p`'s assignment `/* FAKE */`-annotated,
  `p` meeting [[pointer-alias-fake-exception]]. For fight_UpdateDefeatDelayAndModeProgress and func_8003CF84 only.
- **D_80101EC8 per-word reads (Q73):** `D_80101FA0`, `D_80101FA8`, `D_801023EC`,
  `D_801023F4`, `D_80101FBC`, `D_80101FC4`, `D_80102408`, `D_80102410`, read by name only in
  fighter_SetBasePositionSeparationCorrectionsAndGetDistance / fighter_CalcSeparationErrorAndOpposedOffsets, only after the three single-object spellings (direct
  field, typed base pointer, per-record pointer) are banked missing for the shared-base reason
  (`.cse` `use_related_value` and `.greg`/final `.s` addressing), with Q31 mechanism + search.
  Each per-word declaration is `/* FAKE */`-annotated; all other accesses use
  `D_80101EC8`.
- **D_800A37E8 / EA / EC (Q96; Q117 2026-10-06):** fighter_ResolveBoneHit, fighter_ProcessWeaponBodyHits and object_ResolveFighterHits's two
  call sites only (object_ResolveFighterHits: at each argument). Three `s16` scalars;
  the vector's address handed to callees is `&D_800A37E8`, `/* FAKE */`-annotated at the
  assignment. Basis: the `s16[3]` and `{x,y,z}` forms banked at 2 (cse rewrites the dot's [0]
  address to `vec`'s register; cc1psx identical),
  memory/grind/judge-decl-cleanup/followups/fighter_ResolveBoneHit.vec-investigation.md.
- **g_fight_bone_group_tim_requests / D_800A376B (Q97):** fighter_AccumulateBoneGroupFlagWhenGameModeZero only: per-player byte
  `*(&g_fight_bone_group_tim_requests + a0)`, FAKE-annotated; basis the u8[2] form only
  (source in followups/func_8002738C.c12-merge-variant.diff; func_8003B2C8 12 / func_8003B328 16
  per cheat-sweep lane, not banked). No struct form was measured: open debt
  (docs/grind/handoff-2026-09-30.md); a one-object form that matches retires Q97.
- **D_800906A4 / D_8009060C (Q98, item 4):** declared `s16 D_800906A4[39][2]`,
  `s32 D_8009060C[38]` (StatusUpBuf retires). gpu_LatchFogDepthOnTransitionAndQueueMoves only: the pair's [1] read (twice) as
  a byte offset and its [0] through a row pointer, both
  `/* FAKE */`-annotated (element reads schedule above the scalar stores: 8 / 9).
  followups/func_8003DA8C.stagetables.*.
- **CdlREAD D_800A14D0 (Q99, item 4):** one `extern volatile CdlREAD D_800A14D0` in libcd.h,
  with the saved result pointer as `.result` at 0x34 (psyz ReadAttr); every cdread.c access goes
  through it. The cb_read per-member FAKE is retired.
- **D_800A36C2 / D_800A36D2 (Q125, item 4):** comb_ExchangePad only. The high halfwords (+2) of
  the s32 link pad words D_800A36C0 / D_800A36D0 as their own `extern s16` in bb2.h, each
  `/* FAKE */`-annotated with the measured scores; every other access goes through the s32 words.
  Basis: the original reads each word and its low half gp-relative and the high half lui / %lo
  (the `.comm` S+k rule of [[per-file-gp-model]]); every one-object form measured failing (Q33 union
  per word 4, its member reads forcing the address into $s0; (u16) word low reads 4; u16 h[2] 7;
  s16 h[2] with casts 4; aligned halves 4), scores in the fake-reduce row.

Records: docs/grind/decisions.md OWNER RULINGs 2026-08-17, 09-03, 09-26 (Q7, Q13, Q14),
09-30 (Q63, Q73), 10-03 (Q96-Q99), 10-23 (Q125).

Related: [[aggregate-declaration-views]] · [[split-scalars-hide-aggregate]] ·
[[header-type-correction-from-use-sites]]

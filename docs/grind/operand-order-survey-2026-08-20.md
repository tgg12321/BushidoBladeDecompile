# SOTN-master evidence survey — commutative operand order / or-tree grouping

Date: 2026-08-20. Read-only survey preceding an owner ruling on the
commutative-operand-order family (`.claude/rules/or-tree-shape-shift.md`).
Question: does fully-matched PSX code in SOTN-master ship operand orderings of
associative-commutative expressions (`|`, `+`, `&`, `^`) that deviate from
"natural" order (memory/bit-position/struct-field/declaration order), and are
such deviations annotated or accepted silently?

Anchor shape: BB2 `gnd_init_80041688` (memory/grind/func_80041688/evidence.md) —
three `lbu` at +0x18/19/1A (r,g,b) packed via or-tree; natural spelling emits
[r,g,b], target needs [b,r,g]; the only working lever is reparenthesizing
`(b|r)|g` (sched1 INSN_PRIORITY chain-length mechanism, fully characterized
s1–s15). Siblings func_80048530, func_800645B0 hit the same commutative-order
wall (func_800645B0 evidence names "Commutative operand order" explicitly).

## Corpus

- Clone: `tmp/sotn-decomp` at **`aa53500226ee84be763f3e8702b27de06456b3a7`**
  (2026-07-10) — the SAME commit the construct index
  (`docs/reference/sotn-construct-index.md`) was built from.
- PSX provenance only: `src/dra`, `src/ric`, `src/st` `*.c`, excluding every
  `*_psp` directory. (~1,077 non-PSP C files in scope.)
- Match status verified per exhibit file via `grep -c INCLUDE_ASM` (0 = fully
  matched file; noted per row).

## Base rates

| Population (PSX dra/ric/st, non-PSP) | N | deviating order | annotated |
|---|---:|---:|---:|
| Or-chains containing ≥2 shifts (the anchor's exact shape class) | 4 (2 are constant macros in `dra/sound.c`; 2 live RGB555 packers) | **0** | — |
| Single-shift OR packs (`x \| (y << n)` forms, incl. nibble-swap cutscene family) | ~14–16 | 1 ambiguous (`e_shop.c:907` unshifted-first) | 0 |
| Explicit right-grouped commutative sums `a + (b + c)` (parens with NO precedence function) | **8** (2 files) | 8 by construction (non-left-assoc grouping) | **0 — silent** |
| Explicit right-grouped `a \| (b \| c)` | 1 (`dra/7879C.c:3351`, RHS all constants → folds, codegen-neutral) | — | 0 |
| Comment-annotated operand/term-order deviations (from 229 `match_comment` + 75 `fake_comment` entries + fresh keyword sweep) | **3** | 3 | 3 |

The anchor's exact shape (multi-byte RGB or-tree from loads) is essentially
ABSENT from SOTN PSX sources: only two live multi-shift packers exist and both
follow natural ascending bit order.

## Exhibits

### E1 — GOLD: `src/st/no3/e_warg.c:433-445` (file fully matched, 0 INCLUDE_ASM)

```c
// These are the same code, but compiler instruction reordering is messing it
// up. There's almost certainly a single solution, but I can't find it.
#ifdef VERSION_PSP
    part->params = ((g_unkGraphicsStruct.g_zEntityCenter - 8 - (Random() & 7)) << 8) + 1;
#else
    part->params = ((g_unkGraphicsStruct.g_zEntityCenter - (Random() & 7) - 8) << 8) + 1;
#endif
```

Commutative additive-term reorder (`x − 8 − rand` ⇄ `x − rand − 8`; identical
value, terms reassociated). The PSP port keeps the natural spelling; the PSX
spelling exists ONLY to satisfy PSX codegen ordering. The comment states plainly
that the author could not derive it and shipped the order that matched. This is
precisely "a target-matching order found by trial, shipped on master,
annotated." It is on SOTN master in a fully-matched file.

### E2 — `src/dra/7E4BC.c:1570-1573` (file fully matched)

```c
// weird RBG assign order, not RGB
prim->r0 = prim->b0 = prim->g0 = prim->r1 = prim->b1 = prim->g1 =
    prim->r2 = prim->b2 = prim->g2 = prim->r3 = prim->b3 = prim->g3 = ...;
```

Color-channel ordering deviating from the natural r,g,b field/declaration order
(r,b,g), kept to match, annotated with a bare descriptive comment — not
FAKE-tagged, not reverted to natural order.

### E3 — `src/dra/7E4BC.c:1561-1564` (same function as E2)

```c
prim->x0 = prim_x0_x2 + (posX_hi + prim->u0);   // ×4 (x0..x3)
```

Explicit NON-left-associative grouping of a 3-term commutative sum. The parens
have no precedence function; the natural spelling is the unparenthesized
left-assoc chain. Shipped SILENTLY (no annotation), fully matched. This is
the reparenthesization lever itself, on `+` instead of `|`.

### E4 — `src/dra/game_handlers.c:1251-1254` (file fully matched)

```c
prim->x0 = sx + (posX + prim->u0) - 0x80;   // ×4 (x0..x3)
```

Same construct as E3: explicit right-grouping of the middle pair of a 4-term
commutative sum, and the varying term (`sx`) placed FIRST ahead of the base
position — an ordering no "natural reading" produces. Silent, matched.

### E5 — adjacent-family: `src/st/nz1/e_boss_doors.c:232-235` (file fully matched)

```c
offsetY = g_CurrentEntity->posY.i.hi + self->posX.i.hi + g_Tilemap.scrollY.i.hi;
offsetY = 208 - offsetY; // critical, all in one line breaks regalloc
```

Statement split chosen purely for register allocation, annotated. Same species
(source shape exists only for codegen), different axis.

### Natural-order sites (counter-evidence detail)

- `src/st/dai/e_stained_glass.c:76` (matched): `((mask | (r)) | (g << 5)) | (b << 10)`
  — natural ascending bit order; redundant LEFT-assoc parens only.
- `src/st/st0/2DAC8.c:190` (matched): `((t | (tempR)) | (tempG << 5)) | (tempB << 10)`
  — natural order; note the surrounding statement interleaving (g-load between
  r-compute stages, lines 181–189) is itself scheduling-shaped source, silent.
- The cutscene nibble-swap family (~10 sites, e.g. `st/no4/cutscene.c:647`):
  `(dst & 0xF0) | ((src & 0x0F) << 4)` — natural in-place order throughout.
- `src/st/lib/e_shop.c:907` (matched): `ch = *str++ | (ch << 8);` — unshifted
  operand first; no unique natural order exists for this idiom, listed for
  completeness.

## BB2 in-repo norm (step 5)

Landed COMPLETED-C spellings (0 regfix/asmfix rules verified per function):

- `src/display.c:600` **get_ofs** — `return var_v1 | (var_v0 | new_var2);` —
  explicit RIGHT-grouped or-tree, 0 rules. The right-grouping precedent already
  ships in BB2's own corpus.
- `src/display.c:428/506/514` **SetDrawEnv** — `(((b27<<16) | 0x60000000) | (b26<<8)) | b25`
  — descending byte order (27,26,25 vs memory order 25,26,27) with the constant
  spliced mid-tree, 0 rules.
- `src/display.c:134` **ClearImage** — `(arg3<<16)|(arg2<<8)|arg1` — descending
  vs argument-declaration order (r,g,b args packed b,g,r). PsyQ-provenance order.
- `src/gpu.c:327` **GetTPage** — operand order matches the original PsyQ
  getTPage macro verbatim (neither ascending-bit nor by-argument): "natural" is
  defined by original-source provenance, not by a single canonical sort.
- Counter: `src/code6cac.c:525` (func_80019488) and `src/code6cac_b.c:2650`
  (cpu_check_same_dir_timer) ship ascending natural order.
- EXCLUDED as precedent: `src/text1a_post.c:144` (`b | ((r<<16)|(g<<8))`) is the
  anchor function itself in its legacy 3-rule representation; `src/text1b.c:502`
  is func_80048864 (36 rules, queued).

## Counter-evidence summary

For the anchor's EXACT shape (RGB or-tree over byte loads) SOTN-master offers
NO deviating exhibit: 2 live sites, both natural order (0/2 deviating, 0
annotated). Anyone wanting to say "SOTN never reorders an RGB or-tree" is
technically correct — because SOTN barely writes them (the prims are assigned
per-field, not packed). The base rate of the construct is near zero, so absence
of deviation there carries little inferential weight.

## Verdict draft

**SUPPORTED at family level; ABSENT for the exact or-tree shape.** SOTN-master
fully-matched PSX code demonstrably ships commutative term orderings and
non-natural groupings whose only function is to match codegen: one gold
annotated exhibit where the PSX spelling reorders additive terms against the
PSP port's natural spelling with an explicit "compiler ordering / can't find
the single solution" comment (E1, e_warg.c), one annotated non-natural
color-channel ordering kept as-is (E2, "weird RBG assign order"), and eight
SILENT explicit right-groupings of commutative sums — the reparenthesization
lever itself on `+` — in two fully-matched dra files (E3/E4). The exact
anchor shape (multi-shift RGB or-tree) is too rare in SOTN (N=2, both natural)
to yield direct precedent either way, so the exact-shape row reads ABSENT, not
CONTRA. BB2's own landed corpus already contains an explicit right-grouped
or-tree in a zero-rule completion (get_ofs) and provenance-ordered descending
packs (SetDrawEnv, ClearImage, GetTPage), i.e. the in-repo norm already accepts
"the order the original source had," not "a canonical natural order." Net: the
evidence supports ruling that a SINGLE justified target-matching operand
order/grouping is acceptable for this family — SOTN's observed practice is to
ship it, annotating it with a comment when the ordering was found by trial
rather than derived (E1's comment is the model), and silently when mild
(E3/E4). It does NOT support unbounded enumeration: every SOTN exhibit is one
committed ordering with identical runtime semantics, not a search artifact
left uncommented where non-obvious.

## Method note

Greps run from `tmp/sotn-decomp` (patterns: `<<.*\|.*<<`, ` \| \(\w+ \| `,
` \+ \(\w+ \+ `, keyword sweeps for order/swap/reorder/commut/operand comments,
plus the pre-built construct index's fake_comment/match_comment classes).
Match status via per-file `INCLUDE_ASM` count; BB2 rule status via
`grep -c '^<fn>:' regfix.txt asmfix.txt` + queue.json membership.

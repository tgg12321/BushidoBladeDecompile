# The oracle compiler — identity, derivation, and reproduction

**Status: REPRODUCIBLE.** The compiler the oracle match depends on is built
from committed inputs by a committed script. It is no longer a binary that
exists only as a file on one disk.

> **SUPERSEDED 2026-09-26 (owner ruling Q17, ninth batch): a compiler patch
> is a cheat.** Any modification of the build compiler that changes its
> output is a cheat. The narrow PLUS->IOR patch below is output-changing and
> is superseded. The reorg.c `negate_rtx` crash fix (ruling 262930f1b) is kept
> as a host-build fix, as the author's reading, open for the owner to
> overrule; any new host fix needs its own owner ruling. Rule text:
> `.claude/rules/no-compiler-divergence.md` § "Owner ruling 2026-09-26
> (ninth batch, Q17)". The rest of this document is history.
>
> **Adopted 2026-09-25 (owner ruling `bcdc1648e`, second batch).** The
> narrowed PLUS->IOR condition replaced the old no-rewrite patch. The
> per-site record the ruling requires is in § "Adoption record (2026-09-25)"
> under the OPEN QUESTION.

```
upstream : decompals/mips-gcc-2.7.2 @ 43d1cdb67ed135879869b5266f01efaaada5e35a
patch    : tools/cc1-plus-to-ior-narrow.patch     (combine.c: no PLUS->IOR
           rewrite of (plus REG CONST_INT); +6 -1)
crashfix : tools/cc1-reorg-negate-rtx-decl.patch  (reorg.c: the 2026-08-24
           host-ABI negate_rtx declaration; +9, applied in every mode)
recipe   : every object at -O0 (`-g`), combine.o ALONE at `-O`,
           `-fgnu89-inline` throughout
build it : bash tools/build_oracle_cc1.sh              # verify in scratch
           bash tools/build_oracle_cc1.sh --install    # then swap it in
```

`tools/gcc-2.7.2/build/cc1` (Makefile:12) is the operative binary and is
**exactly what that script produces**. `tools/gcc-2.7.2/` is gitignored, which
is why the patches are tracked in `tools/` rather than applied in place. The
script copies the tree to scratch, restores it to the pinned commit, applies
the patches there, and never modifies the live tree.

The binary's own SHA1 is **not** the invariant. Host-toolchain metadata leaks
into it across environments, although on this host the recipe rebuilt
`aa04d761…` identically twice. The invariants are `simplify_rtx`'s size
(`0x3a6a`) and, decisively, the assembly emitted for the project's TUs. The
script's self-check asserts the latter. A full `engine verify-oracle --rebuild`
against `62efab4f73f992798c43e8c730aa43baa10bb4fa` remains the authoritative
gate.

| binary | SHA1 | what it is |
|---|---|---|
| operative `build/cc1` | `aa04d7619cd79215788d18d6870ebc6c8d1d822d` | recipe output, narrow + crash fix (2026-09-25 adoption) |
| `build/cc1.PRE-RECIPE-0f438e42` | `0f438e42548d29798db86d50a76e54bd6f04b64a` | retired no-rewrite compiler with the crash fix. Operative 2026-08-24 to 2026-09-25. The self-check's no-rewrite reference. |
| `build/cc1.PRE-CRASHFIX-045c9543` | `ea11be50d12f24c464123b705c29007efc04e4a8` | recipe output of the 2026-08-07 swap, operative until 2026-08-24. **The suffix is wrong**: the file is `ea11be50`, not `045c9543`. It is kept under that name because `oracle/manifest.json` `notes.cc1_build_2026_08_24` cites it. |
| `build/cc1.PRE-RECIPE-045c9543` | `045c9543d39ab8109583b92137c7adde084f7a25` | the historical 2026-05-18 binary. Segfaults on 5 current TUs (no crash fix). |
| `build/cc1.ORACLE-BACKUP` | `045c9543d39ab8109583b92137c7adde084f7a25` | same, kept as a backup |
| off-tree backup | `045c9543d39ab8109583b92137c7adde084f7a25` | `C:\Users\Trenton\bb2-oracle-cc1-backup\cc1.oracle-compiler-045c9543` |
| diagnostic `cc1` | `888dda6755596e3ac826c38c36be3103297cbac8` | hooks + the same narrow patch; the live `reorg.c` carries the crash fix (see below) |
| `cc1.PRE-PATCH-4096c6fd` | `4096c6fddbc4125a2100507e1e0ac08e289008aa` | the diagnostic it replaced. It was no-rewrite and predated the crash fix, and segfaulted on 5 current TUs. |
| `cc1.PRE-PATCH-8384fd47` | `8384fd47cb51da369462a0ba0b83590eea88513a` | the unpatched diagnostic before that |

On the tree as adopted, the narrowed compiler and the retired no-rewrite
compiler emit **identical** assembly on all 34 TUs. The sites where they
differ, func_80073C78's `+` body and site C's natural spelling, need source
that does not exist yet (study § 1.1). That equivalence is why the adoption
changed no source file.

**Verify the backups periodically — do not assume they persist.** The off-tree
backup has been found missing twice, both times after it had been created and
hash-verified (once by the owner, once during the 2026-08-07 prep). Nothing
explains the disappearances yet. Treat that path as unreliable until something
does: re-check it with `sha1sum` rather than trusting its last known state,
and keep the in-tree copies as the working redundancy.

## What the patch does, and what it does not claim

Stock GCC 2.7.2 rewrites `a + b` into `a | b` whenever it can prove the
operands share no bits. The adopted patch keeps that rewrite, **except** when
the addition is a plain register plus a constant, `(plus REG CONST_INT)`.
There the compiler emits `addu`/`addiu` where stock emits `or`/`ori`. Every
other disjoint-bits addition is rewritten as stock does.

The condition is the **best fit to the evidence**
([ORACLE-COMPILER-STUDY-2026-09-25.md](ORACLE-COMPILER-STUDY-2026-09-25.md)).
It is **not** a recovered historical compiler. The full build links to the
original executable's SHA1, and the condition explains every compiled
discriminating site:
- site A keeps its `addiu`;
- site C's natural spelling matches;
- func_80073C78's `+` body gets its `ori`.

The study's `exprop` condition also fits and stays a live alternative. See
the open question below.

**History of the patch slot.** The previous patch,
`tools/cc1-no-plus-to-ior.patch` (removed 2026-09-25), deleted the rewrite
outright. Its origin was accidental: a 2026-05-18 compiler-patch experiment
whose output binary became the project compiler and was never reverted. The
2026-08-07 forensics
([docs/grind/cc1-forensics-2026-08-07.md](grind/cc1-forensics-2026-08-07.md))
identified it. It was retained, match-proven, on the SOTN pattern (below),
until the 2026-09-25 study showed that it was wrong at two sites.

### Why the compiler is a pinned, patched build (owner election, `d99ab6a6`)

This follows the SOTN pattern. That project's `cc1-psx-26` is itself a
**patched** GCC, built from a pinned `decompals/old-gcc` commit and
distributed as a hash-verified binary. The community bar is *visible, pinned,
reproducible*, not *unpatched stock*. With the patches tracked, the upstream
commit pinned, and the recipe scripted, this project meets that bar.

Removing the patch was the original 2026-08-07 election. It was revised on
Phase-0 evidence, below.

### Scope note — `no-compiler-divergence`

The adopted PLUS->IOR patch (owner ruling `bcdc1648e`) is that rule's
**only** PLUS->IOR amendment. The 2026-08-24 crash-fix declaration stays in
the recipe under its own owner ruling (`262930f1b`). It is not a codegen
change. Neither is a license to patch the compiler again. Any further
divergence is still forbidden, and the rule's reasoning is otherwise
unchanged.

## OPEN QUESTION — did the original compiler perform this conversion?

**Undetermined.** Evidence points both ways, and the project does not assert
an answer. What is settled is that the patched compiler reproduces the
original executable byte-for-byte; that is the only claim the build rests on.

Evidence that the original compiler did **not** perform it:
- The shipped game contains `andi rX,rY,M ; addiu rZ,rX,C` with `M & C == 0`
  at two independent sites — the shape a transform-performing compiler
  rewrites to `ori`. One of them (`func_80079A30`, `(x & 7) + '0'`) was raw
  `INCLUDE_ASM` when this was written, so no C reconstruction could be blamed
  for it. (It has since landed as C, `sprintf` in `src/text1b_b.c`. The
  target bytes are unchanged: `asm/funcs/sprintf.s` 0x80079F0C `andi v0,a0,0x7`
  ; `addiu v0,v0,0x30`.)
- At the real site in `gnd_disp_loop_ctrl`, 21 C spellings were tested and all
  emit `ori` under stock GCC **and** under PsyQ's own `cc1psx` run on the
  actual TU. The current C cannot produce the shipped bytes under either.
- The multiply idiom is 571 `sll;addu` to 0 `sll;or` across the game (weaker:
  most of those are not provably disjoint).

Evidence that it **did**:
- At site C (`main` in `ings`) the original matches stock's codegen, and the
  patched compiler over-folds (at the time this was worked around by build-time
  rules; the function's C was admittedly unfaithful, so the signal is muddy).
- The forensics' isolated probes had `cc1psx` performing the conversion on
  every reachable shape.
- (2026-09-25) `func_80073C78` in `text1b`: the shipped bytes contain the
  conversion (`ori rX,rY,0` at the UV stores). A stock recipe build compiles
  the natural `u + du0` body to exactly those bytes. Across the TU up to that
  function, the only difference from the oracle compiler is those two lines.
  The oracle compiler cannot reach them from any `+` spelling. Evidence:
  `memory/grind/func_80073C78/evidence.md`; owner question logged in
  `docs/grind/borderline.md` (2026-09-25).

Anyone reopening this should start with the scan scripts named in the
forensics doc and the Phase-0 evidence recorded at `d99ab6a6`.

Study (2026-09-25, scratch-only, NOT adopted):
[ORACLE-COMPILER-STUDY-2026-09-25.md](ORACLE-COMPILER-STUDY-2026-09-25.md).
It reports a partial answer: a narrowed rewrite, which skips only
`(plus REG CONST_INT)`, fits every compiled site. The report also corrects
two entries in the evidence lists above: sprintf is neutral, and site C's
natural spelling needs the rewrite.

### Adoption record (2026-09-25, owner ruling `bcdc1648e`)

**Adopted condition: narrow**, meaning no PLUS->IOR rewrite of
`(plus REG CONST_INT)`, in `tools/cc1-plus-to-ior-narrow.patch`. It is the
**best fit to the evidence, not a recovered historical compiler**.

**Live alternative: `exprop`.** `exprop` rewrites only when some operand is
neither a REG nor a CONST_INT, or when a REG operand is known to be zero. It
fits every compiled site, as narrow does. The two differ only on a disjoint
register+register sum whose single use is a return value, a call argument or
a copy (§ "Scan result" above). Under ruling item (D):
- A later site that discriminates under (B) reopens the choice. It is
  recorded here and logged to `docs/grind/borderline.md` as a
  `policy-question`.
- Switching needs a fresh owner ruling.
- Until then, the function with that site stays INCOMPLETE.
- No source-level workaround for either condition is admitted.

**(A) Scan record.**
- Scanner: `tools/rrscan_plus_ior.py`, committed. Run
  `python3 tools/rrscan_plus_ior.py <out.tsv>` from the repo root.
- Method: for every `addu`/`or` with two register operands in
  `asm/funcs/*.s`, it traces each operand's producer backward (`andi`,
  `lbu`/`lhu`, `sll`/`srl`, `slt*`, `lui`/`li`, `and`/`or`/`xor`, moves). It
  derives a nonzero-bits mask from that producer and keeps the site when the
  two masks are disjoint. It flags a site XBLOCK when the trace crosses a
  label or branch, and it classifies the consumer of the result.
- Site list: `docs/ORACLE-COMPILER-RRSCAN-2026-09-25.tsv`, 547 sites. Each
  row gives the site's class (kept `addu` or `or`), its function's state
  (C or INCLUDE_ASM, as of this adoption), and its (B) verdict.
- Totals by function state:

  | state | class | sites | (B) verdict |
  |---|---|---|---|
  | C | `addu` | 118 | not discriminating |
  | C | `or` | 141 | not discriminating |
  | INCLUDE_ASM with a ledger candidate | `addu` | 2 | not discriminating |
  | INCLUDE_ASM with a ledger candidate | `or` | 29 | not discriminating |
  | INCLUDE_ASM without C | `addu` | 105 | **pending** |
  | INCLUDE_ASM without C | `or` | 152 | **pending** |

  - Sites in COMPLETED-C functions are not discriminating. The committed
    tree compiles to identical asm on all 34 TUs under narrow, under
    `exprop` and under the retired no-rewrite compiler.
  - Pending sites become (B) tests under (D) once their function's C
    exists.
  - `tmp/rrscan/report.md`, the pre-adoption run, found the same 547 sites.
    Since then only func_800759D0's two sites have moved, from
    INCLUDE_ASM to C.

**Per-site (B) check before adoption.** Every scanned function with a
`memory/grind/<func>/candidate.c` was compiled under both conditions:
- narrow as the fixed recipe binary;
- `exprop` as the study compiler's run-time policy.

Each ledger candidate was spliced over its INCLUDE_ASM line with nothing else
written. Committed C was compiled from its TU as it stands. The functions:

| function | state | sites | narrow vs `exprop` | verdict |
|---|---|---|---|---|
| func_8001BE20 | INCLUDE_ASM + candidate | 16 | identical | not discriminating |
| func_800204C0 | INCLUDE_ASM + candidate | 1 | identical | not discriminating |
| func_8002DAD0 | INCLUDE_ASM + candidate | 2 | identical | not discriminating |
| **func_8005D554** | INCLUDE_ASM + candidate | 1 | identical | not discriminating |
| func_80073C78 | INCLUDE_ASM + candidate | 1 | identical (both emit the target `ori` pair; the retired compiler did not) | not discriminating |
| func_8008B488 | INCLUDE_ASM + candidate | 10 | identical | not discriminating |
| PutDispEnv, _SsVmKeyOnNow, func_8002CD58, func_8002EBDC, func_80031B24, func_8003FA24, func_80053E9C, func_8006295C, func_80063E10, func_800678A8, func_8006B578, func_8006D808, func_8006ECF4, func_80073728, func_800759D0, func_8007636C, prnt, vmNoiseOn | committed C (each also has a ledger candidate file) | 2–6 each | identical | not discriminating |

Under (C): **no site discriminates, so narrow is adopted** and `exprop` stays
the live alternative above.

**(E) steps.** The manifest, `--stock` self-check and binary table are
re-recorded (see § "Tracked state manifest" and the table at the top). The
patch has been replaced. `build/cc1` and the diagnostic were rebuilt from the
recipe. Verification:
- `verify-oracle --rebuild`: `62efab4f73f992798c43e8c730aa43baa10bb4fa`,
  drift empty after the re-lock;
- `engine test`: 720/720;
- `fixtures-verify`: 5/5;
- `check_completion_integrity.py`: OK;
- `oracle/manifest.json`: re-locked, with a `notes.cc1_build` provenance
  entry.

The toolchain fingerprint moved from `69f8ba78e94cf257` to `85cd2a73ee7a8972`.
`queue auto-return` runs once this change has landed, on a clean tree,
because queue operations can revert uncommitted tracked edits. The items it
returns are recorded in the commit that follows.

### Owner ruling 2026-09-25 — keep the patch; a narrower patch may be STUDIED, not adopted

**Question and answer.** After the 2026-09-25 manual-lane run, the owner asked
the operator for recommendations on four open questions in
`docs/grind/borderline.md`. This one is the 2026-09-25 entry "func_80073C78 —
compiler PLUS->IOR patch vs target `ori`". The question put to the owner,
verbatim: "Our compiler has one deliberate change from standard GCC: it never
turns `a + b` into `a | b`. This function's original machine code contains
exactly that rewrite, and standard GCC reproduces it from the natural `+`
code. Should we revisit that compiler change, or keep it? If we keep it, this
function stays unfinished rather than landing with a deliberately odd `|` in
the C." The operator's recommendation, verbatim:

> **Recommendation: keep the patch for now, don't allow the `|` spelling, and
> approve a study of a narrower patch.**
>
> `docs/ORACLE-COMPILER.md` records evidence both ways. At some sites the
> original game kept `addu`, which only our patched compiler produces; stock
> GCC and Sony's own cc1psx both emit `ori` there. At other sites, including
> func_80073C78 and func_80079A30, the original has `ori`, which our patched
> compiler can't produce. So the original compiler applied this rewrite in
> some cases and not others. Neither "always rewrite" (stock) nor "never
> rewrite" (our patch) is correct.
>
> Switching back to stock would break the places that currently match.
> Writing `|` in the C is the workaround the reviewer rightly rejected. The
> better route is a study of what separates the two groups of sites, for
> example how the compiler proves the two operands share no bits. If one clear
> condition explains every site, a narrower patch would make the compiler
> more faithful, not less.
>
> That study would be a new compiler change, and the no-divergence rule
> reserves those for you. The oracle would check it across the whole game.
> Until then, func_80073C78 stays rotated.

Owner (Trenton), verbatim: **"Go ahead and do your recommendations then"**.

**Correction to the recommendation (recorded 2026-09-25, before anything
spends this ruling).** The recommendation lists func_80079A30 among the
sites where the original has `ori`. It does not. This document lists it under
evidence that the original did NOT perform the rewrite, and the target has
`andi v0,a0,0x7 ; addiu v0,v0,0x30` (`asm/funcs/sprintf.s`, 0x80079F0C). The
known `ori` site is func_80073C78, and site C (`main` in `ings`) points the
same way with the caveat recorded above. In the author's judgment this does
not change what the owner approved: the sites still split both ways, so the
recommendation's premise holds.

**Rule text.** This is the author's narrowing of that recommendation, not the
owner's words.

- **(A) The patch stays.** `tools/cc1-no-plus-to-ior.patch` remains the build
  compiler's only divergence from upstream. `tools/gcc-2.7.2/build/cc1`, the
  recipe, the Makefile and `CC_FLAGS` are unchanged.
- **(B) The `|`-for-`+` spelling stays refused.** C that writes `|` where the
  program means `+`, so as to reproduce stock GCC's rewrite under the patched
  compiler, is a source-level workaround for the patch. The func_80073C78
  layer-2 FAIL (2026-09-25, `rejected/ior-spelling-patch-workaround.c`) stands.
- **(C) A STUDY is authorized, in scratch only.** Its question: what condition
  separates the sites where the original kept `addu`/`addiu` from the sites
  where it emitted `or`/`ori`? For example, how combine proves the operands
  share no bits. Limits:
  - Compiler variants are built outside the repository only (e.g. the WSL home
    directory, as `~/cc1stock` was for func_80073C78), from the pinned
    upstream `43d1cdb6` plus the variant's `combine.c` change. They are never
    installed.
  - Nothing in the repository changes: `tools/gcc-2.7.2/build/cc1`, the
    diagnostic `tools/gcc-2.7.2/cc1`, the live `tools/gcc-2.7.2/` sources (the
    tracked-state manifest above stays as recorded), the Makefile,
    `CC_FLAGS`, `engine/buildconfig.py`, and every other build input.
  - Full builds with a variant run on a scratch copy of the tree, never on the
    repository's `build/`.
  - No function lands on a variant's output. No `src/` change may be
    motivated by, or measured only under, a variant. No study result may be
    cited as grounds for a completion or a construct ruling.
  - This is the study the recommendation says the no-divergence rule reserves
    for the owner. `.claude/rules/no-compiler-divergence.md` items 1 and 4
    stand for everything outside it.
- **(D) ADOPTING a narrowed patch is NOT authorized by this ruling.** Any
  candidate returns to the owner with this evidence:
  - the variant as a diff against upstream `combine.c`;
  - an oracle-green full build (SHA1
    `62efab4f73f992798c43e8c730aa43baa10bb4fa`) of the unchanged current tree
    under it;
  - a per-site table that explains EVERY known site class in this document's
    evidence lists above, with the proposed condition evaluated at each site:
    - the two `andi ; addiu` sites (gnd_disp_loop_ctrl site A, and
      func_80079A30);
    - the multiply idiom (e.g. 0x80033F74);
    - site C (`main` in `ings`);
    - the forensics' isolated `cc1psx` probes;
    - func_80073C78's two UV stores;
  - whether func_80073C78's natural `+` body then reaches 0.

  A site the condition does not explain is reported as unexplained, never
  omitted. Adoption would change the oracle compiler, so it needs its own
  owner ruling, landed before any code spends it. (That ruling is
  § "Owner ruling 2026-09-25 (second batch)" below. It rests on the study
  report, ORACLE-COMPILER-STUDY-2026-09-25.md, which supplies this evidence
  list, and it adds a scan.)
- **(E) func_80073C78 stays rotated** at its honest floor (2/362, the `+`
  body in `memory/grind/func_80073C78/candidate.c`) until an owner ruling
  under (D) lands or an honest spelling is found.

Practical note (not part of the ruling): per the 2026-09-25 borderline entry,
the live `tools/gcc-2.7.2/reorg.c` no longer matches this document's manifest,
so `tools/build_oracle_cc1.sh` refuses to run. The study must not work around
that by re-recording the manifest. Restoring the tree, or re-recording it with
a rationale, is a separate change. (The second-batch ruling below makes
re-recording it, with a rationale, adoption step 1.)

Record: docs/grind/decisions.md 2026-09-25 OWNER RULING — oracle compiler.

### Owner ruling 2026-09-25 (second batch) — adopt the narrowed condition, after a register-plus-register scan

**Question and answer.** The study authorized above landed as `ac5d2b0bb`
([ORACLE-COMPILER-STUDY-2026-09-25.md](ORACLE-COMPILER-STUDY-2026-09-25.md)).
Its § 9 asks the owner whether to switch the project's compiler to the
narrowed condition, or keep the current patch and re-test the narrowed one as
more functions become C. After the first batch was carried out, the operator
reported three open decisions and the owner asked, verbatim, "What are your
recommendations?". The operator's recommendation on this one, verbatim:

> ## 2. Compiler narrowing: adopt it, after one cheap check
> **Recommendation: yes, but first scan the game's own machine code for
> register-plus-register sites.**
>
> The current patch ("never rewrite") is now known to be wrong in two places.
> func_80073C78 can't be matched at all, and `main` needed an artificial
> workaround in the source (granted 2026-08-11). The narrow rule reproduces
> the whole game and gets every known site right. Both give the exact oracle
> build, so adopting it can't break the match, and it's more faithful than
> what we have.
>
> The weak spot is that a second rule (`exprop`) also fits, and the two differ
> only on register-plus-register sums. So before adopting, scan the target
> binary for register-plus-register `or` and `addu` sites whose operands
> provably share no bits (for example, one side comes from `andi` and the
> other from `sll`). If such a site exists, it decides between the two rules.
> If none exists, adopt narrow and record `exprop` as a live alternative, with
> any future discriminating site reopening the choice.
>
> Adopting would then involve:
> - updating the out-of-date compiler manifest and fixing the `--stock`
>   self-check;
> - committing the patch to `tools/`;
> - rebuilding the build compiler from the recipe;
> - confirming the full oracle, engine tests and fixtures.
>
> The compiler change automatically re-measures every rotated function. Then
> func_80073C78 lands from its natural `+` code, and `main`'s workaround can
> be replaced with the natural spelling.

The same recommendation also listed, among the smaller items it would handle
without a ruling, verbatim:

> - **Out-of-date compiler manifest:** re-record the crash fix and the current
>   `build/cc1` hash, and fix the `--stock` self-check. This is paperwork; the
>   build doesn't change.

Owner (Trenton), verbatim, answering all three recommendations together:
**"Go ahead with your recommendations"**.

**Rule text.** This is the author's narrowing of that recommendation, not the
owner's words. The two candidates are the study's (§ 3):
- **narrow** (`noregconst`, fixed-binary form) does NOT rewrite
  `(plus REG CONST_INT)`, and otherwise rewrites as stock does. It is the
  study's § 4 patch, `tmp/pior/cc1-plus-to-ior-narrow.patch`, reproduced in
  full in that section.
- **`exprop`** rewrites only when some operand is neither a REG nor a
  CONST_INT, or a REG operand is known to be zero.

- **(A) The gating scan comes first.** Before anything is adopted, the
  shipped binary (`asm/funcs/*.s`) is scanned for register-plus-register `or`
  and `addu` sites whose two operands provably share no bits (for example,
  one side from `andi` and the other from `sll`). The adoption commit records
  in this document:
  - the scanner and its method, committed under `tools/` or described well
    enough to reproduce (`tmp/` is gitignored);
  - every site found, with its class (kept `addu` or `or`) and the state of
    its function (C, or INCLUDE_ASM);
  - the verdict under (B) for each site.
- **(B) What counts as a discriminating site (the author's narrowing).** A
  site discriminates only when compiling the same C at that site gives
  different bytes under narrow and under `exprop`, and exactly one of the two
  equals the shipped bytes. The C is the function's committed C, or its
  ledger's honest candidate (`memory/grind/<func>/candidate.c`) with nothing
  written for the test.

  A shipped-binary site with no such C does not discriminate by itself. A
  kept `addu` may be a rewrite that combine tried and undid: under stock, 280
  of 281 rewrites never reached the final code (study § 2). An `or` may be
  source `|` (study § 1.3). Such sites are recorded as **pending**, and each
  becomes a future discriminating site under (D) once its function's C
  exists. The study already names candidates: func_80067D14, func_80063084,
  func_800198D0, func_8003993C, func_80048FFC, func_8005E54C and
  func_800693CC.
- **(C) Which condition is adopted.**
  - If at least one site discriminates, and every discriminating site selects
    the same candidate, that candidate is adopted.
  - If no site discriminates, narrow is adopted, and `exprop` is recorded as a
    live alternative in the OPEN QUESTION section.
  - If discriminating sites disagree, or a site matches neither candidate,
    nothing is adopted and the evidence returns to the owner. The owner's
    words do not cover this case; this is the author's narrowing.

  The adopted condition is one of the two exactly as defined above. Any other
  condition, or any change to either, is not authorized. `exprop` so far
  exists only as a run-time policy of the instrumented study compiler. If it
  is selected, it is first written as a fixed patch file, and that patch must
  pass what narrow passed (study § 3): criteria (a)-(c), plus full scratch
  builds to the oracle SHA1 of the current tree, and of the current tree
  with func_80073C78's candidate spliced in.
- **(D) The choice stays open.** After adoption, the OPEN QUESTION section
  records the adopted condition as the best fit to the evidence, not a
  recovered historical compiler, and names the other candidate as a live
  alternative. A later discriminating site under (B) reopens the choice. It is
  recorded in the OPEN QUESTION and logged to `docs/grind/borderline.md` as a
  `policy-question`, and switching needs a fresh owner ruling. Until that
  ruling, the function whose site discriminates stays INCOMPLETE. No
  source-level workaround for the adopted condition is admitted, in either
  direction; ruling (B) of the first 2026-09-25 ruling above stands.
- **(E) Adoption steps. Each is required.** Every step lands in the adoption
  change and is verified oracle-green.
  1. **Manifest.** Re-record the tracked-state manifest with a rationale:
     `reorg.c` carrying the 2026-08-24 crash-fix block (owner ruling
     `262930f1b`, adopted in `fea9fa2ac`; study § 7), and the current
     `build/cc1` hash. Correct the binary table as needed, including the
     backup whose `.PRE-CRASHFIX-045c9543` suffix names the wrong hash. Fix
     the `--stock` self-check in `tools/build_oracle_cc1.sh` to expect the
     divergence it actually measures; the study measured site A only, in
     `ings`. The author's reading: study § 7 found that the recipe applies
     only the PLUS->IOR patch, so a rebuild "from the recipe" would drop the
     owner-approved crash-fix declaration. The recipe therefore applies that
     declaration too, from a tracked input under `tools/` that reproduces exactly the reorg.c block adopted in `fea9fa2ac`, and nothing else. This is not a new
     divergence; it records one the owner already approved.
  2. **Patch.** Commit the adopted patch under `tools/`, replacing
     `tools/cc1-no-plus-to-ior.patch`. Its header states the adoption and
     cites this ruling, in place of "CANDIDATE ONLY -- NOT ADOPTED". Update
     every reference to the old file, and re-record the `simplify_rtx` size
     invariant (the study measured 0x3a6a for narrow).
  3. **Rebuild.** Rebuild `tools/gcc-2.7.2/build/cc1` from the recipe
     (`bash tools/build_oracle_cc1.sh`, then `--install`). Also rebuild the
     diagnostic compiler with the same patch (`tools/build_diagnostic_cc1.sh`),
     so the two still agree on every TU as § The diagnostic compiler requires.
     The diagnostic rebuild is the author's addition. Record both new hashes
     in the manifest.
  4. **Verify.** Run `verify-oracle --rebuild` (SHA1
     `62efab4f73f992798c43e8c730aa43baa10bb4fa`), `engine test`, and
     `fixtures-verify`. Re-lock `oracle/manifest.json`'s toolchain record with
     a `notes.cc1_build` provenance entry, as `fea9fa2ac` did for the crash
     fix. The re-lock is the author's addition, following that precedent.
  5. **Auto-return.** The cc1 change moves the toolchain fingerprint, so
     `queue auto-return` re-measures every rotated candidate. The adoption
     records that it ran and which items returned.

  A layer-2 cheat-reviewer reviews the adoption diff before it lands. It
  checks that the committed patch is the selected candidate, that nothing
  else changes compiler behaviour, and that the scan record and steps 1-5 are
  complete. The review is the author's addition.
- **(F) Afterwards.** Once adoption has landed:
  - func_80073C78 may land from its honest `+` body;
  - `main`'s FAKE chain, granted 2026-08-11
    (`.claude/rules/chained-accumulation-fake-exception.md`), may be replaced by the natural
    spelling.

  Each is an ordinary completion or cleanup commit with its own layer-2
  review. Neither is part of the adoption, and this ruling decides neither
  outcome. The chained-accumulation family itself is unchanged.
- **(G) What does not change.** Beyond the swap of the one PLUS->IOR patch
  (and the recipe carrying the already-approved crash fix),
  `.claude/rules/no-compiler-divergence.md` stands. No other compiler,
  Makefile or `CC_FLAGS` change is authorized. Any further study stays under
  item (C) of the first 2026-09-25 ruling above.

**Scan result (2026-09-25): NO-DISCRIMINATING-SITE.** No site in the shipped
binary currently decides between narrow and `exprop`. Under (C), narrow is selected, provisionally. Before adoption, the adoption commit compiles under both narrow and exprop every scanned site whose function has a `memory/grind/<func>/candidate.c` (at least func_8005D554), and records a per-site (B) verdict. If any of those sites discriminates, (C) applies to that result instead. `exprop` is recorded as a live alternative. The ruling
above does not depend on this result. The adoption commit records the scanner
and its site list in-tree, as (A) requires.

- **Where the rules differ.** They produce different bytes for a disjoint
  register+register sum (neither operand zero) only when the sum's single use
  is one of three things. In each case narrow emits `or` and `exprop` keeps
  `addu`:
  - a return value;
  - a call argument;
  - a copy, including a copy into a `u16`/`u8` local (site A's mechanism).

  Any other consumer leaves both rules at `addu`: arithmetic, a store, a
  compare, a direct `s32` assignment, a `(u16)` cast, `& 0xFFFF`, `<< 16`, or
  a chained sum. Study-compiler probes confirmed each case.
- **Why no site decides.**
  - The COMPLETED-C tree compiles identically under both rules (study § 3,
    criterion (a)), so only INCLUDE_ASM functions could discriminate.
  - The 14 in-block disjoint register+register `addu` sites in INCLUDE_ASM
    functions include none consumed as a return value, call argument, copy,
    or `sh`/`sb`.
  - None of the 181 `or` sites (166 in-block, 15 cross-block) is consumed
    that way.
  - The cross-block `addu` hits are almost all loop counters wrongly flagged
    as zero. The rest give `addu` under both rules: multiply idioms
    (func_800198D0, func_80048FFC, func_800693CC, func_8005D554,
    func_80023F08), chained byte packing (func_80067D14 @0x80068B7C and
    @0x80068BB0; func_80063084 @0x80063808), and the `<< 16` sibling form
    (func_80063084 @0x8006335C).
- **Sites to re-check when each function is worked:**
  - func_800620B8 @0x800624B4;
  - func_800646E8 @0x80064908;
  - func_80065800 @0x80065F2C.

  Each computes `((p[0]>>4)&0x3F) + (p[1]<<6)`, then `andi 0xFFFF`, then
  `sw`. Their COMPLETED-C siblings write `+`. Under the oracle compiler, four
  spellings reproduce the target: a `(u16)` cast, `& 0xFFFF`, and two
  `u16`-local variants. Under narrow, only the `u16`-local spellings become
  `or`. So these sites favour `exprop` only if the function's honest C needs
  a `u16` temporary. If it does, the choice reopens under (D).
- **Limits.** Confidence that no shipped site decides is moderate; confidence
  that the COMPLETED-C tree cannot decide is high.
  - The scanner infers each operand's producer by walking back linearly
    through the assembly block. It can miss disjointness that GCC proves from
    a variable's other assignments.
  - The probes were standalone files built at -G0, not the real TUs.

Record: docs/grind/decisions.md 2026-09-25 OWNER RULING — oracle compiler
adoption (second batch).

## The diagnostic compiler

`tools/gcc-2.7.2/cc1` is the same GCC carrying the `BB2_*_DEBUG` hooks that
`ra_solver` / `sched_solver` read their dumps from. It is built from the
hooked live sources **plus the same PLUS->IOR patch** as the oracle
(`tools/cc1-plus-to-ior-narrow.patch` since 2026-09-25), so it agrees with
the oracle on all 34 TUs. The live `reorg.c` already carries the crash-fix
declaration. Rebuild it with `bash tools/build_diagnostic_cc1.sh [--install]`.

The rebuild on 2026-09-25 (`888dda67…`) also brought the diagnostic up to the
crash fix. Its predecessor (`4096c6fd…`, 2026-08-07) predated the fix and
segfaulted on 5 current TUs: `code6cac_c`, `config`, `main`, `text1a_pre` and
`text1b_b`.

Before 2026-08-07 it lacked the oracle's PLUS->IOR patch, and so disagreed
with the build compiler on `ings` and `code6cac_b`. That gap was formerly
carried as `UNFAITHFUL_STEMS` in `tools/ra_solver/local_extract.py`. The set
is now empty **on the merits**: the divergence was removed, not waived.

Hook inertness still holds. A cc1 built from pristine reverted sources is
behaviourally identical to the fully instrumented one on every TU
(`cc1_hooks.patch.md` carries the details and the two behavioural-knob
declaration rule). The oracle build reverts the hooks anyway, because it must
not depend on their inertness.

## Tracked state manifest of the gitignored compiler tree

Edits inside `tools/gcc-2.7.2/` leave no trace in git. That is exactly how the
`combine.c` patch went unnoticed for three months. This manifest makes the
tree's state detectable, and `tools/build_oracle_cc1.sh` and
`tools/build_diagnostic_cc1.sh` **refuse to run** if the sources drift. Verify
with `sha1sum` from `tools/gcc-2.7.2/`. Any drift means the tree changed, and
it must be re-recorded here, with rationale, in the same change.

```
80c8088750a91b37ef53bea6da51d402c58801c8  flow.c            (BB2 debug hooks)
6f23c5eeeabc97f3049b2d414ab42b6f38b891f6  function.c        (BB2 debug hooks)
46af0a314da8ad6dcb27890ca9b2003006c70ced  global.c          (BB2 debug hooks)
9b8f822a79a1945ac4b58ebfb243017d67b828c5  jump.c            (BB2 debug hooks)
3fb248a6b2c85cd7e9e57b19f5df7daf62b3d5fe  local-alloc.c     (BB2 debug hooks incl. SUGG)
7ddde6b0f2b65172c5cc83be6165789f445953d9  reload1.c         (BB2 debug hooks)
6e1cf6a97c169204304efd7427e066facb49d69f  reorg.c           (BB2 hooks + 2 BEHAVIORAL knobs, inert unless set, + the 2026-08-24 crash-fix block)
3668555e9cb7970b335a505aca4cfdda26e8fc49  sched.c           (BB2 debug hooks)
24c5952113d88cbb96f5c9f7e7152147d1efb8a7  combine.c         (pristine — the patch is applied in scratch, never here)
888dda6755596e3ac826c38c36be3103297cbac8  cc1               (instrumented DIAGNOSTIC binary, narrow patch)
aa04d7619cd79215788d18d6870ebc6c8d1d822d  build/cc1         (THE ORACLE COMPILER — recipe output)
```

**Re-recorded 2026-09-25 (owner ruling `bcdc1648e`, item (E)1).**
- `reorg.c`: `73a15a52…` → `6e1cf6a9…`. The only change is the 2026-08-24
  crash-fix block (owner ruling `262930f1b`, adopted in `fea9fa2ac`). That
  adoption did not re-record this manifest. Removing exactly that block (the
  comment, the `extern rtx negate_rtx PROTO(...)` line and one blank line)
  from the live file gives `73a15a5245d2e7ad55fa9e3c42d724488b1892d6` again,
  byte for byte. The same block is now the tracked recipe input
  `tools/cc1-reorg-negate-rtx-decl.patch`, and nothing else is in that input.
- `build/cc1`: `ea11be50…` became `0f438e42…` at the 2026-08-24 crash fix
  (unrecorded here). The 2026-09-25 adoption then made it `aa04d761…`.
- The diagnostic `cc1`: `4096c6fd…` → `888dda67…` (2026-09-25 rebuild).

Note that `combine.c` in the live tree is and stays **pristine**: the patch is
applied only to the scratch copy. Stale `.bb2bak` leftovers are exactly the
untracked-edit pattern this manifest exists to catch. Clean them, don't create
more.

## History

- **2026-05-18** — `build/cc1` built during a compiler-patch experiment and
  silently adopted as the project compiler. One copy, no record.
- **2026-08-07** — the reproducibility investigation established that no known
  recipe rebuilt it, then the forensics identified the cause: the `combine.c`
  PLUS→IOR removal. Owner elected migration to unpatched stock.
- **2026-08-07 (revised, `d99ab6a6`)** — Phase-0 diligence undercut the premise
  of that election (see the open question above). Owner revised it: keep the
  patch, commit it, pin the upstream, script the recipe, swap `build/cc1` to
  the recipe's own output. That is the current state.
- **2026-09-25** — a research-only stock build reproduced func_80073C78's
  target `ori` pair from the natural `+` body (see the open question). Owner
  ruling: keep the patch and keep the `|` spelling refused. A scratch-only study
  of a narrower patch is authorized. Adopting one returns to the owner with
  evidence. See § Owner ruling 2026-09-25; record in docs/grind/decisions.md
  2026-09-25 OWNER RULING — oracle compiler.
- **2026-09-25 (second batch)** — the study (`ac5d2b0bb`) found that a
  narrowed condition fits every compiled site. Owner ruling: adopt it after a
  register-plus-register scan of the target binary decides between narrow and
  `exprop`, or adopt narrow and keep `exprop` live if no site discriminates.
  The scan found no discriminating site, so narrow is selected, provisionally,
  pending the per-site check of the scanned sites that have a grind candidate;
  `exprop` stays a live alternative. Adoption follows the steps in § Owner ruling
  2026-09-25 (second batch) and has not yet been executed. Record in docs/grind/decisions.md 2026-09-25
  OWNER RULING — oracle compiler adoption (second batch).
- **2026-09-26 (Q17)** — owner ruling: a compiler patch is a cheat. The
  narrow PLUS->IOR adoption is superseded; func_800174F4, the one function
  depending on it, is reverted to `INCLUDE_ASM` and re-queued; the build
  compiler returns to the pinned upstream plus the one kept host fix
  (262930f1b) in a separate change. See `.claude/rules/no-compiler-divergence.md` § "Owner ruling
  2026-09-26 (ninth batch, Q17)".
- **2026-09-25 (adoption executed)** — the per-site (B) check found no
  discriminating site among the scanned functions with a grind candidate, so
  narrow was adopted under (C). `tools/cc1-plus-to-ior-narrow.patch` replaced
  `tools/cc1-no-plus-to-ior.patch`, and the crash fix was pinned as
  `tools/cc1-reorg-negate-rtx-decl.patch`. `build/cc1` was rebuilt from the
  recipe (`aa04d761…`, `simplify_rtx` 0x3a6a), and so was the diagnostic
  (`888dda67…`). Results: full build SHA1 `62efab4f…`, engine test 720/720,
  fixtures 5/5, and the oracle manifest re-locked. See § Adoption record
  (2026-09-25).

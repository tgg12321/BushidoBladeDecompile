# The oracle compiler — identity, derivation, and reproduction

**Status: REPRODUCIBLE.** The compiler the oracle match depends on is built
from committed inputs by a committed script. It is no longer a binary that
exists only as a file on one disk.

```
upstream : decompals/mips-gcc-2.7.2 @ 43d1cdb67ed135879869b5266f01efaaada5e35a
patch    : tools/cc1-no-plus-to-ior.patch      (9 lines removed from combine.c)
recipe   : every object at -O0 (`-g`), combine.o ALONE at `-O`,
           `-fgnu89-inline` throughout
build it : bash tools/build_oracle_cc1.sh              # verify in scratch
           bash tools/build_oracle_cc1.sh --install    # then swap it in
```

`tools/gcc-2.7.2/build/cc1` (Makefile:12) is the operative binary and is
**exactly what that script produces**. `tools/gcc-2.7.2/` is gitignored, which
is why the patch is tracked in `tools/` rather than applied in place; the
script copies the tree to scratch, restores it to the pinned commit, applies
the patch there, and never modifies the live tree.

The binary's own SHA1 is **not** the invariant — host-toolchain metadata leaks
into it across environments. The invariants are `simplify_rtx`'s size
(`0x39ab`) and, decisively, the assembly emitted for the project's TUs. The
script's self-check asserts the latter; a full
`engine verify-oracle --rebuild` against
`62efab4f73f992798c43e8c730aa43baa10bb4fa` remains the authoritative gate.

| binary | SHA1 | what it is |
|---|---|---|
| operative `build/cc1` | `ea11be50d12f24c464123b705c29007efc04e4a8` | recipe output (2026-08-07 swap) |
| `build/cc1.PRE-RECIPE-045c9543` | `045c9543d39ab8109583b92137c7adde084f7a25` | the historical binary it replaced |
| `build/cc1.ORACLE-BACKUP` | `045c9543d39ab8109583b92137c7adde084f7a25` | same, kept as a backup |
| off-tree backup | `045c9543d39ab8109583b92137c7adde084f7a25` | `C:\Users\Trenton\bb2-oracle-cc1-backup\cc1.oracle-compiler-045c9543` |
| diagnostic `cc1` | `4096c6fddbc4125a2100507e1e0ac08e289008aa` | hooks + the same patch (see below) |
| `cc1.PRE-PATCH-8384fd47` | `8384fd47cb51da369462a0ba0b83590eea88513a` | the unpatched diagnostic it replaced |

The two binaries are behaviourally identical: rebuilt-from-recipe vs the
historical `045c9543` shows **zero** differing lines across all 32 TUs. That
equivalence is what let the swap happen without touching a single source file.

**Verify the backups periodically — do not assume they persist.** The off-tree
backup has been found missing twice, on both occasions after being created and
hash-verified (once by the owner, once during the 2026-08-07 prep). Nothing
explains the disappearances yet, so treat that path as unreliable until
something does: re-check it with `sha1sum` rather than trusting its last known
state, and keep the two in-tree copies as the working redundancy. The reason
this matters is unchanged even now that the compiler is reproducible — the
historical binary is the only non-circular reference the recipe's self-check
has.

## What the patch does, and what it does not claim

Stock GCC 2.7.2 rewrites `a + b` into `a | b` whenever it can prove the
operands share no bits. The patch removes that conversion, so the compiler
emits `addu`/`addiu` where stock emits `or`/`ori`. That is the only
behavioural difference — a from-pristine build with the patch is
output-identical to the historical oracle on every TU, and `simplify_rtx`
matches its size to the byte.

Retention is **match-proven**: the full build links to the original
executable's SHA1. It is **not** a claim about what the original PsyQ compiler
did — see the open question below.

The patch's origin is accidental. It came from a 2026-05-18 compiler-patch
experiment whose output binary became the project compiler and was never
reverted; `tools/gcc-2.7.2/` being gitignored is why that left no trace. The
2026-08-07 forensics
([docs/grind/cc1-forensics-2026-08-07.md](grind/cc1-forensics-2026-08-07.md))
identified it.

### Why it is kept rather than removed (owner election, `d99ab6a6`)

On the SOTN pattern. That project's `cc1-psx-26` is itself a **patched** GCC
built from a pinned `decompals/old-gcc` commit and distributed as a
hash-verified binary. The community bar is *visible, pinned, reproducible* —
not *unpatched stock*. With the patch tracked, the upstream commit pinned, and
the recipe scripted, this project now meets that bar.

Removing the patch was the original 2026-08-07 election. It was revised on
Phase-0 evidence, below.

### Scope note — `no-compiler-divergence`

This single documented, owner-elected, match-proven patch is the **only**
amendment to that rule. It is not a license to patch the compiler again. Any
further divergence is still forbidden, and the rule's reasoning is otherwise
unchanged. (The 2026-09-25 owner ruling below authorizes only a scratch-only
study of a narrower patch. It changes nothing in the build, and adopting a
variant would need its own owner ruling.)

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
  owner ruling, landed before any code spends it.
- **(E) func_80073C78 stays rotated** at its honest floor (2/362, the `+`
  body in `memory/grind/func_80073C78/candidate.c`) until an owner ruling
  under (D) lands or an honest spelling is found.

Practical note (not part of the ruling): per the 2026-09-25 borderline entry,
the live `tools/gcc-2.7.2/reorg.c` no longer matches this document's manifest,
so `tools/build_oracle_cc1.sh` refuses to run. The study must not work around
that by re-recording the manifest. Restoring the tree, or re-recording it with
a rationale, is a separate change.

Record: docs/grind/decisions.md 2026-09-25 OWNER RULING — oracle compiler.

## The diagnostic compiler

`tools/gcc-2.7.2/cc1` is the same GCC carrying the `BB2_*_DEBUG` hooks that
`ra_solver` / `sched_solver` read their dumps from. It is now built from the
hooked sources **plus the same patch**, so it agrees with the oracle on all 32
TUs. Rebuild it with `bash tools/build_diagnostic_cc1.sh [--install]`.

Before 2026-08-07 it lacked the patch and therefore disagreed with the build
compiler on `ings` and `code6cac_b` — the gap formerly carried as
`UNFAITHFUL_STEMS` in `tools/ra_solver/local_extract.py`. That set is now
empty **on the merits**: the divergence was removed, not waived.

Hook inertness still holds: a cc1 built from pristine reverted sources is
behaviourally identical to the fully instrumented one on all 32 TUs
(`cc1_hooks.patch.md` carries the details and the two behavioural-knob
declaration rule). The oracle build reverts the hooks anyway — it must not
depend on their inertness.

## Tracked state manifest of the gitignored compiler tree

Edits inside `tools/gcc-2.7.2/` leave no trace in git; that is exactly how the
`combine.c` patch went unnoticed for three months. This manifest makes the
tree's state detectable. `tools/build_oracle_cc1.sh` **refuses to run** if it
drifts. Verify with `sha1sum` from `tools/gcc-2.7.2/`; any drift means the tree
changed and must be re-recorded here, with rationale, in the same change.

```
80c8088750a91b37ef53bea6da51d402c58801c8  flow.c            (BB2 debug hooks)
6f23c5eeeabc97f3049b2d414ab42b6f38b891f6  function.c        (BB2 debug hooks)
46af0a314da8ad6dcb27890ca9b2003006c70ced  global.c          (BB2 debug hooks)
9b8f822a79a1945ac4b58ebfb243017d67b828c5  jump.c            (BB2 debug hooks)
3fb248a6b2c85cd7e9e57b19f5df7daf62b3d5fe  local-alloc.c     (BB2 debug hooks incl. SUGG)
7ddde6b0f2b65172c5cc83be6165789f445953d9  reload1.c         (BB2 debug hooks)
73a15a5245d2e7ad55fa9e3c42d724488b1892d6  reorg.c           (BB2 hooks + 2 BEHAVIORAL knobs, inert unless set)
3668555e9cb7970b335a505aca4cfdda26e8fc49  sched.c           (BB2 debug hooks)
24c5952113d88cbb96f5c9f7e7152147d1efb8a7  combine.c         (pristine — the patch is applied in scratch, never here)
4096c6fddbc4125a2100507e1e0ac08e289008aa  cc1               (instrumented DIAGNOSTIC binary, patched)
ea11be50d12f24c464123b705c29007efc04e4a8  build/cc1         (THE ORACLE COMPILER — recipe output)
```

Note that `combine.c` in the live tree is and stays **pristine**: the patch is
applied only to the scratch copy. Stale `.bb2bak` leftovers are exactly the
untracked-edit pattern this manifest exists to catch — clean them, don't
create more.

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

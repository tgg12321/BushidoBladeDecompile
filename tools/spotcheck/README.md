# spotcheck — standing guards against silent COMPLETED-C regressions

A completion is verified ONCE, at completion time. Nothing re-verifies it
afterwards. These tools close that gap.

| File | What |
|---|---|
| `spot_check_completed.py` | four-mode per-function guard (sandbox / attribute / config / roster) |
| `queue_regen_diff.py` | whole-corpus channel: recompute the queue, diff vs the committed one, without mutating it |

---

## 1. The failure modes, reconciled

Honest distance is a function of three inputs: the C source, the build
pipeline's config, and **the cheat-detector / stripper's config**. The oracle
pins bytes, so it covers the first two and is blind to the third by
construction — those files never feed the build.

### (a) The byte channel — attribution- and latency-blind, not invisible

A COMPLETED-C function carries zero rules and zero cheat-asm, so a codegen
change to it *is* a byte change and *does* break the SHA1. There is no
green-oracle state with byte-regressed completed code. What the oracle does not
give you:

1. **Attribution.** Its verdict is one hash over 606,208 bytes. It never names a
   function, and bisecting that by hand is expensive.
2. **Latency.** Nothing re-checks a completion between builds, so a regressing
   edit can sit in the tree for many sessions.
3. **A live read**, because of the self-reference trap below.

**The self-reference trap.** `engine/sandbox.py:72` scores the cheat-stripped
rebuild of the current `src/<stem>.c` against `build/src/<stem>.o` — an artifact
of whatever source was present at the last full build, *not* the original
executable.

| tree state | what `sandbox --disable all` reads |
|---|---|
| source edited, **not** rebuilt | reference still holds last-green bytes → the **true** regression (this is how `func_8008C184` was seen at 0 → 4) |
| source edited **and** rebuilt | reference regenerated *from the regressed source* → regressed-vs-regressed → **0, falsely clean** |

The per-function signal vanishes exactly when the whole-binary signal appears.

### (b) The detector-config channel — genuinely oracle-invisible

Measured live 2026-08-07. Commit `05d13f9a` normalized
`volatile_extern_allowlist.txt` to LF and ate one newline, gluing two entries:

```
D_800F1AF8    # ...Same Ruling-4 class grant.D_800F7420    # _spu_RQ (u16[4] ...
```

`volatile_cheats._load_volatile_extern_allowlist` strips at the **first** `#`
and takes the first token, so the line yields `D_800F1AF8` and **`D_800F7420`
silently left the allowlist**. The stripper then treated a sanctioned
`extern volatile` as a cheat again and cheat-stripped scoring regressed
`func_80088740` 0 → 7 and `func_8008AAD4` 0 → 54 (since renamed by the naming
wave to `_spu_init` and `SpuSetKey`).

**The build SHA1 never moved.** The allowlist is not among
`oracle/manifest.json`'s `CONFIG_FILES` — correctly, since it affects no bytes.
Repaired in `e5d1779f`.

Two consequences drive the design:

1. **A green oracle is necessary but not sufficient.** It guarantees the
   reference objects are target-truthful; it says nothing about whether
   "cheat-free" still means what it meant yesterday. So `sandbox` mode runs the
   config and roster checks alongside.
2. **No syntax check can catch this.** The glued line is a valid entry that
   parses cleanly — it just parses to one symbol instead of two. A linter, an LF
   check, and a content hash all pass. The loss exists only in the **parse
   result**, so that is what `--mode config` compares.

### The channel that actually fired

The incident did **not** surface as a distance regression. A lost grant makes
the construct count as a cheat, which removes the function from the COMPLETED-C
pool *before* it can be scored — so a distance-only guard reports "all clean"
while silently excluding the regressed functions. `--mode roster` exists for
exactly this: a function **leaving** the COMPLETED-C set is a first-class
finding.

---

## 2. The modes

### `--mode sandbox` (default) — working-tree canary

Enforces `sha1(build/bb2.exe) == oracle` as a hard precondition, which is what
makes `build/src/*.o` target-truthful and licenses reading a non-zero distance
as a regression; refuses to score otherwise rather than emit a number it cannot
back. Groups the sample by source file and builds **one cheat-stripped object
per distinct stem** (the same `empty_overrides` + `write_stripped` recipe
`sandbox <func> --disable all` uses, and the one `queue regen` uses to score a
whole file at once), so N functions cost at most N compiles and usually far
fewer. Runs the config and roster checks too.

### `--mode attribute` — post-red-oracle attributor

Per-function bytes, `build/bb2.exe` vs `disc/SLUS_006.63`, using
`build/bb2.elf`'s symbol table. No compiling, unmasked comparison, and the
baseline is the shipped game so it can never drift. Works precisely where
sandbox mode is untrustworthy, and turns "the SHA1 broke" into a function list.
Also immune to the scorer's HI16/LO16 and branch-target masking
(`memory/sandbox-lo16-text-addend-false-distance.md`).

### `--mode config` — detector-config inventory diff

Diffs each detector config's **parsed symbol inventory** against a git ref.
Comparing inventories rather than contents is the point: a content hash tells
you the file changed (which it legitimately does constantly); only the parse
result distinguishes "an entry was edited" from "an entry disappeared".

Covers `volatile_extern_allowlist.txt` and `inline_asm_canonical.txt`. The
canonical list is parsed by **the engine's own loader** (`cheats.canonical_asm_funcs`
takes a path, so a git blob goes through the real implementation — no mirror).
The allowlist loader hardcodes its filename, so that parser is mirrored; to keep
the mirror honest, `verify_parser_mirror()` checks it against the engine loader
on the working-tree file every run and fails loudly on drift
(`[[buildconfig-mirror-drift-false-mismatch]]` in miniature).

Reads no build artifact, so it is the one check that stays sound — and
sub-second — while a build is running. It deliberately skips the build-in-flight
preflight for that reason.

### `--mode roster` — COMPLETED-C membership diff

Recomputes the COMPLETED-C set as it was at a git ref, by the same definition,
using git blobs for the queue, the canonical list, the rule configs, the
prologue configs, `src/*.c`, **and the allowlist**, then reports departures.

Three subtleties, each of which was a measured bug before it was a design point:

* **The ref's allowlist is used, not the working tree's.** Without this the
  comparison applies today's grants to yesterday's source, so a dropped grant
  changes both sides identically and no departure appears. Measured: with the
  working-tree allowlist the roster read 851 == 851 across the `05d13f9a` glue
  commit and detected nothing. This needed a small engine addition,
  `volatile_cheats.use_allowlist()`.
* **An unlocatable body is not evidence of completion.** `func_cheat_asm_count`
  returns `-1` for "body not found", and treating that as clean invents
  departures: against the pre-wave ref `c733c1a6` it reported `open`, `prnt`,
  `read`, `sprintf`, `write` as regressions purely because those *names* did not
  exist in `src/main.c` then. Ref rosters therefore exclude unknowns.
* **Becoming canonical is not a regression.** Moving into
  `inline_asm_canonical.txt` is an authorized reclassification requiring owner
  sign-off; the naming wave moved `_card_write` / `_new_card` that way. Those are
  reported as reclassifications. Re-entering the queue, or newly carrying a rule
  or cheat construct, *is* the regression signature.

**Limitation: the join key is the function NAME.** A rename makes the old
identifier vanish from the symbol universe, so renames neither produce false
departures (proven across the 334-rename wave) nor can be followed through. The
2026-08-07 incident cannot be replayed through roster mode for that reason — its
two functions were renamed — so the mechanism was validated on today's tree
instead (§4).

**Limitation: the symbol universe is the CURRENT build's objects.** There is no
way to enumerate a past build's symbols without rebuilding at that ref, so a
function that did not exist at the ref appears as an addition, never a
departure. That is the safe direction: departures are never invented.

---

## 3. `queue_regen_diff.py` — the broadest channel

`queue regen` recomputes every function's status, verdict and honest distance
from scratch, so diffing it against the committed queue is a complete regression
report over the **whole corpus**, not a sample. It is also what actually caught
the 2026-08-07 incident (a regen happened to run and someone diffed it against
HEAD); this makes that deliberate.

Exits non-zero on **function-added** (a queue entry means INCOMPLETE, so a
completed function fell out — the 2026-08-07 signature), **distance-increase**,
or **verdict-change**. Removals and distance decreases are progress, reported
but never failures.

**It does not mutate `engine/queue.json`.** `engine.queue.generate()` ends in
`save()`, so calling it directly rewrites the queue — dirt that deadlocks the
Grinder's scope check (`[[grinder-park-queue-dirt-deadlock]]`). The wrapper
repoints `Q.QUEUE_PATH` at a scratch copy for the duration, so `generate()` seeds
its `parked`/`owner_override` preservation from a copy of the real queue and
writes its output to scratch. The lock path derives from `QUEUE_PATH`, so it
moves with it.

**Precondition: not while the Grinder is live.** A regen does one cheat-stripped
build per file (minutes) and reads the build tree throughout; racing a
`queue done` is the write race the queue's own fingerprint check exists to
catch. The tool refuses on a driver lock or moving build artifacts.

**Overlap, stated plainly:** this channel subsumes `--mode sandbox`'s coverage
and largely subsumes `--mode roster` (a departure shows up as a function-added).
If only one thing survives, keep this one. `spot_check_completed.py` earns its
place because `--mode config` catches the *cause* cheaply enough to run on every
config commit, and `--mode attribute` answers what regen cannot: which function
broke the SHA1.

---

## 4. Verification (2026-08-07, HEAD `fe40a52b`, oracle green)

| check | result |
|---|---|
| `engine test` before and after the engine changes | 280 passed both times |
| memoization correctness (`tmp/spotcheck/verify_memo.py`) | identical results; allowlist change observed *through* the memo (plain externs 2 → 12, all-cheats 4 → 14 when grants dropped); returned lists are fresh copies |
| `tools/check_completion_integrity.py` | 3 m 05 s → **43 s**, still reports clean |
| `--mode config` on HEAD | `volatile_extern_allowlist.txt` 27 symbols, `inline_asm_canonical.txt` 179 symbols, exit 0 |
| `--mode config` replayed on the real commits | `05d13f9a~1 → 05d13f9a`: 27 → 26, `LOST=['D_800F7420']`; repair reverses it |
| `--mode roster --ref HEAD` | 0 departures, ~25 s |
| `--mode roster --ref c733c1a6` (across the 334-rename wave) | 0 regressions, reclassifications absorbed |
| roster detection (`tmp/spotcheck/verify_roster_detects.py`) | dropping all allowlist grants departs 16 functions, including `_spu_init` and `SpuSetKey` — the 2026-08-07 pair under their new names — and `func_8008C184` |
| build-in-flight preflight | fired live against a concurrent build (16 artifacts fresh, 9 moving), passed once it finished |

Earlier drafts also measured, on the pre-wave tree: `--mode attribute --all`
1,482 symbols clean in 7.9 s; `--all` sandbox sweep 1,014 functions, 1,006 OK,
0 regressions, 8 UNSCORED, 1 m 36 s.

---

## 5. Cadence and wiring

**By trigger — the high-value path.**

*Shared-surface edits* (the `D_800F1AF4` class) → `--mode sandbox --all`:
`include/*.h`, externs consumed elsewhere, `named_syms.txt` /
`undefined_syms_auto.txt`, `bb2.ld`, volatile-qualifier changes on globals.

*Detector-config edits* (the `D_800F7420` class) → `--mode config`, escalating
to `--mode roster` and `--mode sandbox --all` if it reports anything:
`volatile_extern_allowlist.txt`, `inline_asm_canonical.txt`, the maspsx gate
lists. **Whole-file reformatting of these is the specific hazard** — line-ending
normalization, re-sorting, re-wrapping — because it touches every line at once
and the oracle cannot see the result. `--mode config` costs under a second, so
there is no reason not to run it on every such commit.

**By schedule.** `--all` costs ~1.5 minutes against ~13 s for a 12-function
sample, while weekly sampling of 25 tours a ~1,000-function corpus about 1.3
times a year. Prefer a weekly `--all`, or better, a weekly `queue_regen_diff.py`.

**Grinder.** The cheapest moment for `sandbox` mode is right after a green
build — the precondition is already satisfied. `--mode attribute --all` right
after a **red** build costs a second and hands the driver a function list
instead of a bare hash mismatch.

**Not a commit hook.** `sandbox` mode compiles, and it would fail open whenever
`build/` is stale; a guard that silently no-ops is worse than no guard.
`--mode config` *is* cheap enough for a hook if one is ever wanted.

**Relation to `tools/check_completion_integrity.py`.** They share an enumeration
and nothing else: that tool asks whether a completion is *categorised* honestly,
these ask whether its *bytes* still match. The byte check needs a green or
completed build, which cannot be a precondition of the fast unconditional
category audit — so keep them separate, and consider a closing pointer line in
the integrity tool so the two are discoverable together.

---

## 6. Engine changes these depend on

Both are in `engine/`, both kept the suite at 280/280, and both are useful
independently of this directory.

1. **Memoization** of `inlineasm._strip_spans`, `inlineasm.register_hint_spans`,
   `volatile_cheats.find_alias_renames` (pure in `text`) and
   `volatile_cheats.find_plain_volatile_externs` / `find_all_cheats` (keyed on
   `(text, allowlist)`). These were whole-file scans called once per FUNCTION —
   `func_volatile_cheat_count` measured 348 ms/function, so text1b.c re-scanned
   itself ~1,200 times.

   **The allowlist must be in the key.** `find_plain_volatile_externs` consults
   `volatile_extern_allowlist.txt`, so a text-only memo would let a stale answer
   survive an allowlist edit — reintroducing the exact `D_800F7420` failure class
   *inside the detector*. The loader is mtime-cached and its result is a
   `frozenset`, so passing that frozenset as part of the cache key is both exact
   and cheap. Each wrapper returns a fresh `list` so a mutating caller cannot
   corrupt the cache.

2. **`volatile_cheats.use_allowlist(path)`** — a context manager to read the
   allowlist from somewhere other than the working tree, so a tool can evaluate
   the detector *as it was* at another revision. `path=None` is a no-op. Without
   it, roster mode is structurally blind to the channel it exists for (§2).

Still unmemoized and worth a follow-up: `cheats.func_rule_lines` re-reads and
re-splits `regfix.txt` (3.5k lines) per call, which is the bulk of the remaining
43 s in `check_completion_integrity.py`. Out of scope here.

---

## 7. Open questions / owner-gated

1. **Data-as-code symbols in the pool — RESOLVED (owner ruling 2026-08-07):
   the full structural filter is applied.** The 12 symbols listed `F .text` in
   the objects that are structurally not C functions (`.include`d asm bodies,
   `.aent` alternate entries, instruction-less `glabel` markers — the test
   `queue.not_a_c_function_text` draws) are EXCLUDED from the COMPLETED-C pool
   here and in `check_completion_integrity.py`, in the same commit, so the two
   tools agree. This changed the headline count 1016 → 1004 — a definitional
   correction, not a regression. Both tools print the excluded list on every
   run. Name-based filtering remains wrong (most `D_`-named symbols are
   genuinely un-renamed functions); the filter is structural only. The
   longer-term fix — reclassifying these symbols as data in the split — would
   make the filter moot.
2. **Should `spot_check_completed.py` be an engine subcommand?** It reuses
   `engine.sandbox` / `score` / `cheats` directly and would pick up metrics
   capture free via `engine/metrics.py`. `check_completion_integrity.py` set the
   `tools/` precedent.
3. **Should attribute mode's baseline be materialised?** It derives vram+size
   from `build/bb2.elf` at run time, so it needs a build to exist. A committed
   `oracle/completed_bytes.json` (name → vram, size, sha1 of the *original*
   EXE's bytes) would make it build-independent — `golden_fixtures` widened from
   a handful of functions to all of them.
4. **Which other detector configs need inventory coverage?** The maspsx gate
   lists are next. Each needs its parser mirrored exactly, or better, the engine
   loader refactored to take a path the way `canonical_asm_funcs` already does.
5. **UNSCORED policy.** Currently exits 1 (an unknown is not a pass). Safe, but
   noisy if some file's honest build is chronically broken — in which case the
   fix is that file, not the exit code.
6. **Does `roster` mode survive `queue_regen_diff.py`?** They overlap heavily
   (§3). If the regen channel is wired into the Grinder, roster mode is probably
   redundant and could be dropped to reduce surface.

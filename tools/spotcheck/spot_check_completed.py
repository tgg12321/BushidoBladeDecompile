#!/usr/bin/env python3
"""Standing spot-check guard for silent COMPLETED-C regressions.

A completion is verified ONCE, at completion time; nothing re-verifies it
afterwards. Two distinct things can silently undo one, and the full-build SHA1
oracle catches neither cleanly. Design note: tools/spotcheck/README.md.

WHAT THE ORACLE COVERS, AND WHAT IT DOES NOT
--------------------------------------------
Honest distance is a function of three inputs: the C source, the build
pipeline's config, and the cheat-DETECTOR/stripper's config. The oracle pins
BYTES, so it covers the first two and is blind to the third by construction —
those files never feed the build.

  (a) THE BYTE CHANNEL. A COMPLETED-C function carries zero rules and zero
      cheat-asm, so a codegen change to it IS a byte change and DOES break the
      SHA1. But the oracle's verdict is one hash over 606,208 bytes: it never
      names a function, nothing re-checks completions between builds, and the
      per-function signal is self-erasing (see below).

  (b) THE DETECTOR-CONFIG CHANNEL. Measured live 2026-08-07: a CRLF->LF
      normalization of volatile_extern_allowlist.txt glued two entries onto one
      line, so `D_800F7420` silently stopped parsing. The stripper then treated
      a sanctioned `extern volatile` as a cheat again and cheat-stripped scoring
      regressed func_80088740 0->7 and func_8008AAD4 0->54 — with the build SHA1
      UNCHANGED throughout. No syntax check can catch that: the glued line is a
      perfectly valid entry, it just parses to one symbol instead of two. Only
      the parse RESULT shows the loss.

THE SELF-REFERENCE TRAP (why a naive spot-check is worthless)
-------------------------------------------------------------
engine.sandbox scores the cheat-stripped rebuild of the current src/<stem>.c
against build/src/<stem>.o — an artifact of whatever source was present at the
last full build, NOT the original executable. Edited-but-not-rebuilt: the
reference still holds last-green bytes, so the true regression shows. Edited AND
rebuilt: the reference is regenerated FROM the regressed source, so it compares
regressed against regressed and reads a clean 0. The per-function signal
vanishes exactly when the whole-binary signal appears.

THE MODES
---------
  sandbox   (default) working-tree canary. Honest distance per function, gated
            on sha1(build/bb2.exe) == oracle so the reference is target-truthful
            by construction. One cheat-stripped compile per distinct FILE.
  attribute post-red-oracle attributor. Per-function bytes, build/bb2.exe vs
            disc/SLUS_006.63. No compiling; baseline is the shipped game so it
            can never drift. Turns "the SHA1 broke" into a function list.
  config    detector-config inventory diff vs a git ref. Catches channel (b).
            Reads no build artifact, so it stays sound while a build runs.
  roster    COMPLETED-C membership diff vs a git ref. A function LEAVING the
            set is the signature channel (b) actually produced, and it is
            invisible to a distance-only check because the departed function is
            excluded from the pool before it can be scored.

USAGE (WSL, repo root, .venv active)
------------------------------------
    python3 tools/spotcheck/spot_check_completed.py                 # 25 sampled
    python3 tools/spotcheck/spot_check_completed.py --all
    python3 tools/spotcheck/spot_check_completed.py --func saTan0Main
    python3 tools/spotcheck/spot_check_completed.py --file main
    python3 tools/spotcheck/spot_check_completed.py --mode config   # no build needed
    python3 tools/spotcheck/spot_check_completed.py --mode roster --ref HEAD
    python3 tools/spotcheck/spot_check_completed.py --mode attribute

Exit codes:  0 clean · 1 regression(s) found · 2 preconditions unmet.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import random
import re
import subprocess
import sys
import tempfile
import time
from pathlib import Path

# Walk up to the repo root (the directory holding engine/).
REPO = next(p for p in Path(__file__).resolve().parents if (p / "engine").is_dir())
sys.path.insert(0, str(REPO))
# Engine modules resolve paths relative to CWD (regfix.txt, src/<stem>.c,
# build/src/<stem>.o), as tools/check_completion_integrity.py also does.
os.chdir(REPO)

from engine import buildconfig as cfg  # noqa: E402
from engine import cheats, inlineasm, sandbox, score  # noqa: E402
from engine import pipeline as P  # noqa: E402
from engine import volatile_cheats as vc  # noqa: E402

QUEUE_REL = "engine/queue.json"
MANIFEST = REPO / "oracle" / "manifest.json"
WORKDIR = "tmp/spotcheck_work"

GRIND_LOCK = REPO / "tmp" / "grind" / "grind.lock"
CLOSER_LOCK = REPO / "tmp" / "closer" / "closer.lock"
FRESH_SECS = 45.0
SETTLE_SECS = 2.5


# ==========================================================================
# preconditions
# ==========================================================================
def oracle_sha1() -> str:
    return json.loads(MANIFEST.read_text())["original_exe"]["sha1"]


def file_sha1(p: Path) -> str:
    h = hashlib.sha1()
    with p.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def _build_artifact_mtimes() -> dict[str, float]:
    out = {}
    for p in list(Path("build/src").glob("*.o")) + [Path("build/bb2.exe"),
                                                    Path("build/bb2.elf")]:
        try:
            out[str(p)] = p.stat().st_mtime
        except OSError:
            pass
    return out


def check_no_build_in_flight(ignore_locks: bool) -> tuple[bool, list[str]]:
    """Refuse to measure a tree that is being rebuilt underneath us.

    Measured: three consecutive runs saw COMPLETED-C pools of 1016 / 960 / 995
    purely because another agent was rebuilding build/src/*.o mid-run.
    Everything sandbox/attribute mode reads is a build artifact, so a concurrent
    build races the enumeration AND the green-oracle precondition (which can
    pass and be invalidated two seconds later).

      * Driver locks hold a WINDOWS pid; this tool runs WSL-side where that pid
        is not resolvable, so it cannot do the liveness test grind.ps1 does for
        its own stale-lock reclaim. It refuses and says so rather than guessing,
        with an explicit --ignore-locks escape.
      * Artifact mtimes are decisive and platform-independent, and deliberately
        NOT skippable by --ignore-locks.
    """
    notes: list[str] = []
    ok = True
    for lock, who in ((GRIND_LOCK, "grinder"), (CLOSER_LOCK, "closer")):
        if lock.exists():
            pid = lock.read_text(errors="replace").strip()[:16]
            if ignore_locks:
                notes.append(f"{who} lock present ({lock}, pid {pid}) — ignored by request")
            else:
                ok = False
                notes.append(
                    f"{who} lock present: {lock} (pid {pid}). This tool runs WSL-side "
                    f"and cannot check a Windows pid's liveness, so it will not guess "
                    f"whether the lock is live or stale. Confirm with "
                    f"`pwsh tools/grinder/status.ps1`, then re-run with --ignore-locks "
                    f"if the driver is idle.")
    before = _build_artifact_mtimes()
    if not before:
        return ok, notes + ["no build artifacts found — nothing to race"]
    now = time.time()
    fresh = [p for p, m in before.items() if now - m < FRESH_SECS]
    if fresh:
        ok = False
        notes.append(f"{len(fresh)} build artifact(s) written in the last "
                     f"{FRESH_SECS:.0f}s (e.g. {sorted(fresh)[0]}) — a build just "
                     f"touched this tree; wait for it to finish.")
    time.sleep(SETTLE_SECS)
    after = _build_artifact_mtimes()
    moved = [p for p in set(before) | set(after) if before.get(p) != after.get(p)]
    if moved:
        ok = False
        notes.append(f"{len(moved)} build artifact(s) changed during a "
                     f"{SETTLE_SECS:.1f}s settle window (e.g. {sorted(moved)[0]}) — "
                     f"a build is IN FLIGHT. Any measurement now is unsound.")
    if ok and not notes:
        notes.append(f"no build in flight ({len(before)} artifacts stable over "
                     f"{SETTLE_SECS:.1f}s)")
    return ok, notes


def check_reference_trust() -> tuple[bool, str]:
    """build/src/*.o is target-truthful iff the build it came from was
    byte-identical to the original. Verified from the artifact, so a
    stale-but-green build is fine and a rebuilt-regressed build is caught."""
    exe = Path("build/bb2.exe")
    if not exe.exists():
        return False, "build/bb2.exe missing — no reference build to score against."
    got, want = file_sha1(exe), oracle_sha1()
    if got != want:
        return False, (f"build/bb2.exe SHA1 {got} != oracle {want}.\n"
                       "  The build/src/*.o reference is NOT target-truthful in this "
                       "state, so honest distances measured against it are meaningless "
                       "(a regressed source rebuilds its own reference).\n"
                       "  Run `--mode attribute` instead: it compares the built EXE "
                       "against disc/SLUS_006.63 and names the functions that moved.")
    return True, f"reference build verified: build/bb2.exe == oracle ({want[:12]}…)"


# ==========================================================================
# git helpers
# ==========================================================================
def git_show(ref: str, path: str) -> str | None:
    r = subprocess.run(["git", "show", f"{ref}:{path}"],
                       capture_output=True, text=True)
    return r.stdout if r.returncode == 0 else None


# ==========================================================================
# COMPLETED-C enumeration (shared shape with check_completion_integrity.py)
# ==========================================================================
_RULE_KEY = re.compile(r"^([A-Za-z_][A-Za-z0-9_]*)\s*:")


def rule_counts(texts: dict[str, str | None]) -> dict[str, int]:
    """func -> total rule count, from ONE pass over each config text.

    cheats.func_rule_lines re-reads and re-splits the whole config per call, so
    the natural per-function loop costs ~3 full reads of a 3,500-line
    regfix.txt PER FUNCTION. A rule line is keyed `^<func>\\s*:`
    (cheats._key_re), so one scan collecting each line's key is equivalent.
    """
    counts: dict[str, int] = {}
    for text in texts.values():
        if not text:
            continue
        for ln in text.splitlines():
            m = _RULE_KEY.match(ln)
            if m:
                counts[m.group(1)] = counts.get(m.group(1), 0) + 1
    return counts


def working_rule_texts() -> dict[str, str | None]:
    out = {}
    for c in (cheats.REGFIX, cheats.REGFIX2, cheats.ASMFIX):
        p = Path(c)
        out[c] = p.read_text(encoding="utf-8") if p.exists() else None
    return out


def prologue_funcs() -> set[str]:
    """Every function carrying a prologue_fix entry, from one read of each of
    the three configs (cheats.func_prologue_count re-parses them per call)."""
    out: set[str] = set()
    pc = Path(cheats.PROLOGUE_CONFIG)
    if pc.exists():
        out |= set(json.loads(pc.read_text()).keys())
    for path in (cheats.DELAY_SLOT_RA, cheats.FRAME_FIX):
        out |= cheats._prologue_txt_funcs(path)
    return out


def object_symbols() -> dict[str, list[str]]:
    """stem -> [function symbols], from the built objects. This is the symbol
    UNIVERSE for every mode; it is only available for the current build."""
    out: dict[str, list[str]] = {}
    for stem in sorted(P.c_stems()):
        ref_o = f"build/src/{stem}.o"
        if Path(ref_o).exists():
            out[stem] = list(score._o_func_table(ref_o))
    return out


def classify_pool(symbols: dict[str, list[str]], in_queue: set[str],
                  canon: set[str], rulecount: dict[str, int],
                  prologued: set[str], src_text: dict[str, str | None],
                  unknown_is_completed: bool = True,
                  ) -> tuple[list[tuple[str, str]], list[str], list[tuple[str, str]]]:
    """-> (COMPLETED-C [(func, stem)], violation notes, no-C-body [(func, stem)])

    COMPLETED-C == in a built object, NOT in the queue (queued == INCOMPLETE by
    definition), NOT in inline_asm_canonical.txt (that set is
    COMPLETED-INLINE-ASM-CANONICAL and legitimately non-zero-distance), and
    carrying zero rules / prologue_fix / cheat constructs.

    A function failing only that last clause is neither queued nor canonical yet
    carries a cheat: a completion-integrity VIOLATION, not a regression. It is
    reported separately and handed to check_completion_integrity.py.

    no-C-body collects symbols the object lists as `F .text` that have no C
    definition at all (data extracted as code, e.g. text1b's D_8005xxxx). They
    are surfaced, not filtered — see README §8 q4 (owner-gated).

    `unknown_is_completed` governs a symbol whose cheat count is -1, i.e. the C
    body could not be located at all:

      * True (the CURRENT pool) — counted as COMPLETED-C. This is not an
        opinion, it mirrors tools/check_completion_integrity.py, whose
        `elif cheat_count > 0: ... else: total_completed_c += 1` does exactly
        the same. Changing it would move the project-wide COMPLETED-C count, so
        it is owner-gated (README §8 q4).
      * False (a REF roster) — excluded. "I cannot find this function's body at
        that ref" is not evidence that it WAS completed there, and treating it
        as such invents departures. Measured: against the pre-naming-wave ref
        c733c1a6, the True setting reported open / prnt / read / sprintf /
        write as regressions purely because those NAMES did not exist in
        src/main.c at that ref, so the body lookup failed and the unknown was
        scored as a completion.
    """
    completed: list[tuple[str, str]] = []
    violations: list[str] = []
    no_body: list[tuple[str, str]] = []
    for stem, funcs in symbols.items():
        text = src_text.get(stem)
        # One whole-file strip tells us whether ANY function in the file has
        # cheat asm; when it does not — the common case — every function in it
        # is trivially clean, and we skip the per-function scan entirely.
        file_has_cheat_asm = (text is None
                              or inlineasm.strip_cheat_asm_file(text)[1] > 0)
        for func in funcs:
            if func in in_queue or func in canon:
                continue
            rules = rulecount.get(func, 0)
            prologue = 1 if func in prologued else 0
            cheat_count = (inlineasm.func_cheat_asm_count(text, func)
                           if (file_has_cheat_asm and text is not None) else 0)
            if text is not None and inlineasm._func_body_span(text, func) is None \
                    and func not in inlineasm.whole_body_asm_funcs(text):
                no_body.append((func, stem))
            if cheat_count < 0 and not unknown_is_completed:
                continue  # body unlocatable at this ref — no evidence either way
            if rules or prologue or cheat_count > 0:
                violations.append(
                    f"{func} ({stem}.c): not queued, not canonical, but carries "
                    f"{rules} rule(s) / {prologue} prologue_fix / {cheat_count} cheat "
                    f"construct(s) — a completion-integrity violation, not a "
                    f"regression. Run tools/check_completion_integrity.py.")
                continue
            completed.append((func, stem))
    return completed, violations, no_body


def enumerate_completed_c() -> tuple[list[tuple[str, str]], list[str], list[tuple[str, str]]]:
    q = Path(QUEUE_REL)
    in_queue = ({it["func"] for it in json.loads(q.read_text()).get("items", [])}
                if q.exists() else set())
    src_text = {stem: inlineasm._read_src_cached(stem) for stem in P.c_stems()}
    return classify_pool(object_symbols(), in_queue, cheats.canonical_asm_funcs(),
                         rule_counts(working_rule_texts()), prologue_funcs(), src_text)


def select(pool: list[tuple[str, str]], args) -> list[tuple[str, str]]:
    if args.func:
        want = set(args.func)
        picked = [p for p in pool if p[0] in want]
        missing = want - {f for f, _ in picked}
        if missing:
            print(f"!! not COMPLETED-C (or not built): {', '.join(sorted(missing))}")
        return picked
    if args.file:
        return [p for p in pool if p[1] in set(args.file)]
    if args.all:
        return pool
    return random.Random(args.seed).sample(pool, min(args.n, len(pool)))


# ==========================================================================
# MODE sandbox
# ==========================================================================
def run_sandbox(selection: list[tuple[str, str]], verbose: bool) -> list[dict]:
    """One build_stripped_object per distinct stem scores every sampled function
    in that file — the same empty_overrides + write_stripped recipe
    `sandbox <func> --disable all` uses, and the one queue regen uses to score a
    whole file at once. Sampling N functions costs at most N compiles and
    typically far fewer."""
    by_stem: dict[str, list[str]] = {}
    for func, stem in selection:
        by_stem.setdefault(stem, []).append(func)
    results: list[dict] = []
    for stem in sorted(by_stem):
        ref_o = f"build/src/{stem}.o"
        purec_o = f"{WORKDIR}/{stem}/{stem}.purec.o"
        t0 = time.time()
        try:
            sandbox.build_stripped_object(stem, purec_o, f"{WORKDIR}/{stem}/cfg",
                                          strip_cheat_asm=True)
        except Exception as e:
            # The honest build failed: report the functions as UNSCORED, never
            # as clean. An unscored function is an unknown, not a pass.
            for func in by_stem[stem]:
                results.append({"name": func, "where": stem, "status": "UNSCORED",
                                "detail": str(e)[:300]})
            continue
        if verbose:
            print(f"  built {stem}.c (honest) in {time.time() - t0:.1f}s "
                  f"— {len(by_stem[stem])} function(s) to score")
        for func in by_stem[stem]:
            try:
                d = score.score_func(purec_o, ref_o, func)["score"]
            except KeyError as e:
                results.append({"name": func, "where": stem, "status": "UNSCORED",
                                "detail": f"absent from honest object: {e}"})
                continue
            results.append({"name": func, "where": stem,
                            "status": "OK" if d == 0 else "REGRESSION",
                            "distance": d,
                            "detail": (f"honest distance {d}, expected 0. "
                                       f"See the diff: python3 tools/pairdiff.py "
                                       f"{stem} {func}") if d else ""})
    return results


# ==========================================================================
# MODE config — parsed-INVENTORY diff of the detector configs
# ==========================================================================
def _parse_allowlist_text(text: str) -> set[str]:
    """Mirror of volatile_cheats._load_volatile_extern_allowlist's line
    semantics (strip at the FIRST '#', take the first whitespace token).

    A mirror is a drift hazard ([[buildconfig-mirror-drift-false-mismatch]]),
    so verify_parser_mirror() below checks this against the engine loader's own
    answer on the working-tree file every run. A mirror that silently drifts
    would produce exactly the confident fiction this tool exists to prevent.
    The engine loader itself cannot be reused directly here: it hardcodes the
    filename and caches by mtime, so it cannot be pointed at a git blob.
    """
    out = set()
    for ln in text.splitlines():
        head = ln.split("#", 1)[0].strip()
        if head:
            out.add(head.split()[0])
    return out


def _parse_canonical_text(text: str) -> set[str]:
    """inline_asm_canonical.txt via the ENGINE'S OWN loader — cheats.
    canonical_asm_funcs takes a path, so a git blob can be written to a temp
    file and parsed by the real implementation. No mirror, no drift."""
    with tempfile.NamedTemporaryFile("w", suffix=".txt", delete=False,
                                     encoding="utf-8") as fh:
        fh.write(text)
        tmp = fh.name
    try:
        return set(cheats.canonical_asm_funcs(tmp))
    finally:
        os.unlink(tmp)


_CONFIG_PARSERS = {
    "volatile_extern_allowlist.txt": _parse_allowlist_text,
    cheats.INLINE_ASM_CANONICAL: _parse_canonical_text,
}


def verify_parser_mirror() -> str | None:
    """Fail loudly if the mirrored allowlist parser disagrees with the engine
    loader on the working-tree file. Returns an error string, or None if OK."""
    p = Path("volatile_extern_allowlist.txt")
    if not p.exists():
        return None
    mine = _parse_allowlist_text(p.read_text(encoding="utf-8"))
    vc._volatile_extern_allowlist_cache = None  # force a fresh engine read
    theirs = set(vc._load_volatile_extern_allowlist())
    if mine != theirs:
        return (f"PARSER MIRROR DRIFT: this tool's allowlist parser and "
                f"engine.volatile_cheats disagree on the working tree "
                f"(only-mine={sorted(mine - theirs)}, only-engine="
                f"{sorted(theirs - mine)}). Fix _parse_allowlist_text before "
                f"trusting any config-mode verdict.")
    return None


def run_config(ref: str) -> list[dict]:
    """Diff each detector config's PARSED symbol inventory, working tree vs ref.

    Compares INVENTORIES, not file contents, deliberately: a content hash tells
    you the file changed (which it legitimately does constantly); only the parse
    result distinguishes 'an entry was edited' from 'an entry disappeared'. The
    2026-08-07 glue was a syntactically valid line that parsed to one symbol
    instead of two — invisible to a linter, an LF check, or a hash.
    """
    results: list[dict] = []
    drift = verify_parser_mirror()
    if drift:
        results.append({"name": "parser-mirror", "where": "config",
                        "status": "UNSCORED", "detail": drift})
    for path, parse in _CONFIG_PARSERS.items():
        p = Path(path)
        if not p.exists():
            results.append({"name": path, "where": "config", "status": "UNSCORED",
                            "detail": "file missing"})
            continue
        now = parse(p.read_text(encoding="utf-8"))
        old_text = git_show(ref, path)
        if old_text is None:
            results.append({"name": path, "where": "config", "status": "UNSCORED",
                            "detail": f"not present at {ref}"})
            continue
        old = parse(old_text)
        lost, gained = sorted(old - now), sorted(now - old)
        if lost:
            results.append({
                "name": path, "where": "config", "status": "REGRESSION",
                "distance": len(lost),
                "detail": (f"{len(lost)} symbol(s) present at {ref} no longer parse: "
                           f"{', '.join(lost)}"
                           + (f"; added: {', '.join(gained)}" if gained else "")
                           + f" ({len(now)} parsed now, {len(old)} at {ref}). "
                           "A silently dropped grant re-arms the stripper against a "
                           "sanctioned construct — invisible to the build SHA1.")})
        else:
            results.append({"name": path, "where": "config", "status": "OK",
                            "detail": f"{len(now)} symbol(s) parsed"
                                      + (f"; {len(gained)} added vs {ref}" if gained else "")})
    return results


# ==========================================================================
# MODE roster — COMPLETED-C membership diff vs a git ref
# ==========================================================================
def completed_roster_at(ref: str, symbols: dict[str, list[str]]) -> set[str] | None:
    """The COMPLETED-C set as it was at `ref`, by the same definition, using git
    blobs for every input except the symbol universe.

    LIMITATION, stated because it bounds the verdict: the symbol universe comes
    from the CURRENT build objects (there is no way to enumerate a past build's
    symbols without rebuilding at that ref). So a function that did not exist at
    `ref` appears as an ADDITION, never as a departure — which is the safe
    direction: departures, the thing we are hunting, are never invented.
    """
    qtext = git_show(ref, QUEUE_REL)
    if qtext is None:
        return None
    in_queue = {it["func"] for it in json.loads(qtext).get("items", [])}
    ctext = git_show(ref, cheats.INLINE_ASM_CANONICAL)
    canon = _parse_canonical_text(ctext) if ctext is not None else set()
    rulecount = rule_counts({c: git_show(ref, c)
                             for c in (cheats.REGFIX, cheats.REGFIX2, cheats.ASMFIX)})
    prologued: set[str] = set()
    pj = git_show(ref, cheats.PROLOGUE_CONFIG)
    if pj:
        try:
            prologued |= set(json.loads(pj).keys())
        except json.JSONDecodeError:
            pass
    for path in (cheats.DELAY_SLOT_RA, cheats.FRAME_FIX):
        t = git_show(ref, path)
        if t:
            prologued |= {ln.split()[0] for ln in t.splitlines()
                          if ln.strip() and not ln.strip().startswith("#")}
    src_text = {stem: git_show(ref, f"src/{stem}.c") for stem in symbols}

    # Evaluate the detector with the REF'S allowlist, not the working tree's.
    # Without this the comparison silently applies today's grants to yesterday's
    # source, which makes roster mode structurally blind to exactly the channel
    # it exists for: a dropped grant changes both sides identically, so no
    # departure appears. Verified — with the working-tree allowlist this
    # returned 851 == 851 across the 05d13f9a glue commit and detected nothing.
    al_text = git_show(ref, "volatile_extern_allowlist.txt")
    tmp_al = None
    if al_text is not None:
        with tempfile.NamedTemporaryFile("w", suffix=".txt", delete=False,
                                         encoding="utf-8") as fh:
            fh.write(al_text)
            tmp_al = fh.name
    try:
        with vc.use_allowlist(tmp_al):
            completed, _viol, _nb = classify_pool(symbols, in_queue, canon,
                                                  rulecount, prologued, src_text,
                                                  unknown_is_completed=False)
    finally:
        if tmp_al:
            os.unlink(tmp_al)
    return {f for f, _ in completed}


def run_roster(ref: str, symbols: dict[str, list[str]],
               now_pool: list[tuple[str, str]]) -> list[dict]:
    """A function LEAVING the COMPLETED-C set is a first-class REGRESSION.

    This is the channel the 2026-08-07 incident actually produced: a dropped
    allowlist grant made the construct count as a cheat, which removed the
    function from the pool BEFORE it could be scored. A distance-only check
    reports "all clean" in that state because the regressed functions are no
    longer in the set being checked.
    """
    old = completed_roster_at(ref, symbols)
    if old is None:
        return [{"name": "roster", "where": ref, "status": "UNSCORED",
                 "detail": f"cannot read {QUEUE_REL} at {ref}"}]
    new = {f for f, _ in now_pool}
    stem_of = {f: s for f, s in now_pool}
    for stem, funcs in symbols.items():
        for f in funcs:
            stem_of.setdefault(f, stem)
    departed, joined = sorted(old - new), sorted(new - old)

    # WHY a function departed decides whether it is a finding. Becoming
    # COMPLETED-INLINE-ASM-CANONICAL is an AUTHORIZED reclassification (it
    # requires owner sign-off in inline_asm_canonical.txt), not a regression —
    # measured across the naming wave, which moved _card_write / _new_card into
    # the canonical set. Re-entering the queue or newly carrying a rule or cheat
    # construct IS the regression signature.
    canon_now = cheats.canonical_asm_funcs()
    q = Path(QUEUE_REL)
    queued_now = ({it["func"] for it in json.loads(q.read_text()).get("items", [])}
                  if q.exists() else set())
    rules_now = rule_counts(working_rule_texts())
    results, reclassified = [], []
    for f in departed:
        if f in canon_now:
            reclassified.append(f)
            continue
        if f in queued_now:
            why = "it is back IN THE QUEUE (INCOMPLETE)"
        elif rules_now.get(f, 0):
            why = f"it newly carries {rules_now[f]} regfix/asmfix rule(s)"
        else:
            why = ("it newly carries a cheat construct, or its C body can no "
                   "longer be located")
        results.append({"name": f, "where": stem_of.get(f, "?"), "status": "REGRESSION",
                        "detail": f"was COMPLETED-C at {ref}, is not now — {why}. "
                                  f"Run `--mode config` FIRST: a dropped detector "
                                  f"grant produces exactly this with the C untouched."})
    summary = (f"{len(new)} COMPLETED-C now, {len(old)} at {ref}; "
               f"{len(results)} regression(s)")
    if reclassified:
        summary += (f"; {len(reclassified)} authorized canonical reclassification(s) "
                    f"({', '.join(reclassified[:6])}"
                    f"{'…' if len(reclassified) > 6 else ''})")
    if joined:
        summary += f"; {len(joined)} newly completed"
    if not results:
        results.append({"name": "roster", "where": ref, "status": "OK",
                        "detail": summary})
    else:
        print(f"  (roster: {summary})")
    return results


# ==========================================================================
# MODE attribute
# ==========================================================================
def elf_func_table() -> dict[str, tuple[int, int]]:
    """name -> (vram, size) from build/bb2.elf; zero sizes recomputed from the
    gap to the next function (maspsx does not always emit .size). Same
    derivation engine/fixtures.py uses."""
    rx = re.compile(r"^([0-9a-fA-F]+)\s+\S.*\sF\s+(\S+)\s+([0-9a-fA-F]+)\s+(\S+)\s*$")
    out = subprocess.run([cfg.OBJDUMP, "-t", "build/bb2.elf"],
                         capture_output=True, text=True).stdout
    funcs: dict[str, tuple[int, int]] = {}
    for line in out.splitlines():
        m = rx.match(line)
        if m:
            funcs[m.group(4)] = (int(m.group(1), 16), int(m.group(3), 16))
    addrs = sorted({v for v, _ in funcs.values()})
    nxt = {a: addrs[i + 1] for i, a in enumerate(addrs[:-1])}
    return {n: (v, s if s else max(0, nxt.get(v, v) - v)) for n, (v, s) in funcs.items()}


def run_attribute(selection: list[tuple[str, str]] | None) -> list[dict]:
    built = Path("build/bb2.exe").read_bytes()
    orig = Path(cfg.TARGET_EXE).read_bytes()
    wanted = {f for f, _ in selection} if selection is not None else None
    results: list[dict] = []
    for name, (vram, size) in sorted(elf_func_table().items(), key=lambda kv: kv[1][0]):
        if (wanted is not None and name not in wanted) or size == 0:
            continue
        off = cfg.HEADER_SIZE + (vram - cfg.LOAD_ADDR)
        a, b = built[off:off + size], orig[off:off + size]
        if len(a) != size or len(b) != size:
            results.append({"name": name, "where": "exe", "status": "UNSCORED",
                            "detail": f"vram 0x{vram:08X} +{size} outside the EXE"})
            continue
        if a == b:
            results.append({"name": name, "where": "exe", "status": "OK"})
        else:
            nwords = sum(1 for i in range(0, size, 4) if a[i:i + 4] != b[i:i + 4])
            results.append({"name": name, "where": "exe", "status": "REGRESSION",
                            "distance": nwords,
                            "detail": f"vram 0x{vram:08X} size {size}: {nwords} "
                                      f"differing word(s) vs {cfg.TARGET_EXE}"})
    return results


# ==========================================================================
def report(results: list[dict], header: str) -> int:
    regressions = [r for r in results if r["status"] == "REGRESSION"]
    unscored = [r for r in results if r["status"] == "UNSCORED"]
    okc = sum(1 for r in results if r["status"] == "OK")
    print(f"\n{header}: {okc} OK, {len(regressions)} REGRESSION, "
          f"{len(unscored)} UNSCORED")
    for r in unscored:
        print(f"  ?? {r['name']} ({r['where']}): {r.get('detail', '')}")
    for r in regressions:
        print(f"\n  ** REGRESSION {r['name']} ({r['where']})")
        if r.get("detail"):
            print(f"     {r['detail']}")
    return 1 if (regressions or unscored) else 0


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Spot-check COMPLETED-C functions for silent regressions.")
    ap.add_argument("--mode", choices=("sandbox", "attribute", "config", "roster"),
                    default="sandbox")
    ap.add_argument("-n", type=int, default=25, help="sample size (default 25)")
    ap.add_argument("--seed", type=int, default=None,
                    help="sampling seed (default: the UTC date, so a day's run is "
                         "reproducible and consecutive days cover different functions)")
    ap.add_argument("--all", action="store_true", help="check every COMPLETED-C function")
    ap.add_argument("--func", action="append", default=[], help="check named function(s)")
    ap.add_argument("--file", action="append", default=[],
                    help="check every COMPLETED-C function in src/<stem>.c")
    ap.add_argument("--ref", default="HEAD",
                    help="git ref for config / roster diffs (default HEAD)")
    ap.add_argument("--ignore-locks", action="store_true",
                    help="proceed despite a grinder/closer lock (the mtime "
                         "in-flight check still applies and cannot be skipped)")
    ap.add_argument("--list", action="store_true", help="enumerate the pool and exit")
    ap.add_argument("--json", metavar="PATH", help="also write results as JSON")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    if args.seed is None:
        args.seed = int(time.strftime("%Y%m%d", time.gmtime()))

    if args.mode == "config":
        # Deliberately NO build-in-flight preflight: config mode reads only git
        # objects and the config files, touches no build artifact, and is the
        # one check that stays sound (and sub-second) while a build runs.
        results = run_config(args.ref)
        for r in results:
            mark = {"OK": "  ok", "REGRESSION": "  **", "UNSCORED": "  ??"}[r["status"]]
            print(f"{mark} {r['name']}: {r.get('detail', '')}")
        rc = 1 if [r for r in results if r["status"] != "OK"] else 0
        if args.json:
            Path(args.json).write_text(json.dumps(results, indent=2) + "\n")
        return rc

    # Everything below reads build artifacts, so establish the tree is quiet
    # BEFORE measuring anything (--list included: the pool comes from the
    # object symbol tables).
    quiet, notes = check_no_build_in_flight(args.ignore_locks)
    for n in notes:
        print(f"{'  ' if quiet else '  !! '}{n}")
    if not quiet:
        print("\nRefusing to measure a tree that is being built — the pool, the "
              "reference objects and the oracle SHA1 would all be sampled at "
              "different moments.\n`--mode config` is unaffected and can run now.")
        return 2

    pool, violations, no_body = enumerate_completed_c()
    print(f"COMPLETED-C pool: {len(pool)} functions across "
          f"{len({s for _, s in pool})} built file(s)")
    if violations:
        print(f"\ncompletion-integrity violations ({len(violations)}) — a different "
              f"guard's job, NOT counted as regressions here:")
        for v in violations:
            print(f"  ~~ {v}")
    if no_body:
        print(f"\nno-C-body symbols in the pool ({len(no_body)}) — data extracted as "
              f"code; they will read UNSCORED in sandbox mode. Filtering them is "
              f"OWNER-GATED (it changes the COMPLETED-C count): "
              f"{', '.join(f + ' (' + s + ')' for f, s in sorted(no_body))}")

    if args.list:
        for func, stem in sorted(pool):
            print(f"  {func}\t{stem}.c")
        return 0

    if args.mode == "roster":
        results = run_roster(args.ref, object_symbols(), pool)
        rc = report(results, f"roster vs {args.ref}")
        if args.json:
            Path(args.json).write_text(json.dumps(results, indent=2) + "\n")
        return rc

    selection = select(pool, args)
    if not selection:
        print("nothing selected")
        return 2

    if args.mode == "sandbox":
        ok, msg = check_reference_trust()
        print(f"\n{msg}")
        if not ok:
            return 2
        # A silently-dropped detector grant changes what "honest distance" MEANS
        # without changing a byte of the build, so the green-oracle precondition
        # does not cover it. Run the config + roster checks alongside.
        extra = [r for r in run_config(args.ref) + run_roster(args.ref, object_symbols(), pool)
                 if r["status"] != "OK"]
        for r in extra:
            print(f"  !! {r['name']}: {r.get('detail', '')}")
        print(f"scoring {len(selection)} function(s) in "
              f"{len({s for _, s in selection})} file(s), seed={args.seed}\n")
        results = run_sandbox(selection, args.verbose) + extra
    else:
        if not Path("build/bb2.exe").exists() or not Path("build/bb2.elf").exists():
            print("build/bb2.exe or build/bb2.elf missing — attribute mode needs a "
                  "completed build (it does not need a GREEN one).")
            return 2
        whole = file_sha1(Path("build/bb2.exe"))
        print(f"\nbuild/bb2.exe SHA1 {whole} "
              f"({'== oracle' if whole == oracle_sha1() else '!= ORACLE'})")
        results = run_attribute(None if args.all else selection)

    rc = report(results, f"checked {len(results)}")
    if args.json:
        Path(args.json).parent.mkdir(parents=True, exist_ok=True)
        Path(args.json).write_text(json.dumps(
            {"mode": args.mode, "seed": args.seed, "pool": len(pool),
             "results": results, "violations": violations}, indent=2) + "\n")
    if rc:
        print("\nA COMPLETED-C function carries zero rules, so a non-zero honest "
              "distance means its BYTES moved. Before re-grinding the function, "
              "check `--mode config`: a dropped detector grant produces the same "
              "symptom with the build untouched.")
    else:
        print("\nOK: every sampled COMPLETED-C function still measures 0.")
    return rc


if __name__ == "__main__":
    raise SystemExit(main())

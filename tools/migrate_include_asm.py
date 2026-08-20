#!/usr/bin/env python3
"""asm-until-matched migration (owner ruling 2026-08-19, .claude/rules/asm-until-matched.md).

Per function: bank the committed chassis + rule stack into the ledger, replace
the C body with `INCLUDE_ASM("asm/funcs", <func>);`, delete every
regfix/regfix_stage2/asmfix rule keyed to it, rebuild, and verify the full-build
SHA1 against the oracle. Rollback on any failure. Run under WSL python (LF).

Usage:
  python3 tools/migrate_include_asm.py --funcs f1,f2 [--dry-run]
  python3 tools/migrate_include_asm.py --batch N [--dry-run]   # next N queue items not yet migrated
"""
import argparse
import datetime
import json
import os
import re
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
os.chdir(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from engine import inlineasm  # noqa: E402

ORACLE = "62efab4f73f992798c43e8c730aa43baa10bb4fa"
RULE_FILES = ["regfix.txt", "regfix_stage2.txt", "asmfix.txt"]
TODAY = datetime.date.today().isoformat()
BANK_DIR = "retired-chassis-2026-08"


def rd(p):
    with open(p, "rb") as f:
        return f.read().decode("utf-8")


def wr(p, s):
    with open(p, "wb") as f:
        f.write(s.encode("utf-8"))


def rule_lines(func):
    """[(file, lineno, text)] for every rule keyed to func."""
    out = []
    pat = re.compile(r"^" + re.escape(func) + r"\s*:")
    for rf in RULE_FILES:
        if not os.path.isfile(rf):
            continue
        for i, ln in enumerate(rd(rf).split("\n")):
            if pat.match(ln.strip()):
                out.append((rf, i, ln))
    return out


def sandbox_floor(func):
    """Honest pre-migration floor via the engine sandbox. None on failure."""
    try:
        r = subprocess.run(
            ["python3", "-m", "engine.cli", "sandbox", func, "--disable", "all"],
            capture_output=True, text=True, timeout=600)
        m = re.search(r'"score"\s*:\s*(\d+)', r.stdout)
        return int(m.group(1)) if m else None
    except Exception:
        return None


def build_sha1():
    # Engine clean-driver build, NOT bare make: Windows-side writes leave
    # future-dated files that make's clock-skew handling treats as up to date,
    # so make can exit "successfully" having built nothing (observed 2026-08-19
    # during the prototype run — sha1 never printed, tool saw None).
    r = subprocess.run(["python3", "-m", "engine.cli", "build"],
                       capture_output=True, text=True, timeout=3600)
    m = re.search(r"sha1[\"\s:]+([0-9a-f]{40})", r.stdout + r.stderr)
    if m:
        return m.group(1), r.stdout + r.stderr
    return None, r.stdout + r.stderr


def find_body_span(text, func):
    span = inlineasm._func_body_span(text, func)
    if span is None:
        return None
    lo, hi = span
    # _func_body_span's lo is the SIGNATURE start; its hi is already one PAST
    # the closing brace (engine/inlineasm._match_brace contract) — do NOT +1,
    # that eats the following newline and glues the next definition onto the
    # INCLUDE_ASM line (observed 2026-08-19: two adjacent functions dropped out
    # of the COMPLETED census because their signatures no longer sat at line
    # start). Snap lo to its line start; return hi as-is.
    line_start = text.rfind("\n", 0, lo) + 1
    if hi <= line_start:
        return None  # defensive: never hand back an inverted span
    return line_start, hi


def migrate_one(func, stem, dry, defer_build=False):
    src = f"src/{stem}.c"
    asm = f"asm/funcs/{func}.s"
    text = rd(src)
    if f'INCLUDE_ASM("asm/funcs", {func});' in text:
        return "already-migrated", []
    if not os.path.isfile(asm):
        return f"REFUSED: {asm} missing", []
    span = find_body_span(text, func)
    if span is None:
        return f"REFUSED: no C body span for {func} in {src}", []
    lo, hi = span
    body = text[lo:hi]
    rules = rule_lines(func)
    if dry:
        return ("DRY: would bank %d-line body + %d rule(s), splice INCLUDE_ASM"
                % (body.count("\n") + 1, len(rules))), rules

    floor = sandbox_floor(func)

    led = f"memory/grind/{func}"
    bank = os.path.join(led, BANK_DIR)
    os.makedirs(bank, exist_ok=True)
    wr(os.path.join(bank, "body.c"), body + "\n")
    wr(os.path.join(bank, "rules.txt"),
       "".join(f"{rf}:{no + 1}: {ln}\n" for rf, no, ln in rules) or "(no rules)\n")
    pin = {"floor": floor, "measured": TODAY, "source": "pre-migration chassis",
           "banked_body": f"{BANK_DIR}/body.c", "rules_retired": len(rules)}
    wr(os.path.join(led, "migration_pin.json"), json.dumps(pin, indent=2) + "\n")

    # splice
    new_text = text[:lo] + f'INCLUDE_ASM("asm/funcs", {func});' + text[hi:]
    wr(src, new_text)
    # delete rules (by exact line content, from each file)
    originals = {}
    for rf in RULE_FILES:
        if not os.path.isfile(rf):
            continue
        originals[rf] = rd(rf)
        keep = [ln for ln in originals[rf].split("\n")
                if not re.match(r"^" + re.escape(func) + r"\s*:", ln.strip())]
        wr(rf, "\n".join(keep))

    if defer_build:
        return f"EDITED (build deferred): floor-pin {floor}, {len(rules)} rule(s) retired", rules

    sha, log = build_sha1()
    if sha != ORACLE:
        # rollback everything
        wr(src, text)
        for rf, orig in originals.items():
            wr(rf, orig)
        shutil.rmtree(bank, ignore_errors=True)
        try:
            os.remove(os.path.join(led, "migration_pin.json"))
        except OSError:
            pass
        tail = "\n".join(log.split("\n")[-15:])
        return f"FAILED: sha1={sha} != oracle — ROLLED BACK. Build tail:\n{tail}", rules
    return f"OK: floor-pin {floor}, {len(rules)} rule(s) retired, SHA1 MATCH", rules


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--funcs", default="")
    ap.add_argument("--batch", type=int, default=0)
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--batch-verify", action="store_true",
                    help="edit all targets first, then ONE engine build + SHA1 "
                         "gate for the whole batch (git restores on mismatch)")
    a = ap.parse_args()

    q = json.load(open("engine/queue.json"))
    by_func = {it["func"]: it for it in q["items"]}
    if a.funcs:
        targets = [f.strip() for f in a.funcs.split(",") if f.strip()]
    else:
        targets = []
        for it in q["items"]:
            if it.get("status") not in ("active", "parked"):
                continue
            try:
                if f'INCLUDE_ASM("asm/funcs", {it["func"]});' in rd(f'src/{it["file"]}.c'):
                    continue  # already migrated
            except OSError:
                continue
            targets.append(it["func"])
            if len(targets) >= a.batch:
                break

    failed = []
    edited = []
    for func in targets:
        it = by_func.get(func)
        if not it:
            print(f"{func}: NOT IN QUEUE — skipped")
            continue
        res, _ = migrate_one(func, it["file"], a.dry_run,
                             defer_build=a.batch_verify)
        print(f"{func}: {res}")
        if res.startswith("EDITED"):
            edited.append(func)
        if res.startswith(("FAILED", "REFUSED")):
            failed.append(func)
            with open("tmp/migration-deferred.txt", "a", encoding="utf-8") as f:
                f.write(f"{func}\t{res.splitlines()[0]}\n")
    if a.batch_verify and edited and not a.dry_run:
        sha, log = build_sha1()
        if sha == ORACLE:
            print(f"\nBATCH VERIFIED: {len(edited)} function(s), SHA1 MATCH")
        else:
            print(f"\nBATCH FAILED: sha1={sha} != oracle. NOT rolling back "
                  f"automatically in batch mode — use `git checkout -- src "
                  f"regfix.txt regfix_stage2.txt asmfix.txt` (ledger banks are "
                  f"additive and safe to keep), or bisect with per-function mode.")
            print("\n".join(log.split("\n")[-15:]))
            sys.exit(1)
    if failed:
        print(f"\n{len(failed)} deferred: {', '.join(failed)}")
        sys.exit(1)


if __name__ == "__main__":
    main()

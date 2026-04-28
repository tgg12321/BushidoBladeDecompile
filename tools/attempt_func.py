#!/usr/bin/env python3
"""Single-shot mechanical match pipeline -- the session driver.

Runs all the cheap automatable steps for a function, in order, and stops on
the first that succeeds:

    1. classify_func.py     -- triage; skip BIOS / multi-jr / known blockers
    2. inline-setup OR setup -- stage permuter/<func>/ with target.s + base.c
    3. smart_match.py       -- 16-strategy automated transformation sweep
    4. permute_capped.py    -- bounded permuter run with flat-score early kill
    5. gen_regfix.py        -- emit candidate regfix rules from final diff

Returns one structured line:

    attempt: <func> result=<MATCHED|NEAR_MISS|HARD|SKIPPED>
             score=<best> stage=<which step it stopped at>
             reason=<short tag> elapsed=<s>s

Result tags:

    MATCHED      score==0; smart_match or permute_capped found a byte match
    NEAR_MISS    score>0 but small; gen_regfix output saved to /tmp/<func>.regfix
                  -- model can review and apply rules with `dc.sh add-regfix`
    HARD         classifier flagged a structural blocker (multi-jr, jlabel, etc)
    SKIPPED      classifier flagged BIOS/syscall/psyq-stdlib (use library form)

The model only intervenes for NEAR_MISS. Everything else gets the right
deterministic verdict without burning model tokens on already-mechanical work.
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TOOLS = ROOT / "tools"


def run(cmd: list[str], timeout: int | None = None) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, capture_output=True, text=True,
                          cwd=str(ROOT), timeout=timeout)


def classify_step(func: str) -> dict:
    """Run classify_func.py --json and return the dict."""
    r = run([sys.executable, str(TOOLS / "classify_func.py"), func, "--json"], timeout=30)
    if r.returncode != 0:
        return {"func": func, "found": False, "recommendation": "not_found",
                "raw_stderr": r.stderr.strip()}
    try:
        return json.loads(r.stdout)
    except json.JSONDecodeError:
        return {"func": func, "found": False, "recommendation": "parse_error",
                "raw_stdout": r.stdout, "raw_stderr": r.stderr}


def setup_step(func: str, kind: str) -> tuple[bool, str]:
    """Stage the permuter dir based on src/ kind. kind in
       {include_asm, inline_asm, none}. Returns (ok, log)."""
    if kind == "include_asm":
        # find the src file
        src_file = None
        for p in sorted((ROOT / "src").glob("*.c")):
            txt = p.read_text(encoding="utf-8", errors="ignore")
            if f'INCLUDE_ASM("asm/funcs", {func})' in txt:
                src_file = str(p.relative_to(ROOT))
                break
        if not src_file:
            return False, "INCLUDE_ASM stub claimed but not found in src/"
        r = run(["bash", str(TOOLS / "permuter_setup.sh"), func, src_file], timeout=120)
        return (ROOT / "permuter" / func / "target.o").exists(), r.stdout + r.stderr
    if kind == "inline_asm":
        r = run([sys.executable, str(TOOLS / "extract_inline_asm.py"),
                 "setup", func], timeout=120)
        return (ROOT / "permuter" / func / "target.o").exists(), r.stdout + r.stderr
    # No src record: try the asm/funcs/.s direct path via inline-setup which
    # falls back gracefully when the .s exists.
    r = run([sys.executable, str(TOOLS / "extract_inline_asm.py"),
             "setup", func], timeout=120)
    return (ROOT / "permuter" / func / "target.o").exists(), r.stdout + r.stderr


def smart_match_step(func: str, timeout: int) -> tuple[str | None, str]:
    """Run smart_match.py --func F. Return (matched_strategy_or_None, log)."""
    try:
        r = run([sys.executable, str(TOOLS / "smart_match.py"),
                 "--func", func], timeout=timeout)
    except subprocess.TimeoutExpired:
        return None, "smart_match: timeout"
    log = r.stdout + r.stderr
    # smart_match prints "MATCH via <strategy>!" on success
    import re as _re
    m = _re.search(r"MATCH via (\w+)!", log)
    if m:
        return m.group(1), log
    return None, log


def permute_step(func: str, max_time: int, max_flat: int, threads: int) -> dict:
    """Run permute_capped.py and parse the summary line."""
    r = run([sys.executable, str(TOOLS / "permute_capped.py"), func,
             "--max-time", str(max_time),
             "--max-flat-seconds", str(max_flat),
             "-j", str(threads)], timeout=max_time + 30)
    log = r.stdout + r.stderr
    import re as _re
    m = _re.search(
        r"permute_capped: \S+ matched=(\w+) best=(\S+) attempts=(\d+) "
        r"elapsed=(\d+)s reason=(\w+)",
        log,
    )
    if not m:
        return {"matched": False, "best": None, "attempts": 0,
                "elapsed": 0, "reason": "parse_failed", "raw": log}
    return {
        "matched": m.group(1) == "Y",
        "best": int(m.group(2)) if m.group(2).isdigit() else None,
        "attempts": int(m.group(3)),
        "elapsed": int(m.group(4)),
        "reason": m.group(5),
        "raw": log,
    }


def gen_regfix_step(func: str) -> tuple[str | None, str]:
    """Run gen_regfix.py and capture stdout."""
    try:
        r = run([sys.executable, str(TOOLS / "gen_regfix.py"), func], timeout=60)
    except subprocess.TimeoutExpired:
        return None, "gen_regfix: timeout"
    if r.returncode != 0:
        return None, r.stderr.strip()
    return r.stdout, r.stderr


# ---- Recommendation buckets that should be SKIPPED at classify time ----

SKIP_TAGS = {"bios_or_syscall", "psyq_stdlib"}
HARD_TAGS = {"multi_function", "needs_rodata_split", "aspsx_delay_swra"}


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--time", type=int, default=120,
                    help="permute_capped --max-time (default: 120)")
    ap.add_argument("--max-flat", type=int, default=45,
                    help="permute_capped --max-flat-seconds (default: 45)")
    ap.add_argument("--threads", type=int, default=4,
                    help="permuter parallelism (default: 4)")
    ap.add_argument("--smart-time", type=int, default=180,
                    help="smart_match cap in seconds (default: 180)")
    ap.add_argument("--no-permute", action="store_true",
                    help="Skip the permuter stage (use after smart_match only)")
    ap.add_argument("--force", action="store_true",
                    help="Run even when classifier flags HARD/SKIPPED")
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()

    started = time.monotonic()
    func = args.func

    # Step 1 -- classify
    cls = classify_step(func)
    if not cls.get("found", False):
        elapsed = int(time.monotonic() - started)
        line = (f"attempt: {func} result=NOT_FOUND score=? stage=classify "
                f"reason=no_asm elapsed={elapsed}s")
        print(json.dumps(cls) if args.json else line)
        return 1

    rec = cls["recommendation"]
    rec_root = rec.split(":")[0]

    if not args.force:
        if rec_root in SKIP_TAGS or rec_root.startswith("psyq_stdlib"):
            elapsed = int(time.monotonic() - started)
            print(f"attempt: {func} result=SKIPPED score=? stage=classify "
                  f"reason={rec} elapsed={elapsed}s")
            return 0
        if rec_root in HARD_TAGS:
            elapsed = int(time.monotonic() - started)
            print(f"attempt: {func} result=HARD score=? stage=classify "
                  f"reason={rec} elapsed={elapsed}s")
            return 1

    # Step 2 -- setup permuter dir
    src_kind = cls.get("src", {}).get("kind", "none")
    ok, setup_log = setup_step(func, src_kind)
    if not ok:
        elapsed = int(time.monotonic() - started)
        print(f"attempt: {func} result=ERROR score=? stage=setup "
              f"reason=setup_failed elapsed={elapsed}s")
        sys.stderr.write(setup_log[-800:] + "\n")
        return 1

    # Step 3 -- smart_match
    strategy, sm_log = smart_match_step(func, args.smart_time)
    if strategy:
        elapsed = int(time.monotonic() - started)
        # smart_match writes auto_matches/<func>.c on success
        match_path = ROOT / "auto_matches" / f"{func}.c"
        print(f"attempt: {func} result=MATCHED score=0 stage=smart "
              f"reason=strategy:{strategy} elapsed={elapsed}s")
        if match_path.exists():
            print(f"  -> matched C saved to {match_path.relative_to(ROOT)}")
        return 0

    # Step 4 -- permute_capped
    if args.no_permute:
        # Read last score from smart_match output
        elapsed = int(time.monotonic() - started)
        print(f"attempt: {func} result=NEAR_MISS score=? stage=smart "
              f"reason=no_permute_flag elapsed={elapsed}s")
        return 1

    pres = permute_step(func, args.time, args.max_flat, args.threads)
    if pres["matched"]:
        elapsed = int(time.monotonic() - started)
        print(f"attempt: {func} result=MATCHED score=0 stage=permute "
              f"reason=permuter elapsed={elapsed}s attempts={pres['attempts']}")
        return 0

    best = pres["best"]
    elapsed = int(time.monotonic() - started)

    # Step 5 -- gen_regfix on final diff
    gr_out, gr_err = gen_regfix_step(func)
    regfix_path = None
    if gr_out and gr_out.strip():
        regfix_path = Path(f"/tmp/{func}.regfix.suggestions")
        try:
            regfix_path.write_text(gr_out, encoding="utf-8", newline="\n")
        except OSError:
            regfix_path = None

    # Verdict
    if best is None:
        verdict = "ERROR"
        reason = f"permuter:{pres['reason']}"
    elif best <= 200:
        verdict = "NEAR_MISS"
        reason = f"permute_{pres['reason']}_score{best}"
    else:
        verdict = "HARD"
        reason = f"permute_{pres['reason']}_score{best}"

    summary = (f"attempt: {func} result={verdict} score={best} "
               f"stage=permute reason={reason} "
               f"elapsed={elapsed}s attempts={pres['attempts']}")
    print(summary)
    if regfix_path:
        print(f"  -> regfix suggestions: {regfix_path}")
    return 0 if verdict == "MATCHED" else 1


if __name__ == "__main__":
    sys.exit(main())

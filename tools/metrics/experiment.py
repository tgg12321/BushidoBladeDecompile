"""tools/metrics/experiment.py — controlled re-derivation experiment harness.

Measures how an agent (model x effort) performs at re-deriving an
already-matched, pure-C function from a blanked stub, end-to-end through the
real engine, under a hard time + dollar budget. Writes one authoritative record
to metrics/experiments.jsonl and syncs it into Postgres for the dashboard.

    python tools/metrics/experiment.py --func game_SetPlayerCount \
        --model opus --effort high --budget-usd 8 --minutes 20

Flow: preflight PG -> (baseline rebuild) -> snapshot + blank the function ->
confirm the gap -> launch a headless `claude` agent (hard --max-budget-usd cap +
a wall-clock subprocess timeout) -> gate with sandbox (+ optional full SHA1) ->
ALWAYS restore the source and leave build/ clean -> record + sync.

Two correctness guarantees baked in:
  * The harness's OWN engine calls run with BB2_METRICS_DISABLE=1, so only the
    AGENT's engine commands ever reach metrics/events.jsonl (clean attribution).
  * Cost/tokens are taken from the CLI result JSON (total_cost_usd) — exact,
    not the inflated transcript-sum estimate.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EXPERIMENTS_LOG = ROOT / "metrics" / "experiments.jsonl"
DSN = "host=127.0.0.1 port=5432 user=postgres dbname=bb2_metrics"


def _wsl_path(p: Path) -> str:
    s = str(p)
    return f"/mnt/{s[0].lower()}{s[2:].replace(chr(92), '/')}"


WSL_ROOT = _wsl_path(ROOT)


def write_lf(path: Path, text: str):
    """Write text with LF endings ALWAYS. Windows Path.write_text emits CRLF,
    which breaks the GNU toolchain (and trips the CRLF tooling-incident guard) —
    the exact footgun this harness must never hit."""
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)


# ---------------------------------------------------------------------------
# engine driver (orchestrator calls — metrics DISABLED so they don't pollute)
# ---------------------------------------------------------------------------
def engine(args: str, timeout: int = 600) -> subprocess.CompletedProcess:
    bash = (f'cd "{WSL_ROOT}" && source .venv/bin/activate && '
            f'BB2_METRICS_DISABLE=1 python3 -m engine.cli {args}')
    return subprocess.run(["wsl", "bash", "-c", bash],
                          capture_output=True, text=True, timeout=timeout)


def engine_json(args: str, timeout: int = 600) -> dict:
    r = engine(args, timeout)
    txt = r.stdout
    s, e = txt.find("{"), txt.rfind("}")
    if s >= 0 and e > s:
        return json.loads(txt[s:e + 1])
    raise RuntimeError(f"engine {args!r} gave no JSON.\nstdout:{txt[:300]}\nstderr:{r.stderr[:300]}")


def sandbox_score(func: str) -> int:
    return engine_json(f"sandbox {func} --disable all")["score"]


# ---------------------------------------------------------------------------
# function blanking (comment/string-aware brace matcher)
# ---------------------------------------------------------------------------
def find_body_span(src: str, name: str):
    """Return (sig_start, body_open_idx, body_close_idx) for the DEFINITION of
    `name`, or None. Skips // and /* */ comments and string/char literals."""
    for m in re.finditer(r"(?<![A-Za-z0-9_])" + re.escape(name) + r"\s*\(", src):
        j, depth = m.end() - 1, 0          # at '('
        while j < len(src):
            if src[j] == "(":
                depth += 1
            elif src[j] == ")":
                depth -= 1
                if depth == 0:
                    break
            j += 1
        k = j + 1
        while k < len(src) and src[k] in " \t\r\n":
            k += 1
        if k >= len(src) or src[k] != "{":   # a declaration/prototype, not a body
            continue
        # brace-match the body, skipping comments + literals
        p, depth = k, 0
        while p < len(src):
            c = src[p]
            if c == "/" and p + 1 < len(src) and src[p + 1] == "/":
                p = src.find("\n", p)
                p = len(src) if p < 0 else p
                continue
            if c == "/" and p + 1 < len(src) and src[p + 1] == "*":
                end = src.find("*/", p + 2)
                p = len(src) if end < 0 else end + 2
                continue
            if c in "\"'":
                q = p + 1
                while q < len(src):
                    if src[q] == "\\":
                        q += 2
                        continue
                    if src[q] == c:
                        break
                    q += 1
                p = q + 1
                continue
            if c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return m.start(), k, p
            p += 1
    return None


def blank_function(src_file: Path, func: str) -> str:
    """Replace the function body with a stub. Returns the ORIGINAL text."""
    original = src_file.read_text(encoding="utf-8")
    span = find_body_span(original, func)
    if not span:
        raise RuntimeError(f"could not locate definition of {func} in {src_file}")
    _, open_i, close_i = span
    stub = "{\n    /* EXPERIMENT: body blanked for re-derivation */\n}"
    write_lf(src_file, original[:open_i] + stub + original[close_i + 1:])
    return original


# ---------------------------------------------------------------------------
# the agent
# ---------------------------------------------------------------------------
PROMPT = """You are doing a focused decompilation matching task on the Bushido Blade 2 project. Be efficient — budget ~{minutes} minutes / ~150k tokens.

## Task
The body of `{func}` in `src/{file}.c` has been blanked (replaced with a stub). Reconstruct it in **pure C** so it compiles byte-identically to the original. Success = the engine's cheat-free distance reaches **0**.

## Engine (run in WSL from the repo root)
```
wsl bash -c 'cd "{wsl_root}" && source .venv/bin/activate && python3 -m engine.cli <CMD>'
```
- Score:  `sandbox {func} --disable all`  -> JSON `"score"`. 0 = matched. Re-run after each edit.
- Target: `diagnose {func} --detail`       -> per-instruction diff (TGT vs your BLD). Use it to see exactly what the original does.
- Route:  `canonical {func}`               -> confirms pure-C target.

## Edit only the body of `{func}` in `src/{file}.c`.

## HARD CONSTRAINTS
- Do NOT run `verify-oracle`, `build`, `make`, or `retire`. Use ONLY `sandbox`, `diagnose`, `canonical`.
- Do NOT edit any other function/file. Do NOT git commit/checkout/stash/restore.
- Pure C only: no regfix/asmfix, no `register asm()` pins, no `__asm__`.

## When done
Stop as soon as `sandbox {func} --disable all` is `"score": 0`. Report: final score, # of sandbox iterations, the final C body, and one line on anything tricky. If you run out of budget, report your best score and the current C.
"""


def run_agent(func, file, model, effort, budget, minutes):
    claude = shutil.which("claude") or "claude"
    prompt = PROMPT.format(func=func, file=file, wsl_root=WSL_ROOT, minutes=minutes)
    args = [claude, "--print", "--model", model, "--effort", effort,
            "--output-format", "json", "--max-budget-usd", str(budget),
            "--permission-mode", "bypassPermissions", prompt]
    t0 = time.time()
    try:
        r = subprocess.run(args, capture_output=True, text=True,
                           timeout=minutes * 60, cwd=str(ROOT),
                           stdin=subprocess.DEVNULL)
        wall = time.time() - t0
        try:
            return json.loads(r.stdout), wall, "completed"
        except Exception:
            return {"_stdout": r.stdout[:1000], "_stderr": r.stderr[:1000]}, wall, "error"
    except subprocess.TimeoutExpired:
        return {}, time.time() - t0, "timeout-killed"


# ---------------------------------------------------------------------------
def git_commit() -> str:
    try:
        return subprocess.run(["git", "-C", str(ROOT), "rev-parse", "HEAD"],
                              capture_output=True, text=True).stdout.strip()[:12]
    except Exception:
        return ""


def main():
    ap = argparse.ArgumentParser(description="re-derivation experiment harness")
    ap.add_argument("--func", required=True)
    ap.add_argument("--model", default="opus")
    ap.add_argument("--effort", default="high",
                    choices=["low", "medium", "high", "xhigh", "max"])
    ap.add_argument("--budget-usd", type=float, default=8.0)
    ap.add_argument("--minutes", type=int, default=20)
    ap.add_argument("--assume-clean", action="store_true",
                    help="skip the baseline rebuild (build/ already pristine)")
    ap.add_argument("--no-verify", action="store_true",
                    help="skip the full-build SHA1 gate (rely on sandbox score 0)")
    a = ap.parse_args()

    # 0. preflight (hard gate) — we WANT to know if metrics can't be recorded
    rc = subprocess.run([sys.executable, str(Path(__file__).parent / "preflight.py")])
    if rc.returncode != 0:
        sys.exit("preflight failed — start Postgres before running an experiment.")

    src_file = ROOT / "src" / "config.c"          # resolved below from sandbox
    # 1. baseline pristine build + locate the file + confirm it's a matched pure-C target
    if not a.assume_clean:
        print("[1/7] baseline rebuild ...")
        r = engine_json("verify-oracle --rebuild")
        if not r.get("build_matches"):
            sys.exit("baseline build does not match the oracle — fix the tree first.")
    print(f"[2/7] confirming {a.func} is a matched pure-C target ...")
    sb = engine_json(f"sandbox {a.func} --disable all")
    src_file = ROOT / "src" / f"{sb['file']}.c"
    start_score = sb["score"]
    if start_score != 0:
        sys.exit(f"{a.func} is not currently a clean match (score={start_score}); pick another.")

    # 2. snapshot + blank
    print(f"[3/7] blanking {a.func} in {src_file.name} ...")
    original = blank_function(src_file, a.func)
    matched = full_sha1 = None
    gap = final_score = None
    result, wall, terminal = {}, 0.0, "error"
    try:
        gap = sandbox_score(a.func)
        print(f"      gap opened: score={gap}")
        # 3. the agent
        print(f"[4/7] running agent: {a.model}/{a.effort}  cap=${a.budget_usd}/{a.minutes}min ...")
        result, wall, terminal = run_agent(a.func, sb["file"], a.model, a.effort,
                                           a.budget_usd, a.minutes)
        # 4. gate
        print("[5/7] gating (sandbox) ...")
        final_score = sandbox_score(a.func)
        matched = final_score == 0
        print(f"      final score={final_score}  matched={matched}  terminal={terminal}")
        if matched and not a.no_verify:
            print("[6/7] honest gate: full-build SHA1 ...")
            try:
                full_sha1 = engine_json("verify-oracle --rebuild").get("build_matches")
            except Exception as ex:
                full_sha1 = False
                print(f"      honest gate ERRORED (recorded as no-match): {ex}")
            print(f"      full_sha1_match={full_sha1}")
    finally:
        # 5. ALWAYS restore source (LF!) + leave build/ clean
        print("[7/7] restoring source + build ...")
        try:
            write_lf(src_file, original)
            # build/ is pristine if we never rebuilt from agent code (no honest
            # gate) or if it matched (agent bytes == oracle); else rebuild clean.
            engine_json("verify-oracle" if full_sha1 else "verify-oracle --rebuild")
        except Exception as ex:
            print(f"      WARNING: restore/cleanup hit an error: {ex}")

    # 6. record (authoritative numbers from the CLI result JSON)
    usage = result.get("usage", {}) if isinstance(result, dict) else {}
    rec = {
        "ts": datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "func": a.func, "file": sb["file"], "model": a.model, "effort": a.effort,
        "session_id": result.get("session_id"),
        "budget_usd": a.budget_usd, "minutes": a.minutes,
        "matched": bool(matched), "start_score": gap, "final_score": final_score,
        "full_sha1_match": full_sha1,
        "total_cost_usd": result.get("total_cost_usd"),
        "input_tokens": usage.get("input_tokens"),
        "output_tokens": usage.get("output_tokens"),
        "cache_read_tokens": usage.get("cache_read_input_tokens"),
        "cache_creation_tokens": usage.get("cache_creation_input_tokens"),
        "num_turns": result.get("num_turns"),
        "duration_s": (result.get("duration_ms") or 0) / 1000.0 or None,
        "wall_s": round(wall, 1),
        "terminal_reason": result.get("terminal_reason") or terminal,
        "git_commit": git_commit(),
        "notes": (result.get("result") or "")[:500] if isinstance(result, dict) else "",
    }
    EXPERIMENTS_LOG.parent.mkdir(parents=True, exist_ok=True)
    with open(EXPERIMENTS_LOG, "a", encoding="utf-8", newline="\n") as f:
        f.write(json.dumps(rec) + "\n")

    # 7. sync into Postgres
    print("[sync] loading metrics ...")
    subprocess.run([sys.executable, str(Path(__file__).parent / "sync.py")])

    print("\n=== experiment complete ===")
    print(f"  {a.func}  {a.model}/{a.effort}  matched={rec['matched']}  "
          f"full_sha1={full_sha1}  cost=${rec['total_cost_usd']}  "
          f"turns={rec['num_turns']}  wall={rec['wall_s']}s  terminal={rec['terminal_reason']}")


if __name__ == "__main__":
    main()

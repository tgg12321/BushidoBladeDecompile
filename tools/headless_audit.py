#!/usr/bin/env python3
"""Compact post-hoc audit of headless (claude -p) decomp runs.

Reads a session TRANSCRIPT (the big ~/.claude/projects/<munged>/<sid>.jsonl) and
emits a few lines of signal -- turns, tokens, est cost, tool-call mix, tooling
errors, footgun-guard blocks, duplicate (retried) commands, and the engine-loop
trace -- so you can tell whether an agent was EFFICIENT or wasted time WITHOUT
ever pulling the transcript into context.

Usage:
  python3 tools/headless_audit.py --all                 # one line per headless run (metrics/headless_runs.jsonl)
  python3 tools/headless_audit.py --latest              # newest transcript in the project dir
  python3 tools/headless_audit.py --session <uuid>      # by session id
  python3 tools/headless_audit.py --transcript <path>   # explicit file
  add --json for machine output; --flags lists only runs with concerning signals.

The point: the digest is small. The transcript stays on disk.
"""
from __future__ import annotations

import argparse
import json
import re
from collections import Counter
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
RUNLOG = REPO / "metrics" / "headless_runs.jsonl"

# Candidate locations for the Claude Code projects dir (WSL view first, then
# native-home, then an explicit env override). We pick the subdir for THIS repo.
PROJECTS_CANDIDATES = [
    Path("/mnt/c/Users/Trenton/.claude/projects"),
    Path.home() / ".claude" / "projects",
]

# Genuine TOOLING failures -- the "silly tooling errors" to catch: the agent
# fighting the shell/tools (not the engine giving honest negative feedback).
ERROR_SIGNATURES = [
    "blocked by",                                  # any PreToolUse hook block
    "hook error",
    "traceback (most recent call",                 # a tool/script crashed
    "command not found",
    "is not recognized as the name of a cmdlet",   # PowerShell command-not-found
    "no such file or directory",
    "permission denied",
]
FOOTGUN_SIG = "blocked by shell_footgun_guard"
# Honest engine NEGATIVES -- normal decomp feedback, NOT tooling errors. Tracked
# separately (informational); they do NOT trigger the REVIEW flag.
ENGINE_NEGATIVE_SIGNATURES = ["rolled back", "sha1 mismatch", " mismatch"]
# benign WSL noise we explicitly do NOT count as anything
NOISE_SIG = "failed to start the systemd user session"


def find_projects_dir(override: str | None) -> Path | None:
    cands = [Path(override)] if override else list(PROJECTS_CANDIDATES)
    for c in cands:
        if c.is_dir():
            for sub in c.iterdir():
                if sub.is_dir() and sub.name.endswith("Bushido-Blade-2-Decompile"):
                    return sub
            if c.name.endswith("Bushido-Blade-2-Decompile"):
                return c
    return None


def _result_text(block: dict) -> str:
    c = block.get("content")
    if isinstance(c, str):
        return c
    if isinstance(c, list):
        return " ".join(
            (x.get("text", "") if isinstance(x, dict) else str(x)) for x in c
        )
    return ""


def digest_transcript(path: Path) -> dict:
    turns = 0
    tool_calls = Counter()
    in_tok = out_tok = cache_read = cache_create = 0
    error_results = 0
    footgun_blocks = 0
    engine_rollbacks = 0
    err_breakdown = Counter()  # which tooling-failure signature fired, for triage
    err_samples = []           # bounded short snippets of erroring results
    cmds = Counter()           # normalized Bash/PowerShell command strings
    eng_subcmds = Counter()    # eng.ps1 / engine.cli subcommands invoked
    first_ts = last_ts = None

    with path.open(encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                o = json.loads(line)
            except Exception:
                continue
            ts = o.get("timestamp")
            if ts:
                first_ts = first_ts or ts
                last_ts = ts
            msg = o.get("message") if isinstance(o.get("message"), dict) else {}
            if o.get("type") == "assistant":
                turns += 1
                u = msg.get("usage") or {}
                in_tok += u.get("input_tokens", 0) or 0
                out_tok += u.get("output_tokens", 0) or 0
                cache_read += u.get("cache_read_input_tokens", 0) or 0
                cache_create += u.get("cache_creation_input_tokens", 0) or 0
            content = msg.get("content")
            if not isinstance(content, list):
                continue
            for b in content:
                if not isinstance(b, dict):
                    continue
                bt = b.get("type")
                if bt == "tool_use":
                    name = b.get("name", "?")
                    tool_calls[name] += 1
                    if name in ("Bash", "PowerShell"):
                        cmd = (b.get("input", {}) or {}).get("command", "")
                        norm = re.sub(r"\s+", " ", cmd).strip()
                        if norm:
                            cmds[norm] += 1
                        for m in re.finditer(r"eng\.ps1\s+([a-z][\w-]*(?:\s+[a-z][\w-]*)?)", cmd):
                            eng_subcmds[m.group(1).split()[0]] += 1
                        for m in re.finditer(r"engine\.cli\s+([a-z][\w-]*)", cmd):
                            eng_subcmds[m.group(1)] += 1
                elif bt == "tool_result":
                    txt = _result_text(b).lower()
                    if not txt:
                        continue
                    if FOOTGUN_SIG in txt:
                        footgun_blocks += 1
                    masked = txt.replace(NOISE_SIG, "")
                    matched = [sig for sig in ERROR_SIGNATURES if sig in masked]
                    if matched:
                        error_results += 1
                        for sig in matched:
                            err_breakdown[sig] += 1
                        if len(err_samples) < 8:
                            snip = re.sub(r"\s+", " ", _result_text(b)).strip()[:160]
                            err_samples.append(snip)
                    if any(sig in masked for sig in ENGINE_NEGATIVE_SIGNATURES):
                        engine_rollbacks += 1

    dup = {c: n for c, n in cmds.most_common() if n >= 2}
    # ROUGH cost hint for standalone audits with no run record. The headless
    # runner's cost_usd (from `claude --output-format json`) is authoritative;
    # this is only a ballpark. cache_read is billed at ~10% of the input rate.
    est_cost = round((in_tok + cache_create) / 1e6 * 3 + cache_read / 1e6 * 0.3
                     + out_tok / 1e6 * 15, 2)
    return {
        "transcript": str(path),
        "turns": turns,
        "tool_calls": dict(tool_calls),
        "tool_calls_total": sum(tool_calls.values()),
        "input_tokens": in_tok,
        "output_tokens": out_tok,
        "cache_read_tokens": cache_read,
        "cache_creation_tokens": cache_create,
        "est_cost_usd": est_cost,
        "error_results": error_results,
        "error_breakdown": dict(err_breakdown),
        "error_samples": err_samples,
        "footgun_blocks": footgun_blocks,
        "engine_rollbacks": engine_rollbacks,
        "duplicate_cmds": dup,
        "duplicate_cmd_count": sum(dup.values()),
        "eng_subcmds": dict(eng_subcmds),
        "first_ts": first_ts,
        "last_ts": last_ts,
    }


def _concern(d: dict) -> bool:
    return bool(d.get("error_results") or d.get("footgun_blocks")
               or d.get("duplicate_cmd_count"))


def print_human(d: dict, header: str = "", show_errors: bool = False) -> None:
    if header:
        print(f"=== {header} ===")
    print(f"  turns={d['turns']}  tool_calls={d['tool_calls_total']} {d['tool_calls']}")
    print(f"  tokens: in={d['input_tokens']:,} out={d['output_tokens']:,} "
          f"cache_read={d['cache_read_tokens']:,} cache_new={d['cache_creation_tokens']:,}"
          f"   est_cost~${d['est_cost_usd']}")
    print(f"  engine loop: {d['eng_subcmds']}")
    flag = "OK" if not _concern(d) else "REVIEW"
    print(f"  [{flag}] tooling_errors={d['error_results']}  footgun_blocks={d['footgun_blocks']}  "
          f"duplicate_cmds={d['duplicate_cmd_count']}  (engine_rollbacks={d['engine_rollbacks']}, informational)")
    if d["error_breakdown"]:
        print(f"  tooling-error breakdown: {d['error_breakdown']}")
    if show_errors and d.get("error_samples"):
        print("  error samples (--errors):")
        for s in d["error_samples"]:
            print(f"    | {s}")
    if d["duplicate_cmds"]:
        print("  repeated commands (retries/thrash):")
        for c, n in list(d["duplicate_cmds"].items())[:5]:
            print(f"    {n}x  {c[:90]}")


def load_runs() -> list[dict]:
    if not RUNLOG.exists():
        return []
    out = []
    for line in RUNLOG.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if line:
            try:
                out.append(json.loads(line))
            except Exception:
                pass
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    g = ap.add_mutually_exclusive_group()
    g.add_argument("--all", action="store_true", help="one line per headless run in metrics/headless_runs.jsonl")
    g.add_argument("--latest", action="store_true", help="newest transcript in the project dir")
    g.add_argument("--session", help="session id (uuid)")
    g.add_argument("--transcript", help="explicit transcript path")
    ap.add_argument("--projects-dir", help="override Claude projects dir")
    ap.add_argument("--json", action="store_true", help="machine output")
    ap.add_argument("--flags", action="store_true", help="only show runs with concerning signals")
    ap.add_argument("--errors", action="store_true", help="(single session) print short snippets of erroring tool results")
    a = ap.parse_args()

    pdir = find_projects_dir(a.projects_dir)

    def resolve(sid: str) -> Path | None:
        if pdir and (pdir / f"{sid}.jsonl").exists():
            return pdir / f"{sid}.jsonl"
        return None

    if a.all:
        runs = load_runs()
        if not runs:
            print("no headless runs recorded yet (metrics/headless_runs.jsonl is empty/absent).")
            return 0
        rows = []
        for r in runs:
            sid = r.get("session_id", "")
            tp = resolve(sid)
            d = digest_transcript(tp) if tp else {}
            row = {**r, **{k: d.get(k) for k in
                   ("turns", "footgun_blocks", "error_results", "duplicate_cmd_count",
                    "output_tokens", "est_cost_usd")}}
            rows.append(row)
        if a.flags:
            rows = [r for r in rows if (r.get("error_results") or r.get("footgun_blocks") or r.get("duplicate_cmd_count"))]
        if a.json:
            print(json.dumps(rows, indent=2, default=str))
            return 0
        print(f"{'func':<34} {'turns':>5} {'out_tok':>8} {'cost$':>7} {'errs':>4} {'block':>5} {'dup':>4} {'oracle':>6} {'adv':>3}")
        for r in rows:
            cost = r.get("cost_usd") if r.get("cost_usd") is not None else r.get("est_cost_usd")
            try:
                cost = round(float(cost), 2)
            except (TypeError, ValueError):
                cost = "?"
            print(f"{str(r.get('func'))[:34]:<34} {str(r.get('turns','?')):>5} "
                  f"{str(r.get('output_tokens','?')):>8} {str(cost):>7} "
                  f"{str(r.get('error_results','?')):>4} {str(r.get('footgun_blocks','?')):>5} "
                  f"{str(r.get('duplicate_cmd_count','?')):>4} "
                  f"{('ok' if r.get('oracle_ok') else 'BAD'):>6} {('yes' if r.get('advanced') else 'no'):>3}")
        return 0

    tp = None
    if a.transcript:
        tp = Path(a.transcript)
    elif a.session:
        tp = resolve(a.session)
    elif a.latest:
        if not pdir:
            print("could not locate the Claude projects dir; pass --projects-dir.")
            return 1
        js = sorted(pdir.glob("*.jsonl"), key=lambda p: p.stat().st_mtime, reverse=True)
        tp = js[0] if js else None
    else:
        ap.print_help()
        return 0

    if not tp or not tp.exists():
        print(f"transcript not found ({tp}).")
        return 1
    d = digest_transcript(tp)
    if a.json:
        print(json.dumps(d, indent=2, default=str))
    else:
        print_human(d, header=tp.stem, show_errors=a.errors)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

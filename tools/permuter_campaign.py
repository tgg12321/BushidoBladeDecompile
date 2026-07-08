#!/usr/bin/env python3
"""permuter_campaign — launch/harvest decomp-permuter campaigns WITH telemetry.

Owner directive 2026-07-07: permuter campaigns get real metrics (time-to-find
distribution) and a fresh-seed stopping discipline. This wrapper is the
STANDARD way to run campaigns — raw `permuter.py <dir>` invocations leave no
telemetry and are what made "does long sampling ever pay?" unanswerable.

Events land in metrics/events.jsonl via engine.metrics.record_event (same
silent, best-effort contract as every engine command):

  permuter-launch   {func, dir, label, jobs, base_score, pid}
  permuter-harvest  {func, dir, label, elapsed_s, iterations, finds[], best_new_score,
                     stopped, pid_alive}  — one `finds[]` entry per output-<score>-<ctr>
                     dir, each with seconds_since_launch (mtime - launch ts)

Usage (WSL, repo root, venv active):
  python3 tools/permuter_campaign.py launch  --func <f> --dir tmp/perm_x [--label chassis] [-j 8] [--stop-on-zero]
  python3 tools/permuter_campaign.py harvest --dir tmp/perm_x [--stop] [--reason "..."]
  python3 tools/permuter_campaign.py status

The workspace <dir> must already contain base.c / compile.sh / target.o /
settings.toml (permuter import.py or a hand-built workspace, e.g.
tools/mar_perm_workspace.sh). `launch` snapshots pre-existing output-* dirs so
harvest can distinguish NEW finds from prior ones.

Campaign discipline (see .claude/rules/permuter-directives.md §Campaign
discipline): basins yield early or not at all — harvest + --stop at the
no-novel-find window; never leave campaigns simmering across sessions.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import signal
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from engine.metrics import record_event  # noqa: E402  (best-effort, never raises)

REGISTRY = ROOT / "tmp" / "permuter_campaigns.json"
META_NAME = "campaign_meta.json"
LOG_NAME = "campaign.log"
OUTPUT_RE = re.compile(r"^output-(\d+)-(\d+)$")
ITER_RE = re.compile(r"iteration (\d+)")
BASE_RE = re.compile(r"base score = (\d+)")


def _now_iso():
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


def _load_registry():
    try:
        return json.loads(REGISTRY.read_text(encoding="utf-8"))
    except Exception:
        return {}


def _save_registry(reg):
    REGISTRY.parent.mkdir(parents=True, exist_ok=True)
    REGISTRY.write_text(json.dumps(reg, indent=2) + "\n", encoding="utf-8")


def _pid_alive(pid):
    if not pid:
        return False
    try:
        os.kill(pid, 0)
        return True
    except (OSError, ProcessLookupError):
        return False


def _find_campaign_pids(d: Path):
    """PIDs of permuter.py processes whose workspace arg resolves to d.
    Linux/WSL /proc scan — covers ADOPTED campaigns launched without the
    wrapper (no meta pid to kill)."""
    pids = []
    proc = Path("/proc")
    if not proc.exists():
        return pids
    for p in proc.iterdir():
        if not p.name.isdigit():
            continue
        try:
            cmd = (p / "cmdline").read_bytes().decode("utf-8", "replace").split("\0")
            if not any("permuter.py" in c for c in cmd):
                continue
            cwd = os.readlink(p / "cwd")
            for a in cmd[1:]:
                if not a or a.startswith("-"):
                    continue
                cand = Path(a) if os.path.isabs(a) else Path(cwd) / a
                try:
                    if cand.resolve() == d:
                        pids.append(int(p.name))
                        break
                except OSError:
                    pass
        except (OSError, PermissionError):
            continue
    return pids


def _scan_outputs(d: Path):
    """[(score, ctr, mtime_epoch)] for every output-<score>-<ctr> dir/file."""
    out = []
    try:
        for e in d.iterdir():
            m = OUTPUT_RE.match(e.name)
            if m:
                out.append((int(m.group(1)), int(m.group(2)), e.stat().st_mtime))
    except FileNotFoundError:
        pass
    return sorted(out, key=lambda t: t[2])


def _parse_log(logp: Path):
    """(last_iteration, base_score) from a campaign.log (progress uses \\r)."""
    iters, base = None, None
    try:
        text = logp.read_text(encoding="utf-8", errors="replace").replace("\r", "\n")
    except FileNotFoundError:
        return None, None
    for m in ITER_RE.finditer(text):
        iters = int(m.group(1))
    m = BASE_RE.search(text)
    if m:
        base = int(m.group(1))
    return iters, base


def cmd_launch(args):
    d = (ROOT / args.dir).resolve() if not os.path.isabs(args.dir) else Path(args.dir)
    for req in ("base.c", "compile.sh", "target.o"):
        if not (d / req).is_file():
            sys.exit(f"ERROR: {d} missing {req} — not a permuter workspace")
    meta_path = d / META_NAME
    if meta_path.exists():
        old = json.loads(meta_path.read_text(encoding="utf-8"))
        if _pid_alive(old.get("pid")):
            sys.exit(f"ERROR: campaign already RUNNING in {d} (pid {old['pid']}) — harvest --stop it first")

    cmd = [sys.executable, str(ROOT / "tools" / "decomp-permuter" / "permuter.py"),
           str(d), "-j", str(args.jobs)]
    if args.stop_on_zero:
        cmd.append("--stop-on-zero")
    logf = open(d / LOG_NAME, "a", encoding="utf-8")
    logf.write(f"\n=== campaign launch {_now_iso()} ===\n")
    logf.flush()
    proc = subprocess.Popen(cmd, stdout=logf, stderr=subprocess.STDOUT,
                            cwd=str(ROOT), start_new_session=True)
    launch_epoch = time.time()

    # wait for the base-score print (slow: compile.sh runs first); harvest
    # backfills base_score from the log if we give up waiting here
    base_score = None
    for _ in range(60):
        time.sleep(0.5)
        _, base_score = _parse_log(d / LOG_NAME)
        if base_score is not None:
            break
        if proc.poll() is not None:
            sys.exit(f"ERROR: permuter exited immediately (rc={proc.returncode}) — see {d / LOG_NAME}")

    meta = {
        "func": args.func,
        "label": args.label or d.name,
        "dir": str(d),
        "pid": proc.pid,
        "launch_ts": _now_iso(),
        "launch_epoch": launch_epoch,
        "jobs": args.jobs,
        "base_score": base_score,
        "preexisting_outputs": [f"output-{s}-{c}" for s, c, _ in _scan_outputs(d)],
    }
    meta_path.write_text(json.dumps(meta, indent=2) + "\n", encoding="utf-8")
    reg = _load_registry()
    reg[str(d)] = {"func": args.func, "label": meta["label"], "pid": proc.pid,
                   "launch_ts": meta["launch_ts"], "active": True}
    _save_registry(reg)

    result = {"ok": True, "score": base_score}
    record_event("permuter-launch", args.func, result,
                 extra={"dir": str(d), "label": meta["label"], "jobs": args.jobs,
                        "base_score": base_score, "pid": proc.pid,
                        "stop_on_zero": bool(args.stop_on_zero)})
    print(json.dumps({"launched": True, "pid": proc.pid, "base_score": base_score,
                      "dir": str(d), "label": meta["label"]}, indent=2))


def cmd_harvest(args):
    d = (ROOT / args.dir).resolve() if not os.path.isabs(args.dir) else Path(args.dir)
    meta_path = d / META_NAME
    meta = {}
    if meta_path.exists():
        meta = json.loads(meta_path.read_text(encoding="utf-8"))
    func = args.func or meta.get("func")
    if not func:
        sys.exit("ERROR: no campaign_meta.json in dir and no --func given")
    launch_epoch = meta.get("launch_epoch")
    pid = meta.get("pid")
    pre = set(meta.get("preexisting_outputs", []))

    iters, base_from_log = _parse_log(d / LOG_NAME)
    base_score = meta.get("base_score") or base_from_log
    outputs = _scan_outputs(d)
    finds = []
    for score, ctr, mtime in outputs:
        name = f"output-{score}-{ctr}"
        finds.append({
            "name": name, "score": score,
            "seconds_since_launch": round(mtime - launch_epoch, 1) if launch_epoch else None,
            "new": name not in pre,
        })
    new_finds = [f for f in finds if f["new"]]
    best_new = min((f["score"] for f in new_finds), default=None)
    elapsed = round(time.time() - launch_epoch, 1) if launch_epoch else None
    alive = _pid_alive(pid)

    stopped = False
    killed = 0
    if args.stop:
        targets = set(_find_campaign_pids(d))
        if alive:
            targets.add(pid)
        for t in targets:
            try:
                os.killpg(os.getpgid(t), signal.SIGTERM)
                killed += 1
            except Exception:
                try:
                    os.kill(t, signal.SIGTERM)
                    killed += 1
                except Exception:
                    pass
        if targets:
            time.sleep(1.0)
        stopped = killed > 0 and not _find_campaign_pids(d) and not _pid_alive(pid)

    summary = {
        "func": func, "dir": str(d), "label": meta.get("label", d.name),
        "base_score": base_score, "elapsed_s": elapsed, "iterations": iters,
        "finds_total": len(finds), "finds_new": len(new_finds),
        "best_new_score": best_new, "pid_alive_at_harvest": alive,
        "stopped": stopped, "procs_killed": killed, "stop_reason": args.reason,
        "finds": finds,
    }
    result = {"ok": True, "score": best_new}
    record_event("permuter-harvest", func, result, extra=summary)

    if args.stop:
        reg = _load_registry()
        if str(d) in reg:
            reg[str(d)]["active"] = False
            _save_registry(reg)
    print(json.dumps(summary, indent=2))


def cmd_status(args):
    reg = _load_registry()
    rows = []
    for dpath, info in reg.items():
        alive = _pid_alive(info.get("pid"))
        iters, _ = _parse_log(Path(dpath) / LOG_NAME)
        rows.append({"dir": dpath, "func": info.get("func"), "label": info.get("label"),
                     "launch_ts": info.get("launch_ts"), "pid": info.get("pid"),
                     "alive": alive, "registered_active": info.get("active"),
                     "iterations": iters})
    print(json.dumps(rows, indent=2))
    stale = [r for r in rows if r["registered_active"] and not r["alive"]]
    live = [r for r in rows if r["alive"]]
    print(f"\n{len(live)} live campaign(s), {len(stale)} stale registry entr(ies)",
          file=sys.stderr)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    lp = sub.add_parser("launch", help="launch a campaign detached, with telemetry")
    lp.add_argument("--func", required=True)
    lp.add_argument("--dir", required=True, help="permuter workspace dir")
    lp.add_argument("--label", help="chassis/basin label (default: dir name)")
    lp.add_argument("-j", "--jobs", type=int, default=8)
    lp.add_argument("--stop-on-zero", action="store_true")
    lp.set_defaults(fn=cmd_launch)

    hp = sub.add_parser("harvest", help="log finds/iters for a campaign; optionally stop it")
    hp.add_argument("--dir", required=True)
    hp.add_argument("--func", help="override when no campaign_meta.json exists (adopted campaign)")
    hp.add_argument("--stop", action="store_true", help="terminate the campaign after harvesting")
    hp.add_argument("--reason", help="why stopped (e.g. 'no-novel-find window elapsed')")
    hp.set_defaults(fn=cmd_harvest)

    st = sub.add_parser("status", help="list registered campaigns")
    st.set_defaults(fn=cmd_status)

    args = ap.parse_args()
    args.fn(args)


if __name__ == "__main__":
    main()

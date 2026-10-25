#!/usr/bin/env python3
"""Tests for the headless instance pool (tools/trace/pool.py).

  python tools/trace/test_pool.py unit
      registry logic in a private registry, no emulator: slots are
      exclusive and get distinct ports, the cap is enforced, a dead owner's
      slot is reaped, a live owner's is not, release removes only its own
      lock.

  python tools/trace/test_pool.py parallel --emulator <pcsx-redux.exe> \\
      --bios <ps-30a.bin> --cue <disc.cue> [--spec S] [--frames N] [--n 3]
      the same scripted spec (cut at --frames) run N times at once with
      `gdbtrace.py run --headless`; passes when every run is q124_complete,
      the runs held distinct ports, and their captures are identical: the
      trace events (wall-clock fields dropped) and every screenshot's SHA1.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
sys.path.insert(0, REPO)
from pathlib import Path
from tools.trace import store


def check(cond, msg):
    print(("ok   " if cond else "FAIL ") + msg)
    return bool(cond)


# ------------------------------------------------------------------ unit

def unit() -> int:
    reg = os.path.join(REPO, "tmp", f"pool-unit-{os.getpid()}")
    os.makedirs(reg)
    os.environ["BB2_TRACE_INSTANCES"] = reg
    os.environ["BB2_TRACE_MAX"] = "2"
    os.environ["BB2_TRACE_PORT_BASE"] = "47600"  # clear of real instances
    import pool
    quiet = lambda *a, **k: None  # noqa: E731
    ok = True
    try:
        a = pool.acquire("unit a", 0, quiet)
        b = pool.acquire("unit b", 0, quiet)
        ok &= check(a.n != b.n and a.port != b.port and
                    a.profile != b.profile, "two slots: distinct slot, "
                    "port, profile")
        try:
            pool.acquire("unit c", 0, quiet)
            ok &= check(False, "third acquire refused at cap 2")
        except SystemExit:
            ok &= check(True, "third acquire refused at cap 2")
        ok &= check(not pool.reap(reg, quiet), "live owners not reaped")

        # A dead owner (and a dead emulator pid): reaped.
        dead = subprocess.Popen([sys.executable, "-c", "pass"])
        dead.wait()
        b.record(pid=dead.pid, emulator_pids=[dead.pid])
        ok &= check(pool.reap(reg, quiet) == [b.n], "dead owner's slot "
                    "reaped")
        c = pool.acquire("unit c", 0, quiet)
        ok &= check(c.n == b.n, "reaped slot taken again")

        # release() of a lock someone else holds now leaves it alone.
        b.release()
        ok &= check(os.path.exists(c.lock), "stale handle's release keeps "
                    "the new holder's lock")
        a.release()
        c.release()
        ok &= check(not pool.held(reg), "all released")
    finally:
        for n in range(pool.MAX_SLOTS):
            for p in (pool.lock_path(reg, n),
                      os.path.join(reg, f"slot-{n}", "portable"),
                      os.path.join(reg, f"slot-{n}")):
                if os.path.isfile(p):
                    os.remove(p)
                elif os.path.isdir(p):
                    os.rmdir(p)
        os.rmdir(reg)
    print("unit: " + ("PASS" if ok else "FAIL"))
    return 0 if ok else 1


# -------------------------------------------------------------- parallel

def cut_spec(spec: dict, frames: int) -> dict:
    s = json.loads(json.dumps(spec))
    sc = s["script"]
    sc["end_frame"] = frames
    sc["inputs"] = [i for i in sc.get("inputs", [])
                    if i["frame"] + i.get("hold", 4) <= frames]
    sc["phases"] = [p for p in sc.get("phases", []) if p["frame"] < frames]
    sc["screenshots"] = [x for x in sc.get("screenshots", [])
                         if x["frame"] <= frames]
    names = {p["name"] for p in sc["phases"]}
    s["stubs"] = [x for x in s.get("stubs", [])
                  if "phase" not in x or x["phase"] in names]
    return s


WALL_CLOCK = {"t", "time", "elapsed"}


def events(path: str, manifest=None) -> list:
    out = []
    path = Path(path)
    with store.resolve(path.parent, path.name, manifest).open(encoding="utf-8") as f:
        for line in f:
            e = json.loads(line)
            out.append({k: v for k, v in e.items() if k not in WALL_CLOCK})
    return out


def sha1(path: str) -> str:
    with open(path, "rb") as f:
        return hashlib.sha1(f.read()).hexdigest()


def parallel(args) -> int:
    with open(args.spec, encoding="utf-8") as f:
        spec = cut_spec(json.load(f), args.frames)
    os.makedirs(args.out, exist_ok=True)
    spec_path = os.path.join(args.out, "spec.json")
    with open(spec_path, "w", encoding="utf-8", newline="\n") as f:
        json.dump(spec, f, indent=2)
        f.write("\n")
    procs, caps = [], []
    for i in range(args.n):
        cap = os.path.join(args.out, f"run{i}", "capture.jsonl")
        os.makedirs(os.path.dirname(cap), exist_ok=True)
        log = open(os.path.join(args.out, f"run{i}.log"), "w",
                   encoding="utf-8", newline="\n")
        cmd = [sys.executable, os.path.join(HERE, "gdbtrace.py"), "run",
               "--spec", spec_path, "--out", cap, "--launch", "pcsx-redux",
               "--emulator", args.emulator, "--cue", args.cue,
               "--headless"]
        if args.bios:
            cmd += ["--bios", args.bios]
        procs.append((subprocess.Popen(cmd, stdout=log,
                                       stderr=subprocess.STDOUT), log))
        caps.append(cap)
    t0 = time.time()
    rcs = []
    for p, log in procs:
        rcs.append(p.wait())
        log.close()
    print(f"parallel: {args.n} runs of {args.frames} frames in "
          f"{time.time() - t0:.0f} s, exit codes {rcs}")
    ok = check(all(rc == 0 for rc in rcs), "every run exited 0")
    mans = []
    for cap in caps:
        try:
            with open(cap[:-6] + ".manifest.json", encoding="utf-8") as f:
                mans.append(json.load(f))
        except (OSError, ValueError):
            mans.append({})
    for i, m in enumerate(mans):
        ok &= check(m.get("q124_complete"), f"run{i} q124_complete "
                    f"{m.get('q124_missing', '')}")
        ok &= check((m.get("emulator") or {}).get("headless"),
                    f"run{i} manifest says headless")
    ports = [(m.get("instance") or {}).get("port") for m in mans]
    ok &= check(None not in ports and len(set(ports)) == len(ports),
                f"distinct instance ports {ports}")
    ref = None
    for i, cap in enumerate(caps):
        try:
            ev = events(cap, mans[i])
        except (OSError, ValueError) as e:
            ok &= check(False, f"run{i} capture readable ({e})")
            continue
        shots = {os.path.basename(o["path"])[len("capture."):-4]: o.get("sha1")
                 for o in (mans[i].get("outputs") or [])
                 if o["path"].endswith(".png")}
        if ref is None:
            ref = (ev, shots)
            ok &= check(len(shots) == len(spec["script"]["screenshots"]),
                        f"run0 took all {len(shots)} screenshots")
            continue
        ok &= check(ev == ref[0], f"run{i} trace events == run0 "
                    f"({len(ev)} vs {len(ref[0])})")
        ok &= check(shots == ref[1], f"run{i} screenshots == run0")
    print("parallel: " + ("PASS" if ok else "FAIL"))
    return 0 if ok else 1


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("unit")
    p = sub.add_parser("parallel")
    p.add_argument("--emulator", required=True)
    p.add_argument("--cue", required=True)
    p.add_argument("--bios")
    p.add_argument("--spec", default=os.path.join(
        REPO, "docs", "naming", "runtime", "menu-tour-hitmap-1", "spec.json"))
    p.add_argument("--frames", type=int, default=6500)
    p.add_argument("--n", type=int, default=3)
    p.add_argument("--out", default=os.path.join(REPO, "tmp", "pool-parallel"))
    args = ap.parse_args(argv)
    return unit() if args.cmd == "unit" else parallel(args)


if __name__ == "__main__":
    sys.exit(main())

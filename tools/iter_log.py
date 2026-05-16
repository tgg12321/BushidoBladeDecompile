#!/usr/bin/env python3
"""iter_log.py — per-function build trajectory log.

Each `dc.sh build-active <func>` call appends one JSON line to
`.bb2_iter_log/<func>.jsonl`. Other tools (and the build-active wrapper)
can then surface the trajectory and detect plateaus.

Why this exists: in the 25-round push to match calc_fc_frame_800203B4
(commit a502eb4), the agent bounced between diff counts of 35/37/39/35/40/35
across rounds 13-20 without realizing it had plateaued — every build-active
reported the current count in isolation, so the "I've been stuck at 35
for 5 rounds" signal was invisible. Plateau-on-the-same-knob is the
clearest signal that the technique-switch threshold from §5 has been
hit; don't keep iterating, escalate.

Plateau detection (simple, deliberately conservative):
    - 3+ consecutive entries at the same diff count → plateau on that count
    - 5+ entries within ±2 diffs of each other → "noisy plateau" (techniques
      churning but no net progress)
    - A new diff count strictly LESS than the previous best resets the
      plateau counter (real progress)
"""
import argparse
import json
import os
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LOG_DIR = ROOT / ".bb2_iter_log"


def log_path(func: str) -> Path:
    return LOG_DIR / f"{func}.jsonl"


def record(func: str, diffs: int | None, bytes_off: int | None,
           match: bool, sha1_ok: bool, note: str = "",
           structural: int | None = None, rename: int | None = None,
           branch_offset: int | None = None) -> None:
    """Append one trajectory entry.

    `diffs` is the legacy verify-c byte-diff count. It cascades when
    mine.length != target.length, which makes it unreliable for REGRESSION
    detection (a legitimate delete rule can drop a line and inflate the
    byte-diff count tenfold).

    `structural`, `rename`, `branch_offset` come from binary_diff and are
    cascade-immune. When present, plateau_check() uses `structural` for
    REGRESSION detection instead of `diffs`.
    """
    LOG_DIR.mkdir(parents=True, exist_ok=True)
    entry = {
        "ts": int(time.time()),
        "diffs": diffs,
        "bytes_off": bytes_off,
        "match": match,
        "sha1_ok": sha1_ok,
    }
    if structural is not None:
        entry["structural"] = structural
    if rename is not None:
        entry["rename"] = rename
    if branch_offset is not None:
        entry["branch_offset"] = branch_offset
    if note:
        entry["note"] = note
    with log_path(func).open("a", encoding="utf-8") as f:
        f.write(json.dumps(entry) + "\n")


def read_log(func: str) -> list[dict]:
    p = log_path(func)
    if not p.is_file():
        return []
    out = []
    for line in p.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line:
            continue
        try:
            out.append(json.loads(line))
        except json.JSONDecodeError:
            continue
    return out


def _metric(entry: dict) -> int | None:
    """Pick the metric for plateau/regression checks.

    Prefer `structural` (cascade-immune from binary_diff) when present;
    fall back to `diffs` (verify-c byte count, cascades on length mismatch).

    The choice happens per-entry: if old entries have `diffs` only and new
    ones have `structural`, mixing them would compare apples to oranges, so
    we ALWAYS pick the same key across the run. The decision is: if the
    LATEST entry has `structural`, use it everywhere (treat older
    `structural`-less entries as having structural=None and ignore them
    for the regression baseline). Otherwise fall back to `diffs`."""
    if "structural" in entry and isinstance(entry["structural"], int):
        return entry["structural"]
    return entry.get("diffs") if isinstance(entry.get("diffs"), int) else None


def plateau_check(func: str) -> dict:
    """Return {"status": "ok"|"plateau"|"noisy"|"new"|"regression"|"match", ...}.

    Prefers `structural` over `diffs` when the latest entry has it — that
    metric is cascade-immune (a 1-instruction length change reads as 1
    structural diff, not 140 cascaded byte diffs).
    """
    entries = read_log(func)
    if not entries:
        return {"status": "new", "rounds": 0}

    # Decide which metric to use based on the LATEST entry.
    last_has_struct = (
        isinstance(entries[-1].get("structural"), int)
        if entries else False
    )

    # Filter to entries that have a usable numeric metric for the chosen key.
    if last_has_struct:
        numeric = [e for e in entries if isinstance(e.get("structural"), int)]
        key = "structural"
    else:
        numeric = [e for e in entries if isinstance(e.get("diffs"), int)]
        key = "diffs"
    if not numeric:
        return {"status": "new", "rounds": len(entries)}

    if numeric[-1].get("match"):
        return {"status": "match", "current": 0, "rounds": len(numeric),
                "metric": key}

    current = numeric[-1][key]
    best_ever = min(e[key] for e in numeric)

    # Strict plateau: last 3+ entries all == current
    tail = list(reversed(numeric))
    same_count = 1
    for e in tail[1:]:
        if e[key] == current:
            same_count += 1
        else:
            break

    # Noisy plateau: last 5+ entries within ±2 of current (no net progress)
    near_count = 1
    for e in tail[1:]:
        if abs(e[key] - current) <= 2:
            near_count += 1
        else:
            break

    # Just regressed (current > best in last 5)
    regression = current > min(e[key] for e in tail[:5])

    if same_count >= 3:
        return {
            "status": "plateau",
            "current": current,
            "rounds": same_count,
            "best": best_ever,
            "metric": key,
        }
    if near_count >= 5:
        return {
            "status": "noisy",
            "current": current,
            "rounds": near_count,
            "best": best_ever,
            "range_lo": min(e[key] for e in tail[:near_count]),
            "range_hi": max(e[key] for e in tail[:near_count]),
            "metric": key,
        }
    if regression and len(numeric) >= 2:
        prev = numeric[-2][key]
        if current > prev:
            return {
                "status": "regression",
                "current": current,
                "previous": prev,
                "best": best_ever,
                "metric": key,
            }
    return {"status": "ok", "current": current, "best": best_ever,
            "rounds": len(numeric), "metric": key}


def render_trajectory(func: str, last: int = 5) -> str:
    entries = read_log(func)
    if not entries:
        return ""
    recent = entries[-last:]
    lines = []
    for e in recent:
        ts = time.strftime("%H:%M:%S", time.localtime(e["ts"]))
        if e.get("match"):
            tag = "MATCH"
        elif e.get("diffs") is None and e.get("structural") is None:
            tag = "build-fail"
        else:
            # Prefer the cascade-immune structural breakdown when present.
            if isinstance(e.get("structural"), int):
                s = e["structural"]
                r = e.get("rename")
                b = e.get("branch_offset")
                parts = [f"S={s}"]
                if isinstance(r, int):
                    parts.append(f"R={r}")
                if isinstance(b, int):
                    parts.append(f"B={b}")
                tag = " ".join(parts) + f" ({e.get('diffs', '?')} bytes)"
            else:
                tag = f"{e['diffs']} diffs"
                if e.get("bytes_off"):
                    tag += f" / {e['bytes_off']:+d} bytes"
        note = f"  ({e['note']})" if e.get("note") else ""
        lines.append(f"  {ts}  {tag}{note}")
    return "\n".join(lines)


def render_plateau_advice(check: dict) -> str:
    """Human-facing advice line based on the plateau check result."""
    status = check.get("status")
    metric = check.get("metric", "diffs")
    label = "structural diffs" if metric == "structural" else "byte diffs"
    if status == "match":
        return "  → MATCH — commit when ready (`git commit ...`)."
    if status == "plateau":
        n = check["rounds"]
        d = check["current"]
        return (
            f"  → PLATEAU: {n} consecutive rounds at {d} {label}.\n"
            f"    Manual iteration on the current knob is exhausted. SWITCH technique:\n"
            f"      • `bash tools/dc.sh side-by-side <func>`     — categorized diff (cascade-immune)\n"
            f"      • `bash tools/dc.sh gen-substs <func> --apply` — auto-emit register-rename rules\n"
            f"      • `bash tools/dc.sh permute-adaptive <func>` — randomized C-structural search\n"
            f"      • `bash tools/dc.sh diff-summary <func>`    — categorical breakdown + routing hint\n"
            f"      • Re-read the m2c base.c (top of `dc.sh agent-brief <func>`) — original structure clue\n"
            f"      • Read feedback_quick_reference.md Part 9   — decision trees by symptom\n"
        )
    if status == "noisy":
        n = check["rounds"]
        lo = check["range_lo"]
        hi = check["range_hi"]
        best = check["best"]
        return (
            f"  → NOISY PLATEAU: {n} rounds bouncing in [{lo}..{hi}] {label} (best ever: {best}).\n"
            f"    You're churning the same knob with diminishing returns. SWITCH technique\n"
            f"    — same suggestions as PLATEAU above.\n"
        )
    if status == "regression":
        extra = ""
        if metric == "diffs":
            extra = (
                "\n    NOTE: the metric is byte-diff (cascades on length mismatch). "
                "If you just added a delete\n"
                "    rule, the regression flag may be spurious — verify with "
                "`dc.sh side-by-side <func>`.\n"
                "    Wire build_active to feed structural diffs to silence false alarms."
            )
        return (
            f"  → REGRESSION: {check['previous']} → {check['current']} {label}.\n"
            f"    The last change made it worse. Revert and try a different angle.{extra}\n"
        )
    if status == "ok":
        return ""
    return ""


def cmd_record(args):
    record(
        args.func,
        args.diffs if args.diffs >= 0 else None,
        args.bytes_off if args.bytes_off is not None else None,
        args.match,
        args.sha1_ok,
        note=args.note or "",
        structural=args.structural if args.structural >= 0 else None,
        rename=args.rename if args.rename >= 0 else None,
        branch_offset=args.branch_offset if args.branch_offset >= 0 else None,
    )


def cmd_show(args):
    traj = render_trajectory(args.func, args.last)
    print(f"Iteration log for {args.func} (last {args.last}):")
    if traj:
        print(traj)
    else:
        print("  (no entries yet)")
    print()
    check = plateau_check(args.func)
    advice = render_plateau_advice(check)
    if advice:
        print(advice.rstrip())


def cmd_check(args):
    check = plateau_check(args.func)
    print(json.dumps(check, indent=2))


def cmd_clear(args):
    p = log_path(args.func)
    if p.is_file():
        p.unlink()
        print(f"Cleared {p}")
    else:
        print(f"No log for {args.func}")


def main():
    p = argparse.ArgumentParser(prog="iter_log")
    sub = p.add_subparsers(dest="cmd", required=True)

    rec = sub.add_parser("record", help="Append one trajectory entry")
    rec.add_argument("func")
    rec.add_argument("--diffs", type=int, default=-1,
                     help="legacy byte-diff count (verify-c)")
    rec.add_argument("--bytes-off", type=int, default=None)
    rec.add_argument("--match", action="store_true")
    rec.add_argument("--sha1-ok", action="store_true")
    rec.add_argument("--note", default="")
    rec.add_argument("--structural", type=int, default=-1,
                     help="cascade-immune structural diff (binary_diff)")
    rec.add_argument("--rename", type=int, default=-1,
                     help="register-rename diff count (binary_diff)")
    rec.add_argument("--branch-offset", type=int, default=-1,
                     help="branch-offset diff count (binary_diff)")
    rec.set_defaults(func_=cmd_record)

    show = sub.add_parser("show", help="Print trajectory + plateau advice")
    show.add_argument("func")
    show.add_argument("--last", type=int, default=5)
    show.set_defaults(func_=cmd_show)

    chk = sub.add_parser("check", help="Plateau check as JSON")
    chk.add_argument("func")
    chk.set_defaults(func_=cmd_check)

    clr = sub.add_parser("clear", help="Wipe log for one function (e.g., post-match)")
    clr.add_argument("func")
    clr.set_defaults(func_=cmd_clear)

    args = p.parse_args()
    args.func_(args)


if __name__ == "__main__":
    main()

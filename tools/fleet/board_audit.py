#!/usr/bin/env python3
"""Board integration for the historical cheat-audit campaign.

Turns the "BB2 Decomp" GitHub Project board's Done column into a re-audit tracker:
  - populate-done : un-archive every completed-function card and set it to Done, so
                    the full historical suite is visible as the to-be-audited backlog.
  - clean <func>  : the cheat-audit agent cleared the function -> stamp a "Last Audited"
                    timestamp on its card and ARCHIVE it (Done -> archived).
  - flag <func>   : the agent found a probable cheat -> stamp "REGRESSED ..." and KEEP
                    the card in Done (visible) so it stays surfaced for the owner.
  - setup         : ensure the "Last Audited" text field exists.

Reuses tools/board_sync.py (the stable gh client). Best-effort: any single card op
that fails is logged and skipped; it never raises so the fleet can call it inline
without risk of stalling a review. gh must be authenticated (it is, Windows-side).
"""
from __future__ import annotations

import json
import os
import subprocess
import sys
import time

_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.abspath(os.path.join(_HERE, "..", ".."))
sys.path.insert(0, os.path.join(_ROOT, "tools"))

import board_sync  # noqa: E402

PROJECT_TITLE = getattr(board_sync, "PROJECT_TITLE", "BB2 Decomp")
INDEX_PATH = os.path.join(_ROOT, "tmp", "board_index.json")
AUDIT_FIELD = "Last Audited"

_CACHE = {"pid": None, "fmap": None, "index": None, "login": None}


def _login() -> str:
    if _CACHE["login"] is None:
        r = subprocess.run(["gh", "api", "user", "--jq", ".login"],
                           capture_output=True, text=True)
        _CACHE["login"] = (r.stdout or "").strip() or "tgg12321"
    return _CACHE["login"]


def _project():
    """(project_id, field_map incl. AUDIT_FIELD). Resolved once."""
    if _CACHE["pid"] is None:
        pid = _retry(lambda: board_sync.find_project(PROJECT_TITLE, _login()))
        if not pid:
            raise SystemExit(f"board project '{PROJECT_TITLE}' not found for {_login()}")
        fmap = _retry(lambda: board_sync.ensure_fields(pid))
        if AUDIT_FIELD not in fmap:
            existing = _retry(lambda: board_sync._list_fields(pid))
            fmap[AUDIT_FIELD] = existing.get(AUDIT_FIELD) or _retry(lambda: board_sync._create_field(pid, AUDIT_FIELD, "TEXT", []))
        _CACHE["pid"], _CACHE["fmap"] = pid, fmap
    return _CACHE["pid"], _CACHE["fmap"]


def _index() -> dict:
    if _CACHE["index"] is None:
        with open(INDEX_PATH, encoding="utf-8") as fh:
            _CACHE["index"] = json.load(fh)
    return _CACHE["index"]


def _item_id(func: str):
    e = _index().get(func)
    return e.get("item_id") if isinstance(e, dict) else None


def _now() -> str:
    return time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())


def _retry(fn, tries=5, delay=4):
    """Run fn(), retrying on ANY failure incl. board_sync's sys.exit on a transient
    401 (it treats auth errors as fatal; here a blip must not abort an 897-card run).
    Raises the last error if all tries fail."""
    last = None
    for attempt in range(tries):
        try:
            return fn()
        except BaseException as e:  # noqa: BLE001 — incl. SystemExit from gh 401
            last = e
            time.sleep(delay * (attempt + 1))
    raise RuntimeError(f"retry exhausted: {str(last)[:160]}")


def _set_audited(pid, fmap, item_id, text):
    _retry(lambda: board_sync._set_field(pid, fmap, item_id, AUDIT_FIELD, text, delay=0.2))


# ─────────────────────────────── commands ────────────────────────────────────

def cmd_setup(_args) -> int:
    pid, fmap = _project()
    print(f"project={pid}  '{AUDIT_FIELD}' field id={fmap[AUDIT_FIELD]['id']}")
    return 0


def cmd_populate_done(args) -> int:
    """Un-archive + set Status=Done for every func listed in args.file (one per line)."""
    pid, fmap = _project()
    funcs = [ln.strip() for ln in open(args.file, encoding="utf-8")
             if ln.strip() and not ln.startswith("#")]
    ok = miss = err = 0
    for i, func in enumerate(funcs, 1):
        iid = _item_id(func)
        if not iid:
            miss += 1
            continue
        try:
            _retry(lambda: board_sync._mutate(board_sync._UNARCHIVE, variables={"p": pid, "i": iid}, delay=0.15))
            _retry(lambda: board_sync._set_field(pid, fmap, iid, "Status", "Done", delay=0.15))
            ok += 1
        except BaseException as e:  # noqa: BLE001 — best-effort
            err += 1
            print(f"  ! {func}: {str(e)[:120]}", file=sys.stderr)
        if i % 50 == 0:
            print(f"  ... {i}/{len(funcs)} (done={ok} missing={miss} err={err})")
    print(json.dumps({"ok": ok, "missing_card": miss, "errors": err, "total": len(funcs)}))
    return 0


def cmd_clean(args) -> int:
    """Clean re-audit: stamp Last Audited + archive (Done -> archived)."""
    try:
        pid, fmap = _project()
        iid = _item_id(args.func)
        if not iid:
            print(f"clean: no card for {args.func}", file=sys.stderr)
            return 0
        _set_audited(pid, fmap, iid, f"clean {args.ts or _now()}")
        _retry(lambda: board_sync._mutate(board_sync._ARCHIVE, variables={"p": pid, "i": iid}, delay=0.1))
        print(f"clean {args.func} -> stamped + archived")
    except BaseException as e:  # noqa: BLE001
        print(f"clean {args.func}: best-effort failure: {str(e)[:160]}", file=sys.stderr)
    return 0


def cmd_flag(args) -> int:
    """Regression: stamp REGRESSED note, ensure visible in Done, do NOT archive."""
    try:
        pid, fmap = _project()
        iid = _item_id(args.func)
        if not iid:
            print(f"flag: no card for {args.func}", file=sys.stderr)
            return 0
        # make sure it's visible: un-archive + Status=Done
        _retry(lambda: board_sync._mutate(board_sync._UNARCHIVE, variables={"p": pid, "i": iid}, delay=0.1))
        _retry(lambda: board_sync._set_field(pid, fmap, iid, "Status", "Done", delay=0.1))
        _set_audited(pid, fmap, iid, f"REGRESSED {args.ts or _now()}: {(args.reason or '')[:200]}")
        print(f"flag {args.func} -> REGRESSED, kept in Done")
    except BaseException as e:  # noqa: BLE001
        print(f"flag {args.func}: best-effort failure: {str(e)[:160]}", file=sys.stderr)
    return 0


def main() -> int:
    import argparse
    p = argparse.ArgumentParser(description="board cheat-audit tracker")
    sub = p.add_subparsers(dest="cmd", required=True)
    sub.add_parser("setup").set_defaults(handler=cmd_setup)
    pp = sub.add_parser("populate-done"); pp.add_argument("--file", required=True); pp.set_defaults(handler=cmd_populate_done)
    pc = sub.add_parser("clean"); pc.add_argument("func"); pc.add_argument("--ts", default=""); pc.set_defaults(handler=cmd_clean)
    pf = sub.add_parser("flag"); pf.add_argument("func"); pf.add_argument("--reason", default=""); pf.add_argument("--ts", default=""); pf.set_defaults(handler=cmd_flag)
    args = p.parse_args()
    return args.handler(args)


if __name__ == "__main__":
    raise SystemExit(main())

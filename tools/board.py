#!/usr/bin/env python3
"""tools/board.py — token-efficient, queue-style board CLI for DECOMP AGENTS.

The agent-facing front door to the "BB2 Decomp" GitHub Projects v2 board, built
for ZERO-token pull-from-top, single-card reads, and single-card moves — WITHOUT
ever pulling the full board list (forbidden on the hot path: 448 active + 1024
archived cards = expensive in tokens AND API).

Design (load-bearing):
  - The board is a QUEUE. Always pull the TOP item and work it; everything needs
    decompiling, so there is no prioritization logic.
  - "next" and "card" and "status" are 100% LOCAL reads (no gh / no API):
      * the ordered worklist is engine/queue.json (already distance-sorted),
      * each card's rich briefing is tmp/cards/<func>.md (already generated).
  - Card MOVES (claim / done / block / release) need the GitHub ProjectV2Item id
    per func. A LOCAL INDEX (tmp/board_index.json) caches
    {func: {"item_id", "draft_id", ...}} so a move is a single-card mutation,
    never a list read. The index is built ONCE (the only full read) and cached.
  - Claims live in the index file under each func's entry (write-through). Claim
    state in tmp/board_index.json is BEST-EFFORT and NON-ATOMIC even on a single
    host: two concurrent claims can race (last-writer-wins), so it is a
    coordination HINT, not a lock. `_do_claim` re-reads the index right before
    deciding to narrow the window, and `next --claim` surfaces a failed claim
    (held by someone else) rather than presenting the card as held. Cross-machine
    / truly-atomic claims are a future git-branch upgrade — out of scope here.
  - `done` is IDEMPOTENT: re-running it (re-setting Done / re-archiving / clearing
    an already-cleared claim) is harmless, so a partially-completed `done` is safe
    to repeat.

board_sync.py is the stable gh client and is imported, never modified. We reuse:
  gh_graphql, find_project, ensure_fields, _set_field, _mutate, _ARCHIVE,
  load_queue, PROJECT_TITLE, GhError.

Usage:
    python tools/board.py next [--claim] [--json]
    python tools/board.py card <func> [--refresh]
    python tools/board.py claim <func>
    python tools/board.py done <func>
    python tools/board.py block <func> --reason "..."
    python tools/board.py release <func>
    python tools/board.py index [--rebuild]
    python tools/board.py status

Pure stdlib + subprocess. Repo text read with encoding="utf-8". LF line endings.

NOTE for the orchestrator: board_sync's mirror maps engine-status -> column and
would REVERT an agent's In-Progress / In-Review move on its next run. board.py
just performs the moves; board_sync must be taught to SKIP In-Progress/In-Review
cards (out of scope for this tool).
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import board_sync  # noqa: E402

QUEUE = ROOT / "engine" / "queue.json"
CARDS_DIR = ROOT / "tmp" / "cards"
INDEX_PATH = ROOT / "tmp" / "board_index.json"
LEDGER_PATH = ROOT / "tmp" / "board_enrich_done_ledger.json"
BOARD_URL = "https://github.com/users/tgg12321/projects/2"
LOGIN = "tgg12321"

# Status columns this CLI moves cards into (must exist as options in the board's
# Status single-select — board_sync.ensure_fields guarantees them).
STATUS_IN_PROGRESS = "In-Progress"
STATUS_BLOCKED = "Blocked"
STATUS_DONE = "Done"
STATUS_BACKLOG = "Backlog"


# ---------------------------------------------------------------------------
# local reads — queue + index + card files (NO API)
# ---------------------------------------------------------------------------

def load_queue_items(path=None):
    """Ordered queue items (already distance-sorted easiest-first). LOCAL."""
    return board_sync.load_queue(path or QUEUE)


def load_index(path=None):
    """Load tmp/board_index.json -> {func: {item_id, draft_id, claimed?, reason?}}.
    Missing/corrupt -> {}. LOCAL."""
    p = Path(path or INDEX_PATH)
    if not p.exists():
        return {}
    try:
        data = json.loads(p.read_text(encoding="utf-8"))
    except (ValueError, OSError):
        return {}
    return data if isinstance(data, dict) else {}


def save_index(index, path=None):
    """Write-through the index (utf-8, LF, sorted keys)."""
    p = Path(path or INDEX_PATH)
    p.parent.mkdir(parents=True, exist_ok=True)
    with open(p, "w", encoding="utf-8", newline="\n") as fh:
        json.dump(index, fh, indent=2, sort_keys=True)
        fh.write("\n")


def _claimed_funcs(index):
    """Set of funcs currently claimed by anyone (per the local index)."""
    return {f for f, e in index.items() if isinstance(e, dict) and e.get("claimed")}


# ---------------------------------------------------------------------------
# in-process gh field-map cache (one ensure_fields per invocation)
# ---------------------------------------------------------------------------

_PROJECT_CACHE = {"project_id": None, "field_map": None}


def _project_and_fields():
    """Resolve the project id + field map ONCE per process (for move commands).
    A single ensure_fields call gives the Status option ids board_sync._set_field
    needs. Hot-path reads never call this."""
    if _PROJECT_CACHE["project_id"] is None:
        pid = board_sync.find_project(board_sync.PROJECT_TITLE, LOGIN)
        if pid is None:
            sys.exit(f"FATAL: board project '{board_sync.PROJECT_TITLE}' not found "
                     f"for login {LOGIN}.")
        _PROJECT_CACHE["project_id"] = pid
        _PROJECT_CACHE["field_map"] = board_sync.ensure_fields(pid)
    return _PROJECT_CACHE["project_id"], _PROJECT_CACHE["field_map"]


def _set_status(item_id, status):
    """Single-card Status mutation (no list read). Uses the cached field map."""
    project_id, field_map = _project_and_fields()
    board_sync._set_field(project_id, field_map, item_id, "Status", status, delay=0.0)


def _require_item_id(index, func):
    """item_id for func from the local index, or exit with a rebuild hint."""
    entry = index.get(func)
    if not isinstance(entry, dict) or not entry.get("item_id"):
        sys.exit(f"ERROR: no item_id for {func} in the local index.\n"
                 "  run `python tools/board.py index --rebuild`")
    return entry["item_id"]


# ---------------------------------------------------------------------------
# subcommand: next  (LOCAL, no API on the no-claim path)
# ---------------------------------------------------------------------------

def _top_available(items, index):
    """The first status=='active' queue item whose func is NOT claimed locally.
    Returns the item dict, or None."""
    claimed = _claimed_funcs(index)
    for it in items:
        if it.get("status") != "active":
            continue
        if it["func"] in claimed:
            continue
        return it
    return None


def cmd_next(args):
    items = load_queue_items()
    index = load_index()
    top = _top_available(items, index)
    if top is None:
        if args.json:
            print(json.dumps({"func": None, "reason": "no available active item"}))
        else:
            print("No available active item (queue empty or all claimed).")
        return 0

    func = top["func"]

    # claim_changed: did THIS agent acquire the card on this invocation?
    # claim_msg:     the human-readable claim result (race / held-by-other / ok).
    claim_changed = None
    claim_msg = None
    if args.claim:
        # Claim mutates the board (single mutation) + the local index. A failed
        # claim (someone else grabbed it in the race window) must be SURFACED —
        # the agent does NOT hold this card and must not treat it as claimed.
        claim_changed, claim_msg = _do_claim(func, index)
        index = load_index()  # refresh claim metadata for output

    entry = index.get(func) or {}
    claimed = entry.get("claimed") if isinstance(entry, dict) else None
    claimed_by = claimed.get("by") if isinstance(claimed, dict) else None

    if args.json:
        out = {
            "func": func,
            "file": top.get("file"),
            "verdict": top.get("verdict"),
            "distance": top.get("distance"),
            "rules": top.get("rules", 0),
            "status": top.get("status"),
            "claimed": claimed,
            "url": BOARD_URL,
            "hint": f"python tools/board.py card {func}",
        }
        if args.claim:
            # Whether WE now hold the card (true) or the claim failed because
            # someone else holds it (false, with claimed_by = the other owner).
            out["claimed"] = bool(claim_changed)
            out["claimed_by"] = claimed_by
            out["claim_message"] = claim_msg
        print(json.dumps(out))
        return 0

    print(f"next: {func}")
    print(f"  file:     src/{top.get('file')}.c")
    print(f"  verdict:  {top.get('verdict')}")
    print(f"  distance: {top.get('distance')}")
    print(f"  rules:    {top.get('rules', 0)}")
    print(f"  status:   {top.get('status')}")
    if args.claim:
        if claim_changed:
            print(f"  claim:    OK — YOU hold {func} (claimed by {claimed_by}).")
        else:
            print(f"  claim:    FAILED — you do NOT hold {func}. {claim_msg}")
    elif claimed:
        print(f"  claimed:  by {claimed.get('by')} @ {claimed.get('at')}")
    print(f"  url:      {BOARD_URL}")
    print(f"  -> run `python tools/board.py card {func}` for the full briefing")
    return 0


# ---------------------------------------------------------------------------
# subcommand: card  (LOCAL; regen ONE card on miss / --refresh)
# ---------------------------------------------------------------------------

def _regen_card(func):
    """Regenerate ONE card via board_cards.py <func> (it prints the body to
    stdout) and write it to tmp/cards/<func>.md. Returns the body text or None."""
    try:
        r = subprocess.run(
            [sys.executable, str(ROOT / "tools" / "board_cards.py"), func],
            cwd=str(ROOT), capture_output=True, text=True, encoding="utf-8",
            errors="replace")
    except OSError as e:
        print(f"ERROR: could not run board_cards.py: {e}", file=sys.stderr)
        return None
    if r.returncode != 0:
        sys.stderr.write(r.stderr)
        return None
    body = r.stdout
    CARDS_DIR.mkdir(parents=True, exist_ok=True)
    with open(CARDS_DIR / f"{func}.md", "w", encoding="utf-8", newline="\n") as fh:
        fh.write(body)
    return body


def cmd_card(args):
    func = args.func
    p = CARDS_DIR / f"{func}.md"
    if args.refresh or not p.exists():
        body = _regen_card(func)
        if body is None:
            print(f"ERROR: could not produce a card for {func}.", file=sys.stderr)
            return 1
        sys.stdout.write(body)
        return 0
    sys.stdout.write(p.read_text(encoding="utf-8"))
    return 0


# ---------------------------------------------------------------------------
# subcommand: claim  (single Status -> In-Progress mutation + write-through)
# ---------------------------------------------------------------------------

def _owner():
    return os.environ.get("CLAUDE_SESSION_ID") or "agent"


def _do_claim(func, index):
    """Claim `func`: idempotent. Returns (changed, message). Writes through the
    index and performs ONE Status mutation (no list read).

    Re-reads the on-disk index immediately before deciding so a claim that landed
    AFTER our caller selected `func` (the `next` race window) is still seen — the
    in-memory `index` may be stale. This NARROWS but cannot eliminate the race:
    the index is BEST-EFFORT, NON-ATOMIC even same-host (last-writer-wins); it's a
    coordination hint, not a lock. A `False` return means we do NOT hold the card."""
    me = _owner()
    # Pull the freshest claim state from disk (the passed `index` may predate a
    # concurrent claim). Merge it back so write-through preserves other funcs.
    disk = load_index()
    if isinstance(disk, dict):
        index.update(disk)
    entry = index.get(func)
    if not isinstance(entry, dict):
        entry = {}
    existing = entry.get("claimed")
    if existing:
        owner = existing.get("by")
        if owner == me:
            return False, f"{func} already claimed by you ({me}) — ok."
        return False, f"{func} already claimed by {owner} (since {existing.get('at')})."

    item_id = _require_item_id(index, func)
    entry["claimed"] = {"by": me, "at": time.time()}
    index[func] = entry
    save_index(index)            # write-through BEFORE the mutation
    _set_status(item_id, STATUS_IN_PROGRESS)
    return True, f"claimed {func} (by {me}) -> {STATUS_IN_PROGRESS}."


def cmd_claim(args):
    index = load_index()
    _changed, msg = _do_claim(args.func, index)
    print(msg)
    return 0


# ---------------------------------------------------------------------------
# subcommand: done  (Status -> Done + archive, clear claim, finalize)
# ---------------------------------------------------------------------------

def cmd_done(args):
    func = args.func
    index = load_index()
    item_id = _require_item_id(index, func)
    project_id, _field_map = _project_and_fields()
    _set_status(item_id, STATUS_DONE)
    board_sync._mutate(board_sync._ARCHIVE, variables={"p": project_id, "i": item_id}, delay=0.0)
    entry = index.get(func) or {}
    entry.pop("claimed", None)
    entry["finalized"] = True
    index[func] = entry
    save_index(index)
    print(f"done {func} -> {STATUS_DONE} + archived; claim cleared.")
    print("  reminder: the ENGINE completion is separate — run "
          f"`queue done {func}` under WSL (oracle SHA1 gate). board.py does NOT build.")
    return 0


# ---------------------------------------------------------------------------
# subcommand: block  (Status -> Blocked, clear claim, record reason)
# ---------------------------------------------------------------------------

def cmd_block(args):
    func = args.func
    index = load_index()
    item_id = _require_item_id(index, func)
    _set_status(item_id, STATUS_BLOCKED)
    entry = index.get(func) or {}
    entry.pop("claimed", None)
    entry["reason"] = args.reason
    index[func] = entry
    save_index(index)
    print(f"blocked {func} -> {STATUS_BLOCKED}; claim cleared. reason: {args.reason}")
    return 0


# ---------------------------------------------------------------------------
# subcommand: release  (clear claim, Status -> Backlog)
# ---------------------------------------------------------------------------

def cmd_release(args):
    func = args.func
    index = load_index()
    item_id = _require_item_id(index, func)
    _set_status(item_id, STATUS_BACKLOG)
    entry = index.get(func) or {}
    entry.pop("claimed", None)
    index[func] = entry
    save_index(index)
    print(f"released {func} -> {STATUS_BACKLOG}; claim cleared.")
    return 0


# ---------------------------------------------------------------------------
# subcommand: index  (the ONLY full read — merge ledger Done + ONE visible read)
# ---------------------------------------------------------------------------

# Visible (non-archived / active) items: id + draft id + title, one paginated read.
_VISIBLE_QUERY = (
    "query($id:ID!,$cursor:String){ node(id:$id){ ... on ProjectV2{ "
    "items(first:100,after:$cursor){ "
    "pageInfo{ hasNextPage endCursor } "
    "nodes{ id "
    "content{ __typename ... on DraftIssue{ id title } } } } } } }"
)


def _read_visible_items(project_id):
    """ONE paginated read of the VISIBLE (active) board items -> dict
    title -> {item_id, draft_id}. Only DraftIssue-backed, titled items."""
    out = {}
    cursor = None
    while True:
        data = board_sync.gh_graphql(_VISIBLE_QUERY, variables={"id": project_id, "cursor": cursor})
        page = data["node"]["items"]
        for n in page["nodes"]:
            content = n.get("content") or {}
            if content.get("__typename") != "DraftIssue":
                continue
            title = content.get("title")
            if not title:
                continue
            out[title] = {"item_id": n["id"], "draft_id": content.get("id")}
        if not page["pageInfo"]["hasNextPage"]:
            break
        cursor = page["pageInfo"]["endCursor"]
    return out


def load_ledger(path=None):
    """The board_enrich Done ledger {func: {item_id, draft_id, finalized,...}}."""
    p = Path(path or LEDGER_PATH)
    if not p.exists():
        return {}
    try:
        data = json.loads(p.read_text(encoding="utf-8"))
    except (ValueError, OSError):
        return {}
    return data if isinstance(data, dict) else {}


def build_index(project_id, ledger, visible, existing=None):
    """Merge the Done ids (ledger) + visible active ids into the index.
    Preserves existing `claimed` / `reason` fields per func. Returns
    (index, n_active, n_done). Pure (no API)."""
    existing = existing or {}
    index = {}

    def _carry(func, ids, finalized):
        prev = existing.get(func) if isinstance(existing.get(func), dict) else {}
        entry = {"item_id": ids.get("item_id"), "draft_id": ids.get("draft_id")}
        if finalized:
            entry["finalized"] = True
        # preserve claim / reason across rebuilds
        if prev.get("claimed"):
            entry["claimed"] = prev["claimed"]
        if prev.get("reason"):
            entry["reason"] = prev["reason"]
        index[func] = entry

    n_done = 0
    for func, ids in ledger.items():
        _carry(func, ids, finalized=True)
        n_done += 1
    n_active = 0
    for func, ids in visible.items():
        # visible (active) ids win for funcs in both (a func can leave Done);
        # but keep the Done-finalized flag off for active cards.
        _carry(func, ids, finalized=False)
        n_active += 1
    return index, n_active, n_done


def cmd_index(args):
    if not args.rebuild:
        index = load_index()
        if not index:
            print("index: not built yet — run `python tools/board.py index --rebuild`")
            return 0
        n_total = len(index)
        n_done = sum(1 for e in index.values() if isinstance(e, dict) and e.get("finalized"))
        n_active = n_total - n_done
        n_claimed = len(_claimed_funcs(index))
        print(f"index: {n_total} funcs ({n_active} active + {n_done} done), "
              f"{n_claimed} claimed  [{INDEX_PATH}]")
        return 0

    # --rebuild: the ONLY full read.
    project_id = board_sync.find_project(board_sync.PROJECT_TITLE, LOGIN)
    if project_id is None:
        sys.exit(f"FATAL: board project '{board_sync.PROJECT_TITLE}' not found for login {LOGIN}.")
    ledger = load_ledger()
    visible = _read_visible_items(project_id)
    existing = load_index()
    index, n_active, n_done = build_index(project_id, ledger, visible, existing=existing)
    save_index(index)
    print(f"indexed {len(index)} funcs ({n_active} active + {n_done} done) -> {INDEX_PATH}")
    return 0


# ---------------------------------------------------------------------------
# subcommand: status  (LOCAL — mirror `engine queue status`, instant)
# ---------------------------------------------------------------------------

def cmd_status(args):
    items = load_queue_items()
    total = len(items)
    by_status = {}
    by_verdict = {}
    for it in items:
        by_status[it.get("status")] = by_status.get(it.get("status"), 0) + 1
        by_verdict[it.get("verdict")] = by_verdict.get(it.get("verdict"), 0) + 1
    index = load_index()
    top = _top_available(items, index)

    if args.json:
        print(json.dumps({
            "total": total,
            "by_status": by_status,
            "by_verdict": by_verdict,
            "top": (top["func"] if top else None),
        }))
        return 0

    print(f"queue: {total} items")
    print("  by status:  " + ", ".join(f"{k}={v}" for k, v in sorted(by_status.items(),
                                                                      key=lambda kv: str(kv[0]))))
    print("  by verdict: " + ", ".join(f"{k}={v}" for k, v in sorted(by_verdict.items(),
                                                                      key=lambda kv: str(kv[0]))))
    if top:
        print(f"  top:        {top['func']}  (src/{top.get('file')}.c, "
              f"{top.get('verdict')}, dist {top.get('distance')}, "
              f"{top.get('rules', 0)} rule(s))")
    else:
        print("  top:        (none available)")
    return 0


# ---------------------------------------------------------------------------
# argparse / main
# ---------------------------------------------------------------------------

def build_parser():
    ap = argparse.ArgumentParser(
        prog="board.py",
        description="token-efficient queue-style board CLI for decomp agents "
                    "(pull-from-top, single-card read/move, no full lists)")
    sub = ap.add_subparsers(dest="cmd", required=True)

    p_next = sub.add_parser("next", help="print the TOP available function to work (LOCAL)")
    p_next.add_argument("--claim", action="store_true",
                        help="also claim it (-> In-Progress). Claim state is a "
                             "best-effort, non-atomic HINT (last-writer-wins); a "
                             "failed claim (held by another owner) is reported, "
                             "NOT presented as held.")
    p_next.add_argument("--json", action="store_true")
    p_next.set_defaults(handler=cmd_next)

    p_card = sub.add_parser("card", help="print one card's full briefing (LOCAL)")
    p_card.add_argument("func")
    p_card.add_argument("--refresh", action="store_true", help="regenerate this one card first")
    p_card.set_defaults(handler=cmd_card)

    p_claim = sub.add_parser("claim", help="claim a func + move card to In-Progress")
    p_claim.add_argument("func")
    p_claim.set_defaults(handler=cmd_claim)

    p_done = sub.add_parser("done", help="move card to Done + archive, clear claim "
                                         "(idempotent — safe to re-run)")
    p_done.add_argument("func")
    p_done.set_defaults(handler=cmd_done)

    p_block = sub.add_parser("block", help="move card to Blocked, clear claim, record reason")
    p_block.add_argument("func")
    p_block.add_argument("--reason", required=True)
    p_block.set_defaults(handler=cmd_block)

    p_release = sub.add_parser("release", help="clear claim + move card back to Backlog")
    p_release.add_argument("func")
    p_release.set_defaults(handler=cmd_release)

    p_index = sub.add_parser("index", help="build/refresh the local id index (rebuild = only full read)")
    p_index.add_argument("--rebuild", action="store_true")
    p_index.set_defaults(handler=cmd_index)

    p_status = sub.add_parser("status", help="queue counts + current top (LOCAL)")
    p_status.add_argument("--json", action="store_true")
    p_status.set_defaults(handler=cmd_status)

    return ap


def main(argv=None):
    # Force UTF-8 on stdout/stderr BEFORE any output. Card briefings contain
    # `·`, `—`, and (via errors="replace") `�`, which crash on Windows' default
    # cp1252 console codec. `card` is a core agent command and must not crash.
    for _stream in (sys.stdout, sys.stderr):
        try:
            _stream.reconfigure(encoding="utf-8", errors="replace")
        except (AttributeError, ValueError):
            pass  # older Python / non-reconfigurable stream; best-effort
    ap = build_parser()
    args = ap.parse_args(argv)
    try:
        return args.handler(args)
    except board_sync.GhError as e:
        sys.exit(f"board.py failed (GitHub API error): {e}")


if __name__ == "__main__":
    sys.exit(main())

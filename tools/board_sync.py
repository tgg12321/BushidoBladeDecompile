#!/usr/bin/env python3
"""tools/board_sync.py — one-way projection of the BB2 decomp engine state
into a GitHub Projects v2 board.

Reads engine/queue.json (+ memory/wip/ + inline_asm_canonical.txt, and the
completed-function inventory for --seed-done) and reconciles a GitHub Project
("BB2 Decomp") to match. The board is a MIRROR: this tool never writes back to
engine state, and never deletes board items (only add / update / archive).

Usage:
    python tools/board_sync.py --dry-run        # print the change set, mutate nothing
    python tools/board_sync.py                   # apply: create/update active cards
    python tools/board_sync.py --seed-done       # also backfill completed funcs as archived

Idempotent: re-running with no engine changes makes zero mutations.
"""
from __future__ import annotations

import argparse
import json
import re
import struct
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_QUEUE = ROOT / "engine" / "queue.json"
DEFAULT_WIP_DIR = ROOT / "memory" / "wip"
DEFAULT_MAP = ROOT / "build" / "bb2.map"
DEFAULT_ELF = ROOT / "build" / "bb2.elf"
PROJECT_TITLE = "BB2 Decomp"

# Engine status -> board column (the spec's locked 1:1 mapping).
ENGINE_STATUS_TO_COLUMN = {"active": "Backlog", "authorize": "Needs-Decision", "parked": "Blocked"}

STATUS_OPTIONS = ["Backlog", "Needs-Decision", "Blocked", "In-Progress", "In-Review", "Done"]
VERDICT_OPTIONS = ["C", "ASM-PARTIAL", "ASM-SUSPECT", "ASM-STRUCTURAL", "ASM-WHOLE", "JTBL-INFRA"]
WIP_OPTIONS = ["yes", "no"]
ACTIVE_COLUMNS = {"Backlog", "Needs-Decision", "Blocked"}

# Option colors (single-select options require a color).
OPTION_COLORS = {
    "Backlog": "GRAY", "Needs-Decision": "YELLOW", "Blocked": "RED",
    "In-Progress": "BLUE", "In-Review": "PURPLE", "Done": "GREEN",
    "yes": "BLUE", "no": "GRAY",
}

# (name, dataType, options)  — options is None for non-single-select fields.
FIELD_SPECS = [
    ("Status", "SINGLE_SELECT", STATUS_OPTIONS),
    ("Verdict", "SINGLE_SELECT", VERDICT_OPTIONS),
    ("WIP", "SINGLE_SELECT", WIP_OPTIONS),
    ("File", "TEXT", None),
    ("Distance", "NUMBER", None),
    ("Rules", "NUMBER", None),
]

# Non-functions that appear in build/bb2.map's .text symbol list (excluded from inventory).
_MAP_EXCLUDE = {"g_module_func_tbl", "g_module_type_tbl", "func_80037F08_ret", "cdrom_IrqHandler"}


# ---------------------------------------------------------------------------
# state collector (pure)
# ---------------------------------------------------------------------------

def load_queue(path):
    """Parse queue.json -> list of item dicts. Missing file -> [] (non-fatal)."""
    path = Path(path)
    if not path.exists():
        return []
    return json.loads(path.read_text(encoding="utf-8")).get("items", [])


def wip_exists(func, wip_dir):
    """True iff memory/wip/<func>/ exists (a checkpoint is present)."""
    return (Path(wip_dir) / func).is_dir()


def build_desired_from_queue(items, wip_dir):
    """Turn queue items into desired board cards.

    desired[func] = {"fields": {...}, "archived": bool}
    """
    desired = {}
    for it in items:
        col = ENGINE_STATUS_TO_COLUMN.get(it["status"], "Blocked")
        desired[it["func"]] = {
            "fields": {
                "Status": col,
                "Verdict": it["verdict"],
                "WIP": "yes" if wip_exists(it["func"], wip_dir) else "no",
                "File": it["file"],
                "Distance": it["distance"],
                "Rules": it["rules"],
            },
            "archived": False,
        }
    return desired


TEXT_LO, TEXT_HI = 0x80010000, 0x8008D080

# Object-range line in bb2.map: " .text  0x<vma>  0x<size>  build/src/<stem>.o"
_MAP_OBJ_RE = re.compile(r"^ \.text\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+build/src/(\S+)\.o")


def _load_text_object_ranges(map_path):
    """Parse bb2.map -> sorted list of (lo, hi, stem) for each non-empty .text
    input section from build/src/<stem>.o. Used to map a function address to its
    source file. Pure-Python, cross-platform."""
    ranges = []
    for line in Path(map_path).read_text(encoding="utf-8", errors="replace").splitlines():
        m = _MAP_OBJ_RE.match(line)
        if m:
            lo = int(m.group(1), 16)
            size = int(m.group(2), 16)
            if size > 0:
                ranges.append((lo, lo + size, m.group(3)))
    ranges.sort()
    return ranges


def _stem_for_addr(addr, ranges):
    for lo, hi, stem in ranges:
        if lo <= addr < hi:
            return stem
    return None


def _elf_func_symbols(elf_path):
    """Pure-Python read of STT_FUNC symbols from a 32-bit little-endian ELF.
    Returns {address: name}. Used only for --seed-done."""
    data = Path(elf_path).read_bytes()
    if data[:4] != b"\x7fELF":
        raise ValueError(f"not an ELF file: {elf_path}")
    if data[4] != 1 or data[5] != 1:
        raise ValueError(f"expected 32-bit little-endian ELF: {elf_path}")
    e_shoff = struct.unpack_from("<I", data, 0x20)[0]
    e_shentsize = struct.unpack_from("<H", data, 0x2E)[0]
    e_shnum = struct.unpack_from("<H", data, 0x30)[0]
    sections = []
    for i in range(e_shnum):
        off = e_shoff + i * e_shentsize
        sh_type = struct.unpack_from("<I", data, off + 0x04)[0]
        sh_offset, sh_size = struct.unpack_from("<II", data, off + 0x10)
        sh_link = struct.unpack_from("<I", data, off + 0x18)[0]
        sh_entsize = struct.unpack_from("<I", data, off + 0x24)[0]
        sections.append({"type": sh_type, "offset": sh_offset, "size": sh_size,
                         "link": sh_link, "entsize": sh_entsize})
    syms = {}
    SHT_SYMTAB, STT_FUNC = 2, 2
    for s in sections:
        if s["type"] != SHT_SYMTAB:
            continue
        strtab = sections[s["link"]]
        str_off, str_size = strtab["offset"], strtab["size"]
        ent = s["entsize"] or 16
        for j in range(s["size"] // ent):
            o = s["offset"] + j * ent
            st_name, st_value = struct.unpack_from("<II", data, o)
            st_info = data[o + 12]
            if (st_info & 0xF) != STT_FUNC:
                continue
            ns = str_off + st_name
            ne = data.index(b"\x00", ns, str_off + str_size)
            syms[st_value] = data[ns:ne].decode("ascii", "replace")
    return syms


def load_inventory(map_path, elf_path):
    """Authoritative completed-function inventory: every STT_FUNC symbol in the
    EXE text range (from build/bb2.elf), mapped to its src stem via bb2.map's
    .text object ranges. Returns {func: stem}. --seed-done only. Pure-Python."""
    syms = _elf_func_symbols(elf_path)
    ranges = _load_text_object_ranges(map_path)
    inv = {}
    for addr, name in syms.items():
        if not (TEXT_LO <= addr < TEXT_HI):
            continue
        if name in _MAP_EXCLUDE:
            continue
        stem = _stem_for_addr(addr, ranges)
        if stem:
            inv[name] = stem
    return inv


def build_desired_done(inventory, queue_funcs):
    """completed = inventory - queue_funcs -> archived Done cards."""
    desired = {}
    for func, stem in inventory.items():
        if func in queue_funcs:
            continue
        desired[func] = {"fields": {"Status": "Done", "File": stem}, "archived": True}
    return desired


# ---------------------------------------------------------------------------
# reconciler (pure) — diff desired vs current; emit minimal actions; never delete
# ---------------------------------------------------------------------------

def _val_eq(field, want, got):
    """Field-aware equality. Numbers compare numerically (API returns floats)."""
    if field in ("Distance", "Rules"):
        if want is None:
            return got is None
        if got is None:
            return False
        return float(want) == float(got)
    return want == got


def reconcile(desired, current):
    """Diff desired (dict func->{fields, archived}) against the current board
    snapshot (list of {item_id, title, is_archived, fields}).

    Returns a list of action dicts. Action ops: 'add', 'set', 'archive',
    'unarchive'. NEVER 'delete' — a board item with no desired entry is treated
    as a completed function (Done + archive), only when it is currently in an
    active column. Items already in In-Progress / In-Review (Phase 2 columns)
    are left untouched.
    """
    actions = []
    # Skip board items with no title (e.g. a manually-cleared draft, or a content
    # type outside our fragments): they have no func to match, and must not
    # collapse together in cur_by_title or be mis-archived as "completed".
    current = [c for c in current if c.get("title")]
    cur_by_title = {c["title"]: c for c in current}

    for func, want in desired.items():
        cur = cur_by_title.get(func)
        if cur is None:
            actions.append({"op": "add", "func": func,
                            "fields": want["fields"], "archived": want["archived"]})
            continue
        for fname, dval in want["fields"].items():
            if not _val_eq(fname, dval, cur["fields"].get(fname)):
                actions.append({"op": "set", "item_id": cur["item_id"],
                                "field": fname, "value": dval, "func": func})
        if want["archived"] and not cur["is_archived"]:
            actions.append({"op": "archive", "item_id": cur["item_id"], "func": func})
        elif not want["archived"] and cur["is_archived"]:
            actions.append({"op": "unarchive", "item_id": cur["item_id"], "func": func})

    # On the board but no longer in desired -> completed. Done + archive, but only
    # if it's currently in an active column (don't disturb Phase 2 work-in-flight).
    for func, cur in cur_by_title.items():
        if func in desired:
            continue
        if cur["fields"].get("Status") in ACTIVE_COLUMNS:
            if cur["fields"].get("Status") != "Done":
                actions.append({"op": "set", "item_id": cur["item_id"],
                                "field": "Status", "value": "Done", "func": func})
            if not cur["is_archived"]:
                actions.append({"op": "archive", "item_id": cur["item_id"], "func": func})
    return actions


# ---------------------------------------------------------------------------
# gh client — all network goes through gh_graphql() (the mockable chokepoint)
# ---------------------------------------------------------------------------

class GhError(Exception):
    pass


def gh_graphql(query, variables=None):
    """Run a GraphQL query/mutation via `gh api graphql` and return the 'data'
    object. Variables (strings, numbers, lists, objects) are sent as a JSON
    request body on stdin via `--input -` (gh's -f/-F flags do NOT parse JSON
    arrays into GraphQL lists). sys.exit on missing gh / auth failure (data is
    intact; rerun later); raises GhError on GraphQL or other gh errors."""
    body = json.dumps({"query": query, "variables": variables or {}})
    argv = ["gh", "api", "graphql", "-H", "X-Github-Next-Global-ID: 1", "--input", "-"]
    try:
        r = subprocess.run(argv, input=body, capture_output=True, text=True)
    except FileNotFoundError:
        sys.exit("FATAL: `gh` CLI not found on PATH. Install GitHub CLI and run `gh auth login`. "
                 "Engine state is untouched; rerun board_sync later.")
    if r.returncode != 0:
        err = r.stderr.strip()
        low = err.lower()
        if ("401" in err or "bad credentials" in low or "gh auth login" in low
                or "not logged in" in low or "authentication" in low):
            sys.exit("FATAL: gh is not authenticated (" + (err or "auth error") + ").\n"
                     "  Run `gh auth login` (needs the 'project' scope). "
                     "Engine state is untouched; rerun board_sync later.")
        raise GhError(err or f"gh exited {r.returncode}")
    out = json.loads(r.stdout)
    if out.get("errors"):
        raise GhError(json.dumps(out["errors"]))
    return out["data"]


def _viewer_id():
    return gh_graphql("query{ viewer { id } }")["viewer"]["id"]


def _list_fields(project_id):
    """Return {name: {"id", "type", "options"}} for existing project fields."""
    out = {}
    cursor = None
    while True:
        data = gh_graphql(
            "query($id:ID!,$cursor:String){ node(id:$id){ ... on ProjectV2{ "
            "fields(first:50,after:$cursor){ nodes{ "
            "__typename "
            "... on ProjectV2FieldCommon{ id name } "
            "... on ProjectV2SingleSelectField{ id name options{ id name } } } "
            "pageInfo{ hasNextPage endCursor } } } } }",
            variables={"id": project_id, "cursor": cursor},
        )
        f = data["node"]["fields"]
        for n in f["nodes"]:
            opts = {o["name"]: o["id"] for o in n.get("options", [])}
            out[n["name"]] = {"id": n["id"], "options": opts}
        if not f["pageInfo"]["hasNextPage"]:
            break
        cursor = f["pageInfo"]["endCursor"]
    return out


def _create_field(project_id, name, dtype, options):
    if dtype == "SINGLE_SELECT":
        data = gh_graphql(
            "mutation($p:ID!,$name:String!,$opts:[ProjectV2SingleSelectFieldOptionInput!]!){ "
            "createProjectV2Field(input:{projectId:$p,dataType:SINGLE_SELECT,name:$name,singleSelectOptions:$opts}){ "
            "projectV2Field{ ... on ProjectV2SingleSelectField{ id name options{ id name } } } } }",
            variables={"p": project_id, "name": name,
                       "opts": [{"name": o, "color": OPTION_COLORS.get(o, "GRAY"), "description": ""} for o in options]},
        )
        field = data["createProjectV2Field"]["projectV2Field"]
        return {"id": field["id"], "options": {o["name"]: o["id"] for o in field["options"]}}
    data = gh_graphql(
        "mutation($p:ID!,$name:String!,$dt:ProjectV2CustomFieldType!){ "
        "createProjectV2Field(input:{projectId:$p,dataType:$dt,name:$name}){ "
        "projectV2Field{ ... on ProjectV2FieldCommon{ id name } } } }",
        variables={"p": project_id, "name": name, "dt": dtype},
    )
    field = data["createProjectV2Field"]["projectV2Field"]
    return {"id": field["id"], "options": {}}


def ensure_fields(project_id):
    """Ensure every FIELD_SPEC field exists; return {name: {id, options}}."""
    existing = _list_fields(project_id)
    field_map = {}
    for name, dtype, options in FIELD_SPECS:
        if name in existing:
            field_map[name] = existing[name]
        else:
            field_map[name] = _create_field(project_id, name, dtype, options)
    return field_map


def ensure_project(title, login):
    """Return the node id of the user's project named `title`, creating it if absent."""
    cursor = None
    while True:
        data = gh_graphql(
            "query($login:String!,$cursor:String){ user(login:$login){ "
            "projectsV2(first:100,after:$cursor){ nodes{ id number title } "
            "pageInfo{ hasNextPage endCursor } } } }",
            variables={"login": login, "cursor": cursor},
        )
        pv = data["user"]["projectsV2"]
        for n in pv["nodes"]:
            if n["title"] == title:
                return n["id"]
        if not pv["pageInfo"]["hasNextPage"]:
            break
        cursor = pv["pageInfo"]["endCursor"]
    owner = _viewer_id()
    created = gh_graphql(
        "mutation($ownerId:ID!,$title:String!){ createProjectV2(input:{ownerId:$ownerId,title:$title}){ "
        "projectV2{ id number title } } }",
        variables={"ownerId": owner, "title": title},
    )
    return created["createProjectV2"]["projectV2"]["id"]


_ITEMS_QUERY = (
    "query($id:ID!,$cursor:String){ node(id:$id){ ... on ProjectV2{ "
    "items(first:100,after:$cursor){ "
    "pageInfo{ hasNextPage endCursor } "
    "nodes{ id isArchived "
    "content{ ... on DraftIssue{ title } ... on Issue{ title } ... on PullRequest{ title } } "
    "fieldValues(first:20){ nodes{ "
    "__typename "
    "... on ProjectV2ItemFieldTextValue{ text field{ ... on ProjectV2FieldCommon{ name } } } "
    "... on ProjectV2ItemFieldNumberValue{ number field{ ... on ProjectV2FieldCommon{ name } } } "
    "... on ProjectV2ItemFieldSingleSelectValue{ name optionId field{ ... on ProjectV2FieldCommon{ name } } } "
    "} } } } } } }"
)


def _parse_field_values(fv_nodes):
    fields = {}
    for n in fv_nodes:
        fld = n.get("field") or {}
        name = fld.get("name")
        if not name:
            continue
        t = n.get("__typename")
        if t == "ProjectV2ItemFieldTextValue":
            fields[name] = n.get("text")
        elif t == "ProjectV2ItemFieldNumberValue":
            fields[name] = n.get("number")
        elif t == "ProjectV2ItemFieldSingleSelectValue":
            fields[name] = n.get("name")
    return fields


def list_items(project_id):
    """Return the full board snapshot (including archived items):
    [{item_id, title, is_archived, fields}]."""
    items = []
    cursor = None
    while True:
        data = gh_graphql(_ITEMS_QUERY, variables={"id": project_id, "cursor": cursor})
        page = data["node"]["items"]
        for n in page["nodes"]:
            content = n.get("content") or {}
            items.append({
                "item_id": n["id"],
                "title": content.get("title"),
                "is_archived": n.get("isArchived", False),
                "fields": _parse_field_values(n.get("fieldValues", {}).get("nodes", [])),
            })
        if not page["pageInfo"]["hasNextPage"]:
            break
        cursor = page["pageInfo"]["endCursor"]
    return items


_ADD_DRAFT = ("mutation($p:ID!,$t:String!,$b:String!){ "
              "addProjectV2DraftIssue(input:{projectId:$p,title:$t,body:$b}){ projectItem{ id } } }")
_SET_TEXT = ("mutation($p:ID!,$i:ID!,$f:ID!,$v:String!){ "
             "updateProjectV2ItemFieldValue(input:{projectId:$p,itemId:$i,fieldId:$f,value:{text:$v}}){ projectV2Item{ id } } }")
_SET_NUM = ("mutation($p:ID!,$i:ID!,$f:ID!,$v:Float!){ "
            "updateProjectV2ItemFieldValue(input:{projectId:$p,itemId:$i,fieldId:$f,value:{number:$v}}){ projectV2Item{ id } } }")
_SET_SEL = ("mutation($p:ID!,$i:ID!,$f:ID!,$opt:String!){ "
            "updateProjectV2ItemFieldValue(input:{projectId:$p,itemId:$i,fieldId:$f,value:{singleSelectOptionId:$opt}}){ projectV2Item{ id } } }")
_ARCHIVE = "mutation($p:ID!,$i:ID!){ archiveProjectV2Item(input:{projectId:$p,itemId:$i}){ item{ id isArchived } } }"
_UNARCHIVE = "mutation($p:ID!,$i:ID!){ unarchiveProjectV2Item(input:{projectId:$p,itemId:$i}){ item{ id isArchived } } }"

_NUMBER_FIELDS = {"Distance", "Rules"}


def _mutate(query, variables=None, delay=0.2, max_tries=5):
    """gh_graphql with rate-limit backoff + inter-mutation pacing."""
    for attempt in range(max_tries):
        try:
            data = gh_graphql(query, variables=variables)
            if delay:
                time.sleep(delay)
            return data
        except GhError as e:
            msg = str(e).lower()
            if ("rate limit" in msg or "secondary" in msg
                    or "was submitted too quickly" in msg or "abuse" in msg):
                back = 2 ** attempt
                print(f"  rate-limited; backing off {back}s")
                time.sleep(back)
                continue
            raise
    raise GhError("rate-limit backoff exhausted")


def _set_field(project_id, field_map, item_id, field, value, delay):
    spec = field_map[field]
    if field in _NUMBER_FIELDS:
        _mutate(_SET_NUM, variables={"p": project_id, "i": item_id, "f": spec["id"], "v": value},
                delay=delay)
    elif spec["options"]:  # single-select
        opt_id = spec["options"][value]
        _mutate(_SET_SEL, variables={"p": project_id, "i": item_id, "f": spec["id"], "opt": opt_id},
                delay=delay)
    else:  # text
        _mutate(_SET_TEXT, variables={"p": project_id, "i": item_id, "f": spec["id"], "v": str(value)},
                delay=delay)


def apply(project_id, field_map, actions, delay=0.2):
    """Execute reconciler actions as gh mutations. Never deletes."""
    for a in actions:
        op = a["op"]
        if op == "add":
            body = f"file: {a['fields'].get('File', '')}"
            res = _mutate(_ADD_DRAFT, variables={"p": project_id, "t": a["func"], "b": body}, delay=delay)
            item_id = res["addProjectV2DraftIssue"]["projectItem"]["id"]
            for field, value in a["fields"].items():
                _set_field(project_id, field_map, item_id, field, value, delay)
            if a.get("archived"):
                _mutate(_ARCHIVE, variables={"p": project_id, "i": item_id}, delay=delay)
        elif op == "set":
            _set_field(project_id, field_map, a["item_id"], a["field"], a["value"], delay)
        elif op == "archive":
            _mutate(_ARCHIVE, variables={"p": project_id, "i": a["item_id"]}, delay=delay)
        elif op == "unarchive":
            _mutate(_UNARCHIVE, variables={"p": project_id, "i": a["item_id"]}, delay=delay)
        else:
            raise ValueError(f"unknown action op: {op!r}")


# ---------------------------------------------------------------------------
# driver / main
# ---------------------------------------------------------------------------

def run_sync(queue_path, wip_dir, project_title, login,
             dry_run=False, seed_done=False, map_path=None, elf_path=None):
    """Build desired state, reconcile against the live board, and apply (unless
    dry_run). Returns the number of planned actions. Engine state is never written."""
    items = load_queue(queue_path)
    desired = build_desired_from_queue(items, wip_dir)
    if seed_done:
        desired.update(build_desired_done(load_inventory(map_path, elf_path), {it["func"] for it in items}))
    print(f"board sync -> {project_title} ({'DRY RUN' if dry_run else 'apply'})")
    print(f"  desired cards: {len(desired)} (seed_done={seed_done})")

    project_id = ensure_project(project_title, login)
    field_map = ensure_fields(project_id)
    current = list_items(project_id)
    actions = reconcile(desired, current)
    print(f"  current board items: {len(current)}; planned actions: {len(actions)}")
    for a in actions:
        if a["op"] == "add":
            print(f"    + add {a['func']} ({a['fields'].get('Status')})")
        elif a["op"] == "set":
            print(f"    ~ set {a['func']}.{a['field']} = {a['value']}")
        elif a["op"] in ("archive", "unarchive"):
            print(f"    {'#' if a['op']=='archive' else '^'} {a['op']} {a['func']}")
    if not dry_run and actions:
        apply(project_id, field_map, actions)
    print("board sync complete." if not dry_run else "dry run complete (no mutations).")
    return len(actions)


def main():
    ap = argparse.ArgumentParser(description="project the BB2 decomp engine state onto a GitHub board")
    ap.add_argument("--queue", default=str(DEFAULT_QUEUE))
    ap.add_argument("--wip-dir", default=str(DEFAULT_WIP_DIR))
    ap.add_argument("--map", default=str(DEFAULT_MAP), help="build/bb2.map for --seed-done")
    ap.add_argument("--elf", default=str(DEFAULT_ELF), help="build/bb2.elf for --seed-done")
    ap.add_argument("--project", default=PROJECT_TITLE)
    ap.add_argument("--login", default="tgg12321")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--seed-done", action="store_true",
                    help="backfill completed funcs as archived Done items (one-time)")
    a = ap.parse_args()
    try:
        run_sync(Path(a.queue), Path(a.wip_dir), a.project, a.login,
                 dry_run=a.dry_run, seed_done=a.seed_done, map_path=Path(a.map),
                 elf_path=Path(a.elf))
    except GhError as e:
        sys.exit(f"board sync failed (GitHub API error): {e}")


if __name__ == "__main__":
    main()

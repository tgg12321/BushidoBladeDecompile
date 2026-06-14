#!/usr/bin/env python3
"""tools/board_cards.py — per-function card-body assembler for the BB2 decomp board.

The local, fast, no-network half of the board pipeline. For EVERY board function
(every item in engine/queue.json PLUS every completed function = ELF inventory −
queue) it assembles a rich markdown "one-stop-shop" card body and writes it to
tmp/cards/<func>.md. A sibling tool (board_enrich.py) pushes these to GitHub.

Each card body bundles, in one place:
  - the engine state (file, address, verdict, pure-C distance, rule count, WIP)
  - a one-line brief + next-step guidance keyed to the function's status
  - the identified regfix/asmfix cheats (collect_func_cheats)
  - the WIP checkpoint summary (floor vs head, reviewer, resume hint)
  - inferred techniques (validated lever-rule slugs)
  - commit history (one git-log pass, keyed by func name)
  - memory refs
  - a <details> block with the current C source (extract_c_body)
  - a <details> block with the target disassembly (objdump, truncated to fit)

Card bodies are budgeted to <= 65000 chars (GitHub's hard limit is 65536); the
asm details block is truncated first, then the C block, never the metadata.

Usage:
    python tools/board_cards.py            # assemble ALL board funcs -> tmp/cards/
    python tools/board_cards.py <func>     # assemble ONE func, print body to stdout

Pure stdlib + subprocess. All repo text read with encoding="utf-8",
errors="replace" (Windows cp1252 crashes otherwise). LF line endings.
"""
from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
QUEUE = ROOT / "engine" / "queue.json"
WIP_DIR = ROOT / "memory" / "wip"
MAP = ROOT / "build" / "bb2.map"
ELF = ROOT / "build" / "bb2.elf"
CARDS_DIR = ROOT / "tmp" / "cards"
TECH_INDEX = ROOT / ".claude" / "rules" / "codegen-technique-index.md"
RULES_DIR = ROOT / ".claude" / "rules"
MEMORY_PROJECT = ROOT / "memory" / "project"

# GitHub draft-issue body hard limit is 65536; keep a safety margin.
BODY_LIMIT = 65000
TRUNCATE_TARGET = 60000

sys.path.insert(0, str(ROOT / "tools"))
sys.path.insert(0, str(ROOT / "tmp"))


# ---------------------------------------------------------------------------
# reused recipes (consolidated from tmp/ research scripts + tools/board_sync)
# ---------------------------------------------------------------------------

def _read_text(path) -> str:
    """Read any repo text file UTF-8 with errors=replace (cp1252-safe)."""
    try:
        return Path(path).read_text(encoding="utf-8", errors="replace")
    except OSError:
        return ""


# --- extract_c_body (tmp/extract_c_body.py, inlined verbatim) --------------

def _skip_literal(text, i, n):
    q = text[i]
    i += 1
    while i < n:
        if text[i] == "\\":
            i += 2
            continue
        if text[i] == q:
            return i + 1
        i += 1
    return n


def _match_brace(text, open_idx):
    depth, i, n = 0, open_idx, len(text)
    while i < n:
        c = text[i]
        if c in "\"'":
            i = _skip_literal(text, i, n)
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            nl = text.find("\n", i)
            i = n if nl == -1 else nl
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            ce = text.find("*/", i + 2)
            i = n if ce == -1 else ce + 2
            continue
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    return -1


def _def_pattern(func):
    return re.compile(r"(?m)^[A-Za-z_][\w \t\*]*\b" + re.escape(func) + r"\s*\(")


def _matching_paren(text, i):
    n, depth = len(text), 1
    while i < n and depth > 0:
        c = text[i]
        if c in "\"'":
            i = _skip_literal(text, i, n)
            continue
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
        i += 1
    return i if depth == 0 else -1


def _leading_comment_start(text, def_start):
    start = def_start
    line_start = text.rfind("\n", 0, start) + 1
    i = line_start
    while i > 0:
        prev_nl = text.rfind("\n", 0, i - 1)
        prev_line_start = prev_nl + 1
        prev_line = text[prev_line_start:i - 1] if i - 1 >= prev_line_start else ""
        stripped = prev_line.strip()
        is_comment = (stripped.startswith("//") or stripped.startswith("/*")
                      or stripped.startswith("*") or stripped.endswith("*/"))
        if stripped == "" or not is_comment:
            break
        start = prev_line_start
        i = prev_line_start
        if i == 0:
            break
    return start


def extract_c_body(src_path, func_name):
    """Full C text of func_name's DEFINITION in src_path (signature -> matching
    `}`, plus an immediately-preceding comment), or None if not a definition in
    this file. Column-0-anchored, comment/string-aware. (tmp/extract_c_body.py.)"""
    text = _read_text(src_path)
    if not text:
        return None
    n = len(text)
    for m in _def_pattern(func_name).finditer(text):
        i = _matching_paren(text, m.end())
        if i == -1:
            continue
        j = i
        while j < n:
            c = text[j]
            if c in " \t\r\n":
                j += 1
                continue
            if c == ";":
                k = j + 1
                while k < n and text[k] in " \t\r\n":
                    k += 1
                if k < n and text[k] == "{":
                    j = k
                    continue
                if k < n and (text[k].isalpha() or text[k] == "_"):
                    j = k
                    continue
                break
            break
        if j >= n or text[j] != "{":
            continue
        end = _match_brace(text, j)
        if end == -1:
            continue
        start = _leading_comment_start(text, m.start())
        return text[start:end]
    return None


# --- collect_func_cheats (tmp/collect_cheats.py) ---------------------------

REGFIX_FILES = ["regfix.txt", "regfix_stage2.txt"]
ASMFIX_FILES = ["asmfix.txt"]

OP_CATEGORY = {
    "subst": ("regfix-subst", "Text substitution on one instruction (regex -> replacement @ idx)"),
    "subst_multi": ("regfix-subst-multi", "Multi-line substitution (>=30 lines = full-body asm replacement cheat)"),
    "splice": ("regfix-splice", "Replace an index range with literal asm lines (>=30 lines = asm-from-rule cheat)"),
    "delete": ("regfix-delete", "Delete the instruction at @ idx"),
    "delete_between": ("asmfix-delete-between", "Delete all lines between two regex anchors"),
    "insert": ("regfix-insert", "Inject a literal instruction @ idx (lost-codegen if addu ...,$zero)"),
    "insert_after": ("regfix-insert-after", "Inject a literal instruction after @ idx (lost-codegen if addu ...,$zero)"),
    "insert_before": ("asmfix-insert-before", "Inject literal asm before a regex anchor"),
    "insert_label": ("regfix-insert-label", "Synthesize a new stable branch-target label"),
    "reorder": ("regfix-reorder", "Rearrange instructions in a range to a new index order"),
    "fill_delay": ("regfix-fill-delay", "Move an instruction into a branch/jal delay slot"),
    "drain_delay": ("regfix-drain-delay", "Pull an instruction out of a delay slot"),
    "replace_first": ("asmfix-replace-first", "Replace first line matching a regex (often label retarget)"),
    "replace_with_asmfile": ("regfix-replace-asmfile", "Replace whole function body with asm/funcs/<func>.s (canonical/bridge)"),
    "rename": ("asmfix-rename", "Rename a label (jtbl-rodata-split infrastructure when -> jtbl_<addr>)"),
}

_LCG_RE = re.compile(r'^\S+\s*:\s*insert(?:_after)?\s+["\']addu[^"\']*(?:\$zero|\$0)[^"\']*["\']')
_REGSWAP_RE = re.compile(r'^\S+\s*:\s*\$\d+\s*<->\s*\$\d+')
_OP_RE = re.compile(r'^\S+\s*:\s*(\w+)')


def _classify_cheat(line):
    if _LCG_RE.match(line):
        return ("regfix-lost-codegen-insert",
                "CHEAT: injects an instruction GCC's optimizer ate (addu rd,_,$zero). "
                "Bytes from rule text, not C. See lost-codegen-insert-cheat.md")
    if _REGSWAP_RE.match(line):
        return ("regfix-reg-swap",
                "Register-allocation swap ($a <-> $b), optionally @ range — RA mismatch fixup")
    m = _OP_RE.match(line)
    if m and m.group(1) in OP_CATEGORY:
        return OP_CATEGORY[m.group(1)]
    return ("regfix-unknown", "Unrecognized rule op")


def _trailing_comment(line):
    in_s = None
    for i, c in enumerate(line):
        if in_s:
            if c == "\\":
                continue
            if c == in_s:
                in_s = None
        elif c in "\"'":
            in_s = c
        elif c == "#":
            return line[i + 1:].strip()
    return ""


def collect_func_cheats(func, rule_cache):
    """[{type, rule_text, description, source_file}] for every regfix/asmfix rule
    keyed to `func`. rule_cache: {rel -> [lines]} (read once)."""
    kr = re.compile(rf"^{re.escape(func)}\s*:")
    out = []
    for rel in REGFIX_FILES + ASMFIX_FILES:
        for ln in rule_cache.get(rel, []):
            if not kr.match(ln):
                continue
            typ, desc = _classify_cheat(ln)
            comment = _trailing_comment(ln)
            out.append({
                "type": typ,
                "rule_text": ln.rstrip(),
                "description": desc + (f" — note: {comment}" if comment else ""),
                "source_file": rel,
            })
    return out


def load_rule_cache():
    cache = {}
    for rel in REGFIX_FILES + ASMFIX_FILES:
        p = ROOT / rel
        cache[rel] = _read_text(p).splitlines() if p.exists() else []
    return cache


# --- history_by_func (tmp/history_by_func.py) ------------------------------

SEP = "\x1e"
REC = "\x1f"
FUNC_RE = re.compile(r'\bfunc_[0-9A-Fa-f]{8}\b')
NAMED_RE = re.compile(r'\b([A-Za-z][A-Za-z0-9]*(?:_[0-9A-Fa-f]{8})?)\b')
PREFIX_RE = re.compile(r'^\s*(Match|wip|park|cheat-cleanup|cleanup|auth|'
                       r'inline_asm_canonical|naming|tools|hooks|docs|engine|'
                       r'queue|audit|trace|research|Merge|Revert)\b', re.I)
TECH_RE = re.compile(r'technique=([A-Za-z0-9_]+)')
VERDICT_RE = re.compile(r'(?:cheat-)?reviewer[:\s]+(PASS|FAIL|ACCEPT|REJECT|NEEDS_USER)', re.I)
SCORE_RE = re.compile(r'score=([0-9A-Za-z_]+)')


def _load_known_names():
    names = set()
    try:
        out = subprocess.run(
            ["git", "grep", "-hoE", r"\b(func_[0-9A-Fa-f]{8}|[A-Za-z][A-Za-z0-9_]*)\b",
             "--", "symbol_addrs.txt", "named_syms.txt"],
            cwd=str(ROOT), capture_output=True, text=True, encoding="utf-8",
            errors="replace").stdout
        for tok in out.split():
            if (FUNC_RE.fullmatch(tok) or re.search(r'_[0-9A-Fa-f]{8}$', tok)
                    or re.search(r'[a-z][A-Z]', tok)):
                names.add(tok)
    except Exception:
        pass
    return names


def _git_log():
    fmt = SEP.join(["%H", "%cI", "%s", "%b"]) + REC
    out = subprocess.run(
        ["git", "log", "--all", "--no-merges", f"--format={fmt}"],
        cwd=str(ROOT), capture_output=True, text=True, encoding="utf-8",
        errors="replace").stdout
    for rec in out.split(REC):
        rec = rec.strip("\n")
        if not rec:
            continue
        parts = rec.split(SEP)
        if len(parts) < 4:
            parts += [""] * (4 - len(parts))
        sha, date, subject, body = parts[:4]
        yield sha, date, subject, body


def history_by_func():
    """{name: [{sha, date, subject, prefix, techniques, verdict, scores}]} from
    ONE git-log pass. func_XXXXXXXX keying is reliable; named funcs may be sparse."""
    from collections import defaultdict
    known = _load_known_names()
    idx = defaultdict(list)
    for sha, date, subject, body in _git_log():
        text = subject + "\n" + body
        funcs = set(FUNC_RE.findall(text))
        if known:
            for m in NAMED_RE.findall(text):
                if m in known:
                    funcs.add(m)
        if not funcs:
            continue
        pm = PREFIX_RE.match(subject)
        vm = VERDICT_RE.search(text)
        rec = {
            "sha": sha[:8],
            "date": date[:10],
            "subject": subject,
            "prefix": (pm.group(1).lower() if pm else None),
            "techniques": sorted(set(TECH_RE.findall(text))),
            "verdict": (vm.group(1).upper() if vm else None),
            "scores": SCORE_RE.findall(text),
        }
        for f in funcs:
            idx[f].append(rec)
    for f in idx:
        idx[f].sort(key=lambda r: r["date"], reverse=True)
    return idx


# --- objdump disasm_by_func (research asm recipe) --------------------------

_DISASM_HEADER = re.compile(r"^([0-9a-f]{8}) <([^>]+)>:$")
_DISASM_INSN = re.compile(r"^([0-9a-f]{8}):\t")


def _run_objdump(elf_path=None, repo=None):
    """Run objdump -d ONCE; return stdout, or None on failure. The ELF path must
    be WSL-resolvable: an absolute *Windows* path (C:\\...) cannot be opened by
    the Linux objdump, so default to repo-relative "build/bb2.elf" with cwd=ROOT
    (where wsl resolves it)."""
    repo = str(repo or ROOT)
    elf_path = str(elf_path) if elf_path else "build/bb2.elf"
    try:
        out = subprocess.run(
            ["wsl", "mipsel-linux-gnu-objdump", "-d", elf_path],
            cwd=repo, capture_output=True, text=True, encoding="utf-8",
            errors="replace")
    except (FileNotFoundError, OSError):
        return None
    if out.returncode != 0:
        return None
    return out.stdout


def _disasm_by_header(text):
    """Parse {name: disasm_text} by objdump's `^<addr> <name>:` header lines.
    `.L`-prefixed local-label headers MERGE into the current function's body
    (they are branch targets within it, not new functions). This is the
    fallback when no ELF symbol table is available for address slicing."""
    funcs = {}
    cur_name = None
    cur_lines = []
    for line in text.splitlines():
        m = _DISASM_HEADER.match(line)
        if m:
            name = m.group(2)
            if name.startswith(".L"):
                if cur_name is not None:
                    cur_lines.append(line)
                continue
            if cur_name is not None:
                funcs[cur_name] = "\n".join(cur_lines)
            cur_name = name
            cur_lines = [line]
        else:
            if cur_name is not None:
                cur_lines.append(line)
    if cur_name is not None:
        funcs[cur_name] = "\n".join(cur_lines)
    return funcs


def _disasm_by_address(text, sym_by_addr):
    """Slice the flat objdump instruction stream into per-function bodies by ELF
    symbol address. More robust than header parsing: objdump occasionally omits
    the `<addr> <name>:` separator for a function that "flows" out of the
    preceding one (no blank line), which drops that function from header-based
    parsing — but the instructions are still present. We slice each function as
    [addr, next_symbol_addr) and synthesize the `<addr> <name>:` header so the
    card format is uniform. sym_by_addr: {int_addr: name}."""
    # Collect instruction lines as (addr_int, raw_line) in file order.
    insns = []
    for line in text.splitlines():
        m = _DISASM_INSN.match(line)
        if m:
            insns.append((int(m.group(1), 16), line))
    if not insns:
        return {}
    sorted_addrs = sorted(sym_by_addr)
    # boundaries: a function owns instructions in [addr, next_func_addr).
    funcs = {}
    n = len(sorted_addrs)
    for i, start in enumerate(sorted_addrs):
        end = sorted_addrs[i + 1] if i + 1 < n else None
        name = sym_by_addr[start]
        body = [f"{start:08x} <{name}>:"]
        for addr, raw in insns:
            if addr < start:
                continue
            if end is not None and addr >= end:
                break
            body.append(raw)
        if len(body) > 1:  # skip symbols with no instructions in range
            funcs[name] = "\n".join(body)
    return funcs


def disasm_by_func(elf_path=None, repo=None, sym_by_addr=None):
    """Run objdump ONCE; return {name: disasm_text}. Returns {} if objdump is
    unavailable / fails.

    When `sym_by_addr` ({int_addr: name}) is provided, slice by address (robust
    against objdump's omitted separators); otherwise fall back to header
    parsing."""
    text = _run_objdump(elf_path, repo)
    if text is None:
        return {}
    if sym_by_addr:
        return _disasm_by_address(text, sym_by_addr)
    return _disasm_by_header(text)


def addr_from_disasm(disasm_text):
    """First 8-hex header address in a disasm slice, or None."""
    if not disasm_text:
        return None
    m = _DISASM_HEADER.match(disasm_text.splitlines()[0])
    return m.group(1) if m else None


# ---------------------------------------------------------------------------
# technique index (slug -> one-line description)
# ---------------------------------------------------------------------------

_INDEX_LINE = re.compile(r"^- \*\*([A-Za-z0-9_-]+)\*\*(?:\s*\([^)]*\))?\s*[—-]\s*(.+)$")


def load_technique_index():
    """{slug: one-line-desc} parsed from codegen-technique-index.md bullets."""
    out = {}
    for line in _read_text(TECH_INDEX).splitlines():
        m = _INDEX_LINE.match(line.strip())
        if m:
            out[m.group(1)] = m.group(2).strip()
    return out


def known_rule_slugs():
    return {p.name[:-3] for p in RULES_DIR.glob("*.md")}


# Policy/non-technique slugs that should not be surfaced as a "technique" lever.
_POLICY_SLUGS = {
    "no-new-park-categories", "no-new-regfix-rules", "no-compiler-divergence",
    "review-discipline-before-commit", "difficult-is-not-impossible",
    "completion-standard", "community-standard", "inline-asm-policy",
    "codegen-technique-index", "verify-claims-against-main",
}


# ---------------------------------------------------------------------------
# board function set
# ---------------------------------------------------------------------------

def load_queue_items(path=None):
    path = Path(path or QUEUE)
    if not path.exists():
        return []
    return json.loads(_read_text(path)).get("items", [])


def build_board_set(items, inventory):
    """Return {func: card-input dict} for every board function.

    Active funcs come from queue items (status from queue). Completed funcs =
    inventory − queue funcs (status "completed"); their stem from inventory.
    """
    board = {}
    queue_funcs = set()
    for it in items:
        f = it["func"]
        queue_funcs.add(f)
        board[f] = {
            "func": f,
            "file": it.get("file"),
            "distance": it.get("distance"),
            "verdict": it.get("verdict"),
            "rules": it.get("rules", 0),
            "status": it.get("status"),
            "park_reason": it.get("park_reason"),
        }
    for func, stem in inventory.items():
        if func in queue_funcs:
            continue
        board[func] = {
            "func": func,
            "file": stem,
            "distance": None,
            "verdict": None,
            "rules": 0,
            "status": "completed",
            "park_reason": None,
        }
    return board


# ---------------------------------------------------------------------------
# WIP checkpoint reader
# ---------------------------------------------------------------------------

def read_wip(func, wip_dir=None):
    """Read memory/wip/<func>/meta.json into a normalized dict, or None if no
    WIP dir. Type-guards the reviewer field (sometimes a bare string) and tolerates
    func/function and the several score/session field spellings."""
    wip_dir = Path(wip_dir or WIP_DIR)
    d = wip_dir / func
    meta_p = d / "meta.json"
    if not d.is_dir() or not meta_p.exists():
        return None
    try:
        meta = json.loads(_read_text(meta_p))
    except (ValueError, json.JSONDecodeError):
        return {"raw_error": True}
    scores = meta.get("scores", {}) or {}
    sessions = meta.get("sessions", []) or []

    # reviewer: dict-with-verdict OR bare string
    rev = meta.get("reviewer")
    if isinstance(rev, dict):
        reviewer = rev.get("verdict") or rev.get("decision") or "?"
    elif isinstance(rev, str):
        reviewer = rev
    else:
        reviewer = None

    # latest lever from the last session entry
    latest_lever = None
    if sessions:
        last = sessions[-1]
        latest_lever = (last.get("lever") or last.get("lever_rule")
                        or last.get("summary") or last.get("note"))

    # memory_notes -> sanitized refs (split path token, keep existing paths)
    mem_refs = []
    for note in (meta.get("memory_notes") or []):
        if not isinstance(note, str):
            continue
        tok = note.split()[0] if note.split() else ""
        if tok and (ROOT / tok).exists():
            mem_refs.append(tok)
    # plus func-<addr>-*.md glob
    addr = func[5:].lower() if func.startswith("func_") else None
    if addr:
        for p in sorted(MEMORY_PROJECT.glob(f"func-{addr}-*.md")):
            rel = str(p.relative_to(ROOT)).replace("\\", "/")
            if rel not in mem_refs:
                mem_refs.append(rel)

    remaining = meta.get("remaining_gap") or {}
    gap_summary = remaining.get("summary") if isinstance(remaining, dict) else str(remaining)

    return {
        "candidate_floor": scores.get("candidate_floor"),
        "head_floor": scores.get("head_floor"),
        "target_insns": scores.get("target_insns") or scores.get("build_insns"),
        "reviewer": reviewer,
        "sessions": len(sessions),
        "latest_lever": latest_lever,
        "rejected_forms": len(meta.get("rejected_forms") or []),
        "gap_summary": gap_summary,
        "lever_slugs": _wip_lever_slugs(sessions),
        "memory_refs": mem_refs,
    }


def _wip_lever_slugs(sessions):
    """Collect lever_rule slugs across sessions, stripped of parentheticals."""
    slugs = []
    for s in sessions:
        lr = s.get("lever_rule")
        if not lr or not isinstance(lr, str):
            continue
        # strip a trailing parenthetical / section ref
        slug = re.split(r"[\s(]", lr.strip())[0].strip()
        slug = slug.strip(".,;")
        if slug and slug not in slugs:
            slugs.append(slug)
    return slugs


# ---------------------------------------------------------------------------
# card body assembly
# ---------------------------------------------------------------------------

STATUS_LABEL = {
    "active": "INCOMPLETE (active)",
    "parked": "PARKED",
    "completed": "COMPLETED",
}


def _brief(card):
    status = card["status"]
    verdict = card.get("verdict")
    if status == "completed":
        return "COMPLETED — matched, byte-identical, zero cheats."
    if status == "parked":
        pr = card.get("park_reason") or "see park reason"
        return f"PARKED — {pr}"
    # active
    if verdict and verdict != "C":
        return f"INCOMPLETE — routed {verdict}; may need canonical-asm authorization."
    d = card.get("distance")
    n = card.get("rules", 0)
    return (f"INCOMPLETE — honest pure-C distance {d}, {n} cheat rule(s). "
            "Goal: COMPLETED-C (zero rules, byte-identical).")


def _next_step(card, wip):
    status = card["status"]
    if status == "completed":
        return "Done — reference only. The matched C source is below."
    if status == "parked":
        pr = card.get("park_reason") or "(no reason recorded)"
        return (f"Blocked: {pr}\n\nUnblock by finding a natural pure-C form (or a "
                "canonical-asm authorization where the original was hand-written asm).")
    # active
    parts = []
    verdict = card.get("verdict")
    if verdict and verdict != "C":
        parts.append(f"Routed {verdict}: confirm with the `canonical` gate; if genuinely "
                     "no-C-form, it needs canonical-asm authorization, otherwise grind to pure C.")
    else:
        parts.append("Close the gap to pure C (the listed cheat rules must all go); "
                     "the goal is COMPLETED-C: zero rules, byte-identical.")
    if wip:
        parts.append(f"Resume from the WIP checkpoint: apply `memory/wip/{card['func']}/candidate.c`, "
                     "confirm the documented floor with `sandbox`, iterate from there.")
    return "\n".join(parts)


def _fmt(v, dash="—"):
    return dash if v is None else str(v)


def _section_cheats(cheats, status):
    out = [f"## Identified cheats / rules ({len(cheats)})", ""]
    if cheats:
        for c in cheats:
            line = f"- **{c['type']}** — `{c['rule_text']}`  {c['description']}  _({c['source_file']})_"
            out.append(line)
    elif status == "completed":
        out.append("None — pure C.")
    else:
        out.append("No regfix/asmfix rules; INCOMPLETE via honest distance or cheat-asm.")
    out.append("")
    return "\n".join(out)


def _section_wip(card, wip):
    if not wip:
        return ""
    if wip.get("raw_error"):
        return ("## WIP checkpoint\n\n"
                f"- meta.json present but could not be parsed; inspect `memory/wip/{card['func']}/`.\n")
    out = ["## WIP checkpoint", ""]
    out.append(f"- Floor: candidate **{_fmt(wip['candidate_floor'])}** vs head "
               f"{_fmt(wip['head_floor'])}  (target insns: {_fmt(wip['target_insns'])})")
    out.append(f"- Reviewer: {_fmt(wip['reviewer'])} · Sessions: {_fmt(wip['sessions'])} "
               f"· Rejected forms: {_fmt(wip['rejected_forms'])}")
    if wip.get("latest_lever"):
        out.append(f"- Latest lever: {wip['latest_lever']}")
    if wip.get("gap_summary"):
        out.append(f"- Remaining gap: {wip['gap_summary']}")
    out.append(f"- **Resume:** apply `memory/wip/{card['func']}/candidate.c`, confirm floor with `sandbox`.")
    out.append("")
    return "\n".join(out)


def _section_techniques(wip, tech_index, rule_slugs, history):
    """Validated lever-rule slugs (from WIP sessions) + low-confidence inferred
    techniques from commit-subject 'via X' tails."""
    slugs = []
    if wip:
        for s in wip.get("lever_slugs", []):
            if s in rule_slugs and s not in _POLICY_SLUGS:
                slugs.append(s)
    if not slugs and not _via_hints(history):
        return ""
    out = ["## Techniques", ""]
    for s in slugs:
        desc = tech_index.get(s, "")
        out.append(f"- `{s}`" + (f" — {desc}" if desc else ""))
    via = _via_hints(history)
    if via:
        out.append("- inferred (low-confidence): " + "; ".join(via))
    out.append("")
    return "\n".join(out)


_VIA_RE = re.compile(r"\bvia\s+([A-Za-z0-9_][\w \-]{2,60})", re.I)


def _via_hints(history):
    hints = []
    for rec in (history or [])[:8]:
        m = _VIA_RE.search(rec["subject"])
        if m:
            h = m.group(1).strip().rstrip(".,;)")
            if h and h not in hints:
                hints.append(h)
    return hints[:3]


def _section_history(history):
    if not history:
        return ""
    out = [f"## History ({len(history)} commits, newest first)", ""]
    cap = 15
    for rec in history[:cap]:
        prefix = rec["prefix"] or "commit"
        subj = rec["subject"].replace("\r", " ").strip()
        out.append(f"- `{rec['sha']}` {rec['date']} **{prefix}** — {subj}")
    if len(history) > cap:
        out.append(f"- … and {len(history) - cap} more (see `git log --all`).")
    out.append("")
    return "\n".join(out)


def _section_memrefs(wip):
    if not wip:
        return ""
    refs = wip.get("memory_refs") or []
    if not refs:
        return ""
    out = ["## Memory refs", ""]
    for r in refs:
        out.append(f"- `{r}`")
    out.append("")
    return "\n".join(out)


def _details_block(summary, fence_lang, content):
    """A GFM <details> block (blank line after </summary> before the fence)."""
    fence = "```" + fence_lang if fence_lang else "```"
    return (f"<details><summary>{summary}</summary>\n\n"
            f"{fence}\n{content}\n```\n"
            f"</details>\n")


def _truncate_asm(disasm, keep_lines):
    """Keep the header + first keep_lines lines; append a truncation note."""
    lines = disasm.splitlines()
    total = len(lines)
    if keep_lines >= total:
        return disasm
    kept = lines[:keep_lines]
    dropped = total - keep_lines
    kept.append(f"; ... [truncated {dropped} of {total} lines — full: "
                f"mipsel-linux-gnu-objdump -d build/bb2.elf]")
    return "\n".join(kept)


def build_body(card, *, cheats, wip, history, c_body, disasm, tech_index,
               rule_slugs, head_short):
    """Assemble the full markdown card body, budgeting to <= BODY_LIMIT chars.

    Truncation order when over budget: asm details first (keep header + first K
    lines), then C details. Metadata/cheats/wip/history/memrefs are never cut.
    Returns (body, truncated_bool).
    """
    func = card["func"]
    status = card["status"]
    label = STATUS_LABEL.get(status, status or "?")
    stem = card.get("file")
    file_disp = f"`src/{stem}.c`" if stem else "`?`"
    addr = card.get("addr") or "?"
    verdict = _fmt(card.get("verdict"))
    distance = _fmt(card.get("distance"))
    rules = card.get("rules", 0)
    wip_yn = "yes" if wip else "no"

    head = []
    head.append(f"# {func}  ·  {label}")
    head.append("")
    head.append(f"**File:** {file_disp} · **Address:** `0x{addr}` · **Verdict:** {verdict} "
                f"· **Pure-C distance:** {distance} · **Rules:** {rules} · **WIP:** {wip_yn}")
    head.append("")
    head.append(f"> {_brief(card)}")
    head.append("")
    head.append("## Next step")
    head.append("")
    head.append(_next_step(card, wip))
    head.append("")

    fixed = "\n".join(head)
    fixed += "\n" + _section_cheats(cheats, status)
    wsec = _section_wip(card, wip)
    if wsec:
        fixed += "\n" + wsec
    tsec = _section_techniques(wip, tech_index, rule_slugs, history)
    if tsec:
        fixed += "\n" + tsec
    hsec = _section_history(history)
    if hsec:
        fixed += "\n" + hsec
    msec = _section_memrefs(wip)
    if msec:
        fixed += "\n" + msec

    footer = f"\n---\n*Generated by tools/board_cards.py @ {head_short}. Regenerate via `python tools/board_enrich.py`.*\n"

    c_content = c_body if c_body else f"// C source not available in src/{stem}.c"
    asm_content = disasm if disasm else "; disassembly not available (build/bb2.elf missing or objdump failed)"

    def assemble(c_text, asm_text):
        body = fixed + "\n"
        body += _details_block(f"Current C source (src/{stem}.c)", "c", c_text)
        body += "\n"
        body += _details_block("Target disassembly (build/bb2.elf)", "", asm_text)
        body += footer
        return body

    body = assemble(c_content, asm_content)
    if len(body) <= TRUNCATE_TARGET:
        return body, False

    truncated = True
    # 1) Truncate the asm block. Binary-search the keep count that fits.
    asm_lines = asm_content.splitlines()
    lo, hi, best = 0, len(asm_lines), 0
    while lo <= hi:
        mid = (lo + hi) // 2
        trial = assemble(c_content, _truncate_asm(asm_content, mid))
        if len(trial) <= TRUNCATE_TARGET:
            best = mid
            lo = mid + 1
        else:
            hi = mid - 1
    asm_trunc = _truncate_asm(asm_content, best)
    body = assemble(c_content, asm_trunc)
    if len(body) <= TRUNCATE_TARGET:
        return body, truncated

    # 2) Still over (huge C body): truncate the C block too, keeping asm minimal.
    asm_min = _truncate_asm(asm_content, min(best, 40)) if asm_lines else asm_content
    c_lines = c_content.splitlines()
    lo, hi, best_c = 0, len(c_lines), 0
    while lo <= hi:
        mid = (lo + hi) // 2
        c_trial = "\n".join(c_lines[:mid]) + (
            f"\n// ... [truncated {len(c_lines) - mid} of {len(c_lines)} lines — full: src/{stem}.c]"
            if mid < len(c_lines) else "")
        trial = assemble(c_trial, asm_min)
        if len(trial) <= TRUNCATE_TARGET:
            best_c = mid
            lo = mid + 1
        else:
            hi = mid - 1
    c_trunc = "\n".join(c_lines[:best_c])
    if best_c < len(c_lines):
        c_trunc += (f"\n// ... [truncated {len(c_lines) - best_c} of {len(c_lines)} "
                    f"lines — full: src/{stem}.c]")
    body = assemble(c_trunc, asm_min)
    # Absolute backstop: hard-clip to BODY_LIMIT.
    if len(body) > BODY_LIMIT:
        body = body[:BODY_LIMIT - 40].rstrip() + "\n; [hard-clipped to fit]\n"
    return body, truncated


# ---------------------------------------------------------------------------
# driver
# ---------------------------------------------------------------------------

def _git_head_short():
    try:
        r = subprocess.run(["git", "rev-parse", "--short", "HEAD"], cwd=str(ROOT),
                           capture_output=True, text=True, encoding="utf-8", errors="replace")
        return r.stdout.strip() or "?"
    except Exception:
        return "?"


def assemble_all(one_func=None):
    """Assemble all board funcs (or one). Returns (cards_written_or_body, stats)."""
    import board_sync  # reuse the authoritative inventory loader

    items = load_queue_items()
    sym_by_addr = {}
    if MAP.exists() and ELF.exists():
        inventory = board_sync.load_inventory(MAP, ELF)
        # full STT_FUNC address map (for address-sliced disasm + card addresses)
        try:
            sym_by_addr = board_sync._elf_func_symbols(ELF)
        except Exception as e:
            print(f"WARNING: could not read ELF symbols ({e}); falling back to "
                  "header-based disasm.", file=sys.stderr)
    else:
        inventory = {}
        print("WARNING: build/bb2.map or build/bb2.elf missing — completed funcs "
              "will be absent; only queue funcs will be carded.", file=sys.stderr)
    board = build_board_set(items, inventory)
    addr_by_name = {n: a for a, n in sym_by_addr.items()}

    rule_cache = load_rule_cache()
    tech_index = load_technique_index()
    rule_slugs = known_rule_slugs()
    head_short = _git_head_short()

    print(f"loading git history (one pass) ...", file=sys.stderr)
    history = history_by_func()
    print(f"running objdump (one pass) ...", file=sys.stderr)
    disasm = disasm_by_func(sym_by_addr=sym_by_addr)
    if not disasm:
        print("WARNING: objdump produced no functions — asm blocks will be placeholders.",
              file=sys.stderr)

    # backfill addresses: prefer the authoritative ELF symbol address, fall back
    # to the disasm header.
    for func, card in board.items():
        if func in addr_by_name:
            card["addr"] = f"{addr_by_name[func]:08x}"
        else:
            a = addr_from_disasm(disasm.get(func))
            if a:
                card["addr"] = a

    def build_one(func):
        card = board[func]
        stem = card.get("file")
        c_body = extract_c_body(ROOT / "src" / f"{stem}.c", func) if stem else None
        wip = read_wip(func)
        body, trunc = build_body(
            card,
            cheats=collect_func_cheats(func, rule_cache),
            wip=wip,
            history=history.get(func, []),
            c_body=c_body,
            disasm=disasm.get(func),
            tech_index=tech_index,
            rule_slugs=rule_slugs,
            head_short=head_short,
        )
        return body, trunc, (c_body is not None), (disasm.get(func) is not None), (wip is not None)

    if one_func is not None:
        if one_func not in board:
            print(f"ERROR: {one_func} is not a board function (not in queue, not in inventory).",
                  file=sys.stderr)
            return None, {}
        body, *_ = build_one(one_func)
        return body, {}

    CARDS_DIR.mkdir(parents=True, exist_ok=True)
    stats = {"assembled": 0, "with_asm": 0, "with_c": 0, "with_wip": 0,
             "truncated": 0, "largest": 0, "largest_func": None}
    for func in sorted(board):
        body, trunc, has_c, has_asm, has_wip = build_one(func)
        # Explicit LF line endings (Path.write_text has no newline= on older Py).
        with open(CARDS_DIR / f"{func}.md", "w", encoding="utf-8", newline="\n") as fh:
            fh.write(body)
        stats["assembled"] += 1
        stats["with_asm"] += int(has_asm)
        stats["with_c"] += int(has_c)
        stats["with_wip"] += int(has_wip)
        stats["truncated"] += int(trunc)
        if len(body) > stats["largest"]:
            stats["largest"] = len(body)
            stats["largest_func"] = func
    return CARDS_DIR, stats


def main():
    argv = sys.argv[1:]
    if argv:
        body, _ = assemble_all(one_func=argv[0])
        if body is None:
            sys.exit(1)
        sys.stdout.write(body)
        return
    out_dir, stats = assemble_all()
    print(f"\nboard_cards: assembled {stats['assembled']} cards -> {out_dir}")
    print(f"  with asm: {stats['with_asm']}  ·  with C: {stats['with_c']}  ·  "
          f"with WIP: {stats['with_wip']}  ·  truncated: {stats['truncated']}")
    print(f"  largest body: {stats['largest']} chars ({stats['largest_func']})")
    sample = next(iter(sorted(p.name for p in CARDS_DIR.glob('*.md'))), None)
    print(f"  sample card: {CARDS_DIR / sample if sample else '(none)'}")


if __name__ == "__main__":
    main()

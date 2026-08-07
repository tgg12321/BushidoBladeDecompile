#!/usr/bin/env python3
"""
naming_wave.py — atomic RESET / RENAME cascade for BB2 function names.

Phase 2 of the naming campaign (docs/naming/README.md). Given a set of
per-address operations it rewrites EVERY surface where a function name is a
key, so no surface can silently desync:

  RESET  <addr>            every semantic identifier at <addr> -> func_8XXXXXXX
  RENAME <addr> <newname>  every semantic identifier at <addr> -> <newname>

A function name in this tree is not cosmetic. It keys regfix/asmfix rules, the
maspsx gate lists, the canonical-asm authorization list, the sdata GP-relative
lists, engine/queue.json, and the `.s` filename that INCLUDE_ASM expands into a
literal `.include`. Missing one surface either silently drops a rule or breaks
the link, so this tool treats the surface list as a closed set and reports a
manifest of every edit it makes.

Byte-neutrality is the claim, the oracle is the proof: run
`python3 -m engine.cli verify-oracle` after --apply. --apply refuses to run on a
dirty working tree so `git checkout` is always a clean rollback.

Usage
-----
  # dry run over the census RESET/RENAME rows (writes nothing)
  python3 tools/naming_wave.py --from-census

  # dry run over a chosen subset, with a manifest
  python3 tools/naming_wave.py --from-census --only 0x80017200,0x800836C8 \
      --manifest tmp/wave.json

  # apply
  python3 tools/naming_wave.py --from-census --only 0x80017200 --apply

Out of scope by design: prose. Names inside `//`, `/* */`, docs/, and memory
markdown BODIES are history and stay as written; only ledger DIRECTORY names
move. `make setup` is never run (bb2.ld is hand-maintained).
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import re
import shutil
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CENSUS = ROOT / "docs/naming/function-names.csv"

AUTO_RE = re.compile(r"^func_[0-9A-Fa-f]{8}$")
ADDR_RE = re.compile(r"^0x8[0-9A-Fa-f]{7}$")

# ---------------------------------------------------------------- surfaces --

# Linker-script / symbol-registry files: `name = 0xADDR;` lines. A name here is
# a DEFINITION when no object defines it, so these are edited line-wise (delete
# vs rewrite), never by blind substitution.
SYMBOL_FILES = [
    "named_syms.txt",
    "undefined_syms_auto.txt",
    "undefined_funcs_auto.txt",
    "symbol_addrs.txt",  # splat-only (we never re-run splat) but kept in sync
]

# Rule files: `func: rule...` — the key before ':' plus any name in the body.
RULE_FILES = ["regfix.txt", "regfix_stage2.txt", "asmfix.txt"]

# One-name-per-line (optionally `name  # comment`) gate lists.
LIST_FILES = [
    "sdata_funcs.txt",
    "sdata_exclude.txt",
    "inline_asm_canonical.txt",
    "maspsx_label_nop_funcs.txt",
    "expand_lb_funcs.txt",
    "expand_dest_funcs.txt",
    "multu_funcs.txt",
    "multu_pad_funcs.txt",
    "volatile_extern_allowlist.txt",
    # prologue_fix reads these two alongside prologue_config.json. They live
    # under tools/ rather than the repo root, which is exactly why the first
    # version of this tool missed them.
    "tools/delay_slot_ra_funcs.txt",
    "tools/frame_fix_funcs.txt",
]

# Live tool/engine sources that gate BEHAVIOUR on a function name held in a
# string literal. tools/prologue_fix.py's PROLOGUE_ACCEPT_ARG_REGS_FUNCS is the
# proven case: renaming the function without renaming that set silently drops a
# prologue reorder and drifts 19 bytes. Archived and vendored trees are excluded.
PY_ROOTS = ["tools", "engine"]
PY_EXCLUDE_PARTS = {"archive", "maspsx", "decomp-permuter", "gcc-2.7.2", ".venv",
                    "__pycache__", "permuter"}
# Provenance, not a gate. rename_funcs.py's map RECORDS which Kengo name was
# applied to which address, and docs/naming/build_census.py reads it as an
# evidence source — rewriting it would rewrite the census's own input and turn
# every entry into a self-map. History stays history.
PY_EXCLUDE_FILES = {"tools/rename_funcs.py", "tools/propose_function_names.py"}
# A short string literal is an identifier or dict key; a long one is prose that
# happens to cite a function. Only the former is a KEY worth rewriting.
PY_KEYLIKE_MAXLEN = 60

# Free-form build files edited by whole-word substitution on non-comment text.
PLAIN_FILES = ["bb2.ld", "Makefile"]

LEDGER_DIRS = ["memory/wip", "memory/grind"]


def die(msg: str) -> "NoReturn":  # noqa: F821
    print(f"FATAL: {msg}", file=sys.stderr)
    sys.exit(2)


def read(p: Path) -> str:
    return p.read_text(encoding="utf-8", errors="surrogateescape")


def write_lf(p: Path, text: str) -> None:
    """Write with hard LF endings, no BOM — the toolchain requires it."""
    text = text.replace("\r\n", "\n").replace("\r", "\n")
    with open(p, "w", encoding="utf-8", errors="surrogateescape", newline="\n") as fh:
        fh.write(text)


# ------------------------------------------------------------ comment masks --

_C_TOKEN = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*.*?\*/', re.S)


def sub_c(pattern: re.Pattern, repl, text: str) -> tuple[str, int]:
    """Whole-word substitution in C/H text, skipping comments but NOT strings.

    String literals are in scope on purpose: `__asm__("glabel foo\n")` bodies
    and INCLUDE_ASM's stringified argument are assembled, so a name inside a
    string literal is load-bearing. Comments are prose and are left alone.
    """
    out, n, pos = [], 0, 0
    for m in _C_TOKEN.finditer(text):
        chunk = text[pos:m.start()]
        new, k = pattern.subn(repl, chunk)
        out.append(new)
        n += k
        tok = m.group(0)
        if tok.startswith(("//", "/*")):
            out.append(tok)  # comment: verbatim
        else:
            new, k = pattern.subn(repl, tok)  # string literal: in scope
            out.append(new)
            n += k
        pos = m.end()
    new, k = pattern.subn(repl, text[pos:])
    out.append(new)
    n += k
    return "".join(out), n


def sub_py(pattern: re.Pattern, repl, text: str) -> tuple[str, int, list[str]]:
    """Rewrite names inside SHORT string literals of Python source.

    Tool source is code, not config, so this is deliberately narrow: only
    string literals (never identifiers or comments), and only short ones, which
    are keys rather than prose. Every long-string occurrence is returned as a
    skip note so a name cited in an explanatory message is reported rather than
    silently rewritten or silently ignored.
    """
    import io
    import tokenize

    skips: list[str] = []
    try:
        toks = list(tokenize.generate_tokens(io.StringIO(text).readline))
    except Exception:
        return text, 0, ["(unparseable — left untouched)"]

    edits: list[tuple[tuple[int, int], tuple[int, int], str]] = []
    n = 0
    for tok in toks:
        if tok.type != tokenize.STRING:
            continue
        s = tok.string
        hits = pattern.findall(s)
        if not hits:
            continue
        if len(s) > PY_KEYLIKE_MAXLEN or s.startswith(('"""', "'''", 'r"""', "r'''")):
            skips.append(f"L{tok.start[0]}: {sorted(set(hits))} in prose/docstring — NOT rewritten")
            continue
        new, k = pattern.subn(repl, s)
        if k:
            edits.append((tok.start, tok.end, new))
            n += k

    if not edits:
        return text, 0, skips

    lines = text.split("\n")
    for (sr, sc), (er, ec), new in reversed(edits):
        if sr != er:
            continue
        line = lines[sr - 1]
        lines[sr - 1] = line[:sc] + new + line[ec:]
    return "\n".join(lines), n, skips


def _path_guarded(pattern: re.Pattern) -> re.Pattern:
    """Same alternation, but never matching a name used as a path component."""
    return re.compile(r"(?<![/\w.])(?:" + pattern.pattern.strip() + r")(?![\w.]*\.(?:c|o|s|h)\b)")


def sub_hash(pattern: re.Pattern, repl, text: str) -> tuple[str, int]:
    """Whole-word substitution skipping `#` comments (rule files, gate lists)."""
    lines, n = [], 0
    for line in text.split("\n"):
        code, sep, comment = line.partition("#")
        new, k = pattern.subn(repl, code)
        n += k
        lines.append(new + sep + comment)
    return "\n".join(lines), n


# ------------------------------------------------------------------- model --


class Op:
    """One address-keyed rename operation."""

    def __init__(self, address: str, kind: str, new_name: str, row: dict):
        self.address = address.lower()
        self.kind = kind                 # RESET | RENAME
        self.new_name = new_name
        self.row = row
        self.olds: set[str] = set()      # semantic identifiers being retired
        self.glabel = row.get("glabel", "")
        self.notes: list[str] = []
        self.delete_stale: str | None = None   # stale duplicate .s to remove

    def __repr__(self):
        return f"<{self.kind} {self.address} {sorted(self.olds)} -> {self.new_name}>"


class Wave:
    def __init__(self, ops: list[Op], keep_reverse_aliases: bool = False):
        self.ops = ops
        self.keep_reverse_aliases = keep_reverse_aliases
        # old identifier -> new identifier (code surfaces)
        self.code_map: dict[str, str] = {}
        # identifier -> address, for symbol-file line disposition
        self.sym_owner: dict[str, str] = {}
        # new name -> address
        self.targets: dict[str, str] = {}
        for op in ops:
            self.targets[op.new_name] = op.address
            for old in op.olds:
                if old != op.new_name:
                    self.code_map[old] = op.new_name
                self.sym_owner[old] = op.address
            self.sym_owner[op.new_name] = op.address
        self.pattern = None
        if self.code_map:
            keys = sorted(self.code_map, key=len, reverse=True)
            self.pattern = re.compile(r"\b(" + "|".join(map(re.escape, keys)) + r")\b")

    def repl(self, m):
        return self.code_map[m.group(0)]


# ------------------------------------------------------------------ census --


OVERRIDES = ROOT / "docs/naming/action-overrides.csv"


def load_overrides(path: Path | None = None) -> dict[str, dict]:
    """Rulings that differ from what build_census.py derives.

    function-names.csv is generated, so a hand-edit there is reverted by the next
    regen. Overrides are keyed by address, or by NAME for the rows whose census
    address is unusable (which is exactly the case that needs excluding).
    """
    p = path or OVERRIDES
    if not p.exists():
        return {}
    out: dict[str, dict] = {}
    with open(p, encoding="utf-8", newline="") as fh:
        rows = [ln for ln in fh if not ln.lstrip().startswith("#")]
    for r in csv.DictReader(rows):
        key = (r.get("key") or "").strip()
        if key:
            out[key.lower()] = {
                "action": (r.get("action") or "").strip().upper(),
                "new_name": (r.get("new_name") or "").strip(),
                "note": (r.get("note") or "").strip(),
            }
    return out


def load_census() -> list[dict]:
    with open(CENSUS, encoding="utf-8", errors="replace", newline="") as fh:
        return list(csv.DictReader(fh))


def idents_of(row: dict) -> set[str]:
    out = set()
    for n in [row.get("glabel", ""), row.get("current_name", "")]:
        if n:
            out.add(n.strip())
    for a in (row.get("aliases") or "").split(";"):
        a = a.strip()
        if a:
            out.add(a)
    return {n for n in out if n}


def build_ops(rows: list[dict], only: set[str] | None, allow_invalid: bool) -> tuple[list[Op], list[str]]:
    """Turn census RESET/RENAME rows into validated ops. Returns (ops, rejects)."""
    by_addr: dict[str, list[dict]] = defaultdict(list)
    for r in rows:
        by_addr[(r.get("address") or "").lower()].append(r)

    ops: list[Op] = []
    rejects: list[str] = []
    seen: set[str] = set()

    ov = load_overrides()
    used_ov: set[str] = set()

    for r in rows:
        action = (r.get("action") or "").strip()
        addr = (r.get("address") or "").strip().lower()

        # An override may re-class, re-target, or exclude a row. Keyed by address,
        # or by name where the census address is unusable.
        o = ov.get(addr) or ov.get((r.get("current_name") or "").strip().lower()) \
            or ov.get((r.get("glabel") or "").strip().lower())
        if o and action in ("RESET", "RENAME", ""):
            used_ov.add(o["note"][:40])
            if o["action"] == "EXCLUDE":
                if action in ("RESET", "RENAME"):
                    rejects.append(
                        f"{addr or r.get('current_name')}: EXCLUDED by ruling — {o['note']}")
                continue
            if o["action"] in ("RESET", "RENAME"):
                action = o["action"]
                r = dict(r, action=action, proposed_name=o["new_name"] or r.get("proposed_name", ""))

        if action not in ("RESET", "RENAME"):
            continue
        if only is not None and addr not in only:
            continue

        # P1 — the address is the primary key; without it a RESET has no target.
        if not ADDR_RE.match(addr):
            rejects.append(
                f"{action} row '{r.get('current_name')}' has unusable address "
                f"{r.get('address')!r} — no reset target can be derived"
            )
            continue

        if addr in seen:
            continue  # merged below

        # P2 — the census carries duplicate rows per address (two name layers
        # for the same function). Merge them into ONE op rather than racing.
        peers = [p for p in by_addr[addr] if (p.get("action") or "") in ("RESET", "RENAME")]
        actions = {(p.get("action") or "") for p in peers}
        if len(actions) > 1:
            rejects.append(f"{addr}: census rows disagree on action {sorted(actions)}")
            continue

        if action == "RESET":
            new_name = "func_" + addr[2:].upper()
        else:
            new_name = (r.get("proposed_name") or "").strip()
            if not new_name or not re.match(r"^[A-Za-z_]\w*$", new_name):
                rejects.append(f"{addr}: RENAME row has no usable proposed_name {new_name!r}")
                continue

        op = Op(addr, action, new_name, r)
        for p in peers:
            op.olds |= idents_of(p)
            if p.get("glabel") and not AUTO_RE.match(p["glabel"]):
                op.glabel = p["glabel"]
        # The auto name is the destination, not a thing being retired.
        op.olds = {n for n in op.olds if n != new_name}
        if len(peers) > 1:
            op.notes.append(f"merged {len(peers)} census rows at this address")
        if not op.olds:
            rejects.append(f"{addr}: nothing to rename (already {new_name})")
            continue
        seen.add(addr)
        ops.append(op)

    if rejects and not allow_invalid:
        for x in rejects:
            print(f"  REJECT: {x}")
    return ops, rejects


# --------------------------------------------------------------- preflight --


def preflight(ops: list[Op]) -> dict[str, list[str]]:
    """Collision + consistency checks, keyed by the address they condemn.

    Errors are op-scoped so a single unresolvable function (a stale duplicate
    split artifact, say) can be dropped from the wave with --drop-failing
    instead of blocking the other 300.
    """
    wave = Wave(ops)
    errors: dict[str, list[str]] = defaultdict(list)
    funcs_dir = ROOT / "asm/funcs"
    stems = {p.stem: p for p in funcs_dir.glob("*.s")}

    # Where does every identifier in the tree live? Used to prove a target name
    # is not already claimed by a DIFFERENT function.
    rows = load_census()
    home: dict[str, set[str]] = defaultdict(set)
    for r in rows:
        for n in idents_of(r):
            home[n].add((r.get("address") or "").lower())

    for op in wave.ops:
        # P3 — target name must not already name a different address.
        elsewhere = home.get(op.new_name, set()) - {op.address, ""}
        if elsewhere:
            errors[op.address].append(
                f"target name '{op.new_name}' is already used at "
                f"{sorted(elsewhere)} — refusing to create an ambiguous symbol"
            )

        # P4 — target .s filename must be free, or belong to this function.
        tgt_s = stems.get(op.new_name)
        if tgt_s is not None and op.glabel != op.new_name:
            own_s = stems.get(op.glabel)
            orphan = not asm_file_referenced(op.new_name)
            twin = own_s is not None and opcodes_of(tgt_s) == opcodes_of(own_s) \
                and opcodes_of(tgt_s) != []
            if orphan and twin:
                # A stale split artifact: same instructions, different label,
                # wired to nothing. Deleting it frees the target name.
                op.delete_stale = str(tgt_s.relative_to(ROOT)).replace("\\", "/")
                op.notes.append(
                    f"asm/funcs/{op.new_name}.s is a stale duplicate of "
                    f"asm/funcs/{op.glabel}.s (opcode bytes identical; referenced by no "
                    f"INCLUDE_ASM, bb2.ld entry or LINKED_ASM_FUNCS) — deleted so the "
                    f"reset target name is free")
            else:
                why = []
                if not orphan:
                    why.append("it IS referenced by a build input")
                if not twin:
                    why.append("its opcode bytes DIFFER from this function's")
                errors[op.address].append(
                    f"asm/funcs/{op.new_name}.s already exists while the live glabel "
                    f"is '{op.glabel}', and " + " and ".join(why) +
                    " — resolve by hand before resetting")

        # P5 — a glabel-layer op must have its source .s file present.
        if op.glabel and not AUTO_RE.match(op.glabel):
            if op.glabel not in stems and not _defined_in_src(op.glabel):
                errors[op.address].append(
                    f"glabel '{op.glabel}' has neither "
                    f"asm/funcs/{op.glabel}.s nor a src/ definition"
                )

    # P6 — no two ops may collide on one target name.
    seen: dict[str, str] = {}
    for op in wave.ops:
        if op.new_name in seen:
            errors[op.address].append(
                f"target '{op.new_name}' is also claimed by {seen[op.new_name]}")
        seen[op.new_name] = op.address

    return dict(errors)


_OPCODE_COL = re.compile(r"/\*\s*[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s*\*/")


def opcodes_of(p: Path) -> list[str]:
    """The instruction bytes of a split .s, ignoring labels and whitespace.

    Two splat runs format the same function differently (column widths change),
    so text comparison is useless for deciding whether one file is a stale
    duplicate of another. The opcode column is the invariant.
    """
    return [m.group(1).upper() for m in _OPCODE_COL.finditer(read(p))]


_ASM_REF_CACHE: dict[str, bool] | None = None


def asm_file_referenced(stem: str) -> bool:
    """Is asm/funcs/<stem>.s pulled into the build by anything?

    Three ways in: an INCLUDE_ASM argument (which expands to a literal
    `.include "asm/funcs/<NAME>.s"`), the Makefile's LINKED_ASM_FUNCS, or a
    direct bb2.ld object reference. Anything else in asm/funcs/ is reference-only.
    """
    global _ASM_REF_CACHE
    if _ASM_REF_CACHE is None:
        refs: set[str] = set()
        ia = re.compile(r'INCLUDE_ASM\s*\(\s*"[^"]+"\s*,\s*(\w+)\s*\)')
        for p in sorted((ROOT / "src").glob("*.c")):
            refs |= {m.group(1) for m in ia.finditer(read(p))}
        mk = read(ROOT / "Makefile")
        m = re.search(r"^LINKED_ASM_FUNCS\s*:=\s*(.*)$", mk, re.M)
        if m:
            refs |= set(m.group(1).split())
        for m in re.finditer(r"asm/funcs/(\w+)\.o", read(ROOT / "bb2.ld")):
            refs.add(m.group(1))
        _ASM_REF_CACHE = {r: True for r in refs}
    return stem in _ASM_REF_CACHE


_SRC_DEF_CACHE: dict[str, bool] = {}


def _defined_in_src(name: str) -> bool:
    if name in _SRC_DEF_CACHE:
        return _SRC_DEF_CACHE[name]
    pat = re.compile(r"^[A-Za-z_][\w \t\*]*\b" + re.escape(name) + r"\s*\(", re.M)
    found = False
    for p in sorted((ROOT / "src").glob("*.c")):
        t = read(p)
        for m in pat.finditer(t):
            line = t[t.rfind("\n", 0, m.start()) + 1: m.end()]
            if "extern" not in line:
                found = True
                break
        if found:
            break
    _SRC_DEF_CACHE[name] = found
    return found


# ------------------------------------------------------------------ planner --


_IGNORED_CACHE: dict[str, bool] = {}


def git_ignored(rel: str) -> bool:
    """Is this path gitignored? Such a file cannot be rolled back with git, so
    the wave must not touch it — `include/m2c_context.h` is generated and
    gitignored, and editing it left an unrevertable change during the pilot."""
    if rel not in _IGNORED_CACHE:
        r = subprocess.run(["git", "check-ignore", "-q", rel], cwd=ROOT,
                           capture_output=True)
        _IGNORED_CACHE[rel] = (r.returncode == 0)
    return _IGNORED_CACHE[rel]


class Plan:
    def __init__(self):
        self.file_edits: dict[str, tuple[str, int]] = {}   # relpath -> (new text, hits)
        self.file_renames: list[tuple[str, str]] = []      # relpath old -> new
        self.file_deletes: list[str] = []                 # stale duplicate .s
        self.dir_renames: list[tuple[str, str]] = []
        self.sym_deletes: list[tuple[str, int, str]] = []  # file, lineno, line
        self.json_edits: dict[str, tuple[str, list[str]]] = {}
        self.out_of_scope: list[str] = []

    def note_edit(self, rel: str, new_text: str, hits: int, old_text: str):
        if new_text == old_text:
            return
        if git_ignored(rel):
            self.out_of_scope.append(
                f"{rel}: gitignored (generated) — skipped, because an edit there "
                f"could not be rolled back with git; regenerate it instead")
            return
        self.file_edits[rel] = (new_text, hits)


def plan_wave(wave: Wave) -> Plan:
    plan = Plan()
    if not wave.pattern:
        return plan
    pat, repl = wave.pattern, wave.repl

    # --- C sources and headers -------------------------------------------
    for p in sorted((ROOT / "src").glob("*.c")) + sorted((ROOT / "include").glob("*.h")):
        t = read(p)
        new, n = sub_c(pat, repl, t)
        plan.note_edit(str(p.relative_to(ROOT)).replace("\\", "/"), new, n, t)

    for op in wave.ops:
        if op.delete_stale:
            plan.file_deletes.append(op.delete_stale)

    # --- assembly ---------------------------------------------------------
    # .s files carry no prose comments (only /* addr bytes */ machine columns),
    # so a plain whole-word substitution is safe and covers glabel, endlabel,
    # .globl, jal targets and .word symbol references in one pass.
    asm_files = (
        sorted((ROOT / "asm/funcs").glob("*.s"))
        + sorted((ROOT / "asm").glob("*.s"))
        + sorted((ROOT / "asm/data").glob("*.s"))
    )
    for p in asm_files:
        t = read(p)
        new, n = pat.subn(repl, t)
        rel = str(p.relative_to(ROOT)).replace("\\", "/")
        if rel in plan.file_deletes:
            continue
        plan.note_edit(rel, new, n, t)
        # The INCLUDE_ASM argument expands to a literal `.include "<name>.s"`,
        # so the filename must move with the glabel.
        if p.parent.name == "funcs" and p.stem in wave.code_map:
            plan.file_renames.append((rel, f"asm/funcs/{wave.code_map[p.stem]}.s"))

    # --- symbol registries (line-wise) ------------------------------------
    for rel in SYMBOL_FILES:
        p = ROOT / rel
        if not p.exists():
            continue
        plan_symbol_file(wave, plan, p, rel)

    # --- rule files and gate lists ---------------------------------------
    for rel in RULE_FILES + LIST_FILES:
        p = ROOT / rel
        if not p.exists():
            continue
        t = read(p)
        new, n = sub_hash(pat, repl, t)
        plan.note_edit(rel, new, n, t)

    # --- plain build files ------------------------------------------------
    # bb2.ld and the Makefile mix SYMBOLS with FILE PATHS. A path component
    # like `build/src/foo.o` names a translation unit, not the function; the
    # object only exists because src/foo.c does. Rewriting it would point the
    # hand-maintained linker script at a file that was never produced, so
    # paths are excluded from substitution and reported for manual follow-up.
    for rel in PLAIN_FILES:
        p = ROOT / rel
        if not p.exists():
            continue
        t = read(p)
        new, n = sub_hash(_path_guarded(pat), repl, t)
        plan.note_edit(rel, new, n, t)
        for old in wave.code_map:
            if re.search(r"(?:[/\w]" + re.escape(old) + r"\.(?:c|o|s)\b)|"
                         r"(?:\b" + re.escape(old) + r"\.(?:c|o|s)\b)", t):
                plan.out_of_scope.append(
                    f"{rel}: names a FILE after '{old}' — the symbol is renamed "
                    f"but the file/object path is left alone (a source-file "
                    f"rename is a separate, owner-level decision)"
                )

    # --- prologue_config.json (keys are function names) -------------------
    prel = "tools/prologue_config.json"
    ppath = ROOT / prel
    if ppath.exists():
        raw = read(ppath)
        data = json.loads(raw)
        changed = [k for k in data if k in wave.code_map]
        if changed:
            newdata = {wave.code_map.get(k, k): v for k, v in data.items()}
            plan.json_edits[prel] = (json.dumps(newdata, indent=2) + "\n", changed)

    # --- engine/queue.json (func field only; status/verdict untouched) ----
    qrel = "engine/queue.json"
    qpath = ROOT / qrel
    if qpath.exists():
        raw = read(qpath)
        data = json.loads(raw)
        changed = []
        for item in data.get("items", []):
            if item.get("func") in wave.code_map:
                changed.append(f"{item['func']} -> {wave.code_map[item['func']]}")
                item["func"] = wave.code_map[item["func"]]
        for key in ("build_failures",):
            lst = data.get(key) or []
            for i, v in enumerate(lst):
                if isinstance(v, str) and v in wave.code_map:
                    changed.append(f"{key}[{i}]: {v} -> {wave.code_map[v]}")
                    lst[i] = wave.code_map[v]
        if changed:
            plan.json_edits[qrel] = (json.dumps(data, indent=2) + "\n", changed)

    # --- live tool/engine source (name-keyed behaviour gates) -------------
    for base in PY_ROOTS:
        bp = ROOT / base
        if not bp.exists():
            continue
        for p in sorted(bp.rglob("*.py")):
            rel = str(p.relative_to(ROOT)).replace("\\", "/")
            if PY_EXCLUDE_PARTS & set(p.parts) or rel in PY_EXCLUDE_FILES:
                continue
            t = read(p)
            if not pat.search(t):
                continue
            new, n, skips = sub_py(pat, repl, t)
            plan.note_edit(rel, new, n, t)
            for s in skips:
                plan.out_of_scope.append(f"{rel}:{s}")

    # --- per-function ledger directories ----------------------------------
    for base in LEDGER_DIRS:
        bp = ROOT / base
        if not bp.exists():
            continue
        for d in sorted(bp.iterdir()):
            if d.is_dir() and d.name in wave.code_map:
                plan.dir_renames.append((f"{base}/{d.name}", f"{base}/{wave.code_map[d.name]}"))

    return plan


def _defined_after_wave(op: Op, mapsyms: dict[str, str]) -> bool:
    """Will an OBJECT define op.new_name once the wave has been applied?

    The link map describes the tree as it is now, so it cannot be read
    directly: for a glabel-layer op the definition site is one of the names
    being retired, and renaming it is precisely what makes the target
    object-defined. So the target is safe if either the target is already
    object-defined at this address, or one of the retired names is — because
    that definition moves to the target.
    """
    addr = op.address
    if mapsyms.get(op.new_name, "").lower() == addr:
        return True
    return any(mapsyms.get(old, "").lower() == addr for old in op.olds)


SYM_LINE = re.compile(r"^(\s*)([A-Za-z_]\w*)(\s*=\s*)(0x[0-9A-Fa-f]+)(\s*;)(.*)$")

# A defined symbol in the ld map is `<addr> <name>`; a script assignment is
# `<addr> <name> = <addr>`. Only the former means an object provides the symbol.
MAP_DEF = re.compile(r"^\s+(0x[0-9a-f]+)\s+([A-Za-z_]\w*)\s*$")


def load_map_symbols() -> dict[str, str] | None:
    """Object-defined symbols from the last link map, or None if unavailable.

    This is the safety net behind alias deletion. Some names in this tree are
    resolved ONLY by a linker-script assignment (`getScreenPosition =
    0x80017FA0;` with the code carrying a `func_` glabel). Deleting such a line
    would break the link, so the tool must know which target names an object
    actually defines before it drops anything.
    """
    mp = ROOT / "build/bb2.map"
    if not mp.exists():
        return None
    syms: dict[str, str] = {}
    for line in read(mp).split("\n"):
        m = MAP_DEF.match(line)
        if m:
            syms.setdefault(m.group(2), m.group(1))
    return syms or None


def plan_symbol_file(wave: Wave, plan: Plan, p: Path, rel: str) -> None:
    """Line-wise disposition for `name = 0xADDR;` registries.

    A RESET must delete the semantic claim, not rewrite it — otherwise the
    alias survives under a new spelling. It must also drop any pre-existing
    `func_<ADDR> = 0x<ADDR>;` reverse alias, because after the reset that
    symbol is defined by the object's glabel and the script line becomes a
    second, redundant definition of it.

    A MISNAMED note attached to a deleted line is evidence; it is preserved as
    a standalone comment so the finding does not die with the name.
    """
    lines = read(p).split("\n")
    out: list[str] = []
    hits = 0
    comment_style = "/* %s */" if rel != "symbol_addrs.txt" else "// %s"
    mapsyms = load_map_symbols()

    for i, line in enumerate(lines, 1):
        m = SYM_LINE.match(line)
        if not m:
            out.append(line)
            continue
        indent, name, eq, addr, semi, tail = m.groups()
        owner = wave.sym_owner.get(name)
        if owner is None or owner != addr.lower():
            out.append(line)
            continue

        op = next(o for o in wave.ops if o.address == addr.lower())

        if name == op.new_name:
            # Reverse alias for the destination name. After the wave the
            # object defines it; drop the duplicate script assignment.
            if wave.keep_reverse_aliases:
                out.append(line)
                continue
            plan.sym_deletes.append((rel, i, line))
            hits += 1
            continue

        # A retired semantic claim. Drop it — unless this line is the only
        # thing that resolves the destination name, in which case rewriting it
        # in place is what keeps the link intact.
        note = tail.strip()
        if (rel in ("named_syms.txt", "undefined_syms_auto.txt", "undefined_funcs_auto.txt")
                and mapsyms is not None
                and not _defined_after_wave(op, mapsyms)
                and not getattr(op, "kept_definition", False)):
            op.kept_definition = True
            out.append(f"{indent}{op.new_name}{eq}{addr}{semi}{tail}")
            op.notes.append(
                f"'{op.new_name}' is not object-defined per build/bb2.map — "
                f"{rel}:{i} rewritten in place rather than deleted, so the "
                f"symbol keeps a definition")
            hits += 1
            continue

        plan.sym_deletes.append((rel, i, line))
        hits += 1
        if note:
            note = note.strip("/*").strip("*/").strip()
            out.append(indent + comment_style % (
                f"{op.kind} {addr}: retired name '{name}' — preserved note: {note}"))

    new_text = "\n".join(out)
    plan.note_edit(rel, new_text, hits, "\n".join(lines))


# ------------------------------------------------------------------ report --


def report(wave: Wave, plan: Plan, rejects: list[str], errors: dict[str, list[str]], apply: bool) -> dict:
    print("=" * 78)
    print(f"NAMING WAVE — {len(wave.ops)} operations "
          f"({sum(1 for o in wave.ops if o.kind == 'RESET')} RESET, "
          f"{sum(1 for o in wave.ops if o.kind == 'RENAME')} RENAME)")
    print(f"  {len(wave.code_map)} identifiers remapped")
    print("=" * 78)

    if rejects:
        print(f"\n-- REJECTED ops ({len(rejects)}) — excluded from the wave --")
        for x in rejects:
            print(f"  ! {x}")

    if errors:
        n = sum(len(v) for v in errors.values())
        print(f"\n-- PREFLIGHT ERRORS ({n} across {len(errors)} addresses) "
              f"— apply is blocked (use --drop-failing to exclude them) --")
        for addr, msgs in sorted(errors.items()):
            for x in msgs:
                print(f"  X {addr}: {x}")

    print(f"\n-- file edits ({len(plan.file_edits)}) --")
    groups: dict[str, list[tuple[str, int]]] = defaultdict(list)
    for rel, (_, n) in sorted(plan.file_edits.items()):
        top = rel.split("/")[0] if "/" in rel else "(root)"
        groups[top].append((rel, n))
    for top, items in sorted(groups.items()):
        tot = sum(n for _, n in items)
        print(f"  {top}: {len(items)} files, {tot} substitutions")
        for rel, n in items if top == "(root)" else items[:6]:
            print(f"      {rel}  ({n})")
        if top != "(root)" and len(items) > 6:
            print(f"      ... and {len(items) - 6} more")

    print(f"\n-- .s file renames ({len(plan.file_renames)}) --")
    for a, b in plan.file_renames[:10]:
        print(f"  {a} -> {b}")
    if len(plan.file_renames) > 10:
        print(f"  ... and {len(plan.file_renames) - 10} more")

    print(f"\n-- ledger directory renames ({len(plan.dir_renames)}) --")
    for a, b in plan.dir_renames[:10]:
        print(f"  {a} -> {b}")
    if len(plan.dir_renames) > 10:
        print(f"  ... and {len(plan.dir_renames) - 10} more")

    print(f"\n-- symbol-registry line deletions ({len(plan.sym_deletes)}) --")
    for rel, ln, line in plan.sym_deletes[:12]:
        print(f"  {rel}:{ln}  {line.strip()[:110]}")
    if len(plan.sym_deletes) > 12:
        print(f"  ... and {len(plan.sym_deletes) - 12} more")

    print(f"\n-- JSON key/field edits ({len(plan.json_edits)}) --")
    for rel, (_, changed) in plan.json_edits.items():
        print(f"  {rel}: {len(changed)} changes")
        for c in changed[:5]:
            print(f"      {c}")
        if len(changed) > 5:
            print(f"      ... and {len(changed) - 5} more")

    if plan.out_of_scope:
        print(f"\n-- OUT OF SCOPE / manual follow-up ({len(plan.out_of_scope)}) --")
        for x in sorted(set(plan.out_of_scope)):
            print(f"  ~ {x}")

    manifest = {
        "mode": "apply" if apply else "dry-run",
        "ops": [
            {"address": o.address, "kind": o.kind, "new_name": o.new_name,
             "old_names": sorted(o.olds), "glabel": o.glabel, "notes": o.notes}
            for o in wave.ops
        ],
        "rejects": rejects,
        "preflight_errors": {k: v for k, v in errors.items()},
        "file_edits": {rel: n for rel, (_, n) in sorted(plan.file_edits.items())},
        "file_renames": plan.file_renames,
        "stale_deletions": plan.file_deletes,
        "dir_renames": plan.dir_renames,
        "symbol_deletions": [{"file": f, "line": ln, "text": t.strip()} for f, ln, t in plan.sym_deletes],
        "json_edits": {rel: changed for rel, (_, changed) in plan.json_edits.items()},
        "out_of_scope": sorted(set(plan.out_of_scope)),
    }
    return manifest


# ------------------------------------------------------------------- apply --


def dirty_targets(plan: Plan) -> list[str]:
    """Paths this wave would edit that already carry uncommitted changes.

    A blanket clean-tree requirement is wrong here: other agents work the same
    repo concurrently, and their dirt is none of this tool's business. What
    must hold is that `git checkout -- <our paths>` is a complete rollback, so
    only overlap between their dirt and our edits is disqualifying.
    """
    r = subprocess.run(["git", "status", "--porcelain"], cwd=ROOT,
                       capture_output=True, text=True)
    if r.returncode != 0:
        die("git status failed — cannot establish a rollback point")
    dirty = set()
    for line in r.stdout.splitlines():
        path = line[3:].strip().strip('"')
        if " -> " in path:
            path = path.split(" -> ", 1)[1]
        dirty.add(path)
    ours = set(plan.file_edits) | set(plan.json_edits)
    for a, b in plan.file_renames + plan.dir_renames:
        ours.add(a)
        ours.add(b)
    return sorted(p for p in dirty if p in ours or any(p.startswith(o + "/") for o in ours))


def apply_plan(plan: Plan) -> None:
    for rel, (text, _) in plan.file_edits.items():
        write_lf(ROOT / rel, text)
    for rel, (text, _) in plan.json_edits.items():
        write_lf(ROOT / rel, text)
    for rel in plan.file_deletes:
        r = subprocess.run(["git", "rm", "-q", "--", rel], cwd=ROOT,
                           capture_output=True, text=True)
        if r.returncode != 0:
            (ROOT / rel).unlink(missing_ok=True)
    for old, new in plan.file_renames:
        _git_mv(old, new)
    for old, new in plan.dir_renames:
        _git_mv(old, new)


def _git_mv(old: str, new: str) -> None:
    src, dst = ROOT / old, ROOT / new
    if not src.exists():
        die(f"cannot move missing path {old}")
    if dst.exists():
        die(f"refusing to overwrite existing {new}")
    dst.parent.mkdir(parents=True, exist_ok=True)
    r = subprocess.run(["git", "mv", old, new], cwd=ROOT, capture_output=True, text=True)
    if r.returncode != 0:
        shutil.move(str(src), str(dst))


def residual_audit(wave: Wave) -> dict[str, list[str]]:
    """After --apply, prove no retired name survives anywhere it is a KEY.

    The oracle catches anything that changes bytes, but a stale name in
    engine/queue.json or a ledger directory links and builds fine while
    quietly desyncing the worklist — so those need their own check. Prose
    (comments, docs, memory bodies) is excluded: history stays history.
    """
    out: dict[str, list[str]] = defaultdict(list)
    if not wave.pattern:
        return {}
    pat = wave.pattern

    def scan(rel: str, text: str, comment: str | None):
        if comment == "c":
            hits = set(pat.findall(re.sub(r"//[^\n]*|/\*.*?\*/", "", text, flags=re.S)))
        elif comment == "hash":
            hits = set(pat.findall(re.sub(r"#[^\n]*", "", text)))
        else:
            hits = set(pat.findall(text))
        for h in hits:
            out[h].append(rel)

    for p in sorted((ROOT / "src").glob("*.c")) + sorted((ROOT / "include").glob("*.h")):
        scan(str(p.relative_to(ROOT)), read(p), "c")
    for d in ("asm/funcs", "asm", "asm/data"):
        for p in sorted((ROOT / d).glob("*.s")):
            scan(str(p.relative_to(ROOT)), read(p), None)
    for rel in SYMBOL_FILES + RULE_FILES + LIST_FILES + PLAIN_FILES:
        p = ROOT / rel
        if p.exists():
            scan(rel, read(p), "hash" if rel not in SYMBOL_FILES else "c")
    for rel in ("engine/queue.json", "tools/prologue_config.json"):
        p = ROOT / rel
        if p.exists():
            scan(rel, read(p), None)
    # Name-keyed gates in live tool source: report a surviving SHORT string
    # literal only — a name cited in prose is history and is expected to remain.
    for base in PY_ROOTS:
        bp = ROOT / base
        if not bp.exists():
            continue
        for p in sorted(bp.rglob("*.py")):
            rel = str(p.relative_to(ROOT)).replace("\\", "/")
            if PY_EXCLUDE_PARTS & set(p.parts) or rel in PY_EXCLUDE_FILES:
                continue
            _, _, skips = sub_py(pat, lambda m: m.group(0), read(p))
            leftover, _, _ = sub_py(pat, lambda m: "\x00", read(p))
            if leftover != read(p):
                for h in set(pat.findall(read(p))):
                    out[h].append(str(p.relative_to(ROOT)))
    for base in LEDGER_DIRS:
        bp = ROOT / base
        if bp.exists():
            for d in bp.iterdir():
                if d.is_dir() and d.name in wave.code_map:
                    out[d.name].append(f"{base}/ (directory)")
    for p in (ROOT / "asm/funcs").glob("*.s"):
        if p.stem in wave.code_map:
            out[p.stem].append("asm/funcs/ (filename)")
    return {k: v for k, v in out.items() if v}


# -------------------------------------------------------------------- main --


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--from-census", action="store_true",
                    help="take RESET/RENAME rows from docs/naming/function-names.csv")
    ap.add_argument("--only", default=None,
                    help="comma-separated addresses to restrict the wave to")
    ap.add_argument("--apply", action="store_true", help="write changes (default: dry run)")
    ap.add_argument("--manifest", default=None, help="write the manifest JSON here")
    ap.add_argument("--allow-invalid", action="store_true",
                    help="proceed past unusable census rows instead of listing them loudly")
    ap.add_argument("--drop-failing", action="store_true",
                    help="exclude preflight-failing addresses instead of blocking the wave")
    ap.add_argument("--keep-reverse-aliases", action="store_true",
                    help="do NOT delete pre-existing `func_<ADDR> = 0x<ADDR>;` script lines")
    args = ap.parse_args()

    if not args.from_census:
        die("no operation source: pass --from-census")

    only = None
    if args.only:
        only = {a.strip().lower() for a in args.only.split(",") if a.strip()}

    rows = load_census()
    ops, rejects = build_ops(rows, only, args.allow_invalid)
    if only:
        missing = only - {o.address for o in ops}
        if missing:
            print(f"  NOTE: requested addresses with no usable op: {sorted(missing)}")
    if not ops:
        die("no operations to perform")

    errors = preflight(ops)
    if errors and args.drop_failing:
        dropped = sorted(errors)
        for addr in dropped:
            for msg in errors[addr]:
                rejects.append(f"{addr}: DROPPED — {msg}")
        ops = [o for o in ops if o.address not in errors]
        errors = {}
        print(f"  --drop-failing: excluded {len(dropped)} address(es) from the wave")
        if not ops:
            die("every operation failed preflight")

    wave = Wave(ops, keep_reverse_aliases=args.keep_reverse_aliases)
    plan = plan_wave(wave)
    manifest = report(wave, plan, rejects, errors, args.apply)

    if args.manifest:
        mp = Path(args.manifest)
        if not mp.is_absolute():
            mp = ROOT / mp
        mp.parent.mkdir(parents=True, exist_ok=True)
        write_lf(mp, json.dumps(manifest, indent=2) + "\n")
        print(f"\nmanifest -> {mp}")

    if not args.apply:
        print("\nDRY RUN — nothing written. Re-run with --apply.")
        return 1 if errors else 0

    if errors:
        die(f"{sum(len(v) for v in errors.values())} preflight error(s) — refusing to apply")
    conflicts = dirty_targets(plan)
    if conflicts:
        die("these files already have uncommitted changes AND are in this wave's "
            "edit set, so a rollback could not be told apart from someone else's "
            "work — commit or stash them first:\n  " + "\n  ".join(conflicts))

    apply_plan(plan)

    leftovers = residual_audit(wave)
    if leftovers:
        print(f"\n-- RESIDUAL AUDIT: {len(leftovers)} retired name(s) still present --")
        for name, hits in sorted(leftovers.items()):
            print(f"  {name}: {', '.join(hits[:6])}"
                  + (f" (+{len(hits) - 6} more)" if len(hits) > 6 else ""))
        print("  A surface was missed. Roll back and fix the tool before committing.")
    else:
        print("\nRESIDUAL AUDIT: clean — no retired name survives in any keyed surface.")

    print("\nAPPLIED. Now prove byte-neutrality:")
    print("    python3 -m engine.cli verify-oracle")
    print("  Expect SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa. On any drift,")
    print("  roll back exactly this wave's paths (see the manifest for the list).")
    return 0


if __name__ == "__main__":
    sys.exit(main())

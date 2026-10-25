#!/usr/bin/env python3
"""gte_macro_rewrite.py <func|tu> [--write] [--json] [--no-score]: written-out
PsyQ GTE islands -> calls of the vendored macros (engine/gtemacro.py).

For each function with assembly islands (a function name, or every such
function of a TU id such as main/17AFC), every run of back-to-back islands is
split into consecutive macros of the vendored headers (engine.gtemacro
header_macros, gtemac.h composites included), and replaced by the calls:

  1. statement-exact: each asm statement equals the header's (template,
     operands, clobbers; DMPSX placeholder == its command word, `0($12)` ==
     `($12)`), so cc1 sees the same statements;
  2. instruction-exact: the run's instruction stream equals the macros' once
     registers are numbered, `addu $X,Y,$zero` is `move`, joined statements
     are split and clobbers ignored, and the hand LZC store through `$sp`
     (`"=m"(v)`) is `gte_stlzc(&v)`. Clobbers steer register allocation, so
     only the score can admit these.

The header is the TU's vendored inline header when it includes one, else the
one that converts every run (inline_o.h first; a TU takes one header for all
its functions). Each function is scored with the honest sandbox (`engine
sandbox <func> --disable all --candidate`, the candidate under tmp/); with
--write a score-0 candidate is written to memory/grind/<func>/candidate.c
(an existing one only with --force; refused outright while the Grinder's
tmp/grind/grind.lock names a live pid). Nothing in src/ is ever written.

Reported for the reviewer (decisions the bytes cannot make):
  * macro choice: every other macro (or composite / parts split) with the same
    expansion. The pick prefers a macro the function's comments already name
    (an existing ruling names its macro there), then the fewest calls, then
    Sony's header order; the owner rule is "the macro named for the operation
    the C performs".
  * comments: a comment directly above a replaced run that explains the
    written-out form (it names a header, DMPSX, a clobber, an expansion or a
    hard register) is dropped and printed; other such comments in the body, and
    the function's doc comment, are listed for review.
A function whose islands have no GTE instruction is refused (not GTE).
Runs in WSL (the repo .venv); started on Windows it re-runs itself there.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shlex
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT_DIR = Path("tmp/gte_macro_rewrite")


def _rerun_in_wsl(argv: list[str]) -> int:
    drive, rest = os.path.splitdrive(str(ROOT))
    wsl_root = f"/mnt/{drive[0].lower()}{rest.replace(os.sep, '/')}"
    args = " ".join(shlex.quote(a) for a in argv)
    cmd = f"cd {shlex.quote(wsl_root)} && .venv/bin/python3 tools/gte_macro_rewrite.py {args}"
    return subprocess.run(["wsl", "-e", "bash", "-c", cmd]).returncode


def _engine():
    if str(ROOT) not in sys.path:
        sys.path.insert(0, str(ROOT))
    from engine import completion, gtemacro, inlineasm
    return completion, gtemacro, inlineasm


# --- instruction normalisation ------------------------------------------------
_DOTWORD = re.compile(r"\.word 0x([0-9A-Fa-f]{8})")
_REGNAMES = {"$zero": "$0", "$at": "$1", "$v0": "$2", "$v1": "$3", "$a0": "$4", "$a1": "$5",
             "$a2": "$6", "$a3": "$7", "$t0": "$8", "$t1": "$9", "$t2": "$10", "$t3": "$11",
             "$t4": "$12", "$t5": "$13", "$t6": "$14", "$t7": "$15", "$s0": "$16",
             "$s1": "$17", "$s2": "$18", "$s3": "$19", "$s4": "$20", "$s5": "$21",
             "$s6": "$22", "$s7": "$23", "$t8": "$24", "$t9": "$25", "$k0": "$26",
             "$k1": "$27", "$gp": "$28", "$sp": "$29", "$fp": "$30", "$s8": "$30", "$ra": "$31"}
_REGNAME_RE = re.compile(r"\$(?:zero|at|v[01]|a[0-3]|t[0-9]|s[0-8]|k[01]|gp|sp|fp|ra)\b")
_HOLE = re.compile(r"\{P:([^}]*)\}")


def norm(instr: str) -> str:
    """Comparison form of one normalized instruction: a DMPSX placeholder in
    DMPSX_WORDS is its command word; `0($REG)` in a load/store's memory
    operand is `($REG)` (maspsx cannot parse the header's `($12)`)."""
    _c, G, _i = _engine()
    m = _DOTWORD.fullmatch(instr)
    if m:
        v = int(m.group(1), 16)
        return f".word 0x{G.DMPSX_WORDS.get(v, (v,))[0]:08X}"
    mnem, sep, ops = instr.partition(" ")
    if mnem in G._MEM_OPS and sep:
        head, comma, last = ops.rpartition(",")
        z = G._ZERO_OFFSET_RE.fullmatch(last)
        if z:
            return f"{mnem} {head}{comma}{z.group(1)}"
    return instr


def _flat_norm(instr: str, ops: list[str]) -> str:
    """Instruction-exact form: numbered registers, `%N` -> `{operand}`,
    `addu $X,Y,$0` -> `move $X,Y`, then norm()."""
    instr = _REGNAME_RE.sub(lambda m: _REGNAMES[m.group(0)], instr)
    instr = re.sub(r"%(\d)", lambda m: "{" + (ops[int(m.group(1))]
                                             if int(m.group(1)) < len(ops) else "?") + "}", instr)
    m = re.fullmatch(r"addu (\$\w+),(\S+),\$0", instr)
    if m:
        instr = f"move {m.group(1)},{m.group(2)}"
    return norm(instr)


def _operand_texts(st, param: bool) -> list[str]:
    out = []
    for _c, e in (*st.outputs, *st.inputs):
        if param:
            out.append(("P:" + e[1]) if e[0] == "param" else e[1])
        else:
            out.append(e)
    return out


def flat_source(st) -> list[str]:
    """A source statement's instruction stream (instruction-exact form). The
    hand LZC store `"=m"(v)` addressed through `$sp` (`addiu $2,$sp,N; move
    $12,$2` or `move $12,$sp`) is gte_stlzc's `move $12,&v`."""
    ops = _operand_texts(st, False)
    seq = [_flat_norm(x, ops) for x in st.instrs]
    cons = [c for c, _e in (*st.outputs, *st.inputs)]
    for k, c in enumerate(cons):
        if c.startswith("=m"):
            seq = _fold_stack(seq, ops[k])
    return seq


def _fold_stack(seq: list[str], var: str) -> list[str]:
    out, i = [], 0
    while i < len(seq):
        if (re.fullmatch(r"addiu \$2,\$29,(?:0x[0-9A-Fa-f]+|\d+)", seq[i]) and i + 1 < len(seq)
                and seq[i + 1] == "move $12,$2"):
            out.append("move $12,{&" + var + "}")
            i += 2
            continue
        if seq[i] == "move $12,$29":
            out.append("move $12,{&" + var + "}")
            i += 1
            continue
        out.append(seq[i])
        i += 1
    return out


def flat_macro(macro) -> list[str]:
    out = []
    for st in macro.stmts:
        ops = _operand_texts(st, True)
        out += [_flat_norm(x, ops) for x in st.instrs]
    return out


# --- matching -----------------------------------------------------------------

def _stmt_exact(run, macro) -> dict | None:
    """Binding param -> source expression when `run` is, statement for
    statement, the macro's expansion; else None."""
    if len(run) != len(macro.stmts):
        return None
    binding: dict[str, str] = {}

    def ops_ok(src, hdr) -> bool:
        if len(src) != len(hdr):
            return False
        for (sc, se), (hc, (kind, he)) in zip(src, hdr):
            if sc != hc:
                return False
            if kind == "lit":
                if se != he:
                    return False
            elif binding.setdefault(he, se) != se:
                return False
        return True

    for s, h in zip(run, macro.stmts):
        if not s.volatile or s.clobbers != h.clobbers:
            return None
        if tuple(map(norm, s.instrs)) != tuple(map(norm, h.instrs)):
            return None
        if not ops_ok(s.outputs, h.outputs) or not ops_ok(s.inputs, h.inputs):
            return None
    return binding


def _template_match(seq: list[str], pat: list[str]) -> dict | None:
    if len(seq) != len(pat):
        return None
    binding: dict[str, str] = {}
    for s, p in zip(seq, pat):
        parts = _HOLE.split(p)
        rx, names = "", []
        for k, x in enumerate(parts):
            if k % 2:
                rx += r"\{(.*)\}"
                names.append(x)
            else:
                rx += re.escape(x)
        m = re.fullmatch(rx, s)
        if not m:
            return None
        for n, v in zip(names, m.groups()):
            if binding.setdefault(n, v) != v:
                return None
    return binding


@dataclass
class Unit:
    macro: str
    args: list[str]
    alternatives: list[str] = field(default_factory=list)


def _split(items: list, macros: dict, match, length, prefer: set[str]) -> list[Unit] | None:
    """Split `items` into consecutive macro expansions covering everything:
    fewest units; among equal-length matches of one segment, a macro in
    `prefer`, then Sony's header order. match(items, name) -> binding | None,
    length(name) -> how many items its expansion spans. None when something
    is left over."""
    names = [n for n in macros if length(n) > 0]
    longest = max((length(n) for n in names), default=0)
    n = len(items)
    best: list = [None] * (n + 1)
    best[0] = (0, -1, None, [])
    for j in range(1, n + 1):
        for i in range(max(0, j - longest), j):
            if best[i] is None:
                continue
            hits = []
            for name in names:
                if length(name) != j - i:
                    continue
                b = match(items[i:j], name)
                if b is not None and all(p in b for p in macros[name].params):
                    hits.append((name, b))
            if not hits:
                continue
            hits.sort(key=lambda h: (h[0] not in prefer, names.index(h[0])))
            cand = (best[i][0] + 1, i, hits[0], [h[0] for h in hits[1:]])
            if best[j] is None or cand[0] < best[j][0]:
                best[j] = cand
    if best[n] is None:
        return None
    out, j = [], n
    while j > 0:
        _k, i, (name, b), alts = best[j]
        out.append(Unit(name, [b[p] for p in macros[name].params], alts))
        j = i
    return out[::-1]


def _composite_parts(units: list[Unit], macros: dict) -> None:
    """Note, on each gtemac.h composite, the parts it could be written as."""
    for u in units:
        m = macros[u.macro]
        if m.header == "gtemac.h":
            u.alternatives.append("its parts written out (" + m.header + ")")


# --- source parsing -------------------------------------------------------------

def _strip_comments(raw: str) -> str:
    """Each comment -> one space; string literals untouched."""
    out, i, n = [], 0, len(raw)
    while i < n:
        c = raw[i]
        if c == '"':
            j = i + 1
            while j < n and raw[j] != '"':
                j += 2 if raw[j] == "\\" else 1
            out.append(raw[i:j + 1])
            i = j + 1
        elif raw.startswith("/*", i):
            j = raw.find("*/", i + 2)
            out.append(" ")
            i = n if j < 0 else j + 2
        elif raw.startswith("//", i):
            j = raw.find("\n", i)
            out.append(" ")
            i = n if j < 0 else j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def _string_newlines(raw: str) -> str:
    """A raw newline inside a string literal (GCC 2.7 accepts it) -> `\\n`."""
    out, in_str, i = [], False, 0
    while i < len(raw):
        c = raw[i]
        if in_str:
            if c == "\\":
                out.append(raw[i:i + 2])
                i += 2
                continue
            if c == "\n":
                out.append("\\n")
                i += 1
                continue
            if c == '"':
                in_str = False
        elif c == '"':
            in_str = True
        out.append(c)
        i += 1
    return "".join(out)


def parse_island(text: str, s: int, e: int):
    _c, G, inlineasm = _engine()
    raw = _string_newlines(_strip_comments(text[s:e].rstrip(" \t\n;")))
    kw = re.match(r"(__asm__|__asm|asm)\s*((?:volatile|__volatile__)?)\s*\(", raw)
    if not kw or not raw.endswith(")"):
        raise ValueError("not an asm statement")
    return G.parse_asm(raw[:kw.end() - 1], raw[kw.end():-1])


def _runs(text: str, func: str):
    """Runs of back-to-back islands: [(start, end, Stmt | None), ...]."""
    completion, _G, inlineasm = _engine()
    masked = inlineasm._code_without_comments_and_strings(text)
    runs = []
    for s, e in completion.own_blocks(text, func):
        while e < len(text) and text[e - 1] != ";" and text[e] in " \t":
            e += 1
        if e < len(text) and text[e - 1] != ";" and text[e] == ";":
            e += 1
        try:
            st = parse_island(text, s, e)
        except ValueError:
            st = None
        if runs and not masked[runs[-1][-1][1]:s].strip(" \t\n;"):
            runs[-1].append((s, e, st))
        else:
            runs.append([(s, e, st)])
    return runs


_STALE = re.compile(r"inline_[oc]\.h|gtemac\.h|DMPSX|clobber|expansion|\$1[2-5]\b|\$t[4-7]\b",
                    re.I)


def _attached_comment(text: str, start: int) -> int | None:
    """Start of the block comment directly above `start` (only whitespace
    between; the comment opens its own line), or None."""
    i = start
    while i > 0 and text[i - 1] in " \t\n":
        i -= 1
    if not text.startswith("*/", i - 2):
        return None
    open_ = text.rfind("/*", 0, i - 2)
    if open_ < 0 or text[text.rfind("\n", 0, open_) + 1:open_].strip():
        return None
    return open_


@dataclass
class Result:
    func: str
    stem: str
    header: str | None = None
    units: list[tuple[int, str, list[Unit]]] = field(default_factory=list)  # (line, tier, units)
    dropped_comments: list[tuple[int, str]] = field(default_factory=list)
    review_comments: list[tuple[int, str]] = field(default_factory=list)
    error: str | None = None
    body: str | None = None        # the rewritten function definition
    includes: list[str] = field(default_factory=list)
    score: int | None = None
    candidate: str | None = None

    def ambiguities(self) -> list[str]:
        return [f"L{ln} {u.macro}: also {', '.join(u.alternatives)}"
                for ln, _t, us in self.units for u in us if u.alternatives]


def rewrite_function(text: str, func: str, stem: str = "", header: str | None = None,
                     root: str | Path = ROOT) -> Result:
    """The pure rewrite (no scoring): `text` is the TU, `header` forces the
    inline header (else the TU's, else the first that converts every run)."""
    _c, G, inlineasm = _engine()
    res = Result(func, stem)
    span = inlineasm._func_body_span(text, func)
    if span is None:
        candidates = inlineasm.func_definition_candidates(text, func)
        res.error = (f"ambiguous C definition: {len(candidates)} definition-shaped heads"
                     if len(candidates) > 1 else "no C body")
        return res
    runs = _runs(text, func)
    if not runs:
        res.error = "no assembly islands"
        return res
    if any(st is None for run in runs for *_x, st in run):
        res.error = "an island does not parse as one asm statement"
        return res
    cop2 = any(x.split(" ", 1)[0].lower() in G._COP2_OPS
               or G.cia.CANONICAL_DOTWORD_RE.search(norm(x))
               for run in runs for *_x, st in run for x in st.instrs)
    if not cop2:
        res.error = "not GTE: no island carries a cop2 instruction or GTE command"
        return res
    try:
        tu_hdr = G.tu_header(text)
    except ValueError as exc:
        res.error = str(exc)
        return res
    body_text = text[span[0]:span[1]]
    prefer = set(re.findall(r"\bgte_\w+", "".join(
        m.group(0) for m in re.finditer(r"/\*.*?\*/|//[^\n]*", body_text, re.S))))
    order = [header or tu_hdr] if (header or tu_hdr) else list(G.INLINE_HEADERS)
    failures = []
    for hdr in order:
        macros = G.header_macros(hdr, root)
        plan = []
        for run in runs:
            stmts = [st for *_x, st in run]
            units = _split(stmts, macros, lambda items, n: _stmt_exact(items, macros[n]),
                           lambda n: len(macros[n].stmts), prefer)
            tier = "statement-exact"
            if units is None:
                seq = sum((flat_source(st) for st in stmts), [])
                pats = {n: flat_macro(m) for n, m in macros.items()}
                units = _split(seq, macros, lambda items, n: _template_match(items, pats[n]),
                               lambda n: len(pats[n]), prefer) if seq else None
                tier = "instruction-exact"
            line = text.count("\n", 0, run[0][0]) + 1
            if units is None:
                failures.append(f"{hdr}: the run at line {line} is not a sequence of its macros")
                plan = None
                break
            _composite_parts(units, macros)
            plan.append((run, line, tier, units))
        if plan is not None:
            break
    else:
        res.error = "; ".join(failures)
        return res
    res.header = hdr
    new, edits = text, []
    for run, line, tier, units in plan:
        start, end = run[0][0], run[-1][1]
        ind = text[text.rfind("\n", 0, start) + 1:start]
        calls = ("\n" + ind).join(f"{u.macro}({', '.join(u.args)});" for u in units)
        for m in re.finditer(r"/\*.*?\*/|//[^\n]*", text[start:end], re.S):
            res.dropped_comments.append((text.count("\n", 0, start + m.start()) + 1, m.group(0)))
        att = _attached_comment(text, start)
        if att is not None and _STALE.search(text[att:start]):
            res.dropped_comments.append((text.count("\n", 0, att) + 1, text[att:start].strip()))
            start = att
        edits.append((start, end, calls))
        res.units.append((line, tier, units))
    for s, e, r in sorted(edits, reverse=True):
        new = new[:s] + r + new[e:]
    nspan = inlineasm._func_body_span(new, func)
    res.body = new[nspan[0]:nspan[1]]
    # The function's doc comment too: it may describe the written-out form.
    doc = _attached_comment(new, new.rfind("\n", 0, nspan[0]) + 1)
    if doc is not None:
        c = new[doc:new.find("*/", doc) + 2]
        if _STALE.search(c):
            res.review_comments.append((new.count("\n", 0, doc) + 1,
                                        " ".join(c.split())[:100]))
    for m in re.finditer(r"/\*.*?\*/|//[^\n]*", res.body, re.S):
        if _STALE.search(m.group(0)):
            res.review_comments.append((new.count("\n", 0, nspan[0] + m.start()) + 1,
                                        " ".join(m.group(0).split())[:100]))
    used_composite = any(G.header_macros(hdr, root)[u.macro].header == G.COMPOSITE_HEADER
                         for _l, _t, us in res.units for u in us)
    have = G.tu_includes(text)
    res.includes = [f"#include <psxsdk/{h}>" for h in
                    ([hdr] if hdr not in have else [])
                    + ([G.COMPOSITE_HEADER] if used_composite and G.COMPOSITE_HEADER not in have else [])]
    return res


def candidate_text(res: Result) -> str:
    return "".join(i + "\n" for i in res.includes) + res.body + "\n"


# --- driver -----------------------------------------------------------------------

def _functions(target: str) -> list[tuple[str, str]]:
    """(func, stem) for a function name or every island-carrying function of a TU."""
    completion, _G, inlineasm = _engine()
    if (ROOT / "src" / f"{target}.c").is_file():
        text = (ROOT / "src" / f"{target}.c").read_text(encoding="utf-8")
        names = inlineasm.definition_names(text)
        out = []
        for f in names:
            if len(inlineasm.func_definition_candidates(text, f)) > 1:
                raise SystemExit(f"gte_macro_rewrite: {f}: ambiguous C definition in src/{target}.c")
            try:
                if inlineasm._func_body_span(text, f) and completion.own_blocks(text, f):
                    out.append((f, target))
            except ValueError:
                continue
        return out
    q = json.loads((ROOT / "engine/queue.json").read_text(encoding="utf-8"))
    found = [(target, it["file"]) for it in q.get("items", []) if it["func"] == target][:1]
    ambiguous = []
    for p in sorted((ROOT / "src").rglob("*.c")):
        text = p.read_text(encoding="utf-8")
        if inlineasm._func_body_span(text, target) and not found:
            found = [(target, p.relative_to(ROOT / "src").with_suffix("").as_posix())]
        if len(inlineasm.func_definition_candidates(text, target)) > 1:
            ambiguous.append(p.relative_to(ROOT).as_posix())
    if ambiguous:
        raise SystemExit(f"gte_macro_rewrite: {target}: ambiguous C definition in "
                         + ", ".join(ambiguous))
    if found:
        return found
    raise SystemExit(f"gte_macro_rewrite: {target}: no such function or TU")


def score(res: Result) -> int | None:
    from engine import sandbox
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    path = OUT_DIR / f"{res.func}.c"
    path.write_bytes(candidate_text(res).encode("utf-8"))
    r = sandbox.sandbox_score(res.func, disable="all", strip_cheat_asm=True,
                              candidate=str(path), workdir="tmp/gte_macro_rewrite/sandbox")
    return r.get("score")


GRIND_LOCK = ROOT / "tmp/grind/grind.lock"


def _windows_pid_alive(pid: int) -> bool:
    """grind.ps1 writes a WINDOWS pid; from WSL it is checked through
    tasklist.exe (interop). Anything but a clean "no such pid" is live."""
    try:
        out = subprocess.run(["tasklist.exe", "/FI", f"PID eq {pid}", "/NH", "/FO", "CSV"],
                             capture_output=True, text=True, timeout=30).stdout
    except (OSError, subprocess.SubprocessError):
        return True
    return f'"{pid}"' in out or "INFO:" not in out


def grinder_running(lock: Path = GRIND_LOCK) -> int | None:
    """The live pid the Grinder's lock names (relative to the repo the tool
    runs in), or None. A lock that cannot be read or checked counts as live
    (fail closed; -1 when it names no pid)."""
    try:
        raw = lock.read_text(encoding="utf-8", errors="replace").strip().lstrip("﻿")
    except FileNotFoundError:
        return None
    except OSError:
        return -1
    if not raw.isdigit():
        return -1
    pid = int(raw)
    return pid if _windows_pid_alive(pid) else None


def run(target: str, write: bool = False, do_score: bool = True,
        header: str | None = None, force: bool = False) -> list[Result]:
    _c, G, _i = _engine()
    if write:
        pid = grinder_running()
        if pid is not None:
            raise SystemExit(f"gte_macro_rewrite: --write refused: the Grinder lock "
                             f"{GRIND_LOCK} names a live pid ({pid})")
    funcs = _functions(target)
    texts = {stem: (ROOT / "src" / f"{stem}.c").read_text(encoding="utf-8") for _f, stem in funcs}
    if header is None and len(funcs) > 1:
        # One header for the TU: the one that converts the most functions.
        tally = {h: sum(rewrite_function(texts[s], f, s, h).error is None for f, s in funcs)
                 for h in G.INLINE_HEADERS}
        header = max(G.INLINE_HEADERS, key=lambda h: (tally[h], h == "inline_o.h"))
    results = []
    for f, stem in funcs:
        res = rewrite_function(texts[stem], f, stem, header)
        if res.error is None and do_score:
            res.score = score(res)
            if write and res.score == 0:
                write_candidate(res, force)
        results.append(res)
    return results


def write_candidate(res: Result, force: bool = False, grind: Path = Path("memory/grind")) -> None:
    """Write res's candidate to <grind>/<func>/candidate.c; an existing file
    is kept (res.error says so) unless `force`."""
    dest = grind / res.func / "candidate.c"
    if dest.exists() and not force:
        res.error = f"{dest.as_posix()} exists (--force to overwrite)"
        return
    dest.parent.mkdir(parents=True, exist_ok=True)
    dest.write_bytes(candidate_text(res).encode("utf-8"))
    res.candidate = dest.as_posix()


def _report(res: Result) -> str:
    if res.error:
        return f"{res.func} ({res.stem}): REFUSED: {res.error}"
    n = sum(len(us) for _l, _t, us in res.units)
    tiers = [t for _l, t, _u in res.units]
    out = [f"{res.func} ({res.stem}) {res.header}: {len(res.units)} run(s) -> {n} call(s) "
           f"({tiers.count('statement-exact')} statement-exact, "
           f"{tiers.count('instruction-exact')} instruction-exact); score {res.score}"]
    for ln, tier, us in res.units:
        out.append(f"  L{ln} [{tier}] " + " ".join(f"{u.macro}({', '.join(u.args)});" for u in us))
    for a in res.ambiguities():
        out.append(f"  macro choice: {a}")
    for ln, c in sorted(res.dropped_comments):
        out.append(f"  dropped comment L{ln}: {' '.join(c.split())[:160]}")
    for ln, c in res.review_comments:
        out.append(f"  review comment L{ln}: {c}")
    if res.includes:
        out.append("  TU needs: " + "; ".join(res.includes))
    out.append(f"  candidate: {res.candidate or '(dry run) ' + str(OUT_DIR / (res.func + '.c'))}")
    return "\n".join(out)


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("target", nargs="+", help="function name(s) or TU id(s) (main/17AFC)")
    ap.add_argument("--write", action="store_true",
                    help="write score-0 candidates to memory/grind/<func>/candidate.c "
                         "(refused while the Grinder runs)")
    ap.add_argument("--force", action="store_true",
                    help="with --write: overwrite an existing candidate.c")
    ap.add_argument("--no-score", action="store_true", help="rewrite only, no sandbox")
    ap.add_argument("--header", choices=["inline_o.h", "inline_c.h"])
    ap.add_argument("--json", action="store_true")
    a = ap.parse_args(argv)
    os.chdir(ROOT)
    results = [r for t in a.target
               for r in run(t, a.write, not a.no_score, a.header, a.force)]
    if a.json:
        print(json.dumps([{"func": r.func, "file": r.stem, "header": r.header, "error": r.error,
                           "score": r.score, "candidate": r.candidate,
                           "runs": [{"line": ln, "tier": t, "calls": [
                               {"macro": u.macro, "args": u.args, "alternatives": u.alternatives}
                               for u in us]} for ln, t, us in r.units],
                           "dropped_comments": r.dropped_comments,
                           "review_comments": r.review_comments, "includes": r.includes}
                          for r in results], indent=2))
    else:
        print("\n".join(_report(r) for r in results))
    bad = [r for r in results if r.error or (r.score not in (0, None))]
    return 1 if bad else 0


if __name__ == "__main__":
    if os.name == "nt":
        sys.exit(_rerun_in_wsl(sys.argv[1:]))
    sys.exit(main(sys.argv[1:]))

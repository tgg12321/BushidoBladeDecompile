"""Header-exact PsyQ GTE macro units — the statements the sandbox scores as written.

Owner ruling 2026-09-25 (.claude/rules/inline-asm-policy.md § "Scorer ruling
(owner, 2026-09-25)"; record: docs/grind/decisions.md 2026-09-25 OWNER RULING —
scorer: header-exact GTE macro statements). The sandbox's cheat-stripping
removes every `__asm__` statement without a cop2 instruction. PsyQ's inline_o.h
writes each GTE macro as SEPARATE statements (`move $12,%0`, then the cop2
transfer), so a body that writes such a macro out verbatim lost the header's
own `move`/`nop` statements and the sandbox compiled a different program
(func_800288C8: oracle-green, sandbox 90).

A QUALIFYING UNIT, and nothing else, is kept by the sandbox strip:
  (A) a MAXIMAL run of `__asm__` statements, inline in the source, with only
      whitespace and comments between them, that is — statement for statement,
      in order — the complete expansion of ONE pinned macro below. Every
      statement matches the header in instruction text, operand constraints and
      clobber list; only separators (`;` / newline) and whitespace may differ.
      A macro parameter binds to one operand expression, the same at every use.
      A partial, reordered, extended or edited run is not a unit (`0($12)` for
      the header's `($12)` is an edit);
  (B) the expansion contains at least one cop2 instruction, so a standalone
      `gte_nop()` is never a unit;
  (C) recognition is against the pinned header text, never a grant hash.
Everything outside a unit is stripped exactly as before. Recognition changes
what the sandbox MEASURES only: `func_cheat_asm_count` (the completion gate) and
`strip_cheat_asm_file`'s default mode are untouched, so a unit's GPR-only
statements still count as cheat-asm for any function without a grant.

Conservative choices (each strips as before when in doubt):
  * the run is maximal, so two macros written back to back form one run that
    matches neither and is stripped whole (the ruling's "extra statement
    appended" negative);
  * the run's first statement must start a statement (preceded by `;`, `{`,
    `}` or a label's `:`), so an `if (c)` / `else` guarding only its first
    statement disqualifies it;
  * the statement must be `volatile`, as every header statement is;
  * no statement inside a comment, a string or a preprocessor directive counts;
  * the DMPSX placeholder substitution the ruling also allows is NOT
    implemented: no pinned macro carries a placeholder `.word`.

PINNED holds only the macros needed today (the LZC family of func_800288C8,
func_8002A458, func_8002CD58 and func_80018300). Each excerpt is the header's
own lines, byte for byte; `engine test` re-hashes them. Adding a macro means
adding its verbatim lines with the same provenance fields.
"""
from __future__ import annotations

import functools
import hashlib
import re

from . import inlineasm
from .inlineasm import cia

# --- the pinned header text ------------------------------------------------
# PsyQ Run-time Library Release 4.3 (the release the existing inline_c.h
# authorizations in inline_asm_canonical.txt pin), from the same two
# independent projects. Both files are byte-identical in each project, and are
# identical to PsyQ 4.0 (psyz's psyq400.tar.gz, sha256 5323204...731b4b, pinned
# by Xeeynamo/psyz@a438bda3 decomp/sdk/psyq400.tar.gz.sha256) and 4.5
# (ser-pounce/rood-reverse@11349608 include/psx/) except for the $PSLibId line.
# Excerpts are LF, exactly as the 4.3 files ship.
_SH = "github.com/shdecompilations/silent-hill-decomp@a1f407cb1ed0992997ace33a024e52b47001fdac"
_XG = "github.com/ladysilverberg/xenogears-decomp@54d7ef3e221578afc39d39f34fcd8c15ed83928c"

PINNED = (
    {
        "header": "inline_o.h",
        "pslibid": "$PSLibId: Run-time Library Release 4.3$",
        "source": f"{_SH} include/psyq/inline_o.h",
        "header_sha256": "76f28032e381a78a4c96347eeee753150cfb55b9f0f0be414fd5040bf4c6e47d",
        "second_copy": f"{_XG} include/psyq/inline_o.h (byte-identical, same sha256)",
        "macros": (
            ("gte_ldlzc", (207, 210),
             "f53faecd42a08306743074e152b72e61b450b6bb73f7db458dcfa606066bbe50",
             r'''#define gte_ldlzc(r1) { \
  __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory"); \
}
'''),
            ("gte_stlzc", (1074, 1077),
             "a41933674e7cd7656d48ca0d62f29cb4905886d651956b6062f54fffbb60e69a",
             r'''#define gte_stlzc(r1) { \
  __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory"); \
}
'''),
            ("gte_nop", (1095, 1097),
             "2cf13b80a6f637a275cf1e6b4a6e69e68b2b036134caa7107505762d6c74f52e",
             r'''#define gte_nop() { \
  __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); \
}
'''),
        ),
    },
    {
        "header": "gtemac.h",
        "pslibid": "$PSLibId: Run-time Library Release 4.3$",
        "source": f"{_SH} include/psyq/gtemac.h",
        "header_sha256": "9fe028fd2a187bed8147a67a2c98210b6f1663c2e05d2e36bb1243512c3357d5",
        "second_copy": f"{_XG} include/psyq/gtemac.h (byte-identical, same sha256)",
        "macros": (
            ("gte_Lzc", (174, 178),
             "9edfd76e1dbd612dbe6cf64dda66bf029f78e77f5c6c7053a134fb04a06e101d",
             "#define gte_Lzc(r1,r2)\t\t\t\t\t\t\\\n"
             "\t\t\t\t{\tgte_ldlzc(r1);\t\t\\\n"
             "\t\t\t\t\tgte_nop();\t\t\\\n"
             "\t\t\t\t\tgte_nop();\t\t\\\n"
             "\t\t\t\t\tgte_stlzc(r2);\t}\n"),
        ),
    },
)


def excerpt_sha256(text: str) -> str:
    return hashlib.sha256(text.encode("ascii")).hexdigest()


# --- parsing ---------------------------------------------------------------
_COP2_OPS = frozenset({"mtc2", "mfc2", "ctc2", "cfc2", "lwc2", "swc2", "c2", "cop2"})
_STRING_RE = re.compile(r'"((?:[^"\\\n]|\\.)*)"')
_OPERAND_RE = re.compile(r'\s*"([^"\\]*)"\s*\((.*)\)\s*$', re.S)
_CALL_RE = re.compile(r'([A-Za-z_]\w*)\s*\(')
_IDENT_RE = re.compile(r'[A-Za-z_]\w*$')
_VOLATILE_RE = re.compile(r'\b(?:volatile|__volatile__)\b')


class Stmt:
    """One asm statement, normalized: (volatile, instrs, outputs, inputs,
    clobbers). Operands are (constraint, expr) with expr whitespace-collapsed;
    in a macro expansion expr is ("param", name) or ("lit", text)."""
    __slots__ = ("volatile", "instrs", "outputs", "inputs", "clobbers")

    def __init__(self, volatile, instrs, outputs, inputs, clobbers):
        self.volatile, self.instrs = volatile, instrs
        self.outputs, self.inputs, self.clobbers = outputs, inputs, clobbers


def _split_top(s: str, sep: str) -> list[str]:
    """Split on `sep` outside string literals and parentheses."""
    out, depth, cur, i = [], 0, [], 0
    while i < len(s):
        c = s[i]
        if c == '"':
            m = _STRING_RE.match(s, i)
            if not m:
                raise ValueError("unterminated string")
            cur.append(m.group(0))
            i = m.end()
            continue
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
        if c == sep and depth == 0:
            out.append("".join(cur))
            cur = []
        else:
            cur.append(c)
        i += 1
    out.append("".join(cur))
    return out


def _unescape(s: str) -> str:
    return re.sub(r'\\(.)', lambda m: {"n": "\n", "t": "\t"}.get(m.group(1), m.group(1)), s)


def _norm_instr(piece: str) -> str:
    parts = piece.split(None, 1)
    return parts[0] if len(parts) == 1 else parts[0] + " " + "".join(parts[1].split())


def _templates(section: str) -> tuple[str, ...]:
    """The template section must be string literals only (adjacent literals
    concatenate). Instructions split at `;` / newline; whitespace normalized."""
    pos, text = 0, []
    for m in _STRING_RE.finditer(section):
        if section[pos:m.start()].strip():
            raise ValueError("non-literal template")
        text.append(m.group(1))
        pos = m.end()
    if section[pos:].strip() or not text:
        raise ValueError("non-literal template")
    body = _unescape("".join(text))
    return tuple(_norm_instr(p.strip()) for p in re.split(r"[;\n]", body) if p.strip())


def _operands(section: str) -> tuple[tuple[str, str], ...]:
    if not section.strip():
        return ()
    ops = []
    for item in _split_top(section, ","):
        m = _OPERAND_RE.match(item)
        if not m:
            raise ValueError(f"bad operand {item!r}")
        ops.append((m.group(1), " ".join(m.group(2).split())))
    return tuple(ops)


def _clobbers(section: str) -> tuple[str, ...]:
    if not section.strip():
        return ()
    out = []
    for item in _split_top(section, ","):
        m = _STRING_RE.fullmatch(item.strip())
        if not m:
            raise ValueError(f"bad clobber {item!r}")
        out.append(m.group(1))
    return tuple(out)


def parse_asm(keyword_text: str, body: str) -> Stmt:
    """Parse one `__asm__ [volatile] ( body )`. Raises ValueError on any shape
    this module does not understand — the caller treats that as not-a-unit."""
    sections = _split_top(body, ":")
    if len(sections) > 4:
        raise ValueError("too many asm sections")
    sections += [""] * (4 - len(sections))
    return Stmt(bool(_VOLATILE_RE.search(keyword_text)), _templates(sections[0]),
                _operands(sections[1]), _operands(sections[2]), _clobbers(sections[3]))


def _parse_macro(excerpt: str, known: dict) -> tuple[str, list[str], list[Stmt]]:
    """Expand one pinned `#define NAME(params) { ... }` into its statements.
    Calls to other pinned macros are expanded through `known`."""
    logical = re.sub(r"\\\n", " ", excerpt).strip()
    m = re.match(r"#define\s+([A-Za-z_]\w*)\(([^)]*)\)\s*\{(.*)\}\s*$", logical, re.S)
    if not m:
        raise ValueError(f"unparseable macro excerpt: {excerpt[:40]!r}")
    name, body = m.group(1), m.group(3)
    params = [p.strip() for p in m.group(2).split(",") if p.strip()]
    stmts: list[Stmt] = []
    i = 0
    while True:
        while i < len(body) and body[i] in " \t\n;":
            i += 1
        if i >= len(body):
            break
        am = cia.ASM_KEYWORD_RE.match(body, i)
        if am:
            close = inlineasm._match_paren(body, am.end() - 1)
            st = parse_asm(body[am.start():am.end() - 1], body[am.end():close - 1])
            bind = lambda ops: tuple(
                (c, ("param", e) if e in params else ("lit", e)) for c, e in ops)
            st.outputs, st.inputs = bind(st.outputs), bind(st.inputs)
            stmts.append(st)
            i = close
            continue
        cm = _CALL_RE.match(body, i)
        if not cm or cm.group(1) not in known:
            raise ValueError(f"{name}: unexpected text {body[i:i + 30]!r}")
        close = inlineasm._match_paren(body, cm.end() - 1)
        args = [" ".join(a.split()) for a in _split_top(body[cm.end():close - 1], ",")
                if a.strip()]
        callee_params, callee_stmts = known[cm.group(1)]
        if len(args) != len(callee_params):
            raise ValueError(f"{name}: arity mismatch calling {cm.group(1)}")
        sub = {p: (("param", a) if a in params else ("lit", a))
               for p, a in zip(callee_params, args)}
        for st in callee_stmts:
            re_bind = lambda ops: tuple(
                (c, sub[e[1]] if e[0] == "param" else e) for c, e in ops)
            stmts.append(Stmt(st.volatile, st.instrs, re_bind(st.outputs),
                              re_bind(st.inputs), st.clobbers))
        i = close
    return name, params, stmts


@functools.lru_cache(maxsize=1)
def pinned_macros() -> dict[str, tuple[list[str], list[Stmt]]]:
    """name -> (params, expanded statements), in pin order (callees first)."""
    known: dict[str, tuple[list[str], list[Stmt]]] = {}
    for hdr in PINNED:
        for name, _lines, _sha, text in hdr["macros"]:
            got, params, stmts = _parse_macro(text, known)
            if got != name:
                raise ValueError(f"pinned excerpt defines {got}, not {name}")
            known[name] = (params, stmts)
    return known


def _is_cop2(instr: str) -> bool:
    return (instr.split(" ", 1)[0].lower() in _COP2_OPS
            or bool(cia.CANONICAL_DOTWORD_RE.search(instr)))


# --- recognition in a source file ------------------------------------------

def _preprocessor_ranges(masked: str) -> list[tuple[int, int]]:
    """Spans of preprocessor logical lines (with backslash continuations)."""
    ranges, pos = [], 0
    lines = masked.split("\n")
    i = 0
    while i < len(lines):
        start = pos
        if lines[i].lstrip().startswith("#"):
            while lines[i].rstrip().endswith("\\") and i + 1 < len(lines):
                pos += len(lines[i]) + 1
                i += 1
            ranges.append((start, pos + len(lines[i])))
        pos += len(lines[i]) + 1
        i += 1
    return ranges


def _source_statements(text: str):
    """(start, end, Stmt|None) for every real `__asm__` STATEMENT, in order.
    (start, end) is exactly the span `inlineasm._strip_spans` deletes; end
    includes the terminating `;`, which a statement must have."""
    masked = inlineasm._code_without_comments_and_strings(text)
    pp = _preprocessor_ranges(masked)
    out = []
    for m in cia.ASM_KEYWORD_RE.finditer(text):
        if masked[m.start():m.end()] != text[m.start():m.end()]:
            continue  # inside a comment or string
        if any(s <= m.start() < e for s, e in pp):
            continue
        close = inlineasm._match_paren(text, m.end() - 1)
        if close < 0:
            continue
        end = close
        while end < len(text) and text[end] in " \t":
            end += 1
        if end >= len(text) or text[end] != ";":
            continue
        try:
            st = parse_asm(text[m.start():m.end() - 1], text[m.end():close - 1])
        except ValueError:
            st = None
        out.append((m.start(), end + 1, st))
    return out, masked


def _matches(run: list[Stmt], params: list[str], spec: list[Stmt]) -> bool:
    if len(run) != len(spec):
        return False
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

    for s, h in zip(run, spec):
        if not s.volatile or s.instrs != h.instrs or s.clobbers != h.clobbers:
            return False
        if not ops_ok(s.outputs, h.outputs) or not ops_ok(s.inputs, h.inputs):
            return False
    return True


def match_unit(run: list[Stmt]) -> str | None:
    """Name of the pinned macro `run` is the complete expansion of, or None.
    Applies (B): a macro with no cop2 instruction is never a unit."""
    for name, (params, spec) in pinned_macros().items():
        if not any(_is_cop2(x) for st in spec for x in st.instrs):
            continue
        if _matches(run, params, spec):
            return name
    return None


def unit_spans(text: str) -> tuple[tuple[int, int, str], ...]:
    """(start, end, macro) for every asm statement inside a qualifying unit."""
    return _unit_spans_cached(text)


@functools.lru_cache(maxsize=8)
def _unit_spans_cached(text: str) -> tuple[tuple[int, int, str], ...]:
    stmts, masked = _source_statements(text)
    runs: list[list[tuple[int, int, Stmt | None]]] = []
    for st in stmts:
        if runs and not masked[runs[-1][-1][1]:st[0]].strip():
            runs[-1].append(st)
        else:
            runs.append([st])
    kept = []
    for run in runs:
        if any(st is None for _s, _e, st in run):
            continue
        before = masked[:run[0][0]].rstrip()
        if not before or before[-1] not in ";{}:":
            continue  # not at a statement boundary (if/else/for guard, file scope)
        name = match_unit([st for _s, _e, st in run])
        if name:
            kept.extend((s, e, name) for s, e, _st in run)
    return tuple(kept)

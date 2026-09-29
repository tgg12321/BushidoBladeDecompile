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
  (A) a run of `__asm__` statements, inline in the source, with only
      whitespace and comments between them, that is — statement for statement,
      in order — the complete expansion of ONE pinned macro below. Every
      statement matches the header in instruction text, operand constraints and
      clobber list; only separators (`;` / newline) and whitespace may differ.
      A macro parameter binds to one operand expression, the same at every use.
      A partial, reordered, extended or edited run is not a unit. One spelling
      is equal by owner amendment (2026-09-25, second batch): `0($REG)` and
      `($REG)` in a load/store's memory operand (maspsx cannot parse `($12)`);
      `4($12)`, `0x0($12)`, `00($12)`, `-0($12)`, `0($13)` stay edits. A
      second is equal by owner ruling 2026-09-26 (func_8002DE20, Q11; four
      more words by owner ruling 2026-09-28, func_800187F4, Q29): a header
      DMPSX placeholder `.word` and the post-DMPSX command word DMPSX emits for
      it, for the placeholders in DMPSX_WORDS only (each with an independent
      source);
  (B) the expansion contains at least one cop2 instruction (a DMPSX
      placeholder in DMPSX_WORDS counts: DMPSX turns it into one), so a
      standalone `gte_nop()` is never a unit;
  (C) recognition is against the pinned header text, never a grant hash.
A contiguous run of statements is kept when it splits, in order and with
nothing left over, into consecutive units (owner ruling 2026-09-26, Q11:
macros written back to back, e.g. `gte_ldv0(v); gte_rtv0();`). Any statement
of the run that no unit covers disqualifies the WHOLE run.
Everything outside a unit is stripped exactly as before. Recognition changes
what the sandbox MEASURES only: `func_cheat_asm_count` (the completion gate) and
`strip_cheat_asm_file`'s default mode are untouched, so a unit's GPR-only
statements still count as cheat-asm for any function without a grant.

Conservative choices (each strips as before when in doubt):
  * the run is split into units only when EVERY statement is covered, so a
    statement appended to, inserted into or left over from a macro strips the
    whole run (the ruling's "extra statement appended" negative); a
    standalone `gte_nop()` between two units is left over too;
  * the run's first statement must start a statement (preceded by `;`, `{`,
    `}` or a label's `:`), so an `if (c)` / `else` guarding only its first
    statement disqualifies it;
  * the statement must be `volatile`, as every header statement is;
  * no statement inside a comment, a string or a preprocessor directive counts.

PINNED holds only the macros needed today (the LZC family of func_800288C8,
func_8002A458, func_8002CD58 and func_80018300; gte_ldv0 / gte_rtv0 /
gte_stlvnl / gte_ApplyRotMatrix of func_8002DE20; gte_ldlvl / gte_lddp /
gte_rtv0tr / gte_sqr0 / gte_gpf0 / gte_gpl12 / gte_stlvl of func_800187F4,
owner ruling 2026-09-28, Q29). Each excerpt is the header's
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
            ("gte_ldv0", (16, 20),
             "5c1022524d6230c07244c6a1319d7e4d9a3edeff79ab2bb6b105dcc87d505e53",
             r'''#define gte_ldv0(r1) { \
  __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory"); \
}
'''),
            ("gte_rtv0", (426, 430),
             "fecc6d755f2e63f8e41f49bfa584b80d2324d2c581b54f7c3d687a8a3a841c69",
             r'''#define gte_rtv0() { \
  __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile (".word 0x0000013f": : :"$12","$13","$14","$15","memory"); \
}
'''),
            ("gte_stlvnl", (904, 909),
             "3098a5b8d2f9339f7ad7ceb289180c5231635922a28217fb97ec7b52dfd38b83",
             r'''#define gte_stlvnl(r1) { \
  __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory"); \
}
'''),
            ("gte_ldlvl", (104, 109),
             "c55a4a83e31289451a2576ef9ced83eb485fb95c9555719e896c8a8b3c902039",
             r'''#define gte_ldlvl(r1) { \
  __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("lwc2  $9,($12)": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("lwc2  $10,4($12)": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("lwc2  $11,8($12)": : :"$12","$13","$14","$15","memory"); \
}
'''),
            ("gte_lddp", (144, 147),
             "d03aad9f9558785d758e3940222274120df736e32150515b9dc07f418a204bd5",
             r'''#define gte_lddp(r1) { \
  __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("mtc2  $12,$8": : :"$12","$13","$14","$15","memory"); \
}
'''),
            ("gte_rtv0tr", (451, 455),
             "04d9cd65eea09c789e533b1e696b34c04c5f019e86063383f437023861e1865b",
             r'''#define gte_rtv0tr() { \
  __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile (".word 0x0000027f": : :"$12","$13","$14","$15","memory"); \
}
'''),
            ("gte_sqr0", (646, 650),
             "dcb3bf9cfded3dfb6d9aa817c43cc3c08a015c712113eed294dcce7556fbd6f7",
             r'''#define gte_sqr0() { \
  __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile (".word 0x00000f3f": : :"$12","$13","$14","$15","memory"); \
}
'''),
            ("gte_gpf0", (721, 725),
             "3e7ebfa85402b718a6f499db150427976a556f59477cb98268be84fa577510ff",
             r'''#define gte_gpf0() { \
  __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile (".word 0x000012ff": : :"$12","$13","$14","$15","memory"); \
}
'''),
            ("gte_gpl12", (726, 730),
             "ebcac8aa72cae4537f1c72d00544475026eff9aac7dd1c814e1e307a6e9834f6",
             r'''#define gte_gpl12() { \
  __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile (".word 0x0000133f": : :"$12","$13","$14","$15","memory"); \
}
'''),
            ("gte_stlvl", (898, 903),
             "0cb3538315ec14573014d815f95772af94c982d2e1ecc215c71bd04242016067",
             r'''#define gte_stlvl(r1) { \
  __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("swc2  $9,($12)": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("swc2  $10,4($12)": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("swc2  $11,8($12)": : :"$12","$13","$14","$15","memory"); \
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
            ("gte_ApplyRotMatrix", (354, 357),
             "4a4e57bca9f8d5dcea5758b07ce49c2ad31e47dcfb01d0caaf4349c0e20f9cee",
             "#define gte_ApplyRotMatrix(r1,r2)\t\t\t\t\\\n"
             "\t\t\t\t{\tgte_ldv0(r1);\t\t\\\n"
             "\t\t\t\t\tgte_rtv0();\t\t\\\n"
             "\t\t\t\t\tgte_stlvnl(r2);\t\t}\n"),
        ),
    },
)

# --- DMPSX placeholder command words ----------------------------------------
# Owner ruling 2026-09-26 (func_8002DE20, Q11): Sony's DMPSX post-processor
# rewrote each header placeholder `.word` into the GTE command word after
# compilation; this build has no DMPSX pass, so a unit may carry the post-DMPSX
# word where the header has the placeholder, and the two compare EQUAL.
# placeholder -> post-DMPSX word, only for placeholders whose word is shown by
# a source independent of the BB2 binary (inline-asm-policy.md § Extension
# 2026-09-24 (B)):
#   0x0000013f (gte_rtv0) -> 0x4A486012 = MVMVA sf=1 mx=rotation v=V0 cv=none
#   lm=0: pcsx-redux/nugget@22037bd3 psyq/include/inline_n.h :516-520 and
#   Lameguy64/PSn00bSDK@5d9aa2d3 libpsn00b/include/inline_c.h :1183-1186
#   ("cop2 0x0486012" = 0x4A486012).
# Owner ruling 2026-09-28 (func_800187F4, sixteenth batch, Q29; inline-asm-policy.md
# § Per-function grant: func_800187F4). Each word is 0x4A000000 | the `cop2 IMM`
# of both independent no-DMPSX sources, pcsx-redux/nugget@22037bd3
# psyq/include/inline_n.h and Lameguy64/PSn00bSDK@5d9aa2d3
# libpsn00b/include/inline_c.h (lines: nugget / PSn00bSDK):
#   0x0000027f (gte_rtv0tr) -> 0x4A480012 = MVMVA sf=1 mx=rotation v=V0 cv=TR lm=0
#     (:546-550 / :1208-1211, "cop2 0x0480012")
#   0x00000f3f (gte_sqr0)   -> 0x4AA00428 = SQR sf=0 lm=1
#     (:780-784 / :1404-1407, "cop2 0x0A00428")
#   0x000012ff (gte_gpf0)   -> 0x4B90003D = GPF sf=0 lm=0
#     (:870-874 / :1516-1519, "cop2 0x0190003D")
#   0x0000133f (gte_gpl12)  -> 0x4BA8003E = GPL sf=1 lm=0
#     (:876-880 / :1521-1524, "cop2 0x01A8003E")
# Scoring only: admission of an island carrying a post-DMPSX word still needs its
# own per-function owner grant (class prong (C) is unchanged).
DMPSX_WORDS = {
    0x0000013F: 0x4A486012,
    0x0000027F: 0x4A480012,
    0x00000F3F: 0x4AA00428,
    0x000012FF: 0x4B90003D,
    0x0000133F: 0x4BA8003E,
}


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


# Owner scorer amendment 2026-09-25 (second batch, inline-asm-policy.md § "Scorer
# amendment ... `0(reg)` equals `(reg)`"): maspsx cannot parse the header's
# `($12)`, so a memory operand written `0($REG)` compares equal to `($REG)`.
# Only the single character `0` before `($REG)`, only in the memory operand
# (last operand) of a load/store, never in constraints, clobbers or expressions.
_MEM_OPS = frozenset({"lb", "lbu", "lh", "lhu", "lw", "lwl", "lwr", "sb", "sh",
                      "sw", "swl", "swr", "lwc1", "swc1", "lwc2", "swc2"})
_ZERO_OFFSET_RE = re.compile(r"0(\(\$[A-Za-z0-9_]+\))")


_DOTWORD_RE = re.compile(r"\.word 0x([0-9A-Fa-f]{8})")
_POST_DMPSX = {post: ph for ph, post in DMPSX_WORDS.items()}


def _instr_key(instr: str) -> str:
    """Comparison key for a normalized instruction: `0($REG)` -> `($REG)` in a
    load/store's memory operand, and a post-DMPSX command word (`.word
    0x4A486012`, exactly eight hex digits) -> the header's placeholder
    spelling (`.word 0x0000013f`) for the pairs in DMPSX_WORDS; every other
    character compares as written."""
    m = _DOTWORD_RE.fullmatch(instr)
    if m and int(m.group(1), 16) in _POST_DMPSX:
        return f".word 0x{_POST_DMPSX[int(m.group(1), 16)]:08x}"
    mnem, sep, ops = instr.partition(" ")
    if mnem not in _MEM_OPS or not sep:
        return instr
    head, comma, last = ops.rpartition(",")
    m = _ZERO_OFFSET_RE.fullmatch(last)
    return f"{mnem} {head}{comma}{m.group(1)}" if m else instr


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
    """A cop2 instruction, a GTE command word, or a header DMPSX placeholder in
    DMPSX_WORDS (DMPSX turns it into a GTE command word)."""
    m = _DOTWORD_RE.fullmatch(instr)
    return (instr.split(" ", 1)[0].lower() in _COP2_OPS
            or bool(cia.CANONICAL_DOTWORD_RE.search(instr))
            or bool(m and int(m.group(1), 16) in DMPSX_WORDS))


# --- recognition in a source file ------------------------------------------

def _preprocessor_ranges(masked: str) -> list[tuple[int, int]]:
    """Spans of preprocessor logical lines (with backslash continuations)."""
    ranges, pos = [], 0
    lines = masked.split("\n")
    i = 0
    while i < len(lines):
        start = pos
        if cia.is_directive_lead(lines[i]):
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
    # The same keyword finder inlineasm._strip_spans uses, so the spans agree.
    for kw in cia.find_asm_keywords(text):
        if kw.directive or kw.end < 0:
            continue
        if any(s <= kw.start < e for s, e in pp):
            continue
        close = kw.end
        end = close
        while end < len(text) and text[end] in " \t":
            end += 1
        if end >= len(text) or text[end] != ";":
            continue
        raw = text[kw.start:close]
        try:
            # Parsed from the RAW text: a comment or backslash-newline anywhere
            # in the statement is not header-exact (the pinned statements
            # carry neither), so it is never part of a unit.
            if not cia.is_plain_code(raw):
                raise ValueError("comment or line splice inside the statement")
            st = parse_asm(text[kw.start:kw.paren], text[kw.paren + 1:close - 1])
        except ValueError:
            st = None
        out.append((kw.start, end + 1, st))
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
        if (not s.volatile or s.clobbers != h.clobbers
                or tuple(map(_instr_key, s.instrs)) != tuple(map(_instr_key, h.instrs))):
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
        for (i, j), name in split_units([st for _s, _e, st in run]):
            kept.extend((s, e, name) for s, e, _st in run[i:j])
    return tuple(kept)


def split_units(run: list[Stmt]) -> list[tuple[tuple[int, int], str]]:
    """Split `run` into consecutive qualifying units covering EVERY statement
    (owner ruling 2026-09-26, Q11: macros written back to back). Returns
    [((start, end), macro), ...] with the fewest units (so a whole composite
    such as gte_ApplyRotMatrix wins over its three callees), or [] when some
    statement is left over — then nothing in the run is a unit."""
    longest = max(len(spec) for _p, spec in pinned_macros().values())
    n = len(run)
    best: list[tuple[int, int, str] | None] = [None] * (n + 1)  # (units, prev, name)
    best[0] = (0, -1, "")
    for j in range(1, n + 1):
        for i in range(max(0, j - longest), j):
            if best[i] is None:
                continue
            name = match_unit(run[i:j])
            if name and (best[j] is None or best[i][0] + 1 < best[j][0]):
                best[j] = (best[i][0] + 1, i, name)
    if best[n] is None:
        return []
    out, j = [], n
    while j > 0:
        _k, i, name = best[j]
        out.append(((i, j), name))
        j = i
    return out[::-1]

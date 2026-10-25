"""Header-exact PsyQ GTE macro units — the statements the sandbox scores as written.

Owner ruling 2026-09-25 (.claude/rules/inline-asm-policy.md § "Scorer ruling
(owner, 2026-09-25)"; record: docs/grind/decisions.md 2026-09-25 OWNER RULING —
scorer: header-exact GTE macro statements). The sandbox's cheat-stripping
removes every `__asm__` statement without a cop2 instruction. PsyQ's inline_o.h
writes each GTE macro as SEPARATE statements (`move $12,%0`, then the cop2
transfer), so a body that writes such a macro out verbatim lost the header's
own `move`/`nop` statements and the sandbox compiled a different program
(fighter_ResolveBodyOverlap: oracle-green, sandbox 90).

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
      it, for the placeholders in UNIT_DMPSX_WORDS only (each word cited and
      field-decoded; see DMPSX_WORDS);
  (B) the expansion contains at least one cop2 instruction (a DMPSX
      placeholder in UNIT_DMPSX_WORDS counts: DMPSX turns it into one), so a
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

PINNED holds only the macros needed today (the LZC family of fighter_ResolveBodyOverlap,
fighter_CastWeaponSegment, math_PrepareTriangleCollisionFrame and func_80018300; gte_ldv0 / gte_rtv0 /
gte_stlvnl / gte_ApplyRotMatrix of func_8002DE20; gte_ldlvl / gte_lddp /
gte_rtv0tr / gte_sqr0 / gte_gpf0 / gte_gpl12 / gte_stlvl of func_800187F4,
owner ruling 2026-09-28, Q29; gte_ldlv0 / gte_SetRotMatrix of the
math_ScaleVectorInDirectionFrame / func_8002F2D0 / func_8002F770 cluster, pinned as the 2026-09-26
class grant's (A) allows: an engine: commit with layer-2). Each excerpt is the header's
own lines, byte for byte; `engine test` re-hashes them. Adding a macro means
adding its verbatim lines with the same provenance fields.

VENDORED HEADERS (owner decisions 2026-10-15: R1 vendor whole, R2 DMPSX option
B, R3 class grant): Sony's inline_o.h, inline_c.h and gtemac.h live whole in
include/psxsdk/, and SN's libsn.h (pollhost()) beside them. A C body CALLS
their macros; cpp is the only expander and the header the only source of asm
text. VENDORED pins each file, DMPSX_WORDS holds
the eleven owner-approved placeholder -> command word pairs,
check_vendored() proves each file is Sony's text with only those words
substituted, header_macros() parses every function-like #define, and
call_sites() finds the calls in a body (engine.completion treats each as an
island). The written-out unit recognition above stays until the last
written-out island is gone.
"""
from __future__ import annotations

import bisect
import functools
import hashlib
import re
from pathlib import Path
from typing import NamedTuple

from . import inlineasm
from .inlineasm import cia

# Every repo-relative read resolves from the repo root, not the cwd.
REPO_ROOT = Path(__file__).resolve().parent.parent

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
            ("gte_ldlv0", (95, 103),
             "25eddbcf07dae9b935b63070c470eeb63cdd501580cd06f2f2ffd38f1373b431",
             r'''#define gte_ldlv0(r1) { \
  __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("lhu   $14,4($12)": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("lhu   $13,($12)": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("sll   $14,$14,16": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("or    $13,$13,$14": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("mtc2  $13,$0": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("lwc2  $1,8($12)": : :"$12","$13","$14","$15","memory"); \
}
'''),
            ("gte_SetRotMatrix", (272, 284),
             "1b3eac43829471d66b4599ad7b226ad27e8bde823bf1111d41e1592158ddd803",
             r'''#define gte_SetRotMatrix(r1) { \
  __asm__ volatile ("move  $12,%0": :"r"(r1):"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("lw    $13,($12)": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("lw    $14,4($12)": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("ctc2  $13,$0": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("ctc2  $14,$1": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("lw    $13,8($12)": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("lw    $14,12($12)": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("lw    $15,16($12)": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("ctc2  $13,$2": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("ctc2  $14,$3": : :"$12","$13","$14","$15","memory"); \
  __asm__ volatile ("ctc2  $15,$4": : :"$12","$13","$14","$15","memory"); \
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
# of the no-DMPSX headers pcsx-redux/nugget@22037bd3 psyq/include/inline_n.h
# and Lameguy64/PSn00bSDK@5d9aa2d3 libpsn00b/include/inline_c.h (derived from
# nugget's; DMPSX_WORDS names the independent checks) (lines: nugget /
# PSn00bSDK):
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
# These five are the written-out-unit equalities only; the vendored headers use
# DMPSX_WORDS below. Retires with the written-out machinery.
UNIT_DMPSX_WORDS = {
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
# cia.ASM_KEYWORD_RE plus bare `asm` (LIBSN.H writes `asm("break 1024")`). Only
# for pinned / vendored header text, where `asm` cannot be a register pin.
_HEADER_ASM_RE = re.compile(
    r'\b(__asm__|__asm|asm)\s*(?:volatile|__volatile__|__volatile|const|__const__|__const)?\s*\(')


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
_POST_DMPSX = {post: ph for ph, post in UNIT_DMPSX_WORDS.items()}


def _instr_key(instr: str) -> str:
    """Comparison key for a normalized instruction: `0($REG)` -> `($REG)` in a
    load/store's memory operand, and a post-DMPSX command word (`.word
    0x4A486012`, exactly eight hex digits) -> the header's placeholder
    spelling (`.word 0x0000013f`) for the pairs in UNIT_DMPSX_WORDS; every other
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
        am = _HEADER_ASM_RE.match(body, i)
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
    UNIT_DMPSX_WORDS (DMPSX turns it into a GTE command word)."""
    m = _DOTWORD_RE.fullmatch(instr)
    return (instr.split(" ", 1)[0].lower() in _COP2_OPS
            or bool(cia.CANONICAL_DOTWORD_RE.search(instr))
            or bool(m and int(m.group(1), 16) in UNIT_DMPSX_WORDS))


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


# --- vendored PsyQ headers --------------------------------------------------
# Owner decision 2026-10-15 (R1): Sony's PsyQ 4.0 text (the BB2 libraries are
# 4.0, AGENTS.md), whole, Sony's copyright header kept, from psyz's pinned SDK
# archive. Two mechanical differences from the shipped bytes, both reversed by
# check_vendored(): DOS line ends become LF (build files are LF), and DMPSX's
# command word stands where DMPSX_WORDS has a placeholder (R2, option B).
_PSYQ400 = ("github.com/psyqz/psyz/releases/download/requirements/psyq400.tar.gz"
            " (sha256 532320403f0da881e15d537c74e2adb3a3ad9c0153bbd8d27f435b4ae9731b4b,"
            " pinned by github.com/Xeeynamo/psyz@a438bda3 decomp/sdk/psyq400.tar.gz.sha256)")
_PSLIBID_40 = "$PSLibId: Run-time Library Release 4.0$"

# Admission is automatic for any header listed here: every macro whose
# expansion holds a cop2 or a trap instruction becomes callable
# (Macro.admitted). Adding a header is therefore an engine: commit with
# layer-2, plus an owner ruling when its macros carry a trap (break /
# syscall), as for a new DMPSX word (.claude/rules/inline-asm-policy.md rule 1).
VENDORED = {
    "include/psxsdk/inline_o.h": {
        "pslibid": _PSLIBID_40,
        "source": f"{_PSYQ400} INCLUDE/INLINE_O.H",
        "sony_sha256": "715a9f5b1e411116f279a3f33060416b1cc1e29efd195759d2537dc3281caafc",
        "second_copy": f"{_SH} and {_XG} include/psyq/inline_o.h (Release 4.3,"
                       " sha256 76f28032...c6e47d): equal but for the $PSLibId line",
        "dmpsx": True,
    },
    "include/psxsdk/inline_c.h": {
        "pslibid": _PSLIBID_40,
        "source": f"{_PSYQ400} INCLUDE/INLINE_C.H",
        "sony_sha256": "2910a9972feac80b817d02b4d3a93a761d9ca47f0de16070b5bd7674fab9a71c",
        "second_copy": f"{_SH} include/psyq/inline_c.h (Release 4.3, sha256"
                       " de1a70ef...aa02f8): equal but for the $PSLibId line,"
                       " gte_ldsvrtrow0's input list (4.0 also names an undeclared"
                       " r1) and 4.3's added gte_mvmva_core / gte_mvmva",
        "dmpsx": True,
    },
    "include/psxsdk/gtemac.h": {
        "pslibid": _PSLIBID_40,
        "source": f"{_PSYQ400} INCLUDE/GTEMAC.H",
        "sony_sha256": "15b007e98f3b004ebaf396aa11f87f4c03728e244754feb0ed33e624c2b80fde",
        "second_copy": f"{_SH} and {_XG} include/psyq/gtemac.h (Release 4.3,"
                       " sha256 9fe028fd...3357d5): equal but for the $PSLibId line",
        "dmpsx": False,
    },
    # SN Systems' LIBSN.H: pollhost() / PSYQpause() (`break 1024` / `break
    # 1031`) and the PCdrv prototypes. It replaced a BB2-written libsn.h.
    "include/psxsdk/libsn.h": {
        "pslibid": _PSLIBID_40,
        "source": f"{_PSYQ400} INCLUDE/LIBSN.H",
        "sony_sha256": "713aa3dcf5bcb5fb75ac76fb731fcf4f07c2f54e89bcfeead1ca13fafc00ced9",
        "second_copy": f"{_SH} and {_XG} include/psyq/libsn.h (Release 4.3,"
                       " sha256 f1c0f803...c36817) and ser-pounce/rood-reverse@11349608"
                       " include/psx/libsn.h (Release 4.5, sha256 1bc50ed4...4b923c0f):"
                       " LF, equal but for the $PSLibId line",
        "dmpsx": False,
    },
}
INLINE_HEADERS = ("inline_o.h", "inline_c.h")
COMPOSITE_HEADER = "gtemac.h"
# A vendored header whose macros a TU calls with or without an inline header.
STANDALONE_HEADERS = ("libsn.h",)
VENDORED_BASENAMES = (*INLINE_HEADERS, COMPOSITE_HEADER, *STANDALONE_HEADERS)

# DMPSX placeholder -> (command word, macro, nugget lines, PSn00bSDK lines).
# Owner decision 2026-10-15 (R2, option B): the eleven placeholders the BB2 GTE
# code needs get their command word; every other placeholder stays as Sony
# shipped it, and a call of its macro is refused (Macro.placeholder: no GTE
# command). A new word is an engine: commit with layer-2 and an owner ruling.
# Each word is 0x4A000000 | the `cop2 IMM` of pcsx-redux/nugget@22037bd3
# psyq/include/inline_n.h. Lameguy64/PSn00bSDK@5d9aa2d3
# libpsn00b/include/inline_c.h writes the same IMM, but states its header
# derives from nugget's: a second copy, not a second source. The independent
# checks are each word's field decode against psx-spx (GTE Command Encoding:
# sf, mx, v, cv, lm, command) and byte identity in the BB2 oracle (every call
# site rebuilds SHA1 62efab4f...). The carriers are `macro` (inline_o.h and
# inline_c.h) and `macro_b` (inline_c.h); each line pair is (macro, macro_b) in
# that source.
DMPSX_WORDS = {
    0x0000007F: (0x4A180001, "gte_rtps", (498, 903), (1159, 1543)),
    0x000000BF: (0x4A280030, "gte_rtpt", (504, 904), (1173, 1544)),
    0x000000FF: (0x4A480012, "gte_rt", (510, 905), (1178, 1545)),
    0x0000013F: (0x4A486012, "gte_rtv0", (516, 906), (1183, 1546)),
    0x000001FF: (0x4A49E012, "gte_rtir", (534, 909), (1198, 1549)),
    0x0000027F: (0x4A480012, "gte_rtv0tr", (546, 911), (1208, 1551)),
    0x00000F3F: (0x4AA00428, "gte_sqr0", (780, 950), (1404, 1590)),
    0x0000127F: (0x4B70000C, "gte_op0", (858, 963), (1506, 1603)),
    0x000012BF: (0x4B98003D, "gte_gpf12", (864, 964), (1511, 1604)),
    0x000012FF: (0x4B90003D, "gte_gpf0", (870, 965), (1516, 1605)),
    0x0000133F: (0x4BA8003E, "gte_gpl12", (876, 966), (1521, 1606)),
}
_PLACEHOLDER = ".word 0x{:08x}"  # Sony's spelling
_COMMAND_WORD = ".word 0x{:08X}"  # the vendored word, as SOTN and psyz spell it


def _dmpsx_carriers() -> dict[str, tuple[int, int]]:
    """macro -> (placeholder, word) for every Sony macro DMPSX_WORDS covers."""
    out = {}
    for ph, (word, macro, _nugget, _psn00b) in DMPSX_WORDS.items():
        out[macro] = out[macro + "_b"] = (ph, word)
    return out


_DEFINE_RE = re.compile(r"[ \t]*#[ \t]*define[ \t]+([A-Za-z_]\w*)")


def _define_spans(text: str) -> list[tuple[int, int, str, int]]:
    """(start, end, name, line) of every `#define` logical line (backslash
    continuations joined); line is 1-based."""
    out, pos, i = [], 0, 0
    lines = text.split("\n")
    while i < len(lines):
        start, first = pos, i
        while lines[i].endswith("\\") and i + 1 < len(lines):
            pos += len(lines[i]) + 1
            i += 1
        m = _DEFINE_RE.match(lines[first])
        if m:
            out.append((start, pos + len(lines[i]), m.group(1), first + 1))
        pos += len(lines[i]) + 1
        i += 1
    return out


def _swap_words(text: str, to_word: bool) -> str:
    """Inside the #define of each macro DMPSX_WORDS covers, swap its
    placeholder for its command word (to_word) or back. Keyed by macro, so the
    reverse is unique although two placeholders share 0x4A480012 (gte_rt and
    gte_rtv0tr are the same GTE command)."""
    carriers = _dmpsx_carriers()
    out, last = [], 0
    for s, e, name, _line in _define_spans(text):
        if name not in carriers:
            continue
        ph, word = carriers[name]
        old, new = _PLACEHOLDER.format(ph), _COMMAND_WORD.format(word)
        if not to_word:
            old, new = new, old
        out += [text[last:s], text[s:e].replace(old, new)]
        last = e
    out.append(text[last:])
    return "".join(out)


def dmpsx(text: str) -> str:
    """Sony's LF text -> the vendored text: each DMPSX_WORDS placeholder becomes
    its command word, as DMPSX would have rewritten the object."""
    return _swap_words(text, True)


def undmpsx(text: str) -> str:
    """The vendored text -> Sony's LF text (inverse of dmpsx)."""
    return _swap_words(text, False)


def check_vendored(root: str | Path = REPO_ROOT) -> list[str]:
    """Issues with the vendored headers: missing, a CR, non-ASCII, not Sony's
    text once LF and DMPSX are reversed, or an agreed placeholder left
    unsubstituted. [] when all four VENDORED headers are exactly the pinned
    files."""
    root = Path(root)
    stamps = []
    for rel in VENDORED:
        try:
            st = (root / rel).stat()
            stamps.append((st.st_mtime_ns, st.st_size))
        except OSError:
            stamps.append(None)
    return list(_check_vendored_cached(str(root), tuple(stamps)))


@functools.lru_cache(maxsize=4)
def _check_vendored_cached(root: str, _stamps: tuple) -> tuple[str, ...]:
    issues = []
    for rel, meta in VENDORED.items():
        try:
            data = (Path(root) / rel).read_bytes()
        except OSError:
            issues.append(f"{rel}: vendored PsyQ header missing")
            continue
        if b"\r" in data:
            issues.append(f"{rel}: carriage return in a vendored header (build files are LF)")
            continue
        try:
            text = data.decode("ascii")
        except UnicodeDecodeError:
            issues.append(f"{rel}: non-ASCII byte in a vendored header")
            continue
        sony = (undmpsx(text) if meta["dmpsx"] else text).replace("\n", "\r\n")
        if hashlib.sha256(sony.encode("ascii")).hexdigest() != meta["sony_sha256"]:
            issues.append(f"{rel}: not Sony's {meta['pslibid']} text (sha256 after"
                          f" reversing LF and DMPSX is not {meta['sony_sha256']})")
        elif meta["dmpsx"] and dmpsx(undmpsx(text)) != text:
            issues.append(f"{rel}: a DMPSX_WORDS placeholder is not substituted")
    return tuple(issues)


class Macro(NamedTuple):
    """One function-like #define of a vendored header, expanded."""
    header: str                  # basename of the file defining it
    line: int                    # 1-based line of its #define
    params: tuple[str, ...]
    stmts: tuple[Stmt, ...]      # asm statements, gtemac.h calls expanded
    has_cop2: bool               # a cop2 instruction or a GTE command word
    placeholder: bool            # an unsubstituted DMPSX placeholder remains
    has_trap: bool = False       # a trap instruction (_TRAP_OPS)

    @property
    def admitted(self) -> bool:
        """A call C cannot write: its expansion has a cop2 or a trap
        instruction. gte_nop() (a lone `nop`) is a scheduling barrier."""
        return self.has_cop2 or self.has_trap


# Trap instructions with a code field, which GCC never emits but for its
# divide guards (break 6/7): LIBSN.H's pollhost() / PSYQpause() are `break
# 1024` / `break 1031`.
_TRAP_OPS = frozenset({"break", "syscall"})
_BLOCK_COMMENT_RE = re.compile(r"/\*.*?\*/", re.S)


def _macro_defines(text: str):
    """(name, line, params, body) of every function-like #define; body is the
    replacement list with continuations joined and comments blanked."""
    out = []
    for s, e, name, line in _define_spans(text):
        logical = _BLOCK_COMMENT_RE.sub(" ", text[s:e].replace("\\\n", " "))
        m = re.match(r"\s*#\s*define\s+([A-Za-z_]\w*)\(([^)]*)\)(.*)$", logical, re.S)
        if m:
            params = tuple(p.strip() for p in m.group(2).split(",") if p.strip())
            out.append((name, line, params, m.group(3).strip()))
    return out


def _instr_flags(stmts) -> tuple[bool, bool, bool]:
    cop2 = placeholder = trap = False
    for st in stmts:
        for x in st.instrs:
            dw = _DOTWORD_RE.fullmatch(x)
            if dw and int(dw.group(1), 16) < 0x01000000:
                placeholder = True
            elif x.split(" ", 1)[0].lower() in _COP2_OPS or cia.CANONICAL_DOTWORD_RE.search(x):
                cop2 = True
            elif x.split(" ", 1)[0].lower() in _TRAP_OPS:
                trap = True
    return cop2, placeholder, trap


def _read_vendored(root: str, basename: str) -> str | None:
    p = Path(root) / "include/psxsdk" / basename
    try:
        st = p.stat()
    except OSError:
        return None
    return _read_cached(str(p), st.st_mtime_ns, st.st_size)


@functools.lru_cache(maxsize=16)
def _read_cached(path: str, _mtime: int, _size: int) -> str | None:
    try:
        return Path(path).read_text(encoding="ascii")
    except (OSError, UnicodeDecodeError):
        return None


def parse_header(text: str, header: str, known: dict | None = None):
    """Expand every function-like #define of one vendored file. `known` holds
    the macros calls may resolve to (gtemac.h composites call inline_o.h /
    inline_c.h macros). Returns ({name: Macro}, [(name, reason), ...]) — the
    second list is every #define that did not parse."""
    known = dict(known or {})
    macros, failed = {}, []
    for name, line, params, body in _macro_defines(text):
        excerpt = (f"#define {name}({', '.join(params)}) "
                   + (body if body.startswith("{") else "{ " + body + "; }"))
        try:
            _n, _p, stmts = _parse_macro(excerpt, {k: (list(m.params), list(m.stmts))
                                                   for k, m in known.items()})
        except ValueError as exc:
            failed.append((name, str(exc)))
            continue
        cop2, ph, trap = _instr_flags(stmts)
        macros[name] = known[name] = Macro(header, line, params, tuple(stmts), cop2, ph, trap)
    return macros, failed


def header_macros(inline: str, root: str | Path = REPO_ROOT) -> dict[str, Macro]:
    """Every macro a TU including `inline` (inline_o.h or inline_c.h) and
    gtemac.h can call: the inline header's own, then gtemac.h's composites
    resolved against it. A composite calling a macro the inline header lacks
    is left out (gtemac.h names a few inline_c.h-only macros)."""
    if inline not in INLINE_HEADERS:
        raise ValueError(f"not a vendored inline header: {inline}")
    root = str(root)
    texts = (_read_vendored(root, inline), _read_vendored(root, COMPOSITE_HEADER))
    return dict(_header_macros_cached(inline, *texts))


@functools.lru_cache(maxsize=4)
def _header_macros_cached(inline: str, inline_text, composite_text):
    if inline_text is None:
        return ()
    own, _failed = parse_header(inline_text, inline)
    out = dict(own)
    if composite_text is not None:
        comp, _failed = parse_header(composite_text, COMPOSITE_HEADER, own)
        out.update(comp)
    return tuple(out.items())


def standalone_macros(header: str, root: str | Path = REPO_ROOT) -> dict[str, Macro]:
    """Every macro of one STANDALONE_HEADERS file (libsn.h)."""
    if header not in STANDALONE_HEADERS:
        raise ValueError(f"not a standalone vendored header: {header}")
    return dict(_standalone_cached(header, _read_vendored(str(root), header)))


@functools.lru_cache(maxsize=4)
def _standalone_cached(header: str, text):
    return () if text is None else tuple(parse_header(text, header)[0].items())


def vendored_names(root: str | Path = REPO_ROOT) -> frozenset[str]:
    """Every function-like macro name the vendored files define."""
    return _names_cached(tuple(_read_vendored(str(root), b) for b in VENDORED_BASENAMES))


@functools.lru_cache(maxsize=4)
def _names_cached(texts: tuple) -> frozenset[str]:
    return frozenset(n for t in texts if t is not None for n, _l, _p, _b in _macro_defines(t))


# `#include <psxsdk/inline_o.h>`, `"psxsdk/inline_o.h"` or any path ending in a
# vendored basename (a directive's spelling never hides an inclusion).
_VENDORED_INCLUDE_RE = re.compile(
    r'^[ \t]*(?:#|%:)[ \t]*include[ \t]*[<"](?:[^>"\n]*/)?('
    + "|".join(map(re.escape, VENDORED_BASENAMES)) + r')[>"]', re.M)


@functools.lru_cache(maxsize=16)
def tu_includes(text: str) -> frozenset[str]:
    """Basenames of the vendored headers `text` #includes (comments ignored)."""
    if not any(b in text for b in VENDORED_BASENAMES):
        return frozenset()
    return frozenset(_VENDORED_INCLUDE_RE.findall(cia.compiler_view(text).code))


def tu_header(text: str) -> str | None:
    """The vendored inline header the TU includes, or None. Including both is
    an error: they define the same names with different bodies."""
    inc = tu_includes(text) & set(INLINE_HEADERS)
    if len(inc) > 1:
        raise ValueError("the TU includes both inline_o.h and inline_c.h")
    return next(iter(inc), None)


def tu_macros(text: str, root: str | Path = REPO_ROOT) -> dict[str, Macro]:
    """The vendored macros a call in this TU expands to: its inline header's,
    plus gtemac.h's composites when it includes gtemac.h too, plus those of
    each standalone header (libsn.h) it includes."""
    inline = tu_header(text)
    out = {}
    if inline is not None:
        composites = COMPOSITE_HEADER in tu_includes(text)
        out = {n: m for n, m in header_macros(inline, root).items()
               if composites or m.header != COMPOSITE_HEADER}
    for h in STANDALONE_HEADERS:
        if h in tu_includes(text):
            out.update(standalone_macros(h, root))
    return out


def call_header(text: str, name: str, root: str | Path = REPO_ROOT) -> str:
    """The header a call of `name` is keyed by in a region hash: a standalone
    header's own name, else the TU's inline header ('-' without one)."""
    m = tu_macros(text, root).get(name)
    if m is not None and m.header in STANDALONE_HEADERS:
        return m.header
    return tu_header(text) or "-"


_IDENT_RE = re.compile(r"\b[A-Za-z_]\w*")
_CONTROL_HEADS = frozenset({"if", "while", "for", "switch"})


def _code_view(text: str) -> tuple[str, tuple, list[tuple[int, int]]]:
    """(masked, pos, preprocessor ranges) of the compiler view: comments and
    literals blanked, backslash-newlines spliced out (`gte_n\\<NL>op` is one
    token, as cc1 sees it); pos maps a view index back to `text`."""
    view = cia.compiler_view(text)
    return view.masked, view.pos, _preprocessor_ranges(view.masked)


def _in_ranges(i: int, ranges: list[tuple[int, int]]) -> int:
    """Start of the range holding `i`, or -1."""
    k = bisect.bisect_right(ranges, (i, float("inf"))) - 1
    return ranges[k][0] if k >= 0 and i < ranges[k][1] else -1


def _skip_back(masked: str, k: int, pp: list[tuple[int, int]]) -> int:
    """Index of the last code char at or before `k`, over whitespace and
    preprocessor lines (`#ifdef X` before a call is not the previous token)."""
    while k >= 0:
        if masked[k] in " \t\n":
            k -= 1
            continue
        s = _in_ranges(k, pp)
        if s < 0:
            return k
        k = s - 1
    return k


def _vendored_prefixes(names: frozenset[str]) -> frozenset[str]:
    return frozenset(n[:4] for n in names)


def _statement_position(masked: str, i: int, pp: list[tuple[int, int]] = ()) -> bool:
    """True when the token at `i` starts a statement: after `;`, `{`, `}`, a
    label's `:`, `else`, `do`, or the `)` closing an if/while/for head.
    Preprocessor lines in `pp` are skipped."""
    k = _skip_back(masked, i - 1, pp)
    if k < 0:
        return False
    if masked[k] in ";{}:":
        return True
    heads = ("else", "do")
    if masked[k] == ")":
        depth = 0
        while k >= 0:
            if masked[k] == ")":
                depth += 1
            elif masked[k] == "(":
                depth -= 1
                if depth == 0:
                    break
            k -= 1
        k = _skip_back(masked, k - 1, pp)
        heads = _CONTROL_HEADS
    m = re.search(r"(?<![\w])([A-Za-z_]\w*)$", masked[max(0, k - 16):k + 1])
    return bool(m) and m.group(1) in heads


def _body_open(text: str, span: tuple[int, int]) -> int:
    """Index of the `{` opening the body `span` ends with."""
    i = text.find("{", span[0])
    while i != -1 and i < span[1]:
        if inlineasm._match_brace(text, i) == span[1]:
            return i
        i = text.find("{", i + 1)
    return span[0]


def call_sites(text: str, func: str, root: str | Path = REPO_ROOT,
               span: tuple[int, int] | None = None) -> list[tuple[int, int, str]]:
    """(start, end, macro) for every call of a vendored macro in `func`'s body,
    end just past its `;`. Every TU is scanned, whether or not it includes a
    vendored header itself (a game header or a computed #include can bring
    one in; call_issues refuses the call then). Found in the compiler view:
    comments, strings and preprocessor lines never count, and a
    backslash-newline inside a name does not hide it. A call must be a whole
    statement — raises ValueError otherwise (fail closed), as for a call not
    followed by `;`, one nested in another's arguments, and a vendored name
    not directly followed by `(` (`(gte_nop)()`, `CALL(gte_nop)`: a macro
    reached through another name is no call the gates can see). `span`: the
    definition span to scan instead of locating `func`'s (a Q126 helper found
    in the compiler view, engine.inlineasm.attributed_spans)."""
    if span is None:
        candidates = inlineasm.func_definition_candidates(text, func)
        if len(candidates) > 1:
            raise ValueError(f"{func}: ambiguous C definition: "
                             f"{len(candidates)} definition-shaped heads")
        span = candidates[0] if candidates else None
    names = vendored_names(root)
    if span is None or not names:
        return []
    masked, pos, pp = _code_view(text)
    if not any(p in masked for p in _vendored_prefixes(names)):
        return []
    lo = bisect.bisect_left(pos, _body_open(text, span))
    hi = bisect.bisect_left(pos, span[1])
    out = []
    for m in _IDENT_RE.finditer(masked, lo, hi):
        name = m.group(0)
        if name not in names or _in_ranges(m.start(), pp) >= 0:
            continue
        j = m.end()
        while j < hi and masked[j] in " \t\n":
            j += 1
        if j >= hi or masked[j] != "(":
            raise ValueError(f"{func}: vendored macro {name} is named without a direct "
                             f"call (`{name}(` expected)")
        close = inlineasm._match_paren(masked, j)
        if close < 0 or close > hi:
            raise ValueError(f"{func}: unclosed call of {name}")
        j = close
        while j < hi and masked[j] in " \t\n":
            j += 1
        if j >= hi or masked[j] != ";":
            raise ValueError(f"{func}: {name}(...) is not a statement (no `;` follows the call)")
        if not _statement_position(masked, m.start(), pp):
            raise ValueError(f"{func}: {name}(...) is not at statement position")
        out.append((pos[m.start()], pos[j] + 1, name))
    return out


_ARG_ASM_RE = re.compile(r"\b(?:__asm__|__asm|asm)\b|\(\s*\{")


def call_issues(text: str, func: str, root: str | Path = REPO_ROOT,
                span: tuple[int, int] | None = None) -> list[str]:
    """Why `func`'s vendored macro calls cannot be admitted, [] when they can:
    a header that is not Sony's text, a macro the TU's header does not define
    (wrong header, or gtemac.h without an inline header), a macro that is not
    Macro.admitted (`gte_nop()` is a scheduling barrier by name) or with a DMPSX
    placeholder left, an asm keyword or a GNU statement-expression `({` in the
    arguments, a call in a TU that does not #include a vendored header itself,
    and a source-file #define / #undef of a vendored name.
    call_sites / tu_header errors propagate as ValueError."""
    calls = call_sites(text, func, root, span)
    if not calls:
        return []
    issues = [f"vendored header: {i}" for i in check_vendored(root)]
    if not tu_includes(text):
        issues.append("vendored macro called but TU does not include its header ("
                      + ", ".join(sorted({n for _s, _e, n in calls})) + ")")
    macros = tu_macros(text, root)
    where = tu_header(text) or "no inline header"
    for s, e, name in calls:
        if _ARG_ASM_RE.search(cia.compiler_view(text[s:e]).masked):
            issues.append(f"call of {name}: an asm keyword or a statement-expression "
                          f"`({{` in its arguments")
        m = macros.get(name)
        if not tu_includes(text):
            continue
        if m is None:
            issues.append(f"call of {name}: not defined by the TU's vendored headers ({where})")
        elif m.placeholder:
            issues.append(f"call of {name}: its expansion keeps a DMPSX placeholder "
                          f"(no agreed command word)")
        elif not m.admitted:
            issues.append(f"call of {name}: its expansion has no cop2 or trap instruction "
                          f"(a scheduling barrier by name)")
    for _s, _e, name in shadow_spans(text, root):
        issues.append(f"the TU redefines vendored macro {name} (#define/#undef)")
    return issues


# --- macros outside a gate's sight ------------------------------------------
# Every gate scans unexpanded source, so a macro is seen only through its
# name. The vendored calls are islands (above); anything else that expands to
# asm is counted and stripped by name (engine.inlineasm._strip_spans).

# The headers that may define a macro expanding to inline asm (design section
# 2, header registry): the VENDORED headers, bios.h (the BIOS trampolines,
# inlineasm.bios_macro_spans) and include_asm.h (INCLUDE_ASM).
ASM_MACRO_HEADERS = frozenset(VENDORED) | {"include/bios.h", "include/include_asm.h"}
_HEADER_DIRS = ("include", "src")


def _header_paths(root: str) -> list[Path]:
    return sorted(p for d in _HEADER_DIRS for p in (Path(root) / d).rglob("*.h"))


def _defines_with_kind(text: str) -> list[tuple[str, bool, frozenset, frozenset]]:
    """(name, function_like, params, idents) per #define logical line."""
    from . import volatile_cheats  # local import: volatile_cheats imports inlineasm
    out = []
    for line in cia.compiler_view(text).masked_eol.split("\n"):
        m = volatile_cheats._MACRO_DEFINE_RE.match(line)
        if m:
            out.append((m.group("name"), m.group("params") is not None,
                        frozenset(volatile_cheats._C_IDENT_RE.findall(m.group("params") or "")),
                        frozenset(volatile_cheats._C_IDENT_RE.findall(line[m.end():]))))
    return out


def asm_macro_registry(root: str | Path = REPO_ROOT) -> tuple[tuple[str, str, bool], ...]:
    """(name, header, function_like) for every #define in include/**/*.h and
    src/**/*.h whose replacement reaches an asm keyword, transitively across
    headers (a macro naming another header's asm macro is one too). header is
    the path relative to root."""
    paths = _header_paths(str(root))
    stamps = []
    for p in paths:
        try:
            st = p.stat()
            stamps.append((p.as_posix(), st.st_mtime_ns, st.st_size))
        except OSError:
            pass
    return _asm_macro_registry_cached(str(root), tuple(stamps))


@functools.lru_cache(maxsize=4)
def _asm_macro_registry_cached(root: str, stamps: tuple) -> tuple[tuple[str, str, bool], ...]:
    from . import volatile_cheats
    defs = []
    for path, _mt, _sz in stamps:
        p = Path(path)
        rel = p.relative_to(Path(root)).as_posix()
        text = p.read_text(encoding="utf-8", errors="replace")
        defs += [(rel, *d) for d in _defines_with_kind(text)]
    # A vendored name seeds too: a header macro wrapping gte_nop() is asm.
    seeds = set(cia.ASM_SPELLINGS) | volatile_cheats._BIOS_MACROS | vendored_names(root)
    asm = {name for _r, name, _f, _p, idents in defs if name in seeds or idents & seeds}
    grew = True
    while grew:
        grew = False
        for _r, name, _f, params, idents in defs:
            if name not in asm and (idents - params) & asm:
                asm.add(name)
                grew = True
    return tuple((name, rel, fl) for rel, name, fl, _p, _i in defs if name in asm)


def header_registry_issues(root: str | Path = REPO_ROOT) -> list[str]:
    """Every asm macro defined outside ASM_MACRO_HEADERS, every header that
    includes a vendored header (only a .c TU may: tu_includes reads the TU),
    and every header using the `##` / `%:%:` token paste (paste_spans: a header
    macro could assemble a name for a TU that never spells the paste)."""
    issues = [f"{rel}: #define {name} expands to inline asm outside the vendored "
              f"PsyQ headers, bios.h and include_asm.h"
              for name, rel, _fl in asm_macro_registry(root) if rel not in ASM_MACRO_HEADERS]
    for p in _header_paths(str(root)):
        rel = p.relative_to(Path(root)).as_posix()
        text = p.read_text(encoding="utf-8", errors="replace")
        if rel not in VENDORED and tu_includes(text):
            issues.append(f"{rel}: includes a vendored PsyQ header (only a .c TU may)")
        if paste_spans(text):
            issues.append(f"{rel}: uses the `##` token-paste operator (a cheat construct)")
    return issues


def foreign_asm_macros(root: str | Path = REPO_ROOT) -> dict[str, bool]:
    """name -> function_like for every asm macro a header outside
    ASM_MACRO_HEADERS defines. A vendored name is left out: its calls are
    islands, and the header redefining it is a registry violation."""
    allowed = {name for name, rel, _fl in asm_macro_registry(root) if rel in ASM_MACRO_HEADERS}
    vend = vendored_names(root)
    return {name: fl for name, rel, fl in asm_macro_registry(root)
            if rel not in ASM_MACRO_HEADERS and name not in allowed and name not in vend}


def _invocation_spans(text: str, names: dict[str, bool]) -> list[tuple[int, int, str]]:
    """(start, end, name) of every invocation of `names` (name -> function_like)
    in code — comments, strings and preprocessor lines ignored, backslash-
    newlines spliced (the compiler view) — through the balanced `)` of a
    function-like macro and an optional `;`. An unbalanced invocation spans to
    the end of the text (fail closed)."""
    if not names:
        return []
    masked, pos, pp = _code_view(text)
    rx = re.compile(r"\b(" + "|".join(map(re.escape, sorted(names))) + r")\b")
    out, last = [], -1
    for m in rx.finditer(masked):
        if m.start() < last or _in_ranges(m.start(), pp) >= 0:
            continue
        end = m.end()
        if names[m.group(1)]:
            j = end
            while j < len(masked) and masked[j] in " \t\n":
                j += 1
            if j >= len(masked) or masked[j] != "(":
                continue  # a function-like macro's name alone is not expanded
            close = inlineasm._match_paren(masked, j)
            end = close if close > 0 else len(masked)
        k = end
        while k < len(masked) and masked[k] in " \t":
            k += 1
        if k < len(masked) and masked[k] == ";":
            end = k + 1
        out.append((pos[m.start()], pos[end - 1] + 1, m.group(1)))
        last = end
    return out


_SHADOW_RE = re.compile(r"[ \t]*(?:#|%:)[ \t]*(?:define|undef)[ \t]+([A-Za-z_]\w*)")


def shadow_spans(text: str, root: str | Path = REPO_ROOT) -> list[tuple[int, int, str]]:
    """(start, end, name) of every `#define` / `#undef` of a vendored macro name
    in a source file: a redefinition would replace Sony's text."""
    names = vendored_names(root)
    if not names:
        return []
    masked, pos, pp = _code_view(text)
    out = []
    for s, e in pp:
        m = _SHADOW_RE.match(masked, s)
        if m and m.end() <= e and m.group(1) in names:
            out.append((pos[s], pos[e - 1] + 1 if e > s else pos[s], m.group(1)))
    return out


_PASTE_RE = re.compile(r"##|%:%:")


def paste_spans(text: str) -> list[tuple[int, int]]:
    """Spans of every `##` (or `%:%:`) token-paste operator in code: the whole
    directive line holding it, else the token. A paste can assemble an asm
    keyword or a vendored macro name no gate sees by name, so it is a cheat
    construct. Comments and literals never count; a paste either masked view
    sees is reported (erring toward detection)."""
    spliced = cia._splice(text)[0]  # `#\<newline>#` is a paste too
    if "##" not in spliced and "%:%:" not in spliced:
        return []
    view = cia.compiler_view(text)
    pp = _preprocessor_ranges(view.masked_eol)
    out = set()
    for masked in (view.masked, view.masked_eol):
        for m in _PASTE_RE.finditer(masked):
            s = _in_ranges(m.start(), pp)
            if s >= 0:
                e = next(e for s2, e in pp if s2 == s)
                out.add((view.pos[s], view.pos[e - 1] + 1))
            else:
                out.add((view.pos[m.start()], view.pos[m.end() - 1] + 1))
    return sorted(out)


def macro_strip_spans(text: str, root: str | Path = REPO_ROOT) -> list[tuple[int, int]]:
    """Spans of a source file the cheat strip removes (sandbox) and counts
    (gate) by name: invocations of a foreign header's asm macro, #define /
    #undef of a vendored name, calls of a vendored macro that is not
    Macro.admitted (gte_nop(); scored as stripped since the gate refuses it), and
    token pastes (paste_spans)."""
    spans = [(s, e) for s, e, _n in _invocation_spans(text, foreign_asm_macros(root))]
    spans += [(s, e) for s, e, _n in shadow_spans(text, root)]
    spans += [sp for sp in paste_spans(text)
              if not any(sp[0] < e and s < sp[1] for s, e in spans)]
    try:
        macros = tu_macros(text, root) if tu_header(text) else {}
    except ValueError:  # both inline headers
        macros = {}
    if not macros:  # no (single) inline header: either one's barrier counts
        macros = {**header_macros("inline_c.h", root), **header_macros("inline_o.h", root)}
    for h in STANDALONE_HEADERS:
        macros.update(standalone_macros(h, root))
    free = {n: True for n, m in macros.items() if not m.admitted}
    spans += [(s, e) for s, e, _n in _invocation_spans(text, free)]
    spans += [(s, e) for s, e, _n in bare_spans(text, root)]
    return spans


_NOT_INVOKERS = _CONTROL_HEADS | {"return", "sizeof"}


def bare_spans(text: str, root: str | Path = REPO_ROOT) -> list[tuple[int, int, str]]:
    """(start, end, name) of every code occurrence of a vendored name not
    directly followed by `(`: the macro is reached through another name
    (`#define CALL(m) m()` / `CALL(gte_nop);`, `(gte_nop)();`), which no gate
    sees as a call. The span is the enclosing macro or function invocation
    (through its `)` and an optional `;`) when the name is an argument of one,
    else the name alone; call_sites refuses the same occurrences."""
    names = vendored_names(root)
    if not names:
        return []
    masked, pos, pp = _code_view(text)
    if not any(p in masked for p in _vendored_prefixes(names)):
        return []
    out, last = [], -1
    for m in _IDENT_RE.finditer(masked):
        name = m.group(0)
        if name not in names or m.start() < last or _in_ranges(m.start(), pp) >= 0:
            continue
        j = m.end()
        while j < len(masked) and masked[j] in " \t\n":
            j += 1
        if j < len(masked) and masked[j] == "(":
            continue
        s, e = m.start(), m.end()
        depth, k = 0, s - 1
        while k >= 0 and masked[k] not in ";{}":
            if masked[k] == ")":
                depth += 1
            elif masked[k] == "(":
                if depth == 0:
                    break
                depth -= 1
            k -= 1
        if k >= 0 and masked[k] == "(":
            q = k - 1
            while q >= 0 and masked[q] in " \t\n":
                q -= 1
            im = re.search(r"\b[A-Za-z_]\w*$", masked[max(0, q - 63):q + 1])
            if im and im.group(0) not in _NOT_INVOKERS:
                close = inlineasm._match_paren(masked, k)
                s, e = q + 1 - len(im.group(0)), close if close > 0 else len(masked)
        k = e
        while k < len(masked) and masked[k] in " \t":
            k += 1
        if k < len(masked) and masked[k] == ";":
            e = k + 1
        out.append((pos[s], pos[e - 1] + 1, name))
        last = e
    return out

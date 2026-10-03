#!/usr/bin/env python3
"""Classify each `__asm__` block in src/*.c as CANONICAL or CHEAT.

Inline-asm policy (canonical statement of where BB2 stands relative to the
SOTN community bar — see .claude/rules/inline-asm-policy.md):

  CANONICAL-BODY = full canonical-asm function body. The original was
      hand-written asm OR was emitted via file-scope `__asm__("glabel ...")`.
      Listed in inline_asm_canonical.txt. NOT classified here.
  CANONICAL      = inline `__asm__` in a C function body using opcodes that
      ONLY exist in asm form. Authentic — original developers wrote these.
        - GTE coprocessor ops: ctc2/cfc2/mtc2/mfc2/lwc2/swc2
        - GTE math ops: .word 0x4XXXXXXX (cop2 instruction encoding)
        - BIOS vectors: jumps to 0xA0/0xB0/0xC0
        - Cache/DMA hardware register pokes (specific addresses)
  CHEAT          = inline `__asm__` in a C function body using general-purpose
      GPR opcodes (move/addu/nop/lui/etc.) to steer GCC's allocator or
      scheduler. Workarounds we added; NOT in original source. Forbidden in
      committed source — a function carrying any of these is INCOMPLETE.
  (pure C)       = no inline asm needed. Not counted here.

COMPLETED functions use only CANONICAL-BODY, CANONICAL, or pure C.
CHEAT is the BB2-specific gap; the count of CHEAT functions is what remains
to retire.

Heuristic limitations (documented):
  - `__asm__` statements are found by find_asm_keywords (the compiler's view:
    comments / backslash-newlines between keyword, qualifiers and `(` are
    whitespace), which also supplies each block's full, possibly multi-line,
    parenthesized body.
  - File-scope `__asm__("...glabel...")` blocks are excluded (CANONICAL-BODY).
  - `sw`/`lw` to scratchpad addresses (0x1F8003xx) are CANONICAL (hardware
    pokes); to general addresses they default to CHEAT.

Output formats:
  default — per-function table + summary
  --summary — just the summary counts
  --json — structured JSON for tooling consumption
  --func F — classify ONE function (show its asm blocks)
"""
from __future__ import annotations

import argparse
import bisect
import functools
import json
import re
import sys
from pathlib import Path
from typing import NamedTuple

ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = ROOT / "src"

# --- CANONICAL: authentic asm (no C equivalent exists) ---
CANONICAL_ASM_OPS = frozenset({
    # GTE register transfers
    "ctc2", "cfc2", "mtc2", "mfc2",
    # GTE memory load/store
    "lwc2", "swc2",
    # cop2 misc (rare)
    "ctc0", "mfc0", "mtc0",  # MIPS coprocessor 0 — kernel-mode
})

# CANONICAL: GTE op encodings via .word — top byte 0x48-0x4F is cop2 op range.
CANONICAL_DOTWORD_RE = re.compile(r'\.word\s*0x4[89A-Fa-f]', re.IGNORECASE)

# CANONICAL: BIOS vector jumps. A0/B0/C0 are the PSX BIOS dispatch vectors.
CANONICAL_BIOS_RE = re.compile(
    r'\b(?:j|jal)\s+0x[abc]0\b|\bjr\s+\$t1\b',
    re.IGNORECASE,
)

# CANONICAL: scratchpad / hardware register addresses.
# Scratchpad: 0x1F800000–0x1F8003FF (1 KB).
# I/O ports: 0x1F801000–0x1F803FFF (GPU/SPU/DMA/etc.).
CANONICAL_HW_ADDR_RE = re.compile(r'0x1f80[0123]\b|0x1f8003', re.IGNORECASE)

# --- CHEAT: general-purpose GPR ops used as workarounds ---
CHEAT_ASM_OPS = frozenset({
    # Arithmetic / logical
    "move", "addu", "addiu", "subu", "negu", "neg",
    "add", "sub", "addi",
    "lui", "li", "la",
    "andi", "ori", "xori", "and", "or", "xor", "nor",
    # Shifts
    "sll", "sra", "srl", "sllv", "srlv", "srav",
    # Compare
    "slt", "slti", "sltu", "sltiu",
    # Memory (general — overridden to CANONICAL when address is hardware)
    "sw", "lw", "sh", "lh", "sb", "lb", "lhu", "lbu", "lwl", "lwr", "swl", "swr",
    # Multiply/divide
    "mult", "multu", "div", "divu", "mflo", "mfhi", "mtlo", "mthi",
    # Filler / no-op
    "nop",
    # Branches / jumps (general)
    "beq", "bne", "bgez", "bltz", "blez", "bgtz", "b",
    "bgezl", "bltzl", "blezl", "bgtzl", "beql", "bnel",
    "bgezal", "bltzal",
})

# Empty asm template (typically `__asm__ volatile("" ::: "memory")` —
# scheduling barrier). Count as CHEAT since it's purely codegen control.
EMPTY_ASM_IS_CHEAT = True

# --- AST-light parser ---
# We don't fully parse C. Instead we use line-by-line state tracking.
# This is accurate enough for the BB2 codebase's actual style.

# Matches a C function DEFINITION signature. The body's `{` may be on
# this line OR the next non-blank line (K&R style). Two regexes:
#   FUNC_OPEN_RE — signature + `{` on same line
#   FUNC_SIG_RE  — signature only (must look ahead for `{` on next line)
FUNC_OPEN_RE = re.compile(
    r'^\s*(?:static\s+)?(?:inline\s+)?'
    r'[a-zA-Z_][\w*]*\s+\**\s*([a-zA-Z_]\w*)\s*\([^;{]*\)\s*\{'
)
FUNC_SIG_RE = re.compile(
    r'^\s*(?:static\s+)?(?:inline\s+)?'
    r'[a-zA-Z_][\w*]*\s+\**\s*([a-zA-Z_]\w*)\s*\([^;{]*\)\s*$'
)
BRACE_ALONE_RE = re.compile(r'^\s*\{\s*$')

# File-scope `__asm__("...glabel...")` block (canonical-asm function body).
# We exclude these from inline-asm classification.
GLABEL_HINT_RE = re.compile(r'\bglabel\b|\bendlabel\b')

# `__asm__` keyword (with double underscores) for inline-asm STATEMENTS, as a
# plain regex. ONLY for trusted, pinned text (engine/gtemacro.py parses the
# pinned PsyQ header excerpts with it). Source files MUST be scanned with
# find_asm_keywords() below: this regex runs on raw text, so a comment or a
# backslash-newline between the keyword and `(` hides a statement from it
# (layer-2 finding 2026-09-25: `__asm__ /**/ ("move $5,%0" : : "r"(n));` was
# counted 0 by the completion gate and stripped 0 by the sandbox).
ASM_KEYWORD_RE = re.compile(
    r'\b(__asm__|__asm)\s*(?:volatile|__volatile__|__volatile|const|__const__|__const)?\s*\(')

# Every spelling GCC 2.7.2's keyword table maps to ASM_KEYWORD
# (tools/gcc-2.7.2/c-parse.gperf), and every TYPE_QUAL spelling the grammar's
# `ASM_KEYWORD maybe_type_qual '('` accepts in between (c-parse.y).
ASM_SPELLINGS = ("__asm__", "__asm", "asm")
# The statement spellings. Bare `asm` is left out: it is also the qualifier of
# a register pin / alias declaration (`register T x asm("$N")`), which is
# classified separately below (REGISTER_PIN_RE), and statement-scope bare `asm`
# is caught by engine/volatile_cheats.find_lowercase_asm_cheats.
STATEMENT_SPELLINGS = ("__asm__", "__asm")
ASM_QUALIFIERS = frozenset({"volatile", "__volatile", "__volatile__",
                            "const", "__const", "__const__"})


def _splice(text: str) -> tuple[str, list[int]]:
    """Translation phase 2: delete every backslash-newline. Returns the spliced
    text and, per spliced char, its index in `text` (plus a final len(text))."""
    out, pos, i, n = [], [], 0, len(text)
    while i < n:
        c = text[i]
        if c == "\\":
            if text.startswith("\n", i + 1):
                i += 2
                continue
            if text.startswith("\r\n", i + 1):
                i += 3
                continue
        out.append(c)
        pos.append(i)
        i += 1
    pos.append(n)
    return "".join(out), pos


def _blank(text: str, strings: bool, multiline: bool) -> str:
    """Comments -> spaces (always); string/char literal contents -> spaces too
    when `strings`. Same length, newlines kept.

    A char literal ends at its closing quote or at a newline. A string literal
    does too unless `multiline`: the preprocessor (modern cpp here) lexes an
    unterminated `"` as running to the end of the line, but cc1 2.7.2 accepts a
    string literal spanning physical lines (the tree had one until 2026-10-03: a
    file-scope `__asm__(".section .rodata<NL>.word 0<NL>.text")` in code6cac_c_ab.c)."""
    chars = list(text)
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c in "\"'":
            stops = (c,) if multiline and c == '"' else (c, "\n")
            j = i + 1
            while j < n and text[j] not in stops:
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1 if j < n and text[j] == c else j, n)
            if strings:
                for k in range(i, j):
                    if chars[k] != "\n":
                        chars[k] = " "
            i = j
            continue
        if c == "/" and text.startswith("/", i + 1):
            j = text.find("\n", i)
            j = n if j == -1 else j
        elif c == "/" and text.startswith("*", i + 1):
            j = text.find("*/", i + 2)
            j = n if j == -1 else j + 2
        else:
            i += 1
            continue
        for k in range(i, j):
            if chars[k] != "\n":
                chars[k] = " "
        i = j
    return "".join(chars)


class CompilerView(NamedTuple):
    """`text` as the compiler tokenizes it, index-mapped back to `text`.

    code      backslash-newlines spliced out and comments blanked to spaces
              (to the compiler a comment IS whitespace); literals intact.
              Comments are found the way cpp finds them (strings end at EOL).
    masked    `code` with literal contents blanked too, strings allowed to span
              lines as cc1 reads them.
    masked_eol  the same, but every literal ends at the end of its line, as cpp
              lexes it.
    pos       pos[i] is the index in `text` of view char i; pos[-1] == len(text).

    All views are the same length. A token search needs BOTH masked views: an
    unmatched `"` that cpp never hands to cc1 (a stray quote in an `#if 0`
    block) desyncs `masked` for the rest of the file, while `masked_eol` is
    blind inside a real multi-line string. A keyword either one sees is
    reported (find_asm_keywords) — erring toward detection."""
    code: str
    masked: str
    masked_eol: str
    pos: tuple


@functools.lru_cache(maxsize=8)
def compiler_view(text: str) -> CompilerView:
    spliced, pos = _splice(text)
    code = _blank(spliced, strings=False, multiline=False)
    return CompilerView(code, _blank(code, strings=True, multiline=True),
                        _blank(code, strings=True, multiline=False), tuple(pos))


def is_plain_code(s: str) -> bool:
    """True when `s` contains no comment and no backslash-newline — i.e. the
    compiler view of it is the text itself."""
    return _splice(s)[0] == s and _blank(s, strings=False, multiline=False) == s


class AsmKeyword(NamedTuple):
    """One inline-asm keyword token found by find_asm_keywords. Indexes are
    into the ORIGINAL text; `body` is the view text between the parens
    (comment-free, spliced), which is what the compiler parses."""
    start: int          # the keyword
    kw_end: int         # just past the keyword
    paren: int          # the `(` after any qualifiers, -1 if none follows
    end: int            # just past the matching `)`, -1 if none
    body: str           # between the parens ("" when end == -1)
    spelling: str       # "__asm__" / "__asm" / "asm"
    quals: tuple        # qualifier (or macro) identifiers before the `(`
    directive: bool     # on a preprocessor line (e.g. a `#define` body)
    bol: bool           # first token on its (logical) line
    after: str          # previous significant token ("" at start of file)
    prefix: tuple       # the run of identifiers directly before the keyword
    prefix_after: str   # the token before that run ("" at start of file)
    depth: int          # brace depth (0 = file scope), directives not counted

    @property
    def keyword_text(self) -> str:
        return " ".join((self.spelling,) + self.quals)


_KW_TOKEN_RE = re.compile(r"(?<![\w$])(__asm__|__asm|asm)(?![\w$])")
_IDENT_TOKEN_RE = re.compile(r"[A-Za-z_$][\w$]*")
_PREV_TOKEN_RE = re.compile(r"(?:[A-Za-z_$][\w$]*|\S)$")


def is_directive_lead(lead: str) -> bool:
    """True when `lead` — a line's text up to some point, comments blanked —
    opens a preprocessor directive: `#`, or its digraph `%:`, which the
    build's cpp accepts (`%:define A __asm__` is a real #define)."""
    return lead.lstrip().startswith(("#", "%:"))


def find_asm_keywords(text: str, spellings=STATEMENT_SPELLINGS) -> tuple[AsmKeyword, ...]:
    """Every inline-asm keyword TOKEN in `text` with one of `spellings`.

    Tokens are found the way the compiler sees them (compiler_view): never
    inside a comment or string; whitespace, comments and backslash-newlines may
    sit anywhere between the keyword, its qualifiers and the `(`. Identifiers
    between the keyword and `(` are accepted and reported in `quals` — besides
    the real qualifiers, only a macro can stand there, and a macro that hides
    `volatile` must not hide the statement. A keyword with no `(` after it is
    still reported (paren == end == -1) so callers can fail safe on a shape they
    do not understand (e.g. a macro argument `Q(__asm__)`)."""
    return _find_asm_keywords(text, tuple(spellings))


def _operand_list(masked: str, j: int, directive: bool):
    """From just past a keyword in `masked`: (quals, paren, close) view indexes
    of the qualifier identifiers' `(` and its matching `)`; -1 when absent."""
    n = len(masked)
    quals = []
    while True:
        # A directive ends at its (logical) newline.
        while j < n and masked[j].isspace() and not (directive and masked[j] == "\n"):
            j += 1
        im = _IDENT_TOKEN_RE.match(masked, j)
        if not im:
            break
        quals.append(im.group(0))
        j = im.end()
    if j >= n or masked[j] != "(":
        return tuple(quals), -1, -1
    depth = 0
    for k in range(j, n):
        if masked[k] == "(":
            depth += 1
        elif masked[k] == ")":
            depth -= 1
            if depth == 0:
                return tuple(quals), j, k
    return tuple(quals), j, -1


def _prev_token(masked: str, b: int) -> tuple[str, int]:
    """The significant token ending before view index `b`, and its start."""
    while b > 0 and masked[b - 1].isspace():
        b -= 1
    m = _PREV_TOKEN_RE.search(masked, max(0, b - 256), b)
    return (m.group(0), m.start()) if m else ("", 0)


def _brace_depths(masked: str, points: list[int]) -> dict[int, int]:
    """Brace depth ({ } and the digraphs <% %>) at each view index in
    `points`, not counting braces on preprocessor lines."""
    want, out, depth = set(points), {}, 0
    in_directive, at_line_start, n = False, True, len(masked)
    for i, c in enumerate(masked):
        if at_line_start:
            j = masked.find("\n", i)
            in_directive = is_directive_lead(masked[i:n if j == -1 else j])
            at_line_start = False
        if i in want:
            out[i] = depth
        if c == "\n":
            at_line_start = True
        elif in_directive:
            continue
        elif c == "{" or (c == "<" and masked.startswith("%", i + 1)):
            depth += 1
        elif c == "}" or (c == "%" and masked.startswith(">", i + 1)):
            depth -= 1
    return out


@functools.lru_cache(maxsize=32)
def _find_asm_keywords(text: str, spellings: tuple) -> tuple[AsmKeyword, ...]:
    view = compiler_view(text)
    code, pos = view.code, view.pos
    found = {}
    for masked in (view.masked, view.masked_eol):
        for m in _KW_TOKEN_RE.finditer(masked):
            if m.group(1) in spellings:
                found.setdefault(m.start(), (m, masked))
    depths = _brace_depths(view.masked, list(found))
    out = []
    for v in sorted(found):
        m, masked = found[v]
        line_start = view.masked_eol.rfind("\n", 0, v) + 1
        lead = view.masked_eol[line_start:v]
        directive = is_directive_lead(lead)
        prev, pstart = _prev_token(masked, v)
        prefix = []
        run_after, rstart = prev, pstart
        while run_after and _IDENT_TOKEN_RE.fullmatch(run_after):
            prefix.insert(0, run_after)
            run_after, rstart = _prev_token(masked, rstart)
        # Operands as cc1 reads them first; if that view cannot close the list
        # (it is desynced by a stray quote), the per-line view.
        quals, paren, close = _operand_list(masked, m.end(), directive)
        if close < 0 and masked is view.masked:
            alt = _operand_list(view.masked_eol, m.end(), directive)
            if alt[2] >= 0:
                quals, paren, close = alt
        out.append(AsmKeyword(
            pos[v], pos[m.end() - 1] + 1,
            pos[paren] if paren >= 0 else -1,
            pos[close] + 1 if close >= 0 else -1,
            code[paren + 1:close] if close >= 0 else "",
            m.group(1), quals, directive, not lead.strip(), prev,
            tuple(prefix), run_after, depths[v]))
    return tuple(out)


# Register-asm pin declarations: `register T x asm("$N");` or
# `register T x asm("$N") = expr;`. These are also CHEAT workarounds
# (allocation hints we added) but a different KIND than `__asm__` blocks.
REGISTER_PIN_RE = re.compile(r'\bregister\s+[^=;]*\basm\s*\(\s*"[^"]+"\s*\)')


def classify_template(template: str) -> str:
    """Classify a single asm template string. Returns 'canonical' or 'cheat'."""
    text = template.strip()
    if not text:
        return "cheat" if EMPTY_ASM_IS_CHEAT else "canonical"
    # Check canonical regex signals first (.word cop2, BIOS vectors, HW addr).
    if CANONICAL_DOTWORD_RE.search(text):
        return "canonical"
    if CANONICAL_BIOS_RE.search(text):
        return "canonical"
    if CANONICAL_HW_ADDR_RE.search(text):
        return "canonical"
    # Opcode-based check: get the first whitespace-separated token.
    # Skip `.set noreorder` etc. directives.
    first = text.lstrip().split(None, 1)[0].lower()
    if first.startswith("."):
        # Directive like .set, .word (handled above). Default cheat.
        return "cheat"
    if first in CANONICAL_ASM_OPS:
        return "canonical"
    if first in CHEAT_ASM_OPS:
        return "cheat"
    # Unknown opcode — default to cheat (workaround / unclassified is
    # safer than letting unknowns pass as authentic).
    return "cheat"


def extract_strings(parens_body: str) -> list[str]:
    """Extract C string literals from inside the parentheses of an asm
    block. Each string is the content between consecutive `"`s, with
    \\n / \\t literally preserved (we don't unescape; that would lose
    the multi-instruction grouping)."""
    return re.findall(r'"((?:[^"\\]|\\.)*)"', parens_body)


def split_template(template: str) -> list[str]:
    """Split a multi-line template (with literal `\\n` separators) into
    one entry per logical instruction. The asm template can be either:
       "ctc2 %0, $0"                  — one instruction
       "ctc2 %0, $0\\nlwc2 $0, 0(%1)" — two instructions joined by \\n
    """
    parts = re.split(r'\\n', template)
    return [p.strip() for p in parts if p.strip()]


def scan_file(path: Path) -> list[dict]:
    """Scan one .c file. Returns list of records, one per __asm__ block.
    Each record: {file, line, func, category, template, opcode_first, is_glabel}.
    """
    try:
        text = path.read_text(encoding="utf-8", errors="ignore")
    except OSError:
        return []
    lines = text.splitlines()
    line_starts = [0]
    for ln in text.splitlines(keepends=True):
        line_starts.append(line_starts[-1] + len(ln))
    kws = find_asm_keywords(text)
    kw_lines = [bisect.bisect_right(line_starts, k.start) - 1 for k in kws]
    kw_i = 0

    # Track enclosing function via brace depth. We assume a function body
    # opens with `{` and closes with `}` at column 0 (BB2 style).
    func_stack: list[tuple[str, int]] = []  # (name, depth_at_entry)
    depth = 0
    current_func: str | None = None

    records: list[dict] = []

    # We need to handle __asm__ blocks that span multiple lines (e.g., a
    # multi-string template). Do a single pass: when we see `__asm__(`
    # without the matching `)`, accumulate lines until we find it.
    i = 0
    while i < len(lines):
        line = lines[i]

        # Update brace depth from THIS line (count opens/closes outside
        # of string literals — simplified: just count chars). Not perfect
        # but good enough for BB2 style.
        # First detect function-definition opening (which gives us depth+1).
        # Handle both `void foo(...) {` (same line) and K&R `void foo(...)\n{`.
        # `just_pushed` prevents the pop check below from immediately popping
        # K&R signatures (where the `{` hasn't yet raised depth).
        just_pushed = False
        if depth == 0:
            m = FUNC_OPEN_RE.match(line)
            if m:
                current_func = m.group(1)
                # Save depth-BEFORE the `{` (= 0); pop when depth returns to it.
                func_stack.append((current_func, depth))
                just_pushed = True
            else:
                sig = FUNC_SIG_RE.match(line)
                if sig:
                    # Look ahead for `{` on the next non-blank line.
                    j = i + 1
                    while j < len(lines) and not lines[j].strip():
                        j += 1
                    if j < len(lines) and BRACE_ALONE_RE.match(lines[j]):
                        current_func = sig.group(1)
                        func_stack.append((current_func, depth))
                        just_pushed = True

        # Detect register-asm pins on this line (CHEAT workaround,
        # different KIND from __asm__ blocks).
        for pm in REGISTER_PIN_RE.finditer(line):
            records.append({
                "file": str(path.relative_to(ROOT)),
                "line": i + 1,
                "func": current_func,
                "category": "cheat",
                "kind": "register_pin",
                "templates": [pm.group(0)],
                "first_op": "register-asm pin",
            })

        # Every __asm__ keyword starting on this line — or on a later line
        # the asm blocks found so far extend over — is one record; its body
        # (possibly spanning lines) comes from find_asm_keywords.
        j = i
        while kw_i < len(kws) and kw_lines[kw_i] <= j:
            kw = kws[kw_i]
            kw_i += 1
            asm_body = kw.body
            if kw.end > 0:
                j = max(j, bisect.bisect_right(line_starts, kw.end - 1) - 1)

            # Extract template strings from the asm body.
            strings = extract_strings(asm_body)
            # Strip the operand/clobber portion: a string after `:` doesn't
            # contribute opcodes. Operand strings look like "r"(var), but
            # we only extract from BEFORE the first `:` at depth 1.
            # Simpler: classify each template-position string, then strip
            # operand-position strings by detecting `:` in the body.
            first_colon = asm_body.find(":")
            template_body = asm_body if first_colon == -1 else asm_body[:first_colon]
            template_strs = extract_strings(template_body)
            # Detect glabel/endlabel — CANONICAL-BODY (whole-function asm), skip.
            is_glabel = any(GLABEL_HINT_RE.search(s) for s in template_strs)

            if not is_glabel:
                # Classify each instruction in the templates. If any canonical
                # opcode is present, the block is canonical. Otherwise cheat.
                block_category = "cheat"  # default for empty / unknown
                instrs = []
                if not template_strs:
                    block_category = "cheat" if EMPTY_ASM_IS_CHEAT else "canonical"
                else:
                    instrs = []
                    for t in template_strs:
                        instrs.extend(split_template(t))
                    if not instrs:
                        block_category = "cheat" if EMPTY_ASM_IS_CHEAT else "canonical"
                    else:
                        per_instr_categories = [classify_template(ins) for ins in instrs]
                        if "canonical" in per_instr_categories:
                            block_category = "canonical"
                        else:
                            block_category = "cheat"
                    first_op = instrs[0].split(None, 1)[0].lower() if instrs else ""

                records.append({
                    "file": str(path.relative_to(ROOT)),
                    "line": kw_lines[kw_i - 1] + 1,
                    "func": current_func,
                    "category": block_category,
                    "kind": "asm_block",
                    "templates": template_strs,
                    "first_op": instrs[0] if instrs else "",
                })
        # Skip past the asm blocks: the line loop resumes after the last line
        # they consumed.
        i = j

        # Update brace depth for the WHOLE line (after asm parsing).
        # Simplification: count `{` and `}` ignoring strings/comments.
        # This is approximate but adequate.
        # Remove string literals to avoid braces inside strings.
        stripped = re.sub(r'"(?:[^"\\]|\\.)*"', '', line)
        stripped = re.sub(r"'(?:[^'\\]|\\.)*'", '', stripped)
        # Remove line comments.
        stripped = re.sub(r'//.*$', '', stripped)
        opens = stripped.count('{')
        closes = stripped.count('}')
        depth += opens - closes
        if depth < 0:
            depth = 0  # defensive: don't go negative on mis-parse
        # Pop funcs that have closed. Saved value is depth BEFORE the
        # function's `{`; pop when current depth has returned to it.
        # Skip pop check on lines that just pushed (otherwise K&R
        # signatures would pop immediately, before their `{` arrives).
        if not just_pushed:
            while func_stack and func_stack[-1][1] >= depth:
                func_stack.pop()
        current_func = func_stack[-1][0] if func_stack else None
        i += 1

    return records


def summarize(records: list[dict]) -> dict:
    """Compute per-function and per-project rollup."""
    per_func: dict[str | None, dict[str, int]] = {}
    canonical_total = 0
    cheat_total = 0
    cheat_pins_total = 0
    cheat_blocks_total = 0
    for r in records:
        # Key file-scope records per-file so distinct files don't merge.
        func = r["func"] or f"<file-scope:{r['file']}>"
        bucket = per_func.setdefault(func, {
            "canonical": 0, "cheat": 0, "cheat_pins": 0, "cheat_blocks": 0,
            "file": r["file"],
        })
        bucket[r["category"]] += 1
        if r["category"] == "canonical":
            canonical_total += 1
        elif r["category"] == "cheat":
            cheat_total += 1
            if r.get("kind") == "register_pin":
                cheat_pins_total += 1
                bucket["cheat_pins"] += 1
            else:
                cheat_blocks_total += 1
                bucket["cheat_blocks"] += 1
    # Per-function category label.
    func_categories: dict[str, list[dict]] = {"canonical": [], "cheat": [], "mixed": []}
    for func, b in per_func.items():
        if b["cheat"] > 0 and b["canonical"] > 0:
            func_categories["mixed"].append({"func": func, **b})
        elif b["cheat"] > 0:
            func_categories["cheat"].append({"func": func, **b})
        else:
            func_categories["canonical"].append({"func": func, **b})
    return {
        "totals": {
            "canonical_instances": canonical_total,
            "cheat_instances": cheat_total,
            "cheat_register_pins": cheat_pins_total,
            "cheat_asm_blocks": cheat_blocks_total,
            "canonical_funcs_only": len(func_categories["canonical"]),
            "cheat_funcs_only": len(func_categories["cheat"]),
            "mixed_funcs": len(func_categories["mixed"]),
            "gap_to_sotn_funcs": len(func_categories["cheat"]) + len(func_categories["mixed"]),
            "gap_to_sotn_instances": cheat_total,
        },
        "per_func": per_func,
        "func_categories": func_categories,
    }


def render_human(summary: dict, *, color: bool = False, show_funcs: bool = True) -> str:
    GREEN = "\033[32m" if color else ""
    YELLOW = "\033[33m" if color else ""
    RED = "\033[31m" if color else ""
    DIM = "\033[2m" if color else ""
    RESET = "\033[0m" if color else ""
    t = summary["totals"]
    out = []
    out.append("=== inline-asm classification ===")
    out.append(f"  {GREEN}canonical (authentic GTE/BIOS/HW){RESET}: "
               f"{t['canonical_instances']} instances across "
               f"{t['canonical_funcs_only']} pure-canonical funcs")
    out.append(f"  {RED}cheat (toolchain workaround){RESET}:    "
               f"{t['cheat_instances']} instances "
               f"({t['cheat_asm_blocks']} __asm__ blocks + "
               f"{t['cheat_register_pins']} register-asm pins) "
               f"across {t['cheat_funcs_only']} pure-cheat funcs")
    out.append(f"  {YELLOW}mixed (both categories in one func){RESET}:    "
               f"{t['mixed_funcs']} funcs")
    out.append("")
    out.append(f"  {RED}GAP TO SOTN BAR{RESET}:")
    out.append(f"    {t['gap_to_sotn_funcs']} functions use cheat inline asm")
    out.append(f"    {t['gap_to_sotn_instances']} total cheat instances to retire")
    out.append("")
    out.append(f"  {DIM}(COMPLETED funcs use only canonical-body asm, canonical inline "
               f"asm, or pure C — never cheat asm.){RESET}")
    if show_funcs:
        out.append("")
        out.append("--- cheat-asm functions (sorted by instance count) ---")
        cheats = sorted(summary["func_categories"]["cheat"], key=lambda b: -b["cheat"])
        for b in cheats[:30]:
            out.append(f"  {b['cheat']:3d}  {b['func']:<40s}  ({b['file']})")
        if len(cheats) > 30:
            out.append(f"  ... and {len(cheats) - 30} more")
        if summary["func_categories"]["mixed"]:
            out.append("")
            out.append("--- mixed functions (have both canonical + cheat) ---")
            mx = sorted(summary["func_categories"]["mixed"], key=lambda b: -b["cheat"])
            for b in mx[:20]:
                out.append(f"  canon={b['canonical']:3d} cheat={b['cheat']:3d}  "
                           f"{b['func']:<40s}  ({b['file']})")
            if len(mx) > 20:
                out.append(f"  ... and {len(mx) - 20} more")
    return "\n".join(out)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--json", action="store_true",
                    help="emit JSON to stdout instead of human-readable")
    ap.add_argument("--summary", action="store_true",
                    help="just the summary line (for briefing integration)")
    ap.add_argument("--func", type=str, default=None,
                    help="classify a single function (show its asm blocks)")
    ap.add_argument("--src", type=str, default=None,
                    help="scan a single .c file instead of all of src/")
    args = ap.parse_args()

    files: list[Path]
    if args.src:
        files = [Path(args.src)]
        if not files[0].is_absolute():
            files[0] = ROOT / args.src
    else:
        files = sorted(SRC_DIR.glob("*.c"))

    records: list[dict] = []
    for f in files:
        records.extend(scan_file(f))

    if args.func:
        # Filter to just the requested function.
        records = [r for r in records if r.get("func") == args.func]
        if not records:
            print(f"No __asm__ blocks found in function '{args.func}' across {len(files)} file(s).")
            return 0
        for r in records:
            print(f"{r['file']}:{r['line']}  [{r['category']}]  {r['first_op'][:60]}")
        return 0

    summary = summarize(records)
    if args.json:
        print(json.dumps(summary, indent=2))
        return 0
    if args.summary:
        t = summary["totals"]
        print(f"Inline asm: {t['canonical_instances']} canonical (authentic), "
              f"{t['cheat_instances']} cheat (workaround) "
              f"across {t['gap_to_sotn_funcs']} funcs. "
              f"SOTN gap: {t['gap_to_sotn_funcs']} funcs / "
              f"{t['gap_to_sotn_instances']} instances.")
        return 0
    print(render_human(summary, color=sys.stdout.isatty()))
    return 0


if __name__ == "__main__":
    sys.exit(main())

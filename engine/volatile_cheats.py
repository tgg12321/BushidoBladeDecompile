"""Detector + stripper for `volatile` codegen-coercion cheats.

These patterns produce the same kind of byte coercion as a cheat-asm `__asm__`
barrier or a `register T x asm("$N")` pin — they force GCC's scheduler/CSE/
register-allocator to emit a specific shape that the natural source wouldn't —
yet escape the existing `engine/inlineasm.py` cheat-asm detector (which only
walks `__asm__(...)` blocks and `register T x asm("Y")` pins).

THREE coercion patterns this module catches:

1. **Alias-rename pattern (ALWAYS a cheat).** A `_v`-suffixed extern declaration
   re-aliases an existing global as `volatile` via an asm-name annotation:
       extern volatile s32 D_800A14E4_v asm("D_800A14E4");
   The original `D_800A14E4` is declared non-volatile elsewhere. The alias serves
   ONLY to defeat CSE / force re-materialization at uses of `_v`. There is no
   legitimate reason to re-declare a global as volatile under a synonym; if the
   memory were truly volatile, the primary declaration would be.

2. **Inline `*(volatile T *)&D_globalsym` casts on game-RAM symbols.** Treats a
   plain game-state global as volatile at the access site, defeating CSE/
   scheduling. Distinguished from `*(volatile T *)(ptr + offset)` (pointer-based,
   typically legitimate hardware access via a pointer the function received).

3. **Plain `extern volatile T D_globalsym;` (scalar) on game-RAM symbols.** Same
   as #2 but spelled at declaration. The `D_` prefix marks anonymous game state
   (splat-named) — `g_`-prefixed names (e.g. `g_cd_index_reg`, `g_gpu_data_reg`,
   `g_spu_dma_ctrl`) follow the project's hardware/system-global naming convention
   and are kept (the volatile reflects genuine hardware-mapped memory).

Pointer-to-volatile declarations (`extern volatile T *name;` — the VARIABLE is a
pointer, the POINTEE is volatile) are legitimate even on `D_`-named globals,
because the pointee is whatever address the pointer holds (typically a hardware
register address loaded at startup). Only SCALAR volatile is the coercion.

STRIPPING. The sandbox needs to see the function without these coercions so the
distance reflects honest pure-C. For each cheat span, the strip just removes the
`volatile` qualifier — the declarations and casts remain syntactically valid
without it, and the resulting C compiles with normal (non-coerced) semantics.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

# Project hardware/system convention: `g_`-prefixed globals are the legitimate
# volatile family (CD-ROM registers, SPU/GPU registers, DMA control, scratchpad
# pointers, ISR-shared state). Anything else with `volatile` is a candidate
# coercion.
_LEGIT_NAME_RE = re.compile(r"^g_")

# The cheat patterns target ANONYMOUS game-RAM globals (splat-named `D_xxxxxxxx`).
# A volatile cast on a LOCAL variable (e.g. `*(volatile s32 *)&ptr` where `ptr`
# is a function-local pointer being aliased) is a different idiom and not part
# of this audit. The detector requires the global-cast target to be a `D_`-
# prefixed splat name.
_GLOBAL_NAME_RE = re.compile(r"^D_[0-9A-Fa-f]+")

# Allowlist for the NARROW carve-out (user policy 2026-06-08): `extern volatile
# T G;` on globals asynchronously mutated by an IRQ/MMIO writer at use-sites
# that demonstrably require CSE-defeat (spin-wait, double-read-across-
# sequence-point, IRQ-mutated-loop-bound). Adding a symbol here requires the
# cheat-reviewer audit + the commit-message audit block per
# `.claude/rules/legitimate-volatile-interrupt-touched.md`.
#
# IMPORTANT: this allowlist applies ONLY to pattern 3 (plain
# `extern volatile T D_xxxxxxxx;` scalar declarations). Alias renames
# (pattern 1), inline `*(volatile T *)&G` casts (pattern 2), non-volatile
# alias renames (pattern 4), and macro-hidden `__asm__` (pattern 5) are NOT
# bypassed by this allowlist — they stay forbidden across the board.
_VOLATILE_EXTERN_ALLOWLIST_FILENAME = "volatile_extern_allowlist.txt"
_volatile_extern_allowlist_cache: tuple[Path, float, frozenset[str]] | None = None


def _load_volatile_extern_allowlist() -> frozenset[str]:
    """Load the per-symbol allowlist for `extern volatile T G;` carve-out.
    Returns frozenset of D_xxxxxxxx symbol names. Missing file ⇒ empty set
    (default ban applies). Cached per file mtime so the detector stays cheap
    on hot paths."""
    global _volatile_extern_allowlist_cache
    path = Path(_VOLATILE_EXTERN_ALLOWLIST_FILENAME)
    try:
        mtime = path.stat().st_mtime
    except OSError:
        _volatile_extern_allowlist_cache = None
        return frozenset()
    if (_volatile_extern_allowlist_cache is not None
            and _volatile_extern_allowlist_cache[0] == path
            and _volatile_extern_allowlist_cache[1] == mtime):
        return _volatile_extern_allowlist_cache[2]
    syms: set[str] = set()
    try:
        for raw in path.read_text(encoding="utf-8").splitlines():
            line = raw.split("#", 1)[0].strip()
            if not line:
                continue
            # First whitespace-delimited token on the line is the symbol
            tok = line.split()[0]
            syms.add(tok)
    except OSError:
        return frozenset()
    fs = frozenset(syms)
    _volatile_extern_allowlist_cache = (path, mtime, fs)
    return fs

# Pattern 1: alias rename. `extern volatile T name asm("OrigSym");`
#  - Group "decl" = the full match (for span info)
#  - Group "name" = the C identifier
#  - Group "sym"  = the asm-name target symbol
# Match form supports `T name`, `T *name`, `T **name`, `T name[]`, etc.
_ALIAS_RENAME_RE = re.compile(
    r"(?P<decl>\bextern\s+(?P<vol>volatile)\s+"
    r"(?:\w+\s+)+"           # type words
    r"\**\s*(?P<name>\w+)"   # optional pointer stars + identifier
    r"\s*(?:\[[^\]]*\])?"     # optional array suffix
    r"\s*asm\s*\(\s*\"(?P<sym>\w+)\"\s*\)\s*;)"
)

# Pattern 2: inline `*(volatile T *)&<GLOBAL>` cast (game-state global by name)
# We require the cast target to be `volatile T *` (not `volatile T **`) and the
# operand to be `&IDENT` where IDENT is a non-`g_` symbol (game state).
_GLOBAL_CAST_RE = re.compile(
    r"\*\s*\(\s*(?P<vol>volatile)\s+"
    r"(?:\w+\s+)+"            # type words
    r"\*\s*\)\s*&\s*(?P<name>\w+)"
)

# Pattern 3: plain scalar `extern volatile T <GLOBAL>;`. Distinguished from
# pointer-to-volatile (`extern volatile T *X;`) by absence of `*` between the
# type and the identifier. The identifier must be a game-RAM symbol (not `g_`).
_PLAIN_VOLATILE_RE = re.compile(
    r"(?P<decl>\bextern\s+(?P<vol>volatile)\s+"
    r"(?:\w+\s+)+"            # type words (must NOT contain pointer stars; enforced
                              # by the next char NOT being `*` — checked manually)
    r"(?P<name>\w+)"          # plain identifier (no `*` prefix)
    r"\s*(?:\[[^\]]*\])?"      # optional array suffix
    r"\s*;)"
)

# Pattern 4: NON-volatile alias rename. Same kind of cheat as the volatile
# alias-rename pattern, but without `volatile`: an extern declaration provides
# a SEPARATE C identifier (often `_TYPESUFFIX` style) for an existing global,
# typically with a DIFFERENT type — used to coerce the access width (e.g. force
# `lhu` over `lw` by aliasing an `s32` as `u16`). The alias-rename via asm()
# is the smoking gun; there is no legitimate single-TU reason to provide a
# second C handle for the same memory under a different name.
_NONVOL_ALIAS_RENAME_RE = re.compile(
    r"(?P<decl>\bextern\s+(?!volatile\b)(?:const\s+)?"
    r"(?:\w+\s+)+\**\s*(?P<name>\w+)"
    r"\s*(?:\[[^\]]*\])?"
    r"\s*asm\s*\(\s*\"(?P<sym>\w+)\"\s*\)\s*;)"
)

# Pattern 5: macro `#define NAME ... __asm__(...)`. The C preprocessor expands
# uses to `__asm__(...)` AT EVERY USE SITE, but the existing cheat-asm detector
# (engine/inlineasm.py:_strip_spans) explicitly SKIPS `#define` lines to avoid
# breaking macro uses. So a macro-hidden inline asm injection slips past
# detection — yet at every USE site, the expanded asm runs in the build.
_MACRO_ASM_DEF_RE = re.compile(
    r"(?m)^[ \t]*#[ \t]*define[ \t]+(?P<name>\w+)(?:\([^)]*\))?[^\n]*__asm__"
)

# Pattern 6: UNUSED LOCAL ARRAYS for frame-size coercion. The cheat declares a
# fixed-size array local that is never referenced in the function body, ONLY
# to make GCC reserve those bytes in the stack frame (matching a target frame
# size that natural C wouldn't produce). The names usually announce intent
# (`pad`, `_pad`, `pad2`, `pre_pad`, `_spill`, `buf`, `w`, `tail`, `sp_area`,
# `sp_buf`, `sp_dummy`). Detection requires:
#   - `T NAME[N];` at statement boundary (NOT inside a typedef/struct)
#   - 2 <= N <= 64 (single-element arrays are rare scratch; >64 is genuine)
#   - NO reference to NAME elsewhere in the body
# False positives mostly come from struct-member declarations (handled by the
# pre-context check) and from arrays passed by address to functions later
# (caught by the body-reference check).
_LOCAL_ARRAY_DECL_RE = re.compile(
    r"(?P<lead>[;{}\n][ \t]*)"
    r"(?:(?:volatile|register|static|const|signed|unsigned)\s+)*"
    r"(?P<type>(?:s32|u32|s16|u16|s8|u8|int|long|short|char))"
    r"[ \t]+(?P<name>\w+)\s*\[(?P<n>\d+)\]\s*;"
)


def _is_inside_struct(prev: str, lead_is_open_brace: bool) -> bool:
    """Walk `prev` (text before a candidate decl), tracking `{` depth and
    whether each `{` was preceded by `struct`/`union`. If `lead_is_open_brace`,
    also consider the match's leading `{` (which is a NEW open) — checking
    whether it is preceded by `struct`/`union` in `prev`.
    """
    in_struct = []
    i, n = 0, len(prev)
    while i < n:
        c = prev[i]
        if c == "{":
            j = i - 1
            while j >= 0 and prev[j] in " \t\n\r":
                j -= 1
            tok_end = j + 1
            while j >= 0 and (prev[j].isalnum() or prev[j] == "_"):
                j -= 1
            tok = prev[j + 1:tok_end]
            in_struct.append(tok in ("struct", "union"))
        elif c == "}":
            if in_struct:
                in_struct.pop()
        i += 1
    if lead_is_open_brace:
        # The match starts with `{`; check if THIS `{` is preceded by struct/union.
        j = n - 1
        while j >= 0 and prev[j] in " \t\n\r":
            j -= 1
        tok_end = j + 1
        while j >= 0 and (prev[j].isalnum() or prev[j] == "_"):
            j -= 1
        tok = prev[j + 1:tok_end]
        if tok in ("struct", "union"):
            return True
    return bool(in_struct) and in_struct[-1]


def find_unused_local_arrays(text: str, body_lo: int, body_hi: int) -> list[tuple[int, int, str]]:
    """Find array declarations inside the function body that are never
    referenced afterward. Returns (start, end, name) for each.

    Struct-member declarations are excluded by walking the body up to the
    candidate and tracking which `{` are preceded by `struct`/`union`. The
    match's leading character (a `{` opening the struct, or a `;` ending the
    previous member) is also considered.
    """
    out = []
    body = text[body_lo:body_hi]
    for m in _LOCAL_ARRAY_DECL_RE.finditer(body):
        name = m.group("name")
        n = int(m.group("n"))
        if not (2 <= n <= 64):
            continue
        decl_start = body_lo + m.start("type")
        decl_end = body_lo + m.end()
        # Determine if match's lead is `{` (struct body open) or `;`/`}` (after
        # a sibling statement).
        lead_char = body[m.start()] if m.start() < len(body) else ""
        prev = body[:m.start()]
        if _is_inside_struct(prev, lead_char == "{"):
            continue
        # Reference check: name must not appear in body outside this decl
        rest_before = body[:m.start("name")]
        rest_after = body[m.end():]
        full_search = rest_before + rest_after
        if re.search(rf"\b{re.escape(name)}\b", full_search):
            continue
        out.append((decl_start, decl_end, name))
    return out


# `(void)&local;` — the address-coercion frame-coercion variant. See
# `dead-vars-local-array.md` for the array form; this is the scalar form
# that defeats GCC's DCE by taking the address. There is no legitimate
# pure-C reason to write `(void)&local;` — even taking the address
# legitimately is `&local` (no void cast). The discarded-address-of is
# pure DCE-defeat, used to reserve frame bytes the function doesn't
# semantically need.
#
# Identified via the func_8007CE0C agent investigation (2026-06-01): the
# agent reached sandbox 22 (down from 38) by adding
# `s32 stk_a,b,c,d; (void)&stk_a; ...` to inflate the frame to target's
# 80-byte size. Same intent as the array form, different syntax. Closed
# as a loophole because the function still wasn't pure-C-matchable
# (residual 22 was unrelated codegen) and the policy is no new
# cheat-tolerant patterns.
_ADDR_COERCION_RE = re.compile(
    r"(?m)^[ \t]*\(\s*void\s*\)\s*&\s*(?P<name>\w+)\s*;[ \t]*$"
)


def find_addr_coerced_locals(text: str, body_lo: int, body_hi: int) -> list[tuple[int, int, str]]:
    """Find `(void)&<name>;` statements inside the function body. There is no
    legitimate pure-C reason to discard an address-of, so any such statement
    is a frame-coercion cheat (see dead-vars-local-array.md, expanded
    catalog 2026-06-01).

    Conservative: catches the `(void)&name;` pattern unconditionally. We
    verified (2026-06-01) that the BB2 src/*.c has ZERO existing instances,
    so there is no false-positive risk on the current codebase. Future
    edits that introduce the pattern intentionally must justify each one
    (or the gate refuses completion).
    """
    out = []
    body = text[body_lo:body_hi]
    for m in _ADDR_COERCION_RE.finditer(body):
        start = body_lo + m.start()
        end = body_lo + m.end()
        out.append((start, end, m.group("name")))
    return out


# Lowercase `asm(...)` cheat blocks at STATEMENT scope. The legacy detector
# regex (tools/classify_inline_asm.py:ASM_KEYWORD_RE) excludes bare `asm` to
# avoid false-positives on the register-pin pattern `register T x asm("$N")`.
# Result: lowercase `asm volatile("" ::: "memory")` and similar statement-
# scope cheat-asm blocks bypassed the entire cheat-asm gate. The 2026-06-02
# thorough audit found 5 COMPLETED-C functions affected.
#
# This detector catches STATEMENT-SCOPE lowercase-asm (line-starts with
# `asm`), which distinguishes it from register-pin declarations (which
# start with `register`).
_LOWERCASE_ASM_RE = re.compile(
    r"(?m)^[ \t]*(?:asm)\s*(?:volatile|__volatile__)?\s*\("
)


def find_lowercase_asm_cheats(text: str, body_lo: int, body_hi: int) -> list[tuple[int, int]]:
    """Find statement-scope `asm(...)` / `asm volatile(...)` blocks inside
    the function body. These are inline-asm cheats that bypass the
    ASM_KEYWORD_RE detector (which excludes bare `asm` for register-pin
    safety). Returns list of (start, end) covering the asm-block including
    its trailing semicolon."""
    body = text[body_lo:body_hi]
    out: list[tuple[int, int]] = []
    for m in _LOWERCASE_ASM_RE.finditer(body):
        depth = 1
        i = m.end()
        while i < len(body) and depth > 0:
            c = body[i]
            if c == "(":
                depth += 1
            elif c == ")":
                depth -= 1
            i += 1
        if depth != 0:
            continue
        while i < len(body) and body[i] in " \t":
            i += 1
        if i < len(body) and body[i] == ";":
            i += 1
        out.append((body_lo + m.start(), body_lo + i))
    return out


# Volatile-typed scalar unused locals. Mirror of find_unused_local_arrays
# and find_void_discard_unused_locals for the scalar-volatile case:
#   volatile s32 pad;        // never referenced — frame coercion
#   volatile s32 sp_pad;     // name announces intent
# Excluded (legitimate):
#   volatile s32 *ptr;       // pointer-to-volatile (hardware access)
#   volatile s32 i; for (i = 0; ...) { ... }   // calibration loop counter
#   volatile s32 tmp; tmp = *hw_reg;   // hardware fence read
# Detection requires NO references to the name elsewhere in the body
# (declaration-only). Calibration loops and fence reads have additional
# references and pass the carve-out.
_VOLATILE_LOCAL_DECL_RE = re.compile(
    r"(?m)^[ \t]*volatile\s+"
    r"(?P<type>(?:s8|s16|s32|s64|u8|u16|u32|u64|int|char|short|long|float|double|signed|unsigned)"
    r"(?:\s+(?:int|long|short|char))*)"
    r"\s+(?P<name>\w+)\s*"
    r"(?:=\s*[^;]+)?"
    r"\s*;[ \t]*$"
)


def find_volatile_unused_locals(text: str, body_lo: int, body_hi: int) -> list[tuple[int, int, str]]:
    """Find `volatile <scalar-type> <name>;` declarations whose name has no
    other references in the body. Pointer-to-volatile (`volatile T *name;`)
    is excluded by the regex. Calibration loops and hardware fence reads
    pass via the reference-count carve-out."""
    body = text[body_lo:body_hi]
    out: list[tuple[int, int, str]] = []
    for m in _VOLATILE_LOCAL_DECL_RE.finditer(body):
        name = m.group("name")
        decl_name_span = (m.start("name"), m.end("name"))
        ref_re = re.compile(rf"\b{re.escape(name)}\b")
        other_refs = [
            r for r in ref_re.finditer(body)
            if not (decl_name_span[0] <= r.start() < decl_name_span[1])
        ]
        if len(other_refs) == 0:
            out.append((body_lo + m.start(), body_lo + m.end(), name))
    return out


# `if (1) { ... }` always-true scaffold wrapping. The body has real content;
# the wrapping construct exists to influence basic-block layout / scheduling.
# Identified by the 2026-06-02 thorough audit (3 COMPLETED-C affected).
_IF_ONE_RE = re.compile(r"\bif\s*\(\s*1\s*\)\s*\{")


def find_always_true_if_scaffolds(text: str, body_lo: int, body_hi: int) -> list[tuple[int, int]]:
    """Find `if (1) { ... }` statements in the function body."""
    body = text[body_lo:body_hi]
    out: list[tuple[int, int]] = []
    for m in _IF_ONE_RE.finditer(body):
        depth = 1
        i = m.end()
        while i < len(body) and depth > 0:
            c = body[i]
            if c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
            i += 1
        if depth != 0:
            continue
        out.append((body_lo + m.start(), body_lo + i))
    return out


# `do { } while (0);` — RETIRED 2026-06-04 per SOTN-master-branch evidence.
# Previously: this detector flagged empty `do { } while (0);` as a syntactic
# construct GCC's DCE removes but whose existence steers analysis upstream
# of DCE (identified by 2026-06-02 thorough audit, 5+1 COMPLETED-C affected).
#
# Re-evaluation 2026-06-04 (memory/project/sotn-do-while-zero-research-2026-06-04.md):
# SOTN master ships 18+ instances of `do { ... } while (0);` (both empty and
# non-empty body), including explicit `// FAKE` annotations in sprintf.c. Two
# commits (511fdcfc4, 3aa8b65c5) explicitly accept the construct in code
# review. The construct is a SOTN-accepted matching technique, not a
# cheat-by-spelling. The detector is retired; the function symbol stays as
# a no-op so the rest of the pipeline (which iterates this and ignores empty
# results) continues to compile, and the historical tests still exercise the
# call site.
_EMPTY_DO_WHILE_ZERO_RE = re.compile(
    r"\bdo\s*\{\s*(?:(?://[^\n]*|/\*[\s\S]*?\*/)\s*)*\}\s*while\s*\(\s*0\s*\)\s*;"
)


def find_empty_do_while_zero(text: str, body_lo: int, body_hi: int) -> list[tuple[int, int]]:
    """RETIRED 2026-06-04 -- returns [] always.

    See module-level comment above. SOTN ships this construct as a matching
    technique; flagging it caused BB2's cheat-reviewer to FAIL legitimate
    closing forms (e.g. cpu_check_same_dir_timer's 2026-06-04 permuter find).
    The regex is kept around as documentation for future re-evaluations but
    is not invoked.
    """
    return []


# Empty-body `if (cond) { }` — dead read of cond used purely for codegen
# coercion (forces GCC to load cond at a specific point without doing
# anything with it, influencing scheduling / register allocation).
#
# Identified by the 2026-06-02 cheat-by-spelling audit on 3 affected
# COMPLETED-C functions (func_80072E10 with 4× `if (D_800A3580) {}`,
# func_8003D7B4 with `if (!val) {}`, func_80049A2C with `if (a1_val) {}`).
# Same intent as the dead-conditional-store / Lever D / dead-vars-local-
# array family: a code construct with no semantic purpose, written
# purely to influence GCC's pre-DCE analysis.
#
# Detection: brace-matched scan for `if (...) { ... }` where the body
# contains only whitespace + comments AND there is no `else` clause
# following the close brace. (If/else with empty if-body IS legitimate
# sense-flipped logic; only empty-if-without-else is the cheat.)


def find_empty_if_dead_reads(text: str, body_lo: int, body_hi: int) -> list[tuple[int, int, str]]:
    """Find `if (cond) { }` statements with empty bodies and NO else clause
    inside the function body. Returns list of (start, end, cond_text)."""
    body = text[body_lo:body_hi]
    out: list[tuple[int, int, str]] = []
    for if_m in re.finditer(r"\bif\s*\(", body):
        # Balance parens to find the condition's close.
        depth = 1
        i = if_m.end()
        while i < len(body) and depth > 0:
            c = body[i]
            if c == "(":
                depth += 1
            elif c == ")":
                depth -= 1
            i += 1
        if depth != 0:
            continue
        cond_text = body[if_m.end():i - 1].strip()

        # Skip whitespace + comments to find the `{`.
        j = i
        while j < len(body):
            if body[j] in " \t\n\r":
                j += 1
            elif body[j:j + 2] == "//":
                while j < len(body) and body[j] != "\n":
                    j += 1
            elif body[j:j + 2] == "/*":
                k = body.find("*/", j + 2)
                if k < 0:
                    j = len(body)
                else:
                    j = k + 2
            else:
                break
        if j >= len(body) or body[j] != "{":
            continue
        open_brace = j

        # Balance braces to find the close.
        depth = 1
        k = j + 1
        while k < len(body) and depth > 0:
            c = body[k]
            if c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
            k += 1
        if depth != 0:
            continue
        close_brace = k - 1

        # Body must be empty (only whitespace + comments).
        inner = body[open_brace + 1:close_brace]
        stripped = re.sub(r"//[^\n]*|/\*[\s\S]*?\*/|\s", "", inner)
        if stripped:
            continue

        # Must NOT be followed by `else` (which would be legitimate
        # sense-flipped logic, not a dead-read cheat).
        after = body[k:k + 20].lstrip()
        if re.match(r"\belse\b", after):
            continue

        out.append((body_lo + if_m.start(), body_lo + k, cond_text))
    return out


# `(void) <name>;` — discard cast on a bare identifier (no `&`). For locals,
# this is the scalar variant of the `(void) &<name>;` frame-coercion cheat:
# the declaration reserves a frame slot, the discard cast defeats DCE, the
# variable has no semantic purpose.
#
# For PARAMETERS, the standard C convention `(void) param;` suppresses
# unused-parameter warnings — that's legitimate, NOT flagged.
#
# Identified by the 2026-06-02 cheat-by-spelling audit; complements the
# existing `find_addr_coerced_locals` (which requires `&`) to close the
# scalar-discard loophole.
_VOID_DISCARD_NO_ADDR_RE = re.compile(
    r"(?m)^[ \t]*\(\s*void\s*\)\s*(?P<name>\w+)\s*;[ \t]*$"
)


def find_void_discard_unused_locals(text: str, body_lo: int, body_hi: int,
                                    params: list[str]) -> list[tuple[int, int, str]]:
    """Find `(void) <name>;` discard-cast statements where <name> is a local
    declared in the function body with no semantic use (only the declaration
    + discards). Parameters are explicitly NOT flagged (warning suppression
    is legitimate). Returns list of (start, end, name)."""
    body = text[body_lo:body_hi]
    discards = [(m.start(), m.end(), m.group("name"))
                for m in _VOID_DISCARD_NO_ADDR_RE.finditer(body)]
    if not discards:
        return []

    out: list[tuple[int, int, str]] = []
    for ds, de, name in discards:
        if name in params:
            continue  # Legitimate parameter-warning suppression
        # Confirm <name> is declared as a local in the body (presence of a
        # declaration statement like `T name;`, `T name[N];`, `T *name;`).
        # If absent, the name probably refers to a global / file-scope
        # symbol — skip (different family of cheat if any).
        decl_re = re.compile(
            r"(?m)^[ \t]*"
            r"(?:(?:s8|s16|s32|s64|u8|u16|u32|u64|int|char|short|long|float|"
            r"double|register|static|const|volatile|signed|unsigned|void|"
            r"struct|union|enum)\b\s+|)+"
            r"\**\s*"
            rf"\b{re.escape(name)}\b"
            r"(?:\s*\[\s*\d*\s*\])?\s*"
            r"(?:=[^;]+)?\s*;"
        )
        if not decl_re.search(body):
            continue
        # Count references to <name> that are NOT inside a `(void) name;`
        # discard for this same name.
        ref_re = re.compile(rf"\b{re.escape(name)}\b")
        same_name_discards = [(s, e) for s, e, n in discards if n == name]
        other_refs = [
            r for r in ref_re.finditer(body)
            if not any(s <= r.start() < e for s, e in same_name_discards)
        ]
        # At most 1 reference outside discards = the declaration. The
        # variable serves no semantic purpose.
        if len(other_refs) <= 1:
            out.append((body_lo + ds, body_lo + de, name))
    return out


# Dead-conditional-store: an assignment `<var> = <rhs>;` inside a control-flow
# block (if/else/switch/loop/do) where the IDENTICAL `<var> = <rhs>;` also
# appears at the enclosing function-body scope after the block. The inner
# store is dead — overwritten unconditionally by the outer store. Same family
# as Lever D (dead self-assignment of parameters) and the (void)&local
# frame-coercion: a code construct with no semantic purpose, written purely
# to influence GCC's register-allocation analysis via the flow graph.
#
# Identified by func_8007B844's directed-permuter session (2026-06-01) — the
# permuter found `u32 *p; if (debug) {...; p = ot;} ...; p = ot;` as a sandbox-0
# closing form. The user's standing policy "cheats by any spelling are
# forbidden, full stop" closes this loophole alongside the existing
# unused-local-array / dead-param-assign / (void)&local detectors.
#
# Detection approach: lexical statement-level scan with brace-depth tracking.
# Find all statements of form `<var> = <simple-rhs>;` (LHS = single
# identifier, RHS = single token or simple expr). Group by (var, rhs).
# If any signature has occurrences at BOTH depth 1 (function-body scope)
# AND depth > 1 (inside a sub-block), flag the inner ones. False positives
# are rare for this pattern in our codebase — legitimate code that genuinely
# assigns the same value twice typically has different RHS forms or uses
# the value between the assignments.
_DEAD_COND_STORE_ASSIGN_RE = re.compile(
    # Match a statement-level `<var> = <simple-rhs>;`
    #   var: single identifier (no struct member / array access on LHS)
    #   rhs: single identifier OR simple typed-constant; NOT a function call,
    #        binary op, or anything with nested () / [] / + / etc.
    r"^[ \t]*(?P<var>\w+)\s*=\s*(?P<rhs>[\w&\*]+)\s*;[ \t]*(?:(?://|/\*)[^\n]*)?$",
    re.MULTILINE,
)

_FAKE_ANNOT_RE = re.compile(r"\bFAKE\b", re.IGNORECASE)


def _stmt_fake_annotated(body: str, start: int, end: int) -> bool:
    """True when the statement at [start, end) carries the sanctioned
    `/* FAKE: ... */` / `// FAKE` annotation — trailing on the statement's
    own line, or as a comment on the line immediately above. Sanctioned
    last-resort exceptions (dead-store-fake-exception.md /
    named-local-fake-exception.md / pointer-alias-fake-exception.md,
    owner ruling 2026-07-01) require this annotation; annotated instances
    bypass the Lever-D detectors (and therefore also the cheat-strip, so
    they contribute to the honest sandbox distance like any other source
    line). Un-annotated instances stay flagged."""
    line_start = body.rfind("\n", 0, start) + 1
    line_end = body.find("\n", end)
    if line_end < 0:
        line_end = len(body)
    # The statement's own line (the trailing annotation may sit inside the
    # detector's regex match, so scan the full line, not just past `end`).
    if _FAKE_ANNOT_RE.search(body[line_start:line_end]):
        return True
    nl = body.rfind("\n", 0, start)
    if nl <= 0:
        return False
    prev_start = body.rfind("\n", 0, nl) + 1
    prev_line = body[prev_start:nl].strip()
    return (prev_line.startswith("//") or prev_line.startswith("/*")) and \
        bool(_FAKE_ANNOT_RE.search(prev_line))


def find_dead_conditional_stores(text: str, body_lo: int, body_hi: int) -> list[tuple[int, int, str, str]]:
    """Find `<var> = <rhs>;` statements inside a sub-block where the same
    `<var> = <rhs>;` appears unconditionally at the enclosing scope after
    the block. The inner store is dead and exists only to influence RA via
    flow analysis — a cheat in the same family as forbidden Lever D
    (register-alloc-pure-c.md) and `(void)&local` (this file). Returns
    list of (start, end, var, rhs).

    Detection is lexical (brace-depth tracking), not full CFG analysis;
    handles the agent-discovered shape:
      `if (cond) { ...; var = expr; } ...; var = expr;`
    and switch/loop variants by structural symmetry."""
    body = text[body_lo:body_hi]

    # Compute brace depth at every byte offset in the body.
    # Function body itself opens at depth 0 → 1; sub-blocks bump to >= 2.
    depths = [0] * len(body)
    d = 0
    in_str = False
    in_chr = False
    in_line_comment = False
    in_block_comment = False
    i = 0
    while i < len(body):
        c = body[i]
        c2 = body[i:i + 2]
        depths[i] = d
        if in_line_comment:
            if c == "\n":
                in_line_comment = False
            i += 1
            continue
        if in_block_comment:
            if c2 == "*/":
                in_block_comment = False
                i += 2
                continue
            i += 1
            continue
        if in_str:
            if c == "\\" and i + 1 < len(body):
                i += 2
                continue
            if c == '"':
                in_str = False
            i += 1
            continue
        if in_chr:
            if c == "\\" and i + 1 < len(body):
                i += 2
                continue
            if c == "'":
                in_chr = False
            i += 1
            continue
        if c2 == "//":
            in_line_comment = True
            i += 2
            continue
        if c2 == "/*":
            in_block_comment = True
            i += 2
            continue
        if c == '"':
            in_str = True
        elif c == "'":
            in_chr = True
        elif c == "{":
            d += 1
        elif c == "}":
            d -= 1
        i += 1

    # Find all simple assignment statements + record their depth at the
    # var character's offset.
    # Map (var, rhs) → list of (start_offset, end_offset, depth_at_var)
    by_sig: dict[tuple[str, str], list[tuple[int, int, int]]] = {}
    for m in _DEAD_COND_STORE_ASSIGN_RE.finditer(body):
        var_off = m.start("var")
        depth_here = depths[var_off] if var_off < len(depths) else 0
        if depth_here < 1:
            continue
        key = (m.group("var"), m.group("rhs").strip())
        by_sig.setdefault(key, []).append((m.start(), m.end(), depth_here))

    # Flag inner candidates that satisfy the FULL dead-store signature:
    #   1. The inner is at depth > 1 (inside a sub-block)
    #   2. A SUBSEQUENT outer occurrence at depth 1 exists (offset > inner.end)
    #   3. The inner's enclosing block does NOT early-exit (no goto / return /
    #      break / continue between inner.end and the block's closing brace).
    #      If it does, the outer is never reached after the inner runs — so
    #      the inner store is alive (used along that exit path).
    #   4. No reference to <var> between inner.end and outer.start (in the
    #      textual gap at function-body scope).
    #
    # The order + early-exit constraints distinguish the cheat pattern
    # (`if (cond) { v=x; } ...; v=x;` — inner dead, outer always reached)
    # from common legitimate patterns:
    #   - "init at function entry, re-init in branches" (outer BEFORE inner)
    #   - "set then goto handler" (inner is alive because goto bypasses outer)
    exit_keyword_re = re.compile(r"\b(goto|return|break|continue)\b")
    out: list[tuple[int, int, str, str]] = []
    for (var, rhs), occs in by_sig.items():
        occs_sorted = sorted(occs)
        var_ref_re = re.compile(rf"\b{re.escape(var)}\b")
        for i, (s, e, d) in enumerate(occs_sorted):
            if d <= 1:
                continue  # only inner-depth candidates
            # Find the FIRST outer occurrence (depth 1) at offset > e.
            outer = None
            for j in range(i + 1, len(occs_sorted)):
                s2, e2, d2 = occs_sorted[j]
                if d2 == 1 and s2 > e:
                    outer = (s2, e2)
                    break
            if outer is None:
                continue
            # Find inner's enclosing-block close by walking depths forward
            # until we drop below `d`.
            block_close = len(body)
            for k in range(e, len(depths)):
                if depths[k] < d:
                    block_close = k
                    break
            # Early-exit check: any goto/return/break/continue between
            # inner.end and block_close means the inner is alive along that
            # exit path (outer is not reached).
            tail = body[e:block_close]
            if exit_keyword_re.search(tail):
                continue
            # Use-check: any reference to <var> between inner.end and
            # outer.start means inner is read before outer overwrites.
            gap = body[e:outer[0]]
            if var_ref_re.search(gap):
                continue
            # Sanctioned-exception bypass (2026-07-01): FAKE-annotated
            # instances are legitimate last-resort levers, not cheats.
            if _stmt_fake_annotated(body, s, e):
                continue
            out.append((body_lo + s, body_lo + e, var, rhs))
    out.sort()
    return out


def find_dead_param_assigns(text: str, body_lo: int, body_hi: int,
                            params: list[str]) -> list[tuple[int, int, str]]:
    """Find `param = 0;` or `param = param;` statements where `param` is a
    function parameter AND is never referenced after the statement in the
    function body. This is `register-alloc-pure-c.md` Lever D — a dead store
    that GCC DCE's but uses to break the param-reg value association.

    Distinguished from legitimate local-variable initialization (where the
    LHS is NOT a function parameter — local-init is normal C) by requiring
    the LHS to be in the function's parameter name list."""
    if not params:
        return []
    out = []
    body = text[body_lo:body_hi]
    param_pat = "|".join(re.escape(p) for p in params)
    rx = re.compile(
        rf"(?m)^[ \t]*(?P<name>{param_pat})\s*=\s*(?:0|\1)\s*;[ \t]*(?:(?://|/\*)[^\n]*)?$")
    for m in rx.finditer(body):
        name = m.group("name")
        rest_after = body[m.end():]
        if re.search(rf"\b{re.escape(name)}\b", rest_after):
            continue  # used later → not dead
        # Sanctioned-exception bypass (2026-07-01): FAKE-annotated
        # instances are legitimate last-resort levers, not cheats.
        if _stmt_fake_annotated(body, m.start(), m.end()):
            continue
        out.append((body_lo + m.start(), body_lo + m.end(), name))
    return out


def _extract_func_params(text: str, span: tuple[int, int]) -> list[str]:
    """Extract C identifier list from the parameter list of the function
    whose definition starts at span[0]. Returns [] if extraction fails."""
    start = span[0]
    # The first `(` after `start` opens the param list (assuming a real def).
    open_paren = text.find("(", start)
    if open_paren < 0 or open_paren > span[1]:
        return []
    # Walk to matching `)`.
    depth, i, n = 1, open_paren + 1, len(text)
    while i < n and depth > 0:
        c = text[i]
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                break
        i += 1
    if depth != 0:
        return []
    raw = text[open_paren + 1:i]
    if not raw.strip() or raw.strip() == "void":
        return []
    out = []
    for arg in raw.split(","):
        arg = arg.strip()
        # Last identifier in the arg is the param name (skip type words).
        toks = re.findall(r"[A-Za-z_]\w*", arg)
        if toks:
            # Filter known type words; last remaining token is the name.
            type_words = {"s8", "u8", "s16", "u16", "s32", "u32", "s64", "u64",
                          "int", "long", "short", "char", "void", "float",
                          "double", "signed", "unsigned", "const", "volatile",
                          "register", "extern", "static"}
            name_tok = None
            for t in toks:
                if t not in type_words:
                    name_tok = t  # last non-type-word is the name
            if name_tok and re.match(r"^[A-Za-z_]\w*$", name_tok):
                out.append(name_tok)
    return out


def _is_legit_name(name: str) -> bool:
    return bool(_LEGIT_NAME_RE.match(name))


def _decl_has_pointer(text: str, vol_start: int) -> bool:
    """Return True if the type span between `volatile` and the identifier
    contains a `*` — i.e. it is a pointer-to-volatile declaration (legitimate).
    `vol_start` is the position of `volatile` in `text`."""
    # Scan forward from past `volatile ` to the first identifier; if a `*` appears,
    # it's a pointer type. Skip whitespace and word chars (type names).
    i = vol_start + len("volatile")
    n = len(text)
    while i < n and text[i] in " \t":
        i += 1
    # Walk type-name tokens followed by spaces; track whether we see `*`.
    while i < n:
        c = text[i]
        if c == "*":
            return True
        if c == "[" or c == ";" or c == "=" or c == "," or c == "(":
            return False
        if c.isalnum() or c == "_":
            # consume identifier
            while i < n and (text[i].isalnum() or text[i] == "_"):
                i += 1
            # space/star follows?
            while i < n and text[i] in " \t":
                i += 1
            # if next is `*` -> pointer
            if i < n and text[i] == "*":
                return True
            # if next is identifier (another type word like `s32`) keep scanning;
            # if next is identifier and followed by `;` or `[`, it's the var name
            # and we've reached the end of the type without seeing `*`.
            # We'll check by peeking ahead at this point.
            # Peek: is the next token the variable name? Heuristic: if there is
            # ONE remaining word followed by `;`/`[`, it's the var name.
            j = i
            while j < n and (text[j].isalnum() or text[j] == "_"):
                j += 1
            while j < n and text[j] in " \t":
                j += 1
            if j < n and text[j] in ";[=,()":
                return False  # no `*` before var name -> scalar volatile
            continue
        return False
    return False


def find_alias_renames(text: str) -> list[tuple[int, int, str]]:
    """All `extern volatile T NAME asm("SYM");` alias renames where NAME != SYM.
    Returns (start, end, vol_keyword_index)."""
    out = []
    for m in _ALIAS_RENAME_RE.finditer(text):
        name, sym = m.group("name"), m.group("sym")
        if name == sym:
            continue  # same name; e.g. `extern volatile T g_x asm("g_x");`
        # legit-name on the ALIASED side? If sym is g_xxx, this is still cheat
        # (the alias gives access via a non-`g_` name). Treat as cheat regardless.
        vol_idx = m.start("vol")
        out.append((m.start(), m.end(), vol_idx))
    return out


def find_global_volatile_casts(text: str) -> list[tuple[int, int, str]]:
    """All `*(volatile T *)&<NAME>` casts where NAME is a splat-named global
    (D_xxxxxxxx pattern — anonymous game-RAM symbol). Returns (cast_start,
    cast_end_at_name, volatile_keyword_start)."""
    out = []
    for m in _GLOBAL_CAST_RE.finditer(text):
        name = m.group("name")
        if _is_legit_name(name):
            continue
        if not _GLOBAL_NAME_RE.match(name):
            continue  # exclude locals + non-splat globals
        out.append((m.start(), m.end(), m.start("vol")))
    return out


def find_nonvolatile_alias_renames(text: str) -> list[tuple[int, int, int]]:
    """All `extern T NAME asm("SYM");` declarations (NO `volatile`) where
    NAME != SYM. The cheat anchor is the position of `asm` in the decl — we
    return (start, end, asm_start) so the strip can remove the `asm("SYM")`
    annotation (turning the alias into a separate scoped declaration with the
    SAME C-level name — link error is acceptable since the sandbox sees the
    function compile-failing in this case)."""
    out = []
    for m in _NONVOL_ALIAS_RENAME_RE.finditer(text):
        name, sym = m.group("name"), m.group("sym")
        if name == sym:
            continue
        # Skip function-name aliases (decl has `()` between type and asm — but
        # _NONVOL_ALIAS_RENAME_RE doesn't capture those because the regex
        # requires the identifier+optional-array to come BEFORE asm(...). We
        # double-check by ensuring there's no `(` between the matched NAME
        # and the asm keyword.
        between = text[m.start("name") + len(name):text.find("asm", m.start("name"))]
        if "(" in between:
            continue
        # Find asm keyword position within the match
        asm_idx = text.find("asm", m.start())
        if asm_idx < 0:
            continue
        out.append((m.start(), m.end(), asm_idx))
    return out


def find_macro_asm_defs(text: str) -> list[tuple[int, int, str]]:
    """All `#define NAME ... __asm__(...)` macro definitions. Returns
    (def_start, def_end_eol, macro_name). The strip removes the macro
    definition; uses of the macro will fail compilation, surfacing the cheat
    instead of hiding it."""
    out = []
    for m in _MACRO_ASM_DEF_RE.finditer(text):
        name = m.group("name")
        # Find end of line
        eol = text.find("\n", m.start())
        if eol == -1:
            eol = len(text)
        out.append((m.start(), eol, name))
    return out


def find_plain_volatile_externs(text: str) -> list[tuple[int, int, str]]:
    """All `extern volatile T NAME;` declarations where:
      - The declaration is SCALAR (no `*` between type and NAME),
      - NAME is `D_xxxxxxxx` (splat-named anonymous global, i.e. game RAM),
      - NAME is NOT in `volatile_extern_allowlist.txt` (the IRQ-touched-
        global carve-out, user policy 2026-06-08 — see
        `.claude/rules/legitimate-volatile-interrupt-touched.md`).
    Returns (start, end, vol_keyword_index).

    The allowlist applies ONLY to this pattern (plain scalar extern). Alias
    renames, inline volatile casts, non-volatile alias renames, and
    macro-hidden __asm__ are unchanged — they stay forbidden regardless of
    allowlist membership."""
    allowlisted = _load_volatile_extern_allowlist()
    out = []
    for m in _PLAIN_VOLATILE_RE.finditer(text):
        name = m.group("name")
        if _is_legit_name(name):
            continue
        if not _GLOBAL_NAME_RE.match(name):
            continue
        # Verify scalar (no pointer star between type and NAME)
        if _decl_has_pointer(text, m.start("vol")):
            continue
        if name in allowlisted:
            continue  # narrow IRQ-touched-global carve-out
        out.append((m.start(), m.end(), m.start("vol")))
    return out


def find_all_cheats(text: str) -> list[tuple[int, int, str]]:
    """All categories combined, sorted by start position. Each tuple is
    (cheat_span_start, cheat_span_end_or_anchor, strip_anchor_position).

    For volatile cheats the strip_anchor is the `volatile` keyword position
    (which the stripper removes). For non-volatile alias renames it is the
    `asm` keyword position. For macro `__asm__` definitions it is the `#`
    column-0 position. For unused-locals + dead-param-assigns it is the
    statement start (the whole statement is stripped).

    Spans may OVERLAP in pathological text; the strip path tolerates this by
    keyword-position dedup."""
    out = []
    out.extend(find_alias_renames(text))
    out.extend(find_global_volatile_casts(text))
    out.extend(find_plain_volatile_externs(text))
    # Non-volatile category (asm-anchored, not volatile-anchored)
    for s, e, asm_pos in find_nonvolatile_alias_renames(text):
        out.append((s, e, asm_pos))
    # Macro asm definitions (line-anchored)
    for s, e, _name in find_macro_asm_defs(text):
        out.append((s, e, s))
    # Per-function unused-locals + dead-param-assigns. We need each func's body
    # span and parameter list, so we iterate over function definitions.
    for func_def_match in re.finditer(
        r"(?m)^[A-Za-z_][\w \t\*]*\b([A-Za-z_]\w*)\s*\(",
        text,
    ):
        fname = func_def_match.group(1)
        span = _func_body_span(text, fname)
        if span is None:
            continue
        body_lo, body_hi = span
        # Skip if this match isn't actually the def we located (multi-def file safety)
        if func_def_match.start() != body_lo:
            continue
        params = _extract_func_params(text, span)
        for s, e, _name in find_unused_local_arrays(text, body_lo, body_hi):
            out.append((s, e, s))
        for s, e, _name in find_addr_coerced_locals(text, body_lo, body_hi):
            out.append((s, e, s))
        for s, e, _name in find_void_discard_unused_locals(text, body_lo, body_hi, params):
            out.append((s, e, s))
        for s, e, _name in find_volatile_unused_locals(text, body_lo, body_hi):
            out.append((s, e, s))
        for s, e in find_lowercase_asm_cheats(text, body_lo, body_hi):
            out.append((s, e, s))
        for s, e in find_always_true_if_scaffolds(text, body_lo, body_hi):
            out.append((s, e, s))
        for s, e in find_empty_do_while_zero(text, body_lo, body_hi):
            out.append((s, e, s))
        for s, e, _cond in find_empty_if_dead_reads(text, body_lo, body_hi):
            out.append((s, e, s))
        for s, e, _var, _rhs in find_dead_conditional_stores(text, body_lo, body_hi):
            out.append((s, e, s))
        for s, e, _name in find_dead_param_assigns(text, body_lo, body_hi, params):
            out.append((s, e, s))
    out.sort()
    return out


_VOLATILE_KW_RE = re.compile(r"\bvolatile\b")


def strip_volatile_cheats_file(text: str) -> tuple[str, int]:
    """Return (modified text, count) with every detected coercion cheat
    NEUTRALIZED — volatile/non-volatile aliases, inline volatile casts, plain
    volatile externs, and `#define`-hidden `__asm__` blocks. The neutralization
    is pattern-specific:

      - volatile qualifier on alias / cast / plain extern → keyword removed
        (declaration / expression survives without the volatile semantics)
      - non-volatile `asm("SYM")` alias annotation → `asm("SYM")` removed
        (the C identifier becomes an unresolved extern, surfacing the cheat
        via link error rather than silently compiling)
      - macro `__asm__(...)` definition → the entire `#define` line is
        commented out so the macro becomes undefined; uses fail compilation

    The strip is byte-position-driven; edits are applied in REVERSE order to
    keep positions valid."""
    # Volatile keyword removals (positions for casts / volatile aliases /
    # plain volatile externs).
    vol_positions: set[int] = set()
    for s, e, vp in find_alias_renames(text):
        vol_positions.add(vp)
    for s, e, vp in find_global_volatile_casts(text):
        vol_positions.add(vp)
    for s, e, vp in find_plain_volatile_externs(text):
        vol_positions.add(vp)

    # Non-volatile asm() alias annotations to remove (span: from `asm` to `)`).
    nv_alias_spans: list[tuple[int, int]] = []
    for s, e, asm_pos in find_nonvolatile_alias_renames(text):
        # Find the closing `)` of asm("...")
        paren_open = text.find("(", asm_pos)
        if paren_open < 0:
            continue
        depth = 1
        i = paren_open + 1
        in_str = False
        while i < len(text) and depth > 0:
            c = text[i]
            if in_str:
                if c == "\\":
                    i += 2
                    continue
                if c == "\"":
                    in_str = False
            elif c == "\"":
                in_str = True
            elif c == "(":
                depth += 1
            elif c == ")":
                depth -= 1
            i += 1
        if depth != 0:
            continue
        # Trim trailing whitespace BEFORE asm (so we remove ` asm("...")` cleanly)
        strip_start = asm_pos
        while strip_start > 0 and text[strip_start - 1] in " \t":
            strip_start -= 1
        nv_alias_spans.append((strip_start, i))

    # Macro `__asm__` definitions to comment out (full line).
    macro_spans: list[tuple[int, int, str]] = []
    macro_usage_spans: list[tuple[int, int]] = []
    for s, e, name in find_macro_asm_defs(text):
        macro_spans.append((s, e, name))
        # Strip macro USAGES too: an undefined macro name appearing as
        # `NAME;` becomes a tentative global (`.comm NAME,4,4`) which crashes
        # maspsx and silently truncates the whole-file object. Match the
        # macro name as a standalone statement/expression and neutralize it.
        # Pattern: `NAME` followed by optional `(args)` and a `;` (with
        # surrounding whitespace).
        usage_re = re.compile(
            r"\b" + re.escape(name) + r"\b(?:\s*\([^)]*\))?\s*;"
        )
        for um in usage_re.finditer(text):
            # Skip the definition itself
            if s <= um.start() < e:
                continue
            macro_usage_spans.append((um.start(), um.end()))

    # Unused local arrays + dead-param assignments inside function bodies.
    # We iterate definitions to get their spans, then enumerate per-func.
    body_extra_spans: list[tuple[int, int]] = []
    for func_def_match in re.finditer(
        r"(?m)^[A-Za-z_][\w \t\*]*\b([A-Za-z_]\w*)\s*\(",
        text,
    ):
        fname = func_def_match.group(1)
        span = _func_body_span(text, fname)
        if span is None:
            continue
        if func_def_match.start() != span[0]:
            continue
        body_lo, body_hi = span
        params = _extract_func_params(text, span)
        for s, e, _ in find_unused_local_arrays(text, body_lo, body_hi):
            body_extra_spans.append((s, e))
        for s, e, _ in find_addr_coerced_locals(text, body_lo, body_hi):
            body_extra_spans.append((s, e))
        for s, e, _ in find_void_discard_unused_locals(text, body_lo, body_hi, params):
            body_extra_spans.append((s, e))
        for s, e, _ in find_volatile_unused_locals(text, body_lo, body_hi):
            body_extra_spans.append((s, e))
        for s, e in find_lowercase_asm_cheats(text, body_lo, body_hi):
            body_extra_spans.append((s, e))
        for s, e in find_always_true_if_scaffolds(text, body_lo, body_hi):
            body_extra_spans.append((s, e))
        for s, e in find_empty_do_while_zero(text, body_lo, body_hi):
            body_extra_spans.append((s, e))
        for s, e, _ in find_empty_if_dead_reads(text, body_lo, body_hi):
            body_extra_spans.append((s, e))
        for s, e, _, _ in find_dead_conditional_stores(text, body_lo, body_hi):
            body_extra_spans.append((s, e))
        for s, e, _ in find_dead_param_assigns(text, body_lo, body_hi, params):
            body_extra_spans.append((s, e))

    # Apply edits in reverse order.
    all_edits = []
    for pos in vol_positions:
        m = _VOLATILE_KW_RE.match(text, pos)
        if m:
            end = m.end()
            if end < len(text) and text[end] in " \t":
                end += 1
            all_edits.append((pos, end, ""))
    for s, e in nv_alias_spans:
        all_edits.append((s, e, ""))
    for s, e, name in macro_spans:
        original = text[s:e]
        all_edits.append((s, e, f"/* STRIPPED CHEAT MACRO: {original.strip()} */"))
    for s, e in macro_usage_spans:
        # Replace usage with whitespace so positions stay easy to reason about.
        all_edits.append((s, e, " " * (e - s)))
    for s, e in body_extra_spans:
        # Replace with whitespace so positions stay easy to reason about.
        original = text[s:e]
        all_edits.append((s, e, " " * len(original)))

    # De-overlap: sort by start descending and skip edits whose span overlaps
    # an already-scheduled later edit.
    all_edits.sort(reverse=True)
    applied = []
    used_starts = []
    for s, e, repl in all_edits:
        if any(not (e <= us[0] or s >= us[1]) for us in used_starts):
            continue  # overlaps an already-scheduled edit; skip
        used_starts.append((s, e))
        applied.append((s, e, repl))

    n_stripped = len(applied)
    for s, e, repl in applied:
        text = text[:s] + repl + text[e:]
    return text, n_stripped


def write_stripped(stem: str, out_path: str) -> int:
    """Strip volatile-coercion cheats from src/<stem>.c into out_path. Returns
    count. Mirrors inlineasm.write_stripped's signature so it composes."""
    text = Path(f"src/{stem}.c").read_text(encoding="utf-8")
    stripped, n = strip_volatile_cheats_file(text)
    Path(out_path).parent.mkdir(parents=True, exist_ok=True)
    Path(out_path).write_text(stripped)
    return n


def _func_body_span(text: str, func: str) -> tuple[int, int] | None:
    """Locate `func`'s DEFINITION body. Splat-generated src/*.c put each
    definition at column 0; calls and nested references are indented. We must
    skip extern declarations (`extern T func(args);`) which match the same
    column-0 pattern — these are recognised by the FIRST non-whitespace after
    the closing `)` being `;` (declaration) rather than `{` (definition)."""
    import re as _re

    pattern = _re.compile(r"(?m)^[A-Za-z_][\w \t\*]*\b" + _re.escape(func) + r"\s*\(")
    for m in pattern.finditer(text):
        # Walk forward to the matching `)` of the opening `(`.
        i, n = m.end(), len(text)
        depth = 1  # we matched the opening `(`
        in_str = False
        while i < n and depth > 0:
            c = text[i]
            if in_str:
                if c == "\\":
                    i += 2
                    continue
                if c == "\"":
                    in_str = False
            elif c == "\"":
                in_str = True
            elif c == "(":
                depth += 1
            elif c == ")":
                depth -= 1
            i += 1
        if depth != 0:
            continue
        # Now i is past the closing `)`. Skip whitespace and find next non-ws.
        j = i
        while j < n and text[j] in " \t\n\r":
            j += 1
        if j >= n:
            continue
        if text[j] == ";":
            continue  # declaration, not definition
        if text[j] != "{":
            continue  # weird; skip
        # Match the closing `}` of the body.
        brace = j
        depth, k = 0, brace
        while k < n:
            c = text[k]
            if c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return (m.start(), k + 1)
            k += 1
    return None


def _names_used_in_body(text: str, lo: int, hi: int) -> set[str]:
    """Return the set of C identifiers referenced inside text[lo:hi]."""
    return set(re.findall(r"\b([A-Za-z_]\w*)\b", text[lo:hi]))


def func_volatile_cheat_count(text: str, func: str) -> int:
    """Number of volatile-coercion cheats AFFECTING `func`. Counts:
      - cheats whose source span is INSIDE the function body (inline casts,
        in-body extern decls), AND
      - file-scope alias-renames whose alias NAME is referenced in the body,
        AND
      - file-scope plain volatile externs whose SYMBOL name is referenced in
        the body.

    -1 if the body can't be located (caller falls back to no-signal). Counts
    each cheat once (by unique volatile-keyword position).

    Rationale: an alias-rename declared at file scope (between functions) is
    still load-bearing for any function that uses the alias's name — stripping
    the qualifier changes that function's codegen. The same is true for plain
    `extern volatile T D_x;` decls at file scope. The strip path already
    removes ALL such qualifiers file-wide; this count makes sure `mark_done`
    refuses every function the strip would touch."""
    span = _func_body_span(text, func)
    if span is None:
        return -1
    lo, hi = span
    used = _names_used_in_body(text, lo, hi)
    counted_vol_positions: set[int] = set()
    # In-body cheats (inline casts, in-body extern decls).
    for s, _e, vp in find_all_cheats(text):
        if lo <= s < hi:
            counted_vol_positions.add(vp)
    # File-scope alias renames whose alias name is referenced inside body.
    for s, e, vp in find_alias_renames(text):
        if lo <= s < hi:
            continue  # already counted above
        m = _ALIAS_RENAME_RE.search(text, s, e)
        if m and m.group("name") in used:
            counted_vol_positions.add(vp)
    # File-scope plain volatile externs whose symbol is referenced inside body.
    for s, e, vp in find_plain_volatile_externs(text):
        if lo <= s < hi:
            continue
        m = _PLAIN_VOLATILE_RE.search(text, s, e)
        if m and m.group("name") in used:
            counted_vol_positions.add(vp)
    # Non-volatile alias renames (in-body or file-scope-with-body-use).
    for s, e, asm_pos in find_nonvolatile_alias_renames(text):
        in_body = lo <= s < hi
        m = _NONVOL_ALIAS_RENAME_RE.search(text, s, e)
        if m and (in_body or m.group("name") in used):
            counted_vol_positions.add(asm_pos)
    # Macro `__asm__` definitions whose macro name is referenced in body.
    for s, e, name in find_macro_asm_defs(text):
        if name in used:
            counted_vol_positions.add(s)
    # Unused local arrays + dead-param-assigns inside this function body.
    params = _extract_func_params(text, span)
    for s, e, _name in find_unused_local_arrays(text, lo, hi):
        counted_vol_positions.add(s)
    for s, e, _name in find_addr_coerced_locals(text, lo, hi):
        counted_vol_positions.add(s)
    for s, e, _name in find_void_discard_unused_locals(text, lo, hi, params):
        counted_vol_positions.add(s)
    for s, e, _name in find_volatile_unused_locals(text, lo, hi):
        counted_vol_positions.add(s)
    for s, e in find_lowercase_asm_cheats(text, lo, hi):
        counted_vol_positions.add(s)
    for s, e in find_always_true_if_scaffolds(text, lo, hi):
        counted_vol_positions.add(s)
    for s, e in find_empty_do_while_zero(text, lo, hi):
        counted_vol_positions.add(s)
    for s, e, _cond in find_empty_if_dead_reads(text, lo, hi):
        counted_vol_positions.add(s)
    for s, e, _var, _rhs in find_dead_conditional_stores(text, lo, hi):
        counted_vol_positions.add(s)
    for s, e, _name in find_dead_param_assigns(text, lo, hi, params):
        counted_vol_positions.add(s)
    return len(counted_vol_positions)


def file_func_volatile_cheat_count(stem: str, func: str) -> int:
    try:
        text = Path(f"src/{stem}.c").read_text(encoding="utf-8")
    except OSError:
        return -1
    return func_volatile_cheat_count(text, func)


# ---------------------------------------------------------------------------
# Project-wide audit (for the BEFORE/AFTER report)

def audit_project(src_dir: str = "src") -> dict:
    """Walk src/*.c and report every detected volatile cheat. Returns a
    structured dict keyed by file -> [{kind, name, line, decl}] and a
    function-level rollup (func -> count)."""
    import re as _re

    by_file: dict[str, list[dict]] = {}
    by_func: dict[str, int] = {}
    # All .c files
    for p in sorted(Path(src_dir).glob("*.c")):
        text = p.read_text(encoding="utf-8")
        cheats_ = find_all_cheats(text)
        if not cheats_:
            continue
        # Map char-position -> line number
        line_starts = [0]
        for i, c in enumerate(text):
            if c == "\n":
                line_starts.append(i + 1)
        def line_of(pos: int) -> int:
            # Binary search
            import bisect as _bs
            return _bs.bisect_right(line_starts, pos)

        for s, e, vp in cheats_:
            line = line_of(s)
            # Categorize by which detector caught it (re-test the spans).
            kind = "unknown"
            for m in _ALIAS_RENAME_RE.finditer(text, s, e + 1):
                if m.start() == s:
                    kind = "alias_rename"
                    break
            if kind == "unknown":
                for m in _GLOBAL_CAST_RE.finditer(text, s, e + 1):
                    if m.start() == s:
                        kind = "global_cast"
                        break
            if kind == "unknown":
                for m in _PLAIN_VOLATILE_RE.finditer(text, s, e + 1):
                    if m.start() == s:
                        kind = "plain_extern"
                        break
            if kind == "unknown":
                for m in _NONVOL_ALIAS_RENAME_RE.finditer(text, s, e + 1):
                    if m.start() == s:
                        kind = "nonvol_alias_rename"
                        break
            if kind == "unknown":
                for m in _MACRO_ASM_DEF_RE.finditer(text, s, e + 1):
                    if m.start() == s:
                        kind = "macro_asm"
                        break
            # Extract name + decl snippet
            name = ""
            for m in _re.finditer(r"\b(\w+)\b", text[s:e]):
                tok = m.group(1)
                if tok not in ("extern", "volatile", "asm",
                               "s8", "u8", "s16", "u16", "s32", "u32",
                               "char", "short", "int", "long",
                               "void", "static", "const"):
                    name = tok
                    break
            decl_snip = text[s:min(e, len(text))].replace("\n", " ").strip()
            by_file.setdefault(str(p), []).append({
                "kind": kind, "line": line, "name": name, "decl": decl_snip,
            })
    return {"by_file": by_file,
            "files": sorted(by_file.keys()),
            "total_cheats": sum(len(v) for v in by_file.values())}


if __name__ == "__main__":
    import json as _j
    print(_j.dumps(audit_project(), indent=2))

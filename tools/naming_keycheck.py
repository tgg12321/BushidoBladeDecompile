#!/usr/bin/env python3
"""naming_keycheck.py — prove a naming wave changed identifiers only (.claude/rules/naming-bar.md).

Compares the working tree (or --new <rev>) with --base <rev> (default HEAD). Fails closed: every
changed, added, deleted or renamed file must be explained by the wave's old->new pairs, using the
same substitutions the wave tools apply (tools/naming_wave.py, tools/data_wave.py):

  src/**, include/** *.c *.h  ctokens(new) == ctokens(old with the pairs substituted in code,
                              string literals (escape-aware) and comments), as is or after
                              tools/format.py (a longer name can re-wrap or hoist a comment):
                              C tokens plus each comment at its place and `#define NAME(`
                              adjacency. Comment-only changes are listed for the reviewer.
  memory/**/layer2.jsonl      byte-equal to naming_wave.retarget_layer2_record(old) for its ledger's
                              rename (func retargeted, renamed_from extended, verdict_file moved).
  other files in a moved ledger directory          byte-identical (a pure move).
  docs/naming/function-names.csv  must equal a fresh build_census.py run (working-tree mode;
                              it reads build/bb2.map, so run after verify-oracle --rebuild).
                              In --new mode a changed census FAILS (re-derive it in the tree).
  docs/naming/phase3/**       wave metadata: allowed, listed.
  .claude/**, movovl/**, this tool   any change FAILS (movovl/ is a separate binary the main
                              oracle does not cover).
  registries (named_syms.txt, symbol_addrs.txt, undefined_syms_auto.txt,
  undefined_funcs_auto.txt)   each old line becomes exactly one output the wave tools produce
                              for it (registry_ok): in-place rewrite; data_wave's rewrite with
                              `was <that line's old name>`; or, when the new name stays defined,
                              deletion, naming_wave's preserved-note line, or data_wave's
                              `... is now object-defined` line. A link-breaking deletion is left
                              to the build (SHA1).
  any other text file         equal to one of: whole-word substitution, sub_c, sub_hash, sub_py.
A path rename must be the old path with the pairs substituted; an added or deleted file must be
one side of such a rename. Two tree-wide checks: a non-auto new name must not already occur in
the base tree's build, tool and rule files (src, include, asm, tools, engine, movovl, .claude,
*.ld, *.txt, Makefile; docs/ and memory/ cite proposals), save movovl/ lines that define or
reference the same routine in the overlay's copy of the same libscan-verbatim Sony module
(owner ruling Q130, movovl_same_routine). An auto name must carry the old
name's address; several old names may share a new name only when they name one address, and
no old name may survive in
a build file (src/, include/, asm/, *.ld, root *.txt, Makefile; C comments included).

--sub-comments is the wave's comment pass: it substitutes the pairs in the comments of every
src/ and include/ C file, then exits.

It lists every function whose layer-2 key (engine.layer2.body_key) moved, for the
naming-reviewer's R6: those keys moved by the pairs alone.

Both scoped and global working-tree verification require a fully staged wave.
Every porcelain-v2 entry with an unstaged change (status Y not '.') fails, so
intent-to-add entries (.A, and DA for a path HEAD tracks) fail even for empty files, as
commits do not store them; unmerged entries fail. Status runs with --no-optional-locks,
with --no-renames and --untracked-files=no, so verification never writes or locks the
index and ignores rename config. Working diffs and untracked paths are parsed as NUL
records.
Checks cover every changed row, every index-versus-HEAD difference, every base-
versus-HEAD difference (OID/mode, additions/removals), and tracked cpp inputs.
Hidden index flags (assume-unchanged/skip-worktree) fail on checked paths and cpp
inputs; flags elsewhere are not read. Any explicit filter attribute fails, built-in clean
conversion must preserve raw bytes, and index blobs must equal raw working blobs.
Index modes must match base modes (new regular files require 100644). Working
executable bits are not compared; this is the portable conservative mode rule.
Staged deletions must be absent from index and working files. Staged reverts to a
non-HEAD base may pass; unstaged reverts fail with a staging hint. Unreported raw
edits are compared to the requested base under cpp roots/Makefile; unstaged edits
anywhere in the tree fail through the status rows.
Commit verification and --sub-comments do not run these storage checks.

Scoped pairs require a plain tree on both sides: plain ASCII path components,
no symlinks/submodules, no empty or untracked src/include directories, and literal
includes after line splicing. Include existence probes, paste/stringizing hazards
and unrecognized cpp flags fail before census selection. Only relative tracked
-I/-iquote directories are supported; -Iinclude is today's one joined form.
Three root metadata names are catalog exceptions and remain illegal include targets.
Working snapshots mirror plain-named regular files under cpp roots, including
ignored files, byte for byte. Ignored non-plain editor junk is skipped. Non-regular
entries fail before opening; ignored dependencies fail in working-tree mode on
either side, even when they occur only in an unrelated TU. Working index flags
assume-unchanged/skip-worktree fail; raw tracked contents differing from HEAD
outside the reported change set fail. Makefile, tracked/non-
ignored C/header inputs and every cpp -M dependency in either tree receive byte/
directive/quote checks. Unused ignored
caches remain unchecked. Textual literal-target collection is limited to .c/.h.
Tracked/non-ignored .inc inputs receive byte checks only unless cpp opens them;
unused assembler comments receive no cpp directive checks.
Cpp -M queries every TU on both sides, even for member pairs. It decides selection
within this allowlist; the gate cannot authorize a rename. Case-alias targets fail
rather than bypassing checks. Allowed bytes are tab, LF, printable ASCII and valid UTF-8;
BOM and Unicode line separators fail. Logical lines split only at LF, and splices
are joined before probe scans. Ordinary unterminated quotes end at LF, as in cpp;
unterminated directive quotes fail. C raw-string prefixes are refused. #pragma,
_Pragma and #import are unsupported.
Markdown and literal ## strings are not directives.
Dependency paths cannot contain .., even inside compiler include roots. Untracked
root files such as a symlinked .venv are outside the cpp catalog. Unreadable walks
fail closed. Reader-owned trees cache successful gates and invalidate on mutation.

Scoped pairs apply only to src/include C and header files. T.m=n renames the single declaration of
member m in struct/union tag or typedef T; either pre- or post-global spelling of T is accepted.
T.u.m=n (any depth, T.u.v.m=n) renames member m of the anonymous struct/union declared inline as
member u of T (`union { ... } u;`); each path step must name exactly one such member at depth 1
of its container, m must occur once in T's whole definition, and m must be a depth-1 declarator
of the final container: plain, array, bit-field, function pointer, or an inline aggregate member
(`struct { ... } m;`, whose own body may be tagged). Tagged path steps are not followed (address
them as Tag.m). A flat T.m on a T that holds inline aggregates uses the same depth-1 declarator
census.
Accesses after . or -> may retain m or become n. The destination cannot be a member, type name,
macro name/body token, pasted token or C/GNU keyword (base and post-global trees). Qualifier-only
declarations and unterminated aggregate declarations fail. Other aggregate edits must be explained
by global pairs. The separate oracle SHA1 build is REQUIRED to establish that renamed accesses
resolve to T.

f:l=n replaces the one simple parameter/local binding and its uses in the unique definition of f.
The destination must be fresh in f and C file-scope/enum text, and absent from macro names/bodies
(excluding a macro's own parameters). Real prototype and other functions' parameter lists/bodies
are excluded from freshness; K&R exclusions include the parameter declarations between ) and the
body, using the tree's typedef names. A K&R function as the local-pair target stays unsupported.
Undeclared asm/linker names are allowed. A binding in an inner block needs every use inside that
block, after the declaration. A later declarator of a list binds only as `*`s, the name and array
bounds (`T a, *l`, `T a, l[2]`), after a first declarator naming a plain identifier after
non-aggregate type words (it may carry `*`s, qualifiers, bounds and an initializer); in a list
initializer the name is a use. Statements come from a forward scan of the body that classifies each
`{` as a block, a brace expression (an initializer, a compound literal after a cast) or a GNU
statement expression; an unclassifiable brace, a use inside a statement expression or a statement
that would start at `,` `.` `->` `[` `]` `=` `)` `?` `:` `}` fails. A body statement holding the
name that is not led by a known type word must prove it is no declaration: it opens with a
statement keyword, `(` or a unary operator, or with a name used as a value (not followed by a name,
a keyword or, unless f declares that name after a type word anywhere, `*`); otherwise it fails. A
name before `(` or `*` must also appear in no typedef declaration and not start with `__` (an
unlisted keyword; `__builtin_` functions aside), so `T (l)` or `T *l` fails even if the type
census misses T. The same parser reruns on cpp output, where macro types are expanded and every
typedef is visible; the cc1 differential renames every spelling alike, so it cannot tell one
binding from two. Uses before the binding, inside its own declarator or after its inner block,
a repeat later in the parameter list (`int (*cb)(l)`), a `(` in a declaration with no top-level
`=` before the name, a spelling inside a type name (a cast, sizeof or prototype list) outside an
array bound, other comma declarators, prototype/cast bindings, externs, shadowing and type-name pairs fail. Statement-start
labels, case/default labels, GNU designators, member tokens and aggregate tags stay unchanged.
Ambiguous labels and GNU label addresses fail. All literals, including asm strings, stay exact
under scoped substitution. Local acceptance requires a cc1 differential compile for every affected
TU: BASE, a batched fresh-name variant, and a batched destination-name variant must compile
without errors and emit text-mode identical asm. Only static-local name.<digits> labels are
normalized. The variants isolate local pairs on the base tree; source substitution and cpp proofs
separately cover global/member pairs. The real Makefile cpp/cc1 flags are used, without debug
flags. A missing compiler fails closed. Scoped commits run project cpp -M over every TU in a
bounded pool, on both trees. Only its real dependency sets select contexts for full cpp and cc1
proofs, including edited .c inclusions. Make dependency escapes are decoded without shell quoting.
Every dependency must exist at cpp's cwd inside the snapshot or a compiler include root reported
by cpp -v; /proc and absolute repository includes fail. Git and working-tree symlinks and
existence probes fail before census selection. Filesystem identity uses cached os.stat, without
per-dependency Path.resolve traversals. Variant trees are built once and reused across these TUs.
Literal Make helper and recipe templates are supported; edits to these templates disable scoped
checking until the tool is updated. Declarators are classified name-outward: *F() is a function
returning a pointer, (*F)() and known function-typedef F *l are objects. GNU typeof, macro-
produced, multi-declarator and otherwise uncertain typedefs fail closed. Macro changes inside
aggregate typedef bodies and attributed typedefs remain conservative false rejects. Function
declarations and externs are not local object bindings. GNU nested-function targets are
unsupported. Duplicate static function names in the tree cannot be addressed by f:l and fail. Some
file-scope shadowing is conservatively rejected, including __typeof__-declared
globals. Parenthesised function definitions keep their typedefs in block scope.
Parser scope/binding/freshness checks are reject-only prefilters on both trees.
Recognized file-scope ordinary declarations and enumerators
are conservatively rejected (including the old name); uses, tags, members and other functions'
locals/parameters are excluded. This prefilter is not the binding proof. Cpp function boundaries
must agree with source boundaries; either global spelling of the function scope is accepted and
missing required definitions fail. The parser attempts to exclude typedefs from other function
bodies; it cannot authorize a local rename. Any ordinary redeclaration of a visible typedef in
parameters or statement declarators fails, including commas and parentheses. Qualified casts and
sizeof operands are type contexts. Shadowed typedefs and locals inside aggregate/enum bodies fail.
Aggregate tables across every TU block competing member destinations, including macro-spelled and
.inc declarations.

Scoped destinations cannot equal global sources/destinations. Member destinations may be
shared only by member pairs of distinct aggregates (naming-bar §5: the code shows they hold
the same thing, and all of those pairs are in the wave); the aggregate is the resolved outer
definition (a tag and its typedef are one; T.u.m is T's), and n must still be fresh against
every other member, type and macro. Local destinations may be shared only across distinct functions. For local pairs,
macro arguments containing the old name fail even when unchanged. For member pairs, a macro
argument naming m (edited to n or unchanged) is allowed only when every definition of that
macro expands all its parameters plainly (no dropped, stringized, pasted or variadic parameter;
not an object-like alias; not chained), and its cpp sentinel proves it: emitted by a compiled
TU, never dropped, stringized or pasted (also through nested forwarding), and after . or -> in
every TU that emits it. A stale m that reached the renamed T.m then fails to compile, and member
pairs require the oracle build. Closure macro guards use cpp -dD
definitions from every included file and command-line definition, in both trees. Object- and
function-like call openers fail, as do local tokens adjacent to either kind of macro invocation.
Closure macros containing unbalanced braces or aggregate syntax fail even if never invoked;
balanced statement blocks are allowed. Scoped edits never touch directives; a conservative
conditional-code filter exempts simple outer include guards. Reachability is exactly what the
project's Makefile-configured cpp emitted: every changed code token's base and new source tokens
must survive cpp at their source lines. Each edit is replaced with a fresh unique identifier in
scratch base/new trees. Every relevant TU must emit its sentinel as a whole identifier only,
outside literals, at the expected source line. Normal cpp output must also contain the original
spelling at that site with the same total count as its sentinels. In cpp output, edited local
sentinels cannot become members, tags or labels; a ternary operand before : stays a use. Edited
member sentinels must follow . or -> or be a simple declarator in T. Pragma metadata is excluded.
Conditional colons track unique nesting frames; questions from closed frames cannot classify later
labels. These namespace checks use the whole output token stream. Token alignment is never
evidence. A second function-wide sentinel pass uses the same cpp run to protect local old/new
source tokens; expansions cannot introduce those names. Both trees are preprocessed; include-stack
markers are validated and source line-control directives fail. Results are cached per tree/TU.
Missing cpp or an indeterminate Makefile command fails scoped proofs; global pairs do not need
cpp. Snapshots contain src, include, Makefile and configured include directories; asm is excluded
unless explicitly requested by an include-directory flag. Source freshness includes .inc files and
refuses unbalanced braces inside preprocessor conditionals, except in the first branch of an
#if / #ifdef that tests only C++ macros (inactive for cc1 unless one is defined). A pragma
inside a local-pair target stays unsupported. Only the toolchain's stdc-predef.h flag-3 push is
supported; empty external pushes are tracked but cannot certify edits. Closure-wide include existence probes, token pastes
and digraph directives fail. Command-line -D names and bodies participate in macro checks
alongside source definitions. Scoped old/new names cannot be macro names in either tree. The whole
TU closure must contain no object-like macro call opener. -dD reports cpp definitions from all
entered sources, including the compiler predef header. Each TU has ordinary and sentinel runs on
both trees (four runs), cached across pairs.

Required edits must actually land; untouched definitions cannot certify a pair. Comments in
touched C files may replace scoped words (reported as notes). Scoped pairs never affect paths,
registries, asm, ledgers, global freshness or old-name survival. --from-manifests is unchanged;
--sub-comments accepts global pairs only. The moved-key report unions the historical regex census
(including K&R) with the token census, excluding directives/keywords. Unkeyable changed functions
and malformed delimiters fail cleanly. Base inputs use one git cat-file --batch; per-file analyses
and freshness tables are cached for one proof only.

Usage:
  python3 tools/naming_keycheck.py --pairs old1=new1,old2=new2 [--base HEAD] [--new REV]
  python3 tools/naming_keycheck.py --pairs-file tmp/pairs.txt      # one old=new per line
Exit 0 = OK, 1 = a change the pairs do not explain, 2 = usage error.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
import shlex
import shutil
import uuid
import time
import threading
import stat as stat_mode
from concurrent.futures import ThreadPoolExecutor
from bisect import bisect_left
from functools import lru_cache
from contextlib import contextmanager
from posixpath import normpath
from pathlib import Path

CODE = Path(__file__).resolve().parent.parent
# The repository checked; NAMING_KEYCHECK_ROOT points it at another clone (the probe suite).
ROOT = Path(os.environ.get("NAMING_KEYCHECK_ROOT") or CODE).resolve()
sys.path.insert(0, str(CODE))
sys.path.insert(0, str(CODE / "tools"))
from engine import inlineasm, layer2  # noqa: E402
import naming_wave as nw  # noqa: E402

IDENT = re.compile(r"[A-Za-z_]\w*")
C_KEYWORDS = set("auto break case char const continue default do double else enum extern float for goto if int long register return short signed sizeof static struct switch typedef union unsigned void volatile while".split())
GNU_SPECIFIERS = set("__volatile __volatile__ __const __const__ __signed __signed__ __inline __inline__ inline __attribute __attribute__ __extension__ __typeof __typeof__ typeof __complex __complex__".split())
# cc1 2.7.2 predates C99: _Bool, _Complex, _Imaginary and the restrict spellings
# are ordinary identifiers to it (a typedef may take those names).
C_KEYWORDS |= GNU_SPECIFIERS | {"__asm", "__asm__", "asm", "__label__"}
# The rest of cc1 2.7.2's reserved words (c-parse.gperf).
C_KEYWORDS |= set("__alignof __alignof__ __imag __imag__ __real __real__ __iterator __iterator__".split())
DECL_WORDS = set("void char short int long float double signed unsigned const volatile static extern register auto struct union enum".split()) | GNU_SPECIFIERS
STATEMENT_KEYWORDS = set("return if while for switch do else case default goto sizeof asm __asm __asm__".split())
AUTO = re.compile(r"^(?:func|D)_[0-9A-Fa-f]{8}$")
SELF = "tools/naming_keycheck.py"
METADATA = ("docs/naming/function-names.csv",)
METADATA_DIRS = ("docs/naming/phase3/",)
DENY_DIRS = (".claude/", "movovl/")  # rules/agents; the overlay (its own binary: no oracle here)
LEDGER_DIRS = tuple(d + "/" for d in nw.LEDGER_DIRS)
C_COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)


def git(*args: str, check: bool = True) -> str:
    grep = bool(args and args[0] == "grep")
    result = subprocess.run(["git", *args], cwd=ROOT, check=check and not grep, capture_output=True,
                            text=True, encoding="utf-8", errors="surrogateescape")
    if grep and result.returncode > 1:
        raise subprocess.CalledProcessError(result.returncode, result.args,
                                            output=result.stdout, stderr=result.stderr)
    return result.stdout


def show(rev: str | None, path: str) -> str | None:
    if rev is None:
        p = ROOT / path
        return nw.read(p) if p.is_file() else None
    r = subprocess.run(["git", "show", f"{rev}:{path}"], cwd=ROOT, capture_output=True,
                       text=True, encoding="utf-8", errors="surrogateescape")
    return r.stdout if r.returncode == 0 else None


PLAIN_PATH = re.compile(r"[A-Za-z0-9_][A-Za-z0-9_.+-]*(/[A-Za-z0-9_][A-Za-z0-9_.+-]*)*")
ROOT_METADATA = {".clang-format", ".gitattributes", ".gitignore"}


def shape_path(path):
    if not PLAIN_PATH.fullmatch(path) or any(p in (".", "..") for p in path.split("/")):
        raise ValueError(f"scoped tree shape: non-plain path {path!r}")


def shape_catalog(records):
    """Validate modes before filtering; metadata is never a legal include target."""
    for mode, path in records:
        if mode in ("120000", "160000"):
            raise ValueError(f"scoped tree shape: symlink/submodule {path!r} (mode {mode})")
        if path.startswith(("src/", "include/", "asm/")) or "/" not in path:
            if path not in ROOT_METADATA:
                shape_path(path)


def plain_cpp_flags(makefile, files):
    command = cpp_command(makefile)
    directories = {str(Path(p).parent).replace("\\", "/") for p in files}
    directories.update(parent.as_posix() for p in files for parent in Path(p).parents)
    i = 1
    while i < len(command):
        flag = command[i]
        if flag == "-Iinclude":
            directory = "include"  # The one joined include form in today's Makefile.
        elif flag in ("-I", "-iquote"):
            i += 1
            if i >= len(command):
                raise ValueError("scoped tree shape: missing cpp include directory")
            directory = command[i]
        elif flag in ("-undef", "-Wall", "-lang-c", "-fno-builtin"):
            i += 1
            continue
        elif flag == "-D" or flag.startswith("-D"):
            if flag == "-D":
                i += 1
                if i >= len(command):
                    raise ValueError("scoped tree shape: missing cpp macro")
            body = command[i] if flag == "-D" else flag[2:]
            if "##" in body or "__has_include" in body or "include" in body:
                raise ValueError("scoped tree shape: cpp macro include/paste hazard")
            i += 1
            continue
        else:
            raise ValueError(f"scoped tree shape: unsupported cpp flag {flag!r}")
        shape_path(directory)
        if directory == "asm" or directory.startswith("asm/"):
            raise ValueError("scoped tree shape: asm is not a supported cpp root")
        if directory not in directories:
            raise ValueError(f"scoped tree shape: cpp include directory not tracked: {directory}")
        i += 1


class ShapeTree(dict):
    """Reader-owned tree; cache successful gates without trusting later mutations."""
    pass


def plain_bytes(path, text):
    try:
        data = text.encode("utf-8", errors="surrogateescape")
        data.decode("utf-8", errors="strict")
    except (UnicodeEncodeError, UnicodeDecodeError) as e:
        raise ValueError(f"scoped tree shape: invalid UTF-8: {path} offset {e.start}") from e
    bad = re.search(rb"[\x00-\x08\x0b-\x1f\x7f]", data)
    if bad is not None:
        raise ValueError(f"scoped tree shape: control byte: {path} offset {bad.start()}")
    if any(ch in text for ch in ("\ufeff", "\u2028", "\u2029", "\u0085")):
        raise ValueError(f"scoped tree shape: BOM/Unicode line separator: {path}")


def source_quotes(path, text):
    """Cpp-style LF quote boundaries; unclosed directive quotes fail closed."""
    logical, code = list(text), list(text)
    i = 0
    while i < len(text):
        start = i
        comment = text.startswith(("//", "/*"), i)
        if text.startswith("//", i):
            end = text.find("\n", i + 2)
            i = len(text) if end < 0 else end
        elif text.startswith("/*", i):
            end = text.find("*/", i + 2)
            i = len(text) if end < 0 else end + 2
        elif text[i] in ("'", '\"'):
            if (text[i] == '"' and re.search(
                    r'(?<![A-Za-z0-9_])(?:u8R|LR|uR|UR|R)$', text[max(0, i - 5):i])):
                raise ValueError(f"scoped tree shape: raw string unsupported: {path}")
            quote = text[i]
            i += 1
            while i < len(text) and text[i] != quote and text[i] != "\n":
                i += 2 if text[i] == "\\" else 1
            if i >= len(text) or text[i] == "\n":
                prefix = "".join(logical[text.rfind("\n", 0, start) + 1:start])
                if prefix.lstrip(" \t").startswith("#"):
                    raise ValueError(f"scoped tree shape: unterminated directive quote: {path}")
                # Cpp ends a malformed ordinary-line quote at LF, including #if 0.
                # Its apparent /* cannot consume following source lines.
            else:
                i += 1
        else:
            i += 1
            continue
        for k in range(start, min(i, len(text))):
            if text[k] != "\n":
                code[k] = " "
                if comment:
                    logical[k] = " "
    return "".join(logical), "".join(code)


def cpp_readable_sources(files, roots):
    """Overapproximate literal inputs; never exclude a TU from the real cpp census."""
    paths = {p for p in files if p != "Makefile" and in_cpp_snapshot(p, roots)}
    eligible = getattr(files, "_nonignored", paths)
    sources = ({p for p in paths & set(eligible) if p.endswith((".c", ".h"))}
               | getattr(files, "_dependency_sources", set()))
    folded = {}
    for path in paths:
        folded.setdefault(path.casefold(), set()).add(path)
    for path in tuple(sources):
        for match in re.finditer(r'["<]([A-Za-z0-9_][A-Za-z0-9_./+-]*)[">]',
                                 re.sub(r"\\\n", "", files[path])):
            target = match[1]
            if not target.endswith((".c", ".h")):
                continue
            candidates = [normpath(str(Path(path).parent / target).replace("\\", "/"))]
            candidates.extend(root + "/" + target for root in roots)
            for candidate in candidates:
                if candidate in paths:
                    if candidate in eligible:
                        sources.add(candidate)
                elif candidate.casefold() in folded:
                    raise ValueError(f"scoped tree shape: case-alias include target: {target}")
    return sources


def plain_tree(files):
    """A reject-only gate; cpp remains the census authority within a plain tree."""
    signature = tuple(files.items())
    if isinstance(files, ShapeTree) and getattr(files, "_plain_signature", None) == signature:
        return
    shape_catalog(("100644", path) for path in files)
    if "Makefile" in files:
        plain_bytes("Makefile", files["Makefile"])
        plain_cpp_flags(files["Makefile"], files)
    roots = cpp_snapshot_roots(files.get("Makefile"))
    sources = cpp_readable_sources(files, roots) | getattr(files, "_dependency_sources", set())
    # No asm, ignored caches or unrelated docs receive C byte/text checks.
    for path in sources | {p for p in files if p.endswith(".inc")
                           and p in getattr(files, "_nonignored", files)}:
        plain_bytes(path, files[path])
    names = {"include", "define", "undef", "if", "ifdef", "ifndef", "elif", "else",
             "endif", "error", "warning", "line"}
    for path, text in files.items():
        if path == "Makefile" or not in_cpp_snapshot(path, roots):
            continue
        shape_path(path)
        if path not in sources:
            continue
        if re.search(r"\\[ \t]+\n", text):
            raise ValueError(f"scoped tree shape: ambiguous cpp line splice: {path}")
        spliced = re.sub(r"\\\n", "", text)
        logical, code = source_quotes(path, spliced)
        if re.search(r"\b_Pragma\b", spliced):
            raise ValueError(f"scoped tree shape: _Pragma unsupported: {path}")
        tokens = layer2.tokens(code)
        if (re.search(r"\b__has_include\w*\b", code)
                or "##" in tokens or "%:" in code):
            raise ValueError(f"scoped tree shape: include existence/paste/digraph hazard: {path}")
        for line in logical.split("\n"):
            prefix = line.lstrip(" \t")
            if prefix.startswith("#"):
                directive = re.fullmatch(r"#[ \t]*([A-Za-z_][A-Za-z0-9_]*)?(.*)", prefix)
                if not directive or directive[1] not in names and prefix.strip() != "#":
                    raise ValueError(f"scoped tree shape: unsupported directive: {path}: {prefix}")
                if directive[1] == "include":
                    match = re.fullmatch(r'\s*(?:"([^"\n]+)"|<([^>\n]+)>)\s*', directive[2])
                    if not match:
                        raise ValueError(f"scoped tree shape: non-literal include: {path}")
                    shape_path(match[1] or match[2])
            elif re.search(r"\binclude\w*\b", blank_comments(line, literals=True)):
                if "#" in layer2.tokens(line):
                    raise ValueError(f"scoped tree shape: include stringizing hazard: {path}")
        if any(re.search(r"\binclude\w*\b", blank_comments(line, literals=True))
               and layer2.tokens(line).count("#") > 1
               for line in logical.split("\n")):
            raise ValueError(f"scoped tree shape: include stringizing hazard: {path}")
    if isinstance(files, ShapeTree):
        files._plain_signature = signature


def walk_error(error):
    raise ValueError(f"scoped tree shape: unreadable directory: {error}") from error


def shape_read(path):
    with path.open(encoding="utf-8", errors="surrogateescape", newline="") as source:
        return source.read()


def working_shape(tracked, untracked, roots=("src", "include")):
    shape_catalog(tracked)
    shape_catalog(("100644", path) for path in untracked
                  if any(path.startswith(root + "/") for root in (*roots, "asm")))
    directories = {parent.as_posix() for _, p in tracked for parent in Path(p).parents}
    for directory in dict.fromkeys(roots):
        root = ROOT / directory
        for current, dirs, names in os.walk(root, followlinks=False, onerror=walk_error):
            candidates = [Path(current), *(Path(current) / n for n in dirs + names)]
            for path in candidates:
                info = os.lstat(path)
                if (stat_mode.S_ISLNK(info.st_mode) or
                        getattr(info, "st_file_attributes", 0) & 0x400):
                    raise ValueError(f"scoped tree shape: working symlink/junction {path}")
                if not (stat_mode.S_ISREG(info.st_mode) or stat_mode.S_ISDIR(info.st_mode)):
                    raise ValueError(f"scoped tree shape: non-regular working input {path}")
            if directory in ("src", "include"):
                relative = Path(current).relative_to(ROOT).as_posix()
                if not dirs and not names or relative not in directories:
                    raise ValueError(f"scoped tree shape: empty/untracked directory {relative}")


def read_base_tree(rev: str) -> dict[str, str]:
    """Read all proof inputs using one listing and one cat-file batch process."""
    listing = git("ls-tree", "-r", "-z", rev)
    shape_catalog((entry.split("\t", 1)[0].split()[0], entry.split("\t", 1)[1])
                  for entry in filter(None, listing.split("\0")))
    has_makefile = any(entry.endswith("\tMakefile") for entry in listing.split("\0"))
    if has_makefile:
        result = subprocess.run(["git", "show", f"{rev}:Makefile"], cwd=ROOT,
                                capture_output=True, check=True)
        makefile = result.stdout.decode("utf-8", errors="surrogateescape")
        plain_bytes("Makefile", makefile)
        roots = cpp_snapshot_roots(makefile)
    else:
        roots = ("src", "include")
    entries = []
    for entry in filter(None, listing.split("\0")):
        info, path = entry.split("\t", 1)
        if not (in_cpp_snapshot(path, roots) or path.startswith("asm/")
                or path.endswith(".ld") or path in REGISTRIES):
            continue
        mode, kind, oid = info.split()
        if mode == "120000":
            raise ValueError(f"{path}: symlink base input cannot be proved")
        if kind != "blob":
            raise ValueError(f"{path}: non-blob base input")
        entries.append((oid, path))
    r = subprocess.run(["git", "cat-file", "--batch"], cwd=ROOT,
                       input="".join(oid + "\n" for oid, _ in entries).encode("ascii"),
                       capture_output=True, check=True)
    out, pos = ShapeTree(), 0
    for oid, path in entries:
        end = r.stdout.find(b"\n", pos)
        header = r.stdout[pos:end].decode("ascii").split()
        if end < 0 or len(header) != 3 or header[:2] != [oid, "blob"]:
            raise ValueError(f"{path}: invalid cat-file batch response")
        size = int(header[2])
        pos = end + 1
        data = r.stdout[pos:pos + size]
        pos += size
        if len(data) != size or r.stdout[pos:pos + 1] != b"\n":
            raise ValueError(f"{path}: truncated cat-file batch response")
        pos += 1
        out[path] = data.decode("utf-8", errors="surrogateescape")
    if pos != len(r.stdout):
        raise ValueError("unexpected trailing cat-file batch data")
    plain_tree(out)
    return out


def read_working_tree():
    staged = git("ls-files", "--stage", "-z")
    tracked = [(row.split("\t", 1)[0].split()[0], row.split("\t", 1)[1])
               for row in staged.split("\0") if row]
    untracked = [p for p in git("ls-files", "--others", "--exclude-standard", "-z").split("\0")
                 if p]
    shape_catalog(tracked)
    makefile = shape_read(ROOT / "Makefile") if (ROOT / "Makefile").is_file() else None
    if makefile is not None:
        plain_bytes("Makefile", makefile)
    roots = cpp_snapshot_roots(makefile)
    for row in git("ls-files", "-v", "-z").split("\0"):
        if row and (row[0].islower() or row[0] == "S"):
            path = row[2:]
            if in_cpp_snapshot(path, roots):
                raise ValueError("hidden working index flag (assume-unchanged/skip-worktree): "
                                 + path)
    working_shape(tracked, untracked, roots)
    files = ShapeTree()
    # Mirror all cpp-visible files, including ignored shadow headers and binary caches.
    files._nonignored = {p for _, p in tracked} | set(untracked)
    files._working_nonignored = files._nonignored
    files._tracked = {p for _, p in tracked}
    files._index = index_blobs(staged)
    files._index_modes = {row.split("\t", 1)[1]: row.split()[0]
                          for row in filter(None, staged.split("\0"))}
    paths = set()
    for root in roots:
        for directory, _, names in os.walk(ROOT / root, followlinks=False, onerror=walk_error):
            paths.update((Path(directory) / name).relative_to(ROOT).as_posix() for name in names)
    for key in sorted(paths):
        if key != "Makefile" and in_cpp_snapshot(key, roots):
            path = ROOT / key
            info = os.lstat(path)
            if stat_mode.S_ISLNK(info.st_mode):
                raise ValueError(f"{path}: symlink working input cannot be proved")
            if not stat_mode.S_ISREG(info.st_mode):
                raise ValueError(f"scoped tree shape: non-regular working input {path}")
            if key not in files._nonignored and not PLAIN_PATH.fullmatch(key):
                continue  # Ignored editor junk cannot be a permitted literal include.
            files[key] = shape_read(path)
    if makefile is not None:
        files["Makefile"] = makefile
    plain_tree(files)
    return files


def verify_working_changes(files, baseline, rows):
    """Compare raw working contents to base, without clean filters or EOL conversion."""
    roots = cpp_snapshot_roots(files.get("Makefile"))
    changed = {row[1] for row in rows}
    for path in sorted(files._tracked):
        if (in_cpp_snapshot(path, roots) and path not in changed
                and files.get(path) != baseline.get(path)):
            raise ValueError(f"tracked working bytes differ from base outside change set: {path}")


def index_blobs(staged):
    result = {}
    for row in filter(None, staged.split("\0")):
        fields, path = row.split("\t", 1)
        _, oid, stage = fields.split()
        if stage != "0":
            raise ValueError(f"unmerged working index: {path}")
        result[path] = oid
    return result


def verify_working_storage(rows, files=None, base="HEAD"):
    """Checked working bytes must equal the fully staged, conversion-free wave."""
    # Porcelain v2 distinguishes intent-to-add from a commit-storable empty blob.
    # Stage-first: any unstaged tracked change (Y != '.') fails; Y == 'A' is intent-to-add
    # (.A, or DA after `git rm --cached`). --no-optional-locks keeps status from
    # refreshing (writing) the wave's index; --no-renames drops copy/rename config.
    status = iter(git("--no-optional-locks", "status", "--porcelain=v2", "-z", "--no-renames",
                      "--untracked-files=no").split("\0"))
    for row in status:
        if row.startswith("u "):
            raise ValueError("unmerged working index: " + row.split(" ", 10)[-1])
        if row.startswith(("1 ", "2 ")):
            fields = row.split(" ", 8 if row[0] == "1" else 9)
            if len(fields) != (9 if row[0] == "1" else 10) or len(fields[1]) != 2:
                raise ValueError("invalid porcelain-v2 status entry")
            if fields[1][1] != ".":
                kind = "intent-to-add file" if fields[1][1] == "A" else "unstaged change"
                raise ValueError(kind + " is not staged: " + fields[-1]
                                 + "; stage the wave (git add -A) before the keycheck")
            if row[0] == "2":
                if not next(status, None):  # Original path follows as a NUL record.
                    raise ValueError("missing porcelain-v2 rename source")
    staged = git("ls-files", "--stage", "-z") if files is None else None
    index = files._index if files is not None else index_blobs(staged)
    modes = (getattr(files, "_index_modes", {p: "100644" for p in index}) if files is not None
             else {row.split("\t", 1)[1]: row.split()[0]
                   for row in filter(None, staged.split("\0"))})
    head, head_modes = {}, {}
    for row in filter(None, git("ls-tree", "-r", "-z", "HEAD").split("\0")):
        fields, path = row.split("\t", 1)
        mode, _, oid = fields.split()
        head[path], head_modes[path] = oid, mode
    baseline, base_modes = head, head_modes
    if base != "HEAD":
        baseline, base_modes = {}, {}
        for row in filter(None, git("ls-tree", "-r", "-z", base).split("\0")):
            fields, path = row.split("\t", 1)
            mode, _, oid = fields.split()
            baseline[path], base_modes[path] = oid, mode
    makefile = (files.get("Makefile") if files is not None else
                shape_read(ROOT / "Makefile") if (ROOT / "Makefile").is_file() else None)
    roots = cpp_snapshot_roots(makefile)
    changed = {row[1] for row in rows}
    deleted = {path for status, path in rows if status == "D"}
    def present(path):
        return (path in files if files is not None and in_cpp_snapshot(path, roots)
                else (ROOT / path).is_file())
    staged_changes = {p for p in index.keys() | head.keys()
                      if (index.get(p), modes.get(p)) != (head.get(p), head_modes.get(p))}
    history_changes = {p for p in baseline.keys() | head.keys()
                       if (baseline.get(p), base_modes.get(p)) != (head.get(p), head_modes.get(p))}
    checked = changed | staged_changes | history_changes
    checked |= {p for p in index if in_cpp_snapshot(p, roots)}
    deleted |= {p for p in checked if p not in index and not present(p)}
    for row in filter(None, git("ls-files", "-v", "-z").split("\0")):
        if (row[0].islower() or row[0] == "S") and row[2:] in checked:
            raise ValueError("hidden working index flag: " + row[2:])
    if not checked:
        return
    attr_input = "".join(p + "\0" for p in sorted(checked)).encode("utf-8")
    result = subprocess.run(["git", "check-attr", "-z", "--stdin", "filter"], cwd=ROOT,
                            input=attr_input, capture_output=True, check=True)
    records = result.stdout.decode("utf-8").split("\0")
    if records[-1] != "" or len(records) != len(checked) * 3 + 1:
        raise ValueError("invalid batched filter attribute response")
    for path, (name, attr, value) in zip(sorted(checked), zip(*[iter(records[:-1])] * 3)):
        if name != path or attr != "filter":
            raise ValueError("invalid batched filter attribute path")
        if value != "unspecified":
            raise ValueError("filter attribute unsupported for checked path: " + path)
    hint = "; stage the wave (git add -A) before the keycheck"
    for path in sorted(checked):
        if path in index and modes[path] != base_modes.get(path, "100644"):
            raise ValueError("index mode differs from base mode: " + path)
        if (ROOT / path).is_symlink():
            raise ValueError("checked working symlink unsupported: " + path)
        if path in deleted:
            if path in index or present(path):
                raise ValueError("deletion is not fully staged: " + path + hint)
        elif not present(path) or path not in index:
            raise ValueError("checked working file is not fully staged: " + path + hint)
    paths = sorted(checked - deleted)
    if not paths:
        return
    names = "".join(json.dumps(p, ensure_ascii=False) + "\n" for p in paths).encode("utf-8")
    hashes = []
    for flags in ([], ["--no-filters"]):
        result = subprocess.run(["git", "hash-object", *flags, "--stdin-paths"], cwd=ROOT,
                                input=names, capture_output=True, check=True)
        values = result.stdout.decode("ascii").splitlines()
        if len(values) != len(paths) or any(not re.fullmatch(r"[0-9a-f]{40}", v) for v in values):
            raise ValueError("invalid batched working hash response")
        hashes.append(dict(zip(paths, values)))
    clean, raw = hashes
    for path in paths:
        if clean[path] != raw[path]:
            raise ValueError("Git clean conversion changes working bytes: " + path)
        if index[path] != raw[path]:
            raise ValueError("index blob differs from raw working blob: " + path + hint)
        if in_cpp_snapshot(path, roots) and path not in changed and raw[path] != baseline.get(path):
            raise ValueError("tracked working bytes differ from base outside change set: " + path)


def parse_pairs(spec: str) -> dict[str, str]:
    pairs: dict[str, str] = {}
    for item in re.split(r"[,\n]", spec):
        item = item.strip()
        if not item or item.startswith("#"):
            continue
        old, sep, new = item.partition("=")
        old, new = old.strip(), new.strip()
        names = re.split(r"[.:]", old)
        if (not sep or (len(names) > 2 and (":" in old or len(names) > 8))
                or not all(IDENT.fullmatch(x) for x in names)
                or not IDENT.fullmatch(new) or names[-1] == new):
            sys.exit(f"bad pair {item!r} (want old=new, two different identifiers)")
        if old in pairs and pairs[old] != new:
            sys.exit(f"{old} mapped twice")
        pairs[old] = new
    if not pairs:
        sys.exit("no pairs given")
    return pairs


def scoped_parts(spec):
    """(scope, path, old) of a scoped pair: T.m and f:l have an empty path; T.u.m names
    member m of the anonymous aggregate declared as member u of T (path ("u",))."""
    if ":" in spec:
        scope, old = spec.split(":")
        return scope, (), old
    names = spec.split(".")
    return names[0], tuple(names[1:-1]), names[-1]


def member_scope_key(scope, path):
    """The namespace key of a member pair: T, or T.u... for a nested member."""
    return ".".join((scope,) + tuple(path))


def nested_container(ts, j, end, path):
    """Follow path from the aggregate body ts[j] == '{' .. ts[end] == '}': each step is a
    depth-1 member declared as an inline anonymous struct/union (`union { ... } u;`).
    Returns the (open, close) indices of the last container, or None."""
    qualifiers = {"const", "volatile", "__const", "__const__", "__volatile", "__volatile__"}
    for name in path:
        found, start, k = [], j + 1, j + 1
        while k < end:
            if ts[k] == "{":
                k = balanced(ts, k, "{", "}") + 1
                continue
            if ts[k] == ";":
                seg = list(range(start, k))
                opens = [x for x in seg if ts[x] == "{"]
                if opens:
                    p = opens[0]
                    q = balanced(ts, p, "{", "}")
                    if (p - 1 >= start and ts[p - 1] in ("struct", "union")
                            and all(ts[x] in qualifiers for x in range(start, p - 1))
                            and q == k - 2 and ts[k - 1] == name):
                        found.append((p, q))
                start = k + 1
            k += 1
        if len(found) != 1:
            return None
        j, end = found[0]
    return j, end


def container_declarators(ts, j, end):
    """Index -> name of the depth-1 member declarators of the aggregate body ts[j] == '{'
    .. ts[end] == '}': plain, array, bit-field (`m : 3`) and function-pointer (`(*m)(...)`)
    declarators, and the one declarator of an inline aggregate member
    (`struct { ... } m;`). Unrecognized declarations are skipped (they declare nothing
    this proof can certify)."""
    out, start, k = {}, j + 1, j + 1
    while k < end:
        if ts[k] == "{":
            k = balanced(ts, k, "{", "}") + 1
            continue
        if ts[k] == ";":
            seg = ts[start:k]
            if "{" not in seg:
                depth = 0
                for q in range(start, k):
                    tok = ts[q]
                    depth += (tok in ("(", "[")) - (tok in (")", "]"))
                    if not IDENT.fullmatch(tok) or tok in C_KEYWORDS:
                        continue
                    following = ts[q + 1] if q + 1 < k else ";"
                    if (depth == 0 and following in (";", ",", "[", ":")
                            and ts[q - 1] not in (".", "->", ":")):
                        out[q] = tok  # plain, array or bit-field declarator
                    elif depth == 1 and ts[q - 1] == "*" and ts[q - 2] == "(" and following == ")":
                        out[q] = tok  # function-pointer declarator (*m)(...)
            else:
                p = start + seg.index("{")
                q = balanced(ts, p, "{", "}")
                kw = p - 1 - (IDENT.fullmatch(ts[p - 1]) is not None and ts[p - 1] not in ("struct", "union"))
                if (q == k - 2 and IDENT.fullmatch(ts[k - 1]) and kw >= start
                        and ts[kw] in ("struct", "union")):
                    out[k - 1] = ts[k - 1]
            start = k + 1
        k += 1
    return out


def code_spans(text: str):
    return [m for m in layer2._TOKEN_RE.finditer(text)
            if m.group("skip") is None and m.group("splice") is None]


def balanced(ts, start, left, right):
    depth = 0
    for i in range(start, len(ts)):
        depth += (ts[i] == left) - (ts[i] == right)
        if depth == 0:
            return i
    raise ValueError(f"unbalanced {left}{right}")


def blank_comments(text, literals=False):
    out, pos = [], 0
    for m in layer2._TOKEN_RE.finditer(text):
        out.append(text[pos:m.start()])
        tok = m[0]
        if tok.startswith(("/*", "//")) or literals and tok.startswith(('"', "'")):
            tok = re.sub(r"[^\n]", " ", tok)
        out.append(tok)
        pos = m.end()
    out.append(text[pos:])
    return "".join(out)


def pp_regions(text, allow_guard=True):
    """Directive and conditional spans, with original offsets retained."""
    directives, conditional = [], []
    depth, start, pos = 0, None, 0
    records, outer_branch = [], False
    lines = blank_comments(text, literals=True).splitlines(keepends=True)
    i = 0
    while i < len(lines):
        line, begin = lines[i], pos
        pos += len(lines[i])
        i += 1
        while line.rstrip("\r\n").endswith("\\") and i < len(lines):
            line += lines[i]
            pos += len(lines[i])
            i += 1
        match = re.match(r"\s*#\s*(\w+)", line)
        if not match:
            continue
        directives.append((begin, pos))
        word = match[1]
        records.append((word, line[match.end():].strip(), begin, pos))
        if depth == 1 and word in ("else", "elif"):
            outer_branch = True
        if word in ("if", "ifdef", "ifndef"):
            if not depth:
                start = begin
            depth += 1
        elif word == "endif":
            if not depth:
                raise ValueError("unmatched #endif")
            depth -= 1
            if not depth:
                conditional.append((start, pos))
    if depth:
        raise ValueError("unclosed preprocessor conditional")
    # Only an outer, entire-file, unbranched #ifndef/#define guard is exempt.
    clean = re.sub(r"/\*.*?\*/|//[^\n]*", lambda m: " " * len(m[0]), text, flags=re.S)
    if (allow_guard and len(records) >= 3 and records[0][0] == "ifndef" and records[1][0] == "define"
            and IDENT.fullmatch(records[0][1]) and records[1][1] == records[0][1]
            and records[-1][0] == "endif" and not outer_branch
            and not clean[:records[0][2]].strip() and not clean[records[-1][3]:].strip()
            and not clean[records[0][3]:records[1][2]].strip()):
        guard = (records[0][2], records[-1][3])
        conditional = [span for span in conditional if span != guard]
        # Recover nested conditions that were formerly swallowed by the outer span.
        _, inner = pp_regions(text[records[1][3]:records[-1][2]], allow_guard=False)
        conditional.extend((a + records[1][3], b + records[1][3]) for a, b in inner)
    return directives, conditional


def function_parameter_open(ts, i):
    """Find parameters following a bare or parenthesised function name."""
    left, right = i - 1, i + 1
    while ts[right:right + 1] == [")"] and left >= 0 and ts[left] == "(":
        left -= 1
        right += 1
    return right if ts[right:right + 1] == ["("] else None


def function_definitions(text):
    ms = code_spans(text)
    directives, _ = pp_regions(text)
    indices = [i for i, m in enumerate(ms)
               if not any(a <= m.start() < b for a, b in directives)]
    ts = [ms[i][0] for i in indices]
    stack = []
    for tok in ts:
        if tok in ("{", "(", "["):
            stack.append(tok)
        elif tok in ("}", ")", "]"):
            if not stack or stack.pop() != {"}": "{", ")": "(", "]": "["}[tok]:
                raise ValueError("unbalanced C delimiters")
    if stack:
        raise ValueError("unbalanced C delimiters")
    definitions, depth = [], 0
    for i, tok in enumerate(ts):
        params = function_parameter_open(ts, i)
        if depth == 0 and tok not in C_KEYWORDS and IDENT.fullmatch(tok) and params is not None:
            end = balanced(ts, params, "(", ")")
            if ts[end + 1:end + 2] == ["{"]:
                close = balanced(ts, end + 1, "{", "}")
                definitions.append((tok, indices[i], indices[params], indices[close]))
        depth += (tok == "{") - (tok == "}")
    return definitions


def function_names(text: str) -> set[str]:
    return {name for name, *_ in function_definitions(text)}


def function_tokens(text: str, name: str) -> list[str]:
    if name in C_KEYWORDS:
        return []
    ts = [m[0] for m in code_spans(text)]
    for found, start, _, end in function_definitions(text):
        if found == name:
            return ts[start:end + 1]
    # Historical K&R headers have declarations between ')' and '{'. They must
    # remain visible to key reporting even though scoped binding parsing refuses
    # this declaration form.
    depth = 0
    for i, tok in enumerate(ts):
        if depth == 0 and tok == name and ts[i + 1:i + 2] == ["("]:
            end = balanced(ts, i + 1, "(", ")")
            if (ts[end + 1:end + 2] != [";"]
                    and all(IDENT.fullmatch(t) or t == "," for t in ts[i + 2:end])):
                body = next((k for k in range(end + 1, len(ts)) if ts[k] == "{"), None)
                if body is not None:
                    return ts[i:balanced(ts, body, "{", "}") + 1]
        depth += (tok == "{") - (tok == "}")
    return []


def original_census(text):
    # This is a name census, including declarations, for scoped rename proof.
    # Keep directive blanking and do not narrow it to C body candidates.
    directives, _ = pp_regions(text)
    for a, b in reversed(directives):
        text = text[:a] + re.sub(r"[^\n]", " ", text[a:b]) + text[b:]
    names = {m.group(1) for m in re.finditer(
        r"(?m)^(?:\}[ \t]*)?[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)\s*\(", text)}
    # Split heads (return type on its own line), as the engine reads them.
    names.update(inlineasm.definition_head_names(text))
    return names - C_KEYWORDS


def macro_uses(text: str, name: str, body_only=False, name_only=False) -> bool:
    records, pasted = macro_analysis(text)
    for macro, params, body in records:
        if name_only:
            if macro == name:
                return True
        elif body_only:
            if name in body and name not in (params or ()):
                return True
        elif name == macro or name in (params or ()) or name in body:
            return True
    return not body_only and not name_only and name in pasted


def macro_analysis(text):
    logical = blank_comments(re.sub(r"\\\r?\n", "", text))
    records = []
    for line in logical.splitlines():
        m = re.match(r"\s*(?:#|%:)\s*define\s+([A-Za-z_]\w*)(\([^)]*\))?(.*)", line)
        if m:
            params = frozenset(layer2.tokens(m[2])) if m[2] is not None else None
            records.append((m[1], params, tuple(layer2.tokens(m[3]))))
    ts = layer2.tokens(logical)
    pasted = {t for i, tok in enumerate(ts) if tok == "##" for t in ts[max(0, i - 1):i + 2]}
    return records, pasted


def literal_makefile(makefile):
    logical = re.sub(r"\\\r?\n", " ", makefile)
    names = r"(?:CPP\w*|CC\w*|GP_FILES|NO_SR_FILES|NOWARN_FILES|WARN_FLAGS)"
    if re.search(r"(?m)^\s*(?:-?include|sinclude)\s", logical):
        raise ValueError("unsupported Makefile include directive")
    if re.search(r"(?m)^\s*(?:(?:override|export|private|unexport)\s+)+(?:define\s+)?" + names + r"\b", logical):
        raise ValueError("unsupported Makefile modified toolchain assignment")
    if re.search(r"(?m)^\s*define\s+" + names + r"\b", logical):
        raise ValueError("unsupported Makefile defined toolchain assignment")
    if re.search(r"(?m)^[^#\n\t]*:[^=\n]*\b" + names + r"\s*[:+?]?=", logical):
        raise ValueError("unsupported Makefile target-specific toolchain assignment")
    known = {
        "FLAG_LISTS := GP_FILES PSYQ_LIBRARY_FILES DATA_ONLY_NO_SDATA_FILES EXPAND_LH_FILES NO_SR_FILES NOWARN_FILES",
        "cc_flags_for = $(if $(filter $1,$(GP_FILES)),$(CC_FLAGS_GP),$(CC_FLAGS))$(if $(filter $1,$(NO_SR_FILES)), -fno-strength-reduce)$(if $(filter $1,$(NOWARN_FILES)), -w, $(WARN_FLAGS))",
        "maspsx_flags_for = $(if $(filter $1,$(GP_FILES)),$(MASPSX_FLAGS_GP),$(MASPSX_FLAGS))$(if $(filter $1,$(PSYQ_LIBRARY_FILES) $(DATA_ONLY_NO_SDATA_FILES)),, -G8)$(if $(filter $1,$(EXPAND_LH_FILES)), --expand-lh)",
        "$(foreach l,$(FLAG_LISTS),$(foreach f,$($(l)),$(if $(filter $(f),$(C_IDS)),,$(error $(l) names '$(f)', which is not a TU (no $(SRC_DIR)/$(f).c)))))",
    }
    for line in logical.splitlines():
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        stripped = line.strip()
        if stripped in {"SHELL := /bin/bash", ".SHELLFLAGS := -o pipefail -c",
                        "C_FILES      := $(sort $(shell find $(SRC_DIR) -name '*.c'))",
                        "$(BUILD_DIR)/$(ASM_DIR)/%.o: $(ASM_DIR)/%.s $(shell find $(INCLUDE_DIR) -type f)",
                        "$(BUILD_DIR)/$(ASM_DIR)/data/%.o: $(ASM_DIR)/data/%.s $(shell find $(INCLUDE_DIR) -type f)"}:
            continue
        if (re.match(r"(?:export\s+)?(?:PATH|SHELL|CPATH|GCC_EXEC_PREFIX|DEPENDENCIES_OUTPUT|\.RECIPEPREFIX|\.SHELLFLAGS)\s*[:+?]?=", stripped)
                or re.match(r"(?:vpath\b|VPATH\s*[:+?]?=)", stripped)
                or re.match(r"export\s+(?:PATH|SHELL|CPATH|GCC_EXEC_PREFIX|DEPENDENCIES_OUTPUT)\b", stripped)
                or ("$(shell" in line and not stripped.startswith("PIPELINE_DEPS :="))
                or (re.match(r"[^#\s]+\.o\s*:", stripped) and "%" not in stripped.split(":", 1)[0])):
            raise ValueError("unsupported Makefile recipe/environment configuration")
        if "$(eval" in line or re.search(r"\bMAKEFLAGS\b", line):
            raise ValueError("unsupported Makefile dynamic/environment configuration")
        if stripped in known:
            continue
        recipe = line.startswith("\t") and stripped.startswith("$(CPP) $(CPP_FLAGS) $(CPP_DEFS) $< |")
        expected_deps = "PIPELINE_DEPS := Makefile $(CC1) engine/buildconfig.py tools/prologue_config.json expand_lb_funcs.txt multu_funcs.txt multu_pad_funcs.txt expand_dest_funcs.txt maspsx_prefill_label_funcs.txt maspsx_comm_syms.txt tools/prologue_fix.py tools/multu_pad.py $(wildcard tools/maspsx/*.py tools/maspsx/maspsx/*.py) $(shell find $(INCLUDE_DIR) -type f) $(wildcard asm/funcs/*.s) $(shell find $(SRC_DIR) -name '*.h')"
        dependency = " ".join(stripped.split()) == expected_deps
        if "$(call" in line:
            remainder = line.replace("$(call cc_flags_for,$*)", "").replace("$(call maspsx_flags_for,$*)", "")
            if not recipe or "$(call" in remainder:
                raise ValueError("unsupported Makefile call expression")
        if re.match(r"^\s*\$\(", line) and not line.startswith("\t"):
            if ":" not in line or "=" in line:
                raise ValueError("unsupported Makefile computed line or variable name")
        if re.search(r"\b" + names + r"\b", line):
            literal = re.match(r"^" + names + r"\s*(?::=|\+=)\s*", line)
            if not literal and not recipe and not dependency:
                raise ValueError("unsupported Makefile unrecognized toolchain use")
        prefix = line.split("=", 1)[0]
        if "$(" in prefix and not line.startswith("\t") and not dependency:
            # The project's pattern rules contain computed target paths, not variables.
            if not re.match(r"^\$\(BUILD_DIR\)/\$\(ASM_DIR\)/%\.o:", line) and not re.match(r"^\$\(BUILD_DIR\)/\$\(SRC_DIR\)/%\.o:", line):
                if "=" in line:
                    raise ValueError("unsupported Makefile computed variable name")
    return logical


def cpp_command(makefile):
    """Accept the project's literal configuration; unknown Make syntax fails."""
    logical = literal_makefile(makefile)
    values = {}
    for name in ("CPP", "CPP_FLAGS", "CPP_DEFS"):
        assignments = re.findall(r"(?m)^" + name + r"\s*([:+?]?=)\s*([^\n]*)", logical)
        if not assignments or assignments[0][0] != ":=":
            raise ValueError(f"cannot determine Makefile {name}")
        values[name] = []
        for i, (op, text) in enumerate(assignments):
            if "$" in text or (i and op != "+="):
                raise ValueError(f"ambiguous Makefile {name}")
            values[name].extend(shlex.split(text, comments=True))
    recipe = re.findall(r"(?m)^\t\s*\$\(CPP\)[^\n]*", logical)
    if len(recipe) != 1 or not recipe[0].strip().startswith("$(CPP) $(CPP_FLAGS) $(CPP_DEFS) $< |"):
        raise ValueError("cannot determine the Makefile cpp recipe")
    if len(values["CPP"]) != 1:
        raise ValueError("unsupported Makefile CPP command")
    return values["CPP"] + values["CPP_FLAGS"] + values["CPP_DEFS"]


def cpp_snapshot_roots(makefile):
    """Only source/header roots and explicitly configured include directories."""
    roots = {"src", "include"}
    command = cpp_command(makefile) if makefile is not None else []
    i = 1
    while i < len(command):
        arg, directory = command[i], None
        if arg in ("-I", "-iquote", "-isystem", "-idirafter"):
            i += 1
            if i == len(command):
                raise ValueError("missing cpp include directory")
            directory = command[i]
        elif arg.startswith("-I"):
            directory = arg[2:]
        if directory is not None:
            directory = normpath(directory.replace("\\", "/"))
            if directory == "-" or directory.startswith("/") or Path(directory).is_absolute() or ":" in directory or ".." in Path(directory).parts:
                raise ValueError(f"unsupported external cpp include directory: {directory}")
            roots.add(directory)
        i += 1
    return tuple(sorted(roots))


def command_line_macros(makefile):
    """Expose literal -D definitions to the same checks as source defines."""
    command = cpp_command(makefile) if makefile is not None else []
    definitions, i = [], 1
    while i < len(command):
        arg = command[i]
        if arg == "-D":
            i += 1
            if i == len(command):
                raise ValueError("missing cpp macro definition")
            arg = "-D" + command[i]
        if arg.startswith("-D"):
            name, equal, body = arg[2:].partition("=")
            if not re.fullmatch(r"[A-Za-z_]\w*(?:\([^)]*\))?", name):
                raise ValueError("unsupported cpp macro definition")
            definitions.append("#define " + name + " " + (body if equal else "1"))
        i += 1
    return "\n".join(definitions)


def closure_hazards(text, path):
    logical = blank_comments(re.sub(r"\\\r?\n", "", text), literals=True)
    if re.search(r"\b__has_include\w*\b", logical):
        raise ValueError(f"{path}: __has_include probe cannot be proved in a snapshot")
    if "##" in logical or "%:" in logical:
        raise ValueError(f"{path}: token paste or digraph directive in scoped TU closure")


def closure_macro_checks(records, scoped):
    names = {name for name, _, _ in records}
    for spec, new in (scoped or {}).items():
        old = scoped_parts(spec)[2]
        if old in names or new in names:
            raise ValueError(f"{spec}={new}: scoped names are not fresh in macros (#define name)")
    functions = {name for name, params, _ in records if params is not None}
    for name, params, body in records:
        braces = 0
        bad_braces = False
        for token in body:
            braces += (token == "{") - (token == "}")
            bad_braces |= braces < 0
        if bad_braces or braces or set(body) & {"struct", "union", "enum"}:
            raise ValueError(f"macro changes brace/aggregate scope in scoped TU closure: {name}")
        kind = "object-like" if params is None else "function-like"
        depth = 0
        for i, token in enumerate(body):
            depth += (token == "(") - (token == ")")
            if token in functions and token not in (params or ()):
                if body[i + 1:i + 2] != ("(",):
                    raise ValueError(f"{kind} macro call opener in scoped TU closure: {name}")
                try:
                    balanced(body, i + 1, "(", ")")
                except ValueError as e:
                    raise ValueError(f"{kind} macro call opener in scoped TU closure: {name}") from e
        if depth > 0:
            raise ValueError(f"{kind} macro call opener in scoped TU closure: {name}")


class CppOutput(dict):
    def __init__(self):
        super().__init__()
        self.macros = []
        self.sources = set()
        self.stream = []

    def add_tokens(self, site, text):
        tokens = [m[0] for m in code_spans(text)]
        k = 0
        while k < len(tokens):
            if tokens[k] == "_Pragma":
                if tokens[k + 1:k + 2] != ["("]:
                    raise ValueError("malformed cpp _Pragma operator")
                end = balanced(tokens, k + 1, "(", ")")
                del tokens[k:end + 1]
            else:
                k += 1
        self.setdefault(site, []).extend(tokens)
        self.stream.extend((site, token) for token in tokens)


def in_cpp_snapshot(path, roots):
    return path == "Makefile" or any(root == "." or path.startswith(root + "/") for root in roots)


def cleanup_cpp_snapshots(scratch):
    """Remove abandoned snapshots older than an hour, confined to scratch."""
    cutoff = time.time() - 3600
    for path in scratch.glob("keycheck-cpp-*"):
        try:
            if path.is_dir() and path.stat().st_mtime < cutoff:
                if path.is_symlink() or not path.resolve().is_relative_to(scratch.resolve()):
                    raise ValueError("unsafe stale cpp snapshot cleanup path")
                shutil.rmtree(path)
        except FileNotFoundError:
            pass  # Another run already removed this unique stale directory.



class CppTree(dict):
    def __init__(self, files, label):
        super().__init__(files)
        self.label, self.directory = label, None
        self._nonignored = getattr(files, "_nonignored", set(files))
        self._working_nonignored = getattr(files, "_working_nonignored", None)
        self._snapshot_lock = threading.Lock()

    def materialize(self):
        with self._snapshot_lock:
            return self._materialize()

    def _materialize(self):
        if self.directory is None:
            roots = cpp_snapshot_roots(self.get("Makefile"))
            scratch = ROOT / "tmp"
            scratch.mkdir(exist_ok=True)
            cleanup_cpp_snapshots(scratch)
            self.directory = scratch / ("keycheck-cpp-" + uuid.uuid4().hex + "-" + self.label)
            self.directory.mkdir()
            for path, text in self.items():
                if not in_cpp_snapshot(path, roots):
                    continue
                if Path(path).is_absolute() or ".." in Path(path).parts:
                    raise ValueError(f"unsafe cpp snapshot path {path}")
                destination = self.directory / path
                destination.parent.mkdir(parents=True, exist_ok=True)
                destination.write_bytes(text.encode("utf-8", errors="surrogateescape"))
        return self.directory

    def close(self):
        original = sys.exc_info()[0]
        if self.directory is not None and self.directory.exists():
            if not self.directory.resolve().is_relative_to((ROOT / "tmp").resolve()):
                raise ValueError("unsafe cpp snapshot cleanup path")
            for attempt in range(2):
                try:
                    shutil.rmtree(self.directory)
                    return
                except FileNotFoundError:
                    return
                except OSError as e:
                    if attempt == 0:
                        print(f"snapshot cleanup retry: {self.directory}: {e}", file=sys.stderr)
                    else:
                        message = f"snapshot cleanup failed: {self.directory}: {e}"
                        if original is not None:
                            print(message, file=sys.stderr)
                            return
                        raise ValueError(message) from e


def close_cpp_trees(trees):
    """Attempt every cleanup; a cleanup error cannot replace a failed proof."""
    original, failures = sys.exc_info()[0], []
    for tree in trees:
        try:
            tree.close()
        except (OSError, ValueError) as e:
            failures.append(f"snapshot cleanup failed: {tree.directory}: {e}")
    if failures:
        message = "\n".join(failures)
        if original is not None:
            print(message, file=sys.stderr)
        else:
            raise ValueError(message)


def preprocess(tree: CppTree, tu: str) -> str:
    """Injectable cpp boundary: exact Makefile command, cwd at snapshot root."""
    if "Makefile" not in tree:
        raise ValueError("scoped cpp proof needs the tree's Makefile")
    command = cpp_command(tree["Makefile"])
    binary = command[0]
    executable = str(ROOT / binary) if "/" in binary else shutil.which(binary)
    if not executable or not Path(executable).is_file():
        raise ValueError(f"scoped cpp proof toolchain missing: {binary}")
    try:
        directory = tree.materialize()
    except OSError as e:
        raise ValueError(f"scoped cpp proof snapshot materialization failed: {e}") from e
    try:
        result = subprocess.run([executable, *command[1:], "-dD", tu], cwd=directory,
                                capture_output=True, text=True, encoding="utf-8", errors="surrogateescape")
    except OSError as e:
        raise ValueError(f"scoped cpp proof cannot run {binary}: {e}") from e
    if result.returncode:
        raise ValueError(f"cpp failed for {tu}: {result.stderr.strip()[:400]}")
    return result.stdout


def cc1_command(makefile, tu):
    """Literal build cc1 configuration; unsupported Make recipes fail closed."""
    logical = literal_makefile(makefile)
    def value(name):
        rows = re.findall(r"(?m)^" + name + r"\s*([:+?]?=)\s*([^\n]*)", logical)
        if not rows or rows[0][0] != ":=" or any("$" in text or (i and op != "+=")
                                                for i, (op, text) in enumerate(rows)):
            raise ValueError(f"cannot determine Makefile {name}")
        return [word for _, text in rows for word in shlex.split(text, comments=True)]
    recipe = re.findall(r"(?m)^\t.*\$\(CPP\).*", logical)
    if len(recipe) != 1 or "| $(CC1) $(call cc_flags_for,$*) |" not in recipe[0]:
        raise ValueError("cannot determine Makefile cc1 recipe")
    rule = re.findall(r"(?m)^cc_flags_for\s*=\s*([^\n]*)", logical)
    expected = "$(if $(filter $1,$(GP_FILES)),$(CC_FLAGS_GP),$(CC_FLAGS))$(if $(filter $1,$(NO_SR_FILES)), -fno-strength-reduce)$(if $(filter $1,$(NOWARN_FILES)), -w, $(WARN_FLAGS))"
    if rule != [expected]:
        raise ValueError("cannot determine Makefile per-TU cc1 flags")
    ident = tu.removeprefix("src/").removesuffix(".c")
    flags = value("CC_FLAGS_GP" if ident in value("GP_FILES") else "CC_FLAGS")
    if ident in value("NO_SR_FILES"):
        flags.append("-fno-strength-reduce")
    flags += ["-w"] if ident in value("NOWARN_FILES") else value("WARN_FLAGS")
    if any(flag.startswith("-g") for flag in flags):
        raise ValueError("local cc1 proof requires no debug flags")
    binary = value("CC1")
    if len(binary) != 1:
        raise ValueError("unsupported Makefile CC1 command")
    return binary + flags


def compile_cc1(tree, tu):
    """Injectable runner: plain project cpp followed by build cc1, no asm transforms."""
    makefile = tree.get("Makefile")
    if makefile is None:
        raise ValueError("local cc1 proof needs the tree's Makefile")
    cpp, cc1 = cpp_command(makefile), cc1_command(makefile, tu)
    for command in (cpp, cc1):
        binary = command[0]
        executable = str(ROOT / binary) if "/" in binary else shutil.which(binary)
        if not executable or not Path(executable).is_file():
            raise ValueError(f"local cc1 proof toolchain missing: {binary}")
        command[0] = executable
    try:
        directory = tree.materialize()
    except OSError as e:
        raise ValueError(f"local cc1 proof snapshot materialization failed: {e}") from e
    try:
        source = subprocess.run([*cpp, tu], cwd=directory, capture_output=True,
                                text=True, encoding="utf-8", errors="surrogateescape")
        if source.returncode:
            raise ValueError(f"local cc1 proof cpp failed for {tu}: {source.stderr[:400]}")
        return subprocess.run(cc1, input=source.stdout, cwd=directory,
                              capture_output=True, text=True, encoding="utf-8", errors="surrogateescape")
    except OSError as e:
        raise ValueError(f"local cc1 proof cannot run toolchain: {e}") from e


def verify_local_cc1(base, normal, scoped, global_pairs, base_tree=None):
    """Authoritative local proof, text-mode BASE == FRESH == destination asm.

    Variant trees are built once per wave and reused across every include context.
    Simultaneous globals/members retain their separate source and cpp proofs.
    """
    local = {s: n for s, n in (scoped or {}).items() if ":" in s}
    if not local:
        return
    jobs, fresh_names = {}, []
    fresh_files, dest_files = dict(base), dict(base)
    replacements = {"fresh": {}, "destination": {}}
    cpp_tokens = {tu: [token for _, token in output.stream] for tu, output in normal.items()}
    cpp_definitions = {tu: cpp_functions(ts, declaration_types({"src/cpp.c": " ".join(ts)}))
                       for tu, ts in cpp_tokens.items()}
    begun = time.perf_counter()
    for k, (spec, dest) in enumerate(local.items()):
        scope, old = spec.split(":")
        scope = scoped_type_name(scope, global_pairs)
        found = []
        for path, text in base.items():
            if path.startswith(("src/", "include/")) and path.endswith((".c", ".h")):
                found.extend((path, a, b) for name, a, _, b in function_definitions(text) if name == scope)
        if len(found) != 1:
            raise ValueError(f"{spec}: local cc1 proof requires exactly one source definition")
        path, start, end = found[0]
        fresh = f"__kc_fresh_{k}"
        while any(fresh in text for text in base.values()) or any(fresh in token for ts in cpp_tokens.values() for token in ts):
            fresh += "_"
        fresh_names.append(fresh)
        text, spans = base[path], code_spans(base[path])
        tokens = [m[0] for m in spans]
        if dest in tokens[start:end + 1]:
            raise ValueError(f"{spec}: destination occurs in source function span")
        contexts = [tu for tu, output in normal.items() if path in output.sources]
        if not contexts:
            raise ValueError(f"{spec}: local cc1 proof target function never compiled")
        for tu in contexts:
            matching = [(a, b) for name, a, _, b in cpp_definitions[tu] if name == scope]
            if len(matching) != 1:
                raise ValueError(f"{spec}: local cc1 proof requires one cpp function span")
            a, b = matching[0]
            if dest in cpp_tokens[tu][a:b + 1]:
                raise ValueError(f"{spec}: destination occurs in cpp function span")
            if any(name == dest for name, _, _ in normal[tu].macros):
                raise ValueError(f"{spec}: destination is a cpp macro name")
            jobs.setdefault(tu, []).extend((old, dest, fresh))
        directives, _ = pp_regions(text)
        for index in range(start, end + 1):
            token = spans[index]
            if token[0] != old or local_exception(tokens, index) or any(a <= token.start() < b for a, b in directives):
                continue
            for label, replacement in (("fresh", fresh), ("destination", dest)):
                replacements[label].setdefault(path, []).append((token.start(), token.end(), replacement))
    for label, files in (("fresh", fresh_files), ("destination", dest_files)):
        for path, rows in replacements[label].items():
            for a, b, replacement in sorted(rows, reverse=True):
                files[path] = files[path][:a] + replacement + files[path][b:]
    trees = [CppTree(files, "cc1-" + label) for label, files in
             (("base", base), ("fresh", fresh_files), ("destination", dest_files))]
    borrowed = base_tree is not None and base_tree.directory is not None
    if borrowed:
        trees[0].directory = base_tree.directory
    try:
        for tu, names in sorted(jobs.items()):
            assemblies = []
            pattern = re.compile(r"(?<![\w.])(?:" + "|".join(re.escape(n) for n in names) + r")\.([0-9]+)(?![\w.])")
            for tree in trees:
                result = compile_cc1(tree, tu)
                if result.returncode or re.search(r"undeclared|\berror\b|fatal", result.stderr, re.I):
                    raise ValueError(f"local cc1 proof {tree.label} failed for {tu}: {result.stderr[:400]}")
                assembly = pattern.sub(r"__kc_static__.\1", result.stdout)
                if tree.label == "cc1-fresh" and any(re.search(r"(?<!\w)" + re.escape(n) + r"(?!\w)", assembly) for n in fresh_names):
                    raise ValueError(f"local cc1 proof fresh symbol or label emitted for {tu}")
                assemblies.append(assembly)
            if assemblies[1] != assemblies[0] or assemblies[2] != assemblies[0]:
                raise ValueError(f"local cc1 differential asm differs for {tu}")
    finally:
        close_cpp_trees(trees[1:] if borrowed else trees)
    print(f"  local cc1 differential proof: {len(jobs)} TUs, {time.perf_counter() - begun:.3f}s")


def emitted_lines(output, tree, tu):
    """Return tokens by source site, accepting only consistent include markers."""
    sites, stack, line, started = CppOutput(), [], 0, False
    checked, systems, empty_external = set(), set(), set()
    command_macros = command_line_macros(tree.get("Makefile"))
    closure_hazards(command_macros, "Makefile cpp definitions")

    def source_file(path):
        if path not in tree:
            raise ValueError(f"cpp marker names unavailable source file: {path}")
        if path not in checked:
            text = blank_comments(re.sub(r"\\\r?\n", "", tree[path]), literals=True)
            if re.search(r"(?m)^\s*(?:#|%:)\s*(?:line\b|[0-9])", text):
                raise ValueError(f"{path}: source line-control directive is forbidden")
            closure_hazards(tree[path], path)
            checked.add(path)
            sites.sources.add(path)

    for text in output.splitlines():
        marker = re.fullmatch(r'\s*#\s+(\d+)\s+("(?:\\.|[^"\\])*")((?:\s+[1-4])*)\s*', text)
        if marker:
            line = int(marker[1])
            filename = json.loads(marker[2])
            if not filename.startswith("<"):
                p = Path(filename)
                if p.is_absolute():
                    try:
                        directory = getattr(tree, "directory", None)
                        filename = p.relative_to(directory).as_posix() if directory else filename
                    except ValueError:
                        pass
                filename = normpath(filename.replace("\\", "/"))
            flags = [int(f) for f in marker[3].split()]
            if not stack:
                if filename != tu or flags:
                    raise ValueError("cpp first marker must name the TU")
                stack.append(filename)
                source_file(filename)
            elif 1 in flags:
                if 2 in flags or filename.startswith("<"):
                    raise ValueError("inconsistent cpp include-stack marker")
                if 3 in flags and filename != "/usr/mipsel-linux-gnu/include/stdc-predef.h":
                    raise ValueError("unsupported cpp system header: " + filename)
                if filename in tree:
                    source_file(filename)
                elif 3 in flags:
                    systems.add(filename)
                elif filename.startswith("/") or Path(filename).is_absolute():
                    empty_external.add(filename)
                else:
                    source_file(filename)
                stack.append(filename)
            elif 2 in flags:
                if len(stack) < 2 or filename != stack[-2]:
                    raise ValueError("inconsistent cpp include-stack return marker")
                stack.pop()
            elif filename != stack[-1]:
                # GCC's initial built-in/command-line prelude has no push/pop.
                if started or len(stack) != 1 or filename not in (tu, "<built-in>", "<command-line>"):
                    raise ValueError("inconsistent cpp include-stack file switch")
                stack[0] = filename
        else:
            if re.match(r"\s*#\s*[0-9]", text):
                raise ValueError("malformed cpp line marker")
            if re.match(r"\s*#\s*pragma\b", text):
                line += 1
                continue
            if re.match(r"\s*#\s*(?:define|undef)\b", text):
                closure_hazards(text, stack[-1] if stack else "cpp definitions")
                sites.macros.extend(macro_analysis(text)[0])
                line += 1
                continue
            if text.strip():
                if stack and stack[-1] in systems:
                    closure_hazards(text, stack[-1])
                    sites.add_tokens((stack[-1], line), text)
                    line += 1
                    continue
                if not stack or stack[-1].startswith("<") or stack[-1] in empty_external:
                    raise ValueError("cpp text outside a source file")
                started = True
                sites.add_tokens((stack[-1], line), text)
            line += 1
    if len(stack) > 1:
        raise ValueError("unterminated cpp include stack")
    return sites


def scoped_type_name(scope, global_pairs, new=False):
    sources = [old for old, dest in (global_pairs or {}).items() if dest == scope]
    if scope not in (global_pairs or {}) and sources:
        if len(sources) != 1:
            raise ValueError("ambiguous globally renamed scoped type: " + scope)
        scope = sources[0]
    return (global_pairs or {}).get(scope, scope) if new else scope


def knr_body(ts, end, types):
    """Only an actual identifier-list plus its parameter declarations is K&R."""
    opening = end - 1
    depth = 1
    while opening >= 0 and depth:
        depth += (ts[opening] == ")") - (ts[opening] == "(")
        opening -= 1
    params = ts[opening + 2:end]
    if not params or not all(IDENT.fullmatch(t) or t == "," for t in params):
        return None
    names = {t for t in params if IDENT.fullmatch(t)}
    j, seen = end + 1, set()
    while j < len(ts) and ts[j] in types:
        stop = next((k for k in range(j, len(ts)) if ts[k] in (";", "{", "}")), len(ts))
        if stop == len(ts) or ts[stop] != ";":
            return None
        try:
            declared = set(member_declarators(ts[j:stop]))
        except ValueError:
            return None
        if not declared or not declared <= names or declared & seen:
            return None
        seen.update(declared)
        j = stop + 1
    return j if seen == names and ts[j:j + 1] == ["{"] else None


def cpp_functions(ts, types):
    """Definitions only: no comma/prototype followed by an unrelated brace."""
    definitions, depth, boundary = [], 0, 0
    for i, token in enumerate(ts):
        params = function_parameter_open(ts, i)
        if (depth == 0 and IDENT.fullmatch(token) and token not in C_KEYWORDS
                and params is not None):
            prefix = ts[boundary:i]
            clean, j = [], boundary
            while j < i:
                if ts[j] in ("__attribute", "__attribute__") and ts[j + 1:j + 2] == ["("]:
                    j = balanced(ts, j + 1, "(", ")") + 1
                else:
                    clean.append(ts[j])
                    j += 1
            if clean and clean[0] in types and all(IDENT.fullmatch(t) or t in ("*", "(", ")") for t in clean):
                end = balanced(ts, params, "(", ")")
                tail = end + 1
                if "(" in clean and "*" in clean:
                    while tail < len(ts) and ts[tail] in (")", "("):
                        tail = tail + 1 if ts[tail] == ")" else balanced(ts, tail, "(", ")") + 1
                body = tail if ts[tail:tail + 1] == ["{"] else knr_body(ts, end, types)
                if body is not None:
                    definitions.append((token, params, body, balanced(ts, body, "{", "}")))
        depth += (token == "{") - (token == "}")
        if depth == 0 and token in (";", "}"):
            boundary = i + 1
    return definitions


def cpp_aggregate_table(ts):
    aliases, aggregates = {}, []
    for k in range(len(ts) - 4):
        if ts[k] == "typedef" and ts[k + 1] in ("struct", "union") and ts[k + 4] == ";":
            aliases.setdefault((ts[k + 1], ts[k + 2]), set()).add(ts[k + 3])
    for i, token in enumerate(ts):
        if token not in ("struct", "union"):
            continue
        j = i + 1
        tag = ts[j] if j < len(ts) and IDENT.fullmatch(ts[j]) else None
        j += tag is not None
        if ts[j:j + 1] != ["{"]:
            continue
        end = balanced(ts, j, "{", "}")
        owners = {tag} | aliases.get((token, tag), set())
        if i and ts[i - 1] == "typedef" and ts[end + 2:end + 3] == [";"]:
            owners.add(ts[end + 1])
        names, ambiguous = aggregate_members(" ".join(ts[i:end + 1]))
        aggregates.append((owners, names, ambiguous))
    return aggregates


def cpp_file_declarations(ts, types, functions, wanted):
    """Ordinary declarations only; never uses, tags or parameter names."""
    declared = {name for name, _, _, _ in functions}
    function_tokens = {k for _, start, _, end in functions for k in range(start, end + 1)}
    # Aggregate braces do not create an ordinary-identifier scope in C.
    for i, token in enumerate(ts):
        if token != "enum" or i in function_tokens:
            continue
        j = i + 1
        if j < len(ts) and IDENT.fullmatch(ts[j]):
            j += 1
        if ts[j:j + 1] == ["{"]:
            end = balanced(ts, j, "{", "}")
            first, depth = True, 0
            for k in range(j + 1, end):
                if first and IDENT.fullmatch(ts[k]):
                    declared.add(ts[k])
                    first = False
                depth += (ts[k] in ("(", "[", "{")) - (ts[k] in (")", "]", "}"))
                if ts[k] == "," and not depth:
                    first = True

    def declaration(row):
        j, typed = 0, False
        while j < len(row):
            token = row[j]
            if token in ("__attribute", "__attribute__"):
                if row[j + 1:j + 2] != ["("]:
                    break
                j = balanced(row, j + 1, "(", ")") + 1
            elif token in ("typeof", "__typeof", "__typeof__"):
                if row[j + 1:j + 2] != ["("]:
                    raise ValueError("cannot prove cpp typeof declaration")
                typed = True
                j = balanced(row, j + 1, "(", ")") + 1
            elif token in ("struct", "union", "enum"):
                typed = True
                j += 1
                if j < len(row) and IDENT.fullmatch(row[j]):
                    j += 1
                if row[j:j + 1] == ["{"]:
                    j = balanced(row, j, "{", "}") + 1
            elif token in DECL_WORDS or token == "typedef":
                typed |= token not in {"typedef", "extern", "static", "register", "auto", "const", "volatile",
                                       "__const", "__const__", "__volatile", "__volatile__", "inline", "__inline__",
                                       "__inline", "__extension__"}
                j += 1
            elif token in types and not typed:
                typed = True
                j += 1
            else:
                break
        if not typed:
            if set(row) & wanted:
                raise ValueError("cannot prove cpp file-scope declaration")
            return
        while j < len(row):
            while j < len(row) and row[j] in ("*", "(", "const", "volatile"):
                j += 1
            if j >= len(row):  # tag-only declaration
                break
            if not IDENT.fullmatch(row[j]) or row[j] in C_KEYWORDS:
                if set(row[j:]) & wanted:
                    raise ValueError("cannot prove cpp file-scope declarator")
                break
            declared.add(row[j])
            # Ignore all parameters, array sizes and initializer references.
            depth = 0
            j += 1
            while j < len(row):
                depth += (row[j] in ("(", "[", "{")) - (row[j] in (")", "]", "}"))
                # Parenthesized declarators can start before the name.
                depth = max(depth, 0)
                if row[j] == "," and not depth:
                    j += 1
                    break
                j += 1
    bodies = {k: end for _, opening, body, end in functions for k in (opening, body)}
    start = i = 0
    unknown_body = False
    while i < len(ts):
        if i in bodies:
            i = bodies[i] + 1
            start = i
            unknown_body = False
        elif ts[i] in ("{", "(", "["):
            aggregate = (i and ts[i - 1] in ("struct", "union", "enum")
                         or i >= 2 and ts[i - 2] in ("struct", "union", "enum"))
            if ts[i] == "{" and not aggregate and ts[i - 1:i] != ["="]:
                unknown_body = True
            close = balanced(ts, i, ts[i], {"{": "}", "(": ")", "[": "]"}[ts[i]])
            i = close + 1
        elif ts[i] == ";":
            if unknown_body and set(ts[start:i]) & wanted:
                raise ValueError("unrecognized cpp file-scope body involving scoped name")
            declaration(ts[start:i])
            i += 1
            start = i
            unknown_body = False
        else:
            i += 1
    if set(ts[start:]) & wanted:
        raise ValueError("unterminated cpp file-scope declaration")
    return declared


def file_typedef_types(ts, functions):
    excluded = {k for _, start, _, end in functions for k in range(start, end + 1)}
    return declaration_types({"src/cpp.c": " ".join(t for k, t in enumerate(ts) if k not in excluded)})


def typedef_shadow_check(ts, types, start, body, end, spec):
    typedefs = types - set(DECL_WORDS)
    param_end = balanced(ts, start, "(", ")")
    rows, first, depth = [], start + 1, 0
    for k in range(first, param_end + 1):
        token = ts[k] if k < param_end else ","
        if token == "," and not depth:
            rows.append(ts[first:k])
            first = k + 1
        depth += (token in ("(", "[")) - (token in (")", "]"))
    for k in range(body + 1, end):
        if ts[k] not in types or ts[k - 1] not in ("{", "}", ";"):
            continue
        j, depth = k, 0
        while j < end:
            token = ts[j]
            if token == ";" and not depth:
                rows.append(ts[k:j])
                break
            depth += (token in ("(", "[", "{")) - (token in (")", "]", "}"))
            j += 1
    for row in rows:
        if not row:
            continue
        declared = cpp_file_declarations(row + [";"], types, [], typedefs)
        if declared & typedefs:
            raise ValueError(f"{spec}: shadowed typedef prevents cpp binding proof")


def cpp_definition_ranges(ts, functions):
    """Find every declaration prefix in one ordered TU scan."""
    previous, starts = -1, {}
    openings = {opening for _, opening, _, _ in functions}
    for k, token in enumerate(ts):
        if k in openings:
            starts[k] = previous + 1
        if token in (";", "}"):
            previous = k
    return {fn: range(starts[opening], closing + 1) for fn, opening, _, closing in functions}


def typedef_source_index(source):
    """Token rows by starting line, built once per source text within the proof."""
    spans = code_spans(source)
    newlines = [m.start() for m in re.finditer("\n", source)]
    index = {}
    for k, row in typedef_rows(source):
        line = bisect_left(newlines, spans[k].start()) + 1
        index.setdefault(line, []).append(row)
    return index


def cpp_scope_checks(output, scoped, global_pairs, label, source_tree=None):
    """Reject-only cpp scope/binding/freshness prefilters; cc1 proves locals."""
    ts = [token for _, token in output.stream]
    text = " ".join(ts)
    types = declaration_types({"src/cpp.c": text})
    functions = cpp_functions(ts, types)
    types = file_typedef_types(ts, functions)
    functions = cpp_functions(ts, types)
    aggregates = cpp_aggregate_table(ts) if any("." in spec for spec in (scoped or {})) else []
    wanted = {name for spec, dest in (scoped or {}).items() if ":" in spec
              for name in (spec.split(":")[1], dest)}
    ordinary = cpp_file_declarations(ts, types, functions, wanted)
    definition_ranges = cpp_definition_ranges(ts, functions)
    binding_texts = {}
    uncertain = set()
    if source_tree is not None and any(":" in spec for spec in (scoped or {})):
        for index, row in typedef_rows(text):
            site = output.stream[index][0]
            source = source_tree.get(site[0], "")
            originals = typedef_source_index(source).get(site[1], [])
            if row not in originals:
                uncertain.update(_typedef_classes((" ".join(row),)))
    for spec, dest in (scoped or {}).items():
        scope, _member_path, old = scoped_parts(spec)
        if "." in spec:
            owner = scoped_type_name(scope, global_pairs, label == "new")
            if dest in types:
                raise ValueError(f"{spec}: member destination is a cpp typedef/type name")
            # Both trees are examined. In the new tree T itself may contain n;
            # every other aggregate must exclude a competing m/n declaration.
            for aliases, names, ambiguous in aggregates:
                if (old in names or old in ambiguous) and (dest in names or dest in ambiguous):
                    if label == "base" or owner not in aliases:
                        raise ValueError(f"{spec}: member destination is not fresh in cpp aggregates")
            continue
        scope = scoped_type_name(scope, global_pairs, label == "new")
        definitions = [f for f in functions if f[0] == scope]
        if not definitions:
            raise ValueError(f"{spec}: cpp expected exactly one function definition, found 0 (edit in code the build never compiles)")
        if len(definitions) != 1:
            raise ValueError(f"{spec}: cpp expected exactly one function definition")
        _, start, body, end = definitions[0]
        for name in (old, dest):
            if name in ordinary:
                raise ValueError(f"{spec}: ordinary {name} outside f in cpp file scope/enum")
        binding = old if label == "base" else dest
        region = ts[start:end + 1]
        # A typedef shadowed by an ordinary parameter/local cannot subsequently
        # certify a declaration. Fail rather than infer types from expressions.
        typedef_shadow_check(ts, types, start, body, end, spec)
        scratch = "__kc_binding_check__"
        while scratch in ts:
            scratch += "_"
        # Reuse the deliberately small binding parser on PREPROCESSED text.
        # This is independent evidence; raw-source planning is only preflight.
        # Remove unrelated function bodies before the binding parser sees
        # typedefs: their local types are never visible in f.
        if scope not in binding_texts:
            excluded = {k for fn, indices in definition_ranges.items() if fn != scope for k in indices}
            binding_texts[scope] = " ".join(t for k, t in enumerate(ts) if k not in excluded)
        binding_text = binding_texts[scope]
        _scoped_plan({"src/cpp.c": binding_text}, {scope + ":" + binding: scratch}, uncertain_types=frozenset(uncertain))
        if any(t in ("struct", "union", "enum") and "{" in region[k + 1:k + 3]
               for k, t in enumerate(region)):
            raise ValueError(f"{spec}: local in cpp aggregate/enum body")
    return functions


def sentinel_namespaces(output, names):
    """Check namespaces in cpp's ordered token stream, including across lines."""
    ts = [token for _, token in output.stream]
    declarations, conditional_colons, aggregate_bodies = {}, set(), set()
    nesting, questions = [], []
    for i, token in enumerate(ts):
        if token in ("(", "[", "{"):
            nesting.append((i, token))
        elif token in (")", "]", "}"):
            if not nesting or token != {"(": ")", "[": "]", "{": "}"}[nesting[-1][1]]:
                raise ValueError("unbalanced cpp namespace delimiters")
            if nesting:
                nesting.pop()
                questions = [q for q in questions if len(q) <= len(nesting)]
        elif token == "?":
            questions.append(tuple(nesting))
        elif token == ":" and questions and questions[-1] == tuple(nesting):
            questions.pop()
            conditional_colons.add(i)
    for i, token in enumerate(ts):
        if token in ("struct", "union", "enum"):
            j = i + 1
            if j < len(ts) and IDENT.fullmatch(ts[j]):
                j += 1
            if ts[j:j + 1] == ["{"]:
                aggregate_bodies.update(range(j + 1, balanced(ts, j, "{", "}")))
    functions = cpp_functions(ts, declaration_types({"src/cpp.c": " ".join(ts)}))
    # A member scope "A|B" (pairs sharing m and n) is declared by either.
    members = {alt.split(".")[0] for kind, scope in names.values() if kind == "member"
               for alt in scope.split("|")}
    nested_paths = {}
    for kind, scope in names.values():
        for alt in scope.split("|") if kind == "member" else ():
            if "." in alt:
                owner, *path = alt.split(".")
                nested_paths.setdefault(owner, set()).add(tuple(path))
    if members:
        forward_aliases = {}
        for k in range(len(ts) - 4):
            if ts[k] == "typedef" and ts[k + 1] in ("struct", "union") and ts[k + 4] == ";":
                forward_aliases.setdefault((ts[k + 1], ts[k + 2]), set()).add(ts[k + 3])
        for i, token in enumerate(ts):
            if token not in ("struct", "union"):
                continue
            j = i + 1
            tag = ts[j] if j < len(ts) and IDENT.fullmatch(ts[j]) else None
            j += tag is not None
            if ts[j:j + 1] != ["{"]:
                continue
            end = balanced(ts, j, "{", "}")
            aliases = {tag}
            if i and ts[i - 1] == "typedef" and ts[end + 2:end + 3] == [";"]:
                aliases.add(ts[end + 1])
            aliases.update(forward_aliases.get((token, tag), ()))
            owners = aliases & members
            if not owners:
                continue
            start = j + 1
            if "{" in ts[j + 1:end]:
                # A body with inline aggregates: its depth-1 declarator census.
                for q, name in container_declarators(ts, j, end).items():
                    if name in names:
                        declarations.setdefault(q, set()).update(owners)
            else:
                for k in range(start, end):
                    if ts[k] == ";":
                        try:
                            declared = member_declarators(ts[start:k])
                        except ValueError:
                            declared = ()
                        for q in range(start, k):
                            if ts[q] in names and ts[q] in declared:
                                declarations.setdefault(q, set()).update(owners)
                        start = k + 1
            # A nested member (T.u.m) is declared at depth 1 of the anonymous
            # aggregate the path reaches from T's body.
            for owner in owners:
                for path in nested_paths.get(owner, ()):
                    container = nested_container(ts, j, end, path)
                    if container is None:
                        continue
                    for q, name in container_declarators(ts, *container).items():
                        if name in names:
                            declarations.setdefault(q, set()).add(member_scope_key(owner, path))
    for i, token in enumerate(ts):
        if token not in names:
            continue
        kind, scope = names[token]
        prev = ts[i - 1] if i else None
        following = ts[i + 1] if i + 1 < len(ts) else None
        if kind == "local":
            if (i in aggregate_bodies
                    or not any(name == scope and start <= i <= end for name, start, _, end in functions)
                    or prev in (".", "->", "struct", "union", "enum", "goto")
                    or following == ":" and i + 1 not in conditional_colons):
                raise ValueError("local sentinel changed namespace in cpp output: " + token)
        elif prev not in (".", "->") and not set(scope.split("|")) & declarations.get(i, set()):
            if scope in ("\x00macro-argument", "\x00member-access"):
                raise ValueError("unchanged member name in a " + scope[1:].replace("-", " ")
                                 + " does not expand to a member access: " + token)
            raise ValueError("member sentinel changed namespace in cpp output: " + token)


def cpp_source_guards(output, tree, scoped):
    """Every static macro guard uses this TU's actual -dD definitions."""
    functions, objects = macro_table_records(output.macros)
    names = functions | objects.keys()
    for spec, dest in (scoped or {}).items():
        scope, _path, old = scoped_parts(spec)
        for macro, params, body in output.macros:
            if any(word in body and word not in (params or ()) for word in (old, dest)):
                raise ValueError(f"{spec}={dest}: scoped name in cpp macro body: {macro}")
        for path in output.sources:
            text = tree[path]
            spans = code_spans(text)
            ts = [token[0] for token in spans]
            if "." in spec:
                # Member pairs: an edited argument token (n after . / ->) is proven
                # by its edit sentinel (emitted by a compiled TU, never dropped,
                # stringized or pasted, always after . or ->); an unchanged one by
                # its kept sentinel. The static test refuses macros that may drop,
                # stringize or paste an argument naming m.
                reject_unexpanded_macro_arguments(text, old, output.macros, path)
                continue
            for func, start, _, end in function_definitions(text):
                if func != scope:
                    continue
                reject_macro_arguments(text, old, functions, path, range(start, end + 1))
                for k in range(start, end + 1):
                    if ts[k] not in (old, dest):
                        continue
                    adjacent = (k and ts[k - 1] in names) or ts[k + 1:k + 2] and ts[k + 1] in names
                    if k and ts[k - 1] == ")":
                        depth, j = 1, k - 2
                        while j >= start and depth:
                            depth += (ts[j] == ")") - (ts[j] == "(")
                            j -= 1
                        adjacent = adjacent or not depth and j >= start and ts[j] in functions
                    if adjacent:
                        raise ValueError(f"{path}: local {old} adjacent to cpp macro invocation")


def sentinel_occurrences(output, name):
    """Reject strings/pastes even if another occurrence is a valid identifier."""
    count = 0
    for token in output:
        if name in token:
            if token != name or not IDENT.fullmatch(token):
                raise ValueError("sentinel stringified or pasted: " + name)
            count += 1
    return count


def dependency_identity(tree, path):
    root = tree.directory or ROOT
    candidate = Path(path)
    if not candidate.is_absolute():
        candidate = root / candidate
    candidate = os.path.normpath(str(candidate))
    try:
        stat = os.stat(candidate)
        return (stat.st_dev, stat.st_ino)
    except FileNotFoundError:
        return os.path.normcase(candidate)


def cpp_dependencies(tree, tu):
    """Injectable real cpp -M boundary, with the same literal project cpp flags."""
    if "Makefile" not in tree:
        raise ValueError("scoped dependency census needs the tree's Makefile")
    command = cpp_command(tree["Makefile"])
    binary = command[0]
    executable = str(ROOT / binary) if "/" in binary else shutil.which(binary)
    if not executable or not Path(executable).is_file():
        raise ValueError(f"scoped dependency census toolchain missing: {binary}")
    try:
        directory = tree.materialize()
    except OSError as e:
        raise ValueError(f"dependency snapshot materialization failed: {e}") from e
    try:
        result = subprocess.run([executable, *command[1:], "-M", "-MT", "__kc__", tu],
                                cwd=directory, capture_output=True, text=True,
                                encoding="utf-8", errors="surrogateescape")
    except OSError as e:
        raise ValueError(f"dependency cpp cannot run {binary}: {e}") from e
    if result.returncode:
        raise ValueError(f"dependency cpp failed for {tu}: {result.stderr[:400]}")
    paths = parse_cpp_dependencies(result.stdout, tu)
    return validate_cpp_dependencies(tree, paths, executable, command[1:])


def parse_cpp_dependencies(output, tu):
    """Decode cpp's Make quoting, never shell quoting (quotes are filename bytes)."""
    text = re.sub(r"\\\r?\n", " ", output).strip()
    if not text.startswith("__kc__:") or "\n" in text or "\r" in text:
        raise ValueError(f"cannot parse cpp dependencies for {tu}")
    paths, token, i = set(), [], len("__kc__:")
    while i < len(text):
        ch = text[i]
        if ch in " \t":
            if token:
                paths.add("".join(token))
                token = []
        elif ch == "\\":
            i += 1
            if i == len(text) or text[i] not in " \t#\\":
                raise ValueError(f"ambiguous cpp dependency escape for {tu}")
            token.append(text[i])
        elif ch == "$":
            i += 1
            if i == len(text) or text[i] != "$":
                raise ValueError(f"ambiguous cpp dependency dollar for {tu}")
            token.append("$")
        elif ch in "#:\x00":
            raise ValueError(f"ambiguous cpp dependency token for {tu}")
        else:
            token.append(ch)
        i += 1
    if token:
        paths.add("".join(token))
    if not paths:
        raise ValueError(f"cpp dependency census empty for {tu}")
    return paths


def cpp_system_directories(tree, executable, flags):
    """Discover compiler-owned include roots once per snapshot with cpp -v."""
    with tree._snapshot_lock:
        if not hasattr(tree, "_system_directories"):
            try:
                result = subprocess.run([executable, *flags, "-v", "-E", "-"],
                                        cwd=tree.directory, input="", capture_output=True,
                                        text=True, encoding="utf-8", errors="surrogateescape")
            except OSError as e:
                raise ValueError(f"cannot determine cpp system include directories: {e}") from e
            if result.returncode:
                raise ValueError("cannot determine cpp system include directories")
            match = re.search(r"#include <\.\.\.> search starts here:\n(.*?)End of search list\.",
                              result.stderr, re.S)
            if not match:
                raise ValueError("cannot determine cpp system include directories")
            # -I directories belong to the snapshot; only absolute compiler roots qualify.
            tree._system_directories = tuple(os.path.normpath(line.strip())
                for line in match[1].splitlines() if os.path.isabs(line.strip()))
        return tree._system_directories


def validate_cpp_dependencies(tree, paths, executable, flags):
    """All dependencies must exist at cpp's cwd and remain in the snapshot."""
    root = os.path.abspath(tree.directory)
    result = set()
    if not hasattr(tree, "_checked_dependencies"):
        with tree._snapshot_lock:
            if not hasattr(tree, "_checked_dependencies"):
                tree._checked_dependencies = set()
    for path in paths:
        if ".." in path.replace("\\", "/").split("/"):
            raise ValueError(f"cpp dependency dotdot component: {path}")
        candidate = os.path.normpath(os.path.join(root, path))
        if candidate in tree._checked_dependencies:
            result.add(path)
            continue
        try:
            inside = os.path.commonpath((root, candidate)) == root
        except ValueError as e:
            raise ValueError(f"cpp dependency outside snapshot: {path}") from e
        if not inside:
            # Never let /proc or an absolute include into the real repository qualify.
            repo = os.path.abspath(ROOT)
            if (candidate.startswith("/proc/") or
                    os.path.commonpath((repo, candidate)) == repo):
                raise ValueError(f"cpp dependency outside snapshot: {path}")
            dirs = cpp_system_directories(tree, executable, flags)
            if not any(os.path.commonpath((d, candidate)) == d for d in dirs):
                raise ValueError(f"cpp dependency outside snapshot: {path}")
        with tree._snapshot_lock:
            if candidate not in tree._checked_dependencies:
                try:
                    info = os.stat(candidate)
                except OSError as e:
                    raise ValueError(f"cpp dependency does not exist at cpp cwd: {path}") from e
                if not stat_mode.S_ISREG(info.st_mode):
                    raise ValueError(f"cpp dependency is not a file: {path}")
                tree._checked_dependencies.add(candidate)
        result.add(path)
    return result



def dependency_census(tree, edited):
    """Every TU is queried, in a bounded pool; only cpp can exclude a TU."""
    # A snapshot cannot reproduce existence probes into omitted files. Scan before
    # selection, including unrelated TUs that such a probe could otherwise hide.
    roots = cpp_snapshot_roots(tree.get("Makefile"))
    for path in cpp_readable_sources(tree, roots):
        text = re.sub(r"\\\n", "", tree[path])
        if re.search(r"\b__has_include\w*\b", text):
            raise ValueError(f"{path}: __has_include probe cannot be proved in a snapshot")
    tus = sorted(p for p in tree if p.startswith("src/") and p.endswith(".c"))
    with ThreadPoolExecutor(max_workers=min(8, os.cpu_count() or 1)) as pool:
        records = list(pool.map(lambda tu: cpp_dependencies(tree, tu), tus))
    # Dependency inputs, including ignored headers and .inc files, receive the same gate.
    opened = set().union(*records) if records else set()
    if tree._working_nonignored is not None:
        ignored = (opened & tree.keys()) - tree._working_nonignored
        if ignored:
            raise ValueError("working-tree cpp dependency is ignored: " + sorted(ignored)[0])
    # Initial C/header inputs were gated by the readers/main. Gate every additional
    # dependency here; this includes ignored inputs and cpp-opened assembler files.
    additional = {p for p in opened if p not in tree._nonignored or not p.endswith((".c", ".h"))}
    if additional & tree.keys():
        inputs = ShapeTree(tree)
        inputs._nonignored = set()
        inputs._dependency_sources = additional & tree.keys()
        plain_tree(inputs)
    identities = {}
    def identify(path):
        if path not in identities:
            identities[path] = dependency_identity(tree, path)
        return identities[path]
    wanted = {identify(path) for path in edited}
    selected = set()
    for tu, paths in zip(tus, records):
        dependencies = {identify(path) for path in paths}
        if identify(tu) not in dependencies:
            raise ValueError(f"cpp dependency census omitted its TU: {tu}")
        if wanted & dependencies:
            selected.add(tu)
    return selected


def verify_compiled_edits(base, new, edits, map_path=lambda p: p, scoped=None, global_pairs=None):
    with analysis_cache():
        return _verify_compiled_edits(base, new, edits, map_path, scoped, global_pairs)


def _verify_compiled_edits(base, new, edits, map_path, scoped, global_pairs):
    """Prove every edit with unique cpp sentinels, never token alignment."""
    changes, local_spans = [], []
    for path, replacements in edits.items():
        target = map_path(path)
        if path not in base or target not in new:
            continue
        before, after = code_spans(base[path]), code_spans(new[target])
        if len(before) != len(after):
            raise ValueError(f"{target}: cannot map scoped tokens to cpp source lines")
        for k, (dest, _) in replacements.items():
            destinations = dest if isinstance(dest, tuple) else (dest,)
            if before[k][0] != after[k][0] and after[k][0] in destinations:
                changes.append((path, target, k))
    if not changes:
        if any(":" in spec for spec in (scoped or {})):
            raise ValueError("local cc1 proof requires an applied local edit")
        return
    for spec, dest in (scoped or {}).items():
        if ":" not in spec:
            continue
        func, old = spec.split(":")
        for path, text in base.items():
            if not path.startswith(("src/", "include/")) or not path.endswith((".c", ".h")):
                continue
            for name, start, _, end in function_definitions(text):
                if name == scoped_type_name(func, global_pairs):
                    local_spans.append((path, map_path(path), start, end, old, dest))
    namespaces = {}
    for path, target, k in changes:
        old, dest = code_spans(base[path])[k][0], code_spans(new[target])[k][0]
        owners = []
        for spec, replacement in (scoped or {}).items():
            scope, member_path, source = scoped_parts(spec)
            if source != old or replacement != dest:
                continue
            if "." in spec:
                owners.append(("member", member_scope_key(scoped_type_name(scope, global_pairs), member_path)))
            elif any(name == scoped_type_name(scope, global_pairs) and start <= k <= end
                     for name, start, _, end in function_definitions(base[path])):
                owners.append(("local", scoped_type_name(scope, global_pairs)))
        if len(owners) > 1 and all(kind == "member" for kind, _ in owners):
            # Member pairs sharing m and n: the sentinel may sit in any of their
            # aggregates (an access needs only . or ->).
            owners = [("member", "|".join(sorted(scope for _, scope in owners)))]
        if len(owners) != 1:
            raise ValueError(f"{target}: cannot establish scoped edit namespace")
        namespaces[(path, k)] = namespaces[(target, k)] = owners[0]
    # Member pairs: every unchanged m inside a function-like macro argument is also
    # sentinelized below (an edited one already is, as an edit) and must come out of cpp as a member access (after . or ->)
    # in some compiled TU; a dropped, stringized or pasted argument, nested-macro
    # forwarding into one, #if 0 code or an unincluded header fails. Then a stale m
    # that reached the renamed T.m is compiled, and the oracle build catches it.
    # A macro invocation's expansion comes out on its first line, so a kept or edited
    # site inside one is checked over the invocation's whole line span (merged per file).
    keep, keep_spans = [], {}
    member_olds = {scoped_parts(spec)[2] for spec in (scoped or {}) if "." in spec}
    if member_olds:
        cfiles = {p: t for p, t in base.items()
                  if p.startswith(("src/", "include/")) and p.endswith((".c", ".h"))}
        functions = macro_table(cfiles)[0]
        changed = {(p, k) for p, _, k in changes}
        for path, text in sorted(cfiles.items()):
            target = map_path(path)
            if target not in new:
                raise ValueError(f"{path}: missing from the new tree; cannot prove kept {'/'.join(sorted(member_olds))}")
            spans = code_spans(text)
            after = code_spans(new[target]) if new[target] != text else spans
            if len(after) != len(spans):
                raise ValueError(f"{target}: cannot map tokens to the base tree; cannot prove kept {'/'.join(sorted(member_olds))}")
            in_macro = set()
            for opener, close, args in macro_invocations(text, functions):
                hits = [k for k in sorted(args) if spans[k][0] in member_olds
                        and (path, k) not in changed and after[k][0] == spans[k][0]]
                in_macro.update(hits)
                if not hits and not any((path, k) in changed for k in args):
                    continue
                keep.extend((path, target, k, "macro argument") for k in hits)
                for side, name, body, toks in (("base", path, text, spans), ("new", target, new[target], after)):
                    keep_spans.setdefault((side, name), []).append(
                        (body.count("\n", 0, toks[opener].start()) + 1,
                         body.count("\n", 0, toks[close].start()) + 1))
            # A retained plain .m / ->m outside any macro argument must be emitted by
            # a compiled TU too: in #if 0 code or an unincluded header it would keep
            # the stale name unseen by the oracle build.
            directives, _ = pp_regions(text)
            keep.extend((path, target, k, "member access") for k in range(1, len(spans))
                        if spans[k][0] in member_olds and spans[k - 1][0] in (".", "->")
                        and k not in in_macro and (path, k) not in changed
                        and after[k][0] == spans[k][0]
                        and not any(a <= spans[k].start() < b for a, b in directives))
    for name, intervals in keep_spans.items():
        merged = []
        for first, last in sorted(intervals):
            if merged and first <= merged[-1][1]:
                merged[-1] = (merged[-1][0], max(merged[-1][1], last))
            else:
                merged.append((first, last))
        keep_spans[name] = merged

    def line_span(side, path, line):
        for first, last in keep_spans.get((side, path), ()):
            if first <= line <= last:
                return first, last
        return line, line
    touched = {path for path, _, _ in changes}
    all_tus = {path for path in base if path.startswith("src/") and path.endswith(".c")}
    trees = {label: CppTree(files, label) for label, files in (("base", base), ("new", new))}
    if getattr(new, "_working_nonignored", None) is not None:
        for tree in trees.values():
            tree._working_nonignored = new._working_nonignored
    cache, normal, sentinel_trees = {}, {}, {}
    try:
        before = dependency_census(trees["base"], touched)
        after = dependency_census(trees["new"], {map_path(path) for path in touched})
        if any("." in spec for spec in (scoped or {})):
            tus = sorted(all_tus)  # Member freshness retains its complete aggregate census.
        else:
            tus = sorted(before | {p for p in all_tus if map_path(p) in after})
        for label, tree in trees.items():
            normal[label] = {}
            for tu in tus:
                target = map_path(tu) if label == "new" else tu
                key = (label, target)
                if key not in cache:
                    cache[key] = emitted_lines(preprocess(tree, target), tree, target)
                edit_sites = set()
                for p, q, k in changes:
                    f = p if label == "base" else q
                    first, last = line_span(label, f, tree[f].count("\n", 0, code_spans(tree[f])[k].start()) + 1)
                    edit_sites.update((f, l) for l in range(first, last + 1))
                active_scoped = {spec: dest for spec, dest in (scoped or {}).items()
                    if "." in spec or any(
                        (p if label == "base" else q) in cache[key].sources
                        and any(fn == scoped_type_name(spec.split(":")[0], global_pairs)
                                and opening == start for fn, opening, _, _ in function_definitions(base[p]))
                        for p, q, start, _, old, n in local_spans
                        if old == spec.split(":")[1] and n == dest)}
                if edit_sites & cache[key].keys():
                    closure_macro_checks(cache[key].macros, active_scoped)
                    resolved = {scoped_type_name(spec.split(":")[0], global_pairs, label == "new") + ":" + spec.split(":")[1] if ":" in spec else spec: dest
                                for spec, dest in active_scoped.items()}
                    cpp_source_guards(cache[key], tree, resolved)
                functions = cpp_scope_checks(cache[key], active_scoped, global_pairs, label, tree)
                for path, renamed, start, end, old, dest in local_spans:
                    source = path if label == "base" else renamed
                    if source not in cache[key].sources:
                        continue
                    spans = code_spans(tree[source])
                    expected = ((source, tree[source].count("\n", 0, spans[start].start()) + 1),
                                (source, tree[source].count("\n", 0, spans[end].start()) + 1))
                    matching = [(a, b) for f, a, _, b in functions
                                if f in {scoped_type_name(spec.split(":")[0], global_pairs, label == "new") for spec in (scoped or {}) if ":" in spec}
                                and cache[key].stream[a][0] == expected[0]
                                and cache[key].stream[b][0] == expected[1]]
                    if not matching:
                        raise ValueError(f"{source}: cpp function span disagrees with source (edit in code the build never compiles)")
                # Census every TU's include markers; sentinel/cc1 runs need only
                # contexts containing an edit or a kept macro-argument m (member
                # census above still sees all).
                keep_sites = set()
                for p, q, k, _ in keep:
                    f = p if label == "base" else q
                    line = tree[f].count("\n", 0, code_spans(tree[f])[k].start()) + 1
                    first, last = line_span(label, f, line)
                    keep_sites.update((f, l) for l in range(first, last + 1))
                if (edit_sites | keep_sites) & cache[key].keys():
                    normal[label][target] = cache[key]
        prefix = "__kc_s" + uuid.uuid4().hex + "_"
        while any(prefix in text for tree in trees.values() for text in tree.values()):
            prefix = "__kc_s" + uuid.uuid4().hex + "_"
        for label, tree in trees.items():
            wanted = {}
            for path, target, k in changes:
                wanted[(path if label == "base" else target, k)] = "edit"
            for path, target, k, kind in keep:
                wanted.setdefault((path if label == "base" else target, k), kind)
            for path, target, start, end, old, dest in local_spans:
                filename = path if label == "base" else target
                text = tree[filename]
                directives, _ = pp_regions(text)
                spans = code_spans(text)
                first = text.rfind("\n", 0, spans[start].start()) + 1
                last = text.find("\n", spans[end].end())
                if last < 0:
                    last = len(text)
                starts = [token.start() for token in spans]
                for k in range(bisect_left(starts, first), bisect_left(starts, last)):
                    token = spans[k]
                    if token[0] in (old, dest) and not any(a <= token.start() < b for a, b in directives):
                        wanted.setdefault((filename, k), "local survival")
            # Protect every source occurrence of the same spelling on an edited
            # line, so the normal-output count can be compared without alignment.
            # A line inside a kept macro invocation uses the invocation's whole
            # line span (cpp emits the expansion on its first line).
            line_starts = {}

            def span_offsets(path, first_line, last_line):
                if path not in line_starts:
                    text = tree[path]
                    line_starts[path] = [0] + [i + 1 for i, c in enumerate(text) if c == "\n"] + [len(text) + 1]
                starts = line_starts[path]
                return starts[first_line - 1], starts[min(last_line, len(starts) - 1)] - 1

            for (path, k), kind in list(wanted.items()):
                spans = code_spans(tree[path])
                token = spans[k]
                first_line, last_line = line_span(label, path, tree[path].count("\n", 0, token.start()) + 1)
                first, last = span_offsets(path, first_line, last_line)
                starts = [m.start() for m in spans]
                for other in range(bisect_left(starts, first), bisect_left(starts, last)):
                    if spans[other][0] == token[0]:
                        wanted.setdefault((path, other), "spelling context")
            contents, replacements, sentinels, namespace_names = dict(tree), {}, {}, {}
            for i, ((path, k), kind) in enumerate(sorted(wanted.items())):
                token = code_spans(tree[path])[k]
                site = (path, *line_span(label, path, tree[path].count("\n", 0, token.start()) + 1))
                name = prefix + str(i) + "__"
                sentinels[name] = (site, kind, token[0])
                if kind == "edit":
                    kind_name, scope = namespaces[(path, k)]
                    namespace_names[name] = (kind_name, scoped_type_name(scope, global_pairs, label == "new")
                                             )
                elif kind in ("macro argument", "member access"):
                    # A scope no aggregate declares: the sentinel must follow . or ->.
                    namespace_names[name] = ("member", "\x00" + kind.replace(" ", "-"))
                replacements.setdefault(path, []).append((token.start(), token.end(), name))
            for path, items in replacements.items():
                for start, end, name in sorted(items, reverse=True):
                    contents[path] = contents[path][:start] + name + contents[path][end:]
            marked = CppTree(contents, label + "-sentinel")
            sentinel_trees[label] = marked
            marked_outputs = {}
            for tu, output in normal[label].items():
                key = (marked.label, tu)
                if key not in cache:
                    cache[key] = emitted_lines(preprocess(marked, tu), marked, tu)
                marked_outputs[tu] = cache[key]
            counts = {}
            for name, (site, kind, spelling) in sentinels.items():
                path, first_line, last_line = site
                lines = [(path, line) for line in range(first_line, last_line + 1)]
                applicable = [tu for tu, output in normal[label].items() if any(l in output for l in lines)]
                if not applicable:
                    if kind in ("macro argument", "member access"):
                        raise ValueError(f"{site[0]}:{site[1]}: unchanged {spelling} in a {kind} in code the build never compiles ({label})")
                    raise ValueError(f"{site[0]}:{site[1]}: edit in code the build never compiles ({label})")
                for tu in applicable:
                    found = 0
                    for location, tokens in marked_outputs[tu].items():
                        count = sentinel_occurrences(tokens, name)
                        if count and location not in lines:
                            raise ValueError(f"{site[0]}:{site[1]}: sentinel emitted at wrong source line ({label})")
                        found += count
                    if not found:
                        raise ValueError(f"{site[0]}:{site[1]}: sentinel discarded by cpp ({kind}, {label})")
                    key = (tu, site, spelling)
                    counts[key] = counts.get(key, 0) + found
            for (tu, site, spelling), count in counts.items():
                path, first_line, last_line = site
                if sum(normal[label][tu].get((path, line), []).count(spelling)
                       for line in range(first_line, last_line + 1)) != count:
                    raise ValueError(f"{site[0]}:{site[1]}: original spelling/count differs from sentinel ({label})")
            for output in marked_outputs.values():
                sentinel_namespaces(output, namespace_names)
            # All source spellings in each local scope have been sentinelized.
            # Remaining old/new identifiers on its output lines came from expansion.
            for path, target, start, end, old, dest in local_spans:
                filename = path if label == "base" else target
                tokens = code_spans(tree[filename])
                first = tree[filename].count("\n", 0, tokens[start].start()) + 1
                last = tree[filename].count("\n", 0, tokens[end].end()) + 1
                for output in marked_outputs.values():
                    for (source, line), row in output.items():
                        if source == filename and first <= line <= last and any(token in (old, dest) for token in row):
                            raise ValueError(f"{filename}:{line}: macro expansion produces or captures local {old}/{dest} ({label})")
        verify_local_cc1(base, normal["base"], scoped, global_pairs, trees["base"])
    finally:
        close_cpp_trees([*trees.values(), *sentinel_trees.values()])


def macro_table(cfiles):
    return macro_table_records(record for text in cfiles.values() for record in macro_analysis(text)[0])


def macro_table_records(records):
    functions, objects = set(), {}
    for name, params, body in records:
        if params is not None:
            functions.add(name)
        else:
            objects[name] = body
    # Object-like aliases cannot hide calls of function-like macros.
    changed = True
    while changed:
        aliases = {name for name, body in objects.items() if set(body) & functions}
        changed = not aliases <= functions
        functions.update(aliases)
    return functions, objects


def reject_macro_arguments(text, old, functions, path, indices=None, member_access=False):
    """Refuse `old` inside a function-like macro's arguments. member_access: only an
    occurrence after `.` / `->` counts (a member pair's destination n: elsewhere n may
    be an unrelated local or global)."""
    ms = code_spans(text)
    ts = [m[0] for m in ms]
    directives, _ = pp_regions(text)

    def holds(a, b):
        return any(ts[q] == old and (not member_access or ts[q - 1] in (".", "->"))
                   for q in range(a, b))
    for k in range(len(ts)) if indices is None else indices:
        if any(a <= ms[k].start() < b for a, b in directives):
            continue
        if ts[k] in functions and ts[k + 1:k + 2] == ["("]:
            end = balanced(ts, k + 1, "(", ")")
            if holds(k + 2, end):
                raise ValueError(f"{path}: {old} is an argument of function-like macro {ts[k]}")
            while ts[end + 1:end + 2] == ["("]:
                start = end + 1
                end = balanced(ts, start, "(", ")")
                if holds(start + 1, end):
                    raise ValueError(f"{path}: {old} is an argument of chained function-like macro {ts[k]}")


def reject_unexpanded_macro_arguments(text, old, records, path):
    """Member pairs keep an unchanged macro argument naming m only when every
    definition of that macro expands all of its parameters plainly (each occurs in
    the body, never stringized or pasted): then a stale m that reached the renamed
    T.m would fail to compile, which the required oracle build catches. A macro that
    may drop, stringize or paste an argument, an object-like alias of a function-like
    macro, and chained invocations still fail."""
    forms = {}
    for name, params, body in records:
        forms.setdefault(name, []).append((params, body))
    functions, _ = macro_table_records(records)
    ms = code_spans(text)
    ts = [m[0] for m in ms]
    directives, _ = pp_regions(text)
    for k in range(len(ts)):
        if any(a <= ms[k].start() < b for a, b in directives):
            continue
        if ts[k] not in functions or ts[k + 1:k + 2] != ["("]:
            continue
        end = balanced(ts, k + 1, "(", ")")
        if ts[end + 1:end + 2] == ["("] and old in ts[k + 2:balanced(ts, end + 1, "(", ")")]:
            raise ValueError(f"{path}: {old} is an argument of chained function-like macro {ts[k]}")
        if old not in ts[k + 2:end]:
            continue
        defs = forms.get(ts[k], [])
        if not defs or any(params is None for params, _ in defs):
            raise ValueError(f"{path}: {old} is an argument of function-like macro alias {ts[k]}")
        for params, body in defs:
            names = [p for p in params if IDENT.fullmatch(p)]
            if "..." in params or "__VA_ARGS__" in body:
                raise ValueError(f"{path}: {old} is an argument of variadic macro {ts[k]}")
            for p in names:
                if p not in body:
                    raise ValueError(f"{path}: {old} is an argument of function-like macro {ts[k]}, which drops a parameter")
                for j, tok in enumerate(body):
                    if tok == p and ((j and body[j - 1] in ("#", "##")) or body[j + 1:j + 2] == ("##",)):
                        raise ValueError(f"{path}: {old} is an argument of function-like macro {ts[k]}, which stringizes or pastes a parameter")


def macro_invocations(text, functions):
    """(name index, closing index, argument indices) of each function-like macro
    invocation outside directives; chained invocations (`F(a)(b)`) are one span."""
    ms = code_spans(text)
    ts = [m[0] for m in ms]
    directives, _ = pp_regions(text)
    out = []
    for k in range(len(ts)):
        if any(a <= ms[k].start() < b for a, b in directives):
            continue
        if ts[k] in functions and ts[k + 1:k + 2] == ["("]:
            end = balanced(ts, k + 1, "(", ")")
            args = set(range(k + 2, end))
            while ts[end + 1:end + 2] == ["("]:
                start = end + 1
                end = balanced(ts, start, "(", ")")
                args.update(range(start + 1, end))
            out.append((k, end, args))
    return out


def macro_argument_indices(text, functions):
    """Token indices inside the arguments of function-like macro invocations
    (chained invocations included), outside directives."""
    return {k for _, _, args in macro_invocations(text, functions) for k in args}


def member_freshness(base, name):
    for path, text in base.items():
        if not path.startswith(("src/", "include/")) or not path.endswith((".c", ".h")):
            continue
        if macro_uses(text, name, body_only=True) or macro_uses(text, name, name_only=True):
            raise ValueError(f"{path}: member destination {name} is not fresh in macros")
        if name in macro_analysis(text)[1]:
            raise ValueError(f"{path}: member destination {name} is used with token paste")
        names, ambiguous = aggregate_members(text)
        if name in names or name in ambiguous:
            raise ValueError(f"{path}: member destination {name} is not fresh or cannot be excluded in aggregate declarations")


def aggregate_members(text):
    directives, _ = pp_regions(text)
    ts = [m[0] for m in code_spans(text) if not any(a <= m.start() < b for a, b in directives)]
    names, ambiguous = set(), set()
    for i, tok in enumerate(ts):
        if tok not in ("struct", "union"):
            continue
        j = i + 1
        if j < len(ts) and IDENT.fullmatch(ts[j]):
            j += 1
        if ts[j:j + 1] != ["{"]:
            continue
        end = balanced(ts, j, "{", "}")
        start, depth = j + 1, 0
        for k in range(start, end):
            depth += (ts[k] in ("{", "(", "[")) - (ts[k] in ("}", ")", "]"))
            if ts[k] == ";" and depth == 0:
                try:
                    names.update(member_declarators(ts[start:k]))
                except ValueError:
                    ambiguous.update(ts[start:k])
                start = k + 1
        if ts[start:end]:
            raise ValueError("aggregate member declaration is missing its final semicolon")
    return names, ambiguous


def statement_starts(ts, body, end, types):
    """Forward scan of a function body `ts[body..end]` (body is its `{`): the
    statement start of each token, and the tokens inside GNU statement
    expressions. Each `{` is a block, a brace expression (an initializer list,
    a compound literal after a cast) or a statement expression `({`; a brace
    that fits none fails, so no declaration can hide inside a misread brace."""
    starts, in_stmt_expr, stack, start = {}, set(), [], body + 1

    def cast_before(close):
        depth, opening = 0, close
        while opening > body:
            depth += (ts[opening] == ")") - (ts[opening] == "(")
            if depth == 0:
                break
            opening -= 1
        # A cast's `(` never follows a name or a closer: `g(void) {` and
        # `(g)(void) {` are function declarators, not compound literals.
        callee = ts[opening - 1]
        return opening, (opening + 1 < close and ts[opening + 1] in types
                         and not (IDENT.fullmatch(callee) and callee not in C_KEYWORDS)
                         and callee not in (")", "]"))

    for k in range(body, end + 1):
        t = ts[k]
        starts[k] = start
        if any(kind == "stmtexpr" for kind, _ in stack):
            in_stmt_expr.add(k)
        if t == "{":
            prev = ts[k - 1]
            inner = stack[-1][0] if stack else None
            if k == body:
                kind = "block"
            elif prev == "(":
                kind = "stmtexpr"
            elif inner in ("block", "stmtexpr") and prev in (";", "{", "}", "else", "do", ":"):
                kind = "block"
            elif inner in ("block", "stmtexpr") and prev == ")":
                opening, cast = cast_before(k - 1)
                if ts[opening - 1] in ("if", "while", "for", "switch"):
                    kind = "block"
                elif (IDENT.fullmatch(ts[opening - 1]) and ts[opening - 1] not in C_KEYWORDS
                      and (ts[opening - 2] in types or ts[opening - 2] == "*")):
                    kind = "block"   # a GNU nested function's body
                elif cast:
                    kind = "brace"
                else:
                    raise ValueError(f"cannot classify the brace after `{ts[opening - 1]} (...)`")
            elif inner in ("block", "stmtexpr") and prev == "=":
                kind = "brace"
            elif inner not in ("block", "stmtexpr") and prev in ("{", ",", "=", ":"):
                kind = "brace"
            elif inner not in ("block", "stmtexpr") and prev == ")" and cast_before(k - 1)[1]:
                kind = "brace"
            else:
                raise ValueError(f"cannot classify the brace after `{prev}`")
            stack.append((kind, start))
            if kind in ("block", "stmtexpr"):
                start = k + 1
        elif t in ("(", "["):
            stack.append((t, start))
        elif t in (")", "]", "}"):
            opener = {")": "(", "]": "["}.get(t)
            if not stack or (stack[-1][0] != opener if opener else stack[-1][0] in ("(", "[")):
                raise ValueError("unbalanced function body")
            kind, saved = stack.pop()
            if kind == "block":
                start = k + 1
            elif kind == "stmtexpr":
                start = saved
        elif t == ";" and (not stack or stack[-1][0] in ("block", "stmtexpr")):
            start = k + 1
    return starts, in_stmt_expr


def local_exception(ts, k):
    """Tokens in namespaces other than the local's ordinary identifier binding."""
    prev = ts[k - 1] if k else None
    if prev in (".", "->", "goto", "struct", "union", "enum"):
        return True
    parens = brackets = 0
    questions, label_colons, braces = [], set(), []
    pending_case = False
    for i, tok in enumerate(ts[:k]):
        if tok in ("case", "default"):
            pending_case = True
        if tok == "{":
            braces.append(i)
        elif tok == "}" and braces:
            braces.pop()
        if tok == "?":
            questions.append((parens, brackets, len(braces)))
        elif tok == ":":
            if questions and questions[-1] == (parens, brackets, len(braces)):
                questions.pop()
            elif not parens and not brackets and not questions and pending_case:
                label_colons.add(i)
                pending_case = False
            elif (not parens and not brackets and not questions and i >= 2
                  and IDENT.fullmatch(ts[i - 1])
                  and (ts[i - 2] in (";", "{", "}") or i - 2 in label_colons)):
                label_colons.add(i)
        parens += (tok == "(") - (tok == ")")
        brackets += (tok == "[") - (tok == "]")
    if (ts[k + 1:k + 2] == [":"] and prev in ("{", ",") and braces
            and not any(q == (parens, brackets, len(braces)) for q in questions)):
        return True  # GNU initializer designator (possibly in a compound literal)
    return (ts[k + 1:k + 2] == [":"] and not parens and not brackets and not questions
            and (prev in (";", "{", "}", ",") or k - 1 in label_colons))


def aggregate_indices(ts):
    """All tokens inside aggregate definitions, including nested declarations."""
    out = set()
    for i, tok in enumerate(ts):
        if tok not in ("struct", "union"):
            continue
        j = i + 1
        if j < len(ts) and IDENT.fullmatch(ts[j]):
            j += 1
        if ts[j:j + 1] == ["{"]:
            out.update(range(j + 1, balanced(ts, j, "{", "}")))
    return out


def aggregate_unchanged(old, new, edits, global_pairs=None):
    """Only explicit member declaration edits may change aggregate interiors.
    Checking the whole interior is deliberately stricter than guessing which
    identifiers in complex/nested declarators name members."""
    if global_pairs:
        pat = re.compile(r"\b(?:" + "|".join(map(re.escape, global_pairs)) + r")\b")
        old = nw.sub_c(pat, lambda m: global_pairs[m[0]], old)[0]
    a, b = [m[0] for m in code_spans(old)], [m[0] for m in code_spans(new)]
    if len(a) != len(b):
        return False
    return all(b[k] == (edits[k][0] if k in edits and not edits[k][1] else a[k])
               for k in aggregate_indices(a))


def member_declarators(ts):
    names, parts, depth, start = [], [], 0, 0
    for i, tok in enumerate(ts + [","]):
        depth += (tok in ("[", "(")) - (tok in ("]", ")"))
        if tok == "," and depth == 0:
            parts.append(ts[start:i])
            start = i + 1
    for i, part in enumerate(parts):
        array = next((k for k, tok in enumerate(part) if tok == "["), len(part))
        prefix = " ".join(part[:array])
        pattern = (r"(?:[A-Za-z_]\w* )+(?:\* )*([A-Za-z_]\w*)" if i == 0
                   else r"(?:\* )*([A-Za-z_]\w*)")
        match = re.fullmatch(pattern, prefix)
        if not match:
            raise ValueError("ambiguous/unsupported member declaration")
        if i == 0:
            qualifiers = {"const", "volatile", "__const", "__const__", "__volatile", "__volatile__",
                          "static", "extern", "register", "auto", "inline", "__inline", "__inline__"}
            if not any(IDENT.fullmatch(t) and t not in qualifiers for t in part[:array - 1]):
                raise ValueError("qualifier-only implicit-int member declaration")
        names.append(match[1])
        j = array
        while j < len(part):
            if part[j] != "[":
                raise ValueError("unsupported member array declarator")
            end = balanced(part, j, "[", "]")
            if not all(IDENT.fullmatch(t) or re.fullmatch(r"(?:0[xX][0-9a-fA-F]+|[0-9]+)[uUlL]*", t)
                       or t in ("+", "-", "*", "/", "%", "<<", ">>", "&", "|", "^", "~", "(", ")")
                       for t in part[j + 1:end]):
                raise ValueError("unsupported member array size")
            j = end + 1
    return names


def declaration_types(cfiles):
    types = set(DECL_WORDS)
    for text in cfiles.values():
        ts = [m[0] for m in code_spans(text)]
        for i, tok in enumerate(ts):
            if tok == "typedef":
                j = i + 1
                while j < len(ts):
                    if ts[j] in ("__attribute", "__attribute__", "typeof", "__typeof", "__typeof__"):
                        j += 1
                        if ts[j:j + 1] != ["("]:
                            raise ValueError("unsupported GNU typedef specifier")
                        j = balanced(ts, j, "(", ")") + 1
                    elif ts[j] in ("struct", "union", "enum"):
                        j += 1
                        if j < len(ts) and IDENT.fullmatch(ts[j]):
                            j += 1
                        if ts[j:j + 1] == ["{"]:
                            j = balanced(ts, j, "{", "}") + 1
                    elif ts[j] in types:
                        j += 1
                    else:
                        if j == i + 1 and IDENT.fullmatch(ts[j]):  # underlying typedef from an external header
                            j += 1
                        break
                # Specifiers may precede `typedef` (`int typedef T;`): a name right
                # after it is then the declared type, else the underlying one --
                # a type name either way.
                if i + 1 < len(ts) and IDENT.fullmatch(ts[i + 1]) and ts[i + 1] not in C_KEYWORDS:
                    types.add(ts[i + 1])
                depth = 0
                candidate = None
                while j < len(ts):
                    tok = ts[j]
                    if tok in ("__attribute", "__attribute__") and ts[j + 1:j + 2] == ["("]:
                        j = balanced(ts, j + 1, "(", ")") + 1
                        continue
                    if depth == 0 and tok == ";":
                        if candidate:
                            types.add(candidate)
                        break
                    if depth == 0 and tok == ",":
                        if candidate:
                            types.add(candidate)
                        candidate = None
                    elif candidate is None and IDENT.fullmatch(tok) and tok not in C_KEYWORDS:
                        candidate = tok
                    depth += (tok in ("(", "[")) - (tok in (")", "]"))
                    j += 1
    return types


def typedef_words(cfiles):
    """Every identifier of every typedef declaration, aggregate bodies aside: a
    superset of the typedef names that no declarator spelling (an attribute
    inside the declarator, say) can slip past."""
    words = set()
    for text in cfiles.values():
        ts = [m[0] for m in code_spans(text)]
        for i, tok in enumerate(ts):
            if tok != "typedef":
                continue
            depth, j = 0, i + 1
            while j < len(ts) and not (depth <= 0 and ts[j] == ";"):
                if ts[j] == "{":
                    j = balanced(ts, j, "{", "}") + 1
                    continue
                depth += (ts[j] in ("(", "[")) - (ts[j] in (")", "]"))
                if IDENT.fullmatch(ts[j]) and ts[j] not in C_KEYWORDS:
                    words.add(ts[j])
                j += 1
    return words


def typedef_rows(text):
    spans = code_spans(text)
    ts = [m[0] for m in spans]
    rows = []
    for i, token in enumerate(ts):
        if token == "typedef":
            depth, end = 0, len(ts)
            for j in range(i + 1, len(ts)):
                if ts[j] == ";" and not depth:
                    end = j
                    break
                depth += (ts[j] in ("(", "[", "{")) - (ts[j] in (")", "]", "}"))
            rows.append((i, ts[i:end + 1]))
    return rows


def declarator_kind(row, start):
    """Read name-outward operators: *F() is function, (*F)() is pointer object."""
    i, pointers = start, 0
    while row[i:i + 1] == ["*"]:
        pointers += 1
        i += 1
        while i < len(row) and row[i] in ("const", "volatile", "__const__", "__volatile__"):
            i += 1
    if i < len(row) and IDENT.fullmatch(row[i]) and row[i] not in C_KEYWORDS:
        name, operations = row[i], []
        i += 1
    elif row[i:i + 1] == ["("]:
        name, operations, i = declarator_kind(row, i + 1)
        if row[i:i + 1] != [")"]:
            raise ValueError("uncertain typedef declarator")
        i += 1
    else:
        raise ValueError("uncertain typedef declarator")
    while i < len(row) and row[i] in ("(", "["):
        left = row[i]
        operations.append("function" if left == "(" else "object")
        i = balanced(row, i, left, ")" if left == "(" else "]") + 1
    operations.extend(["object"] * pointers)
    return name, operations, i


def _typedef_classes(texts):
    """Classify simple typedefs; GNU, macro and multi-declarator forms are unknown."""
    rows, classes = [], {}
    for text in texts:
        directives, _ = pp_regions(text)
        spans = code_spans(text)
        for index, row in typedef_rows(text):
            produced = any(a <= spans[index].start() < b for a, b in directives)
            rows.append((row, produced))
    for _ in range(len(rows) + 1):
        changed = False
        for row, produced in rows:
            try:
                names = declaration_types({"src/cpp.c": " ".join(row)}) - set(DECL_WORDS)
            except ValueError:
                names = {row[-2]} if len(row) >= 2 and IDENT.fullmatch(row[-2]) else set()
            depth, multiple = 0, False
            for token in row:
                if token == "," and not depth:
                    multiple = True
                depth += (token in ("(", "[", "{")) - (token in (")", "]", "}"))
            invalid = produced or multiple or any(t in ("typeof", "__typeof", "__typeof__") for t in row)
            kind, j, typed = "unknown", 1, False
            try:
                while j < len(row):
                    token = row[j]
                    if token in ("struct", "union", "enum"):
                        typed, kind = True, "object"
                        j += 1
                        if j < len(row) and IDENT.fullmatch(row[j]):
                            j += 1
                        if row[j:j + 1] == ["{"]:
                            j = balanced(row, j, "{", "}") + 1
                    elif token in DECL_WORDS or token == "typedef":
                        if token in ("void", "char", "short", "int", "long", "float", "double", "signed", "unsigned", "__signed__"):
                            typed, kind = True, "object"
                        j += 1
                    elif not typed:
                        kind = classes.get(token, "unknown")
                        typed = True
                        j += 1
                    else:
                        break
                name, operations, end = declarator_kind(row, j)
                names.add(name)
                if invalid or row[end:] != [";"] or kind == "unknown":
                    result = "unknown"
                else:
                    result = operations[0] if operations else kind
            except ValueError:
                result = "unknown"
            for name in names:
                if classes.get(name) != result:
                    classes[name] = result
                    changed = True
        if not changed:
            break
    return classes




CPLUSPLUS_MACROS = frozenset({"__cplusplus", "c_plusplus", "_LANGUAGE_C_PLUS_PLUS"})
_CPLUSPLUS_TERM = r"(?:defined\s*\(\s*(\w+)\s*\)|defined\s+(\w+)|(\w+))"


def cplusplus_only_spans(text):
    """Spans of the first branch of each #if / #ifdef whose condition tests only C++ macros
    (CPLUSPLUS_MACROS, joined by ||). cc1 compiles C, so that branch is inactive; an #else
    branch stays active. A conditional with an #elif is not handled."""
    directives, _ = pp_regions(text)
    clean = blank_comments(text, literals=True)
    rows = []
    for begin, end in directives:
        m = re.match(r"\s*#\s*(\w+)(.*)", clean[begin:end], re.S)
        rows.append((m[1], m[2].strip(), begin, end))
    spans = []
    for k, (word, cond, _, end) in enumerate(rows):
        if word == "ifdef":
            only = cond in CPLUSPLUS_MACROS
        elif word == "if":
            terms = re.split(r"\s*\|\|\s*", cond)
            only = all((m := re.fullmatch(_CPLUSPLUS_TERM, t)) and
                       next(g for g in m.groups() if g) in CPLUSPLUS_MACROS for t in terms)
        else:
            continue
        if not only:
            continue
        depth = 0
        for word2, _, begin2, _ in rows[k + 1:]:
            if word2 in ("if", "ifdef", "ifndef"):
                depth += 1
            elif word2 == "endif" and depth:
                depth -= 1
            elif not depth and word2 in ("else", "endif"):
                spans.append((end, begin2))
                break
            elif not depth and word2 == "elif":
                break
    return spans


def file_scope_tokens(text, all_macros=frozenset(), known_types=frozenset()):
    text = re.sub(r"\\\r?\n", "", text)
    # C++-only branches (extern "C" { ... } guards) are not compiled: blank them in place,
    # unless one of those macros is defined (c_plusplus is an ordinary identifier to cc1).
    if not CPLUSPLUS_MACROS & (set(all_macros) | {name for name, _, _ in macro_analysis(text)[0]}):
        for a, b in cplusplus_only_spans(text):
            text = text[:a] + re.sub(r"[^\n]", " ", text[a:b]) + text[b:]
    ms = code_spans(text)
    ts = [m[0] for m in ms]
    directives, conditions = pp_regions(text)
    for a, b in conditions:
        # Every branch must balance independently; inactive directives are not C.
        boundaries = [a] + [x for x, y in directives if a < x < b and
                            re.match(r"\s*#\s*(?:elif|else|endif)\b", text[x:y])] + [b]
        for first, last in zip(boundaries, boundaries[1:]):
            braces = sum((m[0] == "{") - (m[0] == "}") for m in ms
                         if first <= m.start() < last and not any(x <= m.start() < y for x, y in directives))
            if braces:
                raise ValueError("unbalanced braces in preprocessor conditional cannot establish file scope")
    pp = {i for i, m in enumerate(ms) if any(a <= m.start() < b for a, b in directives)}
    excluded = set(pp)
    definitions = {start: (begin, end) for _, start, begin, end in function_definitions(text)}
    macro_names = {name for name, _, _ in macro_analysis(text)[0]} | set(all_macros)
    for begin, end in definitions.values():
        excluded.update(range(begin, end + 1))
    depth, boundary = 0, 0
    for i, tok in enumerate(ts):
        if i in pp:
            continue
        if depth == 0 and IDENT.fullmatch(tok) and tok not in C_KEYWORDS and ts[i + 1:i + 2] == ['(']:
            end = balanced(ts, i + 1, '(', ')')
            prefix = [ts[k] for k in range(boundary, i) if k not in pp]
            typed = bool(prefix and all(IDENT.fullmatch(t) or t == '*' for t in prefix)
                         and prefix[0] not in ('return', 'sizeof') and tok not in macro_names
                         and not set(prefix) & macro_names)
            if typed and ts[end + 1:end + 2] == [';']:
                excluded.update(range(i + 1, end + 1))
            elif typed and i not in definitions:
                body = knr_body(ts, end, set(known_types) | declaration_types({"src/cpp.c": text}))
                if body is not None:
                    excluded.update(range(i + 1, balanced(ts, body, '{', '}') + 1))
        depth += (tok == '{') - (tok == '}')
        if depth == 0 and tok in (';', '}'):
            boundary = i + 1
    visible, enums, depth = set(), set(), 0
    for i, tok in enumerate(ts):
        if i in pp:
            continue
        if tok == 'enum':
            j = i + 1
            if j < len(ts) and IDENT.fullmatch(ts[j]):
                j += 1
            if ts[j:j + 1] == ['{']:
                enums.update(ts[j + 1:balanced(ts, j, '{', '}')])
        if i in excluded:
            continue
        if depth == 0:
            visible.add(tok)
        depth += (tok == '{') - (tok == '}')
    return visible, enums


def local_freshness(base, name, all_macros=frozenset(), known_types=None):
    if known_types is None:
        known_types = frozenset(declaration_types({p: t for p, t in base.items() if p.startswith(("src/", "include/")) and p.endswith((".c", ".h", ".inc"))}))
    for path, text in base.items():
        if not path.startswith(('src/', 'include/')) or not path.endswith(('.c', '.h', '.inc')):
            continue
        if macro_uses(text, name, body_only=True) or macro_uses(text, name, name_only=True):
            raise ValueError(f'{path}: destination {name} occurs in a #define name/body or token paste')
        visible, enums = file_scope_tokens(text, all_macros, known_types)
        if name in enums or name in visible:
            raise ValueError(f'{path}: destination {name} is not fresh at file scope or in an enum body')


@contextmanager
def analysis_cache():
    """Bound memoization to one proof; do not retain entire revision texts."""
    names = ("code_spans", "pp_regions", "function_definitions", "macro_analysis",
             "file_scope_tokens", "aggregate_members", "_typedef_classes", "typedef_source_index")
    saved = {name: globals()[name] for name in names}
    try:
        for name, fn in saved.items():
            globals()[name] = lru_cache(maxsize=None)(fn)
        yield
    finally:
        globals().update(saved)


def scoped_plan(base: dict[str, str], pairs: dict[str, str], globals=None):
    with analysis_cache():
        return _scoped_plan(base, pairs, globals)


def _scoped_plan(base: dict[str, str], pairs: dict[str, str], globals=None, uncertain_types=frozenset()):
    """Return path -> token-index substitutions. Deliberately a small C subset:
    simple member declarators and one simple parameter/local binding. Reject
    constructs we cannot distinguish from another binding, never guess."""
    plan = {}
    if "Makefile" in base:
        base = dict(base)
        virtual = "include/.keycheck-cpp-defs.h"
        if virtual in base:
            raise ValueError("reserved cpp macro proof path exists")
        base[virtual] = command_line_macros(base["Makefile"])
    cfiles = {p: t for p, t in base.items() if p.startswith(("src/", "include/")) and p.endswith((".c", ".h"))}
    macro_sources = {p: t for p, t in base.items() if p.startswith(("src/", "include/"))}
    macros, object_macros = macro_table(cfiles)
    macro_records = [r for t in cfiles.values() for r in macro_analysis(t)[0]]
    types = declaration_types(cfiles)
    typedef_names = typedef_words(cfiles)
    global_pat = re.compile(r"\b(?:" + ("|".join(map(re.escape, globals or {})) or "(?!)") + r")\b")
    transformed_base = {p: global_pat.sub(lambda m: globals[m[0]], t) for p, t in base.items()}
    transformed_types = declaration_types({p: t for p, t in transformed_base.items() if p in cfiles or p.startswith(("src/", "include/")) and p.endswith(".inc")})
    for spec, new in pairs.items():
        if new in C_KEYWORDS:
            raise ValueError(f"{spec}: destination {new} is a C/GNU keyword")
        if new in (globals or {}) or new in (globals or {}).values():
            raise ValueError(f"{spec}: scoped destination {new} collides with a global pair")
        peers = [o for o, n in pairs.items() if o != spec and n == new]
        if "." in spec and peers:
            # naming-bar §5: members the code shows hold the same thing may share
            # one new name when all of those pairs land in this wave. Every sharer
            # is a member pair; each names a member of a different aggregate,
            # checked on the resolved definitions below (spellings can alias). n
            # stays fresh against every other member, type and macro.
            if any("." not in o for o in peers):
                raise ValueError(f"{spec}: scoped destination {new} is shared by a pair that is not a distinct member pair")
        elif any("." in spec or "." in o or scoped_type_name(spec.split(":")[0], globals) == scoped_type_name(o.split(":")[0], globals) for o in peers):
            raise ValueError(f"{spec}: scoped destination {new} is shared by another pair")
        old = scoped_parts(spec)[2]
        if any(macro_uses(text, old, name_only=True) or macro_uses(text, new, name_only=True)
               for text in macro_sources.values()):
            raise ValueError(f"{spec}={new}: scoped names are not fresh in macros (#define name)")
    outer_defs = {}  # member spec -> (path, token index of its outer aggregate's "{")
    for spec, new in pairs.items():
        member = "." in spec
        scope, member_path, old = scoped_parts(spec)
        scope = scoped_type_name(scope, globals)
        for macro_path, macro_text in cfiles.items():
            if not member and macro_uses(macro_text, new, body_only=True):
                raise ValueError(f"{spec}: destination {new} occurs in a #define body or token paste in {macro_path}")
        if member:
            if new in types or new in transformed_types:
                raise ValueError(f"{spec}: member destination is a typedef/type name")
            member_freshness(base, new)
            member_freshness(transformed_base, new)
        else:
            local_freshness(transformed_base, new, frozenset(macros) | frozenset(object_macros), frozenset(transformed_types))
        found = []
        for path, text in cfiles.items():
            ms = code_spans(text)
            ts = [m[0] for m in ms]
            if member:
                # Macro arguments naming m (kept or edited) are allowed when the
                # macro expands them plainly; n is fresh, so none occurs in the base.
                reject_macro_arguments(text, new, macros, path, member_access=True)
                reject_unexpanded_macro_arguments(text, old, macro_records, path)
                if macro_uses(text, old, body_only=True):
                    raise ValueError(f"{spec}: {old} occurs in a #define body or token paste ({path})")
                if any(tok == "##" and old in ts[max(0, i - 1):i + 2] for i, tok in enumerate(ts)):
                    raise ValueError(f"{spec}: {old} is used with token paste ({path})")
                for i, tok in enumerate(ts):
                    if tok not in ("struct", "union"):
                        continue
                    j = i + 1
                    tag = ts[j] if j < len(ts) and IDENT.fullmatch(ts[j]) else None
                    j += bool(tag)
                    if j >= len(ts) or ts[j] != "{":
                        continue
                    end = balanced(ts, j, "{", "}")
                    aliases = []
                    if i and ts[i - 1] == "typedef":
                        k = end + 1
                        while k < len(ts) and ts[k] != ";":
                            aliases.append(ts[k])
                            k += 1
                        if scope in aliases and (len(aliases) != 1 or not IDENT.fullmatch(aliases[0])):
                            raise ValueError(f"{spec}: complex typedef alias is unsupported")
                    # Also the common forward typedef: typedef struct Tag Alias;
                    if tag:
                        for k in range(len(ts) - 4):
                            if ts[k:k + 3] == ["typedef", tok, tag] and ts[k + 4] == ";":
                                aliases.append(ts[k + 3])
                    if scope != tag and scope not in aliases:
                        continue
                    if "#" in ts[j + 1:end]:
                        raise ValueError(f"{spec}: nested or preprocessed aggregate is unsupported")
                    hits = [k for k in range(j + 1, end) if ts[k] == old]
                    if len(hits) != 1:
                        raise ValueError(f"{spec}: member must occur exactly once in T's definition")
                    k = hits[0]
                    # The member's container: T's body, or the anonymous aggregate the
                    # nested path names (T.u.m). With inline aggregates inside it, the
                    # depth-1 declarator census decides; a plain body keeps the
                    # member_declarators rule.
                    container = (j, end) if not member_path else nested_container(ts, j, end, member_path)
                    if container is None:
                        raise ValueError(f"{spec}: nested path must name one inline anonymous struct/union member at each step")
                    cj, cend = container
                    if not member_path and "{" not in ts[cj + 1:cend]:
                        a = max([cj] + [x for x in range(cj + 1, k) if ts[x] == ";"]) + 1
                        b = next((x for x in range(k, cend) if ts[x] == ";"), cend)
                        if member_declarators(ts[a:b]).count(old) != 1:
                            raise ValueError(f"{spec}: ambiguous/unsupported member declaration")
                    elif container_declarators(ts, cj, cend).get(k) != old:
                        raise ValueError(f"{spec}: member is not declared at depth 1 of "
                                         + ("the named nested aggregate" if member_path else "T (a nested member needs T.u.m)"))
                    found.append((path, [k]))
                    outer_defs[spec] = (path, j)
            else:
                for name, _, begin, end in function_definitions(text):
                    if name == scope:
                        found.append((path, list(range(begin, end + 1))))
        if len(found) != 1:
            raise ValueError(f"{spec}: expected exactly one {'aggregate' if member else 'function'} definition, found {len(found)}")
        path, indices = found[0]
        text = cfiles[path]
        ms = code_spans(text)
        ts = [m[0] for m in ms]
        if not member:
            if new in ts[indices[0]:indices[-1] + 1]:
                raise ValueError(f"{spec}: destination {new} already occurs in f")
            for macro_path, macro_text in cfiles.items():
                if (macro_uses(macro_text, old, body_only=True)
                        or macro_uses(macro_text, old, name_only=True)):
                    raise ValueError(f"{spec}: {old} occurs in a #define body or token paste in {macro_path}")
            if macro_uses(text, old):
                raise ValueError(f"{spec}: {old} occurs in a #define in {path}")
            reject_macro_arguments(text, old, macros, path, indices)
            for k in indices:
                if ts[k] == old and (ts[k - 1] in object_macros or ts[k + 1:k + 2] and ts[k + 1] in object_macros):
                    raise ValueError(f"{spec}: local token adjacent to object-like macro")
            region = ts[indices[0]:indices[-1] + 1]
            if old not in region or "#" in region:
                raise ValueError(f"{spec}: missing local or preprocessed function")
            if "typedef" in region or any(region[k] == "enum" and "{" in region[k + 1:k + 3]
                                          for k in range(len(region))):
                raise ValueError(f"{spec}: function-local enum/typedef bindings are unsupported")
            if any(region[k] in ("struct", "union") and "{" in region[k + 1:k + 3]
                   for k in range(len(region))):
                raise ValueError(f"{spec}: function-local aggregate definitions are unsupported")
            # Each candidate declaration is limited to a simple type/pointer
            # prefix at a parameter/statement boundary. Reject multiple bindings
            # (including nested shadowing) and uncertain uses in other namespaces.
            declarations = []
            if old in types or "__label__" in region:
                raise ValueError(f"{spec}: typedef/type name or GNU __label__ cannot be a local pair")
            values = set()
            for k in indices:
                token = ts[k]
                if token not in types:
                    continue
                end = k + 1
                while end < len(ts) and (ts[end] in types or ts[end] == "*"):
                    end += 1
                if any(t in ("struct", "union", "enum") for t in ts[k:end]):
                    continue
                if end < len(ts) and IDENT.fullmatch(ts[end]) and ts[end] not in C_KEYWORDS:
                    values.add(ts[end])
            body_open = next(k for k in indices if ts[k] == "{")
            try:
                starts, in_stmt_expr = statement_starts(ts, body_open, indices[-1], types)
            except ValueError as e:
                raise ValueError(f"{spec}: {e}") from None
            for k in indices:
                if ts[k] != old or local_exception(ts, k):
                    continue
                if ts[k - 1] == "&&" and (k < 2 or ts[k - 2] in ("return", "=", "(", ",", "{", ";", "?", ":", "*")):
                    raise ValueError(f"{spec}: GNU label address is not a local use")
                if ts[k + 1:k + 2] == [":"]:
                    # A ternary operand is an ordinary use; other non-start
                    # label spellings are too ambiguous for this small parser.
                    boundary = max([indices[0]] + [j for j in range(indices[0], k) if ts[j] in (";", "{", "}")])
                    prefix_tokens = ts[boundary:k]
                    if prefix_tokens.count("?") <= prefix_tokens.count(":"):
                        raise ValueError(f"{spec}: cannot prove statement-start label or use")
                # In the body the statement comes from the forward brace scan
                # (an initializer's or compound literal's braces belong to it);
                # in the parameter list it starts after `;`, `{` or `}`.
                if k > body_open:
                    if k in in_stmt_expr:
                        raise ValueError(f"{spec}: use inside a GNU statement expression is unsupported")
                    start = starts[k] - 1
                else:
                    start = k - 1
                    while start >= indices[0] and ts[start] not in (";", "{", "}"):
                        start -= 1
                statement = ts[start + 1:k]
                # Inside a type name (a `(` whose contents open with a type
                # word: a cast, sizeof or prototype list) a name declares,
                # unless an array bound nearer the name holds it.
                nesting, bound = 0, False
                for j in range(k - 1, start if k > body_open else k - 1, -1):
                    nesting += (ts[j] in (")", "]")) - (ts[j] in ("(", "["))
                    if nesting < 0:
                        nesting = 0
                        if ts[j] == "[":
                            bound = True
                        elif not bound and (ts[j + 1] in types or ts[j + 1] in typedef_names):
                            raise ValueError(f"{spec}: name inside a type name is unsupported")
                if statement and statement[0] in (",", ".", "->", "[", "]", "=", ")", "?", ":", "}"):
                    raise ValueError(f"{spec}: cannot place the statement start")
                # A body statement not led by a known type word must prove it is
                # no declaration: it opens with a statement keyword, `(` or a
                # unary operator, or a name used as a value (not followed by a
                # name, a keyword or, unless f declares the name after a type
                # word, `*`). Anything else fails. `T (l)` and `T *l` declare
                # when T is a type the census missed, so a name before `(` or
                # `*` must also be in no typedef declaration and not start with
                # `__` (an unlisted keyword), bar cc1 2.7.2's `__builtin_`
                # functions (it has no builtin type).
                if k > body_open and statement and statement[0] not in types:
                    first = statement[0]
                    second = statement[1] if len(statement) > 1 else None
                    if first in STATEMENT_KEYWORDS or first in ("(", "*", "++", "--", "&", "-", "+", "!", "~"):
                        pass
                    elif IDENT.fullmatch(first) and first not in C_KEYWORDS:
                        if (second is None or IDENT.fullmatch(second)
                                or second == "*" and first not in values
                                or second in ("(", "*") and (first in typedef_names or first.startswith("__")
                                                             and not first.startswith("__builtin_"))):
                            raise ValueError(f"{spec}: statement may be a declaration with an unrecognised type")
                    else:
                        raise ValueError(f"{spec}: cannot prove the statement `{first} ...` is no declaration")
                if any(t in ("__attribute", "__attribute__", "__extension__", "typeof", "__typeof", "__typeof__")
                       for t in statement):
                    raise ValueError(f"{spec}: complex GNU declaration/use is unsupported")
                before = k - 1
                parens = False
                while before >= indices[0] and ts[before] in ("(", "*", "const", "volatile"):
                    parens |= ts[before] == "("
                    before -= 1
                # A struct/union/enum tag is a type too (`struct S (*l)`).
                if parens and before >= 0 and (ts[before] in types or before >= 1
                                               and ts[before - 1] in ("struct", "union", "enum")):
                    raise ValueError(f"{spec}: parenthesized declarator is unsupported")
                # A declaration beginning with a known type may hide a second
                # binding behind parentheses or a comma. Calls and expressions
                # do not acquire declaration status merely by containing `(`.
                # A later declarator of the list is a binding only in the plain
                # `T a, *old` shape; inside an initializer it is a use.
                comma_type = None
                if statement and statement[0] in types:
                    # Before a top-level `=`, a `(` belongs to a declarator
                    # (`int (*g[sizeof(x = 1)])(l)` declares, and l is no use).
                    nesting, assigned = 0, False
                    for token in statement:
                        nesting += (token in ("(", "[", "{")) - (token in (")", "]", "}"))
                        assigned |= token == "=" and nesting == 0
                    if "(" in statement and not assigned:
                        raise ValueError(f"{spec}: parenthesized declarator is unsupported")
                    nesting, last_comma, first_comma = 0, None, None
                    for j, token in enumerate(statement):
                        nesting += (token in ("(", "[", "{")) - (token in (")", "]", "}"))
                        if token == "," and nesting == 0:
                            last_comma = j
                            first_comma = j if first_comma is None else first_comma
                    if last_comma is not None:
                        segment = statement[last_comma + 1:]
                        head = statement[:first_comma]
                        lead = next((j for j, token in enumerate(head) if token not in types and token != "*"), None)
                        if all(token == "*" for token in segment):
                            # The type is the first declarator's prefix: the type
                            # words before its name (and before any `*`, whose
                            # qualifiers are that declarator's own), which must be
                            # a plain name.
                            if (lead is None or not IDENT.fullmatch(head[lead]) or head[lead] in C_KEYWORDS
                                    or lead + 1 < len(head) and IDENT.fullmatch(head[lead + 1])
                                    or any(t in ("struct", "union", "enum") for t in head[:lead])):
                                raise ValueError(f"{spec}: comma declarator is unsupported")
                            star = head.index("*") if "*" in head[:lead] else lead
                            comma_type = head[:star] + segment
                        else:
                            # A use needs the declarator's own top-level `=`
                            # (`a = 0, (*g[sizeof(x = 1)])(l)` declares l).
                            nesting, assigned = 0, False
                            for token in segment:
                                nesting += (token in ("(", "[", "{")) - (token in (")", "]", "}"))
                                assigned |= token == "=" and nesting == 0
                            if not assigned:
                                raise ValueError(f"{spec}: comma declarator is unsupported")
                a = k - 1
                while a >= indices[0] and ts[a] not in ("(", ",", ";", "{", "}"):
                    a -= 1
                prefix = " ".join(ts[a + 1:k])
                if comma_type is not None:
                    prefix, declared = " ".join(comma_type), comma_type
                else:
                    declared = ts[a + 1:k]
                if (comma_type is None and re.fullmatch(r"[A-Za-z_]\w* (?:\* )+", prefix + " ")
                        and ts[a + 1] not in C_KEYWORDS and ts[a + 1] not in types and ts[a + 1] not in values):
                    raise ValueError(f"{spec}: ambiguous unknown type/value in pointer declaration or multiplication")
                if (declared and declared[0] in types
                        and re.fullmatch(r"(?:[A-Za-z_]\w* )(?:[A-Za-z_]\w* |\* )*", prefix + " ")):
                    if "extern" in declared:
                        raise ValueError(f"{spec}: block-scope extern is not a local binding")
                    nest = 0
                    for token in ts[indices[0]:k]:
                        nest += (token == "(") - (token == ")")
                    if nest != (1 if k < body_open else 0):
                        raise ValueError(f"{spec}: prototype/cast binding inside nested parentheses")
                    declarator = k + 1
                    while ts[declarator:declarator + 1] == [")"]:
                        declarator += 1
                    classes = _typedef_classes(tuple(cfiles.values()))
                    used = set(declared) & (classes.keys() | uncertain_types)
                    if (ts[declarator:declarator + 1] == ["("]
                            or any(name in uncertain_types or classes[name] == "unknown" for name in used)
                            or "*" not in declared and any(classes[name] == "function" for name in used)):
                        raise ValueError(f"{spec}: function declarator/type or uncertain typedef is not a local object binding")
                    declarations.append(k)
                elif ts[k - 1] in types or ts[k - 1] in ("__attribute", "__attribute__"):
                    raise ValueError(f"{spec}: cannot prove a simple local use/binding")
            if len(declarations) != 1:
                raise ValueError(f"{spec}: expected one simple binding, found {len(declarations)} (shadowing/unsupported declaration)")
            # A local declared in an inner block binds only to that block's end:
            # every use must lie inside it, so the function-wide substitution
            # renames exactly the uses of this one binding.
            body = body_open
            binding = declarations[0]
            declarator_end = binding + 1
            while declarator_end < len(ts) and ts[declarator_end] == "[":
                end = balanced(ts, declarator_end, "[", "]")
                if old in ts[declarator_end + 1:end]:
                    raise ValueError(f"{spec}: outer-name reference in the binding's own declarator")
                declarator_end = end + 1
            if any(ts[k] == old and not local_exception(ts, k) for k in range(indices[0], binding)):
                raise ValueError(f"{spec}: use before the local declaration is unsupported")
            # A later spelling in the parameter list may be a prototype-scope
            # name (`int (*cb)(l)`) rather than a use; refuse it.
            if any(ts[k] == old and not local_exception(ts, k) for k in range(binding + 1, body)):
                raise ValueError(f"{spec}: name repeated in the parameter list")
            if binding > body:
                # statement_starts proved the body balanced, so depth >= 1 here.
                depth = 0
                for k in range(body, binding):
                    depth += (ts[k] == "{") - (ts[k] == "}")
                if depth > 1:
                    block, open_depth = binding - 1, 0
                    while open_depth < 1:   # the innermost `{` still open at the binding
                        open_depth += (ts[block] == "{") - (ts[block] == "}")
                        block -= 1
                    block_end = balanced(ts, block + 1, "{", "}")
                    if any(ts[k] == old and not local_exception(ts, k)
                           for k in range(block_end, indices[-1] + 1)):
                        raise ValueError(f"{spec}: use after the nested local binding's block")
        for p, t in cfiles.items():
            spans = code_spans(t)
            toks = [m[0] for m in spans]
            aggregates = aggregate_indices(toks) if member else set()
            # Member accesses inside macro arguments are planned like any other
            # (optional) access edit; the cpp sentinel proof then requires an edited
            # one to expand after . or -> in every compiled TU that emits it.
            margs = macro_argument_indices(t, macros) if member else set()
            if member and p == path and any(k in margs for k in indices):
                raise ValueError(f"{spec}: member declaration inside a macro argument")
            edits = plan.setdefault(p, {})
            directives, conditions = pp_regions(t)
            for k, tok in enumerate(toks):
                if any(a <= spans[k].start() < b for a, b in directives):
                    continue
                replacement = None
                if tok == old and ((p == path and k in indices)
                                   or member and k and k not in aggregates
                                   and toks[k - 1] in (".", "->")):
                    if member or not local_exception(toks, k):
                        replacement = new
                if replacement is not None and replacement != tok:
                    if any(a <= spans[k].start() < b for a, b in conditions):
                        raise ValueError(f"{spec}: scoped substitution touches preprocessor-conditional code in {p}")
                    value = (replacement, bool(member and not (p == path and k in indices)))
                    if k in edits and edits[k] != value:
                        if edits[k][1] and value[1]:
                            previous = edits[k][0]
                            alternatives = previous if isinstance(previous, tuple) else (previous,)
                            value = (tuple(dict.fromkeys((*alternatives, replacement))), True)
                        else:
                            raise ValueError(f"{spec}: overlapping scoped pairs")
                    edits[k] = value
    # Shared destinations: one sharer per aggregate. Keyed on the resolved outer
    # definition, so a tag and its typedef are one T, and a nested member (T.u.m)
    # counts as T's: no two members of one T share n, at any depth.
    owners = {}
    for spec, new in pairs.items():
        if spec in outer_defs:
            other = owners.setdefault((new, outer_defs[spec]), spec)
            if other != spec:
                raise ValueError(f"{spec}: scoped destination {new} is shared by a pair that is not a distinct member pair "
                                 f"({other} names the same aggregate)")
    return plan


def scoped_applied(base, edits, read_new):
    """Required declaration/use edits must land even in otherwise untouched files."""
    for path, replacements in edits.items():
        required = {k: value[0] for k, value in replacements.items() if not value[1]}
        if not required:
            continue
        new = read_new(path)
        if new is None:
            raise ValueError(f"{path}: scoped pair definition is missing")
        ts = [m[0] for m in code_spans(new)]
        if len(ts) != len(code_spans(base[path])) or any(ts[k] != value for k, value in required.items()):
            raise ValueError(f"{path}: scoped pair was not applied to every required declaration/use")


def scoped_text(text: str, edits: dict, new: str) -> str:
    ms = code_spans(text)
    ns = code_spans(new)
    for k in sorted(edits, reverse=True):
        m = ms[k]
        replacement, optional = edits[k]
        if optional and len(ms) == len(ns) and ns[k][0] == m[0]:
            continue
        if isinstance(replacement, tuple):
            replacement = ns[k][0] if len(ms) == len(ns) and ns[k][0] in replacement else replacement[0]
        text = text[:m.start()] + replacement + text[m.end():]
    return text


def scoped_comments(want: str, new: str, pairs: dict[str, str]):
    """Comments may retain each old word or change it; code/literals are exact."""
    a, b = ctokens(want), ctokens(new)
    notes = False
    allowed = {}
    for spec, dest in pairs.items():
        allowed.setdefault(scoped_parts(spec)[2], set()).add(dest)
    if len(a) != len(b):
        return False, notes
    for x, y in zip(a, b):
        if x == y:
            continue
        if not x.startswith(("/*", "//")) or not y.startswith(("/*", "//")):
            return False, notes
        pattern = "".join("(?:" + "|".join(map(re.escape, [m[0], *allowed[m[0]]])) + ")"
                          if m.group("word") in allowed else re.escape(m[0])
                          for m in re.finditer(r"(?P<word>\b[A-Za-z_]\w*\b)|.", x, re.S))
        if not re.fullmatch(pattern, y):
            return False, notes
        notes = True
    return True, notes


def ctokens(text: str) -> list[str]:
    """engine.layer2.tokens plus what it drops that a wave must not change: each comment, as one
    token at its place (internal whitespace normalised), and `#define NAME(` adjacency (a
    function-like macro), which is a `NAME(` token."""
    out, in_pp, prev = [], False, []
    for m in layer2._TOKEN_RE.finditer(text):
        tok = m.group(0)
        if m.group("splice") is not None:
            continue
        if m.group("skip") is not None:
            if tok.startswith(("/*", "//")):
                out.append(" ".join(tok.split()))
            elif in_pp and "\n" in tok:
                out.append("\n")
                in_pp = False
            continue
        if tok == "#" and not in_pp and not text[text.rfind("\n", 0, m.start()) + 1:m.start()].strip():
            in_pp = True
        if prev[-2:] == ["#", "define"] and re.match(r"(?:\\\r?\n)*\(", text[m.end():]):
            tok += "("
        out.append(tok)
        prev = (prev + [tok])[-2:]
    return out


_FMT = None


def formatted(path: str, text: str) -> str | None:
    """`text` as tools/format.py would land it (format guard: staged C is always formatted), or
    None when the pinned clang-format is not installed (or another version is). A rename can
    lengthen a line, and the formatter then re-wraps or hoists a comment; the wave is checked
    against that output. The belt is format.py's own: an output whose C tokens differ from the
    input is no candidate."""
    global _FMT
    if _FMT is None:
        import format as cfmt  # tools/format.py
        exe = CODE / ".venv/bin/clang-format"
        version = (subprocess.run([str(exe), "--version"], capture_output=True, text=True).stdout
                   if exe.exists() else "")
        _FMT = (cfmt, str(exe)) if cfmt.CLANG_FORMAT_VERSION in version else False
    if not _FMT:
        return None
    cfmt, exe = _FMT
    try:
        status, out = cfmt._format_checked(exe, path, text)
    except Exception:
        return None
    return out if status == "ok" else None


def sub_comments(pat: re.Pattern, repl, text: str) -> tuple[str, int]:
    """Whole-word substitution inside C comments only (the wave's comment pass)."""
    out, n, pos = [], 0, 0
    for m in nw._C_TOKEN.finditer(text):
        tok = m.group(0)
        out.append(text[pos:m.start()])
        if tok.startswith(("//", "/*")):
            tok, k = pat.subn(repl, tok)
            n += k
        out.append(tok)
        pos = m.end()
    out.append(text[pos:])
    return "".join(out), n


def ledger_of(path: str) -> tuple[str, str] | None:
    """(ledger base, function dir) when `path` lies in a per-function ledger directory."""
    for base in sorted(LEDGER_DIRS, key=len, reverse=True):
        if path.startswith(base):
            rest = path[len(base):].split("/")
            if len(rest) >= 2:
                return base, rest[0]
    return None


REGISTRIES = ("named_syms.txt", "symbol_addrs.txt", "undefined_syms_auto.txt",
              "undefined_funcs_auto.txt")


def base_addr(base: str, name: str) -> str | None:
    """`name`'s address in the base tree: its own auto-name address, else the one address that
    ALL of these agree on: its asm/funcs/<name>.s glabel, `name = 0x...;` lines in the
    registries, and every census row naming it. None (unknown) when they disagree, when there is
    none, or when `name` is a #define in src/ or include/ (a macro names no address)."""
    if AUTO.match(name):
        return name[-8:].upper()
    index, macros = tree_address_index(base, {name})
    if name in macros:
        return None
    addrs = index.get(name, set())
    return next(iter(addrs)) if len(addrs) == 1 else None


# Owner ruling Q130: the MOVOVL overlay links its own copy of some verbatim Sony modules. A
# movovl/ line naming the new name is no freshness collision when it names the same routine.
LIBSCAN_SYMS, LIBSCAN_MATCHES = "docs/naming/libscan/libsyms.json", "docs/naming/libscan/matches.json"
MOVOVL_SCAN = "docs/naming/movovl_scan/"
MOVOVL_SYMADDRS = "movovl/symbol_addrs.movovl.txt"


def _verbatim_module(matches, lib, mod, start):
    """The one verbatim libscan placement of lib/mod, which must sit at `start` alone."""
    rows = [r for r in matches.get("results", []) if r.get("lib") == lib and r.get("mod") == mod
            and r.get("status") == "verbatim"]
    if len(rows) != 1 or [int(v, 16) for v in rows[0].get("vaddrs", [])] != [start]:
        return None
    return rows[0]


def movovl_same_routine(read, new: str, addr: str | None, old_src: list[str]):
    """Q130: the movovl/ lines (path -> line numbers) that name main's renamed routine itself.

    `read(path)` reads the base tree; `addr` is the old name's base address; `old_src` the src/
    files naming it. Admitted only when the old name lives in src/main/psxsdk/ alone, libscan
    places it verbatim as `new` (libsyms.json + matches.json), the overlay scan places the same
    lib/module verbatim once with the same word count, and its `new` sits at the same module
    offset (movovl_scan/symbols.txt). Then the lines are: that address's `new = 0x...;` row in
    symbol_addrs.movovl.txt, and in the module's own asm file the glabel/endlabel/nonmatching
    lines and instruction operands naming it. Any failure returns the reason (fail closed)."""
    if addr is None:
        return "the old name has no single base address"
    if not old_src or any(not p.startswith("src/main/psxsdk/") for p in old_src):
        return "the old name does not live in src/main/psxsdk/ alone"
    try:
        syms = json.loads(read(LIBSCAN_SYMS) or "{}")
        matches = json.loads(read(LIBSCAN_MATCHES) or "{}")
        o_matches = json.loads(read(MOVOVL_SCAN + "matches.json") or "{}")
        o_place = json.loads(read(MOVOVL_SCAN + "placements.json") or "{}")
        va = int(addr, 16)
        rows = [r for r in syms.get(f"0x{va:08X}", []) if r.get("name") == new]
        if len(rows) != 1:
            return f"libscan names no single {new} at 0x{va:08X}"
        lib, mod = rows[0]["lib"], rows[0]["mod"]
        start, end = int(rows[0]["mod_start"], 16), int(rows[0]["mod_end"], 16)
        main_mod = _verbatim_module(matches, lib, mod, start)
        if main_mod is None or main_mod["words"] * 4 != end - start or not start <= va < end:
            return f"{lib}/{mod} is not one verbatim libscan placement holding 0x{va:08X}"
        placed = [(int(a, 16), int(b, 16)) for a, b, pl, pm in o_place.get("modules", [])
                  if (pl, pm) == (lib, mod)]
        if len(placed) != 1:
            return f"movovl places {lib}/{mod} {len(placed)} times"
        o_start, o_end = placed[0]
        o_mod = _verbatim_module(o_matches, lib, mod, o_start)
        if o_mod is None or o_mod["words"] != main_mod["words"] or o_end - o_start != o_mod["words"] * 4:
            return f"movovl's {lib}/{mod} is not the same verbatim module"
    except (ValueError, KeyError, TypeError, AttributeError) as e:
        return f"libscan evidence unreadable ({e})"
    want = o_start + (va - start)
    at = []
    for k, line in enumerate((read(MOVOVL_SCAN + "symbols.txt") or "").splitlines(), 1):
        parts = line.split()
        if line.startswith("#") or len(parts) < 3:
            continue
        names = [t for t in parts[2:] if not t.startswith("(")]
        if parts[1] == f"{lib}/{mod}" and new in names:
            try:
                at.append(int(parts[0], 16))
            except ValueError:
                return f"{MOVOVL_SCAN}symbols.txt:{k}: unreadable address {parts[0]!r}"
    if at != [want]:
        return f"movovl's {lib}/{mod} does not name {new} at the same offset (0x{want:08X})"
    allowed: dict[str, set[int]] = {}
    sym_rows = [(i, m) for i, l in enumerate((read(MOVOVL_SYMADDRS) or "").splitlines(), 1)
                if (m := SYM_LINE.match(l)) and m.group(2) == new]
    if len(sym_rows) != 1 or int(sym_rows[0][1].group(4), 16) != want:
        return f"{MOVOVL_SYMADDRS} does not declare {new} once at 0x{want:08X}"
    allowed[MOVOVL_SYMADDRS] = {sym_rows[0][0]}
    asm = f"movovl/asm/{lib.lower()}_{mod.lower()}.s"
    lines = (read(asm) or "").splitlines()
    w = re.escape(new)
    label = re.compile(rf"^\s*(?:glabel|endlabel)\s+{w}\s*$|^\s*nonmatching\s+{w},\s*0x[0-9A-Fa-f]+\s*$", re.A)
    insn = re.compile(r"^\s*/\* [0-9A-F]+ ([0-9A-F]{8}) [0-9A-F]{8} \*/\s+[a-z][\w.]*\s+(.*)$", re.A)
    glabels = [i for i, l in enumerate(lines) if re.fullmatch(rf"\s*glabel\s+{w}\s*", l, re.A)]
    first = next((insn.match(l) for l in lines[glabels[0] + 1:] if insn.match(l)), None) if glabels else None
    if len(glabels) != 1 or first is None or int(first.group(1), 16) != want:
        return f"{asm} does not define {new} once at 0x{want:08X}"
    word = re.compile(rf"(?<!\w){w}(?!\w)", re.A)
    allowed[asm] = {i for i, l in enumerate(lines, 1) if word.search(l) and (
        label.match(l) or ((m := insn.match(l)) and "/*" not in m.group(2) and "#" not in m.group(2)))}
    return allowed


def movovl_hit_excused(text: str, path: str, news: list[str], q130: dict) -> str | None:
    """None when every line of movovl `path` naming a new name is a Q130 same-routine line.
    Words are ASCII, as git grep -w sees them; a hit file with no such line fails."""
    seen = False
    for i, line in enumerate(text.splitlines(), 1):
        for n in news:
            if re.search(rf"(?<![A-Za-z0-9_]){re.escape(n)}(?![A-Za-z0-9_])", line):
                seen = True
                allowed = q130.get(n)
                if not isinstance(allowed, dict):
                    return f"line {i}: {n}: {allowed or 'not the same routine'}"
                if i not in allowed.get(path, ()):
                    return f"line {i}: {n}: not a definition or reference of the same routine"
    return None if seen else "no line names a new name as git grep -w does"


def _is_macro(base: str, name: str) -> bool:
    """True when src/ or include/ #defines `name` in the base tree."""
    return bool(git("grep", "-l", "-E", r"^\s*#\s*define\s+" + re.escape(name) + r"\b",
                    base, "--", "src", "include", check=False))


SYM_LINE = re.compile(r"^(\s*)([A-Za-z_]\w*)(\s*=\s*)(0x[0-9A-Fa-f]{8})(\s*;)(.*)$")
DATE = r"\d{4}-\d{2}-\d{2}"


def registry_outputs(line: str, rel: str, pairs: dict[str, str], deletable, dup_before
                     ) -> list[list] | None:
    """What the wave tools may turn one old registry line into: a list of alternatives, each a
    list of 0 or 1 output lines (a str, or a compiled regex for a dated line). None = the line
    must stay byte-identical."""
    m = SYM_LINE.match(line)
    if not m:
        return None
    indent, name, eq, addr, semi, tail = m.groups()
    a8 = addr[2:].upper()
    open_, close = ("// ", "") if rel == "symbol_addrs.txt" else ("/* ", " */")
    if name not in pairs:
        # A destination's own registry alias can also duplicate its object definition.
        if name in pairs.values() and deletable(name, a8):
            # Reuse the exact retirement forms for a destination's own duplicate alias.
            return [[line], *registry_outputs(line, rel, {name: name}, deletable, dup_before)]
        return None
    new = pairs[name]
    rewritten = f"{indent}{new}{eq}{addr}{semi}{tail}"
    outs: list[list] = [[rewritten]]                       # naming_wave in place / data_wave
    if "data-wave" not in tail:                             # data_wave rewrite with provenance
        outs.append([re.compile(re.escape(rewritten + "  " + open_) + f"data-wave {DATE}: was "
                                + re.escape(name + close) + "$")])
    if not tail.strip() and dup_before(new, a8) and deletable(new, a8):
        outs.append([])                                     # data_wave's duplicate drop
    if deletable(new, a8):
        note = tail.strip()
        if not note:
            outs.append([])                                 # naming_wave deletes a bare line
        note = note.strip("/*").strip("*/").strip().removeprefix("//").strip()
        from data_wave import note_kind
        kind = note_kind(addr, new)
        outs.append([f"{indent}{open_}{kind} {addr}: retired name '{name}' \u2014 "
                     f"preserved note: {note}{close}"])
        outs.append([re.compile(re.escape(open_) + f"data-wave {DATE}: "
                                + re.escape(f"{name} = {addr} retired; {new} is now object-defined"
                                            + close) + "$")])
        if note and name == new:
            outs.append([re.compile(re.escape(indent + open_ + "duplicate of ") +
                r"(?P<home>[\w./-]+):(?P<line>[1-9][0-9]*)" +
                re.escape(" — preserved note: " + note + close) + "$")])
    return outs


def new_tree_paths(rev: str | None) -> list[str]:
    """Definition surfaces in the tree being checked, including working-tree additions."""
    if rev is not None:
        return git("ls-tree", "-r", "--name-only", rev, "--", "src", "asm").splitlines()
    return [p.relative_to(ROOT).as_posix() for folder in ("src", "asm")
            for p in sorted((ROOT / folder).rglob("*")) if p.is_file()]


_ADDRESS_INDEXES = {}
_DEFINITION_INDEXES = {}
_FILE_TEXTS = {}
_READ_TEXTS = {}
_INDEX_QUERIES = {}


def load_tree_texts(rev, names=()):
    """Read name/address candidate files, caching every file once per tree and run."""
    cache = _FILE_TEXTS.setdefault((str(ROOT), rev), {})
    paths = list(REGISTRIES) + list(METADATA) + ["build/bb2.map"]
    if names:
        terms = set(names) | {n[-8:] for n in names if AUTO.match(n)}
        args = ["grep", "-l", "-w", "-E"]
        if rev is None:
            args.append("--untracked")
        args.append("|".join(re.escape(n) for n in sorted(terms)))
        if rev is not None:
            args.append(rev)
        hits = git(*args, "--", "src", "include", "asm", check=False).splitlines()
        paths += [p.split(":", 1)[1] if rev is not None else p for p in hits]
    paths = [p for p in dict.fromkeys(paths) if p not in cache]
    read_cache = _READ_TEXTS.get((str(ROOT), rev), {})
    already_read = {p: read_cache[p] for p in paths if p in read_cache}
    cache.update(already_read)
    paths = [p for p in paths if p not in already_read]
    texts = {p: t for p, t in already_read.items() if t is not None}
    if rev is None:
        texts.update({p: nw.read(ROOT / p) for p in paths if (ROOT / p).is_file()})
        cache.update({p: texts.get(p) for p in paths})
        return texts
    map_path = "build/bb2.map"
    if map_path in paths:
        paths.remove(map_path)
        cache[map_path] = nw.read(ROOT / map_path) if (ROOT / map_path).is_file() else None
        if cache[map_path] is not None:
            texts[map_path] = cache[map_path]
    if not paths:
        return texts
    result = subprocess.run(["git", "cat-file", "--batch"], cwd=ROOT,
                            input="".join(f"{rev}:{p}\n" for p in paths).encode(),
                            capture_output=True, check=True)
    pos = 0
    for path in paths:
        end = result.stdout.index(b"\n", pos)
        header = result.stdout[pos:end].split()
        pos = end + 1
        if header[-1] == b"missing":
            cache[path] = None
            continue
        size = int(header[-1])
        texts[path] = result.stdout[pos:pos + size].decode("utf-8", "surrogateescape")
        pos += size + 1
        cache[path] = texts[path]
    return texts


def address_index(texts, census=False):
    """Parse every definition surface once, then answer symbol queries by dictionary lookup."""
    from collections import defaultdict
    import data_wave as dw
    index, macros, c_objects = defaultdict(set), set(), set()
    for reg in (REGISTRIES if census else REGISTRIES[:3]):
        for line in texts.get(reg, "").splitlines():
            m = SYM_LINE.match(line)
            if m:
                index[m.group(2)].add(m.group(4)[2:].upper())
    for path, text in texts.items():
        if path.startswith("asm/") and path.endswith(".s"):
            for addr, symbol in dw.asm_defs(text):
                index[symbol].add(addr[2:].upper())
        elif path.startswith("src/") and path.endswith(".c"):
            c_objects |= dw.c_data_names(text)
        if path.startswith(("src/", "include/")):
            macros |= set(re.findall(r"(?m)^\s*#\s*define\s+([A-Za-z_]\w*)", text))
    for addr, name in dw.map_defs(texts.get("build/bb2.map", "")):
        if name in c_objects:
            index[name].add(addr[2:].upper())
    if census:
        add_census_addresses(index, texts)
    return index, macros


def add_census_addresses(index, texts):
    import csv
    import io
    for row in csv.DictReader(io.StringIO(texts.get(METADATA[0], ""))):
        names = {row.get("current_name"), row.get("glabel"),
                 *(x.strip() for x in (row.get("aliases") or "").split(";"))}
        for name in names - {None, ""}:
            index.setdefault(name, set()).add((row.get("address") or "").upper().replace("0X", "").zfill(8))


def tree_address_index(rev, names=()):
    key = (str(ROOT), rev)
    queried = _INDEX_QUERIES.setdefault(key, set())
    pending = set(names) - queried
    if key not in _ADDRESS_INDEXES or pending:
        first = key not in _ADDRESS_INDEXES
        texts = load_tree_texts(rev, pending)
        # C candidates may arrive after the map; reuse the cached map without rereading it.
        cached_map = _FILE_TEXTS.get(key, {}).get("build/bb2.map")
        if cached_map:
            texts = {**texts, "build/bb2.map": cached_map}
        delta, macros = address_index(texts)
        definitions = _DEFINITION_INDEXES.setdefault(key, {}) if not first else {}
        for n, a in delta.items():
            definitions.setdefault(n, set()).update(a)
        _DEFINITION_INDEXES[key] = definitions
        index, old_macros = _ADDRESS_INDEXES.get(key, ({}, set()))
        for n, a in delta.items():
            index.setdefault(n, set()).update(a)
        if rev is not None:
            # Function-only registry and census are address evidence, not data definitions.
            for line in texts.get("undefined_funcs_auto.txt", "").splitlines():
                m = SYM_LINE.match(line)
                if m:
                    index.setdefault(m.group(2), set()).add(m.group(4)[2:].upper())
            add_census_addresses(index, texts)
        _ADDRESS_INDEXES[key] = index, old_macros | macros
        queried.update(pending)
    return _ADDRESS_INDEXES[key]


def read_index_file(rev, path):
    key = (str(ROOT), rev)
    indexed = _FILE_TEXTS.get(key, {})
    if path in indexed:
        return indexed[path]
    cache = _READ_TEXTS.setdefault(key, {})
    if path not in cache:
        cache[path] = show(rev, path)
    return cache[path]


def registry_target_addrs(name: str, read_new, paths: list[str]) -> set[str]:
    texts = {p: read_new(p) or "" for p in dict.fromkeys([*paths, *REGISTRIES, "build/bb2.map"])}
    addrs = set(address_index(texts)[0].get(name, set()))
    if AUTO.match(name):
        addrs.add(name[-8:].upper())
    return addrs


def registry_ok(old: str, new: str, rel: str, pairs: dict[str, str], read_new=None,
                paths_new=None, targets_index=None, home_tracked=None) -> str | None:
    """A registry (`name = 0xADDR;  /* ... */` lines) changed by the wave tools. Every old line
    must become exactly one output the tools produce for it (registry_outputs):
      - the in-place rewrite (pairs substituted in the name);
      - data_wave's rewrite plus `  /* data-wave <date>: was <that line's old name> */`, only
        when the old tail had no data-wave note;
      - data_wave's duplicate drop: nothing, when an earlier output line already defines
        (new, addr);
      - when the new name stays defined at the same address (auto name, any registry,
        assembly label, or a C data definition corroborated by the map): deletion, its
        preserved-note line for a line with one (exactly `<RESET if the new name is the
        address's auto name, else RENAME> <addr>: retired name '<old>' -- preserved note:
        <the old comment, stripped as naming_wave strips it>`), or data_wave's
        `<old> = <addr> retired; <new> is now object-defined` line;
      - naming_wave's deletion of a destination's own reverse alias `func_X = 0xX;`.
    Conflicting or unknown target addresses reject retirements. Returns None when OK,
    else the first line no tool produces."""
    got = new.split("\n")
    remaining = {(mm.group(2), mm.group(4)[2:].upper()) for mm in map(SYM_LINE.match, got) if mm}
    emitted: set[tuple[str, str]] = set()
    target_cache = {}
    if targets_index is None and read_new:
        paths = paths_new if paths_new is not None else new_tree_paths(None)
        texts = {p: new if p == rel else read_new(p) or ""
                 for p in dict.fromkeys([*paths, *REGISTRIES, "build/bb2.map"])}
        targets_index = address_index(texts)[0]

    def deletable(name: str, addr: str) -> bool:
        if name not in target_cache:
            addrs = {a for n, a in remaining if n == name}
            if targets_index is not None:
                addrs |= targets_index.get(name, set())
            if AUTO.match(name):
                addrs.add(name[-8:].upper())
            target_cache[name] = addrs
        return target_cache[name] == {addr}

    def fits(want, g: str) -> bool:
        if not isinstance(want, re.Pattern):
            return want == g
        m = want.match(g)
        if not m:
            return False
        if "home" not in m.groupdict():
            return True
        path, lineno = m.group("home"), int(m.group("line"))
        import posixpath
        path = posixpath.normpath(path)
        if path.startswith("../") or not (path in REGISTRIES or
                path.startswith(("src/", "include/", "asm/"))):
            return False
        if home_tracked and not home_tracked(path):
            return False
        text = new if path == rel else read_new(path) if read_new else None
        if not text:
            return False
        lines = text.splitlines()
        if lineno > len(lines):
            return False
        name = SYM_LINE.match(line).group(2)
        addr = SYM_LINE.match(line).group(4)[2:].upper()
        mm = SYM_LINE.match(lines[lineno - 1])
        if path in REGISTRIES:
            return bool(mm and mm.group(2) == name and mm.group(4)[2:].upper() == addr)
        import data_wave as dw
        if path.startswith("asm/"):
            label = dw.LABEL.match(lines[lineno - 1])
            return bool(label and label.group(1) == name and
                        ("0x" + addr.lower(), name) in set(dw.asm_defs(text)))
        prefix = layer2.tokens("\n".join(lines[:lineno - 1]))
        code = re.sub(C_COMMENT, "", "\n".join(lines[lineno - 1:]))
        return bool(path.startswith("src/") and dw.c_data_defined(text, name) and
                    re.search(r"\b" + re.escape(name) + r"\b", re.sub(C_COMMENT, "", lines[lineno - 1])) and
                    prefix.count("{") == prefix.count("}") and re.match(
                    r"\s*(?!extern\b|static\b|typedef\b)[A-Za-z_][\w \t*]*\b" +
                    re.escape(name) + r"\b\s*(?:\[[^]]*\]\s*)*=", code))

    k = 0
    for line in old.split("\n"):
        outs = registry_outputs(line, rel, pairs, deletable, lambda n, a8: (n, a8) in emitted)
        here = got[k] if k < len(got) else None
        if outs is None:
            if here != line:
                return here if here is not None else f"missing: {line}"
        elif here is not None and any(alt and fits(alt[0], here) for alt in outs):
            pass
        elif [] in outs:
            continue
        else:
            return here if here is not None else f"missing: {line}"
        # data_wave's `seen`: only its own rewrites of pair lines make a later one a duplicate
        mo, mm = SYM_LINE.match(line), SYM_LINE.match(here)
        if mo and mo.group(2) in pairs and mm:
            emitted.add((mm.group(2), mm.group(4)[2:].upper()))
        k += 1
    if k != len(got):
        return got[k]
    return None


def changed_files(base: str, new: str | None) -> list[tuple[str, str]]:
    args = ["diff", "--no-renames", "--name-status", "-z", base] + ([new] if new else [])
    fields = git(*args).split("\0")
    if fields[-1] != "" or len(fields) % 2 != 1:
        raise ValueError("invalid NUL-delimited Git diff response")
    if any(not st or not path for st, path in zip(fields[:-1:2], fields[1:-1:2])):
        raise ValueError("empty Git diff status/path")
    rows = [(st[0], path) for st, path in zip(fields[:-1:2], fields[1:-1:2])]
    if new is None:
        rows += [("A", p) for p in
                 git("ls-files", "--others", "--exclude-standard", "-z").split("\0") if p]
    return rows


def main() -> int:
    _ADDRESS_INDEXES.clear()  # working-tree evidence must be fresh for each invocation
    _DEFINITION_INDEXES.clear()
    _FILE_TEXTS.clear()
    _READ_TEXTS.clear()
    _INDEX_QUERIES.clear()
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--pairs", default="")
    ap.add_argument("--pairs-file", default="")
    ap.add_argument("--from-manifests", default="",
                    help="comma-separated naming_wave / data_wave --manifest JSON files: the pairs "
                         "are their ops' old names -> new name (the rule's source of the pairs)")
    ap.add_argument("--base", default="HEAD")
    ap.add_argument("--new", default=None, help="revision to check (default: working tree)")
    ap.add_argument("--sub-comments", action="store_true",
                    help="the wave's comment pass: substitute the pairs in the comments of every "
                         "src/ and include/ C file in the working tree, then exit")
    a = ap.parse_args()
    spec = a.pairs + "\n" + (Path(a.pairs_file).read_text(encoding="utf-8") if a.pairs_file else "")
    alias_pairs = set()
    for mf in filter(None, a.from_manifests.split(",")):
        for op in json.loads(Path(mf).read_text(encoding="utf-8")).get("ops", []):
            new_name = op.get("new_name") or op.get("new")
            for o in op.get("old_names") or op.get("olds") or []:
                if o != new_name:
                    spec += f"\n{o}={new_name}"
                    if op.get("kind") == "retire-alias":
                        alias_pairs.add((o, new_name))
    pairs = parse_pairs(spec)
    scoped = {o: n for o, n in pairs.items() if "." in o or ":" in o}
    pairs = {o: n for o, n in pairs.items() if o not in scoped}
    pat = re.compile(r"\b(" + ("|".join(map(re.escape, sorted(pairs, key=len, reverse=True)))
                              or r"(?!)") + r")\b")
    repl = lambda m: pairs[m.group(1)]  # noqa: E731
    map_path = lambda p: pat.sub(repl, p)  # noqa: E731

    if a.sub_comments:
        if scoped:
            ap.error("--sub-comments accepts global pairs only")
        hits = git("grep", "-l", "-w", "-E", "|".join(map(re.escape, pairs)), "--", "src", "include",
                   check=False).splitlines()
        for path in hits:
            if path.endswith((".c", ".h")):
                text, n = sub_comments(pat, repl, nw.read(ROOT / path))
                if n:
                    nw.write_lf(ROOT / path, text)
                    print(f"{path}: {n} comment substitution(s)")
        return 0

    tree_address_index(a.base, set(pairs) | {kept for _, kept in alias_pairs})
    fails: list[str] = []
    for old, kept in alias_pairs:
        if AUTO.match(old):
            fails.append(f"{old} -> {kept}: retire-alias cannot retire an auto/function name")
            continue
        addr = base_addr(a.base, old)
        base_addr(a.base, kept)  # populate the cached definition index
        defined = _DEFINITION_INDEXES[(str(ROOT), a.base)].get(kept, set())
        macros = tree_address_index(a.base, {kept})[1]
        if not addr or defined != {addr} or kept in macros:
            fails.append(f"{old} -> {kept}: retire-alias requires an existing same-address survivor")
        import data_wave as dw
        texts = _FILE_TEXTS.get((str(ROOT), a.base), {})
        if any((path.startswith("asm/") and any(m.group(1) == old for m in dw.LABEL.finditer(text or ""))) or
               (path.startswith("src/") and re.search(r"(?m)^[\w \t*]+\b" + re.escape(old) +
                 r"\s*\([^;{}]*\)\s*\{", text or "")) for path, text in texts.items()):
            fails.append(f"{old} -> {kept}: retire-alias cannot retire a function or assembly definition label")
    notes: list[str] = []
    scope_edits = {}
    rows = changed_files(a.base, a.new)
    if not a.new and not scoped:
        try:
            verify_working_storage(rows, None, a.base)
        except (ValueError, OSError, subprocess.CalledProcessError) as e:
            fails.append(str(e))
    if scoped:
        try:
            base_files = read_base_tree(a.base)
            scope_edits = scoped_plan(base_files, scoped, pairs)
            scoped_applied(base_files, scope_edits, lambda p: show(a.new, map_path(p)))
            new_files = read_base_tree(a.new) if a.new else read_working_tree()
            if not a.new:
                verify_working_changes(new_files, base_files, rows)
                verify_working_storage(rows, new_files, a.base)
            plain_tree(base_files)
            plain_tree(new_files)
            verify_compiled_edits(base_files, new_files, scope_edits, map_path, scoped, pairs)
        except (ValueError, OSError, subprocess.CalledProcessError) as e:
            fails.append(str(e))
        if any("." in o for o in scoped):
            notes.append("member pairs require a separate oracle SHA1 build to prove access resolution")
    groups: dict[str, list[str]] = {}
    for o, n in pairs.items():
        groups.setdefault(n, []).append(o)
    for n, olds in groups.items():
        if len(olds) > 1:
            addrs = {o: base_addr(a.base, o) for o in olds}
            if None in addrs.values() or len({x.upper() for x in addrs.values()}) != 1:
                fails.append(f"{', '.join(olds)} -> {n}: names of different (or unknown) "
                             f"addresses may not share a new name ({addrs})")
    deleted = {p for st, p in rows if st == "D"}
    added = {p for st, p in rows if st == "A"}
    renames = {}  # new path -> old path
    sources: dict[str, list[str]] = {}
    for d in sorted(deleted):
        if map_path(d) != d and map_path(d) in added:
            sources.setdefault(map_path(d), []).append(d)
    for target, olds in sources.items():
        if len(olds) > 1:  # several aliases' files would land on one path: nothing to compare
            fails.append(f"{target}: paths of a merged name collide ({', '.join(olds)})")
        renames[target] = olds[0]
    work: list[tuple[str, str]] = []  # (old path, new path) to compare
    for st, p in rows:
        if st == "D":
            if map_path(p) not in renames:
                fails.append(f"{p}: deleted, not a rename by the pairs")
        elif st == "A":
            if p in renames:
                work.append((renames[p], p))
            elif p.startswith(METADATA_DIRS):
                notes.append(f"{p}: wave metadata (added)")
            else:
                fails.append(f"{p}: added, not a rename by the pairs")
        else:
            work.append((p, p))

    src_scan: set[str] = set()
    moved_stems: set[str] = set()
    census_changed = False
    for old_path, new_path in work:
        if new_path == SELF or new_path.startswith(DENY_DIRS) or old_path.startswith(DENY_DIRS):
            fails.append(f"{new_path}: rules, agents, the MOVOVL overlay and this tool are never part "
                         f"of a wave")
            continue
        old = read_index_file(a.base, old_path)
        new = read_index_file(a.new, new_path)
        if old is None or new is None:
            fails.append(f"{new_path}: unreadable")
            continue
        if old_path != new_path:
            notes.append(f"{old_path} -> {new_path}: renamed")
            if new_path.endswith(".s"):
                moved_stems.add(Path(new_path).stem)
        if new_path in METADATA:
            census_changed = True
            continue
        if new_path.startswith(METADATA_DIRS):
            notes.append(f"{new_path}: wave metadata")
            continue
        led = ledger_of(new_path)
        if new_path.startswith("memory/") and (not led or old_path == new_path):
            fails.append(f"{new_path}: memory/ changes only as a ledger the pairs move")
            continue
        if led and Path(new_path).name == nw.LAYER2_RECORD:
            old_func = ledger_of(old_path)[1]
            new_func = led[1]
            expect = old if old_func == new_func else \
                nw.retarget_layer2_record(old, old_func, new_func, old_path)[0]
            if new != expect:
                fails.append(f"{new_path}: layer-2 record differs from the retarget of {old_path}")
            continue
        if led and old_path != new_path:
            if new != old:
                fails.append(f"{new_path}: a moved ledger file must move unchanged")
            continue
        if new_path.endswith((".c", ".h")) and new_path.startswith(("src/", "include/")):
            try:
                if scoped:
                    function_definitions(old)
                    function_definitions(new)
                if any("." in o for o in scoped) and not aggregate_unchanged(
                        old, new, scope_edits.get(old_path, {}), pairs):
                    fails.append(f"{new_path}: aggregate definition changed beyond explicit member declarations")
            except ValueError as e:
                fails.append(f"{new_path}: {e}")
                continue
            scoped_old = scoped_text(old, scope_edits.get(old_path, {}), new)
            want = sub_comments(pat, repl, nw.sub_c(pat, repl, scoped_old)[0])[0]
            t_want, t_new = ctokens(want), ctokens(new)
            scope_ok, comment_note = scoped_comments(want, new, scoped)
            if scope_ok:
                t_want = t_new
            if comment_note and scope_ok:
                notes.append(f"{new_path}: scoped comment substitutions (read them)")
            if t_want != t_new:
                fw = formatted(new_path, want)
                if fw is not None and scoped_comments(fw, new, scoped)[0]:
                    t_want = t_new  # the wave's text, as the formatter lands it
                elif fw is not None:
                    t_want = ctokens(fw)  # report against the text the format guard would land
            if t_want == t_new and layer2.tokens(old) == layer2.tokens(new):
                notes.append(f"{new_path}: comment substitutions only (read them)")
            if t_want != t_new:
                i = next((k for k, (x, y) in enumerate(zip(t_want, t_new)) if x != y),
                         min(len(t_want), len(t_new)))
                fails.append(f"{new_path}: token change at token {i}: "
                             f"{' '.join(t_want[max(0, i - 4):i + 4])!r} -> "
                             f"{' '.join(t_new[max(0, i - 4):i + 4])!r}")
            if new_path.startswith("src/"):
                src_scan.add(new_path)
            continue
        if new_path in REGISTRIES:
            bad = registry_ok(old, new, new_path, pairs,
                              read_new=lambda p: read_index_file(a.new, p),
                              targets_index=tree_address_index(a.new, set(pairs.values()))[0],
                              home_tracked=lambda p: p in (git("ls-tree", "-r", "--name-only", a.new,
                                  "--", p).splitlines() if a.new else
                                  git("ls-files", "--", p).splitlines()))
            if bad is not None:
                fails.append(f"{new_path}: a line the wave tools do not produce: {bad[:120]}")
            continue
        candidates = {pat.sub(repl, old), nw.sub_c(pat, repl, old)[0], nw.sub_hash(pat, repl, old)[0]}
        if new_path.endswith(".py"):
            candidates.add(nw.sub_py(pat, repl, old)[0])
        if new not in candidates:
            fails.append(f"{new_path}: differs beyond a substitution of the pairs")
        if new_path.endswith(".s"):
            moved_stems.add(Path(new_path).stem)

    if census_changed:
        if a.new is not None:
            fails.append(f"{METADATA[0]}: changed; --new mode cannot re-derive it (check in the tree)")
        else:
            with tempfile.TemporaryDirectory() as td:
                env = dict(os.environ, BUILD_CENSUS_OUTDIR=td)
                subprocess.run([sys.executable, str(ROOT / "docs/naming/build_census.py")], cwd=ROOT,
                               env=env, capture_output=True, check=False)
                fresh = Path(td) / "function-names.csv"
                if not fresh.is_file() or nw.read(fresh) != show(None, METADATA[0]):
                    fails.append(f"{METADATA[0]}: differs from a fresh build_census.py run")
                else:
                    notes.append(f"{METADATA[0]}: equals a fresh build_census.py run")

    # tree-wide: new names must be new; old names must be gone from build files
    news = []
    for o, n in pairs.items():
        if AUTO.match(n):
            addr = base_addr(a.base, o)
            if addr is None or addr.lower() != n[-8:].lower():
                fails.append(f"{o} -> {n}: an auto name must carry {o}'s own address "
                             f"(found {addr or 'none'})")
        elif (o, n) not in alias_pairs:
            news.append(n)
    if news:
        hits = git("grep", "-l", "-w", "-E", "|".join(news), a.base, "--", "src", "include", "asm",
                   "tools", "engine", "movovl", ".claude", "*.ld", "*.txt", "Makefile", ":!docs", ":!memory",
                   check=False).splitlines()
        q130 = {}
        if any(h.split(":", 1)[1].startswith("movovl/") for h in hits):
            read_base = lambda p: show(a.base, p)  # noqa: E731
            for o, n in pairs.items():
                if n not in news:
                    continue
                old_src = [h.split(":", 1)[1] for h in git("grep", "-l", "-w", "-F", o, a.base, "--", "src",
                                                            check=False).splitlines()]
                q130[n] = ("several old names share it" if n in q130 else
                           movovl_same_routine(read_base, n, base_addr(a.base, o), old_src))
        for h in hits:
            path = h.split(":", 1)[1]
            why = (movovl_hit_excused(show(a.base, path) or "", path, news, q130)
                   if path.startswith("movovl/") else "")
            if why is None:
                notes.append(f"{path}: names the same verbatim Sony routine (Q130)")
            else:
                fails.append(f"new name already in the base tree: {path}" + (f" ({why})" if why else ""))
    grep_rev = [a.new] if a.new else []
    grep_options = [] if a.new else ["--untracked"]
    for h in git("grep", "-l", "-w", "-E", *grep_options, "|".join(map(re.escape, pairs)) or r"a^", *grep_rev,
                 check=False).splitlines():
        path = h.split(":", 1)[1] if a.new else h
        build = path.startswith(("src/", "include/", "asm/")) or path.endswith(".ld") \
            or path == "Makefile" or ("/" not in path and path.endswith(".txt"))
        if not build:
            continue
        text = show(a.new, path) or ""
        n = nw.sub_c(pat, repl, text)[1]
        if path.endswith((".c", ".h")) and path.startswith(("src/", "include/")):
            n += sub_comments(pat, repl, text)[1]  # C comments take the pairs too
        if path.endswith(".txt"):  # registries comment with /* */, gate lists with #
            n = min(n, nw.sub_hash(pat, repl, text)[1])
        if n:
            fails.append(f"{path}: an old name survives ({n})")

    # moved layer-2 keys
    if moved_stems:
        for h in git("grep", "-l", "-w", "-E", *grep_options, "|".join(moved_stems), *grep_rev, "--", "src",
                     check=False).splitlines():
            src_scan.add(h.split(":", 1)[1] if a.new else h)
    rev_pairs: dict[str, list[str]] = {}
    for o, n in pairs.items():
        rev_pairs.setdefault(n, []).append(o)
    moved = []
    for path in sorted(src_scan):
        new = show(a.new, path) or ""
        old = show(a.base, path) or ""
        try:
            definitions = function_names(new) if scoped else set()
        except ValueError as e:
            fails.append(f"{path}: {e}")
            continue
        census = (definitions | original_census(new) if scoped
                  else set(layer2.inlineasm.definition_names(new)))
        for name in sorted({m.group(1) for m in layer2.inlineasm._INCLUDE_ASM_MACRO_RE.finditer(new)} |
                           census):
            k_new = layer2.body_key(new, name, lambda p: show(a.new, p))
            k_old, old_name = None, name
            for cand in rev_pairs.get(name, []) + [name]:
                k_old = layer2.body_key(old, cand, lambda p: show(a.base, p))
                if k_old:
                    old_name = cand
                    break
            if (scoped and name in census and (not k_old or not k_new)
                    and function_tokens(old, old_name) != function_tokens(new, name)):
                fails.append(f"{path}: {name}: layer-2 cannot key this function; scoped proof fails closed")
            if k_new and k_old and k_new != k_old:
                moved.append(f"{path}: {name}" + (f" (was {old_name})" if old_name != name else "")
                             + f"  {k_old[1]} -> {k_new[1]}")

    print(f"pairs: {len(pairs)}  changed paths: {len(rows)}")
    for o, n in sorted(pairs.items()):
        print(f"pair {o} -> {n}")
    for o, n in sorted(scoped.items()):
        print(f"scoped pair {o} -> {n}")
    for f in fails:
        print("FAIL", f)
    for n in notes:
        print("note", n)
    print(f"moved layer-2 keys: {len(moved)}")
    for m in moved:
        print("  ", m)
    print("OK: every change is explained by the pairs" if not fails
          else f"FAIL: {len(fails)} change(s) the pairs do not explain")
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())

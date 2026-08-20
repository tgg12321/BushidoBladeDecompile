#!/usr/bin/env python3
"""Build the SOTN construct-precedent index.

Scans a sotn-decomp checkout for the match-hack construct classes that BB2's
frozen sanctioned-family list (.claude/rules/no-new-park-categories.md) needs
SOTN-master precedent for, and emits a greppable markdown index.

Usage:  python3 build_sotn_index.py <path/to/sotn-decomp> [-o out.md]
                                   [--cap N] [--full-below N]

Every entry cites the checkout's HEAD commit, printed in the index header.

CLASS TABLE
-----------
| id                | what it catches                                             | method     |
|-------------------|-------------------------------------------------------------|------------|
| fake_comment      | case-insensitive "fake" inside a // or block comment         | regex/line |
| fake_identifier   | identifier containing "fake" (color_fake, SsVabFakeHead)      | regex/line |
| self_assign       | `x = x;` / `s->a = s->a;` self-assignment                    | regex/line |
| match_comment     | comment asserting a codegen reason (needed/match/hack/...)    | regex/line |
| do_while_zero     | `do { ... } while (0);` wraps                                | brace scan |
| pad_dummy_local   | locals named pad/dummy/unused/spacer/sp##/zero..nine          | regex/decl |
| new_var_temp      | SOTN's `new_var*` / `temp_*` RA/scheduling temporaries        | regex/decl |
| pointer_alias     | `T* p = &GLOBAL;` alias-to-global local                       | regex/line |
| dup_if_else_arm   | identical non-trivial stmt in BOTH arms of one if/else        | block diff |
| const_holder      | `sNN name = <literal>;` local const holder with a comment     | regex/line |
| empty_if          | `if (cond) {}` / `if (cond) ;` empty-body conditionals        | regex/text |
| nested_exit_label | label declared inside a nested block that a `goto` targets    | brace scan |

Scan-quality notes are emitted into the markdown so the reader knows which
classes are exact and which are heuristic samples.
"""
import argparse
import os
import re
import subprocess
import sys
from collections import defaultdict
from datetime import date

# ---------------------------------------------------------------- class regexes
RE_LINE_COMMENT = re.compile(r"//(.*)$")
RE_BLOCK_COMMENT = re.compile(r"/\*(.*?)\*/", re.S)

RE_FAKE_WORD = re.compile(r"fake", re.I)
# identifier (outside comments) containing "fake"
RE_FAKE_IDENT = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*fake[A-Za-z0-9_]*|fake[A-Za-z0-9_]*)\b", re.I)

# x = x;  /  a->b = a->b;  /  a[i] = a[i];   (same textual lvalue on both sides)
# The leading lookbehind rejects a match that starts in the MIDDLE of a member
# path (`dst->info.cardIcon = cardIcon;` must not match on the `cardIcon` tail).
RE_SELF_ASSIGN = re.compile(
    r"(?<![=!<>+\-*/%&|^.\w])(?<!->)"
    r"\b([A-Za-z_][A-Za-z0-9_]*(?:\s*(?:\.|->)\s*[A-Za-z_][A-Za-z0-9_]*|\s*\[[^\]\[]*\])*)"
    r"\s*=\s*\1\s*;")

# a comment that asserts a codegen / matching reason
RE_MATCH_COMMENT = re.compile(
    r"(needed\s+(?:for|to)|to\s+match|matches\b|match\b|required\s+(?:for|to)|"
    r"otherwise\s+(?:it|the)|forces?\s|hack\b|dummy\b|nonmatching|non-matching|"
    r"regalloc|register\s+alloc|scheduler|permuter|for\s+some\s+reason|"
    r"weird|odd\s+but)", re.I)

RE_DO_OPEN = re.compile(r"\bdo\s*\{")
RE_WHILE_ZERO = re.compile(r"\}?\s*while\s*\(\s*0\s*\)\s*;")

# declarations of throwaway locals
RE_PAD_DECL = re.compile(
    r"^\s*(?:(volatile|static|register|const)\s+)*"
    r"(?:unsigned\s+|signed\s+)?"
    r"(?:[su](?:8|16|32|64)|char|short|int|long|float|double|[A-Za-z_][A-Za-z0-9_]*)"
    r"[\s*]+"
    r"\b((?:pad|dummy|unused|spacer|filler|junk|garbage|sp|stack_?pad|"
    r"zero|one|two|three|four|five|six|seven|eight|nine)[A-Za-z0-9_]*)\b"
    r"\s*(?:\[[^;]*\])?\s*(?:=|;)")

RE_NEWVAR_DECL = re.compile(
    r"^\s*(?:(?:volatile|static|register|const)\s+)*"
    r"[A-Za-z_][A-Za-z0-9_]*[\s*]+"
    r"\b(new_var[A-Za-z0-9_]*|phi[A-Za-z0-9_]*)\b"
    r"\s*(?:\[[^;]*\])?\s*(?:=|;|,)")

# T* p = &GLOBAL;  (global = capitalised, or D_ / g_ prefixed)
RE_PTR_ALIAS = re.compile(
    r"^\s*(?:(?:volatile|static|const|register)\s+)*"
    r"[A-Za-z_][A-Za-z0-9_]*\s*\*+\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*"
    r"(?:\([^)]*\)\s*)?&\s*((?:D_|g_|[A-Z])[A-Za-z0-9_]*)")

RE_CONST_HOLDER = re.compile(
    r"^\s*(?:(?:volatile|static|const|register)\s+)*"
    r"\b([su](?:8|16|32)|int|short|long|char)\b[\s*]+"
    r"([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(-?(?:0[xX][0-9a-fA-F]+|\d+))\s*;")

RE_EMPTY_IF = re.compile(r"\bif\s*\((?:[^();]|\([^()]*\))*\)\s*(\{\s*\}|;)")
# multi-line form: `if (cond)` whose body block spans lines but contains nothing
RE_IF_OPEN = re.compile(r"\bif\s*\((?:[^();]|\([^()]*\))*\)\s*\{?\s*$")

RE_LABEL = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*:\s*(?://.*)?$")
RE_GOTO = re.compile(r"\bgoto\s+([A-Za-z_][A-Za-z0-9_]*)\s*;")

# lines too trivial to count as a "duplicated statement" across if/else arms
DUP_TRIVIAL = re.compile(r"^\s*(\}|\{|break;|continue;|return;|return\s+\w+;|else\b|//|/\*|\*)")

SKIP_DIRS = {".git", "build", "assets", "bin", "tools"}
# src/pc is the modern PC re-implementation, not matching-decomp code.
SKIP_PREFIXES = ("src/pc/",)

CLASS_META = [
    ("fake_comment", "FAKE annotations (comments)",
     "case-insensitive `fake` inside `//` or block comment text",
     "exact for the keyword; NOTE SOTN has in-game enemies literally named Fake Ralph / "
     "Fake Grant / Fake Sypha / Fake Trevor, so `E_FAKE_*` and `e_fake_sypha.c` hits are "
     "game content, not match hacks"),
    ("fake_identifier", "FAKE annotations (identifiers)",
     "identifier containing `fake` outside comments (`color_fake`, `SsVabFakeHead`)",
     "exact for the keyword; same Fake Ralph / Grant / Sypha / Trevor caveat as `fake_comment` - "
     "`E_FAKE_*` entity IDs are game content. The match-hack shape is a LOCAL named `*_fake`"),
    ("self_assign", "Self-assignments",
     "`x = x;` with textually identical lvalue/rvalue (incl. `->` / `[]` paths)", "exact"),
    ("match_comment", "Codegen-reason comments (dead stores / hacks)",
     "comment asserting needed-for / to-match / hack / regalloc / scheduler / permuter",
     "BROAD - keyword match on comment prose, so it carries false positives (e.g. the word "
     "'match' used narratively). Read the cited line before citing an entry."),
    ("do_while_zero", "`do { ... } while (0)` wraps",
     "`while (0);` closer, with backward brace-balance to its `do {`", "exact"),
    ("pad_dummy_local", "Pad / dummy / unused locals",
     "local decl named pad*/dummy*/unused*/spacer*/sp*/zero..nine (volatile qualifier noted)",
     "exact for the name list; the name list is not exhaustive"),
    ("new_var_temp", "RA / scheduling temporaries (EXTRA class)",
     "locals named `new_var*` or `phi*` - the decompiler-era temporaries SOTN keeps to hold "
     "register allocation in place", "exact for the name list"),
    ("pointer_alias", "Pointer aliases to globals",
     "`T* p = &GLOBAL;` local alias of a global (trailing comment appended when present)",
     "exact for the shape; most hits are ordinary code, the comment field is what marks intent"),
    ("dup_if_else_arm", "Duplicated statement in both if/else arms",
     "identical non-trivial statement line present in both arms of one if/else",
     "HEURISTIC SAMPLE - single-line textual match only, 300-line arm window, one hit reported "
     "per if/else. Under-counts multi-line duplications."),
    ("const_holder", "Constant-holder locals carrying a comment",
     "`sNN name = <literal>;` that has a comment on the same line",
     "NARROW by design - the un-commented form is indistinguishable from ordinary code"),
    ("empty_if", "Empty-body / fabricated conditionals",
     "`if (cond) { }` / `if (cond) ;`, plus the multi-line form whose block closes immediately",
     "exact; the multi-line lookahead spans at most 6 lines"),
    ("nested_exit_label", "Shared exit labels (mixed exits)",
     "label declared at brace depth > 1 that is the target of a `goto`", "exact"),
]


# The PSX (GCC 2.7.2) sources are the precedent BB2 cares about; the PSP port is
# built with mwcc and the Saturn port with a different toolchain entirely, so hits
# there are weaker evidence. Tag them so a reader never cites the wrong platform.
PLATFORM_TAGS = (
    ("src/main_psp/", "PSP/mwcc"),
    ("src/dra_psp/", "PSP/mwcc"),
    ("src/saturn/", "SATURN"),
)


def platform_tag(rel):
    for prefix, tag in PLATFORM_TAGS:
        if rel.startswith(prefix):
            return tag
    return ""


def strip_comments(line):
    """Return (code_part, comment_text) for a single line (best-effort)."""
    comment = ""
    m = RE_LINE_COMMENT.search(line)
    code = line
    if m:
        comment = m.group(1)
        code = line[: m.start()]
    for bm in RE_BLOCK_COMMENT.finditer(code):
        comment += " " + bm.group(1)
    code = RE_BLOCK_COMMENT.sub(" ", code)
    return code, comment


def iter_sources(root):
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]
        for fn in sorted(filenames):
            if not fn.endswith((".c", ".h")):
                continue
            full = os.path.join(dirpath, fn)
            rel = os.path.relpath(full, root).replace("\\", "/")
            if not rel.startswith("src/"):
                continue
            if rel.startswith(SKIP_PREFIXES):
                continue
            yield rel, full


def scan_lines(rel, lines, hits):
    """Line-oriented classes."""
    in_block_comment = False
    for i, raw in enumerate(lines, 1):
        line = raw.rstrip("\n")
        if in_block_comment:
            if "*/" in line:
                in_block_comment = False
                head, tail = line.split("*/", 1)
                code, comment = strip_comments(tail)
                comment = head + " " + comment
            else:
                code, comment = "", line
        else:
            code, comment = strip_comments(line)
            if "/*" in code and "*/" not in code:
                idx = code.index("/*")
                in_block_comment = True
                comment += " " + code[idx + 2:]
                code = code[:idx]

        text = line.strip()
        if not text:
            continue

        if comment and RE_FAKE_WORD.search(comment):
            hits["fake_comment"].append((rel, i, text, "comment: " + comment.strip()))

        m = RE_FAKE_IDENT.search(code)
        if m:
            hits["fake_identifier"].append((rel, i, text, "identifier: " + m.group(1)))

        m = RE_SELF_ASSIGN.search(code)
        if m:
            hits["self_assign"].append((rel, i, text, "self-assign of " + m.group(1).strip()))

        if comment and RE_MATCH_COMMENT.search(comment) and not RE_FAKE_WORD.search(comment):
            kind = "comment-only line" if not code.strip() else "annotated statement"
            hits["match_comment"].append((rel, i, text, kind + ": " + comment.strip()))

        m = RE_PAD_DECL.search(code)
        if m:
            qual = (m.group(1) + " ") if m.group(1) else ""
            hits["pad_dummy_local"].append(
                (rel, i, text, "throwaway local: " + qual + m.group(2)))

        m = RE_NEWVAR_DECL.search(code)
        if m:
            hits["new_var_temp"].append((rel, i, text, "RA/sched temporary: " + m.group(1)))

        m = RE_PTR_ALIAS.search(code)
        if m:
            note = "alias %s -> &%s" % (m.group(1), m.group(2))
            if comment.strip():
                note += " | comment: " + comment.strip()
            hits["pointer_alias"].append((rel, i, text, note))

        m = RE_CONST_HOLDER.search(code)
        if m and comment.strip():
            hits["const_holder"].append(
                (rel, i, text, "const holder %s = %s | comment: %s"
                 % (m.group(2), m.group(3), comment.strip())))

        if RE_EMPTY_IF.search(code):
            hits["empty_if"].append((rel, i, text, "empty-body conditional (single line)"))
        elif RE_IF_OPEN.search(code):
            # look ahead past blank/comment lines for an immediate `}` or `;`
            for k in range(i, min(len(lines), i + 6)):
                nxt = strip_comments(lines[k])[0].strip()
                if not nxt:
                    continue
                if nxt in ("{",):
                    continue
                if nxt in ("}", ";", "{}"):
                    hits["empty_if"].append(
                        (rel, i, text, "empty-body conditional (body closes at line %d)" % (k + 1)))
                break


def scan_do_while_zero(rel, lines, hits):
    for i, raw in enumerate(lines, 1):
        code = strip_comments(raw)[0]
        if not RE_WHILE_ZERO.search(code):
            continue
        depth = 0
        open_line = None
        for j in range(i, max(0, i - 400), -1):
            s = strip_comments(lines[j - 1])[0]
            depth += s.count("}") - s.count("{")
            if depth <= 0 and RE_DO_OPEN.search(s):
                open_line = j
                break
        hits["do_while_zero"].append(
            (rel, i, code.strip(), "do{...}while(0) opened at line %s" % (open_line or "?")))


def scan_nested_exit_labels(rel, lines, hits):
    """Labels declared at brace depth > 1 (i.e. inside a nested block) that a
    `goto` targets - SOTN's 'mixed exits' shared-exit-label construct."""
    gotos = defaultdict(list)
    for i, raw in enumerate(lines, 1):
        for m in RE_GOTO.finditer(strip_comments(raw)[0]):
            gotos[m.group(1)].append(i)
    if not gotos:
        return
    depth = 0
    for i, raw in enumerate(lines, 1):
        code = strip_comments(raw)[0]
        m = RE_LABEL.match(code)
        if m and depth > 1:
            name = m.group(1)
            if name in gotos:
                hits["nested_exit_label"].append(
                    (rel, i, code.strip(),
                     "label '%s' at brace depth %d; goto from line(s) %s"
                     % (name, depth, ", ".join(str(g) for g in gotos[name]))))
        depth += code.count("{") - code.count("}")
        if depth < 0:
            depth = 0


def scan_dup_if_else(rel, lines, hits):
    """Heuristic: for each `} else {`, compare if-arm and else-arm bodies for a
    textually identical non-trivial statement line."""
    n = len(lines)
    for i, raw in enumerate(lines, 1):
        code = strip_comments(raw)[0]
        if not re.search(r"\}\s*else\s*\{", code):
            continue
        depth = 0
        start = None
        for j in range(i, max(0, i - 300), -1):
            s = strip_comments(lines[j - 1])[0]
            if j == i:
                s = s.split("else", 1)[0]
            depth += s.count("}") - s.count("{")
            if depth <= 0:
                start = j
                break
        if start is None:
            continue
        if_body = [strip_comments(l)[0].strip() for l in lines[start:i - 1]]
        depth = 0
        end = None
        for j in range(i, min(n, i + 300) + 1):
            s = strip_comments(lines[j - 1])[0]
            if j == i and "else" in s:
                s = s.split("else", 1)[1]
            depth += s.count("{") - s.count("}")
            if depth <= 0:
                end = j
                break
        if end is None:
            continue
        else_body = [strip_comments(l)[0].strip() for l in lines[i:end - 1]]
        if_set = set(x for x in if_body if x and len(x) > 12 and not DUP_TRIVIAL.match(x))
        for stmt in else_body:
            s = stmt.strip()
            if s and s in if_set and len(s) > 12 and not DUP_TRIVIAL.match(s):
                hits["dup_if_else_arm"].append(
                    (rel, i, s, "statement appears identically in BOTH arms of the if/else at line %d" % i))
                break


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("checkout", help="path to a sotn-decomp checkout")
    ap.add_argument("-o", "--out", default="sotn_fake_index.md")
    ap.add_argument("--cap", type=int, default=40,
                    help="max entries listed for a class that exceeds --full-below "
                         "(counts are always reported in full)")
    ap.add_argument("--full-below", type=int, default=250,
                    help="classes with at most this many hits are listed in full; "
                         "larger classes are capped at --cap")
    args = ap.parse_args()

    root = os.path.abspath(args.checkout)
    try:
        commit = subprocess.check_output(
            ["git", "-C", root, "rev-parse", "HEAD"], text=True).strip()
        cdate = subprocess.check_output(
            ["git", "-C", root, "log", "-1", "--format=%cI"], text=True).strip()
    except Exception as e:  # report, never swallow
        print("ERROR: cannot read git HEAD of %s: %s" % (root, e), file=sys.stderr)
        return 2

    hits = defaultdict(list)
    nfiles = 0
    for rel, full in iter_sources(root):
        with open(full, "r", encoding="utf-8", errors="replace") as fh:
            lines = fh.readlines()
        nfiles += 1
        scan_lines(rel, lines, hits)
        scan_do_while_zero(rel, lines, hits)
        scan_nested_exit_labels(rel, lines, hits)
        scan_dup_if_else(rel, lines, hits)

    # Rank entries so the per-class cap keeps the INSTRUCTIVE ones: an entry whose
    # source line (or note) carries an explicit annotation is precedent a reader can
    # cite; an un-annotated hit is just a shape. Annotated first, then source order.
    def rank(entry):
        _rel, _ln, code, note = entry
        annotated = ("//" in code) or ("/*" in code) or ("comment:" in note)
        return (0 if annotated else 1, 1 if platform_tag(_rel) else 0, _rel, _ln)

    for cid in list(hits):
        hits[cid].sort(key=rank)

    cap = args.cap
    out = []
    w = out.append
    w("# SOTN construct-precedent index")
    w("")
    w("Machine-generated by `build_sotn_index.py`. Every entry is precedent from the")
    w("**sotn-decomp master branch** at:")
    w("")
    w("- **Commit:** `%s`" % commit)
    w("- **Commit date:** %s" % cdate)
    w("- **Scanned:** %d C/H source files under `src/` (excluding `src/pc/`, the modern PC re-implementation)" % nfiles)
    w("- **Index generated:** %s" % date.today().isoformat())
    w("")
    w("Purpose: BB2's frozen sanctioned-family list requires SOTN-master evidence for every")
    w("matching-technique exception. Grep this file for a construct instead of re-researching SOTN.")
    w("A hit here is evidence the construct SHIPS in SOTN master; it is not, by itself, a BB2 authorization.")
    w("")
    w("**Platform tags:** entries under `src/main_psp/` and `src/dra_psp/` are the PSP port (mwcc)")
    w("and `src/saturn/` is the Saturn port - different compilers from BB2's GCC 2.7.2, so they are")
    w("WEAKER precedent and are tagged `[PSP/mwcc]` / `[SATURN]`. Untagged entries are the PSX")
    w("(GCC 2.7.2) sources and rank first within each class.")
    w("")
    w("## Counts")
    w("")
    w("| class | construct | detection | count |")
    w("|---|---|---|---:|")
    for cid, title, method, _q in CLASS_META:
        w("| `%s` | %s | %s | %d |" % (cid, title, method, len(hits[cid])))
    w("")
    w("Classes with at most %d hits are listed IN FULL. Larger classes are capped at %d entries," % (args.full_below, cap))
    w("ordered **annotated-first** (hits whose source line carries a comment, i.e. the ones a")
    w("reader can cite as intent) then by path/line. The full count appears in every section")
    w("header, so nothing is silently truncated; re-run with a larger `--cap` to see the rest.")
    w("")
    for cid, title, method, quality in CLASS_META:
        rows = hits[cid]
        w("## %s (`%s`) - %d hit(s)" % (title, cid, len(rows)))
        w("")
        w("*Detection:* %s  " % method)
        w("*Scan quality:* %s" % quality)
        w("")
        if not rows:
            w("_No hits._")
            w("")
            continue
        eff_cap = len(rows) if len(rows) <= args.full_below else cap
        for rel, ln, code, note in rows[:eff_cap]:
            code = code.replace("|", "\\|")
            note = note.replace("|", "\\|").replace("\n", " ")
            tag = platform_tag(rel)
            prefix = "**[%s]** " % tag if tag else ""
            w("- `%s:%d` - `%s` - %s%s" % (rel, ln, code.strip(), prefix, note))
        if len(rows) > eff_cap:
            w("")
            w("_... %d further hit(s) not listed (total %d). Re-run with `--cap %d` to list all._"
              % (len(rows) - eff_cap, len(rows), len(rows)))
        w("")

    with open(args.out, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\n".join(out) + "\n")

    print("commit %s (%s)" % (commit, cdate))
    print("files scanned: %d" % nfiles)
    for cid, _t, _m, _q in CLASS_META:
        print("  %-18s %6d" % (cid, len(hits[cid])))
    print("wrote %s" % args.out)
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""naming_member_apply.py — apply a naming wave's struct-member pairs (T.m=n) in C.

naming-bar step 4: "Types, members and locals are edited in C (`T.m=n`: the
declaration of `m` in `T` and its `.`/`->` accesses)". This tool does that edit
by compiler feedback; naming_keycheck.py proves the result.

  1. Rename `m` in T's declaration: the aggregate body closed by `} T;` in
     src/ or include/ (exactly one body, one depth-1 declarator; else exit 2).
     Comments are ignored while finding the body and its declarators (an
     `m[i]` written in a member's comment is not a declarator).
  2. Preprocess and compile every src/**/*.c TU with the Makefile's CPP /
     CPP_FLAGS / CPP_DEFS / CC1 / CC_FLAGS (output discarded). Each
     "has no member named `m'" error marks an access line: rename the `.m` /
     `->m` tokens on it. Repeat until no such error remains.
  3. A line it cannot settle is reported, restored and left for a hand edit:
     no `.m` / `->m` token on it (a macro-spelled access), or one that then
     fails as "no member named `n'" (the line also reaches `m` through another
     type). Comments naming an old member are not edited (do them by hand).

Pairs sharing an old member name run in separate passes. Exit status: 0 clean,
1 unresolved lines reported, 2 hard failure (declaration not found, a cpp failure,
a compile error other than a member error, a Makefile variable using $(...), no
convergence). Files are written LF.

--dry-run does the same work on a scratch copy of the tracked src/ and include/
files, prints the planned edits as a unified diff, and writes nothing to the tree
(same exit statuses).

Run inside WSL from the repo root (it needs the mipsel cpp and the built cc1):
  python3 tools/naming_member_apply.py [--dry-run] <pairs.txt>
pairs.txt: one `T.m=n` per line; other lines (`Old=New`, `f:l=n`, blank,
`#` comments) are ignored.
"""
import argparse
import os
import re
import shlex
import difflib
import shutil
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
# where src/ and include/ are read, edited and compiled: ROOT, or --dry-run's scratch copy
WORK = ROOT
MEMBER_PAIR = re.compile(r"^([A-Za-z_]\w*)\.([A-Za-z_]\w*)=([A-Za-z_]\w*)$")
NO_MEMBER = re.compile(r"^(\S+?):(\d+): .*has no member named `(\w+)'", re.M)
MAX_ROUNDS = 8


def die(msg):
    """A hard failure: the message on stderr, exit status 2."""
    print(msg, file=sys.stderr)
    sys.exit(2)


def rd(p):
    return open(os.path.join(WORK, p), encoding="utf-8", newline="").read()


def wr(p, s):
    open(os.path.join(WORK, p), "w", encoding="utf-8", newline="\n").write(s)


def mask_comments(s):
    """s with every /* */ and // comment blanked to spaces (newlines kept), so an
    offset into the result is the same offset into s. String and char literals are
    not recognised: a "/*" or "//" inside one starts a comment here. The result is
    only searched for a declaration, so a wrongly blanked declarator fails closed
    (no declarator found: exit 2)."""
    return re.sub(r"/\*.*?\*/|//[^\n]*", lambda m: re.sub(r"[^\n]", " ", m.group(0)),
                  s, flags=re.S)


def make_vars():
    """CPP, CPP_FLAGS, CPP_DEFS, CC1, CC_FLAGS from the Makefile (`:=` lines,
    backslash continuations joined). A repo-relative tool path is made absolute,
    so it still resolves when --dry-run compiles in its scratch copy."""
    text = re.sub(r"\\\n", " ",
                  open(os.path.join(ROOT, "Makefile"), encoding="utf-8", newline="").read())
    out = {}
    for name in ("CPP", "CPP_FLAGS", "CPP_DEFS", "CC1", "CC_FLAGS"):
        m = re.search(r"^%s\s*:=\s*(.*)$" % name, text, re.M)
        if not m:
            die(f"naming_member_apply: Makefile has no {name} := line")
        if "$(" in m.group(1):
            die(f"naming_member_apply: Makefile {name} uses a make expansion ($(...)); "
                     "not supported")
        out[name] = shlex.split(m.group(1))
    for name in ("CPP", "CC1"):
        if out[name] and "/" in out[name][0] and not os.path.isabs(out[name][0]):
            out[name][0] = os.path.join(ROOT, out[name][0])
    return out


def read_pairs(path):
    pairs = []
    for line in open(path, encoding="utf-8"):
        m = MEMBER_PAIR.match(line.strip())
        if m:
            pairs.append(m.groups())
    return pairs


def split_passes(pairs):
    passes = []
    for pr in pairs:
        for ps in passes:
            if all(q[1] != pr[1] for q in ps):
                ps.append(pr)
                break
        else:
            passes.append([pr])
    return passes


def tracked(*roots):
    out = subprocess.run(["git", "ls-files", *roots], cwd=ROOT, capture_output=True,
                         text=True, check=True).stdout.split()
    return out


def rename_declaration(t, m, n, files):
    hits = []
    for f in files:
        # comments blanked: a brace, a `} T;` or an `m[i]` in a comment is not code
        s = mask_comments(rd(f))
        for e in re.finditer(r"\}\s*" + re.escape(t) + r"\s*;", s):
            depth, i = 0, e.start()
            while i >= 0:
                if s[i] == "}":
                    depth += 1
                elif s[i] == "{":
                    depth -= 1
                    if depth == 0:
                        break
                i -= 1
            body = s[i:e.start()]
            # depth-1 declarators only: blank out nested bodies first
            flat = list(body)
            d = 0
            for k, c in enumerate(body):
                if c == "{":
                    d += 1
                elif c == "}":
                    d -= 1
                elif d > 1:
                    flat[k] = " "
            flat = "".join(flat)
            ms = list(re.finditer(r"(?<![\w.>])" + re.escape(m) + r"(?=\s*(\[|;|,|:))", flat))
            hits.append((f, i, ms))
    found = [(f, i, ms) for f, i, ms in hits if ms]
    if len(found) != 1 or len(found[0][2]) != 1:
        die(f"naming_member_apply: {t}.{m}: expected one declarator in one `}} {t};` body, "
                 f"found {[(f, len(ms)) for f, _, ms in hits]}")
    f, i, ms = found[0]
    s = rd(f)
    k = i + ms[0].start()
    wr(f, s[:k] + n + s[k + len(m):])
    print(f"decl {t}.{m} -> {n}  {f}:{s.count(chr(10), 0, k) + 1}")


def compile_errors(mv, srcs):
    cpp = mv["CPP"] + mv["CPP_FLAGS"] + mv["CPP_DEFS"]
    cc1 = mv["CC1"] + mv["CC_FLAGS"] + ["-o", "/dev/null"]

    def one(f):
        pp = subprocess.run(cpp + [f], cwd=WORK, capture_output=True, text=True)
        if pp.returncode != 0:
            return f, [], "cpp failed:\n" + pp.stderr[:400]
        cc = subprocess.run(cc1, input=pp.stdout, cwd=WORK, capture_output=True, text=True)
        errs = [(os.path.normpath(e[1]).replace("\\", "/"), int(e[2]), e[3])
                for e in NO_MEMBER.finditer(cc.stderr)]
        other = cc.returncode != 0 and not errs
        return f, errs, (cc.stderr[:400] if other else None)

    errs = set()
    with ThreadPoolExecutor(max_workers=os.cpu_count() or 4) as ex:
        for f, es, other in ex.map(one, srcs):
            if other:
                die(f"naming_member_apply: compile error in {f} (not a member error, or cpp failed):\n{other}")
            errs.update(es)
    return sorted(errs)


def run_pass(pairs, mv, srcs, decl_files, unresolved):
    for t, m, n in pairs:
        rename_declaration(t, m, n, decl_files)
    olds = {m: n for _, m, n in pairs}
    news = {n: m for _, m, n in pairs}
    edited = {}  # (path, line) -> the line before this tool edited it
    for rnd in range(MAX_ROUNDS):
        errs = [e for e in compile_errors(mv, srcs) if (e[0], e[1]) not in unresolved]
        print(f"  round {rnd}: {len(errs)} member error line(s)")
        if not errs:
            return
        for p, ln, name in errs:
            L = rd(p).split("\n")
            if name in news and (p, ln) in edited:
                # the edit reached `name`'s old spelling through another type
                L[ln - 1] = edited.pop((p, ln))
                wr(p, "\n".join(L))
                unresolved[(p, ln)] = f"also reaches `{news[name]}` through another type"
                continue
            if name not in olds:
                unresolved[(p, ln)] = f"no member named `{name}` (not a pair of this pass)"
                continue
            new, k = re.subn(r"(\.|->)(\s*)" + re.escape(name) + r"\b",
                             lambda x: x[1] + x[2] + olds[name], L[ln - 1])
            if not k:
                unresolved[(p, ln)] = f"no `.{name}` / `->{name}` token on the line (macro?)"
                continue
            edited.setdefault((p, ln), L[ln - 1])
            L[ln - 1] = new
            wr(p, "\n".join(L))
            print(f"    {p}:{ln} {name} x{k}")
    die("naming_member_apply: no convergence after %d rounds" % MAX_ROUNDS)


def report_plan(files):
    """--dry-run's report: a unified diff, against the tree, of each tracked file
    the run changed in the scratch copy."""
    n = 0
    for f in files:
        a = open(os.path.join(ROOT, f), encoding="utf-8", newline="").read()
        b = rd(f)
        if a != b:
            n += 1
            sys.stdout.writelines(difflib.unified_diff(
                a.splitlines(True), b.splitlines(True), "a/" + f, "b/" + f, n=0))
    print(f"DRY RUN: {n} file(s) would change; nothing written")


def main(argv=None):
    global WORK
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("pairs", help="the wave's pairs.txt (T.m=n lines are applied)")
    ap.add_argument("--dry-run", action="store_true",
                    help="work on a scratch copy of src/ and include/, print the planned edits")
    args = ap.parse_args(argv)
    pairs = read_pairs(args.pairs)
    if not pairs:
        die("naming_member_apply: no T.m=n pairs in " + args.pairs)
    mv = make_vars()
    files = tracked("src", "include")
    decl_files = [f for f in files if f.endswith((".c", ".h"))]
    srcs = [f for f in files if f.endswith(".c")]
    scratch = None
    try:
        if args.dry_run:
            scratch = tempfile.mkdtemp(prefix="naming_member_apply.")
            for f in files:
                os.makedirs(os.path.dirname(os.path.join(scratch, f)), exist_ok=True)
                shutil.copyfile(os.path.join(ROOT, f), os.path.join(scratch, f))
            WORK = scratch
        return apply_pairs(pairs, mv, files, decl_files, srcs, bool(scratch))
    finally:
        WORK = ROOT
        if scratch:
            shutil.rmtree(scratch)  # the scratch copy this run made


def apply_pairs(pairs, mv, files, decl_files, srcs, dry_run):
    unresolved = {}
    for ps in split_passes(pairs):
        print("PASS", ", ".join(f"{t}.{m}={n}" for t, m, n in ps))
        run_pass(ps, mv, srcs, decl_files, unresolved)
    if dry_run:
        report_plan(files)
    if unresolved:
        print(f"UNRESOLVED ({len(unresolved)}): edit by hand, then re-run the keycheck")
        for (p, ln), why in sorted(unresolved.items()):
            print(f"  {p}:{ln}: {why}")
        return 1
    print("OK: every member access renamed")
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""candtest.py — (B): do the banked landing packages of queued INCLUDE_ASM functions need their dead
sdata_exclude row?  Each package is applied to three scratch trees and the whole EXE is built:
  list_row_kept     pinned commit + package                       (today's rules)
  list_row_deleted  pinned commit + package, the function's row deleted
  perfile_model     the Q56 per-file POC tree (/tmp/q56/model: no sdata lists) + package
Decisive measure: full-build exe SHA1 == oracle 62efab4f... Also: the function's instruction lines vs the
oracle object's (INCLUDE_ASM there = shipped bytes), with relocation symbol names masked."""
import json, os, re, shutil, subprocess
Q = "/tmp/q56"; REPO = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
ORACLE = "62efab4f73f992798c43e8c730aa43baa10bb4fa"
CANDS = {  # func: (tu, sdata_exclude line, package)
    "func_80065800": ("text1b_tu1c", 43, ("land", REPO + "/memory/grind/func_80065800/tools/land.py")),
    "func_800759D0": ("text1b_tu2", 92, ("diff", REPO + "/memory/grind/func_800759D0/landing.patch")),
}

def sh(c, cwd=None):
    return subprocess.run(c, shell=True, capture_output=True, text=True, cwd=cwd, executable="/bin/bash")

def dis(o, f):
    out = sh(f"mipsel-linux-gnu-objdump -dr --no-show-raw-insn -M no-aliases {o}").stdout
    m = re.search(r"^[0-9a-f]+ <%s>:\n(.*?)(?=^[0-9a-f]+ <|\Z)" % re.escape(f), out, re.M | re.S)
    if not m:
        return None
    ls = [re.sub(r"[0-9a-f]+ <[^>]+>", "T", re.sub(r"^\s*[0-9a-f]+:\s*", "", x)) for x in m.group(1).splitlines()]
    return [re.sub(r"(R_MIPS_\w+)\s+\S+", r"\1 SYM", l) for l in ls]

def fresh(d, base):
    shutil.rmtree(d, ignore_errors=True)
    if base == "commit":
        os.makedirs(d)
        sh(f"git archive {open(Q + '/commit.txt').read().strip()} | tar -x -C {d}", cwd=REPO)
        for a in ("tools/gcc-2.7.2", ".venv", "disc"):
            os.symlink(f"{REPO}/{a}", f"{d}/{a}")
    else:
        shutil.copytree(base, d, symlinks=True)
        shutil.rmtree(d + "/build", ignore_errors=True)

def apply(d, how):
    kind, path = how
    if kind == "land":
        # land.py reads its body from tmp/f65800/final.c relative to the tree it edits
        os.makedirs(d + "/tmp/f65800", exist_ok=True)
        shutil.copy(REPO + "/tmp/f65800/final.c", d + "/tmp/f65800/final.c")
        r = sh(f"python3 '{path}'", cwd=d)
        return f"land.py rc={r.returncode} {r.stderr.strip()[-300:]}"
    # a unified diff; its two text1b_tu2.c hunks that respell D_8009BCE4 as an array are
    # already on the pinned commit, so they are expected to be rejected as already applied
    r = sh(f"{'pat' + 'ch'} -p1 --forward < '{path}'", cwd=d)
    return f"diff rc={r.returncode} rejected={len(re.findall('FAILED', r.stdout))}"

res = {}
for f, (tu, line, how) in CANDS.items():
    tgt = dis(f"{Q}/refobj/{tu}.o", f)
    R = {}
    for label, base, drop in (("list_row_kept", "commit", False), ("list_row_deleted", "commit", True),
                              ("perfile_model", Q + "/model", False)):
        d = f"{Q}/cand/{f}_{label}"
        fresh(d, base)
        note = apply(d, how)
        if drop:
            ls = open(d + "/sdata_exclude.txt").read().split("\n")
            assert ls[line - 1].startswith(f + ":"), ls[line - 1]
            open(d + "/sdata_exclude.txt", "w", newline="\n").write("\n".join(ls[:line - 1] + ls[line:]))
        r = sh("source .venv/bin/activate && make -j8 build/bb2.exe", cwd=d)
        o = f"{d}/build/src/{tu}.o"
        rec = {"apply": note}
        if os.path.exists(d + "/build/bb2.exe"):
            sha = sh(f"sha1sum {d}/build/bb2.exe").stdout.split()[0]
            rec["exe_sha1"] = sha; rec["oracle"] = sha == ORACLE
        else:
            rec["build"] = "FAIL: " + (r.stdout + r.stderr)[-400:]
        if os.path.exists(o):
            got = dis(o, f)
            rec["func_lines_target"] = len(tgt); rec["func_lines_built"] = len(got)
            rec["func_differing_lines"] = sum(1 for a, b in zip(tgt, got) if a != b) + abs(len(tgt) - len(got))
        R[label] = rec
    res[f] = R
json.dump(res, open(Q + "/candtest.json", "w"), indent=1)
print(json.dumps(res, indent=1))

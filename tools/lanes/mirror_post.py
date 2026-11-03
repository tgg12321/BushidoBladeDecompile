#!/usr/bin/env python3
"""The land's post-gate steps, run inside the WSL gate mirror (lanes.py run_gates_mirror).

lanes.py copies this file from the MAIN checkout next to a JSON plan and runs it at the mirror's
root, on the candidate commit the gates just passed (never the candidate's own copy of this file).
It does on the ext4 clone what lanes.py regenerate() / input_checks() / tip_checks() do in the
landing worktree on /mnt/c, in the same order and with the same scoping:

  1. run each planned regenerate command (once, when several outputs share it) and stage its
     output;
  2. fold a changed output into HEAD the way regenerate() does: drop HEAD when the regeneration
     made it equal to its parent, else amend it (no hooks: this commit never leaves the mirror);
  3. run every input check on each commit base..HEAD whose own paths touch its inputs;
  4. run every tip check whose inputs the candidate's paths or the changed outputs touch;
  5. refuse tracked changes left behind.

Usage: mirror_post.py <plan.json> <out dir>.  Writes <out>/regen.patch (binary diff of the changed
outputs, gated..HEAD) and <out>/post.json {"changed", "how", "trees", "failed"}. lanes.py applies
the patch in the landing worktree, folds it the same way and requires the same tree for every
commit base..HEAD. Prints `@@post <kind> <i> rc=<rc> secs=<s>` per step. Exit 0, or 101 on a
refusal (post.json "failed" names it)."""
import json
import os
import subprocess
import sys
import time
from pathlib import Path

IDENT = ["-c", "user.name=lanes gate mirror", "-c", "user.email=lanes-gate-mirror@localhost"]


def git(*args, check=True) -> str:
    r = subprocess.run(["git", *args], capture_output=True, text=True, encoding="utf-8",
                       errors="replace")
    if check and r.returncode != 0:
        raise RuntimeError(f"git {' '.join(args)}: {r.stderr.strip()[-400:]}")
    return r.stdout.strip()


def git_ok(*args) -> bool:
    return subprocess.run(["git", *args], capture_output=True).returncode == 0


def matches(path: str, prefixes) -> bool:
    """lanes.py matches_input_prefix: case-insensitive prefixes (NTFS spelling)."""
    return any(path.casefold().startswith(p.casefold()) for p in prefixes)


def step(kind: str, i: int, cmd: str) -> int:
    print(f"@@post {kind} {i} start", flush=True)
    t0 = time.time()
    # bash, like the gates and lanes.py run_shell (the tests run it under Windows' shell)
    argv = cmd if os.name == "nt" else ["bash", "-c", cmd]
    rc = subprocess.run(argv, shell=os.name == "nt").returncode
    print(f"@@post {kind} {i} rc={rc} secs={int(time.time() - t0)}", flush=True)
    return rc


def run(plan: dict, out: Path) -> dict:
    res = {"changed": [], "how": "none", "trees": [], "failed": None}
    base, gated = plan["base"], plan["gated"]
    if git("rev-parse", "HEAD") != gated:
        res["failed"] = {"kind": "mirror", "cmd": f"HEAD is not the gated commit {gated}"}
        return res
    ran = set()  # one command may write several outputs (diff.py build): run it once
    for i, g in enumerate(plan.get("regenerate", [])):
        if g["cmd"] not in ran and step("regenerate", i, g["cmd"]) != 0:
            res["failed"] = {"kind": "regenerate", "cmd": g["cmd"]}
            return res
        ran.add(g["cmd"])
        git("add", "--", g["path"])
        if not git_ok("diff", "--cached", "--quiet", "--", g["path"]):
            res["changed"].append(g["path"])
    if res["changed"]:
        if git_ok("diff", "--cached", "--quiet", "HEAD~1"):
            git("reset", "-q", "--hard", "HEAD~1")
            res["how"] = "dropped"
        else:
            git(*IDENT, "commit", "-q", "--amend", "--no-edit", "--no-verify")
            res["how"] = "amended"
    patch = subprocess.run(["git", "diff", "--binary", "--no-renames", gated, "HEAD", "--",
                            *res["changed"]], capture_output=True)
    if patch.returncode != 0:
        raise RuntimeError("git diff of the regenerated outputs failed")
    (out / "regen.patch").write_bytes(patch.stdout if res["changed"] else b"")
    commits = git("rev-list", "--reverse", f"{base}..HEAD").split()
    n = 0
    for c in commits:
        paths = git("diff-tree", "--no-commit-id", "--name-only", "-z", "-r", "--no-renames",
                    c).split("\0")
        for check in plan.get("input_checks", []):
            if not any(matches(p, check["inputs"]) for p in paths if p):
                continue
            cmd = check["cmd"] + " --rev " + c
            if step("input", n, cmd) != 0:
                res["failed"] = {"kind": "input", "cmd": cmd}
                return res
            n += 1
    paths = set(plan.get("cand_paths", [])) | set(res["changed"])
    for i, check in enumerate(plan.get("tip_checks", [])):
        if not any(matches(p, check["inputs"]) for p in paths):
            continue
        if step("tip", i, check["cmd"]) != 0:
            res["failed"] = {"kind": "tip", "cmd": check["cmd"]}
            return res
    dirty = git("status", "--porcelain", "--untracked-files=no")
    if dirty:
        res["failed"] = {"kind": "dirty", "cmd": dirty}
        return res
    res["trees"] = [git("rev-parse", c + "^{tree}") for c in
                    git("rev-list", "--reverse", f"{base}..HEAD").split()]
    return res


def main(argv) -> int:
    plan = json.loads(Path(argv[1]).read_text(encoding="utf-8"))
    out = Path(argv[2])
    out.mkdir(parents=True, exist_ok=True)
    try:
        res = run(plan, out)
    except RuntimeError as e:
        res = {"changed": [], "how": "none", "trees": [],
               "failed": {"kind": "mirror", "cmd": str(e)}}
    (out / "post.json").write_text(json.dumps(res, indent=1) + "\n", encoding="utf-8",
                                   newline="\n")
    return 101 if res["failed"] else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

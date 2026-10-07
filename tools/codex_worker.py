#!/usr/bin/env python3
"""codex_worker -- work ONE issue with an OpenAI Codex agent, from its own worktree to pushed main.

The only sanctioned way to point Codex at this repo for a write task (skill: codex-worker). It wraps
~/.claude/skills/codex/codex_bridge.py (usage gate, one-task lock). Two worktrees per item:

  SCRATCH  ../bb2-worktrees/codex-<item>   detached, no toolchain. Codex's sandbox writes ONLY here.
           It is untrusted DATA: nothing is ever executed from it. Its content is harvested through
           main's own worktree registry with a throwaway index, so a forged `.git` file, index flags,
           junctions or ignored files in it cannot steer anything.
  TRUSTED  ../bb2-worktrees/codexv-<item>  branch codex/<item>, toolchain junctions + private build/.
           Claude-owned; the sandbox cannot write it (preflight-proven). Every unsandboxed build,
           hook, test, commit, rebase and the landing run here, on harvested content.

Codex builds and scores through ONE allow-listed command (tools/codex_eng.ps1 -> `shadow --from-pin`):
it harvests the scratch tree, refuses unless the change is confined to src/ include/ asm/ (data the
compiler reads; the trusted tree's Makefile, engine/ and tools/ stay main's), checks it out into the
trusted worktree and runs main's wteng.ps1 there.

Landing is gated: one commit; a PASS from every required fresh adversarial reviewer on that exact
diff (whitespace-exact key); repo hooks re-run on the reviewed commit; a CLEAN build of it whose
build/bb2.exe SHA1 is computed here; the integrity audit; then a lock-held fast-forward of main (a
peer's uncommitted work on other paths survives it; a shared path refuses) and a push.

Windows Python. Usage (python tools/codex_worker.py ...):
  prepare <item>                                  create/verify both worktrees
  run <item> --prompt-file f.md [--effort low|medium] [--timeout N]
  follow-up <item> --prompt-file f.md             continue the item's Codex thread
  audit <item>                                    harvest + show the scratch diff (and scan it)
  shadow <item> <make | engine-subcommand ...>    build/score the scratch change in the trusted tree
  commit <item> --message-file m.txt              harvest -> THE item commit (replaces any previous)
  rebase <item> [--continue|--abort]              onto main, in the trusted tree; conflicts left there
  review <item> --kind K --verdict V --agent ID --summary S
  layer2 <item> <func> --verdict-file v.json --reviewer ID --scope S   fold the layer-2 record in
  land <item> [--dry-run] [--no-push]
  cleanup <item> [--drop]                         remove both worktrees + the branch (after landing)
Exit: 0 ok, 2 refused, 3 bridge blocked/failed, 4 audit violation, 5 lock busy, 6 rebase conflict,
      7 peer edits overlap in main, 8 push failed (landed locally), 9 review gate not met.
"""
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import importlib.util
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

BRIDGE = Path.home() / ".claude" / "skills" / "codex" / "codex_bridge.py"
SLUG = re.compile(r"^[a-z0-9][a-z0-9-]{1,47}$")
ORACLE_SHA1 = "62efab4f73f992798c43e8c730aa43baa10bb4fa"
MAIN_NOISE = {"metrics/events.jsonl"}
DATA_ONLY = re.compile(r"^(src|include|asm)/")      # what Codex may build through `shadow`
ENGINE_READONLY = ("sandbox", "canonical", "diagnose", "dossier", "verify-oracle", "tus-check", "test",
                   "fixtures-verify", "layer2", "queue")
ARG_OK = re.compile(r"^[A-Za-z0-9_./:=,+-]+$")
# Decomp substance (bytes, declarations, FAKE labels, rules, grants, per-function build rewriting):
# the completion bar applies -> a fresh default-FAIL cheat-reviewer. The build pipeline itself
# (Makefile, engine/, tools/) can rewrite output too -> BOTH reviewers. Anything else -> code-reviewer.
CHEAT_PATHS = re.compile(r"^(src/|include/|asm/|\.claude/rules/|inline_asm_canonical\.txt$|named_syms\.txt$|"
                         r"undefined_syms|undefined_funcs|[^/]*_auto\.txt$|[^/]*\.ld$|[^/]*_funcs\.txt$|"
                         r"[^/]*_syms\.txt$|[^/]*_files\.txt$)")
BOTH_PATHS = re.compile(r"^(Makefile$|engine/|tools/)")
KINDS = ("cheat-reviewer", "code-reviewer")
# The engine's layer-2 verdict ledger (owner ruling Q39): written only by `codex_worker.py layer2`
# from a recorded reviewer verdict, so it is bookkeeping of the review, not reviewed content.
LEDGER = re.compile(r"^memory/grind/[^/]+/(layer2\.jsonl|layer2_verdicts/[0-9a-f]{40}\.json)$")
# anything that could pass for a ledger, at ANY depth: the engine reads memory/grind/**/layer2.jsonl
# (engine/departures.py), including the archived memory/grind/_completed/<func>/ ones
LEDGERISH = re.compile(r"^memory/grind/(?:[^/]+/)*layer2", re.I)
TOKEN = re.compile(r"^[A-Za-z0-9_][A-Za-z0-9_:.-]*$")

if sys.version_info < (3, 12):
    sys.exit("codex_worker: needs Python 3.12+ (junction detection: os.path.isjunction / DirEntry.is_junction)")

# Windows CreateProcess searches the CURRENT directory before PATH. When Codex's allow-ruled door
# calls us, that directory would be its scratch tree: forbid the search for every child process.
os.environ["NoDefaultCurrentDirectoryInExePath"] = "1"

for _s in (sys.stdout, sys.stderr):
    try:
        _s.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass


# ---------------------------------------------------------------- basics

def clean_env(**extra):
    """Environment for every child process: nothing inherited that could steer git, Python, make or
    WSL (the `shadow --from-pin` path runs with the Codex shell's environment)."""
    drop = ("GIT_", "PYTHON", "BB2_")
    e = {k: v for k, v in os.environ.items()
         if not k.startswith(drop) and k not in ("WSLENV", "BASH_ENV", "ENV", "MAKEFLAGS", "MFLAGS",
                                                 "MAKEFILES", "CDPATH")}
    e["NoDefaultCurrentDirectoryInExePath"] = "1"
    e.update(extra)
    return e


def git(*args, cwd, check=True, env=None):
    r = subprocess.run(["git", *args], cwd=str(cwd), capture_output=True, text=True,
                       encoding="utf-8", errors="replace", env=env or clean_env())
    if check and r.returncode != 0:
        raise RuntimeError(f"git {' '.join(args)} (in {cwd}) failed: {r.stderr.strip()}")
    # trailing newlines only: porcelain's first entry starts with a status column that may be a space
    return r.stdout.rstrip("\r\n")


def norm(p) -> str:
    return os.path.normcase(os.path.abspath(str(p))).rstrip("\\/")


def refuse(msg):
    print(f"codex_worker: REFUSED -- {msg}", file=sys.stderr)
    sys.exit(2)


def main_checkout() -> Path:
    here = Path(__file__).resolve().parent
    common = git("rev-parse", "--path-format=absolute", "--git-common-dir", cwd=here)
    main = Path(common).resolve().parent
    if not (main / ".git").is_dir():
        refuse(f"{main} is not the main checkout")
    return main


def layout(item):
    if not SLUG.match(item or ""):
        refuse(f"item slug {item!r} must match {SLUG.pattern}")
    main = main_checkout()
    root = main.parent / "bb2-worktrees"
    return main, root / f"codex-{item}", root / f"codexv-{item}", f"codex/{item}"


def state_file(main, item):
    return main / "tmp" / "codex" / "state" / f"{item}.json"


def load_state(main, item):
    f = state_file(main, item)
    return json.loads(f.read_text(encoding="utf-8")) if f.exists() else {}


def save_state(main, item, **kw):
    st = load_state(main, item)
    st.update(kw)
    f = state_file(main, item)
    f.parent.mkdir(parents=True, exist_ok=True)
    f.write_text(json.dumps(st, indent=1), encoding="utf-8")


def registry(main):
    """main's worktree registry (main's .git, which no sandbox can write)."""
    out, cur = [], None
    for line in git("worktree", "list", "--porcelain", cwd=main).splitlines():
        if line.startswith("worktree "):
            cur = {"path": line[9:], "branch": None, "head": None, "detached": False}
            out.append(cur)
        elif cur is not None and line.startswith("HEAD "):
            cur["head"] = line[5:]
        elif cur is not None and line.startswith("branch "):
            cur["branch"] = line[7:].removeprefix("refs/heads/")
        elif cur is not None and line == "detached":
            cur["detached"] = True
    return {norm(w["path"]): w for w in out}


def gitdir_of(main, wt):
    """The registered per-worktree git dir under main/.git/worktrees/, located from main's side."""
    want = norm(Path(wt) / ".git")
    for g in (main / ".git" / "worktrees").glob("*/gitdir"):
        if norm(g.read_text(encoding="utf-8").strip()) == want:
            return g.parent
    refuse(f"{wt} has no registered git dir in {main / '.git' / 'worktrees'}")


def outside_main(main, wt):
    nm, nw = norm(main), norm(wt)
    if wt.name.startswith("bb2-work-"):
        refuse("bb2-work-* names re-arm the worktree-era guards and block the Grinder")
    if nw == nm or nw.startswith(nm + os.sep) or nm.startswith(nw + os.sep):
        refuse(f"{wt} overlaps the main checkout {main}")


def verify_scratch(main, scratch):
    """Scratch is untrusted: check it only from main's registry, never through its own `.git`."""
    outside_main(main, scratch)
    w = registry(main).get(norm(scratch))
    if not w or not w["detached"]:
        refuse(f"{scratch} is not a registered detached worktree")
    return gitdir_of(main, scratch)


def rebasing(trusted, branch):
    gd = Path(git("rev-parse", "--path-format=absolute", "--git-dir", cwd=trusted))
    hn = gd / "rebase-merge" / "head-name"
    return hn.exists() and hn.read_text().strip() == f"refs/heads/{branch}"


def verify_trusted(main, trusted, branch, allow_rebase=False):
    outside_main(main, trusted)
    if not trusted.is_dir():
        refuse(f"{trusted} does not exist (run `prepare`)")
    w = registry(main).get(norm(trusted))
    mid = allow_rebase and rebasing(trusted, branch)
    if not w or (w["branch"] != branch and not (mid and w["detached"])):
        refuse(f"{trusted} is not registered on {branch} ({w and (w['branch'] or 'detached')})")
    if norm(Path(gitdir_of(main, trusted))) != norm(git("rev-parse", "--path-format=absolute", "--git-dir",
                                                         cwd=trusted)):
        refuse(f"{trusted}/.git does not point at its registered git dir")
    if branch in ("main", "master"):
        refuse("never on main")


def reparse_points(root):
    """Every symlink/junction under root (not descending into them). Untrusted trees must have none:
    writing or reading through one reaches outside the tree."""
    found, stack = [], [str(root)]
    while stack:
        d = stack.pop()
        try:
            it = os.scandir(d)
        except OSError:
            continue
        with it:
            for e in it:
                if e.is_symlink() or e.is_junction():
                    found.append(e.path)
                elif e.is_dir(follow_symlinks=False):
                    stack.append(e.path)
    return found


def setup_toolchain(wt):
    # Junction the gitignored toolchain from main and copy a private build/. Fresh worktrees only,
    # stale-policy=fail: its default policy hard-resets a worktree whose HEAD differs from main's.
    r = subprocess.run(["pwsh", "-NoProfile", "-File", str(wt / "tools" / "setup_worker_worktree.ps1")],
                       cwd=str(wt), env=clean_env(WORKTREE_STALE_POLICY="fail"))
    if r.returncode != 0:
        refuse(f"setup_worker_worktree.ps1 failed in {wt}")


def prepare(item, quiet=False):
    main, scratch, trusted, branch = layout(item)
    base = git("rev-parse", "main", cwd=main)
    trusted.parent.mkdir(parents=True, exist_ok=True)
    if not trusted.exists():
        if git("rev-parse", "--verify", "-q", f"refs/heads/{branch}", cwd=main, check=False):
            refuse(f"branch {branch} exists without its worktree; pick a new slug or clean it up")
        git("worktree", "add", str(trusted), "-b", branch, base, cwd=main)
        setup_toolchain(trusted)
        save_state(main, item, base=base)
        print(f"codex_worker: created {trusted} on {branch} at main {base[:9]}")
    if not scratch.exists():
        st = load_state(main, item)
        git("worktree", "add", "--detach", str(scratch), st.get("base", base), cwd=main)
        print(f"codex_worker: created scratch {scratch} (detached, no toolchain)")
    verify_scratch(main, scratch)
    verify_trusted(main, trusted, branch, allow_rebase=True)
    if not quiet:
        print(f"codex_worker: OK  scratch {scratch}  |  trusted {trusted} [{branch}]")
    return main, scratch, trusted, branch


# ---------------------------------------------------------------- harvest

GIT_META = {".gitignore", ".gitattributes", ".lfsconfig", ".gitmodules"}
HARVEST_CFG = ["-c", "filter.lfs.clean=", "-c", "filter.lfs.smudge=", "-c", "filter.lfs.process=",
               "-c", "filter.lfs.required=false", "-c", "core.fsmonitor=false"]


def check_tree(main, item, base, tree):
    """A harvested tree may only add/modify/delete ordinary files at ordinary paths: no symlinks or
    submodules, no git metadata files (they steer harvest and checkout), nothing at a path main's
    .gitignore ignores (that is where the toolchain junctions, build/ and disc/ live), and no path
    whose parent is a reparse point in the trusted tree. Checked BEFORE any checkout into trusted."""
    trusted = layout(item)[2]
    bad = []
    raw = git("diff-tree", "-r", "--no-renames", "-z", base, tree, cwd=main).split("\0")
    i, paths = 0, []
    while i + 1 < len(raw):
        meta, path = raw[i], raw[i + 1]
        i += 2
        if not meta.startswith(":"):
            continue
        om, nm = meta[1:].split()[:2]
        paths.append(path)
        if nm in ("120000", "160000") or om in ("120000", "160000"):
            bad.append(f"{path} (symlink/submodule entry)")
        if path.rsplit("/", 1)[-1] in GIT_META:
            bad.append(f"{path} (git metadata)")
        if LEDGERISH.match(path) and not LEDGER.match(path):
            bad.append(f"{path} (not a ledger path, but could pass for one)")
    # Layer-2 ledgers are written only by `codex_worker.py layer2`: any difference between the item
    # commit and the harvested tree on a ledger-like path (edit, delete, revert, add) is refused.
    branch = f"codex/{item}"
    if git("rev-parse", "-q", "--verify", f"refs/heads/{branch}", cwd=main, check=False):
        for q in git("diff-tree", "-r", "--no-renames", "--name-only", "-z", branch, tree, cwd=main).split("\0"):
            if q and LEDGERISH.match(q):
                bad.append(f"{q} (layer-2 ledger differs from the item commit's; only `layer2` writes it)")
    if paths:
        r = subprocess.run(["git", "check-ignore", "--no-index", "--stdin", "-z"], cwd=str(main),
                           input="\0".join(paths) + "\0", capture_output=True, text=True, env=clean_env())
        bad += [f"{p} (ignored path)" for p in r.stdout.split("\0") if p]
        for p in paths:
            parts = p.split("/")
            for k in range(1, len(parts)):
                q = trusted.joinpath(*parts[:k])
                if os.path.islink(q) or os.path.isjunction(q):
                    bad.append(f"{p} (under the reparse point {'/'.join(parts[:k])})")
                    break
    if bad:
        refuse("the change touches paths no harvest may carry: " + "; ".join(bad[:10]))
    return paths


def harvest(main, item, scratch):
    """Snapshot the scratch worktree's content as a git tree, trusting nothing inside it.
    Returns (base, tree, changed paths)."""
    gd = verify_scratch(main, scratch)
    rp = reparse_points(scratch)
    if rp:
        refuse("the scratch worktree contains symlinks/junctions (refusing to read or write through "
               "them): " + ", ".join(rp[:10]))
    base = load_state(main, item).get("base")
    if not base:
        refuse(f"no recorded base for {item} (run `prepare`)")
    idx = main / "tmp" / "codex" / f"harvest-{item}.idx"
    idx.parent.mkdir(parents=True, exist_ok=True)
    idx.unlink(missing_ok=True)
    env = clean_env(GIT_DIR=str(gd), GIT_WORK_TREE=str(scratch), GIT_INDEX_FILE=str(idx))
    try:
        git(*HARVEST_CFG, "read-tree", base, cwd=scratch, env=env)
        # tmp/ is gitignored (naming it in an exclude pathspec makes `git add` fail once it exists);
        # a .gitignore edit that un-ignores it is refused by check_tree, and `commit` refuses tmp/.
        git(*HARVEST_CFG, "add", "-A", "--", ".", ":(exclude)metrics/events.jsonl", cwd=scratch, env=env)
        tree = git(*HARVEST_CFG, "write-tree", cwd=scratch, env=env)
    finally:
        idx.unlink(missing_ok=True)
    return base, tree, check_tree(main, item, base, tree)


def sync_trusted_to_tree(main, trusted, tree):
    """Make the trusted worktree's files == tree (HEAD unchanged); drop untracked leftovers in the
    data dirs (no `git clean`: it can follow junctions)."""
    git("read-tree", "-u", "--reset", tree, cwd=trusted)
    for p in git("ls-files", "--others", "--exclude-standard", "-z", "--", "src", "include", "asm",
                 "memory/grind", cwd=trusted).split("\0"):
        if p:
            (trusted / p).unlink(missing_ok=True)


def reset_trusted(main, trusted):
    git("reset", "-q", "--hard", "HEAD", cwd=trusted)
    sync_trusted_to_tree(main, trusted, "HEAD")


# ---------------------------------------------------------------- shadow (Codex's build door)

def check_engine_args(args):
    if not args:
        refuse("usage: shadow <item> make | <engine-subcommand> [args]")
    cmd = args[0]
    if cmd == "make":
        if len(args) != 1:
            refuse("`make` takes no arguments here (make variables can run commands)")
    elif cmd not in ENGINE_READONLY:
        refuse(f"'{cmd}' is not allowed (make, {', '.join(ENGINE_READONLY)})")
    elif cmd == "queue" and (len(args) < 2 or args[1] not in ("status", "next")):
        refuse("only `queue status` / `queue next`")
    elif cmd == "layer2" and (len(args) < 2 or args[1] not in ("hash", "check")):
        refuse("only `layer2 hash` / `layer2 check`")
    for a in args:
        if not ARG_OK.match(a) or ".." in a or re.match(r"^/|^[A-Za-z]:", a):
            refuse(f"argument {a!r}: plain relative tokens only")


def pin_file(main):
    return main / "tmp" / "codex" / "active-run.json"


def pid_alive(pid):
    r = subprocess.run(["tasklist", "/FI", f"PID eq {int(pid)}", "/NH", "/FO", "CSV"], capture_output=True,
                       text=True, env=clean_env())
    return any(len(c) > 1 and c[1] == str(int(pid)) for c in
               (l.strip('"').split('","') for l in r.stdout.splitlines()))


def cmd_shadow(a):
    main = main_checkout()
    if a.from_pin:
        f = pin_file(main)
        if not f.exists():
            refuse("no Codex run is live")
        pin = json.loads(f.read_text(encoding="utf-8"))
        if not pid_alive(pin["pid"]):
            refuse("the run that wrote the pin is gone (stale pin)")
        item = pin["item"]
    else:
        item = a.item
    check_engine_args(a.args)
    main, scratch, trusted, branch = layout(item)
    verify_trusted(main, trusted, branch)
    base, tree, paths = harvest(main, item, scratch)
    bad = [p for p in paths if not DATA_ONLY.match(p)]
    if bad:
        refuse("this change touches files outside src/ include/ asm/, which the build would execute: "
               + ", ".join(bad[:10]) + ". Only Claude builds such items.")
    sync_trusted_to_tree(main, trusted, tree)
    if "--candidate" in a.args:
        i = a.args.index("--candidate")
        rel = a.args[i + 1] if i + 1 < len(a.args) else ""
        src = scratch / rel
        if not rel.startswith("tmp/") or Path(rel).suffix not in (".c", ".h", ".s"):
            refuse("--candidate must be a .c/.h/.s file under tmp/")
        st = os.lstat(src) if os.path.lexists(src) else None
        if not st or not os.path.isfile(src) or os.path.islink(src) or st.st_nlink != 1 or st.st_size > 2 << 20:
            refuse(f"--candidate {rel}: not a plain regular file in the scratch worktree")
        dst = trusted / rel
        dst.parent.mkdir(parents=True, exist_ok=True)
        dst.write_bytes(src.read_bytes())
    print(f"codex_worker: building scratch tree {tree[:9]} ({len(paths)} changed file(s)) in {trusted}",
          flush=True)
    r = subprocess.run(["pwsh", "-NoProfile", "-File", str(main / "tools" / "wteng.ps1"), str(trusted),
                        *a.args], env=clean_env())
    git("checkout", "--", "metrics/events.jsonl", cwd=trusted, check=False)
    sys.exit(r.returncode)


# ---------------------------------------------------------------- Codex runs

PREAMBLE = """PROJECT RULES (Bushido Blade 2 matching decomp; non-negotiable):
- Your ONLY workspace is {scratch}. Edit files there and nowhere else. Never touch the main checkout
  {main} or any other directory under {root}.
- Do NOT run git commands that write (commit, add, stash, checkout, reset, branch...). Leave your change
  as plain edits; Claude harvests, reviews and commits it.
- Build and score ONLY with this exact shape, ONE command per shell call (no `;`, `&&`, pipes,
  redirection or variables, or it will not be allowed out of the sandbox):
    pwsh -NoProfile -File "{eng}" make
    pwsh -NoProfile -File "{eng}" sandbox <func> --disable all [--diff] [--candidate tmp/<file>.c]
  (also: canonical, diagnose, dossier, verify-oracle, tus-check, test, fixtures-verify, queue status).
  It copies your src/ include/ asm/ changes into a separate build tree and builds there. It refuses if
  you changed anything outside src/ include/ asm/: then say the build is Claude's and stop building.
  Never call tools/wteng.ps1, make, wsl or the WSL bridge yourself. If it refuses, report why.
- Do not create symlinks or junctions. Build files (src/**/*.c, *.h, *.s) must be LF.
- Never start the Grinder; never run queue regen/done/rotate/reopen.
- Write your report to {scratch}\\tmp\\codex\\report-{item}.md AND end with it as your final message:
  what changed, files touched, verification (build result vs SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa,
  scores), open questions.
- If the task needs a file you were told not to touch, or anything outside your workspace: STOP and report.

"""

RULES_FILE = Path.home() / ".codex" / "rules" / "bb2-codex-eng.rules"
RULES = """# Bushido Blade 2 decomp: the one build/score door for a sandboxed Codex worker.
# Installed by tools/codex_worker.py. tools/codex_eng.ps1 -> `codex_worker.py shadow --from-pin`
# builds only the live run's harvested src/include/asm change, in a tree the sandbox cannot write.
prefix_rule(
    pattern = [["pwsh", "pwsh.exe"], "-NoProfile", "-File", "{eng}"],
    decision = "allow",
)
"""


def eng_path(main):
    return (main / "tools" / "codex_eng.ps1").as_posix()


def ensure_rules(main):
    want = RULES.format(eng=eng_path(main))
    if not RULES_FILE.exists() or RULES_FILE.read_text(encoding="utf-8") != want:
        RULES_FILE.parent.mkdir(parents=True, exist_ok=True)
        RULES_FILE.write_text(want, encoding="utf-8", newline="\n")
        print(f"codex_worker: installed {RULES_FILE}")


# tools/wsl_bridge.ps1's daemon runs any request file in %TEMP%/bb2_wsl_bridge in WSL as the user.
# Codex's sandbox may write %TEMP%: strip every Codex sandbox SID (~/.codex/cap_sid) from that dir,
# with inheritance cut, then prove the lockdown from inside a real sandbox.
HARDEN_PS = r"""
$ErrorActionPreference = 'Stop'
$d = Join-Path ([IO.Path]::GetTempPath()) 'bb2_wsl_bridge'
New-Item -ItemType Directory -Force -Path $d | Out-Null
$j = Get-Content (Join-Path $env:USERPROFILE '.codex\cap_sid') -Raw | ConvertFrom-Json
$sids = @($j.workspace, $j.readonly)
foreach ($k in 'workspace_by_cwd', 'writable_root_by_path') {
    if ($j.$k) { $sids += @($j.$k.PSObject.Properties.Value) } }
$di = [IO.DirectoryInfo]$d
$acl = [IO.FileSystemAclExtensions]::GetAccessControl($di, [Security.AccessControl.AccessControlSections]::Access)
$acl.SetAccessRuleProtection($true, $true)
foreach ($r in @($acl.GetAccessRules($true, $true, [Security.Principal.SecurityIdentifier]))) {
    if ($sids -contains $r.IdentityReference.Value) { [void]$acl.RemoveAccessRuleSpecific($r) } }
[IO.FileSystemAclExtensions]::SetAccessControl($di, $acl)
"""
PROBE_PS = r"""
function Probe([string]$tag, [string]$dir) {
    if (-not $dir -or -not (Test-Path $dir)) { return "$tag-NOPATH" }
    try { [IO.File]::WriteAllText((Join-Path $dir 'codex_probe.tmp'), 'x'); "$tag-WRITABLE" }
    catch [UnauthorizedAccessException] { "$tag-DENIED" }
    catch { "$tag-ERROR $($_.Exception.GetType().Name)" }
}
Probe 'BRIDGE' (Join-Path ([IO.Path]::GetTempPath()) 'bb2_wsl_bridge')
Probe 'MAIN' (Join-Path "$env:BB2_MAIN" 'tmp')
Probe 'MAINGIT' (Join-Path "$env:BB2_MAIN" '.git')
Probe 'TRUSTED' "$env:BB2_TRUSTED"
Probe 'TRUSTEDBUILD' (Join-Path "$env:BB2_TRUSTED" 'build')
Probe 'WORKTREES' (Split-Path "$env:BB2_TRUSTED")
$o = (& wsl.exe -e true 2>&1 | Out-String) -replace "`0", ''
if ($LASTEXITCODE -eq 0) { 'WSL-OPEN' } elseif ($o -match 'E_ACCESSDENIED') { 'WSL-DENIED' } else { "WSL-ERROR $o" }
"""
PROBES = ("BRIDGE", "MAIN", "MAINGIT", "TRUSTED", "TRUSTEDBUILD", "WORKTREES", "WSL")


def sandbox_preflight(main, scratch, trusted):
    """Close the bridge dir, then prove from inside a real Codex sandbox (cwd = scratch) that it can
    write none of: the bridge dir, the main checkout or its .git, the trusted worktree or its build/,
    the worktrees root; nor start WSL except through codex_eng.ps1."""
    def ps(script, sandboxed):
        with tempfile.NamedTemporaryFile("w", suffix=".ps1", delete=False, encoding="utf-8") as f:
            f.write(script)
        cmd = ["pwsh", "-NoProfile", "-File", f.name]
        if sandboxed:
            sys.path.insert(0, str(BRIDGE.parent))
            import codex_bridge
            cmd = [codex_bridge.find_exe(), "sandbox", "-P", ":workspace", "-C", str(scratch), "--"] + cmd
        try:
            return subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="replace",
                                  env=clean_env(BB2_MAIN=str(main), BB2_TRUSTED=str(trusted)))
        finally:
            os.unlink(f.name)
    r = ps(HARDEN_PS, False)
    if r.returncode != 0:
        refuse("could not protect %TEMP%/bb2_wsl_bridge: " + (r.stderr or r.stdout).strip()[-400:])
    r = ps(PROBE_PS, True)
    for f in (Path(tempfile.gettempdir()) / "bb2_wsl_bridge" / "codex_probe.tmp", main / "tmp" / "codex_probe.tmp",
              main / ".git" / "codex_probe.tmp", trusted / "codex_probe.tmp", trusted / "build" / "codex_probe.tmp",
              trusted.parent / "codex_probe.tmp"):
        f.unlink(missing_ok=True)
    for tag in PROBES:
        if f"{tag}-DENIED" not in r.stdout:
            refuse(f"the Codex sandbox is not denied {tag}; probe said: {(r.stdout + r.stderr).strip()[-500:]}")
    print("codex_worker: sandbox preflight OK (" + ", ".join(PROBES) + " all denied)")


class active_pin:
    """While a run is live, main/tmp/codex/active-run.json names its item and this process's pid;
    `shadow --from-pin` builds only that item, and only while this pid lives. The sandbox cannot
    write main (preflight), so the agent cannot redirect it."""
    def __init__(self, main, item):
        self.f = pin_file(main)
        self.item = item

    def __enter__(self):
        self.f.parent.mkdir(parents=True, exist_ok=True)
        self.f.write_text(json.dumps({"item": self.item, "pid": os.getpid()}), encoding="utf-8")

    def __exit__(self, *exc):
        self.f.unlink(missing_ok=True)


def latest_run(item):
    runs = Path.home() / ".claude" / "codex-bridge" / "runs"
    best = None
    for m in runs.glob("*/meta.json"):
        try:
            meta = json.loads(m.read_text(encoding="utf-8"))
        except Exception:
            continue
        label = meta.get("label", "")
        if re.fullmatch(rf"codex-{re.escape(item)}(-followup)*", label) and meta.get("thread_id"):
            if best is None or meta.get("started", "") > best.get("started", ""):
                best = meta
    return best


def launch(main, item, scratch, trusted, cmd):
    ensure_rules(main)
    sandbox_preflight(main, scratch, trusted)
    head_before = registry(main)[norm(scratch)]["head"]
    print("codex_worker: " + " ".join(cmd[1:]), flush=True)
    with active_pin(main, item):
        rc = subprocess.run(cmd, env=clean_env()).returncode
    v = audit(main, item, scratch, trusted, head_before)
    sys.exit(4 if v else (0 if rc == 0 else 3))


def write_prompt(text):
    with tempfile.NamedTemporaryFile("w", suffix=".md", delete=False, encoding="utf-8",
                                     dir=str(main_checkout() / "tmp" / "codex")) as f:
        f.write(text)
        return f.name


def cmd_run(a):
    main, scratch, trusted, branch = prepare(a.item)
    body = Path(a.prompt_file).read_text(encoding="utf-8")
    pf = write_prompt(PREAMBLE.format(scratch=scratch, main=main, root=scratch.parent, item=a.item,
                                      eng=eng_path(main)) + "TASK:\n" + body)
    try:
        launch(main, a.item, scratch, trusted,
               [sys.executable, str(BRIDGE), "run", "--prompt-file", pf, "--cd", str(scratch),
                "--sandbox", "workspace-write", "--label", f"codex-{a.item}", "--by", "codex-worker",
                "--effort", a.effort, "--timeout", str(a.timeout)])
    finally:
        os.unlink(pf)


def cmd_follow(a):
    main, scratch, trusted, branch = prepare(a.item)
    prev = latest_run(a.item)
    if not prev:
        refuse(f"no previous Codex run for {a.item}; use `run`")
    if norm(prev.get("cwd", "")) != norm(scratch):
        refuse(f"run {prev['run_id']} ran in {prev.get('cwd')}, not {scratch}")
    note = (f"(Same rules as before: work only in {scratch}, no git writes; build/score only with "
            f'pwsh -NoProfile -File "{eng_path(main)}" <cmd>, one command per call.)\n\n')
    pf = write_prompt(note + Path(a.prompt_file).read_text(encoding="utf-8"))
    try:
        launch(main, a.item, scratch, trusted,
               [sys.executable, str(BRIDGE), "follow-up", prev["run_id"], "--prompt-file", pf,
                "--by", "codex-worker", "--effort", a.effort, "--timeout", str(a.timeout)])
    finally:
        os.unlink(pf)


def audit(main, item, scratch, trusted, head_before=None):
    v = []
    verify_scratch(main, scratch)
    if head_before and registry(main)[norm(scratch)]["head"] != head_before:
        v.append("the scratch worktree's registered HEAD moved")
    rp = reparse_points(scratch)
    if rp:
        v.append("symlinks/junctions in the scratch worktree: " + ", ".join(rp[:10]))
        print("  ! VIOLATION: " + v[-1])
        return v
    base, tree, paths = harvest(main, item, scratch)
    print(f"== scratch change vs base {base[:9]}: {len(paths)} file(s), tree {tree[:9]}")
    print(git("diff", "--stat", base, tree, cwd=main) or "  (none)")
    for p in paths:
        if p.startswith("tmp/"):
            v.append(f"tmp/ file in the change: {p}")
    behind = git("rev-list", "--count", f"{base}..main", cwd=main)
    if behind != "0":
        print(f"  note: main is {behind} commit(s) past this item's base; `land` rebases")
    for x in v:
        print("  ! VIOLATION: " + x)
    print("== audit: " + ("CLEAN" if not v else f"{len(v)} violation(s)"))
    return v


def cmd_audit(a):
    main, scratch, trusted, branch = layout(a.item)
    sys.exit(4 if audit(main, a.item, scratch, trusted) else 0)


# ---------------------------------------------------------------- commit / reviews

def single_commit(main, branch):
    base = git("merge-base", "main", branch, cwd=main)
    n = int(git("rev-list", "--count", f"{base}..{branch}", cwd=main))
    if n != 1:
        refuse(f"{branch} must carry exactly ONE commit on top of main (has {n})")
    return base, git("rev-parse", branch, cwd=main)


def diff_key(main, rev):
    """Review key of a commit's change: its diff vs its parent with 3 lines of context, whitespace-
    exact (patch-id would not be), binary content and file modes included; only blob ids and hunk
    line numbers dropped. A clean rebase keeps the key unless main changed the hunks' context; a
    hand-resolved conflict invalidates every earlier verdict (cmd_rebase)."""
    d = subprocess.run(["git", "diff", "--no-renames", "--no-color", "--binary", "--full-index", "-U3",
                        f"{rev}^", rev], cwd=str(main), capture_output=True, env=clean_env()).stdout
    keep, skip = [], False
    for line in d.split(b"\n"):
        if line.startswith(b"diff --git "):
            m = re.match(rb"^diff --git a/(.*) b/(.*)$", line)
            skip = bool(m) and m.group(1) == m.group(2) and bool(LEDGER.match(m.group(2).decode("utf-8", "replace")))
        if skip:
            continue
        if line.startswith(b"index "):
            continue
        if line.startswith(b"@@"):                    # keep the context, drop only line numbers
            line = re.sub(rb"^@@ -\d+(,\d+)? \+\d+(,\d+)? @@", b"@@ @@", line)
        keep.append(line)
    return hashlib.sha256(b"\n".join(keep)).hexdigest()[:24]


def required_kinds(main, rev):
    paths = [p for p in git("diff", "--no-renames", "--name-only", "-z", f"{rev}^", rev, cwd=main).split("\0")
             if p and not LEDGER.match(p)]
    kinds = set()
    for p in paths:
        if BOTH_PATHS.match(p):
            kinds.update(KINDS)
        elif CHEAT_PATHS.match(p):
            kinds.add("cheat-reviewer")
        else:
            kinds.add("code-reviewer")
    return sorted(kinds), paths


def format_tree(main, item, tree, paths):
    """`tree` with every changed src/include C file in the repo's C style (tools/format.py,
    token-preserving: the build and layer-2 keys cannot move). Blob in, blob out: nothing in the
    scratch tree is read or executed."""
    spec = importlib.util.spec_from_file_location("bb2_format", main / "tools" / "format.py")
    fmt = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(fmt)
    texts, modes = {}, {}
    for p in paths:
        if not (p.endswith((".c", ".h")) and p.startswith(("src/", "include/"))):
            continue
        entry = git("ls-tree", tree, "--", p, cwd=main)
        if not entry:
            continue  # deleted
        mode = entry.split(" ", 1)[0]
        if mode not in ("100644", "100755"):
            refuse(f"{p} is not a regular file in the harvested tree (mode {mode})")
        raw = subprocess.run(["git", "cat-file", "blob", f"{tree}:{p}"], cwd=str(main), capture_output=True,
                             check=True, env=clean_env()).stdout
        try:
            texts[p] = raw.decode("utf-8")
        except UnicodeDecodeError:
            refuse(f"{p} is not UTF-8")
        modes[p] = mode
    changed = []
    results = fmt.format_blobs(texts, env=clean_env())
    for p in texts:
        status, new = results.get(p, ("error", "no result returned"))
        if status == "error":
            refuse(f"tools/format.py cannot format {p}: {new}")
        if status == "tokens":
            refuse(f"tools/format.py would change C tokens in {p}; not committing")
        if new != texts[p]:
            sha = subprocess.run(["git", "hash-object", "-w", "--stdin"], cwd=str(main), input=new.encode("utf-8"),
                                 capture_output=True, check=True, env=clean_env()).stdout.decode().strip()
            changed.append((modes[p], sha, p))
    if not changed:
        return tree
    idx = main / "tmp" / "codex" / f"format-{item}.idx"
    idx.unlink(missing_ok=True)
    env = clean_env(GIT_INDEX_FILE=str(idx))
    try:
        git("read-tree", tree, cwd=main, env=env)
        for mode, sha, p in changed:
            git("update-index", "--cacheinfo", f"{mode},{sha},{p}", cwd=main, env=env)
        return git("write-tree", cwd=main, env=env)
    finally:
        idx.unlink(missing_ok=True)


def cmd_commit(a):
    """Harvest the scratch change into THE item commit (parent = the item's base; replaces any
    previous one). No hooks run here: the content is unreviewed. `land` re-runs the repo hooks on
    the reviewed commit, in the trusted tree."""
    main, scratch, trusted, branch = prepare(a.item, quiet=True)
    if pin_file(main).exists():
        refuse("a Codex run is live")
    msg = Path(a.message_file).resolve()
    if not msg.is_file():
        refuse(f"no message file {msg}")
    base, tree, paths = harvest(main, a.item, scratch)
    if not paths:
        refuse("the scratch worktree has no change")
    bad = [p for p in paths if p.startswith("tmp/")]
    if bad:
        refuse("tmp/ files in the change: " + ", ".join(bad))
    verify_trusted(main, trusted, branch)
    harvested = tree
    tree = format_tree(main, a.item, tree, paths)
    if tree != harvested:
        codex_idle(main, scratch)  # scratch is re-created below; check before the branch moves
    c = git("commit-tree", tree, "-p", base, "-F", str(msg), cwd=main)
    git("update-ref", f"refs/heads/{branch}", c, cwd=main)
    reset_trusted(main, trusted)
    if tree != harvested:
        # scratch must hold what the item commit holds (scratch_committed)
        recreate_scratch(main, a.item, scratch, c)
        print("codex_worker: C sources formatted (tools/format.py); scratch re-created at the commit")
    print(git("show", "--stat", "--format=%h %s", c, cwd=main))
    kinds, _ = required_kinds(main, c)
    if not kinds:
        refuse("the change is only layer-2 ledger lines; nothing to review or land")
    print(f"codex_worker: {branch} = {c[:9]} (review key {diff_key(main, c)}); reviewers required: "
          + ", ".join(kinds))


def cmd_layer2(a):
    """Record the cheat-reviewer's layer-2 verdict (completion-bar item 6) for a function whose body
    the item changes, in the TRUSTED tree on the reviewed body, and fold the ledger line into the item
    commit. The review key ignores the ledger, so the recorded reviews stay valid."""
    main, scratch, trusted, branch = layout(a.item)
    if not (TOKEN.match(a.func) and TOKEN.match(a.reviewer)):
        refuse("func / reviewer must be plain identifiers")
    if pin_file(main).exists():
        refuse("a Codex run is live")
    verify_trusted(main, trusted, branch)
    codex_idle(main, scratch)
    scratch_committed(main, a.item, scratch, branch)
    base, tip = single_commit(main, branch)
    key = diff_key(main, tip)
    f = reviews_file(main, a.item)
    recs = [json.loads(l) for l in f.read_text(encoding="utf-8").splitlines() if l.strip()] if f.exists() else []
    cut = max((i for i, r in enumerate(recs) if r.get("kind") == "__invalidated__"), default=-1)
    if not any(r.get("key") == key and r["kind"] == "cheat-reviewer" and r["verdict"] == "PASS"
               and r["agent"] == a.reviewer for r in recs[cut + 1:]):
        refuse(f"no cheat-reviewer PASS by {a.reviewer} recorded on this diff (key {key}); `review` it first")
    vf = Path(a.verdict_file).resolve()
    text = vf.read_text(encoding="utf-8")
    verdict = None
    for cand in [text] + re.findall(r"\{.*\}", text, re.S):   # reviewers may wrap the JSON in prose
        try:
            verdict = json.loads(cand)
            break
        except ValueError:
            continue
    if not isinstance(verdict, dict):
        refuse(f"{vf} holds no JSON verdict")
    # the cheat-reviewer schema (and engine/layer2.read_verdict_file) carry the ruling in `decision`
    if (verdict.get("decision") != "PASS" or verdict.get("function") != a.func
            or not re.fullmatch(r"[0-9a-f]{16}", str(verdict.get("body_hash", "")))):
        refuse(f"the verdict must be decision PASS for function {a.func} with a body_hash")
    reset_trusted(main, trusted)
    rel = f"tmp/codex/verdict-{a.func}.json"
    (trusted / rel).parent.mkdir(parents=True, exist_ok=True)
    (trusted / rel).write_bytes(vf.read_bytes())
    ledger = f"memory/grind/{a.func}/layer2.jsonl"
    old = (trusted / ledger).read_bytes() if (trusted / ledger).exists() else b""
    run_checked(["pwsh", "-NoProfile", "-File", str(main / "tools" / "wteng.ps1"), str(trusted), "layer2",
                 "record", a.func, "--verdict-file", rel, "--reviewer", a.reviewer, "--scope", a.scope],
                f"layer2 record {a.func}")
    git("checkout", "--", "metrics/events.jsonl", cwd=trusted, check=False)
    changed = sorted(e[3:] for e in git("status", "--porcelain", "-z", "-uall", cwd=trusted).split("\0") if e)
    copy = f"memory/grind/{a.func}/layer2_verdicts/{hashlib.sha1(vf.read_bytes()).hexdigest()}.json"
    new = (trusted / ledger).read_bytes() if (trusted / ledger).exists() else b""
    extra = [c for c in changed if c not in (ledger, copy)]
    try:
        added = [json.loads(l) for l in new[len(old):].decode("utf-8").splitlines() if l.strip()]
    except ValueError:
        added = []
    if (extra or ledger not in changed or not new.startswith(old) or len(added) != 1
            or added[0].get("verdict") != "PASS" or added[0].get("body_hash") != verdict["body_hash"]
            or added[0].get("func") != a.func
            or (copy in changed and (trusted / copy).read_bytes() != vf.read_bytes())):
        reset_trusted(main, trusted)
        refuse(f"layer2 record changed more than {ledger} (+ its verdict copy): {changed}")
    git("add", "--", *[c for c in (ledger, copy) if c in changed], cwd=trusted)
    r = subprocess.run(["git", "commit", "--amend", "--no-edit", "-q"], cwd=str(trusted), capture_output=True,
                       text=True, env=clean_env())
    print((r.stdout + r.stderr).strip())
    if r.returncode != 0:
        refuse("the repo hooks rejected the amend (output above)")
    tip2 = git("rev-parse", "HEAD", cwd=trusted)
    if diff_key(main, tip2) != key:
        refuse("internal: the ledger amend changed the review key")
    recreate_scratch(main, a.item, scratch, tip2)
    print(f"codex_worker: layer-2 PASS for {a.func} recorded in {tip2[:9]} (review key unchanged {key}); "
          "scratch re-created at it")


def reviews_file(main, item):
    # main's tmp/: the Codex sandbox cannot write there (preflight), so it cannot forge a verdict
    return main / "tmp" / "codex" / "reviews" / f"{item}.jsonl"


def cmd_review(a):
    main, scratch, trusted, branch = layout(a.item)
    base, tip = single_commit(main, branch)
    rec = {"ts": dt.datetime.now().astimezone().isoformat(timespec="seconds"), "kind": a.kind,
           "verdict": a.verdict, "key": diff_key(main, tip), "commit": tip, "agent": a.agent,
           "summary": a.summary}
    f = reviews_file(main, a.item)
    f.parent.mkdir(parents=True, exist_ok=True)
    with f.open("a", encoding="utf-8") as fh:
        fh.write(json.dumps(rec) + "\n")
    print(f"codex_worker: recorded {a.kind} {a.verdict} for {tip[:9]} (key {rec['key']})")


def review_gate(main, item, tip):
    kinds, paths = required_kinds(main, tip)
    if not kinds:
        return kinds, paths, ["the item commit carries no reviewable change (only ledger lines)"]
    key = diff_key(main, tip)
    f = reviews_file(main, item)
    recs = [json.loads(l) for l in f.read_text(encoding="utf-8").splitlines() if l.strip()] if f.exists() else []
    cut = max((i for i, r in enumerate(recs) if r.get("kind") == "__invalidated__"), default=-1)
    mine = [r for r in recs[cut + 1:] if r.get("key") == key]
    problems = []
    fails = [r for r in mine if r["verdict"] != "PASS"]
    if fails:
        problems.append("FAIL recorded on this exact diff (a split verdict is a FAIL): "
                        + "; ".join(f"{r['kind']} {r['agent']}" for r in fails))
    missing = [k for k in kinds if not any(r["kind"] == k and r["verdict"] == "PASS" for r in mine)]
    if missing:
        problems.append("no PASS on this exact diff from: " + ", ".join(missing))
    return kinds, paths, problems


# ---------------------------------------------------------------- rebase / land

def codex_idle(main, scratch):
    """No live run, no bridge task, no codex exec/CLI process, no process naming the scratch tree."""
    if pin_file(main).exists():
        refuse("a Codex run is live")
    r = subprocess.run([sys.executable, str(BRIDGE), "status", "--json"], capture_output=True, text=True,
                       env=clean_env())
    try:
        st = json.loads(r.stdout)
    except Exception:
        refuse("cannot read codex_bridge status to confirm Codex is idle")
    busy = [p for p in st.get("processes", []) if p.get("kind") not in ("desktop-app", "service")]
    if st.get("bridge_slot", {}).get("busy") or busy:
        refuse("Codex is still running (bridge slot or codex processes); wait for it to finish")
    ps = subprocess.run(["pwsh", "-NoProfile", "-Command",
                         "Get-CimInstance Win32_Process | ForEach-Object { $_.CommandLine }"],
                        capture_output=True, text=True, env=clean_env())
    if norm(scratch).lower() in ps.stdout.lower().replace("/", "\\"):
        refuse(f"a process still references {scratch}; wait for it to exit")


def remove_scratch(main, scratch):
    """`rmdir /s /q` removes junctions without following them (unlike `git worktree remove --force`)."""
    if scratch.exists():
        codex_idle(main, scratch)
        subprocess.run(["cmd", "/c", "rmdir", "/s", "/q", str(scratch)], env=clean_env())
        if scratch.exists():
            refuse(f"could not remove {scratch}")
    git("worktree", "prune", cwd=main)


def scratch_committed(main, item, scratch, branch):
    """Refuse to replace scratch while it holds work the item commit does not."""
    if not scratch.exists():
        return
    _, tree, _ = harvest(main, item, scratch)
    if tree != git("rev-parse", f"{branch}^{{tree}}", cwd=main):
        refuse("the scratch worktree has changes the item commit does not; `commit` them first "
               "(or re-run `commit` to see the difference)")


def recreate_scratch(main, item, scratch, at):
    """Point the scratch worktree at `at` by recreating it (never write into an untrusted tree)."""
    remove_scratch(main, scratch)
    git("worktree", "add", "--detach", str(scratch), at, cwd=main)


def run_safe_remove(main, wt):
    """Junction-safe removal. safe_remove_worktree.ps1 detaches reparse points, then `git worktree
    remove`; a scratch tree whose `.git` file was tampered with defeats the latter, so fall back to
    deleting the directory -- only once no reparse point is left in it -- and pruning the registry."""
    if not wt.exists():
        return
    subprocess.run(["pwsh", "-NoProfile", "-File", str(main / "tools" / "safe_remove_worktree.ps1"),
                    str(wt), "-Force"], env=clean_env())
    if wt.exists():
        rp = reparse_points(wt)
        if rp:
            refuse(f"cannot remove {wt}: reparse points remain ({', '.join(rp[:5])}); detach them with "
                   "`cmd /c rmdir <path>` and retry")

        def onexc(fn, path, exc):
            os.chmod(path, 0o666)
            fn(path)
        shutil.rmtree(wt, onexc=onexc)
    git("worktree", "prune", cwd=main)
    if wt.exists():
        refuse(f"could not remove {wt}")


def after_rebase(main, item, scratch, trusted, branch):
    base, tip = single_commit(main, branch)
    save_state(main, item, base=base)
    recreate_scratch(main, item, scratch, tip)
    print(f"codex_worker: {branch} = {tip[:9]} on main {base[:9]} (review key {diff_key(main, tip)}); "
          f"scratch re-created at it")


def cmd_rebase(a):
    main, scratch, trusted, branch = layout(a.item)
    if pin_file(main).exists():
        refuse("a Codex run is live")
    verify_trusted(main, trusted, branch, allow_rebase=True)
    env = clean_env(GIT_EDITOR="true")
    if a.abort:
        r = subprocess.run(["git", "rebase", "--abort"], cwd=str(trusted), capture_output=True, text=True, env=env)
    elif a.cont:
        if not rebasing(trusted, branch):
            refuse("no rebase in progress")
        unmerged = [p for p in git("diff", "--name-only", "-z", "--diff-filter=U", cwd=trusted).split("\0") if p]
        marked = [p for p in unmerged if (trusted / p).exists() and re.search(
            r"^(<<<<<<<|>>>>>>>) ", (trusted / p).read_text(encoding="utf-8", errors="replace"), re.M)]
        if marked:
            refuse("conflict markers remain in: " + ", ".join(marked))
        for p in unmerged:
            if (trusted / p).exists():
                git("add", "--", p, cwd=trusted)
            else:
                git("rm", "-q", "--", p, cwd=trusted)
        r = subprocess.run(["git", "rebase", "--continue"], cwd=str(trusted), capture_output=True, text=True, env=env)
    else:
        scratch_committed(main, a.item, scratch, branch)
        reset_trusted(main, trusted)
        r = subprocess.run(["git", "rebase", "main"], cwd=str(trusted), capture_output=True, text=True, env=env)
    print((r.stdout + r.stderr).strip())
    if rebasing(trusted, branch):
        conflicted = git("diff", "--name-only", "--diff-filter=U", cwd=trusted)
        print(f"codex_worker: CONFLICT -- edit these in {trusted} (the TRUSTED worktree), then "
              f"`codex_worker.py rebase {a.item} --continue`:\n{conflicted}")
        sys.exit(6)
    if r.returncode != 0:
        refuse("rebase failed")
    if a.cont:
        # a hand-resolved conflict can move hunks into different code: no earlier verdict carries over
        f = reviews_file(main, a.item)
        f.parent.mkdir(parents=True, exist_ok=True)
        with f.open("a", encoding="utf-8") as fh:
            fh.write(json.dumps({"kind": "__invalidated__", "ts": dt.datetime.now().astimezone().isoformat(
                timespec="seconds"), "reason": "conflict resolved by hand"}) + "\n")
        print("codex_worker: earlier reviews invalidated (conflict resolution); review the new diff")
    if not a.abort:
        after_rebase(main, a.item, scratch, trusted, branch)


def run_checked(cmd, what, must_contain=None, cwd=None):
    print(f"codex_worker: {what} ...", flush=True)
    r = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, encoding="utf-8", errors="replace",
                       env=clean_env())
    out = (r.stdout or "") + (r.stderr or "")
    print("\n".join((r.stdout or "").strip().splitlines()[-3:]))
    if r.returncode != 0 or (must_contain and must_contain not in out):
        print("\n".join((r.stderr or "").strip().splitlines()[-15:]))
        refuse(f"{what} failed (exit {r.returncode})")


def wsl_path(p):
    s = str(p).replace("\\", "/")
    m = re.match(r"^([A-Za-z]):/(.*)$", s)
    return f"/mnt/{m.group(1).lower()}/{m.group(2)}" if m else s


def verify_tip(main, trusted, tip, paths):
    """Clean build of the exact commit that becomes main, in the trusted tree; SHA1 computed here."""
    if git("rev-parse", "HEAD", cwd=trusted) != tip or git("status", "--porcelain", cwd=trusted):
        refuse("the trusted worktree is not clean at the branch tip")
    marker = main / "tmp" / "codex" / "built" / f"{tip}.ok"
    if marker.exists():
        print(f"codex_worker: {tip[:9]} already built and verified")
        return
    wteng = str(main / "tools" / "wteng.ps1")
    run_checked(["pwsh", "-NoProfile", "-File", wteng, str(trusted), "make", "clean-check"],
                f"clean full build of {tip[:9]}", must_contain="OK: bb2 matches!")
    exe = trusted / "build" / "bb2.exe"
    got = hashlib.sha1(exe.read_bytes()).hexdigest() if exe.exists() else "missing"
    if got != ORACLE_SHA1:
        refuse(f"{exe} SHA1 {got} != oracle {ORACLE_SHA1}")
    print(f"codex_worker: build/bb2.exe SHA1 {got} == oracle")
    if any(not CHEAT_PATHS.match(p) and not p.startswith("docs/") for p in paths):
        run_checked(["pwsh", "-NoProfile", "-File", wteng, str(trusted), "test"], "engine test suite")
    run_checked(["wsl", "bash", "-c", f"cd '{wsl_path(trusted)}' && source .venv/bin/activate && "
                 "python3 tools/check_completion_integrity.py"], "completion-integrity audit")
    git("checkout", "--", "metrics/events.jsonl", cwd=trusted, check=False)
    if git("status", "--porcelain", cwd=trusted):
        refuse("the build/test left the trusted tree dirty: " + git("status", "--porcelain", cwd=trusted))
    marker.parent.mkdir(parents=True, exist_ok=True)
    marker.write_text("ok")


def lock(main, action, item):
    r = subprocess.run(["pwsh", "-NoProfile", "-File", str(main / "tools" / "reintegrate_lock.ps1"),
                        action, "-Label", f"codex-{item}"], capture_output=True, text=True, env=clean_env())
    out = (r.stdout + r.stderr).strip()
    if action != "status":
        print(out)
    return r.returncode == 0, out


def peer_paths(main):
    peer, ent, i = set(), git("status", "--porcelain", "-z", "-uall", cwd=main).split("\0"), 0
    while i < len(ent):
        e = ent[i]
        if len(e) > 3:
            peer.add(e[3:])
            if e[0] in "RC" or e[1] in "RC":   # rename/copy: the next entry is the source path
                i += 1
                peer.add(ent[i])
        i += 1
    return peer


def push(main):
    ahead = git("rev-list", "--count", "origin/main..main", cwd=main, check=False)
    print(f"codex_worker: pushing main to origin ({ahead} commit(s) ahead of origin/main)")
    r = subprocess.run(["git", "push", "origin", "main"], cwd=str(main), capture_output=True, text=True,
                       env=clean_env())
    print((r.stdout + r.stderr).strip())
    if r.returncode != 0:
        print("codex_worker: PUSH FAILED (main is landed locally). Never force-push; report it.")
        sys.exit(8)
    print("codex_worker: PUSHED")


def cmd_land(a):
    main, scratch, trusted, branch = layout(a.item)
    if pin_file(main).exists():
        refuse("a Codex run is live")
    if not os.environ.get("CLAUDE_CODE_SESSION_ID"):
        refuse("CLAUDE_CODE_SESSION_ID is unset: the reintegration lock could not be released")
    verify_trusted(main, trusted, branch)
    tip0 = git("rev-parse", branch, cwd=main)
    already = subprocess.run(["git", "merge-base", "--is-ancestor", tip0, "main"], cwd=str(main),
                             env=clean_env()).returncode == 0
    if already and load_state(main, a.item).get("landed") != tip0:
        refuse(f"{branch} has no commit of its own to land (run `commit` first)")
    if not already:
        scratch_committed(main, a.item, scratch, branch)
    for attempt in range(1, 6):
        if not already:
            reset_trusted(main, trusted)
            base, tip = single_commit(main, branch)
            main_tip = git("rev-parse", "main", cwd=main)
            if base != main_tip:
                print(f"codex_worker: main moved; rebasing {branch} onto {main_tip[:9]}")
                r = subprocess.run(["git", "rebase", "main"], cwd=str(trusted), capture_output=True, text=True,
                                   env=clean_env(GIT_EDITOR="true"))
                if r.returncode != 0:
                    subprocess.run(["git", "rebase", "--abort"], cwd=str(trusted), capture_output=True, env=clean_env())
                    print((r.stdout + r.stderr).strip()[-1500:])
                    print(f"codex_worker: CONFLICT rebasing onto main (aborted, trees clean). Run\n"
                          f"  python tools/codex_worker.py rebase {a.item}  -> edit the conflicted files in {trusted}\n"
                          f"  python tools/codex_worker.py rebase {a.item} --continue\n"
                          f"then land again (its review gate says whether the resolved diff needs new reviews).")
                    sys.exit(6)
                after_rebase(main, a.item, scratch, trusted, branch)
                continue
            kinds, paths, problems = review_gate(main, a.item, tip)
            if problems:
                print("codex_worker: review gate: required " + ", ".join(kinds))
                for p in problems:
                    print("  ! " + p)
                sys.exit(9)
            print(f"codex_worker: reviews OK ({', '.join(kinds)}) on {tip[:9]}")
            # The amend below re-stages nothing, so format_guard cannot see this commit's C; check
            # it here (a hand-resolved rebase conflict is the one way unformatted C gets in).
            tree0 = git("rev-parse", f"{tip}^{{tree}}", cwd=main)
            if format_tree(main, a.item, tree0, paths) != tree0:
                refuse("the commit's C is not formatted (a hand-resolved rebase conflict?): re-run "
                       "`commit` (it formats), then review the new key")
            # The repo hooks (commit-msg chain, pre-commit) run now, on the REVIEWED content, in the
            # trusted tree: re-commit the same tree and message.
            key0 = diff_key(main, tip)
            r = subprocess.run(["git", "commit", "--amend", "--no-edit", "-q"], cwd=str(trusted),
                               capture_output=True, text=True, env=clean_env())
            print((r.stdout + r.stderr).strip())
            if r.returncode != 0:
                refuse("the repo hooks rejected the reviewed commit (output above)")
            tip = git("rev-parse", "HEAD", cwd=trusted)
            if diff_key(main, tip) != key0:
                refuse("the hook re-commit changed the diff -- investigate")
            verify_tip(main, trusted, tip, paths)
            if a.dry_run:
                print(f"codex_worker: DRY RUN -- {tip[:9]} is ready to fast-forward main; stopping here")
                return
        else:
            tip, paths = tip0, []
            print(f"codex_worker: {tip[:9]} is already on main; pushing only")
        _, st = lock(main, "status", a.item)
        held_before = "(yours)" in st
        ok, _ = lock(main, "acquire", a.item)
        if not ok:
            print("codex_worker: the main-reintegration lock is held by another live session; land again "
                  "later (never steal a fresh lock)")
            sys.exit(5)
        try:
            if git("symbolic-ref", "-q", "--short", "HEAD", cwd=main, check=False) != "main":
                refuse("the main checkout is not on branch main")
            if not already:
                if git("rev-parse", "main", cwd=main) != main_tip:
                    print("codex_worker: main moved while taking the lock; retrying")
                    continue
                names = {n for n in git("diff", "--name-only", "-z", "--no-renames", main_tip, tip,
                                        cwd=main).split("\0") if n}
                overlap = sorted(peer_paths(main) & names)
                if overlap:
                    print("codex_worker: a peer session has uncommitted work on paths this commit changes "
                          "in the main checkout: " + ", ".join(overlap) + ". Land again once they commit.")
                    sys.exit(7)
                r = subprocess.run(["git", "merge", "--ff-only", tip], cwd=str(main), capture_output=True,
                                   text=True, env=clean_env())
                if r.returncode != 0:
                    if git("rev-parse", "main", cwd=main) != main_tip:
                        print("codex_worker: main moved during the merge; retrying")
                        continue
                    refuse("fast-forward of main failed: " + (r.stderr or r.stdout).strip())
                if git("rev-parse", "HEAD", cwd=main) != tip:
                    refuse("main is not at the verified commit after the fast-forward -- investigate")
                save_state(main, a.item, landed=tip)
                print(f"codex_worker: LANDED {git('log', '--oneline', '-1', tip, cwd=main)}")
            if not a.no_push:
                push(main)
            return
        finally:
            if not held_before:
                lock(main, "release", a.item)
    refuse("main kept moving; gave up after 5 attempts -- land again")


def cmd_cleanup(a):
    main, scratch, trusted, branch = layout(a.item)
    if pin_file(main).exists():
        refuse("a Codex run is live")
    tip = git("rev-parse", "--verify", "-q", branch, cwd=main, check=False)
    if tip and subprocess.run(["git", "merge-base", "--is-ancestor", tip, "main"], cwd=str(main),
                              env=clean_env()).returncode != 0 and not a.drop:
        refuse(f"{branch} is not on main; pass --drop only if the owner agreed to discard it")
    outside_main(main, trusted)
    outside_main(main, scratch)
    run_safe_remove(main, trusted)
    remove_scratch(main, scratch)
    if tip:
        git("branch", "-D", branch, cwd=main)
    state_file(main, a.item).unlink(missing_ok=True)
    print(f"codex_worker: removed {scratch}, {trusted} and {branch}")


# ---------------------------------------------------------------- cli

def main():
    os.chdir(Path(__file__).resolve().parent.parent)   # never run from a caller's (scratch) cwd
    ap = argparse.ArgumentParser(prog="codex_worker", description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sp = ap.add_subparsers(dest="cmd", required=True)

    def item(p):
        p.add_argument("item")
        return p
    item(sp.add_parser("prepare"))
    for name in ("run", "follow-up"):
        r = item(sp.add_parser(name))
        r.add_argument("--prompt-file", required=True)
        r.add_argument("--effort", default="medium", choices=["low", "medium"])
        r.add_argument("--timeout", type=int, default=30, help="minutes")
    item(sp.add_parser("audit"))
    s = sp.add_parser("shadow")
    s.add_argument("--from-pin", action="store_true", help="(codex_eng.ps1) the live run's item")
    s.add_argument("args", nargs=argparse.REMAINDER)
    m = item(sp.add_parser("commit"))
    m.add_argument("--message-file", required=True)
    rb = item(sp.add_parser("rebase"))
    rb.add_argument("--continue", dest="cont", action="store_true")
    rb.add_argument("--abort", action="store_true")
    v = item(sp.add_parser("review"))
    v.add_argument("--kind", required=True, choices=KINDS)
    v.add_argument("--verdict", required=True, choices=["PASS", "FAIL"])
    v.add_argument("--agent", required=True, help="the reviewer agent's id")
    v.add_argument("--summary", required=True)
    l2 = item(sp.add_parser("layer2"))
    l2.add_argument("func")
    l2.add_argument("--verdict-file", required=True, help="the cheat-reviewer's JSON verdict (PASS + body_hash)")
    l2.add_argument("--reviewer", required=True, help="the agent id recorded with `review`")
    l2.add_argument("--scope", required=True, choices=["match", "cheat-cleanup", "auth"])
    l = item(sp.add_parser("land"))
    l.add_argument("--dry-run", action="store_true")
    l.add_argument("--no-push", action="store_true")
    c = item(sp.add_parser("cleanup"))
    c.add_argument("--drop", action="store_true", help="discard an UNLANDED branch (owner-approved only)")
    a = ap.parse_args()
    if a.cmd == "shadow":
        if a.from_pin:
            a.item = None
        else:
            if not a.args:
                refuse("usage: shadow <item> <make | engine-subcommand ...>")
            a.item, a.args = a.args[0], a.args[1:]
    try:
        {"prepare": lambda: prepare(a.item), "run": lambda: cmd_run(a), "follow-up": lambda: cmd_follow(a),
         "audit": lambda: cmd_audit(a), "shadow": lambda: cmd_shadow(a), "commit": lambda: cmd_commit(a),
         "rebase": lambda: cmd_rebase(a), "layer2": lambda: cmd_layer2(a), "review": lambda: cmd_review(a), "land": lambda: cmd_land(a),
         "cleanup": lambda: cmd_cleanup(a)}[a.cmd]()
    except RuntimeError as e:
        print(f"codex_worker: {e}", file=sys.stderr)
        sys.exit(2)


if __name__ == "__main__":
    main()

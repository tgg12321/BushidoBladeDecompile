"""engine CLI — Phase 0 surface.

  python3 -m engine.cli oracle-lock            capture oracle/manifest.json
  python3 -m engine.cli verify-oracle [--rebuild]
                                               confirm byte-identical build + report drift
  python3 -m engine.cli build                  full clean-driver build -> SHA1 check
  python3 -m engine.cli build-c <stem>         build one C object into build/src/
  python3 -m engine.cli parity [--out D --ref D]
                                               build all C objects with the driver and
                                               byte-compare vs a reference build dir

Run from the repo root, inside WSL.
"""
from __future__ import annotations

import argparse
import json
import sys

from . import buildconfig as cfg
from . import fixtures as F
from . import oracle as O
from . import pipeline as P


def main() -> int:
    ap = argparse.ArgumentParser(prog="engine")
    sub = ap.add_subparsers(dest="cmd", required=True)

    olp = sub.add_parser("oracle-lock", help="capture oracle/manifest.json")
    olp.add_argument("--commit", default=None,
                     help="record this git commit (WSL git can't read the worktree)")
    vp = sub.add_parser("verify-oracle", help="confirm byte-identical build + drift")
    vp.add_argument("--rebuild", action="store_true", help="force a fresh full build")
    sub.add_parser("build", help="full clean-driver build -> SHA1 check")
    bp = sub.add_parser("build-c", help="build one C object")
    bp.add_argument("stem")
    pp = sub.add_parser("parity", help="byte-compare driver C objects vs a reference build")
    pp.add_argument("--out", default="tmp/parity")
    pp.add_argument("--ref", default="build")
    fap = sub.add_parser("fixtures-add", help="record golden fixtures by function name")
    fap.add_argument("names", nargs="+")
    sub.add_parser("fixtures-verify", help="check golden fixtures against the build")

    a = ap.parse_args()

    if a.cmd == "oracle-lock":
        m = O.lock(git_commit=a.commit)
        print(f"locked oracle -> {O.MANIFEST}")
        print(f"  original sha1   {m['original_exe']['sha1']}  ({m['original_exe']['size']} bytes)")
        print(f"  expected build  {m['expected_build_sha1']}")
        print(f"  git commit      {m['git_commit']}")
        print(f"  corpus files    {len(m['corpus'])}")
        print(f"  config files    {len(m['config_files'])}")
        print(f"  golden fixtures {len(m['golden_fixtures'])}")
        return 0

    if a.cmd == "verify-oracle":
        r = O.verify(rebuild=a.rebuild)
        print(json.dumps(r, indent=2))
        return 0 if r.get("ok") else 1

    if a.cmd == "build":
        exe = P.build_all()
        got = P.sha1(exe)
        ok = got == cfg.ORACLE_SHA1
        print(f"built {exe}\n  sha1 {got}\n  want {cfg.ORACLE_SHA1}\n  {'MATCH' if ok else 'MISMATCH'}")
        return 0 if ok else 1

    if a.cmd == "build-c":
        out_o = f"build/src/{a.stem}.o"
        P.build_c_object(a.stem, out_o)
        print(f"built {out_o}  sha1 {P.sha1(out_o)}")
        return 0

    if a.cmd == "parity":
        res = P.parity_objects(a.out, a.ref)
        nmatch = sum(1 for _, s, _ in res if s == "MATCH")
        for stem, status, info in res:
            if status != "MATCH":
                print(f"  {status:6} {stem}  {info or ''}")
        print(f"parity: {nmatch}/{len(res)} C objects byte-identical to {a.ref}/")
        return 0 if nmatch == len(res) else 1

    if a.cmd == "fixtures-add":
        fx = F.add(a.names)
        print(f"golden fixtures ({len(fx)}):")
        for f in fx:
            print(f"  {f['vram']}  {f['size']:>5}B  {f['file']:<14} {f['name']}  {f['text_sha1'][:12]}")
        return 0

    if a.cmd == "fixtures-verify":
        res = F.verify()
        for name, status in res:
            print(f"  {status:8} {name}")
        ok = all(s == "OK" for _, s in res)
        print(f"fixtures: {sum(1 for _, s in res if s == 'OK')}/{len(res)} OK")
        return 0 if ok else 1

    return 2


if __name__ == "__main__":
    sys.exit(main())

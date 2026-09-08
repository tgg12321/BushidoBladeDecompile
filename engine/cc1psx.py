"""cc1psx self-disproof (owner ruling 2026-09-08, rotation-not-foreclosure ruling 2).

Compile a function's current candidate body under BOTH compilers — our rebuilt
cc1 and PsyQ's original cc1psx (tools/cc1psx_wrapper.sh, via dosemu2) — out of
tree, and score each object against the oracle build object with the engine's
own metric. If cc1psx lands strictly closer to the target than our cc1, the
residual is a compiler-FIDELITY lead (a flag/config/patch we have not
replicated), not a spelling problem, and the driver must not rotate the
function on "policy" grounds. If cc1psx is no closer, the residual is
source-side: the original compiler makes the same choice from this source,
so a pure-C preimage with a different spelling exists.

cc1psx is a calibration/self-disproof tool ONLY (.claude/rules/no-compiler-divergence.md,
[[cc1psx-calibration-only]]); it is never a build path.

The candidate defaults to memory/grind/<func>/candidate.c and is spliced over
the function's INCLUDE_ASM line (or its current definition) in a scratch copy
of src/<stem>.c. The tree is never modified.
"""
from __future__ import annotations

import hashlib
import re
import subprocess
from pathlib import Path

from . import buildconfig as cfg
from . import pipeline
from . import score
from .sandbox import func_file

CC1PSX_WRAPPER = "tools/cc1psx_wrapper.sh"


def _splice(tu_text: str, func: str, body: str) -> str:
    inc = re.search(r'(?m)^[ \t]*INCLUDE_ASM\s*\(\s*"asm/funcs"\s*,\s*'
                    + re.escape(func) + r'\s*\)\s*;[ \t]*\n?', tu_text)
    if inc:
        return tu_text[:inc.start()] + body.rstrip("\n") + "\n" + tu_text[inc.end():]
    # fall back to the current definition span (sweep_variants' matcher)
    import importlib.util
    spec = importlib.util.spec_from_file_location("sweep_variants", "tools/sweep_variants.py")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)  # type: ignore[union-attr]
    s, e = mod.find_function_span(tu_text, func)
    return tu_text[:s] + body.rstrip("\n") + "\n" + tu_text[e:]


def _build(stem: str, src_path: str, out_o: str, cc1: str) -> tuple[bool, str]:
    cmd = pipeline.c_pipeline_cmd(stem, out_o, {"src_override": src_path})
    cmd = cmd.replace(f"{cfg.CC1} ", f"{cc1} ", 1)
    Path(out_o).parent.mkdir(parents=True, exist_ok=True)
    r = subprocess.run(cmd, shell=True, capture_output=True, text=True, executable="/bin/bash")
    # cc1 2.7.2 exits non-zero on the TU's known extern-type conflicts but still
    # emits the full asm (the Makefile pipeline swallows that status) — judge by
    # the object's presence, not the pipeline's exit code.
    return Path(out_o).exists(), r.stderr[-600:]


def cc1psx_check(func: str, candidate: str | None = None,
                 workdir: str = "tmp/cc1psx") -> dict:
    stem = func_file(func)
    cand = Path(candidate or f"memory/grind/{func}/candidate.c")
    if not cand.is_file():
        return {"ok": False, "func": func, "error": f"no candidate at {cand}"}
    if not Path(CC1PSX_WRAPPER).is_file() or not Path("tools/cc1psx.exe").is_file():
        return {"ok": False, "func": func, "error": "cc1psx wrapper/exe missing"}
    body = cand.read_text(encoding="utf-8", errors="replace")
    body_sha = hashlib.sha1(body.encode("utf-8")).hexdigest()[:12]
    tu = Path(f"src/{stem}.c").read_text(encoding="utf-8", errors="replace")
    wd = Path(workdir) / func
    (wd / "src").mkdir(parents=True, exist_ok=True)
    src_ovr = wd / "src" / f"{stem}.c"
    src_ovr.write_text(_splice(tu, func, body), encoding="utf-8", newline="\n")
    ref_o = f"build/src/{stem}.o"
    if not Path(ref_o).exists():
        return {"ok": False, "func": func, "error": f"{ref_o} missing — run `engine build` first"}
    out = {"ok": True, "func": func, "file": stem, "candidate": str(cand),
           "candidate_sha": body_sha}
    for label, cc1 in (("ours", cfg.CC1), ("psx", CC1PSX_WRAPPER)):
        o = wd / f"{label}.o"
        if o.exists():
            o.unlink()
        built, err = _build(stem, str(src_ovr), str(o), cc1)
        if not built:
            out[label] = {"score": None, "error": err}
            continue
        try:
            out[label] = score.score_func(str(o), ref_o, func)
        except KeyError as e:
            out[label] = {"score": None, "error": str(e)}
    ours = (out.get("ours") or {}).get("score")
    psx = (out.get("psx") or {}).get("score")
    out["closer"] = (isinstance(ours, int) and isinstance(psx, int) and psx < ours)
    if not isinstance(psx, int):
        out["ok"] = False
        out["error"] = "cc1psx produced no scorable object: " + str((out.get("psx") or {}).get("error"))
    out["verdict"] = ("FIDELITY-LEAD: cc1psx is strictly closer to the target than our cc1 — "
                      "investigate the toolchain, do not rotate" if out["closer"] else
                      "SOURCE-SIDE: the original compiler is no closer from this source — the "
                      "residual is a spelling not yet found")
    return out

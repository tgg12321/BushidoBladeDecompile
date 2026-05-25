"""Clean build pipeline — replaces the Makefile recipe macros.

Per-C-file pipeline (identical stage order to the Makefile):
    cpp | cc1 | prologue_fix | maspsx | [fix_lwl] | [align sed] | multu_pad
        | regfix | regfix_stage2 | asmfix | as -> .o

Then: ld (via splat linker script) -> objcopy -> make_psexe -> SHA1 check.

Runs from the repo root, inside WSL. Every command is the exact shell pipeline
the Makefile produces, so byte-parity is guaranteed once `engine parity` is
green — the only thing that changed is that config is now explicit data and the
pipeline is a single readable function with a `cheat_overrides` seam for the
Phase 1 cheat-invisible sandbox.
"""
from __future__ import annotations

import hashlib
import subprocess
from pathlib import Path

from . import buildconfig as cfg


def sh(cmd: str, **kw) -> subprocess.CompletedProcess:
    """Run a shell command under bash (so pipes / env-prefixes behave)."""
    return subprocess.run(["bash", "-c", cmd], **kw)


def sha1(path: str | Path) -> str:
    h = hashlib.sha1()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def c_stems() -> list[str]:
    return sorted(p.stem for p in Path("src").glob("*.c"))


def c_pipeline_cmd(stem: str, out_o: str, cheat_overrides=None) -> str:
    """Construct the exact per-file shell pipeline for src/<stem>.c -> out_o.

    `cheat_overrides` is the Phase 1 seam: a spec describing which cheat stages
    (regfix / regfix_stage2 / asmfix) to neutralize for a given function so the
    scoring sandbox measures the honest, cheat-free codegen distance. Not yet
    implemented — Phase 0 only needs the faithful, cheats-on pipeline.
    """
    if cheat_overrides:
        raise NotImplementedError(
            "cheat_overrides is a Phase 1 feature (cheat-invisible scoring sandbox)"
        )

    cc_flags = cfg.CC_FLAGS_GP if stem in cfg.GP_FILES else cfg.CC_FLAGS
    if stem in cfg.NO_SR_FILES:
        cc_flags += " -fno-strength-reduce"

    maspsx_flags = cfg.MASPSX_FLAGS_GP if stem in cfg.GP_FILES else cfg.MASPSX_FLAGS
    if stem in cfg.EXPAND_LB_FILES:
        maspsx_flags += " --expand-lb"
    if stem in cfg.EXPAND_LH_FILES:
        maspsx_flags += " --expand-lh"

    stages = [
        f"{cfg.CPP} {cfg.CPP_FLAGS} {cfg.CPP_DEFS} src/{stem}.c",
        f"{cfg.CC1} {cc_flags}",
        cfg.PROLOGUE_FIX,
        f"{cfg.MASPSX} {maspsx_flags}",
    ]
    if stem in cfg.FIX_LWL_FILES:
        stages.append(cfg.FIX_LWL)
    if stem in cfg.RODATA_ALIGN2_FILES:
        stages.append(r'sed "s/\.align\t3/.align\t2/"')
    stages += [
        cfg.MULTU_PAD,
        cfg.REGFIX,
        cfg.REGFIX_STAGE2,
        cfg.ASMFIX,
        f"{cfg.AS} {cfg.AS_FLAGS} -o {out_o}",
    ]
    # NO pipefail, deliberately. The Makefile checks only the final stage's
    # (`as`) exit, and this build legitimately relies on that: cc1 (GCC 2.7.2
    # SN) exits non-zero (e.g. 33, "parse error before `GameObj'") on several
    # m2c-decompiled files, yet still emits the CORRECT assembly that flows
    # downstream to a byte-identical .o. Intermediate exit codes are noise here;
    # the oracle / parity byte-check is the only correctness gate. (See the
    # Phase 0 finding: pipefail broke parity on code6cac et al.)
    return " | ".join(stages)


def build_c_object(stem: str, out_o: str, cheat_overrides=None) -> str:
    Path(out_o).parent.mkdir(parents=True, exist_ok=True)
    cmd = c_pipeline_cmd(stem, out_o, cheat_overrides)
    r = sh(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError(f"C build failed for {stem}\nCMD: {cmd}\nSTDERR:\n{r.stderr}")
    return out_o


def build_asm_object(s_path: str, out_o: str) -> str:
    Path(out_o).parent.mkdir(parents=True, exist_ok=True)
    cmd = f"{cfg.AS} {cfg.AS_FLAGS} {s_path} -o {out_o}"
    r = sh(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError(f"asm build failed for {s_path}\nSTDERR:\n{r.stderr}")
    return out_o


def link(build_dir: str = "build") -> None:
    bd = Path(build_dir)
    syms = " ".join(f"-T {s}" for s in cfg.LD_SYM_FILES)
    cmd = (
        f"{cfg.LD} {cfg.LD_FLAGS} -Map {bd}/bb2.map -T {cfg.LD_SCRIPT} {syms} "
        f"-o {bd}/bb2.elf"
    )
    r = sh(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError(f"link failed\nCMD: {cmd}\nSTDERR:\n{r.stderr}")


def make_exe(build_dir: str = "build") -> str:
    bd = Path(build_dir)
    r1 = sh(f"{cfg.OBJCOPY} -O binary -j .main {bd}/bb2.elf {bd}/bb2.bin",
            capture_output=True, text=True)
    if r1.returncode != 0:
        raise RuntimeError(f"objcopy failed\nSTDERR:\n{r1.stderr}")
    r2 = sh(f"python3 tools/make_psexe.py {cfg.TARGET_EXE} {bd}/bb2.bin {bd}/bb2.exe",
            capture_output=True, text=True)
    if r2.returncode != 0:
        raise RuntimeError(f"make_psexe failed\nSTDERR:\n{r2.stderr}")
    return str(bd / "bb2.exe")


def build_all(build_dir: str = "build") -> str:
    """Full build into build_dir. NOTE: the splat linker script hardcodes
    `build/...` object paths, so a non-default build_dir works for objects but
    not for `link`. Phase 0 full builds use the default 'build'.
    """
    bd = Path(build_dir)
    for stem in c_stems():
        build_c_object(stem, str(bd / "src" / f"{stem}.o"))
    for s in sorted(Path("asm").glob("*.s")):
        build_asm_object(str(s), str(bd / "asm" / f"{s.stem}.o"))
    for s in sorted(Path("asm/data").glob("*.s")):
        build_asm_object(str(s), str(bd / "asm" / "data" / f"{s.stem}.o"))
    link(build_dir)
    return make_exe(build_dir)


def _first_diff(a: bytes, b: bytes) -> int:
    n = min(len(a), len(b))
    for i in range(n):
        if a[i] != b[i]:
            return i
    return n


def parity_objects(out_dir: str = "tmp/parity", ref_dir: str = "build") -> list[tuple]:
    """Build every C object with the clean driver into out_dir and byte-compare
    against the reference objects in ref_dir (the Makefile build). Returns
    (stem, status, info) per file; status in MATCH / DIFF / NO_REF.
    """
    results = []
    for stem in c_stems():
        out_o = f"{out_dir}/src/{stem}.o"
        build_c_object(stem, out_o)
        ref = f"{ref_dir}/src/{stem}.o"
        if not Path(ref).exists():
            results.append((stem, "NO_REF", ref))
            continue
        if sha1(out_o) == sha1(ref):
            results.append((stem, "MATCH", None))
        else:
            da = Path(out_o).read_bytes()
            db = Path(ref).read_bytes()
            off = _first_diff(da, db)
            results.append((stem, "DIFF", f"first-diff@{off} sizes {len(da)} vs {len(db)}"))
    return results

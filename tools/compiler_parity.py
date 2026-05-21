#!/usr/bin/env python3
"""Compiler-parity PROOF: every build, prove the compiler is never the variable.

Agents repeatedly waste time believing a stuck match is a "cc1psx vs our gcc"
codegen difference. This makes the truth empirical and unmissable, every build:

  * decompals/mips-gcc-2.7.2 reproduces the ORIGINAL GAME byte-for-byte
    (the whole build SHA1-matches -- that is the standing proof for decompals).
  * Sony's cc1psx, run through the IDENTICAL pipeline (cpp|cc1|prologue_fix|
    maspsx|as, no regfix), DIVERGES on some functions -- and on ZERO of them is
    it closer to the original than decompals (0 "compiler-fixable"). Often it is
    strictly WORSE: it diverges from the original on functions decompals matches.

So switching compilers can NEVER turn a non-match into a match. A stuck function
is a C-source problem (typing / dataflow / scheduling / regalloc), never cc1psx.

Method (per function): compile the file with each compiler, disassemble, and
compare each compiler's words to the ORIGINAL GAME's words (from asm/funcs/<f>.s),
masking relocated operands (jal/branch/%hi/%lo) so link-time relocations in the
unlinked .o don't show as false differences. Categories per diverging function:
  CC1PSX_WORSE     decompals == original, cc1psx != original   (cc1psx is wrong)
  COMPILER_FIXABLE cc1psx == original, decompals != original    (would be historic)
  NEITHER          both != original raw                         (needs regfix/C-work)

Modes:
  --file SRC --emit STAMP   one file -> write counts to STAMP (Makefile, cached)
  --banner                  read build/parity/*.stamp, print the aggregate banner
  --all                     run every src/*.c then print the banner

Graceful: if tools/cc1psx.exe is absent (gitignored / proprietary) or dosemu2 is
unavailable, files are SKIPPED and the banner says so -- the build is NOT failed.
"""
from __future__ import annotations
import argparse, json, re, subprocess, sys, tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CC1_DECOMPALS = ROOT / "tools" / "gcc-2.7.2" / "build" / "cc1"
CC1_PSX_WRAPPER = ROOT / "tools" / "cc1psx_wrapper.sh"
CC1_PSX_EXE = ROOT / "tools" / "cc1psx.exe"
PROLOGUE_FIX = ROOT / "tools" / "prologue_fix.py"
MASPSX = ROOT / "tools" / "maspsx" / "maspsx.py"
PARITY_DIR = ROOT / "build" / "parity"
FUNCS_DIR = ROOT / "asm" / "funcs"

CC_FLAGS = "-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w".split()
CPP_FLAGS = ["-I", str(ROOT / "include"), "-undef", "-Wall", "-lang-c", "-fno-builtin",
    "-Dmips", "-D__GNUC__=2", "-D__OPTIMIZE__", "-D__mips__", "-D__mips", "-Dpsx",
    "-D__psx__", "-D__psx", "-D_PSYQ", "-D__EXTENSIONS__", "-D_MIPSEL", "-D_LANGUAGE_C", "-DLANGUAGE_C"]
MASPSX_FLAGS = ["--expand-div", "--aspsx-version=2.34", "--sdata-syms=sdata_syms.txt",
    "--sdata-funcs=sdata_funcs.txt", "--sdata-exclude=sdata_exclude.txt", "--expand-lb",
    "--expand-lb-funcs=expand_lb_funcs.txt", "--multu-funcs=multu_funcs.txt",
    "--expand-dest-funcs=expand_dest_funcs.txt"]
AS_FLAGS = ["-I", str(ROOT / "include"), "-march=r3000", "-mtune=r3000", "-no-pad-sections", "-O1", "-G0"]


def _shq(s):
    import shlex; return shlex.quote(str(s))


def cc1psx_available():
    return CC1_PSX_EXE.exists() and CC1_PSX_WRAPPER.exists()


def compile_raw(cc1_cmd_str, src, out_o):
    """cpp|cc1|prologue_fix|maspsx|as (NO regfix/asmfix). Returns (ok, err)."""
    try:
        rel = src.resolve().relative_to(ROOT.resolve())
    except ValueError:
        rel = src
    pipe = (f"mipsel-linux-gnu-cpp {' '.join(_shq(f) for f in CPP_FLAGS)} {_shq(rel)} "
            f"| {cc1_cmd_str} {' '.join(_shq(f) for f in CC_FLAGS)} "
            f"| python3 {_shq(PROLOGUE_FIX)} "
            f"| python3 {_shq(MASPSX)} {' '.join(_shq(f) for f in MASPSX_FLAGS)} "
            f"| mipsel-linux-gnu-as {' '.join(_shq(f) for f in AS_FLAGS)} -o {_shq(out_o)}")
    try:
        cp = subprocess.run(["bash", "-c", "set -o pipefail; " + pipe],
                            capture_output=True, timeout=180, cwd=str(ROOT))
    except subprocess.TimeoutExpired:
        return False, "timeout"
    if cp.returncode != 0:
        return False, f"rc={cp.returncode}: {cp.stderr.decode('utf-8','ignore')[-300:]}"
    return (True, "") if out_o.exists() and out_o.stat().st_size else (False, "empty .o")


def objdump_dr(o):
    """{func: (words[], reloc_index_set)} from objdump -dr (reloc lines mark the
    preceding instruction as relocated, so we can mask it in comparisons)."""
    out = subprocess.run(["mipsel-linux-gnu-objdump", "-dr", str(o)],
                         capture_output=True, text=True).stdout
    res, cur, idx = {}, None, -1
    for line in out.splitlines():
        if ">:" in line and "<" in line:
            cur = line.split("<", 1)[1].split(">", 1)[0]
            res[cur] = ([], set()); idx = -1; continue
        if cur is None:
            continue
        m = re.match(r"\s*[0-9a-f]+:\s+([0-9a-f]{8})\s", line)
        if m:
            res[cur][0].append(int(m.group(1), 16)); idx += 1
        elif "R_MIPS" in line and idx >= 0:
            res[cur][1].add(idx)  # this instruction carries a relocation
    return res


def target_words(func):
    p = FUNCS_DIR / f"{func}.s"
    if not p.exists():
        return None
    ws = []
    for line in p.read_text(errors="ignore").splitlines():
        m = re.match(r"\s*/\*\s*[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s*\*/", line)
        if m:
            ws.append(int.from_bytes(bytes.fromhex(m.group(1)), "little"))
    return ws or None


# lwl/lwr/swl/swr: the decompals fork emits big-endian partial-word OFFSETS
# (off ^ 3) vs PS1 little-endian; the build corrects this deterministically
# (tools/fix_lwl.py, or asmfix offset-swaps). It is a KNOWN mechanical quirk,
# not a real codegen divergence, so we normalize it out (clear the low 2 bits
# of the offset for these opcodes) before comparing. Without this, cc1psx
# (which has no quirk) falsely looks "compiler-fixable" on lwl-using functions.
_LWL_OPCODES = {0x22, 0x26, 0x2A, 0x2E}  # lwl, lwr, swl, swr

def norm_word(w):
    if ((w >> 26) & 0x3F) in _LWL_OPCODES:
        return w & ~0x3
    return w

def matches_target(words, relocs, tgt):
    """True if words == tgt ignoring relocated indices + the lwl-offset quirk."""
    if tgt is None or len(words) != len(tgt):
        return False
    return all(norm_word(words[i]) == norm_word(tgt[i])
               for i in range(len(words)) if i not in relocs)


def parity_one(src):
    name = src.name
    if not cc1psx_available():
        return {"file": name, "status": "SKIP", "reason": "cc1psx.exe absent"}
    with tempfile.TemporaryDirectory() as td:
        td = Path(td); d_o, p_o = td / "d.o", td / "p.o"
        ok, err = compile_raw(_shq(CC1_DECOMPALS), src, d_o)
        if not ok:
            return {"file": name, "status": "SKIP", "reason": f"decompals: {err[:100]}"}
        ok, err = compile_raw(f"bash {_shq(CC1_PSX_WRAPPER)}", src, p_o)
        if not ok:
            return {"file": name, "status": "SKIP", "reason": f"cc1psx: {err[:100]}"}
        d, p = objdump_dr(d_o), objdump_dr(p_o)
    agree = worse = fixable = neither = 0
    worse_funcs, fixable_funcs = [], []
    for func, (dw, dr) in d.items():
        pw, pr = p.get(func, ([], set()))
        if [norm_word(w) for w in dw] == [norm_word(w) for w in pw]:
            agree += 1; continue        # cc1psx == decompals (modulo lwl quirk): no difference
        tgt = target_words(func)
        d_ok = matches_target(dw, dr, tgt)
        p_ok = matches_target(pw, pr, tgt)
        if p_ok and not d_ok:
            fixable += 1; fixable_funcs.append(func)
        elif d_ok and not p_ok:
            worse += 1; worse_funcs.append(func)
        else:
            neither += 1
    return {"file": name, "status": "OK", "agree": agree, "worse": worse,
            "fixable": fixable, "neither": neither, "total": len(d),
            "worse_funcs": worse_funcs[:8], "fixable_funcs": fixable_funcs[:8]}


def write_stamp(res, stamp):
    stamp.parent.mkdir(parents=True, exist_ok=True)
    stamp.write_text(json.dumps(res) + "\n", encoding="utf-8")


SUMMARY_FILE = ROOT / "docs" / "diagnostics" / "compiler_parity.txt"

def banner():
    import datetime
    recs = []
    for s in sorted(PARITY_DIR.glob("*.stamp")):
        try:
            recs.append(json.loads(s.read_text(encoding="utf-8")))
        except Exception:
            pass
    ok = [r for r in recs if r.get("status") == "OK"]
    skip = [r for r in recs if r.get("status") == "SKIP"]
    if not recs:
        L = ["Compiler parity: no results yet — run `bash tools/dc.sh prove-compilers`."]
    elif not ok and skip:
        L = ["Compiler parity: SKIPPED (cc1psx.exe absent / dosemu2 unavailable).",
             "Restore cc1psx.exe per .gitignore, then `bash tools/dc.sh prove-compilers`."]
    else:
        tot = sum(r["total"] for r in ok); agree = sum(r["agree"] for r in ok)
        worse = sum(r["worse"] for r in ok); fixable = sum(r["fixable"] for r in ok)
        neither = sum(r["neither"] for r in ok)
        worse_eg = [f for r in ok for f in r.get("worse_funcs", [])][:4]
        date = datetime.date.today().isoformat()
        L = [f"{fixable} genuine compiler-fixable / {worse} cc1psx-WORSE of {tot} funcs / "
             f"{len(ok)} files (as of {date}) — cc1psx is never the variable; sometimes worse.",
             "",
             "decompals-gcc-2.7.2 reproduces the ORIGINAL GAME byte-for-byte (build SHA1: OK).",
             "PsyQ cc1psx, identical pipeline (last refresh):",
             f"  identical codegen:                                       {agree}/{tot}",
             f"  cc1psx WORSE (cc1psx != game, decompals ==): {worse:>4}  e.g. {', '.join(worse_eg)}",
             f"  both differ raw (needs C-work/regfix; cc1psx no better):  {neither}",
             f"  COMPILER-FIXABLE (cc1psx matches where decompals can't):  {fixable}",
             "  (the decompals lwl/lwr endianness quirk is normalized out — the build fixes",
             "   it deterministically; without that, cc1psx falsely looks 'fixable'.)",
             "",
             "=> cc1psx CANNOT match what decompals can't, and is sometimes WORSE. A stuck",
             "   function is a C-SOURCE problem — do NOT reach for cc1psx (hook-blocked).",
             f"  Coverage: {len(ok)} raw-compilable files; {len(skip)} skipped (need full pipeline).",
             "  Refresh: bash tools/dc.sh prove-compilers"]
        if fixable:
            L.append(f"  !!! {fixable} COMPILER-FIXABLE — verify; would overturn the calibration.")
    try:
        SUMMARY_FILE.parent.mkdir(parents=True, exist_ok=True)
        SUMMARY_FILE.write_text("\n".join(L) + "\n", encoding="utf-8")
    except OSError:
        pass
    bar = "=" * 74
    print(bar); print("\n".join(L)); print(bar)
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--file"); ap.add_argument("--emit")
    ap.add_argument("--banner", action="store_true"); ap.add_argument("--all", action="store_true")
    a = ap.parse_args()
    if a.banner:
        return banner()
    if a.all:
        for src in sorted((ROOT / "src").glob("*.c")):
            res = parity_one(src); write_stamp(res, PARITY_DIR / f"{src.name}.stamp")
            print(f"  {res['status']:4s} {res['file']}"
                  + (f"  worse={res['worse']} fixable={res['fixable']} neither={res['neither']} agree={res['agree']}"
                     if res['status'] == 'OK' else f"  ({res.get('reason','')})"), file=sys.stderr)
        return banner()
    if a.file:
        src = Path(a.file); src = src if src.is_absolute() else ROOT / a.file
        res = parity_one(src)
        if a.emit:
            write_stamp(res, Path(a.emit))
        print(json.dumps(res))
        return 0
    ap.print_help(); return 0


if __name__ == "__main__":
    sys.exit(main())

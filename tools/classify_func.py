#!/usr/bin/env python3
"""Pre-dive classification report for a single function.

Combines size/shape analysis, blocker fingerprints, inline-asm status, Kengo
reference lookup, and PsyQ-stdlib detection into a one-page brief. Replaces
the 2K-token per-function manual pre-dive previously demanded by
feedback_pre_dive_analysis.md.

Output is structured key/value text by default. `--json` emits a machine-
readable dict. The final field `recommendation` is one of:

    permanently_blocked:<reason> — uses break / handwritten add/sub/addi /
                                   $sp swap / no-jr-ra / data-as-code; cannot
                                   be expressed in pure C with GCC 2.7.2.
                                   STOP -- do not attempt. See known_blocked.txt.
    easy_attempt        — low risk, run `dc.sh attempt`
    syscall_trampoline  — BIOS jump pattern, allowed-as-asm exception
    psyq_stdlib_<name>  — memcpy/memset/etc match; replace with C idiom
    needs_rodata_split  — switch with jlabel/jtbl (use feedback_cu_split_for_jtbl)
    needs_lwl_fix       — function uses lwl/swl (toggle FIX_LWL_FILES)
    gte_function        — cop2 ops present; needs gte.h macros
    multi_function      — multiple `jr $ra` (likely merged by splat)
    aspsx_delay_swra    — sw $ra in branch delay slot (likely intractable)
    hard_blocker_<tag>  — generic flag, see notes
    standard            — nothing unusual; standard workflow applies

Exit codes:
    0 — report produced
    1 — function not found
    2 — internal error
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS = ROOT / "asm" / "funcs"
SRC_DIR = ROOT / "src"
KENGO_REF = ROOT / "tools" / "kengo_ref.py"
KNOWN_BLOCKED = ROOT / "known_blocked.txt"

GTE_OPS = {
    "cop2", "mtc2", "mfc2", "lwc2", "swc2", "ctc2", "cfc2",
    "nclip", "rtps", "rtpt", "avsz3", "avsz4", "gpf", "gpl", "mvmva",
    "ncs", "ncds", "ncdt", "ncct", "nccs", "cdp", "cc", "sqr",
    "dpcs", "dpct", "intpl", "op", "dpcl",
}

BRANCH_OPS = {
    "j", "jal", "jr", "jalr", "b",
    "beq", "bne", "beqz", "bnez", "bgez", "bgtz", "blez", "bltz",
    "bgezal", "bltzal", "beql", "bnel",
}


# ----- Asm parsing --------------------------------------------------------

INSN_RE = re.compile(
    r"/\*\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]+)\s+[0-9A-Fa-f]+\s+\*/\s+(\S+)\s*(.*)"
)


def load_asm_lines(func: str) -> list[tuple[int, str, str]] | None:
    """Return [(addr, mnem, ops), ...] from asm/funcs/<func>.s."""
    p = ASM_FUNCS / f"{func}.s"
    if not p.exists():
        return None
    out = []
    for line in p.read_text(encoding="utf-8").splitlines():
        m = INSN_RE.search(line)
        if m:
            out.append((int(m.group(1), 16), m.group(2), m.group(3).strip()))
    return out


# ----- Source file location ----------------------------------------------

def find_in_src(func: str) -> dict:
    """Locate where the function is wired up in src/. Returns:
       {file, kind, line, end_line}  where kind ∈
       {include_asm, inline_asm, c_function, none}."""
    for src in sorted(SRC_DIR.glob("*.c")):
        text = src.read_text(encoding="utf-8", errors="ignore")
        if not text:
            continue
        # 1. INCLUDE_ASM stub
        m = re.search(rf'INCLUDE_ASM\s*\(\s*"asm/funcs"\s*,\s*{re.escape(func)}\s*\)',
                      text)
        if m:
            line = text.count("\n", 0, m.start()) + 1
            return {"file": str(src.relative_to(ROOT)), "kind": "include_asm",
                    "line": line, "end_line": line}
        # 2. Inline __asm__ block
        for blk in re.finditer(r"__asm__\s*\(([^;]*?)\)\s*;", text, re.DOTALL):
            body = blk.group(1)
            if re.search(rf"glabel\s+{re.escape(func)}\b", body):
                start_line = text.count("\n", 0, blk.start()) + 1
                end_line = text.count("\n", 0, blk.end()) + 1
                return {"file": str(src.relative_to(ROOT)), "kind": "inline_asm",
                        "line": start_line, "end_line": end_line}
        # 3. Real C function (returns ... <func>(args) { ... })
        m = re.search(
            rf"(?:^|\n)(?:[\w\s\*]+\s){re.escape(func)}\s*\([^)]*\)\s*\{{",
            text, re.MULTILINE)
        if m:
            line = text.count("\n", 0, m.start()) + 1
            return {"file": str(src.relative_to(ROOT)), "kind": "c_function",
                    "line": line, "end_line": line}
    return {"file": None, "kind": "none", "line": None, "end_line": None}


# ----- Pattern detectors --------------------------------------------------

def is_bios_trampoline(insns) -> tuple[bool, str | None]:
    """Detect three forms of the kernel/BIOS-trampoline pattern:

      a) BIOS table call:
            addiu $tX, $zero, 0xA0|0xB0|0xC0
            jr    $tX
            addiu $tY, $zero, 0xNN
      b) Kernel syscall:
            addiu $a0|$zero, ...
            .word 0x0000000C        # `syscall 0` raw
            jr    $ra
      c) Kernel break:
            .word 0x0000000?...0D   # `break N` raw

    Returns (is_trampoline, kind_str)."""
    if not 2 <= len(insns) <= 6:
        return False, None
    ops_full = [i[2].replace(" ", "") for i in insns]
    mnems = [i[1] for i in insns]

    # Form (a)
    has_jr_t = any(m == "jr" and "$t" in o for m, o in zip(mnems, ops_full))
    addiu_table_match = None
    for _, m, ops in insns:
        ops_n = ops.replace(" ", "")
        if m == "addiu":
            mm = re.search(r"\$t\d+,\$zero,(0x[ABC]0|160|176|192)", ops_n)
            if mm:
                addiu_table_match = mm.group(1)
                break
    if has_jr_t and addiu_table_match:
        return True, f"bios_table_{addiu_table_match}"

    # Forms (b) and (c) -- raw `.word` encodings or splat-decoded mnemonics
    # for `syscall` (0x0C) or `break` (0x0D).
    for _, m, ops in insns:
        ops_n = ops.replace(" ", "").lower()
        if m == ".word" and re.match(r"^0x[0-9a-f]{0,6}0[cd]$", ops_n):
            return True, "syscall_or_break"
        if m in ("syscall", "break"):
            return True, m
    return False, None


def psyq_stdlib_fingerprint(insns) -> str | None:
    """Heuristic match for common PsyQ stdlib leaf shapes."""
    if not insns:
        return None
    mnems = [i[1] for i in insns]
    n = len(insns)
    # memcpy: lbu/sb in a tight loop — at least one lbu+sb pair, addiu +1, branch back
    if n <= 25:
        has_lbu_sb = any(m == "lbu" for m in mnems) and any(m == "sb" for m in mnems)
        has_loop = any(m in {"bne", "bnez", "bgez", "bgtz", "bltz"} for m in mnems)
        if has_lbu_sb and has_loop and n <= 16:
            # memcpy / memmove
            has_neg_step = any(
                m == "addiu" and re.search(r"-1\b", ops) for _, m, ops in insns
            )
            return "memmove" if has_neg_step else "memcpy"
    # memset: sw / sb in loop with the same value register; no lbu read
    if n <= 18:
        has_store_loop = any(m in {"sw", "sb"} for m in mnems) and \
                         any(m in {"bne", "bnez"} for m in mnems)
        has_no_load = not any(m in {"lw", "lb", "lbu", "lh", "lhu"} for m in mnems)
        if has_store_loop and has_no_load:
            return "memset"
    return None


def load_known_blocked() -> dict[str, tuple[str, str]]:
    """Read known_blocked.txt -> {func: (reason, evidence)}."""
    out: dict[str, tuple[str, str]] = {}
    if not KNOWN_BLOCKED.exists():
        return out
    for raw in KNOWN_BLOCKED.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        parts = re.split(r"\s{2,}|\t", line, maxsplit=2)
        if len(parts) < 2:
            continue
        func = parts[0].strip()
        reason = parts[1].strip()
        evidence = parts[2].strip() if len(parts) >= 3 else ""
        out[func] = (reason, evidence)
    return out


def detect_permanent_blockers(insns) -> list[str]:
    """Detect patterns that genuinely cannot be expressed in pure C with
    GCC 2.7.2. Returns reason codes matching known_blocked.txt."""
    tags: list[str] = []
    mnems = [m for _, m, _ in insns]

    # break <code> -- debug trap; GCC 2.7.2 has no __builtin_trap that emits this
    if "break" in mnems:
        tags.append("break_instruction")

    # add / sub / addi (overflow-trap variants). GCC 2.7.2 emits addu / subu /
    # addiu by default; these signed overflow-trapping ops are only produced
    # via inline asm or hand-rolled assembly.
    overflow_ops = {"add", "sub", "addi"}
    if any(m in overflow_ops for m in mnems):
        tags.append("handwritten_overflow_op")

    # $sp written from a non-immediate source (i.e., not addiu $sp,$sp,N).
    # Pattern like `addu $sp,$aN,$zero` is a stack-swap primitive impossible in C.
    sp_write_re = re.compile(r"\$sp\s*,\s*\$(?:[atvs]\d|fp|gp|ra|k\d)")
    for _, m, ops in insns:
        if m in ("addu", "or", "move", "subu") and sp_write_re.match(ops):
            tags.append("sp_manipulation")
            break

    # No `jr $ra` anywhere -> orphaned fragment / data-as-code.
    # Function falls through to next or is accessed as data via mid-text alabels.
    has_jr_ra = any(m == "jr" and "$ra" in ops for _, m, ops in insns)
    if not has_jr_ra and len(insns) > 0:
        tags.append("no_return_fragment")

    return tags


def detect_blockers(insns) -> list[str]:
    """Return a list of blocker tags."""
    tags = []
    mnems = [i[1] for i in insns]
    ops_text = "\n".join(i[2] for i in insns)
    counts = Counter(mnems)
    if counts.get("lwl", 0) or counts.get("swl", 0) or counts.get("lwr", 0) or counts.get("swr", 0):
        tags.append("lwl_swl")
    if "jlabel" in ops_text or "jtbl_" in ops_text:
        tags.append("jlabel_switch")
    if any(op in counts for op in GTE_OPS):
        tags.append("gte_ops")
    # Multiple jr $ra -> probable merged functions
    jr_ra_count = sum(1 for _, m, ops in insns if m == "jr" and "$ra" in ops)
    if jr_ra_count > 1:
        tags.append("multi_jr_ra")
    # sw $ra inside a branch delay slot (heuristic: sw $ra appears after a jr/branch
    # or its index follows an unconditional branch)
    for i, (_, m, ops) in enumerate(insns):
        if m == "sw" and "$ra" in ops and i > 0:
            prev_m = insns[i - 1][1]
            if prev_m in BRANCH_OPS and prev_m != "jal":
                tags.append("aspsx_swra_delay")
                break
    # .word encodings — informational
    word_count = ops_text.count(".word ")
    if word_count > 0:
        tags.append(f"word_encoded_{word_count}")
    return tags


def kengo_lookup(func: str) -> str | None:
    """Best-effort: ask kengo_ref.py for a matched name. Returns short hint."""
    if not KENGO_REF.exists():
        return None
    try:
        r = subprocess.run(
            [sys.executable, str(KENGO_REF), func, "--bb2"],
            capture_output=True, text=True, cwd=str(ROOT), timeout=10,
        )
        out = (r.stdout + r.stderr).strip()
        if "no kengo match" in out.lower() or "not found" in out.lower():
            return None
        # First non-empty meaningful line
        for line in out.splitlines():
            line = line.strip()
            if line and not line.startswith("#"):
                return line[:80]
    except Exception:
        return None
    return None


def neighbor_summary(file_rel: str | None) -> dict:
    """Summarize how many INCLUDE_ASM stubs and inline __asm__ blocks live
    in the same .c file."""
    if not file_rel:
        return {"include_asm_count": 0, "inline_asm_count": 0}
    p = ROOT / file_rel
    if not p.exists():
        return {"include_asm_count": 0, "inline_asm_count": 0}
    text = p.read_text(encoding="utf-8", errors="ignore")
    inc = len(re.findall(r"INCLUDE_ASM\s*\(", text))
    inline = len(re.findall(r"glabel\s+\w+", text))
    return {"include_asm_count": inc, "inline_asm_count": inline}


# ----- Top-level classify -------------------------------------------------

def classify(func: str) -> dict:
    insns = load_asm_lines(func)
    if insns is None:
        return {"func": func, "found": False,
                "recommendation": "not_found",
                "notes": [f"asm/funcs/{func}.s does not exist"]}

    src_info = find_in_src(func)
    blocker_tags = detect_blockers(insns)
    permanent_tags = detect_permanent_blockers(insns)
    manual_block = load_known_blocked().get(func)
    bios, bios_addr = is_bios_trampoline(insns)
    psyq = None if bios else psyq_stdlib_fingerprint(insns)
    kengo = kengo_lookup(func)
    neighbors = neighbor_summary(src_info.get("file"))

    n_insns = len(insns)
    n_branches = sum(1 for _, m, _ in insns if m in BRANCH_OPS)
    n_jal = sum(1 for _, m, _ in insns if m == "jal")
    is_leaf = n_jal == 0
    n_loads = sum(1 for _, m, _ in insns
                  if m in {"lw", "lh", "lhu", "lb", "lbu", "lwc2", "lwl", "lwr"})
    n_stores = sum(1 for _, m, _ in insns
                   if m in {"sw", "sh", "sb", "swc2", "swl", "swr"})

    # Recommendation
    # Permanent blockers FIRST: explicit list overrides auto-detection;
    # if neither matches, fall through to normal classification.
    if manual_block:
        reason, _ = manual_block
        recommendation = f"permanently_blocked:{reason}"
    elif permanent_tags:
        recommendation = f"permanently_blocked:{permanent_tags[0]}"
    elif bios:
        recommendation = f"bios_or_syscall:{bios_addr}"
    elif "multi_jr_ra" in blocker_tags:
        recommendation = "multi_function"
    elif "jlabel_switch" in blocker_tags:
        recommendation = "needs_rodata_split"
    elif "aspsx_swra_delay" in blocker_tags:
        recommendation = "aspsx_delay_swra"
    elif "lwl_swl" in blocker_tags:
        recommendation = "needs_lwl_fix"
    elif "gte_ops" in blocker_tags:
        recommendation = "gte_function"
    elif psyq:
        recommendation = f"psyq_stdlib_{psyq}"
    elif n_insns <= 30 and is_leaf and n_branches <= 3:
        recommendation = "easy_attempt"
    else:
        recommendation = "standard"

    return {
        "func": func, "found": True,
        "size": {
            "insns": n_insns, "branches": n_branches, "jal": n_jal,
            "loads": n_loads, "stores": n_stores, "leaf": is_leaf,
        },
        "src": src_info,
        "neighbors": neighbors,
        "blocker_tags": blocker_tags,
        "permanent_blocker_tags": permanent_tags,
        "manual_block": (
            {"reason": manual_block[0], "evidence": manual_block[1]}
            if manual_block else None
        ),
        "bios_trampoline": {"is": bios, "table": bios_addr},
        "psyq_stdlib": psyq,
        "kengo": kengo,
        "recommendation": recommendation,
    }


def print_report(d: dict):
    if not d.get("found", False):
        print(f"{d['func']}: NOT FOUND ({', '.join(d.get('notes', []))})")
        return
    f = d["func"]
    s = d["size"]
    src = d["src"]
    print(f"=== {f} -- {d['recommendation']} ===")
    print(f"  size       : {s['insns']} insns, {s['branches']} branches, "
          f"{s['jal']} jal, {s['loads']} loads, {s['stores']} stores"
          f"{', leaf' if s['leaf'] else ''}")
    src_str = (f"{src['file']}:{src['line']}-{src['end_line']} ({src['kind']})"
               if src["file"] else "not in src/")
    print(f"  src        : {src_str}")
    print(f"  neighbors  : {d['neighbors']['include_asm_count']} INCLUDE_ASM "
          f"+ {d['neighbors']['inline_asm_count']} inline asm in same file")
    if d["blocker_tags"]:
        print(f"  blockers   : {', '.join(d['blocker_tags'])}")
    if d.get("permanent_blocker_tags"):
        print(f"  PERMANENT  : {', '.join(d['permanent_blocker_tags'])} "
              "-- cannot be expressed in pure C with GCC 2.7.2")
    if d.get("manual_block"):
        mb = d["manual_block"]
        print(f"  SEQUESTERED: {mb['reason']}")
        if mb.get("evidence"):
            print(f"               evidence: {mb['evidence']}")
    if d["bios_trampoline"]["is"]:
        print(f"  bios_call  : table={d['bios_trampoline']['table']} "
              "(allowed-as-asm exception)")
    if d["psyq_stdlib"]:
        print(f"  psyq       : likely {d['psyq_stdlib']}")
    if d["kengo"]:
        print(f"  kengo      : {d['kengo']}")


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()

    d = classify(args.func)
    if args.json:
        print(json.dumps(d, indent=2))
    else:
        print_report(d)
    return 0 if d.get("found", False) else 1


if __name__ == "__main__":
    sys.exit(main())

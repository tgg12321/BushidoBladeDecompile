#!/usr/bin/env python3
"""sig_reconcile -- reconcile called-function signatures against TARGET codegen.

The C declaration surface (arg COUNT of called functions) controls cross-jump
merging: GCC 2.7.2 merges indirect calls whose CALL_INSN_FUNCTION_USAGE is equal
(jump.c:2426), and arg count is what differentiates them. m2c over-infers
signatures (extra trailing args), so the build under-counts call sites and GCC
merges handlers the original kept distinct. See
.claude/rules/cross-jump-call-merge.md.

This tool reads the TARGET disassembly, counts the real arguments set into each
call site (basic-block aware), and compares to the signatures declared in the
permuter base.c. It is GROUNDED in the target asm -- it never minimises diff
blindly, so it cannot introduce a cheat.

Usage:
  python3 tools/sig_reconcile.py <func> [--apply]
    <func>    function name (uses permuter/<func>/{base.c,target.o})
    --apply   rewrite base.c externs to the target-inferred arg counts
              (only the CONFIDENT ones; ambiguous live-in cases are left alone
               and reported for sig_search.py to resolve)
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

ARGREGS = ["a0", "a1", "a2", "a3"]
# mnemonics whose first operand is a SOURCE (read), not a destination (write)
NON_WRITERS = {
    "sw", "sb", "sh", "swl", "swr", "swc1", "swc2", "sc",
    "beq", "bne", "blez", "bgtz", "bltz", "bgez", "bltzal", "bgezal",
    "beqz", "bnez", "j", "jal", "jr", "jalr", "b", "nop",
    "mtc0", "mtc1", "mtc2", "ctc1", "ctc2", "cfc2", "mthi", "mtlo",
    "div", "divu", "mult", "multu",  # write hi/lo, not first operand
}
BRANCHES = {
    "beq", "bne", "blez", "bgtz", "bltz", "bgez", "bltzal", "bgezal",
    "beqz", "bnez", "j", "jr", "b",
}
CALLS = {"jal", "jalr"}


def disasm(obj, func):
    """Return list of dicts {addr, mnem, ops, reloc, targets} for func in obj."""
    out = subprocess.run(
        ["mipsel-linux-gnu-objdump", "-dr", obj],
        capture_output=True, text=True
    ).stdout
    insns = []
    in_f = False
    for line in out.splitlines():
        if re.search(r"<" + re.escape(func) + r">:", line):
            in_f = True
            continue
        if not in_f:
            continue
        if not line.strip():
            break
        rel = re.match(r"\s+[0-9a-fA-F]+:\s+(R_MIPS_\w+)\s+(\S+)", line)
        if rel:
            if insns:
                insns[-1]["reloc"] = (rel.group(1), rel.group(2))
            continue
        mi = re.match(r"\s*([0-9a-fA-F]+):\s+[0-9a-fA-F]+\s+(\S+)\s*(.*)", line)
        if mi:
            addr = int(mi.group(1), 16)
            mnem = mi.group(2)
            ops = mi.group(3).strip()
            insns.append({"addr": addr, "mnem": mnem, "ops": ops, "reloc": None})
    return insns


def branch_target(ins):
    """Address a BRANCH/JUMP goes to (not jal/jalr -- calls return, so they do
    not start a new basic block for our purposes)."""
    if ins["mnem"] not in BRANCHES:
        return None
    m = re.search(r"\b([0-9a-f]+)\s+<", ins["ops"])
    return int(m.group(1), 16) if m else None


def block_starts(insns):
    """Basic-block start addresses: entry, branch targets, insn after a branch
    (skipping the delay slot)."""
    if not insns:
        return set()
    starts = {insns[0]["addr"]}
    addrs = [i["addr"] for i in insns]
    for idx, ins in enumerate(insns):
        tgt = branch_target(ins)
        if tgt is not None and tgt in addrs:
            starts.add(tgt)
        if ins["mnem"] in BRANCHES:
            # the delay slot belongs to the branch; the block starts after it
            if idx + 2 < len(insns):
                starts.add(insns[idx + 2]["addr"])
    return starts


def dest_reg(ins):
    """First-operand destination register, or None if this insn writes no reg
    (or writes via a non-first operand)."""
    if ins["mnem"] in NON_WRITERS:
        return None
    m = re.match(r"([a-z][a-z0-9]+)\b", ins["ops"])
    return m.group(1) if m else None


def called_symbol(insns, call_idx):
    """Symbol for the call at call_idx. jal -> its reloc/target; jalr -> the
    lui %hi(sym) feeding the loaded pointer (scan back for the HI16 reloc)."""
    ins = insns[call_idx]
    if ins["mnem"] == "jal":
        if ins["reloc"]:
            return ins["reloc"][1]
        m = re.search(r"<([^>+]+)", ins["ops"])
        return m.group(1) if m else "<direct>"
    # jalr: backtrack for the HI16 reloc (the fn-ptr global address)
    for j in range(call_idx - 1, max(call_idx - 12, -1), -1):
        rel = insns[j].get("reloc")
        if rel and rel[0] == "R_MIPS_HI16":
            return rel[1]
    return "<indirect?>"


def infer_args(insns, call_idx, starts):
    """Return (confident_count, livein_regs). confident = a-regs WRITTEN inside
    the call's basic block (contiguous a0..). livein = a-regs written earlier in
    the function (potential args passed live-in), reported as ambiguous."""
    call_addr = insns[call_idx]["addr"]
    # find this block's start
    blk_start = max((s for s in starts if s <= call_addr), default=insns[0]["addr"])
    written_inblock = set()
    written_earlier = set()
    # the delay slot (insn AFTER the call) executes with the call and often sets
    # the last argument (e.g. `move a3,sN` / `sra a1`) -- include it.
    if call_idx + 1 < len(insns):
        d = dest_reg(insns[call_idx + 1])
        if d in ARGREGS:
            written_inblock.add(d)
    seen_call_before = False
    for j in range(call_idx - 1, -1, -1):
        a = insns[j]
        d = dest_reg(a)
        in_block = a["addr"] >= blk_start
        if a["mnem"] in CALLS:
            # a prior call clobbers a0-a3; writes before it don't reach us
            seen_call_before = True
        if d in ARGREGS and not seen_call_before:
            if in_block:
                written_inblock.add(d)
            else:
                written_earlier.add(d)
        if a["addr"] < blk_start:
            break
    # confident count = contiguous prefix a0,a1,... present in written_inblock
    confident = 0
    for k, r in enumerate(ARGREGS):
        if r in written_inblock:
            confident = k + 1
        else:
            break
    # live-in candidates: a-regs at/after the confident prefix written earlier
    livein = sorted(r for r in (written_earlier - written_inblock))
    return confident, livein


def parse_decls(base_c):
    """name -> declared arg count, from extern decls in base.c."""
    txt = Path(base_c).read_text(errors="ignore")
    decls = {}
    # extern <ret> (*NAME)(params);  and  extern <ret> NAME(params);
    for m in re.finditer(
        r"extern\s+[\w\s\*]+?\(\s*\*\s*(\w+)\s*\)\s*\(([^)]*)\)\s*;", txt
    ):
        decls[m.group(1)] = _count_params(m.group(2))
    for m in re.finditer(
        r"extern\s+[\w\s\*]+?\b(\w+)\s*\(([^)]*)\)\s*;", txt
    ):
        if m.group(1) not in decls:
            decls[m.group(1)] = _count_params(m.group(2))
    return decls


def _count_params(s):
    s = s.strip()
    if s == "" or s == "void":
        return 0
    return len([p for p in s.split(",") if p.strip()])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("func")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--dir", default=None, help="permuter dir (default permuter/<func>)")
    args = ap.parse_args()

    pdir = Path(args.dir) if args.dir else Path("permuter") / args.func
    target_o = pdir / "target.o"
    base_c = pdir / "base.c"
    if not target_o.exists() or not base_c.exists():
        print(f"ERROR: need {target_o} and {base_c}", file=sys.stderr)
        return 1

    tins = disasm(str(target_o), args.func)
    if not tins:
        print(f"ERROR: {args.func} not found in {target_o}", file=sys.stderr)
        return 1
    starts = block_starts(tins)
    decls = parse_decls(base_c)

    # build object (compile current base.c) for jalr-count comparison
    bins = []
    r = subprocess.run(
        ["bash", str(pdir / "compile.sh"), str(base_c), "-o", "/tmp/sigrec.o"],
        capture_output=True, text=True
    )
    if Path("/tmp/sigrec.o").exists():
        bins = disasm("/tmp/sigrec.o", args.func)

    t_jalr = sum(1 for i in tins if i["mnem"] == "jalr")
    t_jal = sum(1 for i in tins if i["mnem"] == "jal")
    b_jalr = sum(1 for i in bins if i["mnem"] == "jalr")
    b_jal = sum(1 for i in bins if i["mnem"] == "jal")
    print(f"call sites  jalr: build={b_jalr} target={t_jalr}   "
          f"jal: build={b_jal} target={t_jal}")
    if b_jalr < t_jalr:
        print("  -> BUILD UNDER-COUNTS jalr: cross-jump merged handlers the "
              "original kept distinct. Likely an arg-COUNT mismatch below.\n")
    elif b_jalr == t_jalr:
        print("  -> jalr count matches; signatures are structurally consistent.\n")

    # Per-call asm hint (ROUGH: in-block + delay-slot writes; UNDER-counts args
    # passed live-in from predecessor blocks). A hint to bound sig_search, NOT an
    # authoritative count -- arg counts are resolved empirically by sig_search.py
    # against the target jalr count.
    print(f"{'call (target)':<14} {'sym':<20} {'asm-hint':<26} declared")
    print("-" * 72)
    indirect = []
    for idx, ins in enumerate(tins):
        if ins["mnem"] not in CALLS:
            continue
        sym = called_symbol(tins, idx)
        conf, livein = infer_args(tins, idx, starts)
        hint = f">={conf}" + (f" (+live-in {','.join(livein)})" if livein else "")
        dec = decls.get(sym)
        dec_s = "?" if dec is None else str(dec)
        if ins["mnem"] == "jalr":
            indirect.append(sym)
        print(f"{hex(ins['addr']):<14} {sym:<20} {hint:<26} {dec_s}")

    print()
    if b_jalr is not None and b_jalr < t_jalr:
        print(f"VERDICT: build under-counts jalr ({b_jalr} < {t_jalr}) -> the fn-ptr "
              f"calls below are OVER-MERGED because they share an arg count.")
        seen = []
        for s in indirect:
            if s not in seen:
                seen.append(s)
        print("  fn-ptr calls to reconcile: " + ", ".join(seen))
        print(f"  -> resolve with:  python3 tools/sig_search.py {args.func} --apply")
    else:
        print(f"VERDICT: jalr count OK ({b_jalr} == {t_jalr}); no arg-count over-merge.")
    if args.apply:
        print("\nNote: sig_reconcile only DETECTS. Arg-count resolution + apply lives "
              "in sig_search.py (grounded in the target jalr count, not blind diff).")
    return 0


def _apply(base_c, fixes):
    """Rewrite extern fn-ptr decls to the inferred arg count. Drops/keeps trailing
    params; uses generic int params (modes do not affect FUNCTION_USAGE)."""
    txt = Path(base_c).read_bytes().decode("utf-8")
    for sym, n in fixes.items():
        params = "void" if n == 0 else ", ".join(["int"] * n)
        txt = re.sub(
            r"extern\s+[\w\s\*]+?\(\s*\*\s*" + re.escape(sym) + r"\s*\)\s*\([^)]*\)\s*;",
            f"extern void (*{sym})({params});",
            txt,
        )
    Path(base_c).write_bytes(txt.replace("\r\n", "\n").encode("utf-8"))


if __name__ == "__main__":
    sys.exit(main())

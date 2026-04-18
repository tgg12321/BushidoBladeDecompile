import argparse
import shutil
import subprocess
import sys

from typing import List
from pathlib import Path

from maspsx import MaspsxProcessor


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--aspsx-version", type=str)
    parser.add_argument("--run-assembler", action="store_true")
    parser.add_argument("--gnu-as-path", default="mipsel-linux-gnu-as")
    parser.add_argument("--dont-force-G0", action="store_true")
    parser.add_argument("--expand-div", action="store_true")
    parser.add_argument("--expand-lb", action="store_true")
    parser.add_argument("--expand-lh", action="store_true")
    parser.add_argument("--expand-lb-funcs", type=str, default=None,
                        help="Path to file listing functions that need lb expansion (when set, --expand-lb only applies to these)")
    parser.add_argument("--expand-lh-funcs", type=str, default=None,
                        help="Path to file listing functions that need lh expansion (when set, --expand-lh only applies to these)")
    parser.add_argument("--macro-inc", action="store_true")
    parser.add_argument("--dont-expand-li", action="store_true")
    parser.add_argument("--force-stdin", action="store_true")
    parser.add_argument("--use-comm-section", action="store_true")
    parser.add_argument("--use-comm-for-lcomm", action="store_true")
    parser.add_argument("--sdata-syms", type=str, default=None,
                        help="Path to file listing external .sdata symbol names (one per line)")
    parser.add_argument("--sdata-funcs", type=str, default=None,
                        help="Path to file listing function names that should use GP-relative sdata addressing")
    parser.add_argument("--sdata-exclude", type=str, default=None,
                        help="Path to file listing per-function sdata symbol exclusions (func: sym1, sym2)")
    # decomp.me debugging
    parser.add_argument("--print-output", action="store_true")
    parser.add_argument("--print-input", action="store_true")
    # deprecated
    parser.add_argument("--no-macro-inc", action="store_true")
    parser.add_argument("--expand-li", action="store_true")

    args, as_args = parser.parse_known_args()

    if args.no_macro_inc:
        sys.stderr.write(
            "MASPSX: --no-macro-inc is no longer required and will be removed in a future update\n"
        )

    if args.expand_li:
        sys.stderr.write(
            "MASPSX: --expand-li is enabled automatically if --aspsx-version is below 2.56\n"
        )

    read_from_file = sys.stdin.isatty()

    if not read_from_file:
        in_lines = sys.stdin.readlines()
        if len(in_lines) == 0:
            if args.force_stdin:
                sys.stderr.write("MASPSX: --force-stdin but no input from stdin!\n")
                sys.exit(1)
            else:
                sys.stderr.write(
                    "MASPSX: Warning, no input from stdin, will try to read from a file\n"
                )
                read_from_file = True

    if read_from_file:
        try:
            input_file = as_args.pop()
        except IndexError:
            sys.stderr.write("MASPSX: Error, no input file found!\n")
            sys.exit(1)

        with open(input_file, "r", encoding="utf") as f:
            in_lines = f.readlines()

    if args.print_input:
        sys.stderr.write("".join(in_lines))

    preamble = [
        '.include "macro.inc"' if args.macro_inc else "",
    ]

    sdata_limit = 0
    filtered_as_args: List[str] = []
    for arg in as_args:
        # Can we stop gcc from passing us this flag?
        if arg == "-KPIC":
            continue

        # GNU as does not support -mcpu flag
        if arg.startswith("-mcpu="):
            arg = arg.replace("-mcpu=", "-mtune=")

        elif arg.startswith("-G") and len(arg) > 2:
            sdata_limit = int(arg[2:])

        filtered_as_args.append(arg)

    div_uses_tge = False  # use tge instruction instead of break in divide
    nop_at_expansion = False  # insert nop between v0/at?
    nop_mflo_mfhi = True  # ensure 2 ops between mfhi/mflo and div/mult
    sltu_at = True  # sltu uses at?
    expand_li = True  # turn li into lui/ori
    gp_allow_offset = False  # use gp for sym+offset?
    gp_allow_la = False  # use gp for la
    addiu_at = False  # use addiu when expanding lw to use $at

    if args.aspsx_version:
        aspsx_version = tuple(int(x) for x in args.aspsx_version.split("."))
        if (1, 10) < aspsx_version < (2, 10):
            div_uses_tge = True
        if aspsx_version < (2, 30):
            nop_at_expansion = True
            nop_mflo_mfhi = False
            addiu_at = True
        if aspsx_version >= (2, 50):
            expand_li = False
        if aspsx_version >= (2, 70):
            sltu_at = False
            gp_allow_offset = True
        if aspsx_version >= (2, 80):
            gp_allow_la = True

    if args.dont_expand_li and expand_li:
        expand_li = False

    # Load external .sdata symbol list if provided
    sdata_sym_list = []
    if args.sdata_syms:
        with open(args.sdata_syms, "r", encoding="utf") as f:
            sdata_sym_list = [line.strip() for line in f if line.strip() and not line.startswith("#")]

    # Load per-function sdata list if provided
    sdata_func_list = []
    if args.sdata_funcs:
        with open(args.sdata_funcs, "r", encoding="utf") as f:
            sdata_func_list = [line.strip() for line in f if line.strip() and not line.startswith("#")]

    # Load per-function sdata exclusions if provided
    # Format: func_name: sym1, sym2, sym3
    sdata_exclude_map = {}
    if args.sdata_exclude:
        with open(args.sdata_exclude, "r", encoding="utf") as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                if ":" in line:
                    func_name, syms = line.split(":", 1)
                    func_name = func_name.strip()
                    sym_set = set(s.strip() for s in syms.split(",") if s.strip())
                    if func_name in sdata_exclude_map:
                        sdata_exclude_map[func_name].update(sym_set)
                    else:
                        sdata_exclude_map[func_name] = sym_set

    # Load per-function lb/lh expansion lists
    expand_lb_func_list = []
    if args.expand_lb_funcs:
        with open(args.expand_lb_funcs, "r", encoding="utf") as f:
            expand_lb_func_list = [line.strip() for line in f if line.strip() and not line.startswith("#")]

    expand_lh_func_list = []
    if args.expand_lh_funcs:
        with open(args.expand_lh_funcs, "r", encoding="utf") as f:
            expand_lh_func_list = [line.strip() for line in f if line.strip() and not line.startswith("#")]

    maspsx_processor = MaspsxProcessor(
        in_lines,
        sdata_limit=sdata_limit,
        expand_div=args.expand_div,
        expand_lb=args.expand_lb,
        expand_lh=args.expand_lh,
        expand_li=expand_li,
        nop_at_expansion=nop_at_expansion,
        nop_mflo_mfhi=nop_mflo_mfhi,
        sltu_at=sltu_at,
        addiu_at=addiu_at,
        div_uses_tge=div_uses_tge,
        gp_allow_offset=gp_allow_offset,
        gp_allow_la=gp_allow_la,
        use_comm_section=args.use_comm_section,
        use_comm_for_lcomm=args.use_comm_for_lcomm,
        expand_lb_func_list=expand_lb_func_list,
        expand_lh_func_list=expand_lh_func_list,
        sdata_sym_list=sdata_sym_list,
        sdata_func_list=sdata_func_list,
        sdata_exclude_map=sdata_exclude_map,
    )
    try:
        out_lines = maspsx_processor.process_lines()
    except Exception as err:
        sys.stderr.write(f"MASPSX: An exception occurred: {err}\n")
        sys.exit(1)

    out_text = "\n".join(preamble + out_lines)

    # avoid "Warning: end of file not at end of a line; newline inserted"
    out_text += "\n"

    if args.print_output:
        sys.stderr.write(out_text)

    if args.run_assembler:
        gnu_as_path = Path(args.gnu_as_path)
        if not gnu_as_path.is_file() and not shutil.which(args.gnu_as_path):
            sys.stderr.write(f"MASPSX: {args.gnu_as_path} not found")
            sys.exit(1)

        cmd = [
            args.gnu_as_path,
            *filtered_as_args,
            "-",  # read from stdin
        ]
        if not args.dont_force_G0:
            cmd.insert(-1, "-G0")

        with subprocess.Popen(
            cmd, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE
        ) as process:
            out_bytes = out_text.encode("utf")
            stdout, stderr = process.communicate(input=out_bytes)
            if len(stdout):
                sys.stdout.write(stdout.decode("utf"))
            if len(stderr):
                sys.stderr.write(stderr.decode("utf"))
    else:
        sys.stdout.write(out_text)


if __name__ == "__main__":
    main()

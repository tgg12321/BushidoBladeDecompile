#!/usr/bin/env python3
"""Debug tool: dump line-by-line (position, idx) for a function through regfix phases.

Usage:
    python3 tools/regfix_debug.py <func_name> [--phase {init,subst,delete,insert,insert_after,reorder<N>,final}] [--range START-END]

Runs the full pipeline (cpp | cc1 | prologue_fix | maspsx) for the source file
containing the function, then applies regfix phases step-by-step. After each
phase, dumps each line with both POSITION (= sequential position in output) and
IDX (= regfix's stable label, used in reorder rules).

This lets you see exactly which IDX corresponds to which line at any point in
the pipeline, which is essential for writing correct reorder rules when you
have multiple overlapping reorders.

The default --phase is 'all' which prints the dump after EVERY phase.
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
ROOT = SCRIPT_DIR.parent

# Import functions from regfix.py
sys.path.insert(0, str(SCRIPT_DIR))
import regfix  # noqa: E402


def find_source_file(func_name):
    """Use dump_text_indices logic to find source file."""
    src_dir = ROOT / "src"
    decl_re = re.compile(
        rf'^\s*(?:static\s+)?(?:const\s+)?(?:[\w_]+\s*\*?\s+){{1,3}}'
        rf'{re.escape(func_name)}\s*\([^;]*\)\s*\{{',
        re.MULTILINE,
    )
    asm_re = re.compile(rf'INCLUDE_ASM\s*\(\s*"[^"]+"\s*,\s*{re.escape(func_name)}\s*\)')
    for c_file in sorted(src_dir.glob("*.c")):
        text = c_file.read_text()
        if func_name not in text:
            continue
        if decl_re.search(text) or asm_re.search(text):
            return c_file
    return None


def run_pipeline_to_maspsx(c_file):
    """Run cpp | cc1 | prologue_fix | maspsx via stages, capturing each."""
    # Stage 1: cpp
    r1 = subprocess.run(
        ['mipsel-linux-gnu-cpp', '-Iinclude', '-undef', '-Wall', '-lang-c',
         '-fno-builtin', '-Dmips', '-D__GNUC__=2', '-D__OPTIMIZE__',
         '-D__mips__', '-D__mips', '-Dpsx', '-D__psx__', '-D__psx',
         str(c_file)],
        capture_output=True, cwd=str(ROOT))
    cpp_out = r1.stdout

    # Stage 2: cc1 (may return non-zero on warnings; ignore exit)
    r2 = subprocess.run(
        ['tools/gcc-2.7.2/build/cc1', '-O2', '-G0', '-funsigned-char',
         '-quiet', '-mcpu=3000', '-mips1', '-mno-abicalls', '-fno-builtin', '-w'],
        input=cpp_out, capture_output=True, cwd=str(ROOT))
    cc1_out = r2.stdout
    if not cc1_out:
        sys.exit(f"cc1 produced no output")

    # Stage 3: prologue_fix
    r3 = subprocess.run(
        ['python3', 'tools/prologue_fix.py'],
        input=cc1_out, capture_output=True, cwd=str(ROOT))
    prologue_out = r3.stdout

    # Stage 4: maspsx (text input)
    prologue_text = prologue_out.decode(errors='replace')
    r4 = subprocess.run(
        ['python3', 'tools/maspsx/maspsx.py',
         '--expand-div', '--aspsx-version=2.34',
         '--sdata-syms=sdata_syms.txt',
         '--sdata-funcs=sdata_funcs.txt',
         '--sdata-exclude=sdata_exclude.txt',
         '--expand-lb', '--expand-lb-funcs=expand_lb_funcs.txt',
         '--multu-funcs=multu_funcs.txt',
         '--expand-dest-funcs=expand_dest_funcs.txt'],
        input=prologue_text, capture_output=True, text=True, cwd=str(ROOT))
    if not r4.stdout:
        sys.exit(f"maspsx failed: {r4.stderr[:500]}")
    return r4.stdout


def extract_func_lines(asm_text, func_name):
    """Find function in asm text, return (text, idx_or_None) tuples like regfix uses."""
    func_lines = []
    in_func = False
    insn_idx = 0
    for line in asm_text.splitlines(keepends=True):
        stripped = line.strip()
        if not in_func:
            if re.match(rf'^{func_name}:$', stripped):
                in_func = True
                func_lines.append((line, None))
                continue
        else:
            end_match = re.match(rf'^\s*\.end\s+{func_name}', stripped)
            if regfix.is_instruction(stripped):
                func_lines.append((line, insn_idx))
                insn_idx += 1
            else:
                func_lines.append((line, None))
            if end_match:
                break
    return func_lines


def dump_state(lines, label, range_filter=None):
    """Print position, idx, and instruction text for each line."""
    print(f"\n=== {label} ===")
    pos = 0
    for text, idx in lines:
        text_strip = text.rstrip()
        if idx is not None:
            if range_filter is None or range_filter[0] <= pos <= range_filter[1]:
                print(f"  pos {pos:4d} | idx {idx:4d} | {text_strip}")
            pos += 1
        else:
            # Non-instruction lines (labels, comments, directives)
            if range_filter is None or (pos > 0 and range_filter[0] <= pos - 1 <= range_filter[1] + 5):
                print(f"  pos  --- | idx  --- | {text_strip}")


def apply_phases_with_dump(lines, func_config, phase_filter, range_filter):
    """Run regfix phases, dumping after each."""
    swap_list = func_config.get('swaps', [])
    subst_list = func_config.get('substs', [])
    fill_delay_list = func_config.get('fill_delays', [])
    drain_delay_list = func_config.get('drain_delays', [])
    delete_list = list(func_config.get('deletes', []))
    insert_list = list(func_config.get('inserts', []))
    insert_after_list = list(func_config.get('insert_afters', []))
    insert_label_list = list(func_config.get('insert_labels', []))
    reorder_list = func_config.get('reorders', [])

    show = lambda phase: phase_filter == 'all' or phase_filter == phase

    if show('init'):
        dump_state(lines, "PHASE 0: initial (post-maspsx)", range_filter)

    # Phase 1: Swaps
    if swap_list:
        new_lines = []
        for text, idx in lines:
            if idx is not None:
                text = regfix.swap_registers_in_line(text, swap_list, idx)
            new_lines.append((text, idx))
        lines = new_lines
        if show('swap'):
            dump_state(lines, "PHASE 1: after swaps", range_filter)

    # Phase 2: Substs (use original indices)
    if subst_list:
        new_lines = []
        for text, idx in lines:
            if idx is not None:
                for subst_idx, pattern, replacement in subst_list:
                    if idx == subst_idx:
                        text = re.sub(pattern, lambda m: replacement, text, count=1)
            new_lines.append((text, idx))
        lines = new_lines
        if show('subst'):
            dump_state(lines, "PHASE 2: after substs", range_filter)

    # Phase 3: fill_delay (skipped for simplicity in debug — rare)

    # Phase 4: Deletes (sorted descending)
    extra_deletes = []
    if delete_list:
        for del_idx in sorted(delete_list + extra_deletes, reverse=True):
            del_pos = None
            for pos, (text, idx) in enumerate(lines):
                if idx == del_idx:
                    del_pos = pos
                    break
            if del_pos is None:
                continue
            lines.pop(del_pos)
            renumbered = []
            for text, idx in lines:
                if idx is not None and idx > del_idx:
                    renumbered.append((text, idx - 1))
                else:
                    renumbered.append((text, idx))
            lines = renumbered
        if show('delete'):
            dump_state(lines, "PHASE 4: after deletes", range_filter)

    # Phase 5: Inserts (sorted descending)
    if insert_list:
        for insert_idx, asm_text in sorted(insert_list, key=lambda x: x[0], reverse=True):
            insert_pos = None
            for pos, (text, idx) in enumerate(lines):
                if idx == insert_idx:
                    insert_pos = pos
                    break
            if insert_pos is None:
                continue
            new_line = f"\t{asm_text}\n"
            lines.insert(insert_pos, (new_line, insert_idx))
            renumbered = []
            for i, (text, idx) in enumerate(lines):
                if i == insert_pos:
                    renumbered.append((text, idx))
                elif idx is not None and idx >= insert_idx:
                    renumbered.append((text, idx + 1))
                else:
                    renumbered.append((text, idx))
            lines = renumbered
        if show('insert'):
            dump_state(lines, "PHASE 5: after inserts", range_filter)

    # Phase 6: insert_afters
    if insert_after_list:
        # Group by idx to handle multiple at same point — they accumulate in order
        for insert_idx, asm_text in insert_after_list:
            target_pos = None
            for pos, (text, idx) in enumerate(lines):
                if idx == insert_idx:
                    target_pos = pos
                    break
            if target_pos is None:
                continue
            insert_pos = target_pos + 1
            new_line = f"\t{asm_text}\n"
            new_idx = insert_idx + 1
            lines.insert(insert_pos, (new_line, new_idx))
            renumbered = []
            for i, (text, idx) in enumerate(lines):
                if i == insert_pos:
                    renumbered.append((text, idx))
                elif idx is not None and idx >= new_idx:
                    renumbered.append((text, idx + 1))
                else:
                    renumbered.append((text, idx))
            lines = renumbered
        if show('insert_after'):
            dump_state(lines, "PHASE 6: after insert_afters", range_filter)

    # Phase 7: insert_labels (no idx shift)
    if insert_label_list:
        for insert_idx, label_text in insert_label_list:
            target_pos = None
            for pos, (text, idx) in enumerate(lines):
                if idx == insert_idx:
                    target_pos = pos
                    break
            if target_pos is None:
                continue
            new_line = f"{label_text}\n"
            lines.insert(target_pos + 1, (new_line, None))
        if show('insert_label'):
            dump_state(lines, "PHASE 7: after insert_labels", range_filter)

    # Phase 8: Reorders (each one in sequence)
    for ri, (start, end, new_order) in enumerate(reorder_list):
        # Build idx -> position map for current state
        idx_to_pos = {}
        for pos, (text, idx) in enumerate(lines):
            if idx is not None and start <= idx <= end:
                idx_to_pos[idx] = pos
        positions = sorted(idx_to_pos.values())
        if not positions:
            continue
        # Build groups (each instruction + preceding non-insn lines)
        groups = {}
        preamble_lines = []
        for i, insn_pos in enumerate(positions):
            if i == 0:
                group_start = insn_pos
                # Walk back over None-idx lines
                while group_start > 0 and lines[group_start - 1][1] is None:
                    group_start -= 1
                if start == 0:
                    preamble_lines = [lines[p] for p in range(group_start, insn_pos)]
                    group_start = insn_pos
            else:
                group_start = positions[i - 1] + 1
            insn_idx_val = lines[insn_pos][1]
            groups[insn_idx_val] = [lines[p] for p in range(group_start, insn_pos + 1)]

        if start == 0 and preamble_lines:
            span_start = positions[0]
            while span_start > 0 and lines[span_start - 1][1] is None:
                span_start -= 1
        else:
            first_group = groups[lines[positions[0]][1]]
            span_start = None
            for pos in range(len(lines)):
                if lines[pos] is first_group[0]:
                    span_start = pos
                    break
            if span_start is None:
                span_start = positions[0]
        span_end = positions[-1] + 1

        reordered_section = list(preamble_lines)
        for new_idx in new_order:
            if new_idx not in groups:
                print(f"  WARNING: reorder #{ri} references missing idx {new_idx}", file=sys.stderr)
                continue
            reordered_section.extend(groups[new_idx])

        lines = lines[:span_start] + reordered_section + lines[span_end:]

        if show('reorder') or show(f'reorder{ri}'):
            dump_state(lines, f"PHASE 8.{ri}: after reorder #{ri} ({start}-{end} = {new_order})", range_filter)

    if show('final'):
        dump_state(lines, "PHASE 9: FINAL", range_filter)

    return lines


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('func_name')
    parser.add_argument('--phase', default='final',
                        help="which phase to dump: init,subst,delete,insert,insert_after,insert_label,reorder,reorderN,final,all")
    parser.add_argument('--range', default=None,
                        help="position range to filter, e.g. '100-160'")
    parser.add_argument('--config', default='regfix.txt',
                        help="path to regfix config file")
    args = parser.parse_args()

    range_filter = None
    if args.range:
        m = re.match(r'(\d+)-(\d+)', args.range)
        if m:
            range_filter = (int(m.group(1)), int(m.group(2)))

    c_file = find_source_file(args.func_name)
    if c_file is None:
        sys.exit(f"Could not find source file for {args.func_name}")
    print(f"# Found {args.func_name} in {c_file.name}")

    asm = run_pipeline_to_maspsx(c_file)

    config = regfix.load_config(ROOT / args.config)
    func_config = config.get(args.func_name)
    if func_config is None:
        sys.exit(f"No regfix config for {args.func_name}")
    func_config['__name__'] = args.func_name

    lines = extract_func_lines(asm, args.func_name)
    if not lines:
        sys.exit(f"Function {args.func_name} not found in maspsx output")

    apply_phases_with_dump(lines, func_config, args.phase, range_filter)


if __name__ == '__main__':
    main()

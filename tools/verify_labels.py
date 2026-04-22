#!/usr/bin/env python3
r"""Verify regfix label subst rules resolve to the correct target instructions.

When a new function is added to a .c file, GCC's file-wide .L<N> label counter
shifts, breaking regfix subst rules that reference literal label names. This tool
checks every label-referencing regfix subst by:

1. Running the build pipeline to get the post-maspsx asm (pre-regfix)
2. Finding all subst rules whose replacement contains a .L<N> label
3. Resolving each label to its actual instruction in the asm
4. Printing a summary so the user can verify branch targets are correct

Usage:
    python3 tools/verify_labels.py                    # check all functions with label rules
    python3 tools/verify_labels.py --func FUNC        # check one function
    python3 tools/verify_labels.py --func FUNC --all  # show all labels in function, not just regfix ones

Example output:
    saEft00Add:
      Rule line 191: subst "\.L161" ".L161+4" @ 25
        Instruction @ 25: bne  $2,$0,.L161
        .L161 resolves to idx 26: lw  $3,0($16)   <-- verify this is the correct branch target
      Rule line 194: subst "\.L169" ".L169+4" @ 92
        Instruction @ 92: beq  $2,$0,.L169
        .L169 resolves to idx 93: jr  $31           <-- verify this is the correct branch target
"""

import re
import subprocess
import sys
from pathlib import Path


def find_source_file(root, func_name):
    """Find which src/*.c file contains a function."""
    src_dir = root / "src"
    decl_re = re.compile(rf'^[^/\n]*\b{re.escape(func_name)}\s*\([^;]*\)\s*\{{', re.MULTILINE)
    asm_re = re.compile(rf'INCLUDE_ASM\s*\(\s*"[^"]+"\s*,\s*{re.escape(func_name)}\s*\)')

    for c_file in sorted(src_dir.glob("*.c")):
        text = c_file.read_text()
        if func_name not in text:
            continue
        if decl_re.search(text) or asm_re.search(text):
            return c_file
    return None


def get_full_asm(root, func_name, src_file=None):
    """Run the build pipeline and return the full asm output for a function,
    including labels and directives (not just instructions)."""
    sys.path.insert(0, str(root / "tools"))
    from dump_text_indices import build_pipeline_cmd, find_source_file as dti_find

    if src_file is None:
        src_file = dti_find(root, func_name)
        if src_file is None:
            return None, f"{func_name} not found in any src/*.c"

    cmd = build_pipeline_cmd(root, src_file)
    result = subprocess.run(
        ["bash", "-c", cmd],
        capture_output=True, text=True, cwd=str(root)
    )

    if not result.stdout.strip():
        return None, f"Pipeline failed: {result.stderr[:500]}"

    asm_text = result.stdout
    in_func = False
    lines = []
    label_pattern = re.compile(rf'^{re.escape(func_name)}:$')

    for line in asm_text.splitlines():
        s = line.strip()
        if label_pattern.match(s):
            in_func = True
            continue
        if in_func:
            if re.match(rf'^\s*\.end\s+{re.escape(func_name)}', s):
                break
            lines.append(s)

    return lines, None


def is_instruction(line):
    s = line.strip()
    if not s:
        return False
    if s.startswith('.') or s.startswith('#') or s.endswith(':'):
        return False
    if s.startswith('gcc2_compiled'):
        return False
    return True


def build_label_map(asm_lines):
    """Build a map from label name -> instruction index of the NEXT instruction after the label.
    Also build idx -> instruction text map."""
    label_map = {}
    idx_map = {}
    idx = 0
    pending_labels = []

    for line in asm_lines:
        s = line.strip()
        if s.endswith(':') and not s.startswith('.'):
            # Skip non-dot labels (function names, etc.)
            pass
        elif s.endswith(':'):
            label_name = s[:-1]
            pending_labels.append(label_name)
        elif is_instruction(s):
            for lbl in pending_labels:
                label_map[lbl] = idx
            pending_labels = []
            idx_map[idx] = s
            idx += 1

    return label_map, idx_map


def parse_label_rules(regfix_path):
    """Parse regfix.txt and return rules that reference .L<N> labels in their
    pattern or replacement."""
    rules = {}
    label_re = re.compile(r'\.L\d+')

    for lineno, line in enumerate(regfix_path.read_text().splitlines(), 1):
        stripped = line.strip()
        if not stripped or stripped.startswith('#'):
            continue

        m = re.match(r'(\w+)\s*:\s*subst\s+"([^"]+)"\s+"([^"]*)"\s*@\s*(\d+)', stripped)
        if not m:
            continue

        func = m.group(1)
        pattern = m.group(2)
        replacement = m.group(3)
        index = int(m.group(4))

        labels_in_pattern = label_re.findall(pattern)
        labels_in_replacement = label_re.findall(replacement)

        if labels_in_pattern or labels_in_replacement:
            rules.setdefault(func, []).append({
                'lineno': lineno,
                'pattern': pattern,
                'replacement': replacement,
                'index': index,
                'raw': stripped,
                'labels_pattern': labels_in_pattern,
                'labels_replacement': labels_in_replacement,
            })

    return rules


def verify_function(root, func_name, label_rules, show_all_labels=False):
    """Verify label rules for one function. Returns (ok_count, error_count, warnings)."""
    asm_lines, err = get_full_asm(root, func_name)
    if asm_lines is None:
        print(f"  SKIP: {err}")
        return 0, 0, []

    label_map, idx_map = build_label_map(asm_lines)
    ok = 0
    errors = 0
    warnings = []

    if show_all_labels:
        print(f"  All labels in {func_name}:")
        for lbl in sorted(label_map.keys(), key=lambda x: label_map[x]):
            idx = label_map[lbl]
            insn = idx_map.get(idx, "<no instruction>")
            print(f"    {lbl}: -> idx {idx}: {insn}")
        print()

    for rule in label_rules:
        idx = rule['index']
        insn_at_idx = idx_map.get(idx, None)

        print(f"  Rule line {rule['lineno']}: {rule['raw']}")
        if insn_at_idx:
            print(f"    Instruction @ {idx}: {insn_at_idx}")
        else:
            print(f"    WARNING: index {idx} not found in function")
            errors += 1
            continue

        # Check pattern labels exist in the instruction
        for lbl in rule['labels_pattern']:
            if lbl not in insn_at_idx:
                print(f"    ERROR: pattern label {lbl} not in instruction at idx {idx}")
                print(f"           This label may have drifted — check current asm")
                errors += 1
            else:
                print(f"    Pattern label {lbl} found in instruction — OK")
                ok += 1

        # Check replacement labels resolve to valid targets
        for lbl_ref in rule['labels_replacement']:
            # Strip any +N/-N offset from the label for lookup
            base_lbl = re.match(r'(\.L\d+)', lbl_ref).group(1)
            if base_lbl in label_map:
                target_idx = label_map[base_lbl]
                target_insn = idx_map.get(target_idx, "<no instruction>")
                print(f"    Replacement {lbl_ref} resolves to idx {target_idx}: {target_insn}")
                ok += 1
            else:
                print(f"    ERROR: replacement label {base_lbl} not found in function asm")
                print(f"           Available labels: {', '.join(sorted(label_map.keys(), key=lambda x: label_map[x]))}")
                errors += 1

    return ok, errors, warnings


def main():
    import argparse
    parser = argparse.ArgumentParser(description="Verify regfix label references")
    parser.add_argument('--func', type=str, help='Check only this function')
    parser.add_argument('--all', action='store_true',
                        help='Show all labels in the function, not just regfix ones')
    args = parser.parse_args()

    root = Path(__file__).resolve().parent.parent
    regfix_path = root / 'regfix.txt'

    if not regfix_path.exists():
        print("No regfix.txt found")
        sys.exit(0)

    label_rules = parse_label_rules(regfix_path)
    if not label_rules:
        print("No label-referencing subst rules found in regfix.txt")
        sys.exit(0)

    total_ok = 0
    total_err = 0

    funcs_to_check = [args.func] if args.func else sorted(label_rules.keys())
    for func in funcs_to_check:
        if func not in label_rules:
            print(f"{func}: no label rules found")
            continue

        print(f"\n{func}:")
        ok, errs, _ = verify_function(root, func, label_rules[func], show_all_labels=args.all)
        total_ok += ok
        total_err += errs

    print(f"\n{'='*40}")
    print(f"Label checks: {total_ok} OK, {total_err} ERROR(s)")
    if total_err > 0:
        sys.exit(1)


if __name__ == '__main__':
    main()

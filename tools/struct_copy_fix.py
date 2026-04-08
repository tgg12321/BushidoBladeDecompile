#!/usr/bin/env python3
"""
struct_copy_fix.py - Detect and replace consecutive word copies with struct assignments.

GCC 2.7.2 compiles individual field copies as serialized load-store pairs, but
struct assignments as batched loads then stores. Many BB2 functions use struct
copies (3-4 word memcpy-like patterns), so detecting these in m2c output and
converting them to struct assignments is critical for matching.

Usage:
    python3 tools/struct_copy_fix.py input.c                  # print fixed C to stdout
    python3 tools/struct_copy_fix.py input.c -o output.c      # write to file
    python3 tools/struct_copy_fix.py input.c --dry-run         # report only, no changes
    python3 tools/struct_copy_fix.py input.c --asm target.s    # verify against target asm

Detects patterns:
  1. *(s32 *)((u8 *)dst + 0x2C) = *(s32 *)((u8 *)src + 0);
  2. M2C_FIELD(dst, s32 *, 0x2C) = M2C_FIELD(src, s32 *, 0);
  3. dst[11] = src[0];  (word-indexed)
"""

import re
import sys
import argparse
from typing import List, Tuple, Optional, NamedTuple


class FieldCopy(NamedTuple):
    """A single word-sized field copy extracted from a line of C."""
    dst_base: str       # destination base expression
    dst_offset: int     # byte offset into destination
    src_base: str       # source base expression
    src_offset: int     # byte offset into source
    line_idx: int       # line index in the source
    original: str       # original line text


class CopyGroup(NamedTuple):
    """A group of consecutive field copies that form a struct copy."""
    dst_base: str
    dst_start: int      # byte offset of first destination field
    src_base: str
    src_start: int      # byte offset of first source field
    n_words: int        # number of words copied
    copies: List[FieldCopy]


# Pattern 1: *(s32 *)((u8 *)VAR + OFF) = *(s32 *)((u8 *)VAR2 + OFF2);
# Also handles: *(s32 *)((u8 *)VAR + OFF) = VAR2[IDX];
CAST_STORE_RE = re.compile(
    r'^\s*\*\s*\(\s*s32\s*\*\s*\)\s*\(\s*\(\s*u8\s*\*\s*\)\s*(\w+)\s*\+\s*(0x[0-9a-fA-F]+|\d+)\s*\)'
)
CAST_LOAD_RE = re.compile(
    r'=\s*(?:\(s32\)\s*)?\*\s*\(\s*s32\s*\*\s*\)\s*\(\s*\(\s*u8\s*\*\s*\)\s*(\w+)\s*\+\s*(0x[0-9a-fA-F]+|\d+)\s*\)\s*;'
)
# Source as array index: VAR[IDX]
ARRAY_LOAD_RE = re.compile(
    r'=\s*(?:\(s32\)\s*)?(\w+)\s*\[\s*(\d+)\s*\]\s*;'
)

# Pattern 2: M2C_FIELD(VAR, s32 *, OFF) = [cast] M2C_FIELD(VAR2, s32 *, OFF2);
M2C_STORE_RE = re.compile(
    r'^\s*M2C_FIELD\(\s*([^,]+?)\s*,\s*s32\s*\*\s*,\s*(0x[0-9a-fA-F]+|\d+)\s*\)'
)
M2C_LOAD_RE = re.compile(
    r'=\s*(?:\(s32\)\s*)?M2C_FIELD\(\s*([^,]+?)\s*,\s*s32\s*\*\s*,\s*(0x[0-9a-fA-F]+|\d+)\s*\)\s*;'
)

# Pattern 3: direct array: dst[N] = src[M]; (word-indexed, consecutive)
ARRAY_STORE_RE = re.compile(
    r'^\s*(\w+)\s*\[\s*(0x[0-9a-fA-F]+|\d+)\s*\]'
)
ARRAY_LOAD_SIMPLE_RE = re.compile(
    r'=\s*(?:\(s32\)\s*)?(\w+)\s*\[\s*(0x[0-9a-fA-F]+|\d+)\s*\]\s*;'
)


def parse_int(s: str) -> int:
    """Parse decimal or hex integer."""
    s = s.strip()
    if s.startswith('0x') or s.startswith('0X'):
        return int(s, 16)
    return int(s)


def try_parse_field_copy(line: str, line_idx: int) -> Optional[FieldCopy]:
    """Try to parse a line as a word-sized field copy. Returns FieldCopy or None."""

    # Try cast pattern: *(s32 *)((u8 *)dst + OFF) = ...
    dst_m = CAST_STORE_RE.search(line)
    if dst_m:
        dst_base = dst_m.group(1)
        dst_off = parse_int(dst_m.group(2))
        # Try cast source
        src_m = CAST_LOAD_RE.search(line)
        if src_m:
            return FieldCopy(dst_base, dst_off, src_m.group(1), parse_int(src_m.group(2)), line_idx, line)
        # Try array source
        src_m = ARRAY_LOAD_RE.search(line)
        if src_m:
            return FieldCopy(dst_base, dst_off, src_m.group(1), parse_int(src_m.group(2)) * 4, line_idx, line)
        return None

    # Try M2C_FIELD pattern
    dst_m = M2C_STORE_RE.search(line)
    if dst_m:
        dst_base = dst_m.group(1).strip()
        dst_off = parse_int(dst_m.group(2))
        src_m = M2C_LOAD_RE.search(line)
        if src_m:
            return FieldCopy(dst_base, dst_off, src_m.group(1).strip(), parse_int(src_m.group(2)), line_idx, line)
        return None

    # Try simple array pattern: dst[N] = src[M];
    dst_m = ARRAY_STORE_RE.search(line)
    if dst_m:
        src_m = ARRAY_LOAD_SIMPLE_RE.search(line)
        if src_m and '(' not in line:  # avoid matching cast patterns
            dst_base = dst_m.group(1)
            dst_idx = parse_int(dst_m.group(2))
            src_base = src_m.group(1)
            src_idx = parse_int(src_m.group(2))
            if dst_base != src_base:  # must be different base pointers
                return FieldCopy(dst_base, dst_idx * 4, src_base, src_idx * 4, line_idx, line)

    return None


def build_temp_var_map(lines: List[str]) -> dict:
    """Build a map of temp variable assignments from M2C_FIELD or cast loads.

    Detects patterns like:
        temp_a0 = M2C_FIELD(arg1, s32 *, 8);
        temp_a0 = *(s32 *)((u8 *)arg1 + 8);
    Returns: {var_name: (base, byte_offset)} for single-assignment temps.
    """
    # M2C_FIELD: temp = [cast] M2C_FIELD(base, s32 *, off);
    m2c_assign = re.compile(
        r'^\s*(\w+)\s*=\s*(?:\(s32\)\s*)?M2C_FIELD\(\s*([^,]+?)\s*,\s*s32\s*\*\s*,\s*(0x[0-9a-fA-F]+|\d+)\s*\)\s*;'
    )
    # Cast: temp = [cast] *(s32 *)((u8 *)base + off);
    cast_assign = re.compile(
        r'^\s*(\w+)\s*=\s*(?:\(s32\)\s*)?\*\s*\(\s*s32\s*\*\s*\)\s*\(\s*\(\s*u8\s*\*\s*\)\s*([^+]+?)\s*\+\s*(0x[0-9a-fA-F]+|\d+)\s*\)\s*;'
    )
    # Array: temp = [cast] base[idx];
    arr_assign = re.compile(
        r'^\s*(\w+)\s*=\s*(?:\(s32\)\s*)?(\w+)\s*\[\s*(0x[0-9a-fA-F]+|\d+)\s*\]\s*;'
    )

    assignments = {}  # var -> (base, offset, line_idx)
    usage_count = {}  # var -> count of times used on RHS

    for i, line in enumerate(lines):
        for m, is_arr in [(m2c_assign.search(line), False),
                          (cast_assign.search(line), False),
                          (arr_assign.search(line), True)]:
            if m:
                var = m.group(1)
                base = m.group(2).strip()
                off = parse_int(m.group(3)) * (4 if is_arr else 1)
                assignments[var] = (base, off, i)
                break

    # Count usages to avoid resolving multi-use temps (they may have side effects)
    for line in lines:
        for var in assignments:
            # Count occurrences on RHS (after =) excluding the assignment line itself
            if re.search(r'=\s*.*\b' + re.escape(var) + r'\b', line):
                usage_count[var] = usage_count.get(var, 0) + 1
            # Also count as function argument
            if re.search(r'\(\s*' + re.escape(var) + r'\s*[,)]', line):
                usage_count[var] = usage_count.get(var, 0) + 1

    # Return all temp assignments. Mark usage count for cleanup decisions.
    result = {}
    for var, (base, off, lidx) in assignments.items():
        result[var] = (base, off, lidx, usage_count.get(var, 0))

    return result


def try_parse_field_copy_with_temps(line: str, line_idx: int,
                                     temp_map: dict) -> Optional[FieldCopy]:
    """Like try_parse_field_copy but also resolves temp variables on RHS."""
    # First try direct parse
    fc = try_parse_field_copy(line, line_idx)
    if fc:
        return fc

    # Try M2C_FIELD store with temp var on RHS
    dst_m = M2C_STORE_RE.search(line)
    if dst_m:
        # RHS is a temp var?
        rhs_m = re.search(r'=\s*(?:\(s32\)\s*)?(\w+)\s*;', line)
        if rhs_m and rhs_m.group(1) in temp_map:
            var = rhs_m.group(1)
            src_base, src_off, _, _ = temp_map[var]
            return FieldCopy(
                dst_m.group(1).strip(), parse_int(dst_m.group(2)),
                src_base, src_off, line_idx, line
            )

    # Try cast store with temp var on RHS
    dst_m = CAST_STORE_RE.search(line)
    if dst_m:
        rhs_m = re.search(r'=\s*(?:\(s32\)\s*)?(\w+)\s*;', line)
        if rhs_m and rhs_m.group(1) in temp_map:
            var = rhs_m.group(1)
            src_base, src_off, _, _ = temp_map[var]
            return FieldCopy(
                dst_m.group(1), parse_int(dst_m.group(2)),
                src_base, src_off, line_idx, line
            )

    return None


def find_copy_groups(lines: List[str], min_words: int = 3) -> List[CopyGroup]:
    """Find groups of consecutive field copies that form struct copies."""
    temp_map = build_temp_var_map(lines)

    copies = []
    for i, line in enumerate(lines):
        fc = try_parse_field_copy_with_temps(line, i, temp_map)
        if fc:
            copies.append(fc)

    groups = []
    i = 0
    while i < len(copies):
        # Start a potential group
        run = [copies[i]]
        j = i + 1
        while j < len(copies):
            prev = run[-1]
            curr = copies[j]
            # Must be consecutive lines (allow no gaps)
            if curr.line_idx != prev.line_idx + 1:
                break
            # Same base pointers
            if curr.dst_base != prev.dst_base or curr.src_base != prev.src_base:
                break
            # Consecutive offsets (4 bytes apart)
            if curr.dst_offset != prev.dst_offset + 4:
                break
            if curr.src_offset != prev.src_offset + 4:
                break
            run.append(curr)
            j += 1

        if len(run) >= min_words:
            groups.append(CopyGroup(
                dst_base=run[0].dst_base,
                dst_start=run[0].dst_offset,
                src_base=run[0].src_base,
                src_start=run[0].src_offset,
                n_words=len(run),
                copies=run,
            ))
            i = j
        else:
            i += 1

    return groups


def verify_against_asm(asm_path: str, groups: List[CopyGroup]) -> List[CopyGroup]:
    """Filter groups to only those confirmed by batched lw/sw in target asm."""
    with open(asm_path) as f:
        asm_lines = f.readlines()

    # Named register pattern for lw/sw detection
    REG = r'\$(?:v[01]|a[0-3]|t[0-9]|s[0-7]|at|k[01]|gp|sp|fp|ra|zero)'
    LW_RE = re.compile(r'lw\s+(' + REG + r'),\s*\S+\((' + REG + r')\)')
    SW_RE = re.compile(r'sw\s+(' + REG + r'),\s*\S+\((' + REG + r')\)')

    insn_lines = [l for l in asm_lines if '/*' in l and '*/' in l]

    # Check for batched lw/sw patterns
    has_batch = False
    for i in range(len(insn_lines) - 5):
        lw_run = []
        for j in range(i, min(i + 6, len(insn_lines))):
            m = LW_RE.search(insn_lines[j])
            if m:
                lw_run.append(m)
            else:
                break
        if len(lw_run) < 3:
            continue
        # Check for following sw run
        sw_start = i + len(lw_run)
        sw_run = []
        for j in range(sw_start, min(sw_start + 6, len(insn_lines))):
            m = SW_RE.search(insn_lines[j])
            if m:
                sw_run.append(m)
            else:
                break
        if len(sw_run) >= 3:
            has_batch = True
            break

    if has_batch:
        return groups
    return []


def generate_typedef(name: str, n_words: int) -> str:
    """Generate a struct typedef for an N-word copy."""
    fields = ', '.join(f'f{i}' for i in range(n_words))
    return f'typedef struct {{ s32 {fields}; }} {name};'


def detect_line_format(copies: List[FieldCopy]) -> str:
    """Detect whether copies use cast, M2C_FIELD, or array format."""
    sample = copies[0].original
    if 'M2C_FIELD' in sample:
        return 'm2c'
    if '(u8 *)' in sample:
        return 'cast'
    return 'array'


def make_replacement(group: CopyGroup, struct_name: str) -> str:
    """Generate the struct copy replacement line."""
    fmt = detect_line_format(group.copies)
    indent = re.match(r'^(\s*)', group.copies[0].original).group(1)

    if fmt == 'cast':
        dst_off = f'0x{group.dst_start:X}' if group.dst_start else '0'
        src_off = f'0x{group.src_start:X}' if group.src_start else '0'
        return f'{indent}*({struct_name} *)((u8 *){group.dst_base} + {dst_off}) = *({struct_name} *)((u8 *){group.src_base} + {src_off});'
    elif fmt == 'm2c':
        dst_off = f'0x{group.dst_start:X}' if group.dst_start else '0'
        src_off = f'0x{group.src_start:X}' if group.src_start else '0'
        # M2C_FIELD doesn't support struct types well, use cast form
        return f'{indent}*({struct_name} *)((u8 *){group.dst_base} + {dst_off}) = *({struct_name} *)((u8 *){group.src_base} + {src_off});'
    else:
        # Array format
        dst_idx = group.dst_start // 4
        src_idx = group.src_start // 4
        return f'{indent}*({struct_name} *)&{group.dst_base}[{dst_idx}] = *({struct_name} *)&{group.src_base}[{src_idx}];'


def apply_fixes(source: str, groups: List[CopyGroup]) -> str:
    """Apply struct copy replacements to source code."""
    if not groups:
        return source

    lines = source.split('\n')
    temp_map = build_temp_var_map(lines)

    # Collect temp var assignment lines that are consumed by struct copies
    # Only remove if the temp is used nowhere else (usage_count <= 1)
    temp_lines_to_remove = set()
    temps_to_clean = set()
    for group in groups:
        for fc in group.copies:
            rhs_m = re.search(r'=\s*(?:\(s32\)\s*)?(\w+)\s*;', fc.original)
            if rhs_m and rhs_m.group(1) in temp_map:
                var = rhs_m.group(1)
                _, _, assign_line, usage_count = temp_map[var]
                if usage_count <= 1:
                    temp_lines_to_remove.add(assign_line)
                    temps_to_clean.add(var)

    typedefs = []
    lines_to_remove = set()

    # Process groups in reverse order so line indices remain valid
    for i, group in enumerate(reversed(groups)):
        struct_name = f'_struct_copy_{len(groups) - i}'
        typedefs.append(generate_typedef(struct_name, group.n_words))
        replacement = make_replacement(group, struct_name)

        # Replace the group lines with a single line
        first_line = group.copies[0].line_idx
        last_line = group.copies[-1].line_idx
        lines[first_line:last_line + 1] = [replacement]

        # Adjust temp removal indices for the removed lines
        # (handled by collecting all removals first, then applying)

    # Remove temp var assignments (process in reverse order)
    for idx in sorted(temp_lines_to_remove, reverse=True):
        # Adjust index for any prior group replacements
        # Since groups are processed in reverse, we need to recalculate
        # For simplicity, blank out the line instead of removing it
        # to avoid index shifts
        if idx < len(lines) and 'temp_' in lines[idx]:
            lines[idx] = ''

    # Also remove temp var declarations that are now unused
    for var in temps_to_clean:
        decl_re = re.compile(r'^\s+s32 ' + re.escape(var) + r'\s*;')
        for k, line in enumerate(lines):
            if decl_re.match(line):
                lines[k] = ''
                break

    # Clean up blank lines (collapse multiple blanks)
    cleaned = []
    prev_blank = False
    for line in lines:
        if line.strip() == '':
            if not prev_blank:
                cleaned.append(line)
            prev_blank = True
        else:
            cleaned.append(line)
            prev_blank = False
    lines = cleaned

    # Insert typedefs before the function definition
    func_re = re.compile(r'^(?:void|s32|u32|s16|u16|s8|u8|M2C_UNK)\s+\w+\s*\(')
    insert_idx = 0
    for i, line in enumerate(lines):
        if func_re.match(line):
            insert_idx = i
            break

    for td in reversed(typedefs):
        lines.insert(insert_idx, td)

    return '\n'.join(lines)


def main():
    parser = argparse.ArgumentParser(
        description='Detect and replace consecutive word copies with struct assignments')
    parser.add_argument('input', help='Input C file')
    parser.add_argument('-o', '--output', help='Output file (default: stdout)')
    parser.add_argument('--asm', help='Target asm file to verify against')
    parser.add_argument('--dry-run', action='store_true',
                        help='Report findings without modifying')
    parser.add_argument('--min-words', type=int, default=3,
                        help='Minimum consecutive words to detect (default: 3)')
    args = parser.parse_args()

    with open(args.input) as f:
        source = f.read()

    lines = source.split('\n')
    groups = find_copy_groups(lines, args.min_words)

    if args.asm:
        groups = verify_against_asm(args.asm, groups)

    if args.dry_run:
        if not groups:
            print('No struct copy patterns found.')
        for g in groups:
            print(f'  {g.n_words}-word copy: {g.src_base}+0x{g.src_start:X} -> {g.dst_base}+0x{g.dst_start:X}'
                  f'  (lines {g.copies[0].line_idx + 1}-{g.copies[-1].line_idx + 1})')
        return

    result = apply_fixes(source, groups)

    if args.output:
        with open(args.output, 'w', newline='\n') as f:
            f.write(result)
        n = len(groups)
        print(f'Fixed {n} struct cop{"y" if n == 1 else "ies"} -> {args.output}', file=sys.stderr)
    else:
        print(result)


if __name__ == '__main__':
    main()

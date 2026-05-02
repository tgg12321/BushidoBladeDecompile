#!/usr/bin/env python3
"""find_quickwins.py — surface remaining INCLUDE_ASM stubs that are likely
quick wins, defined as: small (<= MAX_INSNS), few callee-saved registers
(<= MAX_SREGS), and no grep-detectable blockers.

Filters out:
  - lwl/lwr/swl/swr (need fix_lwl per-file opt-in)
  - break (debug/assert traps, often intractable)
  - mtc2/mfc2/cop2 (GTE, needs __asm__ macros)
  - syscall, jalr $tN (handwritten asm markers)
  - add/addi/sub (non-u variants — handwritten asm markers)
  - tabled functions listed in CLAIMS.md (avoids re-attempting known-hard)

Output: ranked list with per-stub stats. Optional --json for machine consumption.

Usage:
    python3 tools/find_quickwins.py
    python3 tools/find_quickwins.py --max-insns 250 --max-sregs 7
    python3 tools/find_quickwins.py --include-tabled
    python3 tools/find_quickwins.py --json > tmp/quickwins.json
"""
import argparse
import glob
import json
import os
import re
import sys

DEFAULT_MAX_INSNS = 200
DEFAULT_MAX_SREGS = 5

INCLUDE_ASM_RE = re.compile(r'INCLUDE_ASM\("asm/funcs",\s*(\w+)\)')
INSN_RE = re.compile(r'/\*\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+\*/\s+(\S+)\s*(.*)')
# Callee-saved registers GCC may save: $s0-$s7, $fp/$s8, $ra
SAVE_RE = re.compile(r'^sw\s+(\$s[0-7]|\$fp|\$s8|\$ra)\s*,')
FRAME_RE = re.compile(r'^addiu\s+\$sp\s*,\s*\$sp\s*,\s*(-?\d+)')


def scan_asm(path):
    """Return dict of stats for a single asm/funcs/<func>.s file."""
    stats = {
        'insns': 0,
        'sregs': set(),
        'frame_size': 0,
        'blockers': set(),
        'has_jal': 0,
        'jr_ra_count': 0,
    }
    if not os.path.exists(path):
        stats['blockers'].add('missing-asm')
        return stats

    with open(path) as fh:
        for raw in fh:
            line = raw.strip()
            m = INSN_RE.search(raw)
            if not m:
                continue
            mnem, ops = m.group(1), m.group(2)
            stats['insns'] += 1

            # Frame size (negative addiu sp,sp,N in prologue)
            fm = FRAME_RE.match(f'{mnem} {ops}')
            if fm and stats['frame_size'] == 0:
                v = int(fm.group(1))
                if v < 0:
                    stats['frame_size'] = -v

            # Callee-saved register saves
            sm = SAVE_RE.match(f'{mnem} {ops}')
            if sm:
                reg = sm.group(1)
                if reg != '$ra':  # don't count $ra in s-reg pressure
                    stats['sregs'].add(reg)

            # Blockers
            if mnem in ('lwl', 'lwr', 'swl', 'swr'):
                stats['blockers'].add('lwl')
            if mnem == 'break':
                stats['blockers'].add('break')
            # Jump tables — `jtbl_` symbol reference or %hi(jtbl_...) load
            if 'jtbl_' in ops or '%hi(jtbl_' in ops or '%lo(jtbl_' in ops:
                stats['blockers'].add('jtbl')
            if mnem in ('mtc2', 'mfc2', 'ctc2', 'cfc2', 'lwc2', 'swc2'):
                stats['blockers'].add('gte')
            if mnem.startswith('cop2') or mnem.startswith('c2'):
                if mnem not in ('ctc2', 'cfc2'):
                    stats['blockers'].add('gte')
            if mnem == 'syscall':
                stats['blockers'].add('syscall')
            # Handwritten markers: add (not addu/addi/addiu), sub (not subu)
            if mnem == 'add':
                stats['blockers'].add('handwritten-add')
            if mnem == 'addi':
                stats['blockers'].add('handwritten-addi')
            if mnem == 'sub':
                stats['blockers'].add('handwritten-sub')
            # jalr to temp register = handwritten BIOS pattern
            if mnem == 'jalr' and re.search(r'\$t\d', ops):
                stats['blockers'].add('handwritten-jalr-t')
            if mnem == 'jal':
                stats['has_jal'] += 1
            # Multi-function stub detection: count `jr $ra` returns
            if mnem == 'jr' and ops.strip().startswith('$ra'):
                stats['jr_ra_count'] += 1

    if stats['jr_ra_count'] > 1:
        stats['blockers'].add(f'multi-func({stats["jr_ra_count"]})')

    stats['sregs'] = sorted(stats['sregs'])
    stats['blockers'] = sorted(stats['blockers'])
    return stats


def parse_tabled_funcs(claims_path='CLAIMS.md'):
    """Extract function names that appear in CLAIMS.md rows marked as tabled."""
    tabled = set()
    if not os.path.exists(claims_path):
        return tabled
    with open(claims_path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            if 'tabled' in line.lower() or 'TABLED' in line:
                # Look for func names like func_XXXXXXXX or :NAME pattern
                for m in re.finditer(r'\b(func_[0-9A-Fa-f]+|[a-zA-Z_]\w{3,})\b', line):
                    name = m.group(1)
                    # Skip noise words; only collect things that look like funcs
                    if name.startswith('func_') or '_' in name:
                        tabled.add(name)
    return tabled


def parse_known_bad(path='tools/known_bad_stubs.txt'):
    """Parse known-bad list. Returns dict {funcname: reason}."""
    bad = {}
    if not os.path.exists(path):
        return bad
    with open(path) as fh:
        for line in fh:
            line = line.split('#', 1)
            name = line[0].strip()
            reason = line[1].strip() if len(line) > 1 else ''
            if name and not name.startswith('#'):
                bad[name] = reason
    return bad


def find_stubs(src_dir='src'):
    """Yield (func_name, src_file_path) for every INCLUDE_ASM stub."""
    for cfile in sorted(glob.glob(f'{src_dir}/*.c')):
        with open(cfile) as fh:
            content = fh.read()
        for m in INCLUDE_ASM_RE.finditer(content):
            yield m.group(1), cfile


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--max-insns', type=int, default=DEFAULT_MAX_INSNS,
                    help=f'Max instruction count (default {DEFAULT_MAX_INSNS})')
    ap.add_argument('--max-sregs', type=int, default=DEFAULT_MAX_SREGS,
                    help=f'Max callee-saved registers (default {DEFAULT_MAX_SREGS})')
    ap.add_argument('--include-tabled', action='store_true',
                    help='Include functions listed as tabled in CLAIMS.md')
    ap.add_argument('--json', action='store_true', help='Emit JSON')
    ap.add_argument('--all', action='store_true',
                    help='Skip blocker/size filters; show all stubs ranked')
    ap.add_argument('--asm-dir', default='asm/funcs')
    ap.add_argument('--src-dir', default='src')
    args = ap.parse_args()

    tabled = set() if args.include_tabled else parse_tabled_funcs()
    known_bad = parse_known_bad()
    rows = []
    for func, cfile in find_stubs(args.src_dir):
        asm_path = os.path.join(args.asm_dir, f'{func}.s')
        st = scan_asm(asm_path)
        rows.append({
            'func': func,
            'src': os.path.basename(cfile),
            'insns': st['insns'],
            'sregs': len(st['sregs']),
            'sreg_list': st['sregs'],
            'frame': st['frame_size'],
            'jal': st['has_jal'],
            'blockers': st['blockers'],
            'tabled': func in tabled,
            'known_bad': known_bad.get(func, ''),
        })

    if not args.all:
        rows = [r for r in rows
                if r['insns'] <= args.max_insns
                and r['sregs'] <= args.max_sregs
                and not r['blockers']
                and not r['tabled']
                and not r['known_bad']]

    rows.sort(key=lambda r: (r['insns'], r['sregs']))

    if args.json:
        print(json.dumps(rows, indent=2))
        return

    if args.all:
        print(f'All {len(rows)} stubs (sorted by insn count):')
    else:
        print(f'Quick-win candidates (insns<={args.max_insns}, sregs<={args.max_sregs}, '
              f'no blockers, not tabled): {len(rows)}')
    print(f'{"#":>3} {"func":<32} {"src":<22} {"insns":>5} {"sregs":>5} {"frame":>5} {"jal":>3} blockers')
    for i, r in enumerate(rows, 1):
        bk = ','.join(r['blockers']) if r['blockers'] else ''
        if r['tabled']:
            bk = f'TABLED {bk}'.strip()
        if r['known_bad']:
            bk = f'KNOWN-BAD: {r["known_bad"]} {bk}'.strip()
        print(f'{i:>3} {r["func"]:<32} {r["src"]:<22} {r["insns"]:>5} {r["sregs"]:>5} '
              f'{r["frame"]:>5} {r["jal"]:>3} {bk}')


if __name__ == '__main__':
    main()

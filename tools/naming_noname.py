#!/usr/bin/env python3
"""The NO_NAME ledger, docs/naming/phase3/no_name.tsv: every unnamed function a
naming wave researched and left neutral, so later waves skip it unless new
evidence exists.

  free <tu-prefix>[,...] [--revive ADDR=EVIDENCE ...]
      census func_ rows in those TUs (src_location prefix, e.g. src/main/9F9C.c)
      that are not in the ledger, plus each revived ledger row with the new
      evidence that justifies looking again (a neighbour named since, a type
      landed, a Q124 trace capture). Prints names one per line, then the
      revive notes for the research prompt.
  add <research.md|func_manifest.csv> --tag wNN [--commit SHA]
      merge that wave's NO_NAME rows (rounds += 1), then prune.
  prune
      drop rows whose function has been named since.

Run at the repo root.
"""
import argparse
import csv
import io
import re
import sys

LEDGER = 'docs/naming/phase3/no_name.tsv'
CENSUS = 'docs/naming/function-names.csv'
COLS = ['address', 'name', 'kind', 'rounds', 'waves', 'reason', 'evidence_tried', 'commit']
SECTIONS = ('HOMES', 'USES', 'BODY', 'CALLERS', 'DATA', 'COUNTER-EVIDENCE', 'PRIOR', 'TRACE')
csv.field_size_limit(10**8)


def norm(addr):
    return '0x' + addr[2:].upper()


def load_census():
    with io.open(CENSUS, encoding='utf-8') as f:
        return {norm(r['address']): r for r in csv.DictReader(f)}


def load():
    with io.open(LEDGER, encoding='utf-8') as f:
        return {norm(r['address']): r for r in csv.DictReader(f, delimiter='\t')}


def save(rows):
    with io.open(LEDGER, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\t'.join(COLS) + '\n')
        for a in sorted(rows, key=lambda a: int(a, 16)):
            f.write('\t'.join(str(rows[a][c]).replace('\t', ' ') for c in COLS) + '\n')


def clip(s, n=260):
    s = re.sub(r'\s+', ' ', s).strip()
    return s if len(s) <= n else s[:n - 3].rstrip() + '...'


def reason_of(ev):
    m = re.search(r'WHY[^:]*:\s*(.*?)(?:\s*COUNTER-EVIDENCE\s*:\s*(.*?))?'
                  r'(?:\s*PRIOR\s*:|\s*FRESH\s*:|$)', ev, re.S)
    if not m:
        return clip(ev)
    return clip(m.group(1) + (' | ' + m.group(2) if m.group(2) else ''))


def sections_of(ev):
    return '+'.join(s.lower() for s in SECTIONS
                    if re.search(r'(?<![A-Z-])' + s + r'[ :/]', ev))


def no_name_rows(path):
    """(address, evidence_class, evidence) for each func NO_NAME row; accepts
    Codex research markdown (CSV lines) or a func_manifest.csv."""
    with io.open(path, encoding='utf-8') as f:
        text = f.read()
    if path.endswith('.csv'):
        for r in csv.DictReader(io.StringIO(text)):
            if r.get('action') == 'NO_NAME':
                yield r['addr'], r.get('evidence_class', ''), r.get('evidence', '')
        return
    for line in text.splitlines():
        if not re.match(r'0x8[0-9A-Fa-f]{7},func_', line):
            continue
        r = next(csv.reader([line]))
        if len(r) >= 7 and r[4] == 'NO_NAME':
            yield r[0], r[5], r[6]


def prune(rows, census):
    gone = [a for a in rows if not census.get(a, {}).get('current_name', '').startswith('func_')]
    for a in gone:
        del rows[a]
    return gone


def cmd_free(args):
    census, rows = load_census(), load()
    tus = tuple(args.tus.split(','))
    revive = {}
    for spec in args.revive:
        addr, _, why = spec.partition('=')
        if not why.strip():
            sys.exit(f'--revive {addr}: name the new evidence (ADDR=EVIDENCE)')
        revive[norm(addr)] = why.strip()
    free, notes = [], []
    for a, c in sorted(census.items(), key=lambda kv: int(kv[0], 16)):
        if not (c['current_name'].startswith('func_')
                and (c.get('src_location') or '').startswith(tus)):
            continue
        if a not in rows:
            free.append(c['current_name'])
        elif a in revive:
            free.append(c['current_name'])
            r = rows[a]
            notes.append(f"{c['current_name']}: NO_NAME in {r['waves']} ({r['reason']}); "
                         f"new evidence: {revive.pop(a)}")
    if revive:
        sys.exit('not in the ledger for these TUs: ' + ' '.join(revive))
    skipped = sum(1 for a, c in census.items() if a in rows
                  and (c.get('src_location') or '').startswith(tus)) - len(notes)
    print('\n'.join(free))
    for n in notes:
        print('# revive ' + n)
    print(f'# {len(free)} free, {skipped} skipped as NO_NAME (no new evidence)', file=sys.stderr)


def cmd_add(args):
    census, rows = load_census(), load()
    n = 0
    for addr, evclass, ev in no_name_rows(args.source):
        a = norm(addr)
        c = census.get(a)
        if not c or not c['current_name'].startswith('func_'):
            continue
        r = rows.get(a)
        waves = r['waves'].split(',') if r else []
        if args.tag in waves:
            continue
        rows[a] = {
            'address': a, 'name': c['current_name'], 'kind': 'func',
            'rounds': str(len(waves) + 1), 'waves': ','.join(waves + [args.tag]),
            'reason': reason_of(ev),
            'evidence_tried': 'codex research' + (f' {evclass}' if evclass else '')
                              + (f': {sections_of(ev)}' if sections_of(ev) else ''),
            'commit': args.commit or '-',
        }
        n += 1
    gone = prune(rows, census)
    save(rows)
    print(f'added {n}, pruned {len(gone)} named since, ledger {len(rows)}')


def cmd_prune(args):
    rows = load()
    gone = prune(rows, load_census())
    save(rows)
    print(f'pruned {len(gone)}: {" ".join(gone)}' if gone else 'nothing to prune')


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest='cmd', required=True)
    f = sub.add_parser('free')
    f.add_argument('tus')
    f.add_argument('--revive', action='append', default=[])
    f.set_defaults(fn=cmd_free)
    a = sub.add_parser('add')
    a.add_argument('source')
    a.add_argument('--tag', required=True)
    a.add_argument('--commit')
    a.set_defaults(fn=cmd_add)
    sub.add_parser('prune').set_defaults(fn=cmd_prune)
    args = p.parse_args()
    args.fn(args)


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""The NO_NAME ledger, docs/naming/phase3/no_name.tsv: every placeholder a
naming wave researched and left neutral, so later waves skip it unless new
evidence exists.

A row is a function (kind func, keyed by address) or, for the final sets only,
a global (data, address), a struct member (member, Type.member) or a local or
parameter (local, function:binding). Its `final` column is `-` when the row is
neutral for lack of evidence (OPEN: new evidence may revive it), else the class
that makes it structurally neutral (FINAL: no name can meet the naming bar
whatever evidence comes, and a fresh naming-reviewer passed that claim;
naming-bar N7, Phase 5 and owner ruling Q136; the class tests are
.claude/rules/naming-final-classes.md). FINAL_CLASSES holds the tokens:
  func    empty (jr ra body), dead (no caller or address use; the body returns
          nothing or one literal, writes and calls only final placeholders), forwarder (one call of a
          named callee, arguments passed through unchanged, its result
          returned or dropped)
  data    gap-filler (no reference; no describable contents), write-only (no read anywhere, uncalled
          code, block copies, I/O and disc-asset layouts included; every
          store writes the initial value, so it never changes), two-role
  member  never-accessed (no access in C or in any INCLUDE_ASM body, through
          any alias; no disc-asset layout),
          write-only (as for data), two-role
  local   const-holder, fake-copy (FAKE-labelled non-parameter locals), unused-param (a
          parameter never read, or only passed unchanged to an unused-param,
          whose callers, traced through every unchanged pass-through and
          forwarder and through function-pointer dispatch, do not all supply
          one quantity, named or still placeholder), write-only (a
          non-parameter local never read, address never taken), two-role
  two-role: a FAKE-labelled local whose label says it holds two values, or
  values of different kinds (type, unit or kind of object, not
  another instance), one use cited per role; a refined quantity (an index
  then its element's address, a cast), an advancing cursor, a counter or
  values one use reads alike are one role.
Classes cover game code and types only (not src/main/psxsdk or
include/psxsdk: Sony's names may come, N3.1). A name that meets the bar
still wins over a class. A row becomes final
only through such a review (backlog rows naming-close-final*); every
placeholder not final is OPEN. load and save refuse a token not listed for the
row's kind.

  free <tu-prefix>[,...] [--revive ADDR=EVIDENCE ...] [--force-final]
      census func_ rows in those TUs (src_location prefix, e.g. src/main/9F9C.c)
      that are not in the ledger, plus each revived ledger row with the new
      evidence that justifies looking again (a neighbour named since, a type
      landed, a Q124 trace capture). Prints names one per line, then the
      revive notes for the research prompt. A final row is never offered;
      --revive refuses one unless --force-final (an owner ruling) is given.
  add <research.md|func_manifest.csv> --tag wNN [--commit SHA]
      merge that wave's NO_NAME rows (rounds += 1), then prune.
  prune
      drop rows whose placeholder has been named (or removed) since.
  status
      final vs open placeholders per kind, counted from the tree: unnamed
      census functions; D_8 tokens in game C and headers; unk / field / fXX
      members of game types; placeholder locals and parameters (var_N, temp,
      new_var, spXX, argN, register-style sN / tN / vN / aN) of game
      functions. Game code is src/main without psxsdk, plus include/*.h,
      as git lists them (an ignored file such as the generated
      include/m2c_context.h does not count); a typedef name used only as a
      type (s8) is no local.
  homes [<tu id>,...]
      every open (non-final) placeholder status counts, one `kind key home`
      line, home the game TU owning it (homes() states the rules; the backlog's
      TU sweep rows own placeholders by it); only those TUs if given
      (main/17AFC,main/87A0).

add and prune also fill each `-` commit from git: the `naming: waveNN` commit of
the row's last wave, else a commit whose body cites `wNN-research.md`. Lane
commits are replayed when they land, so `-` is right until the wave is on main.

Run at the repo root.
"""
import argparse
import csv
import glob
import io
import os
import re
import subprocess
import sys

LEDGER = 'docs/naming/phase3/no_name.tsv'
CENSUS = 'docs/naming/function-names.csv'
COLS = ['address', 'name', 'kind', 'rounds', 'waves', 'reason', 'evidence_tried', 'commit',
        'final']
KINDS = ('func', 'data', 'member', 'local')
FINAL_CLASSES = {
    'func': ('empty', 'dead', 'forwarder'),
    'data': ('gap-filler', 'write-only', 'two-role'),
    'member': ('never-accessed', 'write-only', 'two-role'),
    'local': ('const-holder', 'fake-copy', 'two-role', 'unused-param', 'write-only'),
}
SECTIONS = ('HOMES', 'USES', 'BODY', 'CALLERS', 'DATA', 'COUNTER-EVIDENCE', 'PRIOR', 'TRACE')
D_SYM = re.compile(r'\bD_8[0-9A-F]{7}\b')
NEUTRAL_MEMBER = re.compile(r'^(unk_?[0-9A-Fa-f]+(_\w+)?|f[0-9A-Fa-f]{1,4}|field_?[0-9A-Fa-f]+)$')
PLACEHOLDER = re.compile(r'^(var_?[0-9]+|var_[a-z]+[0-9]+|temp[0-9]*|new_var[0-9]*|sp[0-9A-F]+'
                         r'|arg[0-9]+|[stva][0-9])$')
csv.field_size_limit(10**8)


def norm(addr):
    a = addr.strip()
    if a[:2].lower() == '0x':
        a = a[2:]
    return '0x' + a.upper()


def key_of(r):
    return norm(r['address']) if r.get('kind', 'func') in ('func', 'data') else r['address']


_found = {}


def wave_commit(tag):
    m = re.match(r'w(\d+)', tag)
    if not m:
        return ''
    if tag not in _found:
        def log(*grep):
            out = subprocess.run(['git', 'log', '--format=%h', *grep, 'HEAD'],
                                 capture_output=True, text=True).stdout.split()
            return out[-1] if out else ''
        _found[tag] = (log('-E', f'--grep=^naming: wave{m.group(1)}([^0-9]|$)')
                       or log('-F', f'--grep={tag}-research.md'))
    return _found[tag]


def resolve(rows):
    for r in rows.values():
        if r['commit'] == '-':
            r['commit'] = wave_commit(r['waves'].split(',')[-1]) or '-'


def load_census():
    with io.open(CENSUS, encoding='utf-8') as f:
        return {norm(r['address']): r for r in csv.DictReader(f)}


def check_final(r):
    """The row's final token is `-` or a class of its kind (FINAL_CLASSES)."""
    tok, kind = r.get('final') or '-', r.get('kind', 'func')
    if tok != '-' and tok not in FINAL_CLASSES.get(kind, ()):
        raise ValueError(f"{r['address']}: final class {tok!r} is not one of {kind}'s "
                         f"({', '.join(FINAL_CLASSES.get(kind, ())) or 'none'})")


def load():
    with io.open(LEDGER, encoding='utf-8') as f:
        rows = list(csv.DictReader(f, delimiter='\t'))
    for r in rows:
        r['final'] = r.get('final') or '-'
        check_final(r)
    return {key_of(r): r for r in rows}


def is_final(r):
    return r.get('final', '-') not in ('', '-')


def order(item):
    k, r = item
    kind = KINDS.index(r['kind']) if r['kind'] in KINDS else len(KINDS)
    return (kind, int(k, 16) if r['kind'] in ('func', 'data') else 0, k)


def save(rows):
    for r in rows.values():
        check_final(r)
    with io.open(LEDGER, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\t'.join(COLS) + '\n')
        for _, r in sorted(rows.items(), key=order):
            f.write('\t'.join(str(r[c]).replace('\t', ' ') for c in COLS) + '\n')


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


def src_name(c):
    """The function's name in src/: a census alias (name_layer '... alias only')
    is a lead, not a name, so such a row is still its func_ glabel."""
    if 'alias only' in (c.get('name_layer') or '') and c.get('glabel', '').startswith('func_'):
        return c['glabel']
    return c.get('current_name', '')


def unnamed(c):
    return src_name(c).startswith('func_')


# --- tree census of the non-function placeholders -----------------------------

def game_files():
    """Game C files and top-level headers that git tracks or would track: an
    ignored generated file (include/m2c_context.h, tools/gen_m2c_context.py
    output, never built) repeats the tree's declarations and must not count.
    Outside a git checkout every file on disk counts."""
    try:
        out = subprocess.run(['git', 'ls-files', '--cached', '--others', '--exclude-standard',
                              '--', 'src/main', 'include'],
                             capture_output=True, text=True, check=True).stdout.split('\n')
        files = {f for f in out if f and os.path.exists(f)}
    except (OSError, subprocess.CalledProcessError):
        files = {f.replace('\\', '/') for f in
                 glob.glob('src/main/**/*.c', recursive=True) + glob.glob('include/*.h')}
    h = sorted(f for f in files if re.fullmatch(r'include/[^/]+\.h', f))
    c = sorted(f for f in files if re.fullmatch(r'src/main/.+\.c', f) and '/psxsdk/' not in f)
    return h + c


def blank(text):
    """Comments, string / char literals and preprocessor lines as spaces
    (newlines kept)."""
    def sub(m):
        return re.sub(r'[^\n]', ' ', m.group(0))
    text = re.sub(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'',
                  sub, text, flags=re.S)
    return re.sub(r'^[ \t]*#(?:[^\n]*\\\n)*[^\n]*', sub, text, flags=re.M)


def closing(text, i):
    """Index just past the brace that closes the one before i."""
    depth = 1
    while depth and i < len(text):
        depth += {'{': 1, '}': -1}.get(text[i], 0)
        i += 1
    return i


def flat(body):
    """The body with nested brace blocks dropped (their members are their own type's)."""
    out, depth = [], 0
    for ch in body:
        if ch == '{':
            depth += 1
        elif ch == '}':
            depth -= 1
        elif depth == 0:
            out.append(ch)
    return ''.join(out)


def as_type(region, t):
    """The token is a type here: it starts a declaration (`s8 x`, `s8 *p`) or
    is a cast (`(s8)`), not a value (`s8 = 0`, `f(s8)`). A real binding with a
    typedef's name still counts through its declaration (`s32 s8;`)."""
    after = region[t.end():].lstrip()
    if re.match(r'[A-Za-z_*]', after):
        return True
    before = region[:t.start()].rstrip()
    return after[:1] == ')' and before.endswith('(') and not re.search(r'\w\s*\($', before)


def tree_census(files=None):
    """{'data': {D_ name}, 'member': {Type.member}, 'local': {func:binding}}.
    A placeholder-shaped typedef name (s8) is a local only where used as a value."""
    data, members, locals_ = set(), set(), set()
    files = files or game_files()
    texts = {}
    for f in files:
        with io.open(f, encoding='utf-8') as fh:
            texts[f] = blank(fh.read())
    types = {m.group(1) for text in texts.values()
             for m in re.finditer(r'\btypedef\b[^;{}]*?\b(\w+)\s*;', text)}
    for f in files:
        text = texts[f]
        data |= set(D_SYM.findall(text))
        for m in re.finditer(r'\b(typedef\s+)?(struct|union)\s*(\w*)\s*\{', text):
            end = closing(text, m.end())
            tail = re.match(r'\s*(\w+)', text[end:])
            name = (tail.group(1) if m.group(1) and tail else m.group(3))
            if not name:
                continue
            for decl in flat(text[m.end():end - 1]).split(';'):
                d = re.search(r'([A-Za-z_]\w*)\s*(\[[^\]]*\])*\s*(:\s*\w+)?\s*$', decl)
                if d and NEUTRAL_MEMBER.match(d.group(1)) and re.search(r'\w\s', decl.strip()):
                    members.add(f'{name}.{d.group(1)}')
        if not f.endswith('.c'):
            continue
        depth, start = 0, 0
        i = 0
        while i < len(text):
            ch = text[i]
            if ch == '{' and depth == 0:
                sig = text[start:i]
                fn = re.search(r'(\w+)\s*\([^;{}]*\)\s*$', sig)
                end = closing(text, i + 1)
                if fn and not re.search(r'=\s*$|\b(struct|union|enum)\s*\w*\s*$', sig):
                    region = sig[fn.start():] + text[i:end]
                    for t in re.finditer(r'(?<![\w.>])(\w+)\b', region):
                        if (PLACEHOLDER.match(t.group(1)) and region[:t.start()][-2:] != '->'
                                and not (t.group(1) in types and as_type(region, t))):
                            locals_.add(f'{fn.group(1)}:{t.group(1)}')
                i, start = end, end
                continue
            if ch in ';}':
                start = i + 1
            i += 1
    return {'data': data, 'member': members, 'local': locals_}


def prune(rows, census, tree=None):
    gone = []
    for a, r in list(rows.items()):
        kind = r.get('kind', 'func')
        if kind == 'func':
            dead = not unnamed(census.get(a, {}))
        else:
            tree = tree if tree is not None else tree_census()
            dead = (r['name'] if kind == 'data' else r['address']) not in tree.get(kind, ())
        if dead:
            del rows[a]
            gone.append(a)
    return gone


def cmd_free(args):
    census, rows = load_census(), load()
    tus = tuple(t for t in args.tus.split(',') if t)
    if not tus:
        sys.exit('free: give at least one TU prefix')
    revive = {}
    for spec in args.revive:
        addr, _, why = spec.partition('=')
        if not why.strip():
            sys.exit(f'--revive {addr}: name the new evidence (ADDR=EVIDENCE)')
        a = norm(addr)
        if a in rows and is_final(rows[a]) and not args.force_final:
            sys.exit(f'--revive {addr}: final ({rows[a]["final"]}): structurally neutral, '
                     'no evidence revives it (--force-final only with an owner ruling)')
        revive[a] = why.strip()
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
    held = [a for a, c in census.items() if a in rows
            and c['current_name'].startswith('func_')
            and (c.get('src_location') or '').startswith(tus)]
    final = sum(1 for a in held if is_final(rows[a]))
    print('\n'.join(free))
    for n in notes:
        print('# revive ' + n)
    print(f'# {len(free)} free, {len(held) - len(notes) - final} skipped as NO_NAME '
          f'(no new evidence), {final} final', file=sys.stderr)


def cmd_add(args):
    census, rows = load_census(), load()
    n = 0
    for addr, evclass, ev in no_name_rows(args.source):
        a = norm(addr)
        c = census.get(a)
        if not c or not unnamed(c):
            continue
        r = rows.get(a)
        waves = r['waves'].split(',') if r else []
        if args.tag in waves:
            if args.commit:
                r['commit'] = args.commit
            continue
        rows[a] = {
            'address': a, 'name': src_name(c), 'kind': 'func',
            'rounds': str(len(waves) + 1), 'waves': ','.join(waves + [args.tag]),
            'reason': reason_of(ev),
            'evidence_tried': 'codex research' + (f' {evclass}' if evclass else '')
                              + (f': {sections_of(ev)}' if sections_of(ev) else ''),
            'commit': args.commit or '-',
            'final': r['final'] if r else '-',
        }
        n += 1
    gone = prune(rows, census)
    resolve(rows)
    save(rows)
    print(f'added {n}, pruned {len(gone)} named since, ledger {len(rows)}')


def cmd_prune(args):
    rows = load()
    gone = prune(rows, load_census())
    resolve(rows)
    save(rows)
    print(f'pruned {len(gone)}: {" ".join(gone)}' if gone else 'nothing to prune')


def homes(census, rows):
    """[(kind, key, home TU id)] for every open (non-final) game placeholder; the home rules
    of a TU sweep (naming-session skill, Sweeps). func: its census src_location; local: the TU
    defining its function; data: the code TU (not a data-only d_*.c) with the most uses, C
    tokens outside `extern` lines plus the asm/funcs bodies it INCLUDE_ASMs (ties: lowest TU
    id), else the TU defining it in C, else `-`; member: one home per type, the .c defining
    it, else the code TU naming the type most, else the home of a type embedding it, else the
    code TU with most `.m` / `->m` accesses of the type's placeholder members (ties: lowest)."""
    files = game_files()
    heads = [f for f in files if not f.endswith('.c')]
    cfiles = [f for f in files if f.endswith('.c')]
    code = [f for f in cfiles if not os.path.basename(f).startswith('d_')]
    tid = lambda f: f[4:-2] if f.startswith('src/') and f.endswith('.c') else f  # noqa: E731
    raw = {}
    for f in files:
        with io.open(f, encoding='utf-8') as fh:
            raw[f] = fh.read()
    texts = {f: blank(t) for f, t in raw.items()}
    final = {(r['kind'], r['name'] if r['kind'] == 'data' else key_of(r))
             for r in rows.values() if is_final(r)}
    tree = tree_census(files)
    out = [('func', a, tid((c.get('src_location') or '-').split(':')[0]))
           for a, c in sorted(census.items()) if unnamed(c) and ('func', a) not in final]
    for f in cfiles:
        for k in sorted(tree_census(heads + [f])['local'] & tree['local']):
            if ('local', k) not in final:
                out.append(('local', k, tid(f)))

    def best(cnt):
        cnt = {f: n for f, n in cnt.items() if n}
        return tid(sorted(cnt, key=lambda f: (-cnt[f], f))[0]) if cnt else None
    asm = {}
    for f in code:
        bodies = []
        for n in re.findall(r'INCLUDE_ASM\(\s*"asm/funcs"\s*,\s*(\w+)\s*\)', raw[f]):
            if os.path.exists(f'asm/funcs/{n}.s'):
                with io.open(f'asm/funcs/{n}.s', encoding='utf-8', errors='replace') as fh:
                    bodies.append(fh.read())
        asm[f] = '\n'.join(bodies)
    for d in sorted(tree['data']):
        if ('data', d) in final:
            continue
        w = re.compile(rf'\b{d}\b')
        uses = {f: len(w.findall(asm[f])) + sum(len(w.findall(ln))
                                                for ln in texts[f].splitlines()
                                                if not re.match(r'\s*extern\b', ln))
                for f in code}
        dfn = [f for f in cfiles if any(w.search(ln) for ln in texts[f].splitlines()
                                        if not re.match(r'\s*extern\b', ln))]
        out.append(('data', d, best(uses) or (tid(dfn[0]) if dfn else '-')))
    defined, members, embeds = {}, {}, {}
    for f in files:
        for m in re.finditer(r'\b(typedef\s+)?(struct|union)\s*(\w*)\s*\{', texts[f]):
            end = closing(texts[f], m.end())
            tail = re.match(r'\s*(\w+)', texts[f][end:])
            name = tail.group(1) if m.group(1) and tail else m.group(3)
            if name:
                defined.setdefault(name, f)
                for t in re.findall(r'\b([A-Za-z_]\w*)\s+\w+\s*(?:\[[^\]]*\]\s*)*;',
                                    flat(texts[f][m.end():end - 1])):
                    embeds.setdefault(t, name)
    for k in sorted(tree['member']):
        if ('member', k) not in final:
            members.setdefault(k.split('.')[0], []).append(k)

    def type_home(t, seen=()):
        if defined.get(t, '').endswith('.c'):
            return tid(defined[t])
        h = best({f: len(re.findall(rf'\b{t}\b', texts[f])) for f in code})
        if h:
            return h
        if t in embeds and embeds[t] not in seen:
            h = type_home(embeds[t], seen + (t,))
            if h != '-':
                return h
        ms = [k.split('.')[1] for k in members.get(t, [])]
        return best({f: sum(len(re.findall(rf'(?:->|\.)\s*{m}\b', texts[f])) for m in ms)
                     for f in code}) or '-'
    for t, ks in sorted(members.items()):
        h = type_home(t)
        out += [('member', k, h) for k in ks]
    return out


def cmd_homes(args):
    want = {t for t in (args.tus or '').split(',') if t}
    rows = homes(load_census(), load())
    for kind, key, home in rows:
        if not want or home in want:
            print(f'{kind}\t{key}\t{home}')
    shown = [r for r in rows if not want or r[2] in want]
    print('# ' + ', '.join(f'{k} {sum(1 for r in shown if r[0] == k)}' for k in KINDS),
          file=sys.stderr)


def cmd_status(args):
    census, rows = load_census(), load()
    tree = tree_census()
    tree['func'] = {a for a, c in census.items() if unnamed(c)}
    print('kind\tplaceholders\tfinal\topen\topen_in_ledger')
    for kind in KINDS:
        have = tree[kind]
        ledger = {(a if kind != 'data' else r['name']): r for a, r in rows.items()
                  if r['kind'] == kind}
        final = sum(1 for k, r in ledger.items() if is_final(r) and k in have)
        held = sum(1 for k, r in ledger.items() if not is_final(r) and k in have)
        print(f'{kind}\t{len(have)}\t{final}\t{len(have) - final}\t{held}')
    stale = [a for a, r in rows.items()
             if (r['name'] if r['kind'] == 'data' else a) not in tree.get(r['kind'], ())]
    if stale:
        print('# not in the tree (run prune): ' + ' '.join(stale), file=sys.stderr)


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest='cmd', required=True)
    f = sub.add_parser('free')
    f.add_argument('tus')
    f.add_argument('--revive', action='append', default=[])
    f.add_argument('--force-final', action='store_true')
    f.set_defaults(fn=cmd_free)
    a = sub.add_parser('add')
    a.add_argument('source')
    a.add_argument('--tag', required=True)
    a.add_argument('--commit')
    a.set_defaults(fn=cmd_add)
    sub.add_parser('prune').set_defaults(fn=cmd_prune)
    sub.add_parser('status').set_defaults(fn=cmd_status)
    h = sub.add_parser('homes')
    h.add_argument('tus', nargs='?')
    h.set_defaults(fn=cmd_homes)
    args = p.parse_args()
    args.fn(args)


if __name__ == '__main__':
    main()

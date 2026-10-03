"""Module-size census for the PAD_NOPS survey (tmp/pad-survey/PLAN.md section 1).

Modules: every (lib, mod, mod_start, mod_end) carrying at least one XDEF in
docs/naming/libscan/libsyms.json (libscan verbatim placements). Size = mod_end - mod_start.

Classification is by how OUR tree represents the module's XDEF functions, not by Sony's
source language:
  asm : an XDEF is supplied as INCLUDE_ASM("asm/funcs", X), a file-scope `glabel X` block,
        or BIOS_[ABC]_FUNCTION(X, ...)
  C   : an XDEF has a C definition in src/*.c
  ?   : none of the XDEFs found in src (e.g. libscan placements rejected as unreachable)
A module mixing both prints as 'asm/C' (none did); '?/asm' = some XDEFs unresolved.
Run from the repo root:  python tmp/pad-survey/census.py
"""
import collections, glob, json, re

d = json.load(open('docs/naming/libscan/libsyms.json'))
mods = {}
for es in d.values():
    for e in es:
        if e['kind'] != 'xdef':
            continue
        key = (e['lib'], e['mod'], e['mod_start'], e['mod_end'])
        mods.setdefault(key, []).append(e['name'])
src = ''.join(open(p, encoding='utf-8', errors='replace').read() for p in glob.glob('src/*.c'))


def kind(n):
    q = re.escape(n)
    if (re.search(r'INCLUDE_ASM\("asm/funcs",\s*' + q + r'\)', src)
            or re.search(r'glabel ' + q + r'\\n', src)
            or re.search(r'BIOS_[ABC]_FUNCTION\(' + q + ',', src)):
        return 'asm'
    if re.search(r'^[A-Za-z_][\w\s\*]*\b' + q + r'\s*\([^;]*\)\s*\{', src, re.M):
        return 'C'
    return '?'


rows = []
for (lib, mod, s, e), names in sorted(mods.items(), key=lambda x: int(x[0][2], 16)):
    size = int(e, 16) - int(s, 16)
    ks = '/'.join(sorted(set(kind(n) for n in names)))
    rows.append((lib, mod, s, size, size % 16, ks, ','.join(names)))
print('summary (class, size%16==0):', dict(collections.Counter((r[5], r[4] == 0) for r in rows)))
print('total modules:', len(rows))
print('lib\tmod\tstart\tsize\tsize%16\tclass\txdefs')
for r in rows:
    print('\t'.join([r[0], r[1], r[2], hex(r[3]), str(r[4]), r[5], r[6]]))

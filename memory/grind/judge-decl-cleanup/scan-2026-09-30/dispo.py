"""Build disposition.tsv: one row per scan.txt hit -> object/class or 'not debt: <reason>'.
Mechanical rows come from a per-function base->object map; MANUAL holds the hand-classified rows
(keyed by file:line). usage: python dispo.py <scan.txt> <out.tsv>   (prints unresolved hits)"""
import re
import sys

PMR = 'PracticeMenuRec record by byte offset [T]'
REC44 = 'Rec44 [T]'
SCR = 'scratchpad view (0x1F80xxxx)'
SCRB = 'scratchpad 0x1F8002B8 block passed as obj'
MODEL = 'model/geometry data'
R106 = 'D_80106A78 0x64-byte records'
R104 = 'D_80104E88 0x2C-byte records'
F5F68 = 'D_800F5F68 0x1B8-byte records [T]'
REC1C = 'Rec1C records via s32* params'
CALLB = 'call-boundary (s32 *) cast of a typed local'
STACKB = "caller's stack buffers (func_80023F08 sp+0x18 / sp+0x9C)"
UNTR = 'untraced object (no direct caller; offsets match D_80106A78 velocity fields)'

MAP = {
    'func_80018094': {'arg0': MODEL, 'dst': MODEL},
    'func_80018300': {'arg0': MODEL, 'base': MODEL, 'data': MODEL, 'p1': MODEL, 'p2': MODEL, 'out': SCR},
    'func_800187F4': {'arg0': MODEL, 'arg1': MODEL, 'node': MODEL},
    'func_8001A820': {'arg0': PMR + ' (unk_168 vector)', 'arg1': PMR + ' (unk_168 vector)', 'arg2': PMR,
                      'arg3': PMR, 'cam': REC44 + ' (func_8001A538 call cast)',
                      'scr': SCR + ' (CamScratch view of 0x1F800000; (s32 *)scr->... call-boundary casts)'},
    'func_8001B294': {'a0': PMR, 'a1': PMR},
    'func_8001B3C0': {'a0': PMR},
    'func_8001B748': {'base': PMR, 'D_80101EC8': PMR},
    'func_8001BAE4': {'arg0': REC1C, 'arg1': REC1C},
    'func_8001F2E4': {'obj': PMR, 'a': STACKB, 'b': STACKB},
    'func_800233AC': {'arg0': PMR, 'out1': CALLB},
    'func_80023648': {'arg0': PMR},
    'func_800283D0': {'arg0': PMR, 'temp_s4': PMR + ' (unk_00 record)', 'tail': PMR + ' (unk_00 record)'},
    'func_8002A458': {'obj': PMR, 'partner': PMR + ' (unk_00 record)', 'rec': F5F68, 'scr': SCR, 'pos': SCR,
                      'p': SCR},
    'func_8002D320': {'obj': SCRB, 'vin': SCRB, 'vout': SCRB},
    'func_8002D780': {'obj': SCRB, 'p118': SCRB, 'p124': SCRB, 'p10C': SCRB},
    'func_8002DAD0': {'obj': SCRB, 'mat': SCRB},
    'func_8002E838': {'obj': SCRB, 'mat': SCRB, 'vec': SCRB},
    'func_8002EA24': {'obj': SCRB, 'vin': SCRB, 'vout': SCRB},
    'func_8002EBDC': {'scr': SCR, 'mat': SCR, 'vec': SCR},
    'func_8002F2D0': {'scr': SCR, 'mat': SCR, 'vec': SCR, 'a0': MODEL + ' (MATRIX *)',
                      'a1': 's16 triple held in an s32 * param'},
    'func_8002F770': {'scr': SCR, 'init_scr': SCR, 'mat': SCR, 'vec': SCR},
    'func_8003032C': {'a0': UNTR},
    'func_80030580': {'obj': R106, 'src': PMR, 'arg0': PMR},
    'func_80030D7C': {'obj': R106, 'scr': SCR, 'nrm': SCR + ' (nrm; call-boundary (s32 *) cast)'},
    'func_80032064': {'src': PMR, 's0': R104, 'ptr': R104, 'v1': R104},
    'func_80032314': {'t0': R104, 'a3': R104},
    'func_800325E0': {'D_800A36B4': REC44 + ' (pointer held in s32 global D_800A36B4)'},
}

BASE_RES = [
    re.compile(r'\(\s*\w+\s*\*+\s*\)\s*\(\s*(?:\(\s*u8\s*\*\s*\)\s*)?&?\s*\(?\s*([A-Za-z_]\w*)'),
    re.compile(r'\(\s*\w+\s*\*+\s*\)\s*\(?\s*&?\s*([A-Za-z_]\w*)'),
    re.compile(r'\(\s*\(\s*\w+\s*\*\s*\)\s*([A-Za-z_]\w*)\s*\)\s*\['),
    re.compile(r'\b([A-Za-z_]\w*)\s*\+\s*(?:0x[0-9A-Fa-f]+|\d+)\s*\)'),
    re.compile(r'\*\s*\(\s*([A-Za-z_]\w*)\s*[+-]'),
]
TYPEWORDS = {'u8', 's8', 's16', 'u16', 's32', 'u32', 'int', 'void', 'char', 'short'}

MANUAL = {}   # 'file:line' -> disposition, filled from manual.tsv if present


def classify(fn, line, kinds, text):
    key = line
    if key in MANUAL:
        return MANUAL[key]
    if re.search(r'\(\s*\w+\s*\*\s*\)\s*0x1F80', text):
        return SCR
    m = MAP.get(fn, {})
    found = []
    for r in BASE_RES:
        for b in r.findall(text):
            if b in m and m[b] not in found:
                found.append(m[b])
    if found:
        return ' | '.join(found)
    if kinds == 'offset':
        if not re.search(r'\*\s*\(|\(\s*\w+\s*\*\s*\)', text):
            return 'not debt: integer arithmetic (no pointer arithmetic or cast on the line)'
    return None


def main(scan, out):
    import os
    mf = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'manual.tsv')
    if os.path.exists(mf):
        for row in open(mf, encoding='utf-8'):
            if row.strip() and not row.startswith('#'):
                k, v = row.rstrip('\n').split('\t', 1)
                MANUAL[k] = v
    rows, unresolved, fn = [], [], None
    for ln in open(scan, encoding='utf-8'):
        if ln.startswith('== '):
            fn = ln.split()[1]
            continue
        m = re.match(r'\s+(src/\S+:\d+)\s+\[([^\]]+)\]\s+(.*)$', ln)
        if not m:
            continue
        loc, kinds, text = m.groups()
        d = classify(fn, loc, kinds, text)
        if d is None:
            unresolved.append(f'{fn}\t{loc}\t{kinds}\t{text}')
            d = 'UNRESOLVED'
        rows.append(f'{fn}\t{loc}\t{kinds}\t{d}\t{text}')
    with open(out, 'w', encoding='utf-8', newline='\n') as f:
        f.write('function\tsource\tconstruct\tdisposition\ttext\n')
        f.write('\n'.join(rows) + '\n')
    print(f'{len(rows)} rows, {len(unresolved)} unresolved')
    for u in unresolved:
        print(u)


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2])

"""Q2 (a1)/(a2) spelling sweep for cdrom_StartAudio's CdState object (0x80101E58..).
Run under WSL from the repo root:
  python3 tmp/audit-2026-09-29/q2-startaudio/sa.py [--psx] [--files b5_post,b4_post,b5] [--diff FUNC] SPEC...
SPEC = LAYOUT:PAIRKIND:BODY
  LAYOUT  object boundaries, e.g. 'M' (one object E58..EA7), 'LANDED' (header+src untouched),
          or a '/'-separated list of object start offsets (hex, low byte of 0x80101Exx), each object
          running to the next start; a trailing '|END' sets where the last modelled object ends
          (default A8).  e.g. '58/64'  = [E58..E63][E64..EA7]
                               '58/64/66/68/6A/6C' = head, four s16 scalars, tail from E6C
  PAIRKIND S = CamPair member/object, A = s32[2], W = two s32 (pair_a, pair_b)
  BODY    cdrom_StartAudio copy spelling: <readback><copy>
          readback F = landed FAKE read-back (idx = unk00; cam = table; entry = cam + idx*8)
                   I = inline read (table + unk00*8)       R = CamPair-array index table[unk00]
                   G = no read-back, (s16)arg0*8           X = cdrom_StartRead's sval = (a0<<16)>>13
          copy     B = aggregate copy (PAIRKIND S only)    W = word copy through CamPair *e
Every other consumer in the chosen files is respelled mechanically through the layout."""
import sys, subprocess, json, re, hashlib
from pathlib import Path
sys.path.insert(0, '.')
from engine import score
from engine import buildconfig as cfg

WD = Path('tmp/audit-2026-09-29/q2-startaudio/runs')
args = sys.argv[1:]
psx = '--psx' in args; args = [a for a in args if a != '--psx']
difffn = None
if '--diff' in args:
    i = args.index('--diff'); difffn = args[i + 1]; del args[i:i + 2]
if '--wd' in args:
    i = args.index('--wd'); WD = Path(args[i + 1]); del args[i:i + 2]
vol = set()
if '--volatile' in args:  # set-aside measurement only: declare these objects volatile
    i = args.index('--volatile'); vol = set(args[i + 1].split(',')); del args[i:i + 2]
files = ['b5_post']
if '--files' in args:
    i = args.index('--files'); files = args[i + 1].split(','); del args[i:i + 2]

MEM = [('filter', 'CdlFILTER', 0x58, 4), ('o_unk04', 's32', 0x5C, 4), ('unk00', 's16', 0x60, 2),
       ('unk02', 's16', 0x62, 2), ('unk04', 's16', 0x64, 2), ('unk06', 's16', 0x66, 2),
       ('unk08', 's16', 0x68, 2), ('unk0A', 's16', 0x6A, 2), ('pair', 'CamPair', 0x6C, 8),
       ('unk14', 's32', 0x74, 4), ('unk18', 's32', 0x78, 4), ('unk1C', 's32', 0x7C, 4),
       ('sectors_remaining', 's32', 0x80, 4), ('dest_buffer', 's32', 0x84, 4), ('unk28', 's32', 0x88, 4),
       ('unk2C', 's32', 0x8C, 4), ('unk30', 'u8', 0x90, 1), ('unk34', 's32', 0x94, 4),
       ('unk38', 's16', 0x98, 2), ('unk3A', 's16', 0x9A, 2), ('unk3C', 's16', 0x9C, 2),
       ('unk3E', 'u16', 0x9E, 2), ('expected_pos', 's32', 0xA0, 4), ('unk44', 's32', 0xA4, 4)]


def symname(off):
    return f'D_80101E{off:02X}' if off < 0x100 else f'D_80101F{off - 0x100:02X}'


def layout_objs(lay, pk):
    """-> list of (sym, [(member, type, off)]) ; pair expanded by pk."""
    mem = []
    for n, t, o, sz in MEM:
        if n == 'pair' and pk == 'W':
            mem += [('pair_a', 's32', 0x6C), ('pair_b', 's32', 0x70)]
        elif n == 'pair' and pk == 'A':
            mem.append(('pair', 's32 [2]', 0x6C))
        else:
            mem.append((n, t, o))
    if lay == 'M':
        starts = [0x58]
    else:
        starts = [int(x, 16) for x in lay.split('/')]
    objs = []
    for i, s in enumerate(starts):
        e = starts[i + 1] if i + 1 < len(starts) else 0xA8
        ms = [m for m in mem if s <= m[2] < e]
        assert ms and ms[0][2] == s, f'object start {s:X} is not a member boundary'
        off = 0
        for n, t, o in ms:  # the object must be spellable in C: member offsets == address deltas
            al = {'u8': 1, 's16': 2, 'u16': 2, 'CdlFILTER': 2}.get(t, 4)
            off = (off + al - 1) // al * al
            assert off == o - s, f'{lay}: object at E{s:02X} cannot place {n} at +{o - s:X} (C puts it at +{off:X})'
            off += {'u8': 1, 's16': 2, 'u16': 2, 'CdlFILTER': 4, 'CamPair': 8, 's32 [2]': 8}.get(t, 4)
        if s % 4 and any({'u8': 1, 's16': 2, 'u16': 2, 'CdlFILTER': 2}.get(t, 4) == 4 for n, t, o in ms):
            raise AssertionError(f'{lay}: object at E{s:02X} holds a 4-aligned member but starts 2 mod 4')
        objs.append((symname(s), ms))
    return objs


def decl(t, name):
    return f's32 {name}[2];' if t == 's32 [2]' else f'{t} {name};'


def build(objs):
    hdr, where = '', {}
    for sym, ms in objs:
        if len(ms) == 1:
            n, t, o = ms[0]
            hdr += 'extern ' + ('volatile ' if sym in vol else '') + decl(t, sym) + '\n'
            where[n] = sym
        else:
            hdr += ('typedef struct { ' + ' '.join(decl(t, n) for n, t, o in ms) + f' }} T_{sym};\n'
                    f'extern {"volatile " if sym in vol else ""}T_{sym} {sym};\n')
            for n, t, o in ms:
                where[n] = f'{sym}.{n}'
    return hdr, where


def expr(tok, where, pk):
    if tok == '&pair':
        return {'S': f'&{where.get("pair")}', 'A': f'&{where.get("pair")}[0]', 'W': f'&{where.get("pair_a")}'}[pk]
    if tok in ('pair.a', 'pair.b'):
        ab = tok[-1]
        return {'S': f'{where.get("pair")}.{ab}', 'A': f'{where.get("pair")}[{"0" if ab == "a" else "1"}]',
                'W': where.get('pair_' + ab)}[pk]
    if tok == 'pair':
        assert pk == 'S', 'aggregate pair use under non-struct pair kind'
    return where[tok]


def tokenize(s):
    s = s.replace('        ReplayCamRec *rec = &D_80101E58.rec;\n', '')
    s = s.replace('&D_80101E58.rec.pair', '@@&pair@@')
    s = re.sub(r'D_80101E58\.rec\.pair\.([ab])', r'@@pair.\1@@', s)
    s = s.replace('D_80101E58.unk04', '@@o_unk04@@')
    s = s.replace('D_80101E58.filter', '@@filter@@')
    s = re.sub(r'D_80101E58\.rec\.(\w+)', r'@@\1@@', s)
    s = re.sub(r'\brec->(\w+)', r'@@\1@@', s)
    assert 'D_80101E58' not in s and 'rec->' not in s, 'unmapped use'
    return s


SA_HEAD = '''s32 cdrom_StartAudio(s32 arg0, s32 arg1) {
    s16 *s0 = &@@unk02@@;

    if (*s0 != 0) {
        return 0;
    }

    {
        extern u8 g_cd_file_table;
%s
    }
'''
SA_TAIL = '''
    {
        extern u8 g_cd_file_table;
        @@unk14@@ = CdPosToInt((s32)(&g_cd_file_table + @@unk00@@ * 8)) + (*(u32 *)((u8 *)&g_cd_file_table_plus_0x4 + (@@unk00@@ << 3)) >> 11) - 0x96;
    }

    if (arg1 < 0) {
        @@unk34@@ = 0;
        @@unk30@@ = 5;
    } else {
        @@unk34@@ = 1;
        @@filter@@.file = 1;
        @@filter@@.chan = arg1;
        CdControlB(0xD, (u8 *)&@@filter@@, 0);
        @@unk30@@ = 0xC8;
    }

    @@unk04@@ = 0;
    @@unk08@@ = 0;
    @@unk0A@@ = 0;
    @@unk02@@ = 0x10;

    return 1;
}
'''


def sa_copy(body):
    rb, cp = body[0], body[1]
    pre = ''
    if rb == 'F':
        pre = ('        s32 idx;\n        u8 *cam;\n        @@unk00@@ = arg0;\n        idx = @@unk00@@;\n'
               '        cam = &g_cd_file_table;\n')
        src = '(CamPair *)(cam + idx * 8)'
    elif rb == 'I':
        pre = '        @@unk00@@ = arg0;\n'
        src = '(CamPair *)(&g_cd_file_table + @@unk00@@ * 8)'
    elif rb == 'R':
        pre = '        @@unk00@@ = arg0;\n'
        src = '&((CamPair *)&g_cd_file_table)[@@unk00@@]'
    elif rb == 'G':
        pre = '        @@unk00@@ = arg0;\n'
        src = '(CamPair *)(&g_cd_file_table + (s16)arg0 * 8)'
    elif rb == 'X':
        pre = '        s32 sval = ((s32)(arg0 << 16)) >> 13;\n        @@unk00@@ = arg0;\n'
        src = '(CamPair *)((u8 *)&g_cd_file_table + sval)'
    else:
        raise SystemExit(f'bad readback {rb}')
    if cp == 'B':
        return pre + f'        @@pair@@ = *{src};'
    if cp == 'P':  # FAKE-family pointer alias to the destination (aggregate copy)
        return '        CamPair *dst;\n' + pre + f'        dst = &@@pair@@;\n        *dst = *{src};'
    if cp == 'W':
        return '        CamPair *e;\n' + pre + f'        e = {src};\n        @@pair.a@@ = e->a;\n        @@pair.b@@ = e->b;'
    raise SystemExit(f'bad copy {cp}')


FILES = {'b5_post': ('code6cac_b5_post', False), 'b4_post': ('code6cac_b4_post', False), 'b5': ('code6cac_b5', True)}
hdr_src = Path('include/code6cac.h').read_text()
H0 = hdr_src.index('typedef struct {\n    s16 unk00; /* 0x80101E60 */')
H1 = hdr_src.index('extern CdState D_80101E58;\n') + len('extern CdState D_80101E58;\n')
CDLF = re.search(r'/\* libcd CdlFILTER.*?\} CdlFILTER;\n', hdr_src, re.S).group(0)
EXTRA = {symname(o): 0x80101E00 + o for _, _, o, _ in MEM}
EXTRA.update({'D_80101E70': 0x80101E70})
REF = {}


def respell(stem, s, where, pk, body):
    if stem == 'code6cac_b5_post' and body != 'LANDED':
        a = s.index('s32 cdrom_StartAudio(s32 arg0, s32 arg1) {')
        b = s.index('s32 func_80037110(s32 arg0) {')
        s = s[:a] + (SA_HEAD % sa_copy(body)).replace('%%', '%') + SA_TAIL + s[b:]
    s = tokenize(s) if 'D_80101E58' in s or '@@' in s else s
    if pk != 'S':  # cdrom_StartRead's aggregate copy, word by word
        old = '    @@pair@@ = *(CamPair *)((u8 *)&g_cd_file_table + sval);\n'
        if old in s:
            s = s.replace(old, '    @@pair.a@@ = ((CamPair *)((u8 *)&g_cd_file_table + sval))->a;\n'
                               '    @@pair.b@@ = ((CamPair *)((u8 *)&g_cd_file_table + sval))->b;\n')
    return re.sub(r'@@([&\w.]+)@@', lambda m: expr(m.group(1), where, pk), s)


def run(spec):
    lay, pk, body = spec.split(':')
    if body[1:] in ('B', 'P') and pk != 'S':
        return None
    tag = f'{lay.replace("/", "-")}_{pk}_{body}' + ('_vol' + '-'.join(sorted(vol)) if vol else '') + ('_psx' if psx else '')
    d = WD / tag
    (d / 'inc').mkdir(parents=True, exist_ok=True)
    if lay == 'LANDED':
        hdr, where = hdr_src, None
    else:
        decls, where = build(layout_objs(lay, pk))
        hdr = hdr_src[:H0] + CDLF + decls + hdr_src[H1:]
    (d / 'inc' / 'code6cac.h').write_text(hdr)
    out = {}
    for fk in files:
        stem, gp = FILES[fk]
        src = Path(f'src/{stem}.c').read_text()
        s = src if lay == 'LANDED' and body == 'LANDED' else respell(stem, src, where or {}, pk, body)
        (d / f'{stem}.c').write_text(s)
        cc = 'tools/cc1psx_wrapper.sh' if psx else cfg.CC1
        flags = cfg.CC_FLAGS_GP if gp else cfg.CC_FLAGS
        pre = f'{cfg.CPP} -I{d}/inc {cfg.CPP_FLAGS} {cfg.CPP_DEFS} {d}/{stem}.c'
        r = subprocess.run(['bash', '-o', 'pipefail', '-c',
                            f'{pre} > {d}/{stem}.i && {cc} {flags} < {d}/{stem}.i > {d}/{stem}.s'],
                           capture_output=True, text=True)
        if r.stderr.strip():
            print(f'[{tag} {fk}] compile stderr:\n{r.stderr[-3000:]}')
        if r.returncode or not (d / f'{stem}.s').stat().st_size:
            out[fk] = 'COMPILE FAILED'; continue
        o = d / f'{stem}.o'
        mf = cfg.MASPSX_FLAGS_GP if gp else cfg.MASPSX_FLAGS
        post = f'{cfg.PROLOGUE_FIX} < {d}/{stem}.s | {cfg.MASPSX} {mf} | {cfg.MULTU_PAD} | {cfg.AS} {cfg.AS_FLAGS} -o {o}'
        r = subprocess.run(['bash', '-o', 'pipefail', '-c', post], capture_output=True, text=True)
        if r.stderr.strip():
            print(f'[{tag} {fk}] asm stderr:\n{r.stderr[-3000:]}')
        if r.returncode:
            out[fk] = 'ASSEMBLE FAILED'; continue
        st = score._symtab()
        for k, v in EXTRA.items():
            st.setdefault(k, v)
        ref = f'build/src/{stem}.o'
        res = {}
        for f in score._o_func_table(ref):
            try:
                sc = score.score_func(str(o), ref, f)['score']
            except KeyError:
                sc = 'MISSING'
            res[f] = sc
        out[fk] = res
        if difffn and difffn in res:
            dd = score.insn_diff(str(o), ref, difffn)
            (d / f'{difffn}.diff.json').write_text(json.dumps(dd, indent=1))
        if 'cdrom_StartAudio' in res:
            ins = score.normalized_insns(str(o), 'cdrom_StartAudio')
            (d / 'cdrom_StartAudio.insns').write_text('\n'.join(ins) + '\n')
            res['_sa_hash'] = hashlib.sha1('\n'.join(ins).encode()).hexdigest()[:8]
    return tag, out


CONSUMERS = ['cdrom_Init', 'cdrom_ReadyCallback', 'func_80036940', 'cdrom_IsIdle', 'cdrom_StartRead',
             'cdrom_StartReadAt', 'cdrom_Pause', 'game_FrameLoop', 'cdrom_StartAudio', 'func_80037110',
             'func_800371AC', 'func_800371E8', 'func_800371F8', 'func_80037234', 'func_80037250',
             'func_80037260', 'func_800372C0']
for spec in args:
    try:
        r = run(spec)
    except AssertionError as ex:
        print(f'{spec:40s} NOT SPELLABLE: {ex}'); continue
    if r is None:
        print(f'{spec:40s} (n/a)'); continue
    tag, out = r
    flat = {}
    for fk, res in out.items():
        if isinstance(res, str):
            flat[fk] = res
        else:
            flat.update(res)
    sa = flat.get('cdrom_StartAudio', '-')
    other = {k: v for k, v in flat.items() if k not in ('cdrom_StartAudio', '_sa_hash') and v != 0}
    print(f'{spec:40s} SA={sa!s:4s} h={flat.get("_sa_hash", "-")} other_nonzero={other or "none"}', flush=True)

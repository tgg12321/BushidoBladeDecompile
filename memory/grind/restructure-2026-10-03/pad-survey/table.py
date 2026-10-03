import json, subprocess
sites = [l.split() for l in subprocess.run(['python','tmp/pad-survey/gen.py'],capture_output=True,text=True).stdout.splitlines() if l.count(' ')==4]
nm = {}
for l in open('tmp/pad-survey/nm.txt'):
    p = l.split()
    if len(p) == 4 and p[2] in 'TtA' and p[3] not in nm: nm[p[3]] = (int(p[0],16), int(p[1],16))
    elif len(p) == 3 and p[1] in 'Tt': nm.setdefault(p[2], (int(p[0],16), None))
lib = json.load(open('docs/naming/libscan/libsyms.json'))
mods = {}
for es in lib.values():
    for e in es:
        mods[e['name']] = (e['lib'], e['mod'], int(e['mod_start'],16), int(e['mod_end'],16))
SIZE_NOSZ = {'_patch_gte': 0x9c, 'gte_ReadIR1IR2Sra2': 0x20}
print('| # | site | n | preceding | addr..end | next @ | mod16/mod8 | PsyQ module (span) | body | representation |')
print('|---|---|---|---|---|---|---|---|---|---|')
for i,(stem, ln, name, n, kind) in enumerate(sites, 1):
    n = int(n); a, s = nm[name]; s = s or SIZE_NOSZ[name]
    end = a + s; nxt = end + 4*n
    m = mods.get(name)
    if name == 'func_800790A4': m = mods['_send_pad']
    mtxt = f"{m[0]} {m[1]} {m[2]:08X}..{m[3]:08X}" if m else 'none (game code)'
    ok = m and m[3] == nxt
    body = {'inc':'INCLUDE_ASM (canonical)','c':'C + GTE island (canonical)','blk':'file-scope glabel block (canonical)'}[kind]
    rep = ('move nops to end of its .s' if kind=='inc' else 'nop into the block' if kind=='blk' else 'OWNER Q: whole-body .s (orig. asm module) incl. pad')
    if not m: rep = 'item 2: move nops to end of its .s'
    print(f"| {i} | {stem}.c:{ln} | {n} | {name} | {a:08X}..{end:08X} | {nxt:08X} | {nxt%16:X}/{nxt%8} | {mtxt}{' (ends at gap end)' if ok else ''} | {body} | {rep} |")

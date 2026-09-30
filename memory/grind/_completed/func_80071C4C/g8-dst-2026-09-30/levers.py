import subprocess, json, re
from pathlib import Path
t = Path('src/text1b_tu1d.c').read_text()
a = t.index('void func_80071C4C(s32 arg0) {'); b = t.index('\n}\n', a) + 3
body = t[a:b]
pat = re.compile(r'            s32 dst = i \* 10; /\*.*?\*/\n\n(\s*)\*\(u8 \*\)\(D_800A3568 \+ dst( \+ 1)?\)', re.S)
assert len(pat.findall(body)) == 2
forms = {
 'D+i*10':      lambda k: f'*(u8 *)(D_800A3568 + i * 10{" + 1" if k else ""})',
 'i*10+D':      lambda k: f'*(u8 *)(i * 10 + D_800A3568{" + 1" if k else ""})',
 'D+10*i':      lambda k: f'*(u8 *)(D_800A3568 + 10 * i{" + 1" if k else ""})',
 'u8ptr[i*10]': lambda k: f'((u8 *)D_800A3568)[i * 10{" + 1" if k else ""}]',
 'rows[i][k]':  lambda k: f'((u8 (*)[10])D_800A3568)[i][{k}]',
}
out = []
for name, f in forms.items():
    nb = pat.sub(lambda m: m.group(1) + f(1 if m.group(2) else 0), body)
    p = f'/tmp/g8probe/c4c_lever_{re.sub(r"[^A-Za-z0-9]", "_", name)}.c'; Path(p).write_text(nb)
    r = subprocess.run(['python3','-m','engine.cli','sandbox','func_80071C4C','--disable','all','--candidate',p], capture_output=True, text=True)
    d = json.loads(r.stdout); line = f'{name}: score {d["score"]} ({d["build_insns"]}/{d["target_insns"]}) {r.stderr.strip()[-200:]}'
    print(line); out.append(line)
Path('tmp/audit-2026-09-29/g8-dump71c4c/levers.txt').write_text('\n'.join(out) + '\n')

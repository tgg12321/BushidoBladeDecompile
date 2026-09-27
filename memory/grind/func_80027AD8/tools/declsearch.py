"""Random declaration-order / init-order search; prints allocation signature per variant.
usage (WSL): python3 declsearch.py <base.c> <n> <seed>"""
import random, subprocess, sys, re, os
root = '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile'
base = open(sys.argv[1]).read()
n = int(sys.argv[2]); random.seed(int(sys.argv[3]))
decl_re = re.compile(r'(\{\n)((?:    [^\n]*;\n)+)\n', re.M)
m = decl_re.search(base)
decls = m.group(2).splitlines(keepends=True)
inits = ["    vec = &D_800A37E8;\n", "    player = *(s16 *)(ch + 4);\n", "    opp = *(u8 **)ch;\n"]
tail = "    scr = (u8 *)0x1F8000A8 + limb * 12 + player * 0x108;\n"
assert all(i in base for i in inits)
os.makedirs(f'{root}/tmp/func_80027AD8/ds', exist_ok=True)
seen = {}
for k in range(n):
    d = decls[:]; random.shuffle(d)
    it = inits[:]; random.shuffle(it)
    src = base[:m.start(2)] + ''.join(d) + base[m.end(2):]
    blk = ''.join(inits) + tail
    src = src.replace(blk, ''.join(it) + tail)
    p = f'{root}/tmp/func_80027AD8/ds/v{k}.c'
    open(p, 'w').write(src)
    out = subprocess.run(['bash', f'{root}/tmp/func_80027AD8/quick.sh', p], capture_output=True, text=True).stdout
    sig = out.split('\n')[0]
    key = re.sub(r'\s+', ' ', sig)
    seen.setdefault(key, []).append(k)
for key, ks in seen.items():
    print(len(ks), ks[:5], key)

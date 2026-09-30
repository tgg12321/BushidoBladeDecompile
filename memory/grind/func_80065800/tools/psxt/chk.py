import sys, re
L = [l.strip() for l in open(sys.argv[1]) if l.strip() and not l.strip().startswith(('.set', '#'))]
i = [k for k, l in enumerate(L) if 'D_800F0BA8' in l][0]
j = [k for k in range(i, len(L)) if L[k].startswith('slt') and L[k].endswith(',8')][0]
k = [m for m in range(j, len(L)) if 'D_800A3490' in L[m]][0]
head = L[i-2:j+2]; then = L[j+2:k]
at = any('D_800F0BA8(' in l for l in L[i:k+10])
reload = [m for m, l in enumerate(then) if re.match(r'lh\s', l)]
st = [m for m, l in enumerate(then) if 'D_800A3488' in l]
hoist = (reload and st and reload[0] < st[0])
lafirst = None
la = [m for m, l in enumerate(head) if l.startswith('la') and 'D_800F0BA8' in l]
sll = [m for m, l in enumerate(head) if l.startswith('sll') and l.endswith(',1')]
if la and sll: lafirst = la[0] < sll[-1]
print(sys.argv[1].split('/')[-1], 'at' if at else 'reg', 'reload' if reload else 'cse', 'hoist' if hoist else '-', 'lafirst' if lafirst else '-')

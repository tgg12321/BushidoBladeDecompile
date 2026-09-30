"""Print scored hunks (compact) of sweep outputs: python hk.py name [name...]"""
import sys, os
d = os.path.join(os.path.dirname(__file__), '..', 'sandbox_sweep', 'func_800759D0')
for name in sys.argv[1:]:
    p = os.path.join(d, name + '.c.txt')
    lines = open(p, encoding='utf-8').read().splitlines()
    print('=====', name)
    keep = False
    for ln in lines:
        if ln.startswith('@ hunk'):
            keep = 'not-scored' not in ln
            if keep:
                print(ln.split('  —')[0])
            continue
        if keep and ln.strip():
            print(ln)
        if ln.startswith('  NOTE'):
            break

import sys, itertools, subprocess, re
base = open(sys.argv[1]).read()
m = re.search(r"(        (mask_a|mask_b|mask_c|npass|hit|deep) = 0;\n){6}", base)
old = m.group(0)
stm = ["mask_a", "mask_b", "mask_c", "npass", "hit", "deep"]
want = ["sw\t$0,56($sp)", "sw\t$0,64($sp)", "sw\t$0,72($sp)", "lh\t$2,150($19)", "move\t$9,$0", "sw\t$0,32($sp)"]
hits = []
for p in itertools.permutations(stm):
    t = base.replace(old, "".join("        %s = 0;\n" % x for x in p))
    fn = 'tmp/func_8002AB08/perm/f.c'
    open(fn, 'w', newline='\n').write(t)
    subprocess.run(['bash', 'tmp/func_8002AB08/fast.sh', fn])
    L = [l.strip() for l in open(fn + '.s')]
    i = next(k for k, l in enumerate(L) if l.startswith('addu\t$19,$19,1100'))
    seq = L[i + 1:i + 1 + len(want)]
    ok = seq == want
    if ok:
        hits.append(p)
    print(ok, ' '.join(p), '|', ' ; '.join(seq[:8]), flush=True)
print("HITS", hits)

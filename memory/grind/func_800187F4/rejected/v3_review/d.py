import difflib,sys
t=sys.argv[1]
a=[l.split(' ',1)[1] for l in open('tmp/func_800187F4/tgt.s').read().splitlines() if ' ' in l]
b=[l.split(' ',1)[1] for l in open(f'tmp/func_800187F4/fast_{t}/ours.s').read().splitlines() if ' ' in l]
sm=difflib.SequenceMatcher(None,a,b,autojunk=False)
n=0
for op,i1,i2,j1,j2 in sm.get_opcodes():
    if op!='equal':
        n+=1
        if n<=int(sys.argv[2] if len(sys.argv)>2 else 40): print(op,i1,a[i1:i2][:6],'||',b[j1:j2][:6])

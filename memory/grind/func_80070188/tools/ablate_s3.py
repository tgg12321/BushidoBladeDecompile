import re,sys
src=open('memory/grind/func_80070188/candidate.c').read()
V={}
s=re.sub(r"        /\* FAKE: named intermediate.*?\*/\n        s32 rec = i \* 3;\n\n","",src,flags=re.S)
assert s!=src
V['no_rec']=s.replace("D_800A3560[rec + 1]","D_800A3560[i * 3 + 1]").replace("D_800A3560[rec]","D_800A3560[i * 3]")
D="                    s32 idx; /* FAKE: per-walk record offset, mechanism at `rec` */\n\n"
parts=src.split(D)
assert len(parts)==5
def strip(seg): return seg.replace("                    idx = i * 3;\n","",1).replace("D_800A3560[idx + 1]","D_800A3560[i * 3 + 1]",1)
for n in range(1,5):
    out=parts[0]
    for m in range(1,5):
        out+=("" if m==n else D)+(strip(parts[m]) if m==n else parts[m])
    V['no_idx%d'%n]=out
V['no_idx']=parts[0]+''.join(strip(p) for p in parts[1:])
s=src.replace("                s32 sel = i * 3; /* FAKE: record offset, mechanism at `rec` */\n\n","")
V['no_sel']=s.replace("D_800A3560[sel + 2]","D_800A3560[i * 3 + 2]").replace("D_800A3560[sel] =","D_800A3560[i * 3] =")
s=src.replace("                    s32 ofs = i * 3; /* FAKE: record offset, mechanism at `rec` */\n\n","")
V['no_ofs']=s.replace("D_800A3560[ofs]","D_800A3560[i * 3]")
names=[]
for k,v in V.items():
    assert v!=src and (k!='no_sel' or '[sel' not in v) and (k!='no_ofs' or 'ofs]' not in v),k
    fn='tmp/func_80070188/s3/fin_%s.c'%k
    open(fn,'w',newline='\n').write(v); names.append(fn)
open('tmp/func_80070188/s3/fin.lst','w',newline='\n').write(' '.join(names))

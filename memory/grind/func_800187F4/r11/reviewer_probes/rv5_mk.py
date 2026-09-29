import sys, re
R='/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/' if sys.platform!='win32' else ''
T=open(R+'memory/grind/func_800187F4/template.c',encoding='utf-8').read()
def rep(s,a,b,n=1):
    c=s.count(a)
    assert c>=n, (a,c)
    return s.replace(a,b) if n=='all' else s.replace(a,b,n)
def work_split(s):
    s=rep(s,'                s32 work;\n','                s32 sq1;\n                s32 dist1;\n')
    s=rep(s,'work = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];\n                temp = work;\n                if (work < 0x400) {\n                    work = (&D_8008D118)[work] >> 3;',
            'sq1 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];\n                temp = sq1;\n                if (sq1 < 0x400) {\n                    dist1 = (&D_8008D118)[sq1] >> 3;')
    s=rep(s,'temp = (&D_8008D118)[work >> nbits];\n                    work = (temp << 16)','temp = (&D_8008D118)[sq1 >> nbits];\n                    dist1 = (temp << 16)')
    s=rep(s,'if (work >= r)','if (dist1 >= r)')
    s=rep(s,'tot = work + dist2;','tot = dist1 + dist2;')
    s=rep(s,'if (work != 0) {\n                    work = pen / work;','if (dist1 != 0) {\n                    dist1 = pen / dist1;')
    s=rep(s,'@gte_lddp(work);','@gte_lddp(dist1);')
    return s
V={}
V['rv5_base']=T
w=work_split(T); V['rv5_wk_pv']=w
V['rv5_wk_cmp_copy']=rep(w,'if (sq1 < 0x400) {','if (temp < 0x400) {')
V['rv5_wk_small_copy']=rep(w,'dist1 = (&D_8008D118)[sq1] >> 3;','dist1 = (&D_8008D118)[temp] >> 3;')
V['rv5_wk_idx_copy']=rep(w,'temp = (&D_8008D118)[sq1 >> nbits];','temp = (&D_8008D118)[temp >> nbits];')
x=rep(V['rv5_wk_cmp_copy'],'dist1 = (&D_8008D118)[sq1] >> 3;','dist1 = (&D_8008D118)[temp] >> 3;')
V['rv5_wk_allcopy']=rep(x,'temp = (&D_8008D118)[sq1 >> nbits];','temp = (&D_8008D118)[temp >> nbits];')
# nbits fully inline, no locals
n=rep(T,'                    s32 nbits;\n','')
n=rep(n,'nbits = lz[0];\n                    nbits = 0x16 - (nbits & ~1);\n                    temp = (&D_8008D118)[work >> nbits];\n                    work = (temp << 16) >> (0x13 - (nbits >> 1));',
        'temp = (&D_8008D118)[work >> (0x16 - (lz[0] & ~1))];\n                    work = (temp << 16) >> (0x13 - ((0x16 - (lz[0] & ~1)) >> 1));')
V['rv5_nb_inline']=n
# nbits split, lzcount at ellipsoid-body scope
n=rep(T,'                    s32 nbits;\n','                    s32 shift;\n')
n=rep(n,'                s32 work;\n','                s32 work;\n                s32 lzcount;\n')
n=rep(n,'nbits = lz[0];\n                    nbits = 0x16 - (nbits & ~1);\n                    temp = (&D_8008D118)[work >> nbits];\n                    work = (temp << 16) >> (0x13 - (nbits >> 1));',
        'lzcount = lz[0];\n                    shift = 0x16 - (lzcount & ~1);\n                    temp = (&D_8008D118)[work >> shift];\n                    work = (temp << 16) >> (0x13 - (shift >> 1));')
V['rv5_nb_ellscope']=n
# nbits split, shift computed with lzcount in the index expression folded: shift read via lz directly after lzcount
n=rep(T,'                    s32 nbits;\n','                    s32 lzcount, shift;\n')
n=rep(n,'nbits = lz[0];\n                    nbits = 0x16 - (nbits & ~1);\n                    temp = (&D_8008D118)[work >> nbits];\n                    work = (temp << 16) >> (0x13 - (nbits >> 1));',
        'lzcount = lz[0];\n                    shift = 0x16 - (lzcount & ~1);\n                    temp = (&D_8008D118)[work >> shift];\n                    work = (temp << 16) >> (0x13 - (shift >> 1));')
V['rv5_nb_onedecl']=n
# delta split, both declared at collision scope, dy0 first
def delta_split(s,decl):
    s=rep(s,'            s32 delta;\n',decl)
    s=rep(s,'delta = SCR->pos[1] - SCR->ground;\n            if (delta > 0) {\n                if (delta > 0x3200) {','dg = SCR->pos[1] - SCR->ground;\n            if (dg > 0) {\n                if (dg > 0x3200) {')
    s=rep(s,'vy_new = vy - delta / 8;','vy_new = vy - dg / 8;')
    s=rep(s,'delta = SCR->cpos[1] - SCR->sph[idx][1];\n                if (delta < -r || r < delta) {','dy0 = SCR->cpos[1] - SCR->sph[idx][1];\n                if (dy0 < -r || r < dy0) {')
    s=rep(s,'SCR->d0[1] = delta;','SCR->d0[1] = dy0;')
    return s
V['rv5_dl_coll_dy0first']=delta_split(T,'            s32 dy0;\n            s32 dg;\n')
V['rv5_dl_coll_dgfirst']=delta_split(T,'            s32 dg;\n            s32 dy0;\n')
d=delta_split(T,'            s32 dg;\n')
d=rep(d,'                s32 dx0, dz0, dy1, dx1, dz1;\n','                s32 dy0, dx0, dz0, dy1, dx1, dz1;\n')
V['rv5_dl_dy0_in_list']=d
d=delta_split(T,'            s32 dg;\n')
d=rep(d,'                s32 dx0, dz0, dy1, dx1, dz1;\n','                s32 dx0, dz0, dy1, dx1, dz1;\n                s32 dy0;\n')
d=rep(d,'dg = SCR->pos[1] - SCR->ground;\n            if (dg > 0) {','if (SCR->pos[1] > SCR->ground) {\n                dg = SCR->pos[1] - SCR->ground;')
V['rv5_dl_cmp_direct']=d
# nforce: each count block-local in a compound block around its loop
nf=rep(T,'        s32 nforce;\n','')
nf=rep(nf,'        nforce = node[7];\n        bits = node[9];\n        for (idx = 0; idx < nforce; idx++) {','        {\n        s32 nforce_add = node[7];\n        bits = node[9];\n        for (idx = 0; idx < nforce_add; idx++) {')
nf=rep(nf,'        nforce = node[8];\n        bits2 = node[11];\n        for (idx = 0; idx < nforce; idx++) {','        }\n        {\n        s32 nforce_sub = node[8];\n        bits2 = node[11];\n        for (idx = 0; idx < nforce_sub; idx++) {')
nf=rep(nf,'        SCR->vel[0] = vx;\n','        }\n        SCR->vel[0] = vx;\n')
V['rv5_nf_blocks']=nf
V['rv5_nf_blocks_const']=nf.replace('s32 nforce_add =','const s32 nforce_add =').replace('s32 nforce_sub =','const s32 nforce_sub =')
for k,v in V.items():
    open(R+'tmp/func_800187F4/'+k+'.c','w',encoding='utf-8',newline='\n').write(v)
print(' '.join(V))

import re
D='tmp/func_800187F4/'
T=open(D+'rv6_base.c',encoding='utf-8').read()
def rep(s,a,b):
    assert s.count(a)==1,(a,s.count(a)); return s.replace(a,b)
V={}
# temp split: copy lzc_in, table bytes typed u8 / u16, at arm scope
def tsplit(s,bt,scope_arm=True):
    s=rep(s,'                temp = work;\n','                lzc_in = work;\n')
    s=rep(s,'@gte_Lzc(temp, &lz[0]);','@gte_Lzc(lzc_in, &lz[0]);')
    s=rep(s,'temp = (&D_8008D118)[work >> nbits];\n                    work = (temp << 16)','byte1 = (&D_8008D118)[work >> nbits];\n                    work = (byte1 << 16)')
    s=rep(s,'temp = (&D_8008D118)[sq2 >> nbits2];\n                    dist2 = (temp << 16)','byte2 = (&D_8008D118)[sq2 >> nbits2];\n                    dist2 = (byte2 << 16)')
    s=rep(s,'                s32 temp;\n','                s32 lzc_in;\n')
    if scope_arm:
        s=rep(s,'                    s32 nbits;\n','                    s32 nbits;\n                    %s byte1;\n'%bt)
        s=rep(s,'                    s32 nbits2;\n','                    s32 nbits2;\n                    %s byte2;\n'%bt)
    else:
        s=rep(s,'                s32 lzc_in;\n','                s32 lzc_in;\n                %s byte1, byte2;\n'%bt)
    return s
V['rv6_tp_u8']=tsplit(T,'u8')
V['rv6_tp_u16']=tsplit(T,'u16')
V['rv6_tp_u8_ell']=tsplit(T,'u8',False)
V['rv6_tp_s32_ell_onedecl']=tsplit(T,'s32',False)
# nforce: one count kept as a local, the other read in its loop test (reuse-free structural)
nf=rep(T,'        nforce = node[8];\n','')
nf=rep(nf,'        for (idx = 0; idx < nforce; idx++) {\n            s32 *f_sub;','        for (idx = 0; idx < node[8]; idx++) {\n            s32 *f_sub;')
V['rv6_nf_sub_intest']=nf
nf=rep(T,'        nforce = node[7];\n','')
nf=rep(nf,'        for (idx = 0; idx < nforce; idx++) {\n            s32 *f_add;','        for (idx = 0; idx < node[7]; idx++) {\n            s32 *f_add;')
V['rv6_nf_add_intest']=nf
# delta: ground depth held in vy_new's neighbour? no -- per-value with dg as the assignment in the condition
d=rep(T,'            s32 delta;\n','            s32 dg;\n')
d=rep(d,'                s32 dx0, dz0, dy1, dx1, dz1;\n','                s32 dy0, dx0, dz0, dy1, dx1, dz1;\n')
d=rep(d,'            delta = SCR->pos[1] - SCR->ground;\n            if (delta > 0) {\n                if (delta > 0x3200) {','            if ((dg = SCR->pos[1] - SCR->ground) > 0) {\n                if (dg > 0x3200) {')
d=rep(d,'vy_new = vy - delta / 8;','vy_new = vy - dg / 8;')
d=rep(d,'delta = SCR->cpos[1] - SCR->sph[idx][1];\n                if (delta < -r || r < delta) {','dy0 = SCR->cpos[1] - SCR->sph[idx][1];\n                if (dy0 < -r || r < dy0) {')
d=rep(d,'SCR->d0[1] = delta;','SCR->d0[1] = dy0;')
V['rv6_dl_condassign']=d
for k,v in V.items():
    open(D+k+'.c','w',encoding='utf-8',newline='\n').write(v)
print(' '.join(V))

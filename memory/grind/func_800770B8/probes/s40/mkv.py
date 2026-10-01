"""mkv.py: variants of k0's post-call block -> tmp/func_800770B8/v_<name>.c"""
import sys
k0 = open('tmp/func_800770B8/k0.c').read()
OLD = """        s32 *prev = p_old;
        p_old = (s32 *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = (u8 *)p_old;
        ((SelWork *)p_old)->f04 = prev;
        SELWORK->f30 = 0;
        SELWORK->f34 = 0;
"""
assert k0.count(OLD) == 1
V = {
 'p1': """        s32 *prev = p_old;
        p_old = (s32 *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = (u8 *)p_old;
        *++p_old = (s32)prev;
        SELWORK->f30 = 0;
        SELWORK->f34 = 0;
""",
 'p2': """        s32 *prev = p_old;
        p_old = (s32 *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = (u8 *)p_old;
        p_old++;
        *p_old = (s32)prev;
        SELWORK->f30 = 0;
        SELWORK->f34 = 0;
""",
}
for k, v in V.items():
    open(f'tmp/func_800770B8/v_{k}.c', 'w', newline='\n').write(k0.replace(OLD, v))
print(' '.join(f'tmp/func_800770B8/v_{k}.c' for k in V))
V2 = {
 'p3': """        s32 *prev = p_old;
        p_old = (s32 *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = (u8 *)p_old;
        ((SelWork *)p_old)->f04 = prev;
        p_old = (s32 *)SELWORK;
        ((SelWork *)p_old)->f30 = 0;
        ((SelWork *)p_old)->f34 = 0;
""",
 'p4': """        s32 *prev = p_old;
        p_old = (s32 *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = (u8 *)p_old;
        ((SelWork *)p_old)->f04 = prev;
        p_old = (s32 *)SELWORK;
        ((SelWork *)p_old)->f30 = 0;
        SELWORK->f34 = 0;
""",
}
for k, v in V2.items():
    open(f'tmp/func_800770B8/v_{k}.c', 'w', newline='\n').write(k0.replace(OLD, v))
V3 = {
 'p5': """        s32 *prev = p_old;
        p_old = (s32 *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = (u8 *)p_old;
        p_old = (s32 *)&((SelWork *)p_old)->f04;
        *p_old = (s32)prev;
        SELWORK->f30 = 0;
        SELWORK->f34 = 0;
""",
}
for k, v in V3.items():
    open(f'tmp/func_800770B8/v_{k}.c', 'w', newline='\n').write(k0.replace(OLD, v))

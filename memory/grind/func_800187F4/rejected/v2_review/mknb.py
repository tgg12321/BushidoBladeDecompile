V='memory/grind/func_800187F4/r11/variants_v2/'
O='tmp/func_800187F4/'
tags=[]
b=open(V+'r11pv_nbits.c').read()
D='                    s32 lzcount;\n                    s32 shift;\n'
A='                    lzcount = lz[0];\n                    shift = 0x16 - (lzcount & ~1);\n'
assert D in b and A in b
def w(tag,s):
    assert s!=b, tag
    open(O+tag+'.c','w',newline='\n').write(s); tags.append(tag)
w('nb_N1', b.replace(D,D+'                    s32 even;\n').replace(A,'                    lzcount = lz[0];\n                    even = lzcount & ~1;\n                    shift = 0x16 - even;\n'))
w('nb_N3', b.replace(A,'                    lzcount = lz[0] & ~1;\n                    shift = 0x16 - lzcount;\n'))
for ty in ['u32','s16','u16','int','unsigned int','long']:
    w('nb_N5_'+ty.replace(' ','_'), b.replace('                    s32 lzcount;\n','                    %s lzcount;\n'%ty))
w('nb_N6', b.replace(A,'                    lzcount = lz[0];\n                    shift = 0x16 - (lzcount & -2);\n'))
w('nb_N10', b.replace(A,'                    lzcount = lz[0];\n                    shift = 0x16 - (lzcount & 0xFFFFFFFE);\n'))
w('nb_N11', b.replace(A,'                    lzcount = lz[0];\n                    shift = -(lzcount & ~1) + 0x16;\n'))
w('nb_CE1', b.replace('[sq1 >> shift]','[sq1 >> (shift + lzcount - lzcount)]'))
w('nb_CE2', b.replace('(0x13 - (shift >> 1))','(0x13 - (shift >> 1) + lzcount - lzcount)'))
w('nb_DS1', b.replace(A,A+'                    lzcount = 0; /* FAKE */\n'))
w('nb_DS2', b.replace(A,'                    lzcount = 0; /* FAKE */\n'+A))
w('nb_DS3', b.replace(A,'                    shift = 0; /* FAKE */\n'+A))
w('nb_SA1', b.replace(A,A+'                    lzcount = lzcount; /* FAKE */\n'))
w('nb_DW1', b.replace(A,'                    do { lzcount = lz[0]; } while (0); /* FAKE */\n                    shift = 0x16 - (lzcount & ~1);\n'))
w('nb_DW2', b.replace(A,'                    lzcount = lz[0];\n                    do { shift = 0x16 - (lzcount & ~1); } while (0); /* FAKE */\n'))
w('nb_F6a', b.replace(A,A+'                    if (!lzcount) { } /* !FAKE */\n'))
w('nb_F6b', b.replace(A,'                    lzcount = lz[0];\n                    lzcount++; lzcount--; /* !FAKE */\n                    shift = 0x16 - (lzcount & ~1);\n'))
w('nb_F6c', b.replace(A,A+'                    shift++; shift--; /* !FAKE */\n'))
open('tmp/rv187/tags_c.txt','w',newline='\n').write('\n'.join(tags)+'\n'); print(len(tags))

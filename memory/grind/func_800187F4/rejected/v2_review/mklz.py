V='memory/grind/func_800187F4/r11/variants_v2/'
O='tmp/func_800187F4/'
tags=[]
def w(tag,s,base):
    assert s!=base, tag
    open(O+tag+'.c','w',newline='\n').write(s); tags.append(tag)
for bn in ['r11abl_temp_lzc_in','r11pv_temp']:
    b=open(V+bn+'.c').read()
    decl='                s32 lzc_in;\n'
    cp='                lzc_in = sq1;\n'
    lz='                    @gte_Lzc(lzc_in, &lz[0]);\n'
    assert decl in b and cp in b and lz in b
    # P1: copy inside else arm right before Lzc, decl in arm
    s=b.replace(decl,'').replace(cp,'').replace(lz,'                    s32 lzc_in;\n\n                    lzc_in = sq1;\n'+lz if False else lz)
    # arm declarations: insert after arm open "} else {" following sq1 < 0x400
    i=s.index('if (sq1 < 0x400) {'); j=s.index('} else {',i)+len('} else {\n')
    s1=s[:j]+'                    s32 lzc_in;\n'+s[j:]
    s1=s1.replace(lz,'                    lzc_in = sq1;\n'+lz)
    w('lz_%s_P1'%bn,s1,b)
    # P2: decl at ellipsoid scope, copy in arm
    s2=b.replace(cp,'').replace(lz,'                    lzc_in = sq1;\n'+lz); w('lz_%s_P2'%bn,s2,b)
    # P3: copy before sq1 compute? impossible; copy after if-test split: copy at arm top before comment
    # P4: function-scope decl, copy in arm
    s4=b.replace(decl,'').replace(cp,'').replace(lz,'                    lzc_in = sq1;\n'+lz)
    s4=s4.replace('    s32 tot, pen;\n','    s32 tot, pen;\n    s32 lzc_in;\n'); w('lz_%s_P4'%bn,s4,b)
    # P5: function-scope decl, copy at same place
    s5=b.replace(decl,'').replace('    s32 tot, pen;\n','    s32 tot, pen;\n    s32 lzc_in;\n'); w('lz_%s_P5'%bn,s5,b)
    # P6: copy in arm + dead store lzc_in=0 before tot
    s6=s2.replace('                sq1 = 0;\n','                sq1 = 0;\n                lzc_in = 0; /* FAKE */\n'); w('lz_%s_P6'%bn,s6,b)
    # P7: copy at original place, FAKE dead store of lzc_in right after Lzc in arm
    s7=b.replace(lz, lz+'                    lzc_in = 0; /* FAKE */\n'); w('lz_%s_P7'%bn,s7,b)
    # P8: self-assign in arm
    s8=b.replace(lz, lz+'                    lzc_in = lzc_in; /* FAKE */\n'); w('lz_%s_P8'%bn,s8,b)
    # P9: copy declared with initializer at its decl point (s32 lzc_in = sq1;) after sq1 computed -> in a new block
    s9=b.replace(decl,'').replace(cp,'').replace(lz,'                    s32 lzc_in = sq1;\n'+lz) 
    # needs decl at block start: arm starts with comment + decl nbits; place at arm start
    s9=b.replace(decl,'').replace(cp,'')
    i=s9.index('if (sq1 < 0x400) {'); j=s9.index('} else {',i)+len('} else {\n')
    s9=s9[:j]+'                    s32 lzc_in = sq1;\n'+s9[j:]; w('lz_%s_P9'%bn,s9,b)
open('tmp/rv187/tags_b.txt','w',newline='\n').write('\n'.join(tags)+'\n'); print(len(tags))

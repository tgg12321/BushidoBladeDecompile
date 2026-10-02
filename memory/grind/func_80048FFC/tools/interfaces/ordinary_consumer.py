from pathlib import Path
import sys,re,itertools
sys.path.insert(0,'.')
from engine import cheats,pipeline,score,inlineasm
wd=Path('tmp/codex_cam/plain_consumer')
s=Path(wd/'plain.c').read_text()
old='''        s32 base_val;
        if (func_8003F268() == 0) {
            base_val = 0x6590;
        } else {
            base_val = 0x55F0;
        }
        step = base_val - arg0;'''
forms=[
'''        s32 base_val = func_8003F268() == 0 ? 0x6590 : 0x55F0;
        step = base_val - arg0;''',
'''        s32 base_val;
        if (func_8003F268()) base_val = 0x55F0;
        else base_val = 0x6590;
        step = base_val - arg0;''',
'''        s32 base_val;
        s32 players = func_8003F268();
        if (players == 0) base_val = 0x6590;
        else base_val = 0x55F0;
        step = base_val - arg0;''',
'''        s32 base_val = 0x55F0;
        if (func_8003F268() == 0) base_val = 0x6590;
        step = base_val - arg0;''',
'''        if (func_8003F268() == 0) step = 0x6590 - arg0;
        else step = 0x55F0 - arg0;''',
'''        step = (func_8003F268() == 0 ? 0x6590 : 0x55F0) - arg0;''',
'''        s32 base_val;
        if (!func_8003F268()) base_val = 0x6590;
        else base_val = 0x55F0;
        step = -arg0 + base_val;''',
]
for ix,f in enumerate(forms):
 assert old in s
 s2=s.replace(old,f)
 p=wd/('ordinary'+str(ix)+'.c');p.write_text(s2,newline='\n');clean,_=inlineasm.strip_cheat_asm_file(s2);p2=wd/('ordinary'+str(ix)+'.build.c');p2.write_text(clean,newline='\n');ov=cheats.empty_overrides(str(wd/'cfg'));ov['src_override']=str(p2);o=str(wd/('ordinary'+str(ix)+'.o'));pipeline.build_c_object('code6cac_c2',o,cheat_overrides=ov)
 print('ordinary'+str(ix),score.score_func(o,'build/src/code6cac_c2.o','func_8003DBE4'),flush=True)

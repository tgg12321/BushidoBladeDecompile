from pathlib import Path
import sys,re
sys.path.insert(0,'.')
from engine import cheats,pipeline,score,inlineasm
from engine.cli import _print_insn_diff
wd=Path('tmp/codex_cam/plain_consumer');wd.mkdir(exist_ok=True)
(wd/'gpu.h').write_text(Path('tmp/codex_cam/sotn_interfaces/gpu.h').read_text(),newline='\n')
s=Path('tmp/codex_cam/sotn_interfaces/code6cac_c2.c').read_text()
dead='            step = base_val - arg0; /* FAKE: steers base_val into v0 to match target reg-alloc */\n'
plain=s.replace(dead,'')
variants=[('plain',plain),('split',plain.replace('        step = base_val - arg0;','        step = base_val;\n        step -= arg0;'))]
variants.append(('direct',plain.replace('        s32 base_val;\n','').replace('base_val = 0x','step = 0x').replace('        step = base_val - arg0;','        step -= arg0;')))
for n in range(1,6):
 for site in ['        step = base_val - arg0;','            base_val = 0x55F0;','            base_val = 0x6590;']:
  f=plain.replace(site,site+'\n        '+' '.join(['base_val++; base_val--;']*n)+' /* !FAKE: F6 bounded base-value reference probe. */' if 'base_val =' in site else '        '+' '.join(['base_val++; base_val--;']*n)+' /* !FAKE: F6 bounded base-value reference probe. */\n'+site)
  variants.append(('pair'+str(len(variants)),f))
variants.append(('unconddead',plain.replace('        step = base_val - arg0;', '        step = base_val - arg0; /* FAKE: unconditional dead local store probe. */\n        step = base_val - arg0;')))
for name,s2 in variants:
 p=wd/(name+'.c');p.write_text(s2,newline='\n');clean,_=inlineasm.strip_cheat_asm_file(s2);p2=wd/(name+'.build.c');p2.write_text(clean,newline='\n');ov=cheats.empty_overrides(str(wd/'cfg'));ov['src_override']=str(p2);o=str(wd/(name+'.o'));pipeline.build_c_object('code6cac_c2',o,cheat_overrides=ov)
 r=score.score_func(o,'build/src/code6cac_c2.o','func_8003DBE4');print(name,r,flush=True)
 if r['score']==0:print('ZERO',name,flush=True)

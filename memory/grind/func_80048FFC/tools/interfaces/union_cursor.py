from pathlib import Path
import sys,re,json
sys.path.insert(0,'.')
from engine import cheats,pipeline,score,inlineasm
wd=Path('tmp/codex_cam/union_cursor');wd.mkdir(exist_ok=True)
h=Path('tmp/codex_cam/union_interfaces/gpu.h').read_text().replace('extern DmaTag *D_800A378C;','extern u32 *D_800A378C;')
(wd/'gpu.h').write_text(h,newline='\n')
original=Path('tmp/codex_cam/union_interfaces/code6cac_c2.c').read_text()
original=original.replace('DmaTag *pal = &D_800A378C[idx];','u32 *pal = &D_800A378C[idx];').replace('DmaTag *pal = D_800A378C;','u32 *pal = D_800A378C;').replace('pal->tag.word','*pal').replace('pal[0xFFB].tag.word','pal[0xFFB]').replace('DmaTag *ptr = D_800A378C;','u32 *ptr = D_800A378C;').replace('ptr[0x3FFC / 4].tag.word','ptr[0x3FFC / 4]')
for mode in ['index','packet','packet_inc','physical_end','bank_u8','bank_inline']:
 s=original
 if mode in ['packet','packet_inc','physical_end']:
  s=s.replace('DR_MOVE (*colors)[2];','DR_MOVE *packet;\n    DR_MOVE (*colors)[2];').replace('bank = D_800A36AC & 1;','bank = D_800A36AC & 1;\n    packet = &(*colors)[bank];')
  s=s.replace('(*colors)[bank].tag.tag.word','packet->tag.tag.word').replace('(u32)&(*colors)[bank]','(u32)packet')
  s=s.replace('colors++;','colors++;\n                packet = &(*colors)[bank];')
  if mode=='packet_inc':s=s.replace('colors++;\n                packet = &(*colors)[bank];','arg2++;\n                packet = &(*arg2)[bank];')
  if mode=='physical_end':s=s.replace('func_8003DDF8((u32)packet);','func_8003DDF8((u32)colors + bank * sizeof(DR_MOVE));')
 elif mode=='bank_u8':s=s.replace('s32 bank;','u8 bank;')
 elif mode=='bank_inline':s=s.replace('bank = D_800A36AC & 1;','').replace('[bank]','[D_800A36AC & 1]')
 p=wd/(mode+'.c');p.write_text(s,newline='\n');clean,_=inlineasm.strip_cheat_asm_file(s);p2=wd/(mode+'.build.c');p2.write_text(clean,newline='\n');ov=cheats.empty_overrides(str(wd/'cfg'));ov['src_override']=str(p2);o=str(wd/(mode+'.o'));pipeline.build_c_object('code6cac_c2',o,cheat_overrides=ov)
 r=score.score_func(o,'build/src/code6cac_c2.o','func_8003DBE4');print(mode,r,flush=True)
 from engine.cli import _print_insn_diff
 if r['score']<15:_print_insn_diff(score.insn_diff(o,'build/src/code6cac_c2.o','func_8003DBE4'))
for stem in ['text1b','text1a_c','text1a_pre']:
 s=Path('tmp/codex_cam/union_interfaces/'+stem+'.c').read_text().replace('DmaTag *D_800A378C','u32 *D_800A378C').replace('(DmaTag *)','(u32 *)').replace('func_80044504(DmaTag *a0)','func_80044504(u32 *a0)').replace('D_800A378C[0].tag.word','D_800A378C[0]')
 if stem=='text1a_pre':s=s.replace('DmaTag *ot;','OTag *ot;').replace('ot = D_800A378C;', 'ot = (OTag *)D_800A378C;').replace('.tag.fields.addr','.addr')
 p=wd/(stem+'.c');p.write_text(s,newline='\n');clean,_=inlineasm.strip_cheat_asm_file(s);p2=wd/(stem+'.build.c');p2.write_text(clean,newline='\n');ov=cheats.empty_overrides(str(wd/'cfg'));ov['src_override']=str(p2);o=str(wd/(stem+'.o'));pipeline.build_c_object(stem,o,cheat_overrides=ov)
 for fn in {'text1b':['func_80048BA4'],'text1a_c':['func_80044504'],'text1a_pre':['gpu_AddDrawMove']}[stem]:print(fn,score.score_func(o,'build/src/'+stem+'.o',fn),flush=True)

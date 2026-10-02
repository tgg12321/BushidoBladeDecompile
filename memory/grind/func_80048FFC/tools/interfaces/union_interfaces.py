from pathlib import Path
import sys,re,json
sys.path.insert(0,'.')
from engine import cheats,pipeline,score,inlineasm
wd=Path('tmp/codex_cam/union_interfaces');wd.mkdir(exist_ok=True)
header=Path('include/gpu.h').read_text().replace('typedef struct { OTag tag; u32 code[5]; } DR_MOVE;','typedef struct { union { OTag fields; u32 word; } tag; } DmaTag;\ntypedef struct { DmaTag tag; u32 code[5]; } DR_MOVE;').replace('extern OTag *D_800A378C;','extern DmaTag *D_800A378C;')
(wd/'gpu.h').write_text(header,newline='\n')
checks={'gpu':['SetDrawMove'],'text1a_pre':['gpu_AddDrawMove'],'text1a_c':['func_800444BC','func_800444E0','func_80044504'],'code6cac_c2':['func_8003D91C','gpu_SetDrawMoveArray','func_8003DA8C','func_8003DBE4','func_8003DDF8'],'text1b':['func_80046BF4','func_80048BA4','func_80048F58','func_80054F68']}
results=[]
for stem,fs in checks.items():
 s=Path('src/'+stem+'.c').read_text()
 s=s.replace('OTag *D_800A378C','DmaTag *D_800A378C').replace('(OTag *)(old_ptr + 0x10)','(DmaTag *)(old_ptr + 0x10)').replace('(OTag *)(v3 + 0x10)','(DmaTag *)(v3 + 0x10)')
 if stem=='gpu': s=s.replace('a0->tag.len','a0->tag.tag.fields.len')
 if stem=='text1a_c':s=s.replace('void func_80044504(OTag *a0)','void func_80044504(DmaTag *a0)')
 if stem=='text1a_pre':
  s=s.replace('OTag *ot;', 'DmaTag *ot;').replace('OTag *pkt;', 'DR_MOVE *pkt;').replace('pkt = (OTag *)D_800A3378;', 'pkt = D_800A3378;').replace('pkt->addr','pkt->tag.tag.fields.addr')
  s=re.sub(r'(ot\[[^\]]+\])\.addr',r'\1.tag.fields.addr',s)
 if stem=='text1b':
  s=s.replace('*(s32 *)D_800A378C','D_800A378C[0].tag.word')
 if stem=='code6cac_c2':
  s=s.replace('gpu_SetDrawMoveArray(RECT *, s32, DR_MOVE *);','gpu_SetDrawMoveArray(RECT *, s32, DR_MOVE (*)[2]);').replace('gpu_SetDrawMoveArray(RECT *a0, s32 a1, DR_MOVE *a2)','gpu_SetDrawMoveArray(RECT *a0, s32 a1, DR_MOVE (*a2)[2])').replace('DR_MOVE *s1 = a2;', 'DR_MOVE (*s1)[2] = a2;')
  s=s.replace('light_effect_col[0]);','light_effect_col);').replace('D_800A4340[0]);','D_800A4340);').replace('SetDrawMove(s1,','SetDrawMove(&(*s1)[0],').replace('s1[1] = s1[0];','(*s1)[1] = (*s1)[0];').replace('s1 += 2;','s1++;')
  s=s.replace('func_8003DBE4(s32, s32, s32 *, s32, s32)','func_8003DBE4(s32, s32, DR_MOVE (*)[2], s32, s32)').replace('(s32 *)light_effect_col','light_effect_col').replace('(s32 *)D_800A4340','D_800A4340')
  s=s.replace('func_8003DBE4(s32 arg0, s32 arg1, s32 *arg2, s32 arg3, s32 arg4)','func_8003DBE4(s32 arg0, s32 arg1, DR_MOVE (*arg2)[2], s32 arg3, s32 arg4)')
  a,b=inlineasm._func_body_span(s,'func_8003DBE4');f=s[a:b]
  f=f.replace('s32 *colors;', 'DR_MOVE (*colors)[2];\n    s32 bank;').replace('colors = (s32 *)((u8 *)colors + (D_800A36AC & 1) * 24);','bank = D_800A36AC & 1;')
  f=f.replace('s32 *pal = (s32 *)((u32)idx * 4 + (u32)D_800A378C);','DmaTag *pal = &D_800A378C[idx];').replace('*pal','pal->tag.word').replace('*colors','(*colors)[bank].tag.tag.word').replace('(u32)colors','(u32)&(*colors)[bank]')
  f=f.replace('colors = (s32 *)((u8 *)colors + 0x30);','colors++;').replace('s32 pal->tag.word = (s32 *)D_800A378C;','DmaTag *pal = D_800A378C;').replace('*(s32 *)((u8 *)pal + 0x3FEC)','pal[0xFFB].tag.word')
  # Avoid blanket replacements changing pointer declarations.
  f=f.replace('DR_MOVE ((*colors)[bank].tag.tag.word)[2];','DR_MOVE (*colors)[2];').replace('DmaTag pal->tag.word =','DmaTag *pal =')
  s=s[:a]+f+s[b:]
  s=s.replace('u32 *ptr = (u32 *)D_800A378C;', 'DmaTag *ptr = D_800A378C;').replace('ptr[0x3FFC / 4] = arg0;', 'ptr[0x3FFC / 4].tag.word = arg0;')
 p=wd/(stem+'.c');p.write_text(s,newline='\n');clean,_=inlineasm.strip_cheat_asm_file(s);p2=wd/(stem+'.build.c');p2.write_text(clean,newline='\n')
 ov=cheats.empty_overrides(str(wd/'cfg'));ov['src_override']=str(p2);o=str(wd/(stem+'.o'));pipeline.build_c_object(stem,o,cheat_overrides=ov)
 for func in fs:
  r=score.score_func(o,'build/src/'+stem+'.o',func);results.append({'file':stem,'function':func,**r});print(func,r,flush=True)
Path('tmp/codex_cam/union_interfaces/receipts.json').write_text(json.dumps(results,indent=2),newline='\n')

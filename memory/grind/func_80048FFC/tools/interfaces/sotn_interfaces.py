from pathlib import Path
import sys,re,json
sys.path.insert(0,'.')
from engine import cheats,pipeline,score,inlineasm
wd=Path('tmp/codex_cam/sotn_interfaces');wd.mkdir(exist_ok=True)
header=Path('include/gpu.h').read_text().replace('extern OTag *D_800A378C;','/* SDK OT_TYPE: one DMA tag word per table entry. */\nextern u32 *D_800A378C;')
(wd/'gpu.h').write_text(header,newline='\n')
checks={'gpu':['SetDrawMove'],'text1a_pre':['gpu_AddDrawMove'],'text1a_c':['func_800444BC','func_800444E0','func_80044504'],'code6cac_c2':['func_8003D91C','gpu_SetDrawMoveArray','func_8003DA8C','func_8003DBE4','func_8003DDF8'],'text1b':['func_80046BF4','func_80048BA4','func_80048F58','func_80054F68']}
results=[]
for stem,fs in checks.items():
 s=Path('src/'+stem+'.c').read_text()
 s=s.replace('OTag *D_800A378C','u32 *D_800A378C').replace('(OTag *)(old_ptr + 0x10)','(u32 *)(old_ptr + 0x10)').replace('(OTag *)(v3 + 0x10)','(u32 *)(v3 + 0x10)')
 if stem=='text1a_c':s=s.replace('void func_80044504(OTag *a0)','void func_80044504(u32 *a0)')
 if stem=='text1a_pre':s=s.replace('ot = D_800A378C;','''/* FAKE: SDK bitfield view of an OT word retains tag length;
         * PS1 use: src/main/psxsdk/libgpu/sys.c:288; see interface ledger. */
        /* SOTN: include/psxsdk/libgpu.h:88 @db41b28eee52969244a52cc269c8163d1ed8826a */
        ot = (OTag *)D_800A378C;''')
 if stem=='text1b':s=s.replace('*(s32 *)D_800A378C','D_800A378C[0]')
 if stem=='code6cac_c2':
  s=s.replace('gpu_SetDrawMoveArray(RECT *, s32, DR_MOVE *);','gpu_SetDrawMoveArray(RECT *, s32, DR_MOVE (*)[2]);').replace('gpu_SetDrawMoveArray(RECT *a0, s32 a1, DR_MOVE *a2)','gpu_SetDrawMoveArray(RECT *a0, s32 a1, DR_MOVE (*a2)[2])').replace('DR_MOVE *s1 = a2;', 'DR_MOVE (*s1)[2] = a2;')
  s=s.replace('light_effect_col[0]);','light_effect_col);').replace('D_800A4340[0]);','D_800A4340);').replace('SetDrawMove(s1,','SetDrawMove(&(*s1)[0],').replace('s1[1] = s1[0];','(*s1)[1] = (*s1)[0];').replace('s1 += 2;','s1++;')
  s=s.replace('func_8003DBE4(s32, s32, s32 *, s32, s32)','func_8003DBE4(s32, s32, DR_MOVE (*)[2], s32, s32)').replace('(s32 *)light_effect_col','light_effect_col').replace('(s32 *)D_800A4340','D_800A4340')
  s=s.replace('func_8003DBE4(s32 arg0, s32 arg1, s32 *arg2, s32 arg3, s32 arg4)','func_8003DBE4(s32 arg0, s32 arg1, DR_MOVE (*arg2)[2], s32 arg3, s32 arg4)')
  a,b=inlineasm._func_body_span(s,'func_8003DBE4');f=s[a:b]
  f=f.replace('s32 *colors;', 'u32 *colors;').replace('colors = arg2;','''/* FAKE: aggregate word view keeps the original full-word tag operations;
     * direct packet/pair and union views miss (interface ledger). */
    /* SOTN: src/dra/4DA70.c:10 @db41b28eee52969244a52cc269c8163d1ed8826a */
    colors = (u32 *)arg2;''')
  f=f.replace('s32 *pal = (s32 *)((u32)idx * 4 + (u32)D_800A378C);','u32 *pal = (u32 *)((u32)idx * 4 + (u32)D_800A378C);').replace('colors = (s32 *)','colors = (u32 *)').replace('s32 *pal = (s32 *)D_800A378C;','u32 *pal = D_800A378C;').replace('*(s32 *)((u8 *)pal + 0x3FEC)','pal[0xFFB]')
  f=f.replace('(u8 *)colors +','(u32)colors +')
  f=f.replace('colors = (u32 *)((u32)colors + 0x30);','''/* Advance the encoded DMA address. At the end it may identify
                 * the following packet without accessing that packet's storage. */
                colors = (u32 *)((u32)colors + 0x30);''')
  s=s[:a]+f+s[b:]
  s=s.replace('u32 *ptr = (u32 *)D_800A378C;', 'u32 *ptr = D_800A378C;')
 if stem=='gpu':
  s=s.replace('/* FAKE: packed RECT word read follows matched Sony-library precedent.\n     * SOTN:', '/* FAKE: packed RECT word read follows matched Sony-library precedent. */\n    /* SOTN:')
 p=wd/(stem+'.c');p.write_text(s,newline='\n');clean,_=inlineasm.strip_cheat_asm_file(s);p2=wd/(stem+'.build.c');p2.write_text(clean,newline='\n')
 ov=cheats.empty_overrides(str(wd/'cfg'));ov['src_override']=str(p2);o=str(wd/(stem+'.o'));pipeline.build_c_object(stem,o,cheat_overrides=ov)
 for func in fs:
  r=score.score_func(o,'build/src/'+stem+'.o',func);results.append({'file':stem,'function':func,**r});print(func,r,flush=True)
Path('tmp/codex_cam/sotn_interfaces/receipts.json').write_text(json.dumps(results,indent=2),newline='\n')

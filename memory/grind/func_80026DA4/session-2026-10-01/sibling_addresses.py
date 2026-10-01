from pathlib import Path
import sys,re,json,os
root=Path('/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile')
sys.path.insert(0,str(root))
from engine import pipeline,score,inlineasm,cheats
os.chdir(root)
out=Path('tmp/codex-26da4')
base=(root/'src/code6cac_tu2.c').read_text()
span=inlineasm._func_body_span(base,'func_8001CE60')
body=base[span[0]:span[1]]
origasm='.include "macro.inc"\n.set noat\n.set noreorder\n'+(root/'asm/funcs/func_8001CE60.s').read_text()
(out/'sibling-target.s').write_text(origasm)
pipeline.build_asm_object(str(out/'sibling-target.s'),str(out/'sibling-target.o'))
variants={'control':body}
old='g_practice_menu_table[0].unk_6A'
forms={
'int-field':'*(s16 *)(u32)&g_practice_menu_table[0].unk_6A',
'int-byte':'*(s16 *)((u32)g_practice_menu_table + 0x6A)',
'int-base-struct':'((PracticeMenuRec *)(u32)g_practice_menu_table)->unk_6A',
'int-byte-unsigned':'*(u16 *)((u32)g_practice_menu_table + 0x6A)',
'int-signed':'*(s16 *)((s32)g_practice_menu_table + 0x6A)',
'byte-offset':'*(s16 *)((u8 *)g_practice_menu_table + 0x6A)',
'scalar-array':'((s16 *)g_practice_menu_table)[0x35]',
'volatile-field':'*(volatile s16 *)&g_practice_menu_table[0].unk_6A',
}
for name,form in forms.items():
    for site in ('both','first','second'):
        if site=='both':v=body.replace(old,form)
        elif site=='first':v=body.replace(old,form,1)
        else:v=body.replace('id = '+old,'id = '+form,1)
        variants[name+'-'+site]=v
for name,v in variants.items():
    (out/f'sibling-{name}.c').write_text(v)
    src=inlineasm.substitute_body(base,'func_8001CE60',v)
    p=out/'sibling-src.c';p.write_text(src)
    ov=cheats.empty_overrides(str(out/'sibling-cfg'));ov['src_override']=str(p)
    try:
        pipeline.build_c_object('code6cac_tu2',str(out/'sibling-built.o'),ov)
        result=score.score_func(str(out/'sibling-built.o'),str(out/'sibling-target.o'),'func_8001CE60')
        print(name,json.dumps(result),flush=True)
    except Exception as exc:print(name,str(exc)[-600:],flush=True)

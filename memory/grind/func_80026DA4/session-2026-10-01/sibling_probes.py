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
read='            id = g_practice_menu_table[0].unk_6A;'
call='            func_8005C650(0x9F, 0x7F, 0x7F);'
store='            D_800A37E1 = 1;'
for label,statement in [('load',read),('call',call),('store',store)]:
    variants['wrap-'+label]=body.replace(statement, '            /* FAKE: loop-note probe. */\n            do {\n'+statement+'\n            } while (0);',1)
variants['split-store']=body.replace('            D_800A3816 = 0x3C;','            do { D_800A3816 = 0x3C; } while (0);',1)
variants['load-before-store']=body.replace(store+'\n'+read,read+'\n'+store,1)
variants['unsigned-load']=body.replace(read,'            id = (u16)g_practice_menu_table[0].unk_6A;')
variants['signed-temp']=body.replace('            u16 id;','            s16 state;\n            u16 id;',1).replace(read,'            state = g_practice_menu_table[0].unk_6A;\n            id = state;',1)
variants['unsigned-view-first']=body.replace('(u16)g_practice_menu_table[0].unk_6A == 6','*(u16 *)&g_practice_menu_table[0].unk_6A == 6')
variants['unsigned-view-second']=body.replace(read,'            id = *(u16 *)&g_practice_menu_table[0].unk_6A;')
variants['signed-view-second']=body.replace(read,'            id = *(s16 *)&g_practice_menu_table[0].unk_6A;')
variants['unsigned-view-both']=variants['unsigned-view-first'].replace(read,'            id = *(u16 *)&g_practice_menu_table[0].unk_6A;')
variants['pointer-intermediate']=body.replace(read,'            { s16 *mode = &g_practice_menu_table[0].unk_6A; id = *mode; }')
# Same conditional tree in an explicit lexical else block.
begin=body.index('        } else if ((u16)g_practice_menu_table[0].unk_6A == 6')
end=body.index('        if (D_800A36CC != 0) {',begin)
suffix=body[begin:end]
block='        } else {\n'+suffix.replace('        } else if (','            if (',1)+'        }\n'
variants['lexical-block']=body[:begin]+block+body[end:]
condition='(u16)g_practice_menu_table[0].unk_6A == 6 || (u16)g_practice_menu_table[1].unk_6A == 6'
block2=block.replace('            if ('+condition+') {', '            s32 anyReady = '+condition+';\n            if (anyReady) {',1)
variants['boolean-value']=body[:begin]+block2+body[end:]
duplicated='''(u16)g_practice_menu_table[0].unk_6A == 6) {
            D_800A3816 = 0x3C;
        } else if ((u16)g_practice_menu_table[1].unk_6A == 6'''
variants['split-or']=body.replace(condition,duplicated,1)
# Binding the first state code as a real once-written value.
block3=block.replace('            if ('+condition+') {', '            u16 firstState = g_practice_menu_table[0].unk_6A;\n            if (firstState == 6 || (u16)g_practice_menu_table[1].unk_6A == 6) {',1)
variants['state-value']=body[:begin]+block3+body[end:]
variants['wrap-else-chain']=body[:begin]+'        } else {\n            do {\n'+suffix.replace('        } else if (','            if (',1)+'            } while (0);\n        }\n'+body[end:]
variants['wrap-boolean']=body[:begin]+block2.replace('            s32 anyReady = '+condition+';', '            s32 anyReady;\n            do { anyReady = '+condition+'; } while (0);',1)+body[end:]
variants['first-boolean']=body[:begin]+block.replace('            if ('+condition+') {','            s32 firstReady = (u16)g_practice_menu_table[0].unk_6A == 6;\n            if (firstReady || (u16)g_practice_menu_table[1].unk_6A == 6) {',1)+body[end:]
variants['second-boolean']=body[:begin]+block.replace('            if ('+condition+') {','            s32 secondReady = (u16)g_practice_menu_table[1].unk_6A == 6;\n            if ((u16)g_practice_menu_table[0].unk_6A == 6 || secondReady) {',1)+body[end:]
variants['bit-or']=body.replace(condition,'((u16)g_practice_menu_table[0].unk_6A == 6) | ((u16)g_practice_menu_table[1].unk_6A == 6)',1)
for label,statement in [('id-decl','            u16 id;'),('call',call),('store',store),('read',read)]:
    variants['empty-before-'+label]=body.replace(statement, '            /* FAKE: empty single-level loop-note probe. */\n            do { } while (0);\n'+statement,1)
    variants['empty-after-'+label]=body.replace(statement, statement+'\n            /* FAKE: empty single-level loop-note probe. */\n            do { } while (0);',1)
for count in (1,2,3,4,8):
    decls=''.join(f'    s32 unused{i}; /* FAKE: dead scalar probe. */\n' for i in range(count))
    variants[f'dead-scalars-{count}']=body.replace('    u8 buf[4];',decls+'    u8 buf[4];',1)
for depth in (2,3):
    variants[f'nested-call-{depth}']=body.replace(call, '            /* FAKE: nested loop-note probe; single-level measured insufficient. */\n'+'            do {\n'*depth+call+'\n'+'            } while (0);\n'*depth,1)
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

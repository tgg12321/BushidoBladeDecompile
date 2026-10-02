from pathlib import Path
import sys
sys.path.insert(0,'.')
from engine import cheats,pipeline,score
from engine.cli import _print_insn_diff
wd=Path('tmp/codex_cam/type_ablations');wd.mkdir(exist_ok=True)
src=Path('src/code6cac_c2.c').read_text()
for name,s in [('plainrect',src.replace('(u16)a0->y + (u16)a0->h','a0->y + a0->h').replace('(u16)a0->x + (u16)a0->w','a0->x + a0->w'))]:
 p=wd/(name+'.c');p.write_text(s,newline='\n');ov=cheats.empty_overrides(str(wd/'cfg'));ov['src_override']=str(p);o=str(wd/(name+'.o'));pipeline.build_c_object('code6cac_c2',o,cheat_overrides=ov)
 print(name,score.score_func(o,'build/src/code6cac_c2.o','gpu_SetDrawMoveArray'));_print_insn_diff(score.insn_diff(o,'build/src/code6cac_c2.o','gpu_SetDrawMoveArray'))
gpu=Path('src/gpu.c').read_text()
for name,s in [('component_pack',gpu.replace('*(s32 *)&a1->x','((u32)(u16)a1->y << 16) | (u16)a1->x').replace('*(s32 *)&a1->w','((u32)(u16)a1->h << 16) | (u16)a1->w'))]:
 p=wd/(name+'.c');p.write_text(s,newline='\n');ov=cheats.empty_overrides(str(wd/'cfg'));ov['src_override']=str(p);o=str(wd/(name+'.o'));pipeline.build_c_object('gpu',o,cheat_overrides=ov)
 print(name,score.score_func(o,'build/src/gpu.o','SetDrawMove'));_print_insn_diff(score.insn_diff(o,'build/src/gpu.o','SetDrawMove'))

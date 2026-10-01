# land.py : apply func_8002AB08's landing edits to the working tree (run ONLY under the landing lock, on top
# of laneB's committed PracticeMenuRec). Prints a summary; mine.patch is made with git diff afterwards.
import re
def sub1(path, a, b):
    s = open(path, encoding='utf-8').read()
    assert s.count(a) == 1, (path, a[:60])
    open(path, 'w', encoding='utf-8', newline='\n').write(s.replace(a, b))
body = open('tmp/func_8002AB08/typed/final.c', encoding='utf-8').read()
sub1('src/code6cac_b_tu2.c', 'INCLUDE_ASM("asm/funcs", func_8002AB08);\n', body)
sub1('include/code6cac.h', "    u8  unk_8C[0x8E - 0x8C];\n", "    s16 unk_8C;\n")
sub1('include/code6cac.h', "    u8  unk_92[0x96 - 0x92];\n", "    s16 unk_92;\n    u8  unk_94[0x96 - 0x94];\n")
sub1('include/code6cac.h', "    Vec4i32 unk_114;\n    Vec4i32 unk_124;\n", "    Vec4i32 unk_114[2];             /* [limb]: func_8002AB08 indexes by its 0/1 blade flag */\n")
for k, v in (("114.vx", "114[0].vx"), ("114.vy", "114[0].vy"), ("114.vz", "114[0].vz"),
             ("124.vx", "114[1].vx"), ("124.vy", "114[1].vy"), ("124.vz", "114[1].vz")):
    sub1('src/code6cac_tu2.c', f"    p->unk_{k} = 0;\n", f"    p->unk_{v} = 0;\n")
sub1('undefined_syms_auto.txt', "retire with func_80023F08, func_8002AB08, func_80055B60 */", "retire with func_80023F08, func_80055B60 */")
sub1('src/code6cac_b_tu2.c', 'void func_8002A458(u8 *obj, s32 *hit, s32 *deep, s32 quiet) {', 'void func_8002A458(u8 *obj, u32 *hit, u32 *deep, s32 quiet) {')
print("applied")

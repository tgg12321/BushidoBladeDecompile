# hdr.py : tmp/func_8002AB08/typed/inc/include/code6cac.h = the INDEX's include/code6cac.h (laneB's staged
# PracticeMenuRec) + this function's members; prints the edit list
import subprocess, os
h = subprocess.run(['git', 'show', ':include/code6cac.h'], capture_output=True, text=True).stdout
E = [("    u8  unk_8C[0x8E - 0x8C];\n", "    s16 unk_8C;\n"),
     ("    u8  unk_92[0x96 - 0x92];\n", "    s16 unk_92;\n    u8  unk_94[0x96 - 0x94];\n"),
     ("    Vec4i32 unk_114;\n    Vec4i32 unk_124;\n", "    Vec4i32 unk_114[2];\n")]
for a, b in E:
    assert h.count(a) == 1, a
    h = h.replace(a, b)
os.makedirs('tmp/func_8002AB08/typed/inc/include', exist_ok=True)
open('tmp/func_8002AB08/typed/inc/include/code6cac.h', 'w', newline='\n').write(h)
print('ok')

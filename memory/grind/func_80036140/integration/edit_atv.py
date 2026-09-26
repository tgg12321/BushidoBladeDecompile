"""Apply CdlATV aggregate merge to scratch header (argv1) and scratch source (argv2), candidate (argv3)."""
import sys, re
from pathlib import Path
h = Path(sys.argv[1]); t = h.read_text()
for d in ["extern u8 D_800A36B9;", "extern u8 D_800A36BA;", "extern u8 D_800A36BB;",
          "extern u8 g_cd_atv_plus_0x1;", "extern u8 g_cd_atv_plus_0x2;", "extern u8 g_cd_atv_plus_0x3;"]:
    assert t.count(d + "\n") == 1, d
    t = t.replace(d + "\n", "")
t = t.replace("extern s32 D_800A36B4;\n", "extern s32 D_800A36B4;\ntypedef struct {\n    u8 val0, val1, val2, val3;\n} CdlATV;\nextern CdlATV D_800A36B8;\nextern CdlATV g_cd_atv;\n", 1)
open(h, 'w', newline='\n').write(t)
m = {"D_800A36B9": "D_800A36B8.val1", "D_800A36BA": "D_800A36B8.val2", "D_800A36BB": "D_800A36B8.val3",
     "g_cd_atv_plus_0x1": "g_cd_atv.val1", "g_cd_atv_plus_0x2": "g_cd_atv.val2", "g_cd_atv_plus_0x3": "g_cd_atv.val3"}
for f in sys.argv[2:]:
    p = Path(f); s = p.read_text()
    s = s.replace("extern u8 g_cd_atv;\n", "").replace("extern u8 D_800A36B8;\n", "")
    s = s.replace("extern void CdMix(u8 *);", "extern void CdMix(CdlATV *);")
    for k, v in m.items():
        s = re.sub(r"\b" + k + r"\b", v, s)
    s = re.sub(r"\bg_cd_atv = \(u8\)arg0", "g_cd_atv.val0 = (u8)arg0", s)
    s = re.sub(r"\bD_800A36B8 = \(u8\)arg1", "D_800A36B8.val0 = (u8)arg1", s)
    open(p, 'w', newline='\n').write(s)

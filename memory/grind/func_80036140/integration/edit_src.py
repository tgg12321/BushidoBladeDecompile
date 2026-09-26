import sys, re
from pathlib import Path
p = Path(sys.argv[1])
t = p.read_text()
m = {"D_80101E78": "D_80101E60.unk18", "D_80101E7C": "D_80101E60.unk1C",
     "g_cdread_sectors_remaining": "D_80101E60.sectors_remaining", "g_cdread_dest_buffer": "D_80101E60.dest_buffer",
     "D_80101E88": "D_80101E60.unk28", "D_80101E8C": "D_80101E60.unk2C", "D_80101E90": "D_80101E60.unk30",
     "D_80101E94": "D_80101E60.unk34", "D_80101E98": "D_80101E60.unk38", "D_80101E9A": "D_80101E60.unk3A",
     "D_80101E9C": "D_80101E60.unk3C", "D_80101E9E": "D_80101E60.unk3E",
     "g_cdread_expected_pos": "D_80101E60.expected_pos", "D_80101EA4": "D_80101E60.unk44"}
for k, v in m.items():
    t = re.sub(r"\b" + k + r"\b", v, t)
open(p, 'w', newline='
').write(t)

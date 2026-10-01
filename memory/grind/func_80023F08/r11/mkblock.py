import sys
src, dst = sys.argv[1:3]
s = open(src, encoding='utf-8').read()
for v in ('lim', 'gap', 'turn', 'side'):
    s = s.replace(f"    s32 {v};\n", "", 1)
s = s.replace("                lim = (rec->unk_58[2] >> 4) * 0x88;", "                s32 lim = (rec->unk_58[2] >> 4) * 0x88;\n", 1)
s = s.replace("        s32 tgt[3];\n", "        s32 tgt[3];\n        s32 gap;\n", 1)
s = s.replace("    turn = (rec->unk_14C * rec->unk_44) / 24576;", "    {\n        s32 turn = (rec->unk_14C * rec->unk_44) / 24576;\n", 1)
i = s.index("        s32 turn = (rec->unk_14C * rec->unk_44) / 24576;\n")
j = s.index("    rec->unk_14C -= turn;\n", i) + len("    rec->unk_14C -= turn;\n")
s = s[:i] + s[i:j].replace("\n    rec->", "\n        rec->") + "    }\n" + s[j:]
s = s.replace("        side = (rec->unk_24.held & 0x1000) ? 1", "        s32 side = (rec->unk_24.held & 0x1000) ? 1", 1)
open(dst, 'w', encoding='utf-8', newline='\n').write(s)

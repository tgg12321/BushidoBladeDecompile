"""Append case 1 cross-jump unmerge + tail register fix rules to regfix.txt."""
import pathlib

rules = r"""
# coli_HitPauseKatana_2 case 1: un-cross-jump. Target inlines (srl,lhu,andi,or)
# before addiu, plus `sh $3,0($4)` between sra and lw. 5 extra insns in TRUE body.
# File order below is REVERSE of desired final order for inserts at same idx.
coli_HitPauseKatana_2: insert "or $3,$3,$2" @ 46
coli_HitPauseKatana_2: insert "andi $2,$2,0x00ff" @ 46
coli_HitPauseKatana_2: insert "lhu $3,0($4)" @ 46
coli_HitPauseKatana_2: insert "srl $2,$8,16" @ 46
coli_HitPauseKatana_2: insert "sh $3,0($4)" @ 48
# TRUE branch j delay slot: change srl to lui (becomes mask-prep for tail join)
coli_HitPauseKatana_2: subst "srl\s+\$2,\$8,16" "lui $2,(16711680 >> 16) & 0xFFFF" @ 53
# Case 1 tail: fix result register from $8 (t0) to $2 (v0)
coli_HitPauseKatana_2: subst "and\s+\$8,\$8,\$2" "and $2,$8,$2" @ 68
coli_HitPauseKatana_2: subst "or\s+\$10,\$10,\$8" "or $10,$10,$2" @ 70
"""

path = pathlib.Path("regfix.txt")
content = path.read_text()
if "coli_HitPauseKatana_2: insert " in content:
    print("Rules already present, skipping")
else:
    path.write_text(content + rules)
    print("Rules appended")

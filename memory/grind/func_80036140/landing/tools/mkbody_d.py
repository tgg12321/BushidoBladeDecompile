"""body_d.c = the Match body (commit D): CdState record members; g_cd_result as today's splat byte handles."""
from pathlib import Path
H = Path('tmp/func_80036140')
t = (H / 'body_tpl.c').read_text()
for k, v in {'@E9C_POSTINC@': 'D_80101E58.rec.unk3C++', '@E9C@': 'D_80101E58.rec.unk3C',
             '@EA4_SUB4@': 'D_80101E58.rec.unk44 -= 4', '@EA4@': 'D_80101E58.rec.unk44'}.items():
    t = t.replace(k, v)
assert '@' not in t
(H / 'body_d.c').write_bytes(t.encode())
print('ok')

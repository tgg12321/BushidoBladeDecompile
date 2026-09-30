"""dz spellings (no parameter staging) on the ab_noflag_noax base -> tmp/D780/dz_*.c"""
from pathlib import Path

base = Path("tmp/D780/ab_noflag_noax.c").read_bytes().decode()
BLK = """                s32 ax = cx - x0;
                s32 dz;
                s32 dx;
                s32 az;
                s32 bz;
                dz = z2 - z0;
                dx = x2 - x0;
                az = cz - z0;
                kc = (dz * ax) - (dx * az);
                bz = pz - z0;
                kp = (dz * (px - x0)) - (dx * bz);
"""
assert BLK in base
V = {
 "inline": """                s32 ax = cx - x0;
                s32 dx = x2 - x0;
                s32 az = cz - z0;
                kc = ((z2 - z0) * ax) - (dx * az);
                kp = ((z2 - z0) * (px - x0)) - (dx * (pz - z0));
""",
 "init": """                s32 dz = z2 - z0;
                s32 ax = cx - x0;
                s32 dx = x2 - x0;
                s32 az = cz - z0;
                kc = (dz * ax) - (dx * az);
                kp = (dz * (px - x0)) - (dx * (pz - z0));
""",
 "dxfirst": """                s32 ax = cx - x0;
                s32 dx;
                s32 dz;
                s32 az;
                dx = x2 - x0;
                dz = z2 - z0;
                az = cz - z0;
                kc = (dz * ax) - (dx * az);
                kp = (dz * (px - x0)) - (dx * (pz - z0));
""",
 "e2": """                s32 ex = x2 - x0;
                s32 ez = z2 - z0;
                kc = ez * (cx - x0) - ex * (cz - z0);
                kp = ez * (px - x0) - ex * (pz - z0);
""",
}
for k, blk in V.items():
    Path(f"tmp/D780/dz_{k}.c").write_bytes(base.replace(BLK, blk).encode())
# outer-scope dz: declared with the other edge values
t = base.replace(BLK, BLK.replace("                s32 dz;\n", "").replace("                dz = z2 - z0;\n", ""))
t = t.replace("        s32 kp = z0 * px - x0 * pz;\n", "        s32 kp = z0 * px - x0 * pz;\n        s32 dz = z2 - z0;\n", 1)
Path("tmp/D780/dz_outer.c").write_bytes(t.encode())
print("ok")

"""Prepare the manifest sweep: copy every banked func_800187F4 body (plus the v8 reviewer probes and the
w1/w2 ablation chassis) to tmp/func_800187F4/mf_<n>.c; write tmp/f187/sweep_tags.tsv (tag, path)."""
import pathlib, shutil

L = pathlib.Path("memory/grind/func_800187F4")
D = pathlib.Path("tmp/func_800187F4")
paths = [p for p in sorted(L.rglob("*.c"))
         if "func_800187F4(" in p.read_bytes().decode("utf-8", "replace")]
extra = sorted(D.glob("rv8_*.c")) + [D / "w1.c", D / "w2.c"]
rows = []
for n, p in enumerate(paths + extra):
    tag = f"mf_{n:03d}"
    shutil.copy(p, D / f"{tag}.c")
    rel = str(p.relative_to(L)) if str(p).startswith(str(L)) else "tmp:" + p.name
    rows.append(f"{tag}\t{rel}")
open("tmp/f187/sweep_tags.tsv", "w", newline="\n").write("\n".join(rows) + "\n")
print(len(rows))

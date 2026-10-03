"""owners4e.py: every src/**/*.c file (and asm/data/*.s) naming each array of the two LIBCD/LIBETC data files."""
import re, glob
syms = []
for f in ("src/text1a_b_post_rodata.c", "src/text1a_b_tail_rodata.c"):
    syms += re.findall(r"^const char (\w+)\[(\d+)\]", open(f, encoding="utf-8").read(), re.M)
files = [f for f in glob.glob("src/**/*.c", recursive=True) if "_rodata" not in f]
texts = {f: open(f, encoding="utf-8", errors="replace").read() for f in files}
data = {f: open(f, encoding="utf-8", errors="replace").read() for f in glob.glob("asm/data/*.s")}
inc = {f: open(f, encoding="utf-8", errors="replace").read() for f in glob.glob("include/**/*.h", recursive=True)}
for s, n in syms:
    rx = re.compile(r"\b" + s + r"\b")
    users = [f.replace("\\", "/")[4:-2] for f, t in texts.items() if rx.search(t)]
    d = [f for f, t in data.items() if rx.search(t)]
    h = [f for f, t in inc.items() if rx.search(t)]
    print(f"{s:14s} {n:>4s} src={users} data={d} include={h}")

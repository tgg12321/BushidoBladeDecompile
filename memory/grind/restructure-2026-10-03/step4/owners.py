"""owners.py (run from the repo root): every src/**/*.c and asm/**/*.s file that names each array defined in
src/text1a_b_post_rodata.c (the restructure 4d sole-referrer check; asm/funcs/*.s of C-matched functions are
unlinked reference copies)."""
import re, glob, os
syms = re.findall(r"^const char (\w+)\[(\d+)\]", open("src/text1a_b_post_rodata.c").read(), re.M)
files = [f for f in glob.glob("src/**/*.c", recursive=True) if "post_rodata" not in f]
texts = {f: open(f, encoding="utf-8", errors="replace").read() for f in files}
asm = {f: open(f, encoding="utf-8", errors="replace").read() for f in glob.glob("asm/**/*.s", recursive=True)}
other = {f: open(f, encoding="utf-8", errors="replace").read() for f in ["undefined_syms_auto.txt", "symbol_addrs.txt", "bb2.ld"] if os.path.exists(f)}
for s, n in syms:
    rx = re.compile(r"\b" + s + r"\b")
    users = [f.replace("\\", "/")[4:-2] for f, t in texts.items() if rx.search(t)]
    a = [f for f, t in asm.items() if rx.search(t)]
    o = [f for f, t in other.items() if rx.search(t)]
    print(f"{s:20s} {n:>4s}  src={users} asm={a} other={o}")

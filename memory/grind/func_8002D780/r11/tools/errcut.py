"""Cut the func_8002D780 pass=1 SCHEDDBG trace from <dir>/sched.err to <dir>/sched1.D780.txt;
print ADJPRI / PICK lines for the given insn uids and the block headers."""
import sys

d = sys.argv[1]
uids = set(sys.argv[2:])
lines = open(f"{d}/sched.err", encoding="utf-8", errors="replace").read().split("\n")
out, on = [], False
for l in lines:
    if l.startswith("SCHEDDBG FUNC "):
        on = "func=func_8002D780 pass=1" in l
    if on:
        out.append(l)
open(f"{d}/sched1.D780.txt", "w", encoding="utf-8", newline="\n").write("\n".join(out))
blk = None
for l in out:
    if l.startswith("SCHEDDBG block="):
        blk = l
    if "ADJPRI" in l and any(f"insn={u} " in l for u in uids):
        print(blk, "|", l)
    if "PICK" in l and any(f"picked={u} " in l for u in uids):
        print(blk, "|", l)

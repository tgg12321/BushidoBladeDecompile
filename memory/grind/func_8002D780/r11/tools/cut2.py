"""Cut func_8002D780's section from every dump in <dir> (incl. .sched and sched.err) into <file>.D780."""
import sys

d = sys.argv[1]
for f in ["tu.i.flow", "tu.i.combine", "tu.i.lreg", "tu.i.greg", "tu.i.sched"]:
    t = open(f"{d}/{f}", encoding="utf-8").read()
    i = t.index(";; Function func_8002D780")
    j = t.find(";; Function", i + 10)
    open(f"{d}/{f}.D780", "w", encoding="utf-8", newline="\n").write(t[i:j if j >= 0 else len(t)])
# sched.err: the trace is emitted per function in compile order; cut between the markers of
# the functions before/after by locating the first ADJPRI insn uids present in the .sched section
err = open(f"{d}/sched.err", encoding="utf-8", errors="replace").read().split("\n")
print(len(err), "stderr lines")

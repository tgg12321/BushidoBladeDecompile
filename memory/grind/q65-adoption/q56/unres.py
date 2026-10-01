import re,os,subprocess
T="/tmp/q56/tree"
addr=set(l.split()[2] for l in subprocess.run(f"mipsel-linux-gnu-nm {T}/build/bb2.elf",shell=True,capture_output=True,text=True).stdout.splitlines() if len(l.split())==3)
REL=re.compile(r"%(gp_rel|hi|lo)\(([A-Za-z_.$][\w.$]*)")
u={}
for fn in os.listdir(T+"/asm/funcs"):
    for m in REL.finditer(open(T+"/asm/funcs/"+fn,errors="replace").read()):
        if m.group(2) not in addr: u.setdefault(m.group(2),set()).add(m.group(1))
gp=[k for k,v in u.items() if "gp_rel" in v]
print(len(u), "gp_rel unresolved:", gp[:20]); print(sorted(u)[:30])

#!/usr/bin/env python3
"""Splice a candidate, build via sandbox, and report the callee-saved register->value
mapping in the FINAL asm vs target, plus a rotation-match count. This isolates the
register rotation (independent of scheduling/masked score).
Usage: python3 tmp/regmatch.py <candidate.c>
Target: s0=status s1=saved s2=i1494 s3=i1496 s4=arg1 s5=tbl s6=i1495 s7=arg0"""
import re, subprocess, sys
from pathlib import Path
SRC=Path("src/system.c"); BAK=Path("tmp/system.c.orig")
OUR="tmp/sandbox/marionation_Exec/system.o"
TARGET={"s0":"status","s1":"saved","s2":"i1494","s3":"i1496","s4":"arg1","s5":"tbl","s6":"i1495","s7":"arg0"}
def extract_fn(text):
    m=re.search(r"(^|\n)(s32\s+marionation_Exec\s*\([^;{]*\))\s*\{",text)
    start=m.start(2); i=text.index("{",m.end(2)-1); d=0
    while i<len(text):
        if text[i]=="{":d+=1
        elif text[i]=="}":
            d-=1
            if d==0:return start,i+1
        i+=1
def regmap(objpath):
    r=subprocess.run(["mipsel-linux-gnu-objdump","-d",objpath],capture_output=True,text=True)
    ins=[]; on=False
    for l in r.stdout.splitlines():
        m=re.match(r"^[0-9a-f]+ <(.+)>:$",l)
        if m: on=(m.group(1)=="marionation_Exec"); continue
        if on and re.match(r"^\s+[0-9a-f]+:",l):
            p=l.split("\t"); t="\t".join(p[2:]).strip() if len(p)>=3 else p[-1].strip()
            ins.append(re.sub(r"\s+"," ",t))
    mp={}
    body="\n".join(ins)
    # i1494 base: sX in "addiu sY,sX,1" (i1495) — sX is i1494, sY is i1495
    m=re.search(r"addiu (s\d),(s\d),1\b",body)
    if m: mp["i1495"]=m.group(1); mp["i1494"]=m.group(2)
    m=re.search(r"addiu (s\d),(s\d),2\b",body)
    if m: mp["i1496"]=m.group(1)
    # tbl: 'addu vX,vX,sY' in printf (base added to shifted idx)
    m=re.search(r"addu \w+,\w+,(s\d)",body)
    if m: mp["tbl"]=m.group(1)
    # arg0/arg1 prologue moves
    m=re.search(r"move (s\d),a0\b",body)
    if m: mp["arg0"]=m.group(1)
    m=re.search(r"move (s\d),a1\b",body)
    if m: mp["arg1"]=m.group(1)
    # status: move sX,v0 after a jal (poll result) — heuristic: 'move sX,v0'
    m=re.search(r"move (s\d),v0\b",body)
    if m: mp["status"]=m.group(1)
    # saved: andi sX,..,0x3
    m=re.search(r"andi (s\d),\w+,0x3\b",body)
    if m: mp["saved"]=m.group(1)
    return {v:k for k,v in mp.items()}  # reg->value
def sandbox():
    r=subprocess.run(["bash","-c","python3 -m engine.cli sandbox marionation_Exec --disable all 2>&1"],capture_output=True,text=True)
    for l in r.stdout.splitlines():
        if '"score"' in l: return l.split(":")[1].strip().rstrip(",")
    return "?"
base=BAK.read_text(); cand=Path(sys.argv[1]).read_text()
s,e=extract_fn(base); cs,ce=extract_fn(cand); SRC.write_text(base[:s]+cand[cs:ce]+base[e:])
score=sandbox()
ours=regmap(OUR)
match=0
print(f"== {sys.argv[1]}: masked={score}")
print(f"  reg  target      ours        ok")
for reg in ["s0","s1","s2","s3","s4","s5","s6","s7"]:
    tv=TARGET[reg]; ov=ours.get(reg,"?")
    ok = (tv==ov)
    if ok: match+=1
    print(f"  {reg}   {tv:10s}  {ov:10s}  {'YES' if ok else 'no'}")
print(f"  ROTATION MATCH: {match}/8 callee-saved regs correct")

#!/usr/bin/env python3
"""Splice a candidate, sandbox it, and show an LCS-ALIGNED diff of marionation's
objdump (ours vs target) so insertions/deletions are visible, not just misalignment.
Usage: python3 tmp/adiff.py <candidate.c>"""
import re, subprocess, sys, difflib
from pathlib import Path
SRC=Path("src/system.c"); BAK=Path("tmp/system.c.orig")
TGT="build/src/system.o"; OUR="tmp/sandbox/marionation_Exec/system.o"
def extract_fn(text):
    m=re.search(r"(^|\n)(s32\s+marionation_Exec\s*\([^;{]*\))\s*\{",text)
    start=m.start(2); i=text.index("{",m.end(2)-1); d=0
    while i<len(text):
        if text[i]=="{":d+=1
        elif text[i]=="}":
            d-=1
            if d==0:return start,i+1
        i+=1
base=BAK.read_text(); cand=Path(sys.argv[1]).read_text()
s,e=extract_fn(base); cs,ce=extract_fn(cand); SRC.write_text(base[:s]+cand[cs:ce]+base[e:])
r=subprocess.run(["bash","-c","python3 -m engine.cli sandbox marionation_Exec --disable all 2>&1"],capture_output=True,text=True)
score="?"
for l in r.stdout.splitlines():
    if '"score"' in l: score=l.split(":")[1].strip().rstrip(",")
def dump(obj):
    rr=subprocess.run(["mipsel-linux-gnu-objdump","-d",obj],capture_output=True,text=True)
    out,on=[],False
    for l in rr.stdout.splitlines():
        m=re.match(r"^[0-9a-f]+ <(.+)>:$",l)
        if m: on=(m.group(1)=="marionation_Exec"); continue
        if on and re.match(r"^\s+[0-9a-f]+:",l):
            p=l.split("\t"); ins="\t".join(p[2:]).strip() if len(p)>=3 else p[-1].strip()
            ins=re.sub(r"\s+"," ",ins); ins=re.sub(r"0x[0-9a-f]+","",ins); ins=re.sub(r" <[^>]+>","",ins)
            # normalize branch/jump target addresses (differ only by function base address)
            ins=re.sub(r"(\b(?:b\w+|j)\b[^,]*,)[0-9a-f]+$", r"\1TGT", ins)
            ins=re.sub(r"^(j|jal)\s+[0-9a-f]+$", r"\1 TGT", ins)
            out.append(ins)
    return out
our,tgt=dump(OUR),dump(TGT)
print(f"masked={score} ours={len(our)} target={len(tgt)}")
sm=difflib.SequenceMatcher(a=our,b=tgt,autojunk=False)
oi=ti=0
for tag,i1,i2,j1,j2 in sm.get_opcodes():
    if tag=="equal":
        continue
    if tag=="replace":
        n=max(i2-i1,j2-j1)
        for k in range(n):
            o=our[i1+k] if i1+k<i2 else ""
            t=tgt[j1+k] if j1+k<j2 else ""
            print(f"  ~ ours[{i1+k:3}] {o:30.30s} | tgt[{j1+k:3}] {t}")
    elif tag=="delete":
        for k in range(i1,i2): print(f"  - ours[{k:3}] {our[k]:30.30s} | (missing in target)")
    elif tag=="insert":
        for k in range(j1,j2): print(f"  + (ours missing)                  | tgt[{k:3}] {tgt[k]}")

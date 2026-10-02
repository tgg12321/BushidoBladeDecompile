from pathlib import Path
import re
for name in ['plain','pair4']:
 d=Path('tmp/codex_cam/plain_consumer/dump_'+name)
 for suf in ['cse','lreg','greg','sched']:
  s=(d/('t.i.'+suf)).read_text()
  m=re.search(r'^;; Function func_8003DBE4.*?(?=^;; Function |\Z)',s,re.M|re.S)
  (d/(suf+'.txt')).write_text(m.group() if m else 'MISSING',newline='\n')
 lines=(d/'err.txt').read_text().splitlines();buf=[];found=False
 for l in lines:
  if l.startswith('ALLOCDBG func='):
   if 'func=func_8003DBE4' in l:found=True;break
   buf=[]
  elif l.startswith('QTYDBG'):buf.append(l)
 (d/'qty.txt').write_text('\n'.join(buf)+'\n',newline='\n')
 print(name,'found',found,'qty',len(buf))

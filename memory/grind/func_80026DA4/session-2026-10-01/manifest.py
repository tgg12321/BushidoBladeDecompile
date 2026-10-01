from pathlib import Path
import os,re,json,sys,subprocess
root=Path('/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile');os.chdir(root);sys.path.insert(0,str(root))
from engine import inlineasm,layer2
out=Path('tmp/codex-26da4');items=[]
files=['src/code6cac.c','src/code6cac_b.c','src/code6cac_b_tu2.c','src/code6cac_c_ab.c','src/code6cac_tu2.c','src/text1b.c']
for file in files:
    text=Path(file).read_text();old=subprocess.check_output(['git','show','ecd42b52:'+file],text=True)
    names=set(re.findall(r'^\w[\w *]+\s+(\w+)\([^;\n]*\)\s*\{',text,re.M))
    for name in sorted(names):
        span=inlineasm._func_body_span(text,name)
        if not span:continue
        h=layer2._key(text[span[0]:span[1]])
        os_=inlineasm._func_body_span(old,name)
        oh=layer2._key(old[os_[0]:os_[1]]) if os_ else ''
        if h!=oh:items.append({'func':name,'file':file,'body_hash':h,'previous_body_hash':oh})
(out/'review-manifest.json').write_text(json.dumps(items,indent=2)+'\n')
print(json.dumps(items,indent=2))

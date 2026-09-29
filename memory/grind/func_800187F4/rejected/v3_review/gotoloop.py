import re, sys
D='tmp/func_800187F4/'
def find_loops(s):
    return [m for m in re.finditer(r'^( *)for \((\w+) = 0; (\w+) < ([^;]+); \2\+\+\) \{\n', s, re.M)]
def block_end(s, start):
    # start = index just after the opening '{\n'; returns index of the matching '}' line start
    depth=1; i=start
    while True:
        o=s.find('{',i); c=s.find('}',i)
        if o!=-1 and o<c: depth+=1; i=o+1
        else:
            depth-=1
            if depth==0: return c
            i=c+1
def to_goto(s, k, form='guard', lab=None):
    m=find_loops(s)[k]
    ind, v, v2, lim = m.group(1), m.group(2), m.group(3), m.group(4)
    bstart=m.end(); bend=block_end(s,bstart)
    body=s[bstart:bend]
    # body ends with indentation of closing brace
    body=body[:len(body)-len(ind)] if body.endswith(ind) else body
    lab=lab or f'L{k}'
    after=s[bend+1:]
    has_cont='continue;' in body
    if has_cont:
        body=body.replace('continue;', f'goto {lab}_next;')
    if form=='guard':
        new=(f'{ind}{v} = 0;\n{ind}if ({v} < {lim}) {{\n{ind}{lab}:\n{ind}    {{\n'
             + re.sub(r'^', '    ', body, flags=re.M).rstrip(' ') 
             + f'{ind}    }}\n' + (f'{ind}{lab}_next:\n' if has_cont else '') + f'{ind}    {v}++;\n{ind}    if ({v} < {lim}) goto {lab};\n{ind}}}')
    elif form=='jtest':
        new=(f'{ind}{v} = 0;\n{ind}goto {lab}_test;\n{ind}{lab}:\n{ind}{{\n' + body + f'{ind}}}\n'
             + (f'{ind}{lab}_next:\n' if has_cont else '') + f'{ind}{v}++;\n{ind}{lab}_test:\n{ind}if ({v} < {lim}) goto {lab};')
    elif form=='dowhile':
        new=(f'{ind}{v} = 0;\n{ind}if ({v} < {lim}) {{\n{ind}    do {{\n'
             + re.sub(r'^', '    ', body, flags=re.M).rstrip(' ')
             + f'{ind}    }} while (++{v} < {lim});\n{ind}}}')
        assert not has_cont
    elif form=='while':
        new=(f'{ind}{v} = 0;\n{ind}while ({v} < {lim}) {{\n' + body + f'{ind}    {v}++;\n{ind}}}')
        assert not has_cont
    return s[:m.start()]+new+after
if __name__=='__main__':
    src, out = sys.argv[1], sys.argv[2]
    s=open(D+src+'.c').read()
    for spec in sys.argv[3:]:
        k, form = spec.split(':')
        # loops index: 0=node? node loop is 'for (i = 0; i < count; i++, node += 16)' not matched
        s=to_goto(s, int(k), form)
    open(D+out+'.c','w',newline='\n').write(s)

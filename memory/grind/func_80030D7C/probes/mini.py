"""mini.py IN OUT KEEPFUNC -- strip every function body except KEEPFUNC from a preprocessed
TU (bodies become prototypes) and drop top-level __asm__ statements, so pycparser can
parse it. Braces inside string/char literals are skipped."""
import re
import sys

src = open(sys.argv[1]).read()
keep = sys.argv[3]
out = []
i = 0
n = len(src)
depth = 0


def skip_literal(j):
    q = src[j]
    j += 1
    while j < n and src[j] != q:
        if src[j] == '\\':
            j += 1
        j += 1
    return j + 1


while i < n:
    c = src[i]
    if depth == 0 and src.startswith('__asm__', i) and (i == 0 or src[i - 1] == '\n'):
        # drop a top-level asm statement up to its terminating ');'
        j = i
        par = 0
        while j < n:
            if src[j] in '"\'':
                j = skip_literal(j)
                continue
            if src[j] == '(':
                par += 1
            elif src[j] == ')':
                par -= 1
                if par == 0:
                    break
            j += 1
        j += 1
        while j < n and src[j] in ' \t':
            j += 1
        if j < n and src[j] == ';':
            j += 1
        i = j
        continue
    if c in '"\'':
        j = skip_literal(i)
        out.append(src[i:j])
        i = j
        continue
    if c == '{':
        if depth == 0:
            prev = ''.join(out).rstrip()
            if prev.endswith(')'):
                # function definition: find its name
                head = prev[prev.rfind(';') + 1:] if ';' in prev else prev
                m = re.findall(r'(\w+)\s*\(', head)
                name = m[0] if m else ''
                if name != keep:
                    # skip body
                    d = 0
                    j = i
                    while j < n:
                        if src[j] in '"\'':
                            j = skip_literal(j)
                            continue
                        if src[j] == '{':
                            d += 1
                        elif src[j] == '}':
                            d -= 1
                            if d == 0:
                                break
                        j += 1
                    out.append(';\n')
                    i = j + 1
                    continue
        depth += 1
    elif c == '}':
        depth -= 1
    out.append(c)
    i += 1

open(sys.argv[2], 'w').write(''.join(out))

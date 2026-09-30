import sys, json
sys.path.insert(0, '.')
from engine import completion
NL = chr(10)
t = open('src/text1b_tu1c.c', encoding='utf-8').read()
h = completion.region_hashes(t, 'func_80065800')
p = 'tools/canonical_asm_regions.json'
s = open(p, 'rb').read().decode('utf-8')
old = '      ]' + NL + '    }' + NL + '  }' + NL + '}' + NL
assert s.endswith(old)
ent = '      ]' + NL + '    },' + NL + '    "func_80065800": {' + NL + '      "file": "text1b_tu1c",' + NL + '      "sha256": [' + NL
ent += (',' + NL).join('        "%s"' % x for x in h) + NL + '      ]' + NL + '    }' + NL + '  }' + NL + '}' + NL
s = s[:-len(old)] + ent
json.loads(s)
open(p, 'wb').write(s.encode('utf-8'))
print('ok', len(h))

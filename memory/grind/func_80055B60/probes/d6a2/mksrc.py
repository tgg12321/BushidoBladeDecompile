# mksrc.py <candidate.c> [out]: src/text1b.c with func_80055B60's INCLUDE_ASM replaced by the candidate and
# func_80055138's pad-word stores respelled onto PracticeMenuRec.unk_3D0 (PadState). Run from the repo root.
import sys

def sub1(s, old, new):
    assert s.count(old) == 1, (old[:70], s.count(old))
    return s.replace(old, new)

def transform(s, cand):
    s = sub1(s, 'INCLUDE_ASM("asm/funcs", func_80055B60);\n', cand)
    s = sub1(s, "    p->unk_3E0 = 0;\n    p->unk_3DC = 0;\n    p->unk_3D8 = 0;\n",
             "    p->unk_3D0.released = 0;\n    p->unk_3D0.pressed = 0;\n    p->unk_3D0.held = 0;\n")
    s = sub1(s, "    p->unk_3E4 = -1;\n", "    p->unk_3D0.unheld = -1;\n")
    return s

if __name__ == '__main__':
    cand = open(sys.argv[1], encoding='utf-8').read()
    out = sys.argv[2] if len(sys.argv) > 2 else 'tmp/func_80055B60/text1b.c'
    s = open('src/text1b.c', encoding='utf-8', newline='').read()
    open(out, 'w', encoding='utf-8', newline='\n').write(transform(s, cand))
    print('src ok ->', out)

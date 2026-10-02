"""genz4.py <P_all_Zsplit.c> <outdir>: Ruling 4 split-assignment spellings of the per-block rot_z locals."""
import sys
from pathlib import Path
base = open(sys.argv[1]).read()
out = Path(sys.argv[2]); out.mkdir(exist_ok=True)
forms = {
    'a': "{v} = z * c;\n{I}{v} = ({v} - x * sn) >> 12;\n",
    'b': "{v} = z * c - x * sn;\n{I}{v} >>= 12;\n",
    'c': "{v} = z * c;\n{I}{v} -= x * sn;\n{I}{v} >>= 12;\n",
    'e': "{v} = -(x * sn);\n{I}{v} = (z * c + {v}) >> 12;\n",
}
for fk, f in forms.items():
    for which in ('both', 'loop', 'first'):
        s = base
        for k, ind in (('0', '        '), ('1', '                ')):
            if (which == 'loop' and k == '0') or (which == 'first' and k == '1'):
                continue
            v = 'rot_z' + k
            old = f"{ind}{v} = (z * c - x * sn) >> 12;\n"
            assert s.count(old) == 1
            s = s.replace(old, ind + f.format(v=v, I=ind))
        open(out / f'Z4{fk}_{which}.c', 'w', newline='\n').write(s)

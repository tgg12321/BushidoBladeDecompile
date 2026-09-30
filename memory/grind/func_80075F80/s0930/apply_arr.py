import sys, os
# apply_arr.py <tree>: D_8009BCE4 as u8[20] in text1b_tu2.c (all extern lines) + its consumers indexed as an array
T = sys.argv[1]
p = os.path.join(T, 'src/text1b_tu2.c')
s = open(p, newline='').read()
assert s.count('extern u8 D_8009BCE4;\n') == 3
s = s.replace('extern u8 D_8009BCE4;\n', 'extern u8 D_8009BCE4[20];\n')
s = s.replace('(&D_8009BCE4)[', 'D_8009BCE4[')
assert '(&D_8009BCE4)' not in s
open(p, 'w', newline='\n').write(s)
print('array model applied')

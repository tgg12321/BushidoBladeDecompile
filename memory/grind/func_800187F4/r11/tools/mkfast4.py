"""fast4.sh = fast3.sh with the prototype patched to match the body's signature in either direction."""
s = open("tmp/func_800187F4/fast3.sh").read()
old = "src=src.replace('void func_800187F4(s32 arg0, s32 *arg1);','void func_800187F4(s16 *arg0, s32 *arg1);') if 'func_800187F4(s16 *arg0' in cand else src\n"
assert s.count(old) == 1
new = ("S16='void func_800187F4(s16 *arg0, s32 *arg1);'; S32='void func_800187F4(s32 arg0, s32 *arg1);'\n"
       "src=src.replace(S32,S16) if 'func_800187F4(s16 *arg0' in cand else src.replace(S16,S32)\n")
s = s.replace(old, new)
open("tmp/func_800187F4/fast4.sh", "w", newline="\n").write(s)
print("ok")

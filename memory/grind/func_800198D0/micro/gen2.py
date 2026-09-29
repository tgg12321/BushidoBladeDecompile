import subprocess, re, os
HDR = "typedef unsigned int u32;\ntypedef int s32;\ntypedef unsigned short u16;\n"
FUNC = """
void f(u32 *ptr, u16 *work, s32 bits, u32 cur, s32 *res) {
    s32 i;
    for (i = 0; i < 63; i++) {
        if (bits < 12) {
            %s
        } else {
            work[i + 3] = cur >> 20; cur <<= 12; bits -= 12;
        }
    }
    res[0] = (s32)ptr; res[1] = bits; res[2] = cur;
}
"""
V = {
 "left_end": "u32 hi = cur >> (32 - bits); s32 need = 12 - bits; s32 left = 32 - need; cur = *ptr++; work[i+3] = (hi << need) | (cur >> left); cur <<= need; bits = left;",
 "left_end_b": "u32 hi = cur >> (32 - bits); s32 need = 12 - bits; s32 left = 32 - need; cur = *ptr++; work[i+3] = (hi << need) | (cur >> left); bits = left; cur <<= need;",
 "left_mid": "u32 hi = cur >> (32 - bits); s32 need = 12 - bits; s32 left = 32 - need; cur = *ptr++; bits = left; work[i+3] = (hi << need) | (cur >> left); cur <<= need;",
 "load_first": "u32 hi = cur >> (32 - bits); s32 need, left; cur = *ptr++; need = 12 - bits; left = 32 - need; work[i+3] = (hi << need) | (cur >> left); cur <<= need; bits = left;",
}
os.makedirs("memory/grind/func_800198D0/micro/gen2", exist_ok=True)
for name, body in V.items():
    p = f"memory/grind/func_800198D0/micro/gen2/{name}.c"
    open(p, "w", newline="\n").write(HDR + FUNC % body)
    out = subprocess.run(["bash", "memory/grind/func_800198D0/micro/cc.sh", p], capture_output=True, text=True).stdout
    lines = [l.strip() for l in out.splitlines() if re.match(r"^\s+[a-z]", l) and not l.strip().startswith(".")]
    body_s = "\n".join(lines)
    has_copy = bool(re.search(r"subu\t(\$\d+),\$\d+,\$\d+\n(?:.*\n){0,2}move\t\$\d+,\1", body_s))
    print(name, "copy=", has_copy)
    print(body_s[:900])
    print("---")

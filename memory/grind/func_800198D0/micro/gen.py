#!/usr/bin/env python3
"""Generate many refill spellings; compile each; report whether the target's
copy shape (subu T; move bits,T; ... srlv x,cur,bits) appears."""
import subprocess, re, itertools, os, sys

HDR = """typedef unsigned int u32;
typedef int s32;
typedef unsigned short u16;
typedef short s16;
"""

FUNC = """
void f(u32 *ptr, u16 *work, s32 bits, u32 cur, s32 *res) {
    %(decls)s
    s32 i;
    for (i = 0; i < 63; i++) {
        if (bits < 12) {
            %(refill)s
        } else {
            work[i + 3] = cur >> 20; cur <<= 12; bits -= 12;
        }
    }
    res[0] = (s32)ptr; res[1] = bits; res[2] = cur;
    %(tail)s
}
"""

variants = {}
D = "u32 hi, v; s32 need, t;"
variants["base"] = (D, "hi = cur >> (32 - bits); cur = *ptr++; need = 12 - bits; bits = 32 - need; work[i+3] = (hi << need) | (cur >> bits); cur <<= need;")
variants["bits_minus_eq"] = (D, "hi = cur >> (32 - bits); cur = *ptr++; need = 12 - bits; bits -= 12; bits += 32; work[i+3] = (hi << need) | (cur >> bits); cur <<= need;")
variants["bits_neg"] = (D, "hi = cur >> (32 - bits); cur = *ptr++; need = 12 - bits; bits = -need + 32; work[i+3] = (hi << need) | (cur >> bits); cur <<= need;")
variants["bits_32_then_sub"] = (D, "hi = cur >> (32 - bits); cur = *ptr++; need = 12 - bits; bits = 32; bits -= need; work[i+3] = (hi << need) | (cur >> bits); cur <<= need;")
variants["need_neg"] = (D, "hi = cur >> (32 - bits); cur = *ptr++; need = bits - 12; bits = 32 + need; work[i+3] = (hi << -need) | (cur >> bits); cur <<= -need;")
variants["bits_from_hi_shift"] = (D, "t = 32 - bits; hi = cur >> t; cur = *ptr++; need = 12 - bits; bits = 32 - need; work[i+3] = (hi << need) | (cur >> bits); cur <<= need;")
variants["cond_bits"] = (D, "hi = cur >> (32 - bits); cur = *ptr++; need = 12 - bits; bits = (bits = 32 - need); work[i+3] = (hi << need) | (cur >> bits); cur <<= need;")
variants["v_carrier"] = (D, "hi = cur >> (32 - bits); cur = *ptr++; need = 12 - bits; bits = 32 - need; v = (hi << need) | (cur >> bits); cur <<= need; work[i+3] = v;")
variants["u_need"] = ("u32 hi, v, need; s32 t;", "hi = cur >> (32 - bits); cur = *ptr++; need = 12 - bits; bits = 32 - need; work[i+3] = (hi << need) | (cur >> bits); cur <<= need;")
variants["s16_need"] = ("u32 hi, v; s16 need; s32 t;", "hi = cur >> (32 - bits); cur = *ptr++; need = 12 - bits; bits = 32 - need; work[i+3] = (hi << need) | (cur >> bits); cur <<= need;")
variants["u8_need"] = ("u32 hi, v; u8 need; s32 t;", "hi = cur >> (32 - bits); cur = *ptr++; need = 12 - bits; bits = 32 - need; work[i+3] = (hi << need) | (cur >> bits); cur <<= need;")
variants["bits_minus_12_plus"] = (D, "hi = cur >> (32 - bits); cur = *ptr++; need = 12 - bits; bits = bits + 32 - 12; work[i+3] = (hi << need) | (cur >> bits); cur <<= need;")
variants["need_then_bits_expr"] = (D, "hi = cur >> (32 - bits); cur = *ptr++; bits = 32 - (need = 12 - bits); work[i+3] = (hi << need) | (cur >> bits); cur <<= need;")
variants["neg_bits"] = (D, "hi = cur >> (32 - bits); cur = *ptr++; need = 12 - bits; bits = 32 - 12 + bits; work[i+3] = (hi << need) | (cur >> bits); cur <<= need;")
variants["sep_lo"] = ("u32 hi, lo; s32 need, t;", "hi = cur >> (32 - bits); cur = *ptr++; need = 12 - bits; bits = 32 - need; lo = cur >> bits; work[i+3] = (hi << need) | lo; cur <<= need;")
variants["shift_first"] = (D, "hi = cur >> (32 - bits); cur = *ptr++; need = 12 - bits; bits = 32 - need; hi <<= need; work[i+3] = hi | (cur >> bits); cur <<= need;")
variants["hi_after_load_var"] = ("u32 hi, old; s32 need;", "old = cur; cur = *ptr++; need = 12 - bits; hi = old >> (32 - bits); bits = 32 - need; work[i+3] = (hi << need) | (cur >> bits); cur <<= need;")

variants["CONTROL_tail"] = (D, "hi = cur >> (32 - bits); cur = *ptr++; need = 12 - bits; t = 32 - need; bits = t; work[i+3] = (hi << need) | (cur >> bits); cur <<= need;")
os.makedirs("memory/grind/func_800198D0/micro/gen", exist_ok=True)
for name, (decls, refill) in variants.items():
    path = f"memory/grind/func_800198D0/micro/gen/{name}.c"
    with open(path, "w", newline="\n") as fh:
        fh.write(HDR + FUNC % {"decls": decls, "refill": refill, "tail": ("t = 0; res[3] = t;" if name.startswith("CONTROL") else "")})
    out = subprocess.run(["bash", "memory/grind/func_800198D0/micro/cc.sh", path], capture_output=True, text=True).stdout
    lines = [l.strip() for l in out.splitlines() if re.match(r"^\s+[a-z]", l) and not l.strip().startswith(".")]
    body = "\n".join(lines)
    has_copy = bool(re.search(r"subu\t(\$\d+),\$\d+,\$\d+\n(?:.*\n){0,2}move\t\$\d+,\1", body))
    print(f"{name:24s} copy={has_copy}  n={len(lines)}")
    if "-v" in sys.argv:
        print(body)

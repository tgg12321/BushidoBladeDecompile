#!/usr/bin/env python3
"""Batch decompile INCLUDE_ASM stubs in text1a.c"""
import os

filepath = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "src", "text1a.c")

with open(filepath, "r") as f:
    content = f.read()

count = 0
def replace_func(name, code):
    global content, count
    target = f'INCLUDE_ASM("asm/funcs", {name});'
    if target in content:
        content = content.replace(target, code)
        count += 1
        print(f"OK: {name}")
    else:
        print(f"NOT FOUND: {name}")

# func_800414FC: relocate pointers in array entries
replace_func("func_800414FC", """void func_800414FC(s32 a0, s32 *a1, s32 a2) {
    s32 v1;
    if (a2 == 0) return;
    v1 = a2 - 1;
    a1 = (s32 *)((u8 *)a1 + 0xC);
    do {
        s32 v0 = *a1;
        if (v0 != 0) {
            *a1 = v0 + a0;
        }
        v1--;
        a1 = (s32 *)((u8 *)a1 + 0x68);
    } while (v1 != -1);
}""")

# func_80040400: setup slot entry, find end of chain
# Using goto to avoid GCC 2.7.2 loop optimizer crash
replace_func("func_80040400", """void func_80040400(s32 a0, s16 *a1, s16 a2) {
    if (*(s16 *)((u8 *)a1 + 2) == -1) {
        goto L_setup;
    }
    a1 = (s16 *)((u8 *)a1 + 0x68);
L_40400_loop:
    if (*(s16 *)((u8 *)a1 + 2) != -1) {
        a1 = (s16 *)((u8 *)a1 + 0x68);
        goto L_40400_loop;
    }
    a1 = (s16 *)((u8 *)a1 - 0x68);
L_setup:
    *(s16 *)((u8 *)a1 + 2) = 2;
    *(u8 *)a1 = 3;
    *(s32 *)((u8 *)a1 + 0xC) = a0 + 0x270;
    *(s16 *)((u8 *)a1 + 6) = 1;
    *(u8 *)((u8 *)a1 + 1) = 0;
    *(s16 *)((u8 *)a1 + 8) = 0;
    *(s16 *)((u8 *)a1 + 0xA) = 0;
    *(s16 *)((u8 *)a1 + 4) = a2;
    *(s32 *)((u8 *)a1 + 0x58) = 0;
    *(s16 *)((u8 *)a1 + 0x6A) = -1;
}""")

# func_80040A78: init offset table - using goto to avoid crash
replace_func("func_80040A78", """void func_80040A78(s32 *a0) {
    s32 i = 0;
    s32 *v1 = (s32 *)((u8 *)a0 + 0x94);
    *(s32 *)((u8 *)a0 + 0x18F4) = (s32)((u8 *)a0 + 0x2B4);
    *(s32 *)((u8 *)a0 + 0x18F8) = (s32)((u8 *)a0 + 0x24C);
    *(s32 *)((u8 *)a0 + 0x18FC) = (s32)((u8 *)a0 + 0x1E4);
    *(s32 *)((u8 *)a0 + 0x1900) = (s32)((u8 *)a0 + 0x454);
    *(s32 *)((u8 *)a0 + 0x1904) = (s32)((u8 *)a0 + 0x3EC);
    *(s32 *)((u8 *)a0 + 0x1908) = (s32)((u8 *)a0 + 0x384);
    *(s32 *)((u8 *)a0 + 0x190C) = (s32)((u8 *)a0 + 0x17C);
    *(s32 *)((u8 *)a0 + 0x1910) = (s32)((u8 *)a0 + 0x114);
    *(s32 *)((u8 *)a0 + 0x1914) = (s32)((u8 *)a0 + 0x72C);
    *(s32 *)((u8 *)a0 + 0x1918) = (s32)((u8 *)a0 + 0x6C4);
    *(s32 *)((u8 *)a0 + 0x191C) = (s32)((u8 *)a0 + 0x5F4);
    *(s32 *)((u8 *)a0 + 0x1920) = (s32)((u8 *)a0 + 0x58C);
    *(s32 *)((u8 *)a0 + 0x1924) = (s32)((u8 *)a0 + 0x524);
    *(s32 *)((u8 *)a0 + 0x1928) = (s32)((u8 *)a0 + 0xAC);
    *(s32 *)((u8 *)a0 + 0x192C) = (s32)((u8 *)a0 + 0x31C);
    *(s32 *)((u8 *)a0 + 0x1930) = (s32)((u8 *)a0 + 0x4BC);
    *(s32 *)((u8 *)a0 + 0x1934) = (s32)((u8 *)a0 + 0x794);
    *(s32 *)((u8 *)a0 + 0x1938) = (s32)((u8 *)a0 + 0x65C);
    *(s32 *)((u8 *)a0 + 0x193C) = (s32)((u8 *)a0 + 0x7FC);
    *(s32 *)((u8 *)a0 + 0x1940) = (s32)((u8 *)a0 + 0x864);
L_0A78_loop:
    *(s32 *)((u8 *)a0 + 0x1994) = (s32)((u8 *)v1 + 0x18);
    v1 = (s32 *)((u8 *)v1 + 0x68);
    i++;
    a0 = (s32 *)((u8 *)a0 + 4);
    if (i < 0x14) goto L_0A78_loop;
}""")

# func_800455AC: register allocation entry
replace_func("func_800455AC", """void func_800455AC(s32 a0) {
    s16 *v1;
    s32 v0;
    func_800453E0(a0);
    v0 = D_800A33AC;
    D_800A33AC = v0 + 1;
    v1 = (s16 *)((u8 *)D_800EED10 + (v0 << 4));
    v1[0] = a0;
    *(s32 *)((u8 *)v1 + 0xC) = 0;
    *(s32 *)((u8 *)v1 + 4) = D_800A33A0;
}""")

# func_8004574C: search D_800EED10 for matching entry
replace_func("func_8004574C", """s32 *func_8004574C(s32 a0) {
    s32 count = D_800A33AC;
    s32 i;
    s16 *p;
    if (count <= 0) return 0;
    p = D_800EED10;
    i = 0;
    do {
        if (*(s16 *)((u8 *)D_800EED10 + i) == a0) {
            return (s32 *)p;
        }
        i += 0x10;
        p = (s16 *)((u8 *)p + 0x10);
    } while (i < count * 16);
    return 0;
}""")

# func_80044010: setup texture table entry
replace_func("func_80044010", """void func_80044010(s32 *a0, s16 a1) {
    s32 *base = a0;
    s32 v1 = *a0;
    s32 v0 = (v1 | 0x8000) & 0xFFFF;
    *a0 = v0;
    a0 = a0 + 1;
    D_80103608[a1] = (s32 *)a0;
    D_80103658[a1] = (u16)(v1 & 0x7FFF);
    v0 = v1;
    if ((v1 & 0x8000) == 0) {
        s32 count = v0 & 0xFFFF;
        if (count > 0) {
            s32 i = 0;
            do {
                *a0 = *a0 + (s32)base;
                i++;
                a0++;
            } while (i < count);
        }
    }
}""")

# func_80044098: remove texture table entry
replace_func("func_80044098", """void func_80044098(s16 a0) {
    s32 *v1 = D_80103608[(s32)(s16)a0];
    s32 a0_val = *(v1 - 1);
    s32 *base;
    if (!(a0_val & 0x8000)) return;
    base = v1 - 1;
    a0_val &= 0x7FFF;
    *(v1 - 1) = a0_val;
    a0_val--;
    if (a0_val == -1) return;
    do {
        *v1 = *v1 - (s32)base;
        a0_val--;
        v1++;
    } while (a0_val != -1);
}""")

# func_80044100: scroll texture table
replace_func("func_80044100", """void func_80044100(s32 a0, s32 a1) {
    s32 *base = (s32 *)D_80103608;
    s32 *entry = &base[a0];
    s32 *v1 = *((s32 **)entry);
    s32 count = (s32)D_80103658[a0];
    s32 a3 = a1;
    if (a1 < 0) {
        a3 = a1 + 3;
    }
    v1 = (s32 *)((u8 *)base + ((a3 >> 2) << 2));
    count--;
    *((s32 **)entry) = v1;
    if (count == -1) return;
    do {
        *v1 = *v1 + a1;
        count--;
        v1++;
    } while (count != -1);
}""")

# func_80045510: find and update allocation
replace_func("func_80045510", """void func_80045510(s32 a0, s32 a1) {
    s32 i = 0;
    s32 count = D_800A33AC;
    if (count <= 0) return;
    {
        s32 *p = (s32 *)((u8 *)D_800EED10 + 8);
        s32 offset = 0;
        do {
            if (*(s16 *)((u8 *)D_800EED10 + offset) == a0) {
                s32 old = *p;
                s32 diff = a1 - old;
                if (diff == 0) return;
                func_80045294(i + 1, diff);
                *p = a1;
                return;
            }
            p = (s32 *)((u8 *)p + 0x10);
            i++;
            offset += 0x10;
        } while (i < D_800A33AC);
    }
}""")

# func_80045600: allocate from pool
replace_func("func_80045600", """void func_80045600(s32 a0, s32 a1) {
    s32 i = 0;
    s32 count = D_800A33AC;
    s16 *entry;
    if (count <= 0) goto L_5600_err;
    entry = D_800EED10;
    do {
        if (entry[0] == a0) goto L_5600_found;
        i++;
        entry = (s16 *)((u8 *)entry + 0x10);
    } while (i < count);
    if (i >= D_800A33AC) goto L_5600_err;
L_5600_found:
    {
        s32 off = a1 - D_800A33A0;
        D_800A33A0 += off;
        D_800A33A4 -= off;
        *(s32 *)((u8 *)entry + 8) = off;
    }
    return;
L_5600_err:
    func_80052C10();
}""")

# func_800450F4: check and load palette
replace_func("func_800450F4", """s32 func_800450F4(s32 a0, s32 a1) {
    s32 a0_idx;
    s32 *base;
    s32 v1;
    if (D_800A3244 == 0) return 0;
    a0_idx = a0 - 0x1E;
    if ((u32)a0_idx >= 7) return 0;
    {
        s32 val = *(s16 *)((u8 *)&D_800963EC + a0_idx * 2);
        if (val == -1) return 0;
        base = (s32 *)D_800A3398;
        v1 = *(s32 *)base;
        if (v1 != 5) {
            D_800A3244 = 0;
            return 0;
        }
        {
            s32 *p = (s32 *)((u8 *)base + val * 4);
            s32 off = *(s32 *)((u8 *)p + 4);
            s32 sz = *(s32 *)((u8 *)p + 8);
            func_800520B8((s32)((u8 *)base + ((off >> 2) << 2)), sz - off);
        }
    }
    return 1;
}""")

# func_80046048: iterate and apply sound effects
replace_func("func_80046048", """void func_80046048(s32 a0, s32 a1) {
    s32 *p;
    s32 count;
    func_80044100(6, a0);
    p = (s32 *)func_800457A0(6);
    if (!p) return;
    {
        s32 v = *p;
        if (v == 0) return;
        p = (s32 *)((u8 *)p + v);
    }
    count = *p - 1;
    p++;
    if (count == -1) return;
    do {
        func_8005C4C0(a1, *p);
        p++;
        count--;
    } while (count != -1);
}""")

# func_80045AA4: allocate and setup structure
replace_func("func_80045AA4", """void func_80045AA4(s32 a0, s32 a1) {
    if (a0 < 3) {
        func_80041430(a0, a1);
        return;
    }
    {
        s32 idx = a0 - 3;
        s32 *p;
        func_80044100(idx, a1);
        func_80044100(a0, a1);
        p = (s32 *)func_800457A0(idx);
        if (!p) return;
        {
            s32 v = *(s32 *)((u8 *)p + 0x1C);
            *(s32 *)((u8 *)p + 0x1C) = v + a1;
            func_8004019C(p, a1);
            v = *p;
            if (((v >> 1) & 1) == 0) return;
            {
                s32 h = *(s16 *)((u8 *)p + 4);
                func_8005C4C0(a1, h * 2 + h + 1);
            }
        }
    }
}""")

with open(filepath, "w", newline="\n") as f:
    f.write(content)

print(f"\nTotal replaced: {count}")
remaining = content.count('INCLUDE_ASM("asm/funcs"')
print(f"Remaining INCLUDE_ASM stubs: {remaining}")

"""Body spellings of func_80026DA4 for `engine.cli sandbox --candidate`."""
import re
d = "tmp/ffc/26da4"
b0 = open(f"{d}/v0_control.c", encoding="utf-8").read()


def w(name, s):
    open(f"{d}/{name}.c", "w", encoding="utf-8", newline="\n").write(s)


def sub(s, a, b, n=1):
    assert s.count(a) == n, (a, s.count(a))
    return s.replace(a, b)


# v2: honest names only (s0 -> rec, s1 -> other)
b2 = re.sub(r"\bs0\b", "rec", b0)
b2 = re.sub(r"\bs1\b", "other", b2)
w("v2_renamed", b2)

# v3: v2 + void prototypes of the two later-defined callees in scope
w("v3_prototyped", "extern void func_80027A58(s32 *);\n"
  "extern void func_80032854(s32, s32, u8 *, s16 *);\n" + b2)

# v3b: v2 + prototypes returning s32 (callee defined void; probe only)
w("v3b_proto_s32", "extern s32 func_80027A58(s32 *);\n"
  "extern s32 func_80032854(s32, s32, u8 *, s16 *);\n" + b2)

# v5: 0x1C arm gets its own pair of locals, so `rec` is never re-pointed;
# the pre-tail reset then re-stores a held value on every path -> dropped.
b5 = sub(b2, "    u8 *rec;\n    u8 *other;\n",
         "    u8 *rec;\n    u8 *other;\n    u8 *win;\n    u8 *lose;\n")
b5 = sub(b5, """        rec = (u8 *)&D_80101EC8 + idx * 0x44C;
        other = (u8 *)&D_80101EC8;
        if (idx == 0) {
            other += 0x44C;
        }
        *(s16 *)(rec + 0x286) = 3;
        *(s16 *)(other + 0x286) = 4;""", """        win = (u8 *)&D_80101EC8 + idx * 0x44C;
        lose = (u8 *)&D_80101EC8;
        if (idx == 0) {
            lose += 0x44C;
        }
        *(s16 *)(win + 0x286) = 3;
        *(s16 *)(lose + 0x286) = 4;""")
w("v5_1c_own_locals_keep_resets", b5)
b5b = sub(b5, "    rec = (u8 *)&D_80101EC8;\ntail:", "tail:")
w("v5b_1c_own_locals_no_pretail", b5b)
b5c = sub(b5b, """        rec = (u8 *)&D_80101EC8;
        other = rec + 0x44C;
        D_800A389C++;""", """        other = rec + 0x44C;
        D_800A389C++;""")
w("v5c_1c_own_locals_no_resets", b5c)

# v6: one pointer per section (top/loop base, 0x1C pair, post-loop pair, tail pair)
b6 = sub(b2, "    u8 *rec;\n    u8 *other;\n",
         "    u8 *base;\n    u8 *win;\n    u8 *lose;\n    u8 *rec;\n    u8 *other;\n    u8 *a;\n    u8 *b;\n")
b6 = sub(b6, "    rec = (u8 *)&D_80101EC8;\n    D_800A3824 = -1;", "    base = (u8 *)&D_80101EC8;\n    D_800A3824 = -1;")
b6 = sub(b6, """        rec = (u8 *)&D_80101EC8 + idx * 0x44C;
        other = (u8 *)&D_80101EC8;
        if (idx == 0) {
            other += 0x44C;
        }
        *(s16 *)(rec + 0x286) = 3;
        *(s16 *)(other + 0x286) = 4;""", """        win = (u8 *)&D_80101EC8 + idx * 0x44C;
        lose = (u8 *)&D_80101EC8;
        if (idx == 0) {
            lose += 0x44C;
        }
        *(s16 *)(win + 0x286) = 3;
        *(s16 *)(lose + 0x286) = 4;""")
b6 = sub(b6, "            p = rec + i * 0x44C;", "            p = base + i * 0x44C;")
b6 = sub(b6, "    rec = (u8 *)&D_80101EC8;\ntail:\n    other = rec + 0x44C;",
         "tail:\n    a = (u8 *)&D_80101EC8;\n    b = a + 0x44C;")
# tail region uses a/b
head, tail = b6.split("tail:\n", 1)
tail = re.sub(r"\brec\b", "a", tail)
tail = re.sub(r"\bother\b", "b", tail)
b6 = head + "tail:\n" + tail
w("v6_one_pointer_per_section", b6)
print("ok")

# v7 (proposed): v2 + Q51/Q53 annotations (comments only)
b7 = sub(b2, "    u8 *rec;\n    u8 *other;\n",
"""    /* FAKE: one record pointer re-pointed per section, admitted on SOTN reuse
     * precedent (owner Q51, Q53). It holds record 0 at entry, the selected
     * record in the 0x1C arm, and is re-set to record 0 after the drift loop
     * and before the tail even where it already holds record 0. Mechanism:
     * each re-set is its own lui/addiu of the base into $s0 in the target
     * (asm/funcs/func_80026DA4.s:9-10, 133-134, 249-250); cse.c works per
     * extended basic block, so a re-set after the loop or at the tail join
     * re-materializes the base. Exhaustion (memory/grind/func_80026DA4/
     * evidence.md): no post-loop re-set 61; own locals for the 0x1C arm 16,
     * 18, 82; one pointer per section 66; 2026-09-24 split grid 29-93. */
    u8 *rec;
    /* FAKE: the record paired with rec, re-derived from rec at each re-set
     * (same Q51/Q53 route and exhaustion as rec). */
    u8 *other;
""")
b7 = sub(b7, """        rec = (u8 *)&D_80101EC8;
        other = rec + 0x44C;
        D_800A389C++;""", """        rec = (u8 *)&D_80101EC8; /* SOTN: src/weapon/w_011.c:346 @db41b28 */
        other = rec + 0x44C;
        D_800A389C++;""")
b7 = sub(b7, "    rec = (u8 *)&D_80101EC8;\ntail:",
         "    rec = (u8 *)&D_80101EC8; /* SOTN: src/boss/rbo0/e_fake_sypha.c:816 @db41b28 */\ntail:")
w("v7_proposed", b7)
print("v7 ok")

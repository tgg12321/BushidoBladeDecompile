# func_80033D38 spellings (code6cac_b_tu2.c)
import os
OUT = os.path.dirname(os.path.abspath(__file__)) + "/b"
BODY = """void func_80033D38(void) {
DECL    s32 n = 3;
    s32 j;
    s32 k;

    while (1) {
        j = n - 1;
        if (R.times[j].unk_4 < D_800A3858) {
            break;
        }
        n = j;
        if (n <= 0) {
            break;
        }
    }
    D_800A38E9 = (u8)n;
    if (n < 3) {
        for (k = 2; k > n; k--) {
            R.times[k] = R.times[k - 1];
        }
        R.times[n].unk_0 = (u8)g_practice_menu_table[0].unk_0A;
        R.times[n].unk_1 = (u8)g_practice_menu_table[0].unk_0E;
        R.times[n].unk_4 = D_800A3858;
    }
}
"""
def mk(decl, r):
    return BODY.replace("DECL", decl).replace("R.", r)
V = {
    "landed": mk("    FileRecord *rec = &D_80106A50;\n", "rec->"),
    "direct": mk("", "D_80106A50."),
}
# times base pointer
T = BODY.replace("DECL", "    FileTimeRec *t = D_80106A50.times;\n").replace("R.times", "t")
V["times_ptr"] = T
# direct, loop copying member-wise
V["direct_memberwise"] = V["direct"].replace("D_80106A50.times[k] = D_80106A50.times[k - 1];",
    "D_80106A50.times[k].unk_0 = D_80106A50.times[k - 1].unk_0;\n            D_80106A50.times[k].unk_1 = D_80106A50.times[k - 1].unk_1;\n            D_80106A50.times[k].unk_4 = D_80106A50.times[k - 1].unk_4;")
# direct, for-loop search
V["direct_for"] = """void func_80033D38(void) {
    s32 n;
    s32 k;

    for (n = 3; n > 0; n--) {
        if (D_80106A50.times[n - 1].unk_4 < D_800A3858) {
            break;
        }
    }
    D_800A38E9 = (u8)n;
    if (n < 3) {
        for (k = 2; k > n; k--) {
            D_80106A50.times[k] = D_80106A50.times[k - 1];
        }
        D_80106A50.times[n].unk_0 = (u8)g_practice_menu_table[0].unk_0A;
        D_80106A50.times[n].unk_1 = (u8)g_practice_menu_table[0].unk_0E;
        D_80106A50.times[n].unk_4 = D_800A3858;
    }
}
"""
V["rec_for"] = V["direct_for"].replace("    s32 n;\n", "    FileRecord *rec = &D_80106A50;\n    s32 n;\n").replace("D_80106A50.times", "rec->times")
for k, b in V.items():
    open(f"{OUT}/{k}.c", "w").write(b)
print(len(V))

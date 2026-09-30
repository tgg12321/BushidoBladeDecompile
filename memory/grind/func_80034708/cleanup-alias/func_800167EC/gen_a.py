# func_800167EC spellings (ings.c). Writes one candidate body per variant.
import os
OUT = os.path.dirname(os.path.abspath(__file__)) + "/a"
HEAD = "void func_800167EC(void) {\n"
V = {}
V["landed"] = """    s32 i = 0;
    u32 c = 0x1A5E0;
    FileRecord *rec;

    rec = &D_80106A50;
    D_800A3710 = 0;
    D_80106A50.flags = 0;
    rec->unk_00 = 0x7007;
    D_80106A50.unk_04 = 0;
    for (i = 0; i < 3; i++) {
        rec->times[i].unk_0 = 0;
        rec->times[i].unk_1 = 0;
        rec->times[i].unk_4 = c;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
"""
V["direct"] = """    s32 i;

    D_800A3710 = 0;
    D_80106A50.flags = 0;
    D_80106A50.unk_00 = 0x7007;
    D_80106A50.unk_04 = 0;
    for (i = 0; i < 3; i++) {
        D_80106A50.times[i].unk_0 = 0;
        D_80106A50.times[i].unk_1 = 0;
        D_80106A50.times[i].unk_4 = 0x1A5E0;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
"""
V["direct_c"] = """    s32 i;
    u32 c = 0x1A5E0;

    D_800A3710 = 0;
    D_80106A50.flags = 0;
    D_80106A50.unk_00 = 0x7007;
    D_80106A50.unk_04 = 0;
    for (i = 0; i < 3; i++) {
        D_80106A50.times[i].unk_0 = 0;
        D_80106A50.times[i].unk_1 = 0;
        D_80106A50.times[i].unk_4 = c;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
"""
V["direct_order2"] = """    s32 i;

    D_800A3710 = 0;
    D_80106A50.unk_00 = 0x7007;
    D_80106A50.flags = 0;
    D_80106A50.unk_04 = 0;
    for (i = 0; i < 3; i++) {
        D_80106A50.times[i].unk_0 = 0;
        D_80106A50.times[i].unk_1 = 0;
        D_80106A50.times[i].unk_4 = 0x1A5E0;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
"""
V["direct_order3"] = """    s32 i;

    D_80106A50.unk_00 = 0x7007;
    D_800A3710 = 0;
    D_80106A50.flags = 0;
    D_80106A50.unk_04 = 0;
    for (i = 0; i < 3; i++) {
        D_80106A50.times[i].unk_0 = 0;
        D_80106A50.times[i].unk_1 = 0;
        D_80106A50.times[i].unk_4 = 0x1A5E0;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
"""
V["direct_while"] = """    s32 i = 0;

    D_800A3710 = 0;
    D_80106A50.flags = 0;
    D_80106A50.unk_00 = 0x7007;
    D_80106A50.unk_04 = 0;
    do {
        D_80106A50.times[i].unk_0 = 0;
        D_80106A50.times[i].unk_1 = 0;
        D_80106A50.times[i].unk_4 = 0x1A5E0;
        i++;
    } while (i < 3);
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
"""
V["reconly"] = """    s32 i;
    FileRecord *rec = &D_80106A50;

    D_800A3710 = 0;
    rec->flags = 0;
    rec->unk_00 = 0x7007;
    rec->unk_04 = 0;
    for (i = 0; i < 3; i++) {
        rec->times[i].unk_0 = 0;
        rec->times[i].unk_1 = 0;
        rec->times[i].unk_4 = 0x1A5E0;
    }
    rec->times[0].unk_4 = 0x6978;
    func_8001945C();
"""
V["times_ptr"] = """    s32 i;
    FileTimeRec *t = D_80106A50.times;

    D_800A3710 = 0;
    D_80106A50.flags = 0;
    D_80106A50.unk_00 = 0x7007;
    D_80106A50.unk_04 = 0;
    for (i = 0; i < 3; i++) {
        t[i].unk_0 = 0;
        t[i].unk_1 = 0;
        t[i].unk_4 = 0x1A5E0;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
"""
V["times_walk"] = """    s32 i;
    FileTimeRec *t;

    D_800A3710 = 0;
    D_80106A50.flags = 0;
    D_80106A50.unk_00 = 0x7007;
    D_80106A50.unk_04 = 0;
    t = D_80106A50.times;
    for (i = 0; i < 3; i++, t++) {
        t->unk_0 = 0;
        t->unk_1 = 0;
        t->unk_4 = 0x1A5E0;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
"""
V["unk00_last"] = """    s32 i;

    D_800A3710 = 0;
    D_80106A50.flags = 0;
    D_80106A50.unk_04 = 0;
    D_80106A50.unk_00 = 0x7007;
    for (i = 0; i < 3; i++) {
        D_80106A50.times[i].unk_0 = 0;
        D_80106A50.times[i].unk_1 = 0;
        D_80106A50.times[i].unk_4 = 0x1A5E0;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
"""
V["struct_init_order"] = """    s32 i;

    D_800A3710 = 0;
    D_80106A50.unk_00 = 0x7007;
    D_80106A50.unk_04 = 0;
    for (i = 0; i < 3; i++) {
        D_80106A50.times[i].unk_0 = 0;
        D_80106A50.times[i].unk_1 = 0;
        D_80106A50.times[i].unk_4 = 0x1A5E0;
    }
    D_80106A50.flags = 0;
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
"""
for k, b in V.items():
    open(f"{OUT}/{k}.c", "w").write(HEAD + b + "}\n")
print(len(V))

# func_800167EC: pointer forms with the least mixing
import os
OUT = os.path.dirname(os.path.abspath(__file__)) + "/a"
HEAD = "void func_800167EC(void) {\n"
V = {}
V["rec_lastdirect"] = """    s32 i;
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
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
"""
V["rec_lastdirect_c"] = """    s32 i;
    u32 c = 0x1A5E0;
    FileRecord *rec = &D_80106A50;

    D_800A3710 = 0;
    rec->flags = 0;
    rec->unk_00 = 0x7007;
    rec->unk_04 = 0;
    for (i = 0; i < 3; i++) {
        rec->times[i].unk_0 = 0;
        rec->times[i].unk_1 = 0;
        rec->times[i].unk_4 = c;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
"""
V["rec_first_lastdirect"] = """    s32 i;
    FileRecord *rec;

    rec = &D_80106A50;
    D_800A3710 = 0;
    rec->flags = 0;
    rec->unk_00 = 0x7007;
    rec->unk_04 = 0;
    for (i = 0; i < 3; i++) {
        rec->times[i].unk_0 = 0;
        rec->times[i].unk_1 = 0;
        rec->times[i].unk_4 = 0x1A5E0;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
"""
V["rec_reconly_c"] = """    s32 i;
    u32 c = 0x1A5E0;
    FileRecord *rec = &D_80106A50;

    D_800A3710 = 0;
    rec->flags = 0;
    rec->unk_00 = 0x7007;
    rec->unk_04 = 0;
    for (i = 0; i < 3; i++) {
        rec->times[i].unk_0 = 0;
        rec->times[i].unk_1 = 0;
        rec->times[i].unk_4 = c;
    }
    rec->times[0].unk_4 = 0x6978;
    func_8001945C();
"""
V["rec_timesonly"] = """    s32 i;
    FileRecord *rec = &D_80106A50;

    D_800A3710 = 0;
    D_80106A50.flags = 0;
    D_80106A50.unk_00 = 0x7007;
    D_80106A50.unk_04 = 0;
    for (i = 0; i < 3; i++) {
        rec->times[i].unk_0 = 0;
        rec->times[i].unk_1 = 0;
        rec->times[i].unk_4 = 0x1A5E0;
    }
    D_80106A50.times[0].unk_4 = 0x6978;
    func_8001945C();
"""
for k, b in V.items():
    open(f"{OUT}/{k}.c", "w").write(HEAD + b + "}\n")

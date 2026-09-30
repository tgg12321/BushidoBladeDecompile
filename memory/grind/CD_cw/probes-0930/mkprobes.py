from pathlib import Path
def W(p, t): Path(p).write_bytes(t.encode("utf-8"))
b = Path("memory/grind/CD_cw/candidate.c").read_bytes().decode("utf-8")
p1 = b.replace("    s32 i;\n\n    if (CD_debug > 1)", "    s32 i;\n    s32 j;\n\n    if (CD_debug > 1)")
p1 = p1.replace("for (i = 0; i < D_800A12FC[com + 0x40]; i++) {", "for (j = 0; j < D_800A12FC[com + 0x40]; j++) {")
p1 = p1.replace("*g_cd_req_reg = param[i];", "*g_cd_req_reg = param[j];")
assert p1.count("param[j]") == 1 and "s32 j;" in p1
W("tmp/CD_cw/p_twocounters.c", p1)
cw = b[b.index("s32 CD_cw("):]
cw = cw.replace("    set_alarm(D_8001626C);\n", "    Alarm.time = VSync(-1) + 0x3C0;\n    Alarm.count = 0;\n    Alarm.name = D_8001626C;\n")
old = """        if (get_alarm()) {
            return -1;
        }
        if (CheckCallback()) {
            callback();
        }
"""
assert old in cw
cw = cw.replace(old, """        if (Alarm.time < VSync(-1) || Alarm.count++ > 0x3C0000) {
            puts(&D_800161B8);
            printf(&D_800161C8, Alarm.name, CD_comstr[CD_com],
                   CD_intstr[Intr.sync], CD_intstr[Intr.ready]);
            CD_flush();
            return -1;
        }
        if (CheckCallback()) {
            s32 status;
            u8 saved;

            saved = *D_800A147C & 3;
            while (1) {
                status = getintr();
                if (status == 0) {
                    break;
                }
                if ((status & 4) && CD_cbready != 0) {
                    ((void (*)(u8, void *))CD_cbready)(Intr.ready, &Result_plus_0x8);
                }
                if ((status & 2) && CD_cbsync != 0) {
                    ((void (*)(u8, void *))CD_cbsync)(Intr.sync, &Result);
                }
            }
            *D_800A147C = saved;
        }
""")
cw = cw.replace("    _memcpy(result, &Result, 8);\n", """    if (result != 0) {
        u8 *src = (u8 *)&Result;
        u32 n = 8;
        while (n--) {
            *result++ = *src++;
        }
    }
""")
assert "get_alarm" not in cw and "callback()" not in cw and "_memcpy" not in cw
W("tmp/CD_cw/p_nohelpers.c", cw)
print("ok")

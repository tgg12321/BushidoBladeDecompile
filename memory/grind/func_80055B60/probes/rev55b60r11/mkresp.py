b = open('F_staged_body.c').read()
def mk(name, pairs):
    t = b
    for o, n in pairs:
        assert t.count(o) == 1, (name, o)
        t = t.replace(o, n)
    open('R_%s.c' % name, 'w', newline='\n').write(t)
mk('mask_tern', [("""                    temp = 0x20;
                    if (rec->unk_414[i][1] >= rec->unk_424 * 4) {
                        temp = 0x60;
                    }
                    rec->unk_430 |= temp;""", """                    rec->unk_430 |= rec->unk_414[i][1] >= rec->unk_424 * 4 ? 0x60 : 0x20;""")])
mk('far_inline', [("                temp3 = temp2 + 800;\n", ""),
                  ("} else if (temp3 >= D_800A387C) {", "} else if (temp2 + 800 >= D_800A387C) {"),
                  ("rec->unk_42E = temp3;", "rec->unk_42E = temp2 + 800;")])
mk('sign_inline', [("""        work >>= 31;
        if (work != (rec->unk_3F0 >> 15)) {
            rec->unk_3F0 = 0;
        }
        rec->unk_3F0 += work ? -1 : 1;""", """        if ((work >> 31) != (rec->unk_3F0 >> 15)) {
            rec->unk_3F0 = 0;
        }
        rec->unk_3F0 += work < 0 ? -1 : 1;""")])
mk('sign_inline2', [("""        work >>= 31;
        if (work != (rec->unk_3F0 >> 15)) {
            rec->unk_3F0 = 0;
        }
        rec->unk_3F0 += work ? -1 : 1;""", """        if ((work >> 31) != (rec->unk_3F0 >> 15)) {
            rec->unk_3F0 = 0;
        }
        rec->unk_3F0 += (work >> 31) ? -1 : 1;""")])
mk('lim_inline', [("    temp2 = (D_80099D88[rec->unk_443].unk7 * 25u) >> 3;\n", ""),
                  ("    if (temp2 < (work >= 0 ? work : -work)) {", "    if (((D_80099D88[rec->unk_443].unk7 * 25u) >> 3) < (work >= 0 ? work : -work)) {")])
mk('add_tern', [("""                temp = 4;
                if ((u16)rec->unk_6A == 0x11) {
                    temp = 8;
                }""", """                temp = (u16)rec->unk_6A == 0x11 ? 8 : 4;""")])

R = '                    if (D_800A3560[D_800A3554 * 3] == 0xFF) {\n'
W = '                        D_800A3560[3] = 0xFF;\n'
T = '                } else if (D_800A3554 == 1) {\n'
VARIANTS = {
    'p1': [(R, '                    if (D_800A3560[port_ofs * 3] == 0xFF) {\n'),
           (W, '                        D_800A3560[port_ofs * 3] = 0xFF;\n')],
    'p1r': [(R, '                    if (D_800A3560[port_ofs * 3] == 0xFF) {\n')],
    'p2': [(T, '                } else if (port_ofs == 1) {\n'),
           (R, '                    if (D_800A3560[port_ofs * 3] == 0xFF) {\n'),
           (W, '                        D_800A3560[port_ofs * 3] = 0xFF;\n')],
    'p3': [(T, '                } else if (port_ofs == 1) {\n')],
}

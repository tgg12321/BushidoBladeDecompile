R = '                    if (D_800A3560[D_800A3554 * 3] == 0xFF) {\n'
W = '                        D_800A3560[3] = 0xFF;\n'
def both(x):
    return [(R, f'                    if (D_800A3560[{x}] == 0xFF) {{\n'), (W, f'                        D_800A3560[{x}] = 0xFF;\n')]
VARIANTS = {
    'q1': both('D_800A3554 * 2 + D_800A3554'),
    'q2': both('D_800A3554 + D_800A3554 * 2'),
    'q3': both('(D_800A3554 << 1) + D_800A3554'),
    'q5': both('3 * D_800A3554'),
    'q6': both('D_800A3554 * 3 + 0'),
}

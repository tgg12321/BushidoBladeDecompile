def patch(s):
    # MEASUREMENT: D_8009BD24 as a struct { u8 chr[2][5][2]; s32 flags; } in the TU
    s = s.replace('extern u8 D_8009BD24[];',
                  'extern Cfg5E54C D_8009BD24;')
    s = s.replace('D_8009BD24[0] < 0xC', 'D_8009BD24.chr[0][0][0] < 0xC')
    s = s.replace('func_8006E534(a0, D_800A35E0, D_8009BD24, D_800A35E8)',
                  'func_8006E534(a0, D_800A35E0, (u8 *)&D_8009BD24, D_800A35E8)')
    return s

/* REJECTED (s5, 2026-08-25): Borrowing the mainline s2-offset local (`off`) as case 3's second carrier: case 3 itself reaches target's first eight instructions, but hoisting that local costs 13 instructions elsewhere in the mainline. sandbox-equivalent diffs=22 (246 insns). */
void func_800460E4(s32 stage_id, s32 arg1) {
    s32 *s0;
    s32 s7;
    s32 *s6, *s4, *s2;
    s32 s3;
    s32 *s1;
    s32 *fp_ptr;
    s32 *sp10, *sp18, *sp20;
    s32 off1_raw;
    s32 off;
    s32 off3;

    s0 = func_800457A0(7);
    if (s0 != ((void *)0)) {
        if (D_80099478 == stage_id) {
            s7 = 1;
            switch (stage_id) {
            case 3:
                break;
            case 4:
            case 7:
            case 18:
                s3 = s0[0];
                {
                    s32 off = (((u32)(s0[s3 - 1]) >> 2) << 2);
                    func_8003EDC0(((s32)((u8 *)(s0) + (off))), 7);
                }
                break;
            case 34:
                s7 = 0;
                break;
            }
            stage_ExecInitFunc();
            if (s7 != 0) {
                return;
            }
        }
    }

    D_80099478 = (s16)stage_id;
    s7 = 7;
    s0 = func_800455AC(7);

    if (arg1 != 0) {
        func_80044F30(stage_id, arg1);
    } else {
        func_80044F30(stage_id, (s32)s0);
    }

    if (arg1 != 0) {
        s3 = *(s32 *)arg1;
        func_80045824(arg1, (s32)s0, ((s32 *)arg1)[s3]);
    }

    {
        off1_raw = s0[1];
        s3 = s0[0];
        s6 = (s32 *)((u8 *)s0 + (((u32)(off1_raw) >> 2) << 2));
        {
            s32 *a0_ptr = (s32 *)((s3 << 2) + (s32)s0);
            s4 = (s32 *)((u8 *)s0 + (((u32)(a0_ptr[-1]) >> 2) << 2));
            sp10 = (s32 *)((u8 *)s0 + (((u32)(s0[2]) >> 2) << 2));
            sp18 = (s32 *)((u8 *)s0 + (((u32)(s0[3]) >> 2) << 2));
            sp20 = (s32 *)((u8 *)s0 + (((u32)(s0[4]) >> 2) << 2));

            {
                off = (((u32)(a0_ptr[0]) >> 2) << 2);
                s2 = (s32 *)((u8 *)s0 + off);
            }

            if (arg1 != 0) {
                fp_ptr = (s32 *)((u8 *)arg1 + (((u32)(((s32 *)arg1)[s3]) >> 2) << 2));
            } else {
                fp_ptr = s2;
                {
                    off3 = (((u32)(a0_ptr[1]) >> 2) << 2);
                    func_80045230(((s32)((u8 *)(s0) + (off3))));
                }
            }
        }
    }

    D_8009947A = 0;





    s1 = (s32 *)((s32)s4 - (s32)s0);
    s1 = (s32 *)((s32)s1 + (s32)s0);
    switch (stage_id) {
    case 3:
        s1 = s2;
        off1_raw = s0[s3 - 2];
        off = s0[s3 - 1];
        s6 = &s0[(u32)off1_raw >> 2];
        s4 = &s0[(u32)off >> 2];
        D_8009947A = 1;
        break;
    case 4:
    case 7:
    case 18:
        s1 = s2;
        func_80044010(((s32)((u8 *)(s0) + ((((u32)(s0[5]) >> 2) << 2)))), 8);
        s1 = (s32 *)func_80044670(((s32)((u8 *)(s0) + ((((u32)(s0[6]) >> 2) << 2)))), 8, (s32)s1);
        break;
    case 11:
        snd_SetVolume((s32)s1);
        s1 = (s32 *)((s32)s1 + snd_GetMaxFade());
        break;
    case 13:
        s1 = s2;
        s6 = (s32 *)((u8 *)s0 + (((u32)(s0[s3 - 2]) >> 2) << 2));
        s4 = (s32 *)((u8 *)s0 + (((u32)(s0[s3 - 1]) >> 2) << 2));
        func_80044010(((s32)((u8 *)(s0) + ((((u32)(s0[5]) >> 2) << 2)))), 8);
        D_8009947A = 1;
        break;
    case 34:
        s1 = s2;
        D_8009947A = 1;
        s4 = (s32 *)((u8 *)s0 + (((u32)(s0[5]) >> 2) << 2));
        break;
    }

    func_80044010((s32)s6, 7);
    func_800481E8((s32)fp_ptr, 0);
    func_8003EDC0((s32)s4, 7);
    func_80054410((s32)sp10);
    D_800A33B0 = (s32)sp18;
    D_800A33B4 = (s32)sp20;
    DrawSync(0);
    func_80045600(s7, (s32)s1);
    func_80045694(s7, (s32)func_800466C0);
    stage_ExecInitFunc();
    if (D_800A38DC != 0) {
        if (stage_id != 0x22) {
            func_8004659C(-1);
        }
    }
}

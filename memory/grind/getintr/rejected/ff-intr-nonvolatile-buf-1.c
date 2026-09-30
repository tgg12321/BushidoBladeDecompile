s32 getintr(void) {
    volatile char nReg;
    Result_t buf;
    s32 i, j;
    s32 bHasError;

    *g_cd_index_reg = 1;

    nReg = *g_cd_irq_reg & 0x7;

    if (nReg == 0) {
        return 0;
    }

    bHasError = 0;

    while (nReg != (*g_cd_irq_reg & 7)) {
        nReg = *g_cd_irq_reg & 0x7;
    }

    for (i = 0; i < 8; i++) {
        if ((*g_cd_index_reg & 0x20) == 0) {
            break;
        }
        buf[i] = *g_cd_param_fifo;
    }
    for (j = i; j < 8; j++) {
        buf[j] = 0;
    }

    *g_cd_index_reg = 1;
    *g_cd_irq_reg = 7;
    *g_cd_req_reg = 7;
    if (nReg != 3 || D_800A137C[CD_com]) {
        if (!(*(s32 *)&CD_status & 0x10) && (buf[0] & 0x10)) {
            CD_nopen++;
        }
        *(s32 *)&CD_status = buf[0];
        CD_status1 = buf[1];
        bHasError = *(s32 *)&CD_status;
        bHasError &= 0x1D;
    }
    if (nReg == 5) {
        if (CD_debug > 0) {
            printf(D_800161E4);
        }
        if (CD_debug > 0) {
            printf(D_800161F0, CD_comstr[CD_com], *(s32 *)&CD_status, CD_status1);
        }
    }
    switch (nReg) {
    case 3:
        if (bHasError) {
            Intr.sync = 5;
            _memcpy(&Result, &buf, sizeof(Result_t));
            return 2;
        }
        if (D_800A127C[CD_com]) {
            Intr.sync = 3;
            _memcpy(&Result, &buf, sizeof(Result_t));
            return 1;
        }
        Intr.sync = 2;
        _memcpy(&Result, &buf, sizeof(Result_t));
        return 2;
    case 2:
        Intr.sync = bHasError ? 5 : 2;
        _memcpy(&Result, &buf, sizeof(Result_t));
        return 2;
    case 1:
        if (bHasError && i == 1) {
            bHasError = 0;
        }
        Intr.ready = bHasError ? 5 : 1;
        _memcpy(&Result_plus_0x8, &buf, sizeof(Result_t));
        *g_cd_index_reg = 0;
        *g_cd_irq_reg = 0;
        return 4;
    case 4:
        Intr.ready = Intr.c = 4;
        _memcpy(&Result_plus_0x10, &buf, sizeof(Result_t));
        _memcpy(&Result_plus_0x8, &buf, sizeof(Result_t));
        return 4;
    case 5:
        Intr.sync = Intr.ready = 5;
        _memcpy(&Result, &buf, sizeof(Result_t));
        _memcpy(&Result_plus_0x8, &buf, sizeof(Result_t));
        return 6;
    default:
        puts(D_8001620C);
        printf(D_80016220, nReg);
        return 0;
    }
}

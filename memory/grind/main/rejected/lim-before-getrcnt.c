/* REJECTED — score 7 (vs best-known 2). Computing lim before the GetRCnt
 * call hoists the lbu of D_800A36F1 ABOVE the jal; target loads it after the
 * call returns. The lim value must be computed after GetRCnt.
 * (Also: statement-splitting lim does NOT defeat the combine fold — the
 * ((x-1)<<8)+0x80 -> (x<<8)-128 transform is RTL combine.c:8196, not a
 * tree-level fold. Splitting is byte-neutral, not a fix.)
 */
    do {
        s32 lim = D_800A36F1 - 1;
        if (GetRCnt(0xF2000001u) >= (lim << 8) + 0x80) break;
        rand();
    } while (1);

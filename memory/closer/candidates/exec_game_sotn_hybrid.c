/* Closer session 2026-07-09/10 (phase 3): exec_game (_spu_gcSPU, LIBSPU
   S_M_INT) candidate. HEAD honest floor = 121 (measured this session);
   this SOTN-chassis hybrid = 30. NOT completed; HEAD body restored so its
   104 positional rules stay coherent.
   FINDINGS:
   - cc1 ICE (SIGSEGV): structured pointer-walk scan loops
     (while(p->addr==sent)p++; / for(..;..;p++); / do{p++}while) ALL crash
     cc1 inside this function; the goto-spelled scan compiles. HEAD dodged
     it the same way (goto p1_scan). index-only scan also compiles.
   - Target phase 1 = dual index+pointer walk (scan increments p AND j,
     then re-derives p = base + j), guard if(key_b>=0) + do-while framing,
     base cached in t0, cur walking in a3, key_b RELOADED at loop bottom.
   - Residuals at 30: (a) our loop opt CACHES g_spu_voice_key_b (t6) while
     target reloads per iteration; (b) strength reduction SPLITS cur into
     addr-ptr + size-ptr (addiu a3,t0,4) where target keeps ONE walking
     pointer with 0(a3)/4(a3); (c) p-derivation operand order
     (addu base,scaled vs scaled,base — try (j<<3)+(s32)base spelling);
     (d) constant li placement order in prologue.
   - Phases 2-5 not yet target-diffed at this score.
   Next probe: kill the giv split (access cur->size via *(u32*)((u8*)cur+4)?
   or reorder member uses); kill key_b invariant caching (why does loop.c
   treat it invariant here but not in SpuFree?).
*/
/* PsyQ 4.0 LIBSPU s_m_int: _spu_gcSPU -- verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp src/main/psxsdk/libspu/s_m_m.c */
void exec_game(void) {
    s32 i, j;

    if (g_spu_voice_key_b >= 0) {
        SpuMemRec *base = _spu_memList;
        SpuMemRec *cur = base;
        SpuMemRec *p;
        i = 0;
        do {
            if (cur->addr & 0x80000000) {
                j = i + 1;
                p = base + j;
            scan:
                if (p->addr == 0x2FFFFFFF) {
                    p++;
                    j++;
                    goto scan;
                }
                p = base + j;
                if ((p->addr & 0x80000000) &&
                    ((p->addr & 0x0FFFFFFF) ==
                     ((cur->addr & 0x0FFFFFFF) + cur->size))) {
                    p->addr = 0x2FFFFFFF;
                    cur->size += p->size;
                    continue;
                }
            }
            cur++;
            i++;
        } while (g_spu_voice_key_b >= i);
    }
    for (i = 0; i <= g_spu_voice_key_b; i++) {
        if (_spu_memList[i].size == 0) {
            _spu_memList[i].addr = 0x2FFFFFFF;
        }
    }
    for (i = 0; i <= g_spu_voice_key_b; i++) {
        if (_spu_memList[i].addr & 0x40000000) {
            break;
        }
        for (j = i + 1; j <= g_spu_voice_key_b; j++) {
            if (_spu_memList[j].addr & 0x40000000) {
                break;
            }
            if ((_spu_memList[j].addr & 0x0FFFFFFF) <
                (_spu_memList[i].addr & 0x0FFFFFFF)) {
                u32 swapAddr = _spu_memList[i].addr;
                u32 swapSize = _spu_memList[i].size;
                _spu_memList[i].addr = _spu_memList[j].addr;
                _spu_memList[i].size = _spu_memList[j].size;
                _spu_memList[j].addr = swapAddr;
                _spu_memList[j].size = swapSize;
            }
        }
    }
    for (i = 0; i <= g_spu_voice_key_b; i++) {
        if (_spu_memList[i].addr & 0x40000000) {
            break;
        }
        if (_spu_memList[i].addr == 0x2FFFFFFF) {
            _spu_memList[i].addr = _spu_memList[g_spu_voice_key_b].addr;
            _spu_memList[i].size = _spu_memList[g_spu_voice_key_b].size;
            g_spu_voice_key_b = i;
            break;
        }
    }
    for (i = g_spu_voice_key_b - 1; i >= 0; i--) {
        if (!(_spu_memList[i].addr & 0x80000000)) {
            break;
        }
        _spu_memList[i].addr &= 0x0FFFFFFF;
        _spu_memList[i].addr |= 0x40000000;
        _spu_memList[i].size += _spu_memList[g_spu_voice_key_b].size;
        g_spu_voice_key_b = i;
    }
}
/* kengo:HIGH  |  md_game/exec_game  |  194i */

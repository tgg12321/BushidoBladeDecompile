/* 28 game functions: the sound-bank loader and sound requests (snd_Init,
 * snd_Quit, snd_LoadCommonVab, snd_VabOpen, snd_VabFakeOpen, ...). .text
 * 0x8005B43C (ROM 0x4BC3C). Start boundary: SEAM (owner ruling 2026-10-17;
 * Codex seam #108): no direct call crosses the cut; the sound statics
 * D_800A3400..D_800A340C follow 43500.c's in .sbss, and the strings
 * D_800158B4 / D_800158CC follow 4510C.c's jump tables in .rodata. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "bb2.h"
#include "include_asm.h"
#include <psxsdk/libgte.h>

extern s32 snd_LoadVabPack(Unk8005C2A8Pack *, s16, s32);
/* Defined in 51268.c with an Unk800F1198Record *; the caller passes the
 * relocated s32 offset, so it stays unprototyped here. */
extern void eff_SetBillboardList();

/* This file's statics (.sbss, allocated per file in link order), in address
 * order (Q65). */
static s16 D_800A3400;
static s32 g_vab_sticky_sbaddr;
static s32 D_800A3408;
static s32 D_800A340C;
/* not named by any code or data: size from the gap */
static s32 D_800A3410[2];

extern s32 g_vab_vb_sbaddr[];
extern Unk8005C2A8Pack *g_vab_rec_ptr[];
extern void func_800858D0(s32);
extern s32 SsUtSetReverbType(s32);

void snd_Init(void) {
    s32 *vab_sbaddr_cursor;
    Unk8005C2A8Pack **vab_record_cursor;
    s32 vabs_cleared;
    s32 queue_index;

    vabs_cleared = 0;
    vab_sbaddr_cursor = g_vab_vb_sbaddr;
    vab_record_cursor = g_vab_rec_ptr;
    do {
        *vab_record_cursor = 0;
        *vab_sbaddr_cursor = 0;
        vab_sbaddr_cursor += 1;
        vabs_cleared += 1;
        vab_record_cursor += 1;
    } while (vabs_cleared < 0x10);
    SsInit();
    func_800858D0(0);
    SsUtReverbOff();
    SsUtSetReverbType(0);
    SsUtSetReverbDepth(0, 0);
    SsSetReservedVoice(0);
    SsSetTickMode(1);
    for (queue_index = 0; queue_index < 24; queue_index++) {
        g_snd_queue[queue_index].req = 0;
        g_snd_queue[queue_index].volr = g_snd_queue[queue_index].voll = 0x7F;
    }
    SsStart();
    D_800A3408 = 0;
    D_800A3400 = 0;
}

void func_800858D0(s32);

void snd_Quit(void) {
    s32 vabs_cleared;
    s32 *vab_sbaddr_cursor;
    Unk8005C2A8Pack **vab_record_cursor;
    func_800858D0(0);
    SsUtReverbOff();
    SsUtSetReverbType(0);
    SsUtSetReverbDepth(0, 0);
    SsEnd();
    SsQuit();
    vabs_cleared = 0;
    vab_sbaddr_cursor = g_vab_vb_sbaddr;
    vab_record_cursor = g_vab_rec_ptr;
    do {
        *vab_record_cursor = 0;
        *vab_sbaddr_cursor = 0;
        vab_sbaddr_cursor++;
        vabs_cleared++;
        vab_record_cursor++;
    } while (vabs_cleared < 0x10);
    D_800A3408 = 0;
}

void snd_StopAllVoices(void) { func_800858D0(0); }

void snd_ResetVoicesAndQueue(void) {
    s32 i;
    func_800858D0(0);
    for (i = 0; i < 24; i++) {
        g_snd_queue[i].req = 0;
        g_snd_queue[i].volr = g_snd_queue[i].voll = 0x7F;
        func_80086130((s16)i, 0, 0);
    }
}

void snd_StopAllVoicesAndCloseSlotVab(s32 a0) {
    s32 v;
    func_800858D0(0);
    v = a0 * 2 + a0 + 1;
    SsVabClose(v);
    g_vab_rec_ptr[v] = 0;
    g_vab_vb_sbaddr[v] = 0;
}

void snd_StopAllVoicesAndCloseSelectedEntryVabs(void) {
    func_800858D0(0);
    SsVabClose(2);
    g_vab_rec_ptr[2] = 0;
    g_vab_vb_sbaddr[2] = 0;
    SsVabClose(5);
    g_vab_rec_ptr[5] = 0;
    g_vab_vb_sbaddr[5] = 0;
}

void snd_CloseVab1(void) {
    SsVabClose(1);
    g_vab_rec_ptr[1] = 0;
    g_vab_vb_sbaddr[1] = 0;
}

s32 SsUtSetReverbType(s32);

void snd_CloseNonCommonVabsAndResetVoicesQueueAndReverb(void) {
    s32 vab_id;
    s32 *vab_sbaddr_cursor;
    Unk8005C2A8Pack **vab_record_cursor;
    func_800858D0(0);
    SsUtReverbOff();
    SsUtSetReverbType(0);
    SsUtSetReverbDepth(0, 0);
    vab_sbaddr_cursor = &g_vab_vb_sbaddr[1];
    vab_record_cursor = &g_vab_rec_ptr[1];
    for (vab_id = 1; vab_id < 0x10; vab_id++) {
        SsVabClose((s16)vab_id);
        *vab_record_cursor = 0;
        *vab_sbaddr_cursor = 0;
        vab_sbaddr_cursor++;
        vab_record_cursor++;
    }
    D_800A3408 = 0;
    snd_ResetVoicesAndQueue();
}

const char D_800158B4[24] = "common_vab start:%08x\n";

s32 snd_LoadCommonVab(s32 load_addr) {
    s32 file_index;
    u32 file_size;
    s32 body_offset;

    func_800858D0(0);
    printf(&D_800158B4, load_addr);
    cdrom_WaitIdle();
    file_index = cdrom_GetFileIndex(2, 1);
    cdrom_StartRead(file_index, load_addr);
    file_size = cdrom_GetFileSize(file_index);
    cdrom_WaitIdle();
    D_800A3408 = 0;
    D_800A340C = 0x1010;
    g_vab_sticky_sbaddr = 0x1010;
    body_offset =
        snd_LoadVabPack((Unk8005C2A8Pack *)load_addr, 0, load_addr + file_size);
    D_800A340C = g_vab_sticky_sbaddr;
    return body_offset;
}

void snd_StopAllVoicesAndClosePairedFileVabs(void) {
    func_800858D0(0);
    SsVabClose(8);
    g_vab_rec_ptr[8] = 0;
    g_vab_vb_sbaddr[8] = 0;
    SsVabClose(4);
    g_vab_rec_ptr[4] = 0;
    g_vab_vb_sbaddr[4] = 0;
}

s32 snd_ClosePairedFileVabsThenLoadFromConsecutiveFiles(s32 load_addr) {
    s32 first_file_index;
    s32 size;
    s32 first_body_offset;
    s32 second_file_index;

    snd_StopAllVoicesAndClosePairedFileVabs();
    func_800858D0(0);
    first_file_index = cdrom_GetFileIndex(2, 0x5D);
    cdrom_WaitIdle();
    cdrom_StartRead(first_file_index, load_addr);
    size = cdrom_GetFileSize(first_file_index);
    cdrom_WaitIdle();
    first_body_offset =
        snd_LoadVabPack((Unk8005C2A8Pack *)load_addr, 8, load_addr + size);
    second_file_index = cdrom_GetFileIndex(2, 0x5E);
    cdrom_WaitIdle();
    cdrom_StartRead(second_file_index, load_addr + first_body_offset);
    size = cdrom_GetFileSize(second_file_index) + first_body_offset;
    cdrom_WaitIdle();
    return snd_LoadVabPack((Unk8005C2A8Pack *)(load_addr + first_body_offset),
                           4, load_addr + size) +
           first_body_offset;
}

s32 snd_VabFakeOpen(s32, s16);

void snd_VabFakeOpen8And4(s32 rebase_delta) {
    snd_VabFakeOpen(rebase_delta, 8);
    snd_VabFakeOpen(rebase_delta, 4);
}

void snd_StopAllVoicesAndCloseSingleFileVab(void) {
    func_800858D0(0);
    SsVabClose(9);
    g_vab_rec_ptr[9] = 0;
    g_vab_vb_sbaddr[9] = 0;
}

s32 snd_CloseThenReadSingleFileAndLoadVab(s32 load_addr) {
    s32 file_index;
    s32 size;
    snd_StopAllVoicesAndCloseSingleFileVab();
    file_index = cdrom_GetFileIndex(2, 8);
    cdrom_WaitIdle();
    cdrom_StartRead(file_index, load_addr);
    size = cdrom_GetFileSize(file_index);
    cdrom_WaitIdle();
    return snd_LoadVabPack((Unk8005C2A8Pack *)load_addr, 9, load_addr + size);
}

void snd_VabFakeOpen9(s32 rebase_delta) { snd_VabFakeOpen(rebase_delta, 9); }

typedef struct {
    s32 off;
    s32 size;
} VabEnt;

typedef struct {
    VabEnt ent[3];
    s32 len[3];
} VabLoad;

extern u8 g_vab_id_list[];

s32 snd_ReloadListedVabsFromFileAndInstallBillboards(
    s32 load_addr, s32 arg1, s32 arg2, s32 arg3) {
    VabLoad vab_load;
    u8 *p;
    s32 base;
    s32 file_index;
    s32 size;
    u8 vab_count;
    s32 vab_list_index;
    u32 copy_byte_index;

    p = (u8 *)load_addr;
    func_800858D0(0);
    for (vab_list_index = 0; vab_list_index < 3; vab_list_index++) {
        SsVabClose(g_vab_id_list[vab_list_index]);
        g_vab_rec_ptr[g_vab_id_list[vab_list_index]] = 0;
        g_vab_vb_sbaddr[g_vab_id_list[vab_list_index]] = 0;
    }
    file_index = cdrom_GetFileIndex(2, arg1 + 9);
    cdrom_WaitIdle();
    cdrom_StartRead(file_index, (s32)p);
    size = cdrom_GetFileSize(file_index);
    cdrom_WaitIdle();
    ((s32 *)p)[12] += (s32)p;
    eff_SetBillboardList(((s32 *)p)[12]);
    vab_count = 3;
    base = (s32)p;
    if (arg2 == arg3) {
        vab_count = 2;
    }
    vab_load.ent[0].off = ((VabEnt *)p)[0].off;
    vab_load.ent[0].size = ((VabEnt *)p)[0].size;
    vab_load.ent[1].off = ((VabEnt *)p)[arg2 + 1].off;
    vab_load.ent[1].size = ((VabEnt *)p)[arg2 + 1].size;
    if (vab_count == 3) {
        vab_load.ent[2].off = ((VabEnt *)p)[arg3 + 1].off;
        vab_load.ent[2].size = ((VabEnt *)p)[arg3 + 1].size;
    }
    for (vab_list_index = 0; vab_list_index < vab_count; vab_list_index++) {
        vab_load.ent[vab_list_index].off += (s32)p;
        vab_load.len[vab_list_index] =
            snd_LoadVabPack((Unk8005C2A8Pack *)vab_load.ent[vab_list_index].off,
                            g_vab_id_list[vab_list_index], (s32)p + size);
    }
    for (vab_list_index = 0; vab_list_index < vab_count; vab_list_index++) {
        for (copy_byte_index = 0;
             copy_byte_index < (u32)vab_load.len[vab_list_index];
             copy_byte_index++) {
            p[copy_byte_index] =
                ((u8 *)vab_load.ent[vab_list_index].off)[copy_byte_index];
        }
        snd_VabFakeOpen((s32)p - vab_load.ent[vab_list_index].off,
                        g_vab_id_list[vab_list_index]);
        vab_load.ent[vab_list_index].off = (s32)p;
        p += vab_load.len[vab_list_index];
    }
    if (vab_count == 2) {
        g_vab_rec_ptr[6] = g_vab_rec_ptr[3];
    }
    return (s32)p - base;
}

void snd_RebaseListedVabs(s32 rebase_delta) {
    u8 vab_count;
    s32 vab_list_index;
    func_800858D0(0);
    vab_count = (g_vab_rec_ptr[6] == g_vab_rec_ptr[3]) ? 2 : 3;
    vab_list_index = 0;
    if (vab_count != 0) {
        do {
            snd_VabFakeOpen(rebase_delta, g_vab_id_list[vab_list_index & 0xFF]);
            vab_list_index += 1;
        } while ((u32)(vab_list_index & 0xFF) < (u32)vab_count);
    }
    if (vab_count == 2) {
        g_vab_rec_ptr[6] = g_vab_rec_ptr[3];
    }
}

void snd_CloseListedVabs(void) {
    s32 i;
    for (i = 0; i < 3; i++) {
        SsVabClose(g_vab_id_list[i]);
        g_vab_rec_ptr[g_vab_id_list[i]] = 0;
        g_vab_vb_sbaddr[g_vab_id_list[i]] = 0;
    }
}

extern s16 D_8009AD1C[][2];

s32 snd_SetReverb(s32 arg0) {
    s32 result;
    s16 *p;
    s16 temp_a0;
    s16 *base;
    s32 var_4;
    func_800858D0(0);
    base = &D_8009AD1C[0][0];
    p = base + arg0 * 2;
    var_4 = arg0 << 1;
    if (*p >= 0) {
        SsUtReverbOff();
        SsUtSetReverbType(0);
        SsUtSetReverbDepth(0, 0);
        result = SsUtSetReverbType(*p);
        SpuClearReverbWorkArea(*p);
        temp_a0 = var_4 + 1;
        SsUtSetReverbDepth(temp_a0, temp_a0);
        SsUtReverbOn();
    } else {
        result = -1;
    }
    return (s16)result;
}

void snd_StopVoicesAndReverb(void) {
    func_800858D0(0);
    SsUtReverbOff();
    SsUtSetReverbType(0);
    SsUtSetReverbDepth(0, 0);
}

extern s32 SsVabFakeHead();

s32 snd_MoveVabBody(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    func_800858D0(0);
    SsVabClose((s16)arg1);
    SpuSetTransferStartAddr(arg3);
    SpuRead(arg0, g_vab_rec_ptr[arg1]->unk_0C);
    SpuIsTransferCompleted(1);
    SpuSetTransferStartAddr(arg2);
    SpuWrite(arg0, g_vab_rec_ptr[arg1]->unk_0C);
    SpuIsTransferCompleted(1);
    SsVabFakeHead(g_vab_rec_ptr[arg1]->unk_04, (s16)arg1, arg2);
    SsVabFakeBody((s16)arg1);
    g_vab_vb_sbaddr[arg1] = arg2;
    return arg2 + g_vab_rec_ptr[arg1]->unk_0C;
}

/* SPU VAB compaction: sorts the resident VAB slots 1..15 by SPU address, then
 * walks them from the end of slot 0, moving down (snd_MoveVabBody) the first
 * slot that is not already contiguous and every slot after it. `arg0` is never
 * read. `addr` is assigned inside the first (otherwise empty) loop: the target
 * computes it after the loop's `count > 0` guard. */
s32 snd_CompactVabBodies(s16 arg0, s32 base) {
    s16 var_0[16];
    s16 count;
    u16 mask;
    u32 min;
    s16 minidx;
    s16 i;
    s16 j;
    s16 k;
    s32 addr;

    count = 0;
    mask = 0;
    for (;;) {
        min = 0x7FFFF;
        minidx = -1;
        for (i = 1; i < 16; i++) {
            if (!((mask >> i) & 1) && g_vab_vb_sbaddr[i] != 0 &&
                g_vab_vb_sbaddr[i] < min) {
                min = g_vab_vb_sbaddr[i];
                minidx = i;
            }
        }
        if (minidx == -1) {
            break;
        }
        var_0[count++] = minidx;
        mask += 1 << minidx;
    }
    for (j = 0; j < count; j++) {
        addr = g_vab_vb_sbaddr[0] + g_vab_rec_ptr[0]->unk_0C;
    }
    for (j = 0; j < count; j++) {
        if (g_vab_vb_sbaddr[var_0[j]] == addr) {
            addr += g_vab_rec_ptr[var_0[j]]->unk_0C;
        } else {
            for (k = j; k < count; k++) {
                addr = snd_MoveVabBody(
                    base, var_0[k], addr, g_vab_vb_sbaddr[var_0[k]]);
            }
            return 0;
        }
    }
    return 0;
}

/* snd_LoadVabPack - load a VAB: close whatever occupies slot `arg1`,
 * recompute the sticky SPU address from the resident banks (compacting them
 * with snd_CompactVabBodies for arg1 != 0), turn the pack's first three
 * offsets (Unk8005C2A8Pack unk_00..unk_08) into addresses, open it with
 * snd_VabOpen and record its slot and SPU address. Returns unk_08 less the
 * pack's address (the third offset, unrelocated), or 0 on failure. snd_VabOpen
 * returns s32, as this call site's bytes say (the sll/sra in its body comes
 * from an explicit cast). */

extern s32 snd_VabOpen(Unk8005C2A8Pack *, s16);

const char D_800158CC[20] = "vab id:%d mistake\n";

s32 snd_LoadVabPack(Unk8005C2A8Pack *arg0, s16 arg1, s32 arg2) {
    s16 i;
    s16 id;

    if (GetWorkingRecord()->unk14_0 == 3 && arg1 == 5) {
        return 0;
    }
    func_800858D0(0);
    if (g_vab_rec_ptr[arg1] != 0) {
        SsVabClose(arg1);
        g_vab_rec_ptr[arg1] = 0;
        g_vab_vb_sbaddr[arg1] = 0;
    }
    if (arg1 != 0) {
        g_vab_sticky_sbaddr = g_vab_vb_sbaddr[0];
        for (i = 0; i < 16; i++) {
            if (g_vab_rec_ptr[i] != 0) {
                g_vab_sticky_sbaddr += g_vab_rec_ptr[i]->unk_0C;
            }
        }
    }
    D_800A3408 = g_vab_sticky_sbaddr - D_800A340C;
    if (arg1 != 0) {
        snd_CompactVabBodies(arg1, arg2);
    }
    arg0->unk_00 += (s32)arg0;
    arg0->unk_04 += (s32)arg0;
    arg0->unk_08 += (s32)arg0;
    id = snd_VabOpen(arg0, arg1);
    SsVabTransCompleted(1);
    if (id != -1) {
        g_vab_rec_ptr[id] = arg0;
        D_800A3408 += arg0->unk_0C;
        g_vab_sticky_sbaddr = D_800A340C + D_800A3408;
        g_vab_vb_sbaddr[arg1] = SsUtGetVBaddrInSB(arg1);
        return arg0->unk_08 - (s32)arg0;
    }
    printf(D_800158CC, arg1);
    return 0;
}

/* snd_VabFakeOpen tail: the s16 result carrier and single trailing return give
 * the target's CFG (li -1 in its own block, shared sign-extend join); a direct
 * return or an s32 carrier does not match
 * (proven-spelling-class-reconstruction). */
s32 snd_VabFakeOpen(s32 arg0, s16 arg1) {
    s32 idx;
    s16 ret;
    func_800858D0(0);
    idx = arg1;
    if (g_vab_rec_ptr[idx] != 0) {
        g_vab_rec_ptr[idx] =
            (Unk8005C2A8Pack *)((u8 *)g_vab_rec_ptr[idx] + arg0);
        g_vab_rec_ptr[idx]->unk_00 += arg0;
        g_vab_rec_ptr[idx]->unk_04 += arg0;
        SsVabClose(idx);
        ret = SsVabFakeHead(
            g_vab_rec_ptr[idx]->unk_04, idx, g_vab_vb_sbaddr[idx]);
        if (ret != idx) {
            return ret;
        }
        ret = SsVabFakeBody(ret);
    } else {
        ret = -1;
    }
    return ret;
}

void SsVabOpenHeadSticky(s32, s16, s32);
s32 SsVabTransBody(s32, s16);

s32 snd_VabOpen(Unk8005C2A8Pack *a0, s16 a1) {
    SsVabClose(a1);
    SsVabOpenHeadSticky(a0->unk_04, a1, g_vab_sticky_sbaddr);
    ((VabHdr *)a0->unk_04)->id = a1;
    return (s16)SsVabTransBody(a0->unk_08, a1);
}

void snd_StopVoicesSetMaxMasterVolumeStereoAndClearAutoKeyOffMode(void) {
    SsSetMVol(0x7F, 0x7F);
    func_800858D0(0);
    SsSetStereo();
    SsSetAutoKeyOffMode(0);
}

extern u16 D_8009AA70[][2];

/* FAKE: declared int though no value is returned (plausibly an implicit-int
 * function in the original): with void, its call in snd_QueuePositionalSound
 * sets no $v0, so reload re-places the var_7 product in $v0 there (score 2). */
s32 snd_QueueSound(s32 a0, s32 a1, s32 a2) {
    s16 i = 0;
    u16 *var_1 = D_8009AA70[a0];
    do {
        if (g_snd_queue[i].req == 0) {
            g_snd_queue[i].req = var_1;
            g_snd_queue[i].volr = a1;
            g_snd_queue[i].voll = a2;
            return;
        }
        i++;
    } while (i < 0x18);
}

/* Per-frame sound-request flush: walk the 24-entry pending-sound pool, and for
 * every entry whose VAB is loaded, find the first free SPU voice at or after
 * the running `next` cursor and key the note on with the entry's
 * stored volumes. Each pool slot is cleared as it is visited.
 */
void snd_FlushSoundQueue(void) {

    u8 var_0[24];
    s16 i;
    s16 var_2;
    s16 next;
    u16 var_4;
    u16 *p;
    u32 *var_6;

    SpuGetAllKeysStatus(var_0);
    next = 0;
    for (i = 0; i < 0x18; i++) {
        p = g_snd_queue[i].req;
        if (p != 0 && (s32)g_vab_rec_ptr[*p] < 0) {
            var_2 = next;
            for (; var_2 < 0x18; var_2++) {
                if (SpuGetKeyStatus(1 << var_2) != 1) {
                    var_4 = *p;
                    if (var_4 == 6 && g_vab_rec_ptr[6] == g_vab_rec_ptr[3]) {
                        var_4 = 3;
                    }
                    var_6 = &((u32 *)g_vab_rec_ptr[var_4]->unk_00)[p[1]];
                    SsUtKeyOnV((s16)var_2, (s16)var_4, (s16)(*var_6 & 0x7F),
                               (s16)((*var_6 >> 7) & 0xF),
                               (s16)((*var_6 >> 11) & 0x7F),
                               (s16)((*var_6 >> 18) & 0x7F),
                               g_snd_queue[i].voll, g_snd_queue[i].volr);
                    next = var_2 + 1;
                    break;
                }
            }
        }
        g_snd_queue[i].req = 0;
    }
}

/* .data 0x8009AD1C..0x8009ADB4, this file's whole .data (the data-only
 * d_8A588.c's g_vab_id_list precedes): snd_SetReverb's
 * rows, picked by its argument (38, the size up to 4D0A8.c's .data); [0] is
 * the reverb type it sets, or < 0 to leave the reverb as is; [1] has no
 * reader. */
s16 D_8009AD1C[38][2] = {
    {2, 2},  {1, 2},  {1, 2},  {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0},
    {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0},
    {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0},
    {4, 32}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 8}, {-1, 0}, {1, 8},  {-1, 2},
    {1, 2},  {1, 2},  {-1, 2}, {1, 2},  {1, 2},  {-1, 0},
};

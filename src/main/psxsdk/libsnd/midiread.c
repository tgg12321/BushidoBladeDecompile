/* PsyQ LIBSND MIDIREAD: _SsSeqPlay, _SsSeqGetEof and func_80084CC0
 * (_SsGetSeqData). .text 0x80084974..0x80085064, the whole region between PLAY
 * and MIDITIME. BB2 links an interim LIBSND build (between PsyQ 4.0 and 4.1).
 * Module start (Q109): PLAY's one REL26 is jal _SsSeqPlay (0x80084974), and
 * MIDIREAD's XDEFs sit at +0x0 / +0x108 / +0x34C, the offsets of the three
 * functions here. */
#include "common.h"
#include "libsnd_i.h"

/* sotn-decomp's seqread.c _SsSeqPlay over BB2's SeqStruct (SOTN's
 * delta_value / unk70 / unk6E are BB2's delta_value / unk54 / unk52). */
void _SsSeqPlay(s16 a0, s16 a1) {
    struct SeqStruct *score = &_ss_score[a0][a1];
    s32 var_s0;

    if (score->delta_value - score->unk54 > 0) {
        if (score->unk52 > 0) {
            score->unk52--;
        } else if (score->unk52 == 0) {
            score->unk52 = score->unk54;
            score->delta_value--;
        } else {
            score->delta_value -= score->unk54;
        }
    } else if (score->delta_value <= score->unk54) {
        var_s0 = score->delta_value;
        do {
            do {
                func_80084CC0(a0, a1);
            } while (score->delta_value == 0);
            var_s0 += score->delta_value;
        } while (var_s0 < score->unk54);
        score->delta_value = var_s0 - score->unk54;
    }
}

void _SsSeqGetEof(s16 a0, s16 a1) {
    struct SeqStruct *score = &_ss_score[a0][a1];

    score->unk21++;
    if (score->unk20 == 0) {
        score->unk88 = 0;
        score->unk1C = 0;
        score->delta_value = 0;
        if (_ss_score[a0][a1].unk98 & 0x400) {
            score->read_pos = score->unk0C;
        } else {
            score->read_pos = score->next_sep_pos;
        }
        return;
    }

    if (score->unk21 < score->unk20) {
        score->unk88 = 0;
        score->unk1C = 0;
        score->delta_value = 0;
        if (_ss_score[a0][a1].unk98 & 0x400) {
            score->read_pos = score->unk0C;
            score->loop_pos = score->unk0C;
        } else {
            score->read_pos = score->next_sep_pos;
            score->loop_pos = score->next_sep_pos;
        }
        return;
    }

    _ss_score[a0][a1].unk98 &= ~1;
    _ss_score[a0][a1].unk98 &= ~8;
    _ss_score[a0][a1].unk98 &= ~2;
    _ss_score[a0][a1].unk98 |= 0x200;
    _ss_score[a0][a1].unk98 |= 4;
    score->unk14 = 0;

    if (_ss_score[a0][a1].unk98 & 0x400) {
        score->loop_pos = score->unk0C;
    } else {
        score->loop_pos = score->next_sep_pos;
    }

    if (score->unk22 != 0xFF) {
        score->unk14 = 0;
        _SsSndNextSep(score->unk22, score->unk23);
        _SsVmSeqKeyOff((s16)(a0 | (a1 << 8)));
    }
    _SsVmSeqKeyOff((s16)(a0 | (a1 << 8)));
    score->delta_value = score->unk54;
}

s32 func_80084CC0(s16 a0, s16 a1) {
    /* Each status-byte arm reads its operands through its own block-local
     * pointer, except `velocity`: one function-level local written in both
     * 0x90 arms and read only as noteon's 4th argument, the reuse SOTN's
     * _SsGetSeqData makes of var_s3 (Q51, no-new-park-categories). */
    struct SeqStruct *state;
    s32 cmd;
    u8 *ptr;
    u8 b;
    u8 prev;
    /* SOTN: src/main/psxsdk/libsnd/seqread.c:57 @aa53500 */
    /* FAKE: Q51 reused variable (Q53); a separate local per 0x90 arm changes
       the register allocation order. */
    u8 velocity;
    s32 ret;
    state = &_ss_score[a0][a1];
    ptr = state->read_pos;
    state->read_pos = ptr + 1;
    b = ptr[0];
    ret = 0;
    if ((_ss_score[a0][a1].unk98 & 0x401) == 0x401) {
        if (state->read_pos == state->unk10 + 1) {
            /* Every _SsSeqGetEof call passes a 3rd argument (the target's two
               calls load $a2; as SOTN's _SsGetMetaEvent(s16, s16, u8) call
               does), but BB2's _SsSeqGetEof reads only two. */
            ((void (*)(s16, s16, u8))_SsSeqGetEof)(a0, a1, state->unk10[1]);
            return -1;
        }
    }
    if (b & 0x80) {
        state->channel = b & 0xF;
        cmd = b & 0xF0;
        switch (cmd) {
        case 0x90: {
            u8 *cmd_ptr;
            u32 note;
            state->unk16 = 0x90;
            cmd_ptr = state->read_pos;
            state->read_pos = cmd_ptr + 1;
            note = cmd_ptr[0];
            state->read_pos = cmd_ptr + 2;
            velocity = cmd_ptr[1];
            state->delta_value = _SsReadDeltaValue(a0, a1);
            D_800F3340.noteon(a0, a1, note, velocity);
            goto end;
        }

        case 0xB0: {
            u8 *cmd_ptr;
            u8 databyte;
            state->unk16 = 0xB0;
            cmd_ptr = state->read_pos;
            state->read_pos = cmd_ptr + 1;
            databyte = cmd_ptr[0];
            D_800F3340.control[0](a0, a1, databyte);
            goto end;
        }

        case 0xC0: {
            u8 *cmd_ptr;
            u8 databyte;
            state->unk16 = 0xC0;
            cmd_ptr = state->read_pos;
            state->read_pos = cmd_ptr + 1;
            databyte = cmd_ptr[0];
            D_800F3340.programchange(a0, a1, databyte);
            goto end;
        }

        case 0xE0:
            state->unk16 = 0xE0;
            state->read_pos++;
            D_800F3340.pitchbend(a0, a1);
            goto end;

        case 0xF0: {
            u8 *cmd_ptr;
            u8 databyte;
            state->unk16 = 0xFF;
            cmd_ptr = state->read_pos;
            state->read_pos = cmd_ptr + 1;
            databyte = cmd_ptr[0];
            if (databyte == 0x2F) {
                ret = 1;
                ((void (*)(s16, s16, u8))_SsSeqGetEof)(a0, a1, 0x2F);
                goto end;
            }
            D_800F3340.metaevent(a0, a1, databyte);
            goto end;
        }

        default:
            goto end;
        }

    } else {
        prev = state->unk16;
        switch (prev) {
        case 0x90: {
            u8 *cmd_ptr = state->read_pos;
            state->read_pos = cmd_ptr + 1;
            velocity = cmd_ptr[0];
            state->delta_value = _SsReadDeltaValue(a0, a1);
            D_800F3340.noteon(a0, a1, b, velocity);
            goto end;
        }

        case 0xB0:
            D_800F3340.control[0](a0, a1, b);
            goto end;

        case 0xC0:
            D_800F3340.programchange(a0, a1, b);
            goto end;

        case 0xE0:
            D_800F3340.pitchbend(a0, a1);
            goto end;

        case 0xFF:
            if (b == 0x2F) {
                ret = 1;
                ((void (*)(s16, s16, u8))_SsSeqGetEof)(a0, a1, 0x2F);
                goto end;
            }
            D_800F3340.metaevent(a0, a1, b);

        default:
            goto end;
        }
    }
end:
    return ret;
}

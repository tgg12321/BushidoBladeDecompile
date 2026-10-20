"""wave47 rows: the effect-command core (eff_) and the motion-stream decoder (motion_).
Cites at main 032e23f8c; refresh after the 51268.c / 3AB48.c TU split lands."""

FUNCS = [
    ("0x80060A68", "eff_StartCommand", "RENAME",
     "BODY 51268.c:92-114 (void): for the command block D_800A3468 points at (idx = its low "
     "halfword unk00.h) zeroes D_800F10D0[idx], copies the block's three source words unk0C / "
     "halfwords unk10 into unk20 / unk18 and points D_800A347C / D_800A3478 at the copies, calls "
     "the handler D_8009BA7C[D_8009BA60[idx] + D_800F10D0[idx]] (the command's first handler, "
     "phase 0), stores its result through unk14 (a D_800F1150 status byte) and sets D_800A32BC = "
     "0xA when bit 21 of unk00.w is set. D_800F10D0[idx] is the command's phase: handlers advance "
     "it (e.g. func_80064E90 sets D_800F10D0[4] = 1, 51268.c:1608; raw scan tmp/p3w47/scan.txt: "
     "stores from 39 functions). Callers: the 18 wrappers func_800611A4 .. func_80061EC0 "
     "(51268.c:310-688), each after filling D_800F116C (command word 0x..XXXX with idx in the low "
     "halfword, sources, status slot). WHY: starts one effect command at phase 0; 'eff_' as "
     "eff_Init (6CF8.c:248), whose loader func_80060CB8 calls func_80060C60, and the module's "
     "'eff prim over' message (51268.c:262). COUNTER-EVIDENCE: what each of the 28 commands draws "
     "is not established; the name claims only the start of a command."),
    ("0x80060B70", "eff_StepCommand", "RENAME",
     "BODY 51268.c:116-140 (void): copies the block's s16 triple unk04 and s32 triple unk08 (the "
     "two arrays func_80061064 received, through func_80060E38's seed) to D_800A346C / "
     "D_800A3470, passes them to func_80061FAC with D_800A3474 (a MATRIX), then calls the handler "
     "D_8009BA7C[D_8009BA60[idx] + D_800F10D0[idx]] for the command's current phase and stores the "
     "result through unk14. Caller: func_80061064 only (51268.c:273), once per frame for each "
     "command whose status byte is non-zero. WHY: runs one frame of a running effect command (its "
     "current phase's handler). COUNTER-EVIDENCE: none."),
    ("0x80060C60", "eff_ResetCommands", "RENAME",
     "BODY 51268.c:144-162 (void): zeroes the 0x1C phases D_800F10D0[] and status bytes "
     "D_800F1150[], then D_800A345C[0..1], D_800A3454[0..1], D_800A344C[0..1], D_800A3460, "
     "D_800A3444, D_800A3448. Callers: func_80060CB8 (51268.c:199, eff_Init's loader) and "
     "func_80061178 (51268.c:291). WHY: stops every effect command (status 0 is 'not running': "
     "func_80061064 steps only non-zero ones). COUNTER-EVIDENCE: the seven other statics it "
     "clears are unnamed; the name covers the command tables, its main work."),
    ("0x80061064", "eff_UpdateCommands", "RENAME",
     "BODY 51268.c:264-288 (s16 *a0, s32 *a1): func_80060E38(a0, a1) seeds the scratchpad block "
     "and pointers; for i in 0..0x1B with D_800F1150[i] != 0 sets the block's idx and calls "
     "func_80060B70; then counts D_800A32BC down (>= 2: func_80041E10(&D_800F1140, D_800A3464); "
     "1: func_800421A4, then 0) and prints 'eff prim over :%d' when D_800A37D4 - D_800A3720 "
     "reaches 0x1C2 primitives. Callers: 9F9C.c:1974 / :2007 (after camera_CalcEye, passing the "
     "camera record's angle / position arrays) and 2B344.c:1088. WHY: the per-frame update of "
     "every running effect command. COUNTER-EVIDENCE: the D_800A32BC countdown (set by "
     "func_80060A68 on bit 21) is a side task whose meaning is unnamed."),
    ("0x8001979C", "motion_InitStream", "RENAME",
     "BODY 9F9C.c:64-84 (s32 arg0, u32 *ptr): D_800F1B18[arg0] (Unk800F1B18Rec, include/"
     "game.h:1351-1362: 'one motion-stream decoder record') gets stream = ptr, 63 12-bit initial "
     "channel values into pose[3..] and 63 2-bit channel codes into code[3..] (GETBITS), its four "
     "cache slots' frame = -2 (empty) and ctr = 0. Callers: func_80021210 (9F9C.c:3108-3113), "
     "2B344.c:1100-1102, 3AB48.c:784. WHY: sets up decoder slot arg0 on a motion bitstream; "
     "'motion_' as the MotionFrame type (game.h:1317-1335) its sibling decodes into. "
     "COUNTER-EVIDENCE: none."),
    ("0x800198D0", "motion_DecodeFrame", "RENAME",
     "BODY 9F9C.c:99-329 (s32 obj, s32 arg1, MotionFrame *out, u16 *work): decodes entry arg1 of "
     "D_800F1B18[obj]'s stream into the 0x84-byte MotionFrame out, from its group of 8 (offset "
     "table at stream + 0x70) or from a cached predecessor, keeping the result in one of four "
     "cache slots (the function's own comment, 9F9C.c:88-98). Callers: 9F9C.c:4704-4705 (a "
     "character record's two frames), 28708.c:1389-1390, 3AB48.c:850 (then the frame's values "
     "scale a position). WHY: decodes one motion frame; 'Frame' as its MotionFrame output. "
     "COUNTER-EVIDENCE: none."),
    ("0x80021210", "motion_InitStreams", "RENAME",
     "BODY 9F9C.c:3107-3115 (void): func_8001979C(0, D_80102760.unk_10); func_8001979C(1, "
     "D_801027B0[0].unk_10) when D_800A38C4[0] != 0; func_8001979C(2, D_801027B0[1].unk_10) when "
     "D_800A38C4[1] != 0. Caller: 9F9C.c:1844 (before func_80021280(0) / (1)). WHY: sets up the "
     "three decoder slots on their streams (2B344.c:1100-1102 does the same three calls inline, "
     "unconditionally). COUNTER-EVIDENCE: what D_80102760 / D_801027B0 hold beyond the stream "
     "pointer at +0x10, and D_800A38C4's meaning, are not named here."),
]

DATA = [
    ("0x800F10D0", "g_eff_command_phase",
     "RENAME. s32 [0x1C] (extern include/bb2.h:584). ACCESSES (raw scan tmp/p3w47/scan.txt, 42 "
     "from 39 functions): func_80060A68 W 0 and R (handler index), func_80060B70 R, func_80060C60 "
     "W 0 (all 0x1C), and stores from the command handlers (e.g. func_80064E90 D_800F10D0[4] = 1, "
     "51268.c:1608). WHY: each effect command's phase, added to its handler base D_8009BA60[idx] "
     "to pick the handler. COUNTER-EVIDENCE: none."),
    ("0x800F1150", "g_eff_command_status",
     "RENAME. u8 [0x1C] (extern 51268.c:142). ACCESSES (raw scan, 84 from 20 functions): "
     "func_80060C60 W 0; func_80061064 R (step only non-zero) and its address as the block's "
     "result slot unk14; the wrappers point unk14 at their slot and some test / clear pairs of "
     "slots (func_80061250: [9] / [10]). WHY: the per-command status byte the handlers' result is "
     "stored to; 0 = not running. COUNTER-EVIDENCE: non-zero values beyond 'running' are not "
     "decoded."),
    ("0x800F116C", "g_eff_command_block",
     "RENAME. Unk1F800000Unk00 (extern 51268.c:296). ACCESSES (raw scan, 72 from 18 functions): "
     "the wrappers func_800611A4 .. func_80061EC0 point D_800A3468 at it and fill its command "
     "word, sources and status slot before func_80060A68. WHY: the command block a new effect "
     "command is started from (the per-frame steps use the scratchpad block instead, "
     "func_80060E38). COUNTER-EVIDENCE: none."),
    ("0x800F1B18", "g_motion_decoders",
     "RENAME. Unk800F1B18Rec [3] (extern include/bb2.h:588). ACCESSES (raw scan, 33): "
     "func_8001979C and func_800198D0 only. WHY: the three motion-stream decoder records "
     "(game.h:1351). COUNTER-EVIDENCE: none."),
]

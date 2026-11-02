"""wave120 rows (func-second-pass-29): func_80037D14, func_8001C624, func_800164F8 (3 RENAME).
Research input tmp/codex/func-second-pass-29-research.md (untrusted); every code claim re-derived at HEAD f613d4089.
The stubs func_80044E64 / func_80044E6C / func_800457D4 are NO_NAME (not rows here; see notes.txt).

Re-derived code facts:
- func_80037D14 (src/main/28514.c:18-101): 5-state machine on g_memcard_info_load_state (.sdata :105). Only use: call 28708.c:464 in
  memcard_Update (def :462), `D_800A31F8 = func_80037D14(0, 0)` while g_memcard_request_state < 2; jal asm/funcs/memcard_Update.s:8. extern
  include/bb2.h:797. Consumer memcard_StepRequest (28708.c:282) reads D_800A31F8 at :293 (-1 retry :294, -2 format/0xA branch :299/:317-326, 1..2 ok).
- func_8001C624 (src/main/9F9C.c:1279-1325): only use call 9F9C.c:1425 in fight_UpdateDefeatDelayAndModeProgress (def :1355), switch case 3 (:1399),
  when g_fight_start_position_pair_index changed (:1408-1424); jal asm/funcs/fight_UpdateDefeatDelayAndModeProgress.s:198.
- func_800164F8 (src/main/6CF8.c:38-44; asm/funcs/func_800164F8.s: break 1, 10000 iterations, jr ra): 3 jal sites = 6CF8.c:239 (sys_Panic),
  6CF8.c:498 (main), 35000.c:268 (ndata_ReadChecked); data word D_800A2674[0] (psxsdk/libsn/73E98.c:31) is SNMAIN's __text start.
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply 2026-11-01 (lane naming-c): PASS 3/3 (memcard_StepCardInfoAndLoad, fight_ResetFighter0ToStartPosition, sys_PollHost10000Times; primary over fallback); NO_NAME stubs 0x80044E64/0x80044E6C/0x800457D4 agreed"
NOTES = {}
DROPPED = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-c")

_SRC = {}
for _f in sorted(glob.glob(os.path.join(WT, "src", "main", "*.c"))):
    _SRC[os.path.basename(_f)] = open(_f, encoding="utf-8", errors="replace").read().split("\n")


def B(fn, f):
    """src/main/<file>:<first>-<last> of the C definition of fn."""
    lines = _SRC[f]
    for k, l in enumerate(lines):
        if not re.match(r"^[A-Za-z_][\w \*]*\b%s\(" % fn, l) or l.startswith("extern"):
            continue
        j = k
        while j < len(lines) and lines[j] != "}" and not (j == k and l.rstrip().endswith("}")):
            j += 1
        return f"src/main/{f}:{k + 1}-{j + 1}" if j != k else f"src/main/{f}:{k + 1}"
    raise SystemExit(f"no C definition of {fn}")


_CARD = (
    f"BODY {B('func_80037D14', '28514.c')}: a 5-state machine over g_memcard_info_load_state (28514.c:105), channel = (port << 4) + card. State 0 issues "
    "_card_info(channel), sets state 1, zeroes g_memcard_poll_count and the return latch D_800A3890. State 1 polls memcard_PollSwEventsTimeout "
    "(memcard.c:67; 0 none, 1 ioe, 2 err, 3 timeout, 4 new card): 1 stores pending 1 and goes to state 4 if g_memcard_card_loaded else state 2; 3 pending -1, state 4; "
    "4 pending 2, memcard_AckHwEvents, _card_clear, memcard_WaitHwEvent, state 2; 2/other pending -3, state 4. State 2: memcard_AckSwEvents, _card_load(channel), "
    "state 3, poll count zeroed. State 3 polls again, goes to state 4: 1 sets g_memcard_card_loaded = 1 (pending kept), 3 -> -1, 4 -> -2, 2/other -3. State 4: "
    "D_800A3890 = pending, state 0. Returns D_800A3890 (0 while running, the result on the completion step). "
    "USES (all): one call, src/main/28708.c:464 in memcard_Update (def :462), `D_800A31F8 = func_80037D14(0, 0)` while g_memcard_request_state < 2; "
    "jal asm/funcs/memcard_Update.s:8 (0x8003874C); extern include/bb2.h:797; no data word holds the address. Consumer memcard_StepRequest (28708.c:282) reads "
    "D_800A31F8 at :293 (1..2 ok, -1 retry :294, -2 format/0xA branch :299, :317-326). "
    "WHY: every write, call and return is the card-info then card-load sequence (_card_info / _card_load / _card_clear, include/psxsdk/libcard.h), driven one "
    "state per call like its consumer memcard_StepRequest; the callees are memcard_ (memcard.c) and libcard, the globals g_memcard_*, the only caller memcard_Update. "
    "'CardInfoAndLoad' tells it from the file-request machine. _card_clear runs only on the new-card event inside the same sequence. "
    "COUNTER-EVIDENCE: a wider name (memcard_StepCardDetect) would claim a role the -2/format use only hints at; none of the uses is non-memcard. Census: no row; "
    "named_syms.txt:1049 and symbol_addrs.txt data comments (:924, :940, :1225, :1886) describe it, none is evidence. "
    "Trace: docs/naming/runtime/matrix/facts.tsv:1759 reach (hit in 135 of 135 targeted captures), corroborates only that it runs; no role taken."
)

_RESET0 = (
    f"BODY {B('func_8001C624', '9F9C.c')} on D_80101EC8[0]: base_pos = fight_GetStartPosition(0, pair = g_fight_start_position_pair_index) (:1284); "
    "root_offset = (0, -0x384, 0); world_pos = base + root_offset; collision_pos = base (:1297-1299); prev_collision_pos and prev_root_offset copied; zeroes "
    "base_velocity, unk_24C (copy of it), weapon_point_delta[0..1], decaying_velocity, unk_144, remaining_turn, unk_150, use_root_offset_yaw, forward_move_rate; "
    "ground_y = collision y (:1323); softbody_RequestSnap(0) (:1324). It also calls fight_GetStartPosition(1, &local) and adjusts local.vy by -0x384 (:1286, :1308) "
    "but local is never stored or read (comment :1306-1307), so that has no lasting effect. "
    "USES (all): one call, 9F9C.c:1425 in fight_UpdateDefeatDelayAndModeProgress (def :1355), switch case 3 (:1399), only when g_fight_start_position_pair_index "
    "changed (:1408-1424; step-table rows 0x1F / 0x33 / 0x51 set pair 1 / 2 / 3); the caller then restarts fighter 0's current variant script (:1426-1428). "
    "jal asm/funcs/fight_UpdateDefeatDelayAndModeProgress.s:198 (0x8001CBAC); no other jal, no data word (git grep over src, include, asm, registries, docs/engine). "
    "named_syms.txt:1107 is a stale unk_B8 comment, not evidence. "
    "WHY: every write puts fighter 0 back at its start point with its motion state cleared, and the snap request makes the softbody follow. fight_ comes from its "
    "callee fight_GetStartPosition (9F9C.c:3685) and caller; Fighter0 follows the landed fight_StartFighter0DefaultVariantScriptIfMoveKind0x32Or0x11 (9F9C.c:1239). "
    "'Reset' rather than 'Zero' because the unknown fields unk_144 / unk_150 are also cleared. "
    "COUNTER-EVIDENCE: the dead player-1 read is the one part the name does not cover and it has no effect; fighter_Init (9F9C.c:1256) is a different, full initializer. "
    "Trace: matrix/facts.tsv:724 never_hit, :1551 reach 0 of 135; not used, the name rests on code only (statically reached, not dead). Census: no row."
)

_POLL = (
    f"BODY {B('func_800164F8', '6CF8.c')}: for (i = 9999; i >= 0; i--) pollhost(); where pollhost() is PsyQ include/psxsdk/libsn.h:18 `asm(\"break 1024\")` "
    "(Sony's own name for the instruction, N3 tier 1). asm/funcs/func_800164F8.s: break code 0x0001000D, 10000 iterations, jr ra; no stores, no other calls, void; "
    "inline_asm_canonical.txt:137 (gcc-cannot-emit:psyq_pollhost_break). "
    "USES (all): jal sites 6CF8.c:239 in sys_Panic (def :236; asm/funcs/func_80016C3C.s:9), 6CF8.c:498 in main (def :428; asm/funcs/main.s:144), 35000.c:268 in "
    "ndata_ReadChecked (def :253; asm/funcs/ndata_ReadChecked.s:41); each is the body of a `while (1)` after a printf, i.e. a halt, so the finite burst is what "
    "the caller loops. Data word D_800A2674[0] (psxsdk/libsn/73E98.c:31): SNMAIN's __text start word (+ __textlen 0x76B78); it holds the address because the function is the "
    "first .text byte, not a role. externs include/bb2.h:683, 6CF8.c:25, psxsdk/libsn/73E98.c:15. "
    "ALIAS: the census row (function-names.csv:3, KEEP, wave05) and symbol_addrs.txt:1353 carry alias breakpoint_trap_loop_800164F8; this RENAME replaces it "
    "(naming_wave deletes the retired alias line; named_syms.txt:1 records the earlier retirement of syscall_wrapper_break_800164F8 and is left alone). 'trap loop' "
    "misdescribes it: the loop is finite and returns, and the trap is specifically SN's pollhost. "
    "WHY: the name says pollhost (Sony's name for the break) and the count 10000, which is the only thing that tells it from a bare pollhost(). sys_ matches its principal "
    "caller sys_Panic in the same TU. "
    "COUNTER-EVIDENCE: pcdrv_ (the SN host link) is the alternative prefix; rejected because the body does no file I/O (pcdrv_LoadFile uses PCopen). libsn.h has no "
    "function wrapping pollhost, so there is no verbatim Sony name to keep. "
    "FALLBACK if a reviewer objects to the count in the name: sys_PollHostRepeatedly (fresh, 0 hits). "
    "Trace: matrix/facts.tsv:714 never_hit, :1473 reach 0 of 135 (no capture hits a halt); not used, static callers exist."
)

FUNCS = [
    ("0x80037D14", "memcard_StepCardInfoAndLoad", "RENAME", _CARD),
    ("0x8001C624", "fight_ResetFighter0ToStartPosition", "RENAME", _RESET0),
    ("0x800164F8", "sys_PollHost10000Times", "RENAME", _POLL),
]

DATA = []
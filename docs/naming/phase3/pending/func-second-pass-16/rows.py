"""wave107 rows (func-second-pass-16): the particle command trios 0x02 / 0x03 of
eff_StartRecycledVectorCommandWithRedLight (51268.c:502-525; command words 0x210002 into status[2], 0x210003 into
status[3]). Spawns func_80066EC0 / 80066EF4 (command 2, mode 0, row 0, phases 0/1) and func_80066F28 / 80066F5C
(command 3, mode 1, row 1, phases 0/1), draws func_800676C8 (command 2) and func_80067704 (command 3)
(d_8BC50.c:209-214 prototypes, handlers 4..9 at :273-278).
Research input tmp/codex/func-second-pass-16-research.md (untrusted; every claim re-derived at HEAD 8a6830a29, see
notes.txt). Words: Cone (modes 0/1 spawn angle jitter mask 0xFF / offset 0x7F, eff_SpawnParticles 51268.c:2669-2671,
vs 0xFFF / 0 for modes 2/3 at :2672-2674 and +-0x3F for modes 4-7 at :2675-2681), MultiQuad (modes 0-3 emit 2-3 quads per
particle, the p_n loop falling through at :3151-3153; modes 4-7 break after one quad), FirstSlot / SecondSlot (the
status byte the issuer eff_StartRecycledVectorCommandWithRedLight takes: status[2] first, status[3] second, :508-517),
FirstHalf / SecondHalf (the spawn phase, half_index 0 / 1). Verdicts PENDING (the reviewer fills them). (DATA is empty.)
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply 2026-10-31 (naming-b, HEAD 8a6830a29): PASS 6/6. Cone = mode 0/1 rotation jitter (angle_mask 0xFF / offset 0x7F, 51268.c:2669-2671) vs modes 2/3 any direction (0xFFF, :2672-2674); MultiQuad = 2-3 quads per particle (p_n=0 arm :3093 unreachable) vs one for modes 4-7; only the compound is unique (Q132). Each trio borrows the other word from its command, as wave104. FirstSlot/SecondSlot = status[2]/[3] of the only issuer eff_StartRecycledVectorCommandWithRedLight (:512-517). Capture 69cd79b21 reach-only."
NOTES = {}
DROPPED = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-b")

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


# addr: (old, cmd, phase, handler idx, d_8BC50 proto line, d_8BC50 table line, asm endlabel line, census csv line,
#        named_syms RESET line, coverage.tsv line)
FN = {
    "0x80066EC0": ("func_80066EC0", 2, 0, 4, 209, 273, 15, 871, 1680, 879),
    "0x80066EF4": ("func_80066EF4", 2, 1, 5, 210, 274, 15, 872, 1681, 880),
    "0x800676C8": ("func_800676C8", 2, 2, 6, 211, 275, 17, 888, 1432, 896),
    "0x80066F28": ("func_80066F28", 3, 0, 7, 212, 276, 15, 873, 1682, 881),
    "0x80066F5C": ("func_80066F5C", 3, 1, 8, 213, 277, 15, 874, 1683, 882),
    "0x80067704": ("func_80067704", 3, 2, 9, 214, 278, 17, 889, 1433, 897),
}
CMD = {
    2: dict(mode=0, row=0, nth="FirstSlot", il="status[2] first-free at :512-514, word 0x210002", draw="func_800676C8"),
    3: dict(mode=1, row=1, nth="SecondSlot", il="status[3] only while status[2] is set and [3] clear at :515-517, word 0x210003", draw="func_80067704"),
}

_DISPATCH = ("dispatch: eff_StartCommand (51268.c:84-115) clears g_eff_command_phase[command_word.h] (:89) and calls "
             "g_eff_command_handlers[g_eff_command_handler_bases[idx] + phase], storing the result in status_dest; eff_StepCommand "
             "(:117-144) repeats the call once per step; tables d_8BC50.c:200-201 (bases[2] = 4, bases[3] = 7) and :268-330 (handlers, "
             "index 0 at :269). Lifecycle by the code: phase 0 handler (spawn half 0, phase := 1, returns 1), next frame the phase 1 "
             "handler (spawn half 1, phase := 2, returns 1), then the phase 2 handler every frame until eff_LinkParticleQuads returns 0 "
             "(no quads written this frame), which zeroes the status and frees the issuer's slot")

_NOUN = ("eff_ and Particles = unchanged from the landed siblings (eff_SpawnParticles 51268.c:2628, eff_PrepareParticles :2779, "
         "eff_UpdateParticles :2893, eff_LinkParticleQuads :3158); the Spawn / DrawAndAdvance split and the FirstSlot / SecondSlot / "
         "FirstHalf / SecondHalf words follow wave104 (eff_SpawnTextureSelectedParticles*, eff_DrawAndAdvanceTextureSelectedParticles*); "
         "'Red' is not used (the 0xFF0000 point light is the issuer's write at :524, not an act of these handlers); no visual word")

_PRIOR = ("docs/naming/phase3/no_name.tsv ledger rounds w58, w72, w77, w81, w83, w94 (NO_NAME: the particle_mode tests read as range tests "
          "only; the `< 2` arm that separates modes 0/1 from 2/3, and the quad count of modes 0-3 against 4-7, was not weighed)")

_QUADS = ("the p_n selection (51268.c:3090-3097) yields 1 or 2 (the middle `p_n = 0` arm at :3093-3094 needs squared_length > var_1 / 2 "
          "after the `var_1 / 3` test already failed to hold, so it cannot be taken), and the loop `for (; *p_n >= 0; (*p_n)--)` "
          "(:3103) falls through :3151-3153 for modes 0-3, so 2-3 semi-transparent POLY_FT4 per live particle; modes 4-7 break after "
          "one quad (:3127-3149)")


def _evidence(addr):
    old, cmd, phase, hidx, proto, tbl, endl, csvl, nsl, cov = FN[addr]
    c = CMD[cmd]
    draw = phase == 2
    mode, row = c["mode"], c["row"]
    nm = ("eff_DrawAndAdvanceConeMultiQuadParticles" + c["nth"]) if draw else (
        "eff_SpawnConeMultiQuadParticles" + c["nth"] + ("FirstHalf" if phase == 0 else "SecondHalf"))
    if draw:
        body = (f"eff_PrepareParticles({mode}, {row}); eff_UpdateParticles({mode}, {row}); return eff_LinkParticleQuads({mode}, {row}); "
                f"mode {mode} takes the `particle_mode < 2` arms: Prepare 51268.c:2798-2804 (base 0x40 x 0x20, D_8009B890, 3 positions), "
                f"Update :3026-3036 (perspective-only size) and :3122-3123 (tint R + (G << 8), B = 0); {_QUADS}; Link (:3158-3205) links "
                f"the new quads into the OT and returns 1 only if quads were written, which becomes status[{cmd}] (0 ends the command)")
    else:
        body = (f"spawn_result = eff_SpawnParticles({mode}, {row}, {phase}); g_eff_command_phase[{cmd}] = {phase + 1}; return spawn_result. "
                f"eff_SpawnParticles (51268.c:2628-2720) at mode {mode} takes the `particle_mode < 2` arm angle_mask 0xFF / angle_offset 0x7F "
                f"(:2669-2671; modes 2/3 take 0xFFF / 0 at :2672-2674, modes 4-7 0x7F / 0x3F at :2675-2681), and each particle's rotation "
                f"angles are (rand & mask) - offset about the emitter vector (:2694-2717), so the velocity stays in a cone around "
                f"g_eff_particle_emitter_vectors[{row}]. half_index {phase}: " +
                ("resets the row fill index, latches g_eff_command_position / g_eff_command_vector into the emitter (:2651-2666), then spawns "
                 "records 0..23 (:2685-2687); returns 1"
                 if phase == 0 else
                 "records 24..47 from the emitter vector latched by the half 0 call (the latch at :2653 is not taken); returns 1"))
    callers = (f"none by name: the one address-taken use is g_eff_command_handlers[{hidx}] = g_eff_command_handler_bases[{cmd}] + phase {phase} "
               f"(src/main/d_8BC50.c:{tbl}; prototype :{proto}); all other occurrences of {old} are the C definition in src/main/51268.c, "
               f"the asm glabel/endlabel asm/funcs/{old}.s:1,{endl}, the census docs/naming/function-names.csv:{csvl}, the named_syms.txt:{nsl} "
               "RESET note (retired efc_ name, address-keyed), the memory/grind ledger and the no_name.tsv row" +
               ("; plus two comments in include/game.h, :783 ('particle_row_index 0..3 from the callers func_800676C8..eff_DrawAndAdvance"
                "AdditiveSingleQuadParticlesSecondSlot', a range start) and :1118 ('each of the wrappers func_800676C8 ..', likewise): the "
                "dry run does NOT edit include/game.h, so the apply must hand-edit both comments in the wave and rerun the keycheck "
                "--sub-comments" if addr == "0x800676C8" else "") +
               f". Command {cmd} is issued only by eff_StartRecycledVectorCommandWithRedLight ({c['il']}); " + _DISPATCH)
    why = (("DrawAndAdvance = the body steps frame_age, kills records and draws POLY_FT4 quads linked into the OT, keeps the command alive "
            "until no quads remain" if draw else
            "Spawn = it fills the particle records; FirstHalf / SecondHalf = half_index 0 / 1 (records 0..23 / 24..47, the first also "
            "latches the emitter), phase := 1 / 2 written") +
           "; Cone = the mode 0/1 spawn jitter (angle_mask 0xFF, :2669-2671) " +
           ("taken from the command's spawns (call-graph role)" if draw else "in this handler's own callee arm") +
           "; MultiQuad = modes 0-3 emit 2-3 quads per particle, modes 4-7 one " +
           ("in this draw's own output" if draw else "(the command's phase-2 draw " + c["draw"] + "; call-graph role)") +
           f"; {c['nth']} = status[{cmd}] ({'first' if cmd == 2 else 'second'} free byte of the issuer). Uniqueness: modes 2/3 are multi-quad "
           "but not cone (0xFFF), modes 4-7 are cone (+-0x3F) but single-quad, so only the compound is true of modes 0/1. " + _NOUN)
    counter = ("(1) 'Cone' alone is shared with modes 4-7 (tighter) and 'MultiQuad' alone with modes 2/3; only the compound is unique "
               "(the structure of wave104's AdditiveSingleQuad); (2) particle row " + str(row) + " is shared with command " +
               ("0x15" if cmd == 2 else "0x14") + " (modes 3 / 2 on rows 0 / 1), so 'Slot' is the command slot, not the row; (3) " +
               ("the draw does not compute the cone itself, it takes Cone from its command's spawns; fallback if the reviewer refuses the "
                "borrowing: NO_NAME for the two draws and keep the four spawns; " if draw else
                "the multi-quad word is the command's draw, not this handler's own output; fallback as for the draws; ") +
               "(4) 'Red' / 'Droplet' are not used: the red light is the issuer's write (:524)" +
               ("; (5) row 1 shared with func_80067740 (command 0x14 draw), name rests on code plus slot symmetry with command 2" if cmd == 3 else ""))
    trace = (f"TRACE (Q124, corroborating only): docs/naming/runtime/matrix/coverage.tsv:{cov} hit "
             + ("in 40 captures" if cmd == 2 else "in 4 captures (story, versus-cpu-clear opp1-3)") +
             "; all only_x.tsv pair rows for this address are inconclusive=1; no role or visual word is taken" +
             ("; the Q124 stub-and-observe docs/naming/runtime/versus-2p-kimigaura-blood-link-stub-1 (69cd79b21) stubbed eff_LinkParticleQuads "
              "and its first stubbed hits had ra 0x800676F4 = this body's return, args (0, 0): reach only, the whole screen diverges from "
              "frame 10592 so no visual word is admissible" if addr == "0x800676C8" else ""))
    return nm, body, callers, why, counter, trace


FUNCS = []
for _addr in FN:
    _nm, _body, _callers, _why, _counter, _trace = _evidence(_addr)
    _ev = (f"BODY: {B(FN[_addr][0], '51268.c')}: {_body}. CALLERS: {_callers}. WHY: {_why}. COUNTER-EVIDENCE: {_counter}. "
           f"Prior manifests: {_PRIOR}. {_trace}")
    FUNCS.append((_addr, _nm, "RENAME", _ev))

DATA = []

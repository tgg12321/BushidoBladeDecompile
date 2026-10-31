"""wave108 rows (func-second-pass-17): the particle command trios 0x14 / 0x15 of
eff_StartRecycledVectorCommandAndJitteredBillboardsWithPaleRedLight (51268.c:563-591; command words 0x210014 into
status[20], 0x210015 into status[21]). Spawns func_80066F90 / 80066FC4 (command 0x14, mode 2, row 1, phases 0/1) and
func_80066FF8 / 8006702C (command 0x15, mode 3, row 0, phases 0/1), draws func_80067740 (command 0x14) and func_8006777C
(command 0x15) (d_8BC50.c:247-252 prototypes, handlers 42..47 at :311-316; bases[20] = 0x2A, bases[21] = 0x2D at :203).
Research input tmp/codex/func-second-pass-17-research.md (untrusted; every claim re-derived at HEAD a1a8066ed, see
notes.txt). Words: Omnidirectional (modes 2/3 spawn angle jitter mask 0xFFF / offset 0 on all three rotation axes,
eff_SpawnParticles 51268.c:2672-2674, vs 0xFF / 0x7F for modes 0/1 at :2669-2671 and 0x7F / 0x3F for modes 4-7 at
:2675-2681), MultiQuad (modes 0-3 emit 2-3 quads per particle, the p_n loop falling through at :3151-3153; modes 4-7
break after one quad), FirstSlot / SecondSlot (the status byte the issuer takes: status[20] first, status[21] second,
:573-584), FirstHalf / SecondHalf (the spawn phase, half_index 0 / 1). Verdicts PENDING (the reviewer fills them).
(DATA is empty.)
"""
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply 2026-10-31 (naming-a, HEAD a1a8066ed): PASS 6/6. Omnidirectional = mode 2/3 rotation jitter angle_mask 0xFFF / offset 0 on all three axes (51268.c:2672-2674) vs Cone 0x7F (modes 0/1) and 0x3F (modes 4-7); MultiQuad = modes 0-3 fall-through, 2-3 quads per particle (:3103, :3151-3153). Draws borrow Omnidirectional from their command spawns, as wave107. FirstSlot/SecondSlot = status[20]/[21] of the issuer (:569-584). Knockout 0b67c7ce6 corroborates slot order only."
NOTES = {}
DROPPED = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-a")

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


# addr: (old, cmd byte, phase, handler idx, d_8BC50 proto line, d_8BC50 table line, asm endlabel line, census csv line,
#        named_syms RESET line, coverage.tsv line)
FN = {
    "0x80066F90": ("func_80066F90", 0x14, 0, 42, 247, 311, 15, 875, 1684, 883),
    "0x80066FC4": ("func_80066FC4", 0x14, 1, 43, 248, 312, 15, 876, 1685, 884),
    "0x80067740": ("func_80067740", 0x14, 2, 44, 249, 313, 17, 890, 1434, 898),
    "0x80066FF8": ("func_80066FF8", 0x15, 0, 45, 250, 314, 15, 877, 1686, 885),
    "0x8006702C": ("func_8006702C", 0x15, 1, 46, 251, 315, 15, 878, 1687, 886),
    "0x8006777C": ("func_8006777C", 0x15, 2, 47, 252, 316, 17, 891, 1435, 899),
}
CMD = {
    0x14: dict(mode=2, row=1, idx=20, nth="FirstSlot", il="status[20] when clear at :573-578, word 0x210014", draw="func_80067740"),
    0x15: dict(mode=3, row=0, idx=21, nth="SecondSlot", il="status[21] only while status[20] is set and [21] clear at :579-584, word 0x210015", draw="func_8006777C"),
}

_DISPATCH = ("dispatch: eff_StartCommand (51268.c:84-115) clears g_eff_command_phase[command_word.h] and calls "
             "g_eff_command_handlers[g_eff_command_handler_bases[idx] + phase], storing the result in status_dest; eff_StepCommand "
             "(:117-144) repeats the call once per step; tables d_8BC50.c:203 (bases[20] = 0x2A, bases[21] = 0x2D) and :268-330 "
             "(handlers, index 0 at :269). Lifecycle by the code: phase 0 handler (spawn half 0, phase := 1, returns 1), next frame the "
             "phase 1 handler (spawn half 1, phase := 2, returns 1), then the phase 2 handler every frame until eff_LinkParticleQuads "
             "returns 0 (no quads written this frame), which zeroes the status and frees the issuer's slot")

_NOUN = ("eff_ and Particles = unchanged from the landed siblings (eff_SpawnParticles 51268.c:2628, eff_PrepareParticles :2779, "
         "eff_UpdateParticles :2893, eff_LinkParticleQuads :3158); the Spawn / DrawAndAdvance split and the FirstSlot / SecondSlot / "
         "FirstHalf / SecondHalf words follow wave104 and wave107 (eff_SpawnConeMultiQuadParticles*, eff_DrawAndAdvanceConeMultiQuad"
         "Particles*); 'PaleRed' is not used (the 0xFF8080 point light is the issuer's write at :591, not an act of these handlers); "
         "no visual or role word")

_PRIOR = ("docs/naming/phase3/no_name.tsv rows 67-72, ledger rounds w58, w72, w77, w83, w94 (w97 for the second-slot rows): NO_NAME "
          "'modes 2 and 3 take identical branches ... the twin differs only by slot/particle row'. Answer: the mode tests are range "
          "tests whose `< 4` arm (after `< 2`) is unique to modes 2/3 (0xFFF / 0 angle, :2672-2674), and the slot-only distinction "
          "between twins is the one wave104 and wave107 landed")

_QUADS = ("the p_n selection (51268.c:3090-3097) yields 1 or 2 (the middle `p_n = 0` arm at :3093-3094 needs squared_length > var_1 / 2 "
          "after the `var_1 / 3` test already failed to hold, so it cannot be taken), and the loop `for (; *p_n >= 0; (*p_n)--)` "
          "(:3103) falls through :3151-3153 for modes 0-3, so 2-3 semi-transparent POLY_FT4 per live particle; modes 4-7 break after "
          "one quad (:3127-3149)")


def _evidence(addr):
    old, cmd, phase, hidx, proto, tbl, endl, csvl, nsl, cov = FN[addr]
    c = CMD[cmd]
    draw = phase == 2
    mode, row, idx = c["mode"], c["row"], c["idx"]
    nm = ("eff_DrawAndAdvanceOmnidirectionalMultiQuadParticles" + c["nth"]) if draw else (
        "eff_SpawnOmnidirectionalMultiQuadParticles" + c["nth"] + ("FirstHalf" if phase == 0 else "SecondHalf"))
    if draw:
        body = (f"eff_PrepareParticles({mode}, {row}); eff_UpdateParticles({mode}, {row}); return eff_LinkParticleQuads({mode}, {row}); "
                f"modes 2/3 take the `particle_mode < 4` arms: Prepare 51268.c:2805-2811 (D_8009B890, uv 7x7, base 0x20 x 0x10, 3 "
                f"positions; modes 0/1 base 0x40 x 0x20 at :2798-2804), Update :3037-3051 (perspective size plus the age boost "
                f"size * 20 / (age * 6 + 1)) and :3124-3126 (tint R + ((R >> 2) << 8) + (B << 16)); {_QUADS}; Link (:3158-3205) links "
                f"the new quads into the OT and returns 1 only if quads were written, which becomes status[{idx}] (0 ends the command)")
    else:
        body = (f"spawn_result = eff_SpawnParticles({mode}, {row}, {phase}); g_eff_command_phase[{idx}] = {phase + 1}; return spawn_result. "
                f"eff_SpawnParticles (51268.c:2628-2720) at mode {mode} takes the `particle_mode < 4` arm angle_mask 0xFFF / angle_offset 0 "
                f"(:2672-2674; modes 0/1 take 0xFF / 0x7F at :2669-2671, modes 4-7 0x7F / 0x3F at :2675-2681), and each particle's "
                f"rotation angles are (rand & mask) - offset about the emitter vector (:2694-2717), so every axis takes any angle of the "
                f"full 0..0xFFF turn and the velocity can point in any direction around g_eff_particle_emitter_vectors[{row}]. "
                f"half_index {phase}: " +
                ("resets the row fill index, latches g_eff_command_position / g_eff_command_vector into the emitter (:2651-2666), then spawns "
                 "records 0..23 (:2685-2687); returns 1"
                 if phase == 0 else
                 "records 24..47 from the emitter vector latched by the half 0 call (the latch at :2653 is not taken); returns 1"))
    callers = (f"none by name: the one address-taken use is g_eff_command_handlers[{hidx}] = g_eff_command_handler_bases[{idx}] + phase {phase} "
               f"(src/main/d_8BC50.c:{tbl}; prototype :{proto}); all other occurrences of {old} are the C definition in src/main/51268.c, "
               f"the asm glabel/endlabel asm/funcs/{old}.s:1,{endl}, the census docs/naming/function-names.csv:{csvl}, the named_syms.txt:{nsl} "
               "RESET note (retired efc_ name, address-keyed), the memory/grind ledger (spawns only), docs/naming/apiscan/evidence.json, "
               "the no_name.tsv row and recorded runtime data. Command " + f"0x{cmd:02X} is issued only by "
               f"eff_StartRecycledVectorCommandAndJitteredBillboardsWithPaleRedLight ({c['il']}), whose only caller is "
               f"fight_RecordReplayEventAndDispatchSoundOrEffect case 14 (17AFC.c:5093); " + _DISPATCH)
    why = (("DrawAndAdvance = the body steps frame_age, kills records and draws POLY_FT4 quads linked into the OT, keeps the command alive "
            "until no quads remain" if draw else
            "Spawn = it fills the particle records; FirstHalf / SecondHalf = half_index 0 / 1 (records 0..23 / 24..47, the first also "
            "latches the emitter), phase := 1 / 2 written") +
           "; Omnidirectional = the mode 2/3 spawn jitter (angle_mask 0xFFF on all three axes, :2672-2674) " +
           ("taken from the command's spawns (call-graph role)" if draw else "in this handler's own callee arm") +
           "; MultiQuad = modes 0-3 emit 2-3 quads per particle, modes 4-7 one " +
           ("in this draw's own output" if draw else "(the command's phase-2 draw " + c["draw"] + "; call-graph role)") +
           f"; {c['nth']} = status[{idx}] ({'first' if cmd == 0x14 else 'second'} free byte of the issuer). Uniqueness: modes 0/1 are "
           "multi-quad but cone (0xFF / 0x7F), modes 4-7 are cone (+-0x3F) and single-quad, so only modes 2/3 are both omnidirectional "
           "and multi-quad (Omnidirectional alone is also unique; MultiQuad is kept to pair with the ConeMultiQuad siblings that share "
           "the draw path). " + _NOUN)
    counter = ("(1) 'Omnidirectional' claims only that any direction is possible (three uniformly drawn Euler angles are not a uniform "
               "sphere, and speed scales with the emitter vector), which the 0xFFF mask on every axis shows; (2) particle row " + str(row) +
               " is shared with command " + ("0x03 (mode 1 on row 1)" if row == 1 else "0x02 (mode 0 on row 0)") +
               ", so 'Slot' is the command slot, not the row; (3) mode 2 = mode 3 in every range test, so the slot word is the only "
               "separator of the twins (the palered-slot2-poke capture bd629c940 shows 0x15 pixel-identical to 0x14); (4) " +
               ("the draw does not compute the spread itself, it takes Omnidirectional from its command's spawns; fallback if the reviewer "
                "refuses the borrowing: NO_NAME the two draws and keep the four spawns (all-or-none with wave107's draws, which landed on "
                "the same borrowing); " if draw else
                "the multi-quad word is the command's draw, not this handler's own output; fallback as for the draws; ") +
               "(5) 'Spark' / 'Spray' / colour / 'PaleRed' are not used: whole-screen divergence in the stub runs and the red light is "
               "the issuer's write (:591)")
    if draw:
        sub = ("docs/naming/runtime/versus-2p-hidden-suminagashi-pale-red-stub-1 (0b67c7ce6, Q124 stub-and-observe): " + (
            "stubbing func_80067740 (ret 0 from fight 8805) removes the effect after both impacts and the second impact is routed to "
            "0x14 again because ret 0 leaves status[20] 0 (inferred by the commit, no start breakpoint); whole screen differs from "
            "8958 so no visual word" if cmd == 0x14 else
            "stubbing func_8006777C leaves the first impact pixel-identical through 9034 and removes the second impact's effect: "
            "the slot order the code gives; whole screen differs from 9038 so no visual word"))
    else:
        sub = (("lib-versus-2p-hidden-suminagashi-1/phases.tsv:11 (first impact at 8949: func_80066F90 8949, func_80066FC4 8951, "
                "func_80067740 8953) and versus-2p-hidden-suminagashi-double-throw-1/phases.tsv:11") if cmd == 0x14 else
               ("versus-2p-hidden-suminagashi-double-throw-1/phases.tsv:12 (second thrown object lands at 9029 while status[20] is 01: "
                "func_80066FF8 9029, func_8006702C 9031, func_8006777C 9033..9130)"))
    trace = (f"TRACE (Q124, corroborating only; phases.tsv / knockout.md read, reach and slot order only): {sub}; "
             f"docs/naming/runtime/matrix/coverage.tsv:{cov} predates these captures and is not rebuilt; no role or visual word is taken")
    return nm, body, callers, why, counter, trace


FUNCS = []
for _addr in FN:
    _nm, _body, _callers, _why, _counter, _trace = _evidence(_addr)
    _ev = (f"BODY: {B(FN[_addr][0], '51268.c')}: {_body}. CALLERS: {_callers}. WHY: {_why}. COUNTER-EVIDENCE: {_counter}. "
           f"Prior manifests: {_PRIOR}. {_trace}")
    FUNCS.append((_addr, _nm, "RENAME", _ev))

DATA = []

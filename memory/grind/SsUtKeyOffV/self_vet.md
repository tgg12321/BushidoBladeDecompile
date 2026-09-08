# SELF-VET — SsUtKeyOffV
CONSTRUCTS: none

Diff (src/main.c, replaces `INCLUDE_ASM("asm/funcs", SsUtKeyOffV);`):

    s16 SsUtKeyOffV(s16 voice) {
        if (_snd_ev_flag == 1) {
            return -1;
        }
        _snd_ev_flag = 1;
        if (voice >= 0 && voice < 24) {
            _svm_cur.voice = voice;
            _SsVmKeyOffNow(0);
            _snd_ev_flag = 0;
            return 0;
        }
        _snd_ev_flag = 0;
        return -1;
    }

No new declarations, no new globals, no casts, no locals, no gotos, no volatile,
no asm, no FAKE annotations. Uses the existing `extern s32 _snd_ev_flag` (main.c:227),
the existing `struct struct_svm _svm_cur` (include/sound.h:28) and the existing
`_SsVmKeyOffNow` (main.c:1175, matched C).

## T1 semantic purpose: every statement is load-bearing. The re-entrancy flag test/set/clear
bracket, the 0..23 voice range check, the current-voice store and the key-off call are
the function's whole specification (PsyQ libsnd SsUtKeyOffV). Removing any statement
changes behaviour.
## T2 human-programmer: yes — it is the same shape Sony/psyz use for the sibling
SsUtKeyOff (tmp/psyq_prov/psyz/decomp/src/libsnd/ut_key.c:76-98): guard flag, range check
with the success work inside the `if`, clear flag, return -1 on the fall-out path.
`voice >= 0 && voice < 24` is the ordinary way to bounds-check a signed voice index.
## T3 GCC-internals justification: none required for any construct. The ledger explains
WHY arm order matters (jump.c range swap / sched1 / jump2 cross-jump) as pass attribution
for the audit trail, but the code itself is the natural specification of the function;
arm order between two equally natural spellings was chosen by measurement, which is
ordinary matching-decomp practice, not a coercion construct.
## T4 permuter/search provenance: no permuter run; three hand-written variants measured
(evidence.md E5), all ordinary C. The winner is a documented psyq/psyz idiom.
## T5 family check: no forbidden family matched by shape or analogy — no pins, asm,
barriers, volatile, dead stores, dead locals, pads, alias renames, casts, or
always-true wrappers. No sanctioned-family carve-out is being claimed either.
## T6 naming-announces-intent: no new names introduced; `voice` is the psyz/PsyQ parameter name.

SANCTIONED-FAMILY-CLAIMS: none
ANNOTATION-CONFORMANCE: n/a — no FAKE construct

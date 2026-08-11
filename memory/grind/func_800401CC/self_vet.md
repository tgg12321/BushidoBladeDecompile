# SELF-VET — func_800401CC

STATUS: Not a candidate-ready session — session 3 (permuter) outcome is a
ruling-request; no completion is being submitted, so the full six-test vet is
not filed. This file replaces the prior session's vet, which the driver
validator rejected on token-matching grounds.

CONSTRUCTS (currently at rest in src/text1a_pre.c): one staged-mask
assignment through the pre-existing dead-after-call texture-V coordinate
local (widened s16 -> s32; value live, read by both packet-link statements;
FAKE-annotated in src with what + mechanism (global.c call-arg copy
preference) + lever-exhaustion pointer). This instance was ruled "properly
annotated and evidenced" by the prior layer-1 review. Sandbox distance with
only this construct: 2.

The symmetric twin staging for the low 24-bit mask through the texture-U
local (which reaches sandbox 0, see candidate.c) is NOT applied in src and is
NOT being claimed — its classification is the subject of this session's
ruling-request.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: staged-value-reused-variable (the applied V-coordinate instance)
  SCOPE: "SANCTIONED 2026-07-03: a load places too LATE (fresh single-set dest gets the scheduler's load-late LAUNCH priority) → stage the value through an EXISTING currently-dead local (`v0 = idx[1]; arg5 = tbl[v0];`), FAKE-annotated + lever-exhaustion; live code only (zero dead stores); SOTN ships the shape (\"fake reuse of i\", 6 files)."
  PRECEDENT: .claude/rules/codegen-technique-index.md:21

ANNOTATION-CONFORMANCE: one FAKE construct at rest, annotated at its set in
src/text1a_pre.c (what + mechanism: global.c allocno call-arg copy
preference + lever-exhaustion: hypotheses.md K1-K4 + P1/P2 + s2 A-probes).

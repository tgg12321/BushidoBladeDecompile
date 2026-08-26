# SELF-VET — func_800871D4

CONSTRUCTS: (1) explicit `& 0xFFFF` width mask on a value read from `extern u16
D_8010280A`, written twice (`vc = raw & 0xFFFF;` at the top and again as the
restore at the end of the else arm); (2) in-place `vc -= 0x10;` in the else arm
followed by that restore; (3) interleaved statement ordering of the two
read-modify-write groups (D_801078D8/D_800F1B10 and D_801078DA/D_800F1B12).
No FAKE construct, no dead store, no dead local, no volatile, no inline asm, no
pointer alias, no register pin, no `do {} while (0)`, no empty conditional, no
pad/dummy local, no alias rename, no scheduling barrier. The diff is one
function body replacing one `INCLUDE_ASM(...)` line in src/main.c; zero
regfix/asmfix rules; full-build SHA1 == oracle.

## T1 semantic purpose:
(3) Statement interleaving: this is pure ordering of six mutually independent
statements over four distinct globals; it has no semantic content by
construction and needs none — statement order is an authoring choice, not a
construct. It is what actually closed the function this session (score 6 -> 0).
(2) The in-place `vc -= 0x10` plus the restore IS semantically required: the
shift amount for voices 16..23 is `vc - 16`, and the voice index is then needed
again, unmodified, for the `_svm_voice[]` offset `vc * 54`. Remove either half
and the function computes the wrong offset; this is real program logic.
(1) The `& 0xFFFF` masks: HONEST ANSWER — under the current `extern u16
D_8010280A` declaration these masks are value-preserving, so they have no
observable effect on the function's behaviour. They are attached to assignments
that ARE required (the initialisation of `vc`, and its restore), but the mask
itself is width-redundant. I am not claiming otherwise. What removing them does
is documented and measured, not asserted: `rejected/s4-no-mask-plain-copies.c`
(no masks) = score 6 / 53 insns; `rejected/s4-u16-locals-promote-mode-
truncations.c` (u16 locals, the "just use unsigned short" reading) = 55 insns;
`rejected/s4-u32-raw-u16-vc-narrow.c` (type-level narrowing) = 54 insns.

## T2 human-programmer:
(3) Yes, trivially — grouping "load both words, OR both masks in, store both
back, then clear both bits from the key-on pair" is arguably the more natural
way to write a routine that maintains two parallel 16-bit voice-mask words than
finishing one word before starting the other.
(2) Yes — it is the routine's actual arithmetic.
(1) A reader could ask "why mask a value that came from a 16-bit load?", so I
will not pretend this passes on first principles. It passes on precedent: the
identical shape (`u16` value loaded from memory, copied into a `u32` local
through `& 0xFFFF`) is already shipping in accepted BB2 C in the SAME FILE and
the SAME Sony library family — `SpuGetVoiceVolume` at src/main.c:2884
(`temp_a3 = temp_v1 & 0xFFFF;` where `temp_v1` is a `u16` loaded from
`*(u16 *)(temp)`) and src/main.c:2891, plus `_SsVmVSetUp` at src/main.c:1250
(`if ((a0 & 0xFFFFu) >= 0x10)`). Those functions carry zero regfix/asmfix rules,
are not in engine/queue.json, are not in inline_asm_canonical.txt, and are
covered by the oracle build I re-ran this session. So this is not a first reach
of a new spelling: it is the tree's established idiom for this exact situation.

## T3 GCC-internals justification:
Mixed, stated plainly. For (3), the pass attribution (global.c `allocno_compare`
priority = floor_log2(n_refs)*n_refs*size/live_length; the else-arm mask's
live_length falls when its last reference moves ~5 RTL insns earlier) is how I
PREDICTED the interleave would work and is required by the brief's
pass-attribution rule — but the construct itself is ordinary statement order
that stands on its own semantically, so the GCC mechanism is an explanation,
not the justification. For (1) I will not hide it: combine.c's `can_combine_p`
refusing to fold a load into an AND whose source stays live is the mechanism by
which the masks survive, and that mechanism is why s3 kept them. That is the
weakest point of this diff and I am flagging it rather than dressing it up.

## T4 permuter/search provenance:
None. A permuter campaign WAS run this session (mandated modality;
tools/decomp-permuter/nonmatchings/func_800871D4_s4, base_score 160, best find
40, harvested and stopped) and NOTHING from it is in this diff. The winning
delta came from reading s3's ra_solver live-range accounting and reasoning about
which of the four inverse.py atom families could be reached without adding an
instruction. No auto-search output was adopted.

## T5 family check:
No sanctioned-family carve-out is claimed, so no FAKE annotation is emitted.
The nearest FORBIDDEN family to construct (1) is F2 (`redundant width casts`,
listed as "evidence insufficient" in .claude/rules/no-new-park-categories.md).
The difference I rely on, stated so a reviewer can check it: F2 as adjudicated
in this repo (docs/grind/decisions.md, func_8001F938) is the SIGNEDNESS-SPLIT
DUAL READ — the same memory read through two differently-typed views so combine
manufactures a SECOND load. There is exactly ONE load of D_8010280A here, one
typed view of it, and no signedness split; the masks change no value and
manufacture no memory access. Whether that difference is enough is the
reviewer's call, not mine, and I am not asserting a family grant. Constructs
(2) and (3) match no forbidden family at all — (2) is program arithmetic and (3)
is statement order.

## T6 naming-announces-intent:
Every local was renamed away from s3's m2c register names this session to
semantic names drawn from the function's identity (Sony LIBSND `_SsVmKeyOffNow`):
`raw`, `vc`, `mask_lo`, `mask_hi`, `off`, `keyoff_lo`, `keyoff_hi`, `keyon_lo`,
`keyon_hi`. No `pad`/`dummy`/`unused`/`spill`/`_buf`/`tail`/`slack` names. Every
local is written and read on a live path; none exists only to be discarded, and
none has its address taken. Verified codegen-neutral: the renamed form measures
score 0 / 52 insns exactly as the register-named form did.

SANCTIONED-FAMILY-CLAIMS: none

ANNOTATION-CONFORMANCE: n/a — no FAKE construct

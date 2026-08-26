# SELF-VET — special_camera_get_rot_dir

Session s7 (rederive modality, 2026-08-26). Diff = src/code6cac_b2_post.c ONLY
(verified with `git status --short`: the sole modified build input). No header
edit, no regfix/asmfix rule, no inline asm, no pin, no volatile, no goto, no
dead local, no /* FAKE */ construct anywhere in the diff.

Measured THIS session, on THIS chassis, with these edits in place in src/:
  - `sandbox special_camera_get_rot_dir --disable all` -> score 0, 72/72 insns
  - `sandbox func_800372F4 --disable all`              -> score 0, 21/21 insns
  - `verify-oracle`                                    -> "ok": true, "build_matches": true
    (full clean-driver build+link, SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa)

CONSTRUCTS: (1) block-scope `extern s32 CdRead(s32, s32, s32);` inside
func_800372F4, correcting a stale 1-argument prototype in include/code6cac.h that
contradicts this repo's own matched definition of CdRead; (2) func_800372F4's
signature widened to (s32 nbytes, s32 buf, s32 mode), with both new parameters
READ and forwarded in the body; (3) a `CamRot` struct typedef + one aggregate
assignment `*(CamRot *)dest = *(CamRot *)&sp_buf[0x10];`; (4) local `mode` holding
the CD mode 0x80, read at both func_800372F4 call sites; (5) locals `index` and
`cam_base`, each read twice; (6) an ordinary `for (;;) { ... continue; ... break; }`
retry loop.

## T1 semantic purpose: does each construct change observable behaviour?
(1) YES. Without the corrected prototype the call `CdRead(sectors, buf, mode)` does
not compile at all, and the 1-argument form leaves CdRead's buffer address and CD
mode unset. CdRead's matched definition (src/system.c:901) reads all three
parameters: `D_800A14DC = mode; D_800A14D4 = buf; *ps = sectors;`. Dropping buf and
mode is a semantic bug, not a simplification.
(2) YES. func_800372F4's whole job is "issue a blocking CdRead of N bytes into buf
with mode M". Its two call sites pass the destination buffer (sp_buf / dest[2]) and
the mode; those values reach CdRead only through this wrapper. This is the point
that the 2026-08-26 01:09 layer-1 FAIL correctly attacked in the PREVIOUS form,
where the parameters were declared but never read while the body still called the
1-argument CdRead. That defect is fixed here: both parameters appear in the body,
in the CdRead call, and there is no claim anywhere that they "fall through
untouched".
(3) YES. It performs the 60-byte copy of the rotation record out of the sector
buffer into the caller's array — the function's entire output.
(4) YES. It is the third argument of both CD reads; deleting it deletes the mode.
(5) YES. `index + cam_base` is the CD directory entry address; both are read.
(6) YES. It is the retry semantics: any failed read restarts from the seek.

## T2 human-programmer: would a human write this from the spec?
Yes, for (2)-(6). The spec is "seek to this stage's entry, read a sector, copy the
60-byte rotation record out of it, seek to the next sector, read the block described
by dest[2]/dest[3], and retry the whole sequence on any failure". A human writes a
struct copy for a fixed-size record, a named `mode` for a CD mode constant, a
`for(;;)` retry loop with `continue`/`break`, and a wrapper that forwards its buffer
and mode to CdRead. Construct (1) is the one a human would NOT normally write — a
human would have the right prototype in their libcd header. It exists here only
because our copy of that header (include/code6cac.h:510) is factually wrong and is
OUT OF SCOPE for this candidate per the standing driver constraint. The clean
placement of this declaration is that header; see the handoff note at the bottom.
The build prints `conflicting types for CdRead` at the declaration — the correction
is deliberately visible, not hidden, and this same file already ships an analogous
pre-existing prototype diagnostic at src/code6cac_b2_post.c:387.

## T3 GCC-internals justification: is any construct justified by a GCC pass?
No. Every construct above is argued from program semantics and from the target's
own argument setup, not from allocator/scheduler/DCE behaviour. Specifically:
 - The copy is one aggregate assignment because the record IS one 60-byte record;
   the fact that GCC's MIPS block-move expander turns that into the target's 4-word
   loop + 3-word tail is an OBSERVATION about how the natural C compiles (pass
   attribution), not the reason the C is written that way.
 - `mode` is a named constant because it is a CD mode; the target keeping 0x80 in
   callee-saved $s4 across the whole loop is corroborating evidence that the
   original source held it in a variable, not the justification for holding it.
 - There is no pre-loop pointer local for sp_buf2 because none is needed: the
   address is simply taken at its two use sites. The construct here is the ABSENCE
   of an extra local, i.e. the simpler spelling.
No construct in this diff was chosen to move a live length, a reg_n_refs count, an
allocno priority, a REG_EQUIV note, or an instruction's schedule slot.

## T4 permuter/search provenance: was anything found by auto-search?
No. The permuter modality for this function was measured DEAD in s4 (~46k iters,
two chassis, zero score-0 finds) and none of its output is used here. This form was
re-derived from the target's own semantics: the wrapper's argument setup in the
target (the caller loads $a1/$a2 before `jal func_800372F4`), the matched CdRead
definition already in this repo, and the 60-byte record shape.

## T5 family check: does anything match a forbidden family, even by analogy?
No.
 - NOT the banned asymmetric loop-spelling pair (banned 2026-08-26 00:52): there is
   no hand-written copy loop in this form at all and no `goto` anywhere in the diff.
   The only loop written by hand is the retry loop, spelled structurally.
 - NOT the banned unread-parameter widening (banned 2026-08-26 01:09): buf and mode
   are read in func_800372F4's body and forwarded to CdRead.
 - NOT an alias rename: `CdRead` keeps its own name; no `asm("Sym")` anywhere. An
   alias rename supplies a SECOND handle for one object under a different
   identifier; this supplies the CORRECT type for the one identifier that already
   exists, matching its definition.
 - NOT volatile coercion, NOT a register pin, NOT hardcoded-$N asm, NOT a
   scheduling barrier, NOT a dead store / dead local / constant holder (every local
   in the diff is read), NOT a redundant width cast (the `(s32)` casts convert
   pointers to the `s32` buffer parameter that CdRead's matched definition declares),
   NOT a pointer alias to a global, NOT a do/while(0) wrap, NOT a declaration-order
   trick (moving the CdRead declaration between block scope and the header does not
   change any byte — it changes only whether the 3-argument call is legal C).
 - Block-scope `extern` declarations are ordinary C and pre-exist in this very file
   at src/code6cac_b2_post.c:242 and src/code6cac_b2_post.c:313 (`extern u8
   SpecialCam;`) and in src/system.c:903 (`extern volatile s32 D_800A14D0;`).

## T6 naming-announces-intent: do any names announce coercion?
No. Names used: nbytes, buf, mode, v, dest, sp_buf, sp_buf2, index, cam_base, v0,
CamRot, rot. None is pad/dummy/unused/spill/slack/tail/_buf-style, and every one has
a live read. `sp_buf`/`sp_buf2` follow this repo's existing stack-buffer naming
(src/code6cac_b2_post.c already uses `sp`-prefixed stack arrays) and are the actual
CD destination buffers, not frame padding.

SANCTIONED-FAMILY-CLAIMS: none — this diff contains no coercion construct, so no
sanctioned-family carve-out is claimed or needed.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.

---
HANDOFF NOTE (not part of the vet): the canonical home for construct (1) is
include/code6cac.h:510, replacing `extern void CdRead(s32);` with
`extern s32 CdRead(s32, s32, s32);` to agree with the matched definition at
src/system.c:901 (and with include/m2c_context.h:1123, which already carries the
3-argument form). That path is out of candidate scope for this function, so the
declaration is placed at block scope instead; both placements measure byte-identical
(score 0, 72/72 — the header-edit placement was the 2026-08-26 out-of-scope form).
If the operator prefers the header, the precedent mechanism is a
tools/grinder/scope_allow.txt grant, as already granted for
`replay_camera_Init include/code6cac.h` and `func_80038170 include/code6cac.h`.

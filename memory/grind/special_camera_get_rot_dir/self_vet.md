# SELF-VET — special_camera_get_rot_dir  (session s7, rederive modality, 2026-08-26)

Measured this session with the edits in place in src/:
  `sandbox special_camera_get_rot_dir --disable all` = **score 0**, 72/72 insns.
  `sandbox func_800372F4 --disable all`              = **score 0**, 21/21 insns.
  `verify-oracle` = ok:true, build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa.

All three numbers were re-measured in THIS session against the tree as it now stands, after
the one change made to the inherited draft: the redundant block-scope `extern u8 SpecialCam;`
inside `special_camera_get_rot_dir` was REMOVED (`include/game.h:9` already declares it at
file scope). Removing it is byte-neutral (score 0, 72/72 before and after), which is exactly
why it was removed: a declaration whose placement is not load-bearing has no business in the
body, and its absence means no declaration-order/declaration-placement construct survives
anywhere in this diff. The diff as submitted is `tmp/grind/special_camera_get_rot_dir/s7/
final_diff.patch` (91 lines, two files: `src/code6cac_b2_post.c`, `include/code6cac.h`).

CONSTRUCTS: (1) `typedef struct { s32 rot[15]; } CamRot;` plus one aggregate struct
assignment `*(CamRot *)dest = *(CamRot *)&sp_buf[0x10];`; (2) local `s32 mode = 0x80;`
passed as the third argument at both `func_800372F4` call sites; (3) `func_800372F4`
declared with its true three-parameter signature `(s32 nbytes, s32 buf, s32 mode)` and
its body forwarding both new parameters into the CD read: `CdRead(nbytes >> 11, buf, mode);`
(4) `include/code6cac.h` `CdRead` declaration corrected from `extern void CdRead(s32);`
to `extern s32 CdRead(s32, s32, s32);`; (5) an ordinary `for (;;) { ... continue; ... break; }`
retry loop. NO goto, NO inline asm, NO register pins, NO volatile, NO dead locals, NO
dead stores, NO pad/unused arrays, NO FAKE constructs anywhere in the diff.

## T1 semantic purpose
(1) The struct assignment IS the function's payload: it copies the 60-byte camera-rotation
record from offset 0x10 of the sector buffer into the caller's `dest`. Delete it and the
function returns nothing to its caller. Real observable effect.
(2) `mode` carries the CD transfer mode byte 0x80 (CdlModeSpeed) into the read. It is READ:
it is argument 3 of `func_800372F4`, which forwards it to `CdRead`'s `mode` parameter, which
at `src/system.c:903-916` stores it to `D_800A14DC` and switches on `mode & 0x30` to pick the
transfer word count. Changing its value changes runtime CD behaviour.
(3)/(4) `buf` and `mode` are consumed inside `func_800372F4` by name, at the `CdRead(...)`
call. `buf` is the DMA destination address for the sector data; without it CdRead writes
nowhere. This is the opposite of the 2026-08-26 01:09 layer-1 FAIL, whose objection was
precisely "neither identifier appears anywhere in the function body" — see the ban section
below.
(5) The retry loop is the function's error handling: any non-zero read result restarts the
seek+read sequence. Observable.

## T2 human-programmer
Yes to every construct. A programmer given the spec "seek to this stage's SpecialCam entry,
read a sector, pull the 60-byte rotation record out of it, then seek forward one sector and
read the block described by dest[2]/dest[3]; retry the whole thing on any failure" writes
exactly this: a struct for the record, one assignment to copy it, a named `mode` constant for
the CD mode byte, a helper that wraps "round bytes up to sectors, CdRead, spin on CdReadSync",
and a `for(;;)` with `continue`/`break`. Nothing in the body invites a "why is this here?"
question — there is no statement whose only justification is a byte.

## T3 GCC-internals justification
No construct is justified by a GCC pass. GCC internals appear in this session's ledger in
exactly one role — as forensic explanation of why the earlier hand-written forms were wrong,
not as the reason any surviving construct is present:
  - The inner 4-word copy loop (`.L800373C0` / `bne $a2,$s5`) is NOT written in the C at all.
    It is emitted by the MIPS backend's block-move expander (`config/mips/mips.c`
    `expand_block_move` -> `block_move_loop`) from the single aggregate assignment. Six prior
    sessions chased that loop's register `$s5` through live-length algebra and one of them
    faked it with a hand-written `goto copyloop;` — that construct is BANNED and is ABSENT here.
    The correct C is the plain struct copy, and the whole 4-register rotation falls out.
  - The `mode` local: the load-bearing claim is about the ORIGINAL SOURCE, not about a pass.
    The target holds 0x80 in callee-saved `$s4` across the entire loop and re-uses it at both
    calls (`addu $a2,$s4,$zero` at 0x800373AC and 0x80037434). A literal argument would be
    rematerialised per call site. That is ordinary decomp provenance reasoning — the target
    bytes tell us the original had a variable here — and the variable is independently required
    by the semantics (T1). It is not a lever: it is not placed, ordered, typed, or duplicated
    to steer any pass.
No allocno priority, no `reg_n_refs`, no `reg_live_length`, no scheduling, no REG_EQUIV, no
LUID reasoning is load-bearing for anything in this diff.

## T4 permuter/search provenance
No construct came from a permuter or a directed spelling enumeration. The permuter modality was
run in s4 (~46k iterations, 2 chassis) and produced ZERO score-0 finds; nothing from it is used
here. This form came from re-deriving what the function DOES (a 60-byte record copy out of a CD
sector) and from reading `func_800372F4`'s and `CdRead`'s actual asm/definitions. The two edits
this session (the `CdRead` declaration fix and the `func_800372F4` parameter forwarding) were
derived from `src/system.c:901` and `asm/funcs/func_800372F4.s`, not from a search.

## T5 family check
None of the forbidden families apply, by spelling or by analogy: no register-asm pin, no
hardcoded-`$N` asm, no regfix/asmfix rule, no scheduling barrier, no volatile of any kind, no
alias rename, no unused local array or frame pad, no dead-param assign, no dead conditional
store, no empty-body `if`, no `if (1)`, no dead goto label, no DImode chain, no goto-end
accumulator, no param-local alias, no opaque `s32 one = 1;`, no width-redundant cast, no linker
reorder. Equally, NO sanctioned SOTN family is being claimed — every construct here is ordinary
C that a reader would write from the specification, so no family citation and no `/* FAKE */`
annotation is required or present.

## T6 naming-announces-intent
No `pad`, `dummy`, `unused`, `spill`, `slack`, `tail`, `_buf`, or `_frame_pad` identifier
exists in the diff. Names are `dest`, `sp_buf`, `sp_buf2`, `index`, `cam_base`, `v0`, `mode`,
`CamRot`, `rot`, `nbytes`, `buf`. Every one of them is read; none is declaration-only,
address-of-only, or discarded.

## THE PRIOR BAN — stated explicitly, and why this diff is not it
The 2026-08-26 01:09 layer-1 review FAILed the previous form and banned:
  "s32 func_800372F4(s32 arg0, u32 *buf, s32 mode) { ... } — buf and mode are added to the
   signature but neither identifier appears anywhere in the function body; the sole CD call is
   unchanged: CdRead(arg0 >> 11); (still single-argument, matching include/code6cac.h:510
   extern void CdRead(s32);, also unchanged by this diff)."
That objection was CORRECT and its stated remedy is exactly what this session did. The reviewer's
own next action was: "drop the buf/mode arguments from both call sites UNLESS a genuine semantic
need for them can be shown (e.g. CdRead's real declaration takes 3 args and func_800372F4
actually needs...)". The genuine semantic need is now shown from ground truth in this repo, not
from inference:
  - **`src/system.c:901`** — this project already contains a MATCHED, byte-verified decompile of
    `CdRead` itself, and its definition is `s32 CdRead(s32 sectors, s32 buf, s32 mode)`. It reads
    all three: `D_800A14DC = mode;` (src/system.c:903), `D_800A14D4 = buf;` (src/system.c:919),
    `*ps = sectors;` (src/system.c:920).
  - **`include/code6cac.h:510`** therefore did not merely under-describe an external API — it
    CONTRADICTED a definition that lives in this same repository. `extern void CdRead(s32);`
    against `s32 CdRead(s32,s32,s32)` is a declaration/definition conflict; correcting it is
    ordinary C bug-fixing, not a coercion. (`include/m2c_context.h:1123` independently carries
    the 3-argument form.)
  - The two banned call sites are unchanged in shape but no longer unread: `buf` and `mode`
    now appear by name inside `func_800372F4`, in the `CdRead` call.
  - The banned SELF-VET CLAIM ("$a1/$a2 fall through untouched into CdRead") is NOT made in this
    vet and is not relied on anywhere. This diff does not depend on argument fall-through: the
    parameters are forwarded explicitly in C. (For the record, GCC emits no instruction for
    forwarding an incoming `$a1`/`$a2` to the same outgoing slot, which is why
    `func_800372F4` still measures 21/21 with score 0 — but that is a measured consequence, not
    a justification.)
If the reviewer nonetheless holds that widening `func_800372F4` is banned per se regardless of
the parameters now being read, the correct disposition is a ruling, not a silent respell — the
alternative (a one-argument `func_800372F4` whose caller cannot supply a buffer address) cannot
express what the target executes, since the target demonstrably sets `$a1 = sp+0x10` and
`$a2 = $s4` immediately before `jal func_800372F4` at 0x800373A4-0x800373AC.

SANCTIONED-FAMILY-CLAIMS: none — no construct in this diff belongs to any coercion family,
sanctioned or otherwise. Supporting (non-family) precedent for correcting a header declaration
to agree with an in-repo definition: `src/system.c:901`.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. There is no coercion construct in the diff to
annotate; the four cheat-checklist-relevant constructs are all semantically load-bearing (T1).

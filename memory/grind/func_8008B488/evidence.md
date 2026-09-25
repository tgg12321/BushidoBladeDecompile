# func_8008B488 — evidence (manual s1, 2026-09-25)

## What the function is
Per-voice SPU attribute setter with the shape of PsyQ LIBSPU `SpuSetVoiceAttr`
(`SpuVoiceAttr *`, 0x40 bytes: the callers func_800858D0 / main.c:~1349 build it in
an `s32 buf[16]`). Loops voice 0..23 over `attr->voice`, applies each `mask` block,
then runs the inline `sp14 *= 13` wait loop (two volatile stack slots).
The name stays auto: near-tier-ruling-2026-09-07 lists `SpuSetVoiceAttr` as PROBABLE
with no verbatim caller pinning it.

References (fetched in tmp/): sotn-decomp `src/main/psxsdk/libspu/s_sva.c`
(`_SpuSetVoiceAttr`) and psyz `decomp/src/libspu/sr_sv.c` (`_SpuRSetVoiceAttr`, 4.0).
BB2 links an OLDER build. Differences from both: no min/max range (fixed 0..23),
no `nowait` argument (the wait loop is unconditional), block order
pitch/sample_note/note/VOLL/VOLR/WDSA/LSAX/ADSR1/ADSR2/AR/DR/SR/RR/SL, the clamps
load first and then clamp (`rate = attr->ar; if (rate >= 0x80) rate = 0x7F;`),
the volume is masked `& 0x7FFF` before the mode switch, SR mode defaults to
0x100 when its mode bit is clear, and the switches have no explicit default arms.

## Floor
- Stub: 387/387.
- Closing form, rejected/sotn-shared-rate-ruling5-0.c: sandbox `--disable all` =
  **0/387** with the landing chassis in place, and **full-build SHA1 ==
  62efab4f73f992798c43e8c730aa43baa10bb4fa** (verify-oracle --rebuild
  --allow-dirty, 2026-09-25). On the clean tree (no chassis) the sandbox reads
  4: only the two ADDR_VEC `%lo` addends, operand-only.
- candidate.c = best LANDABLE form (one rate local per ADSR block): **12/387**
  with the chassis, **16/387** on the clean tree; all operand-only (SR block:
  rate/smode swap a1<->a2; SL block: rate in a0, target a1).

## Load-bearing constructs (each measured)
1. `*(volatile u16 *)(_spu_RXX + ...)` for every SPU register access. Without
   volatile, the `& 0xFF` read-modify-write narrows to `lbu` (v2 = 34).
2. `adsr = raw; adsr &= MASK; raw = adsr | x;` in AR/DR/SR/RR (s32 adsr), the
   SOTN/psyz spelling (Ruling 4 compound split). Without the split: 38 (v3).
   SL keeps the single-expression `(adsr & 0xFFF0) | rate`, also SOTN's
   spelling (split there = 6, v4).
3. **`u16 rate` declared once at function scope and reused by the AR, DR, SR, RR
   and SL blocks**. This is SOTN's `var_a2`. Measured alternatives (with the
   chassis in place):
   - one function-scope local per block (`ar_rate`..`sl_rate`, either decl order): 12
   - all block-scoped locals: 12 (rate declared first in each block: 12)
   - SOTN if/else clamp form (`if (attr->ar > 0x7F) rate = 0x7F; else rate = attr->ar;`): 10, 392 insns
   - ternary clamp, per-block: 12; s32 per-block rates: 15
   - sharing subsets: {DR,SL} 18; {SR,RR} 4; {DR,SR,RR,SL} 0; {AR,DR,SR,SL} 0;
     one per ADSR register ({AR,DR,SL} -> ADSR1, {SR,RR} -> ADSR2): 0
     (rejected/rate-per-adsr-register-multiwrite-0.c)
   Mechanism (reading of the diff, not an allocator dump): in the SL block
   nothing but the clamped value is live besides v0/v1, so a block-local pseudo
   takes a0. The target has a1 there, which a global-alloc pseudo gets only
   when it also spans a block where a0 is busy (DR: `andi a0,v0,0xff0f` while
   rate is live). SR's rate/smode seat order flips for the same reason.
   Allocator effect alone is never sufficient under Ruling 5, so this
   construct is NOT landable under the current rules.
4. Pitch store written `(pos + 2) * 2` like every other store; `voice * 16 + 4`
   also scores 0, so the uniform form was kept.
5. The two volume-mode switches have cases 1..7 and no default. The target's
   index is `(s16)(x - 1)` on a `lhu`, matching `switch ((s16)attr->volmode.left)`
   with SpuVolume's s16 fields (same idiom as SpuSetCommonAttr).

## Landing chassis (needed even once the construct question is settled)
The compiler now emits the two ADDR_VECs, so:
- delete the hand-transcribed `const u32 jtbl_80016460[8]` and
  `const u32 jtbl_80016480[7]` at the end of src/main.c;
- remove `main` from `RODATA_ALIGN2_FILES` (Makefile:136) and from its mirror
  in engine/buildconfig.py. The target has a zero word at 0x8001647C between
  the two tables, which is final.c's unconditional `.align 3` before the second
  table (decisions.md 2026-09-20 func_800747D8 s10 forensics). All five main.c
  ADDR_VECs (0x800163C0, 0x80016420, 0x80016440, 0x80016460, 0x80016480) are
  8-aligned and main.o(.rodata) starts at 0x800163C0, so dropping the
  downgrade changes nothing else. Verified: build/src/main.o .rodata 2**3,
  D_8001649C at .rodata+0xDC (0x8001649C), full SHA1 == oracle.
- update the jtbl comment block above D_8001649C and the stale
  `(already so at HEAD; the sole caller func_8008B488 is INCLUDE_ASM)` line in
  the _spu_note2pitch comment.
Patches as spliced for the oracle run: tmp/f8b488/landing_main_c.patch and
tmp/f8b488/landing_build.patch (tmp/ is gitignored; regenerate from this note).

## Manual s2 (2026-09-25): allocator dump — why every per-block form stops at 12
Standalone harness: tmp/f8b488s2/ (head.h + pp.sh + compile.sh + score.py; the
standalone TU reproduces the sandbox exactly: candidate 12 + 1 jtbl addend,
shared-rate form 0 + 1 addend). Instrumented cc1 (tools/gcc-2.7.2/cc1,
BB2_ALLOC_DEBUG / BB2_FINDREG_DEBUG) on candidate.c, pseudos 77..81 = ar..sl_rate:
- **SL (4 hunks).** sl_rate (p81, nrefs 8, livelen 10, pri 24000) conflicts
  ONLY with v0, v1, sp; someone_prefers empty; no own preferences. find_reg
  therefore takes the first free reg, a0. For the target's a1, the SL value's
  allocno must conflict with a0 (or a lower-priority conflicting allocno must
  prefer a0). Nothing in the target's SL block touches a0, the loop-wide
  pseudos cross calls (so prune_preferences strips call-used regs from their
  preferences), and there is no call in the block. So the SL value's pseudo
  has to be live in some OTHER block where a0 is busy, i.e. be one pseudo
  across blocks. **No per-block spelling can produce a1 for SL** (global.c
  find_reg/prune_preferences, read 2026-09-25). Measured corroboration:
  sharing only {AR,SR} leaves exactly the 4 SL hunks (tmp/f8b488s2/probe_arsr.c).
- **SR (8 hunks).** smode (p253: 5 refs x loop weight 2 = 10, livelen 27,
  pri 11111) outranks sr_rate (p79: 4 refs = 8, livelen 31, pri 7741), so smode
  is allocated first and takes a1 (a0 = masked adsr). The target needs rate
  first. Rate's live range necessarily contains smode's plus the load+clamp,
  and its RTL at flow has 4 refs (set, zero_extend for the compare, set 0x7F,
  ior) against smode's 5 (0x100, 0, 0x200, 0x300, ior), so rate can never
  outrank smode unless its refs come from other blocks (shared pseudo:
  nrefs 40, pri 21739). Type/spelling probes that leave the output unchanged
  or worse: sr/sl s32/u32, smode u16/u32, `> 0x7F`, operand swaps, ternary,
  `a < b ? a : 0x7F` (MIN_EXPR), inline clamp helper (16), block-local
  load + if/else copy (14, 388 insns), all 13 incl. addend unless noted.
- **Permuter** (first run on this function; hand workspace tmp/f8b488s2/perm,
  standalone TU, 2 workers, random mode from candidate.c, ~2.8k iterations):
  base 65. Its one find (45) writes `sl_rate` in the note block too
  (`sl_rate = D_800A28A4[voice]; center = sl_rate;`). That makes one pseudo
  span a block where a0 holds call arguments, and SL moves to a1, exactly as
  the proof predicts. Inadmissible (two roles), kept as evidence:
  rejected/perm-sl-shares-center-8.c.
- Construct status: the shared-`rate` closing form was ruled inadmissible by
  a layer-2 construct ruling on 2026-09-25 (Ruling 5 1(a)/(b)/(c)/(e), Ruling
  6; Ruling 8 not citable). That settles it; it is not pending the owner.

## Rejected bodies
rejected/*.c: block-scoped-locals-12 and rate-per-adsr-register-multiwrite-0 are
function-only files (no typedef) and were scored with the chassis in place.
sotn-shared-rate-ruling5-0, sotn-ifelse-clamp-10 and volatile-no-and-split-38
carry the typedef.
